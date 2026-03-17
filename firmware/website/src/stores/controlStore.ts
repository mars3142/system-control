import { writable } from 'svelte/store';
import { createLogger } from './logger';
import { createLatestOnlySender } from './common';

// Types for state and REST/WebSocket messages
export interface ControlState {
	on: boolean;
	mode: string;
	schema?: string;
	color?: { r: number; g: number; b: number };
	clock?: string;
}

interface StatusMessage extends Partial<ControlState> {
	type: 'status';
}

export const DEFAULT_CONTROL_STATE: ControlState = {
	on: false,
	mode: 'day',
	schema: 'schema_01.csv',
	color: { r: 0, g: 0, b: 0 },
	clock: '00:00'
};

export const createDefaultControlState = (): ControlState => ({
	...DEFAULT_CONTROL_STATE,
	color: DEFAULT_CONTROL_STATE.color ? { ...DEFAULT_CONTROL_STATE.color } : undefined
});

const STATUS_ENDPOINT = '/api/light/status';
const LIGHT_ENDPOINT = '/api/light/power';
const MODE_ENDPOINT = '/api/light/mode';
const SCHEMA_ENDPOINT = '/api/light/schema';
const WS_ENDPOINT = '/ws';
const WS_RECONNECT_DELAY_MS = 3000;

const isBrowser = typeof window !== 'undefined';

const resolveHost = () => {
	if (import.meta.env.DEV) return 'system-control.local';
	return isBrowser ? window.location.host : '';
};

const buildBaseUrl = (host: string) => {
	if (!host) return '';
	const protocol = isBrowser ? window.location.protocol : import.meta.env.DEV ? 'http:' : 'https:';
	return `${protocol}//${host}`;
};

const buildWebSocketUrl = (host: string) => {
	if (!isBrowser) return '';
	const wsProtocol =
		window.location.protocol === 'https:' && !import.meta.env.DEV ? 'wss:' : 'ws:';
	return `${wsProtocol}//${host}${WS_ENDPOINT}`;
};

const isStatusMessage = (value: unknown): value is StatusMessage => {
	if (!value || typeof value !== 'object') return false;
	return (value as { type?: unknown }).type === 'status';
};

const parseJson = (raw: string): unknown => {
	try {
		return JSON.parse(raw);
	} catch {
		return null;
	}
};

const createControlStore = () => {
	const log = createLogger('controlStore');
	const store = writable<ControlState>(createDefaultControlState());
	const { subscribe: internalSubscribe, set } = store;
	type StoreSubscribe = typeof internalSubscribe;
	type StoreRun = Parameters<StoreSubscribe>[0];
	type StoreInvalidate = Parameters<StoreSubscribe>[1];

	const host = resolveHost();
	const baseUrl = buildBaseUrl(host);
	const wsUrl = buildWebSocketUrl(host);

	let ws: WebSocket | null = null;
	let wsReconnectTimer: ReturnType<typeof setTimeout> | null = null;
	let shouldReconnect = true;
	let subscriberCount = 0;

	const applyState = (nextState: Partial<ControlState>) => {
		set({ ...createDefaultControlState(), ...nextState });
	};

	const clearReconnectTimer = () => {
		if (!wsReconnectTimer) return;
		clearTimeout(wsReconnectTimer);
		wsReconnectTimer = null;
	};

	const scheduleReconnect = () => {
		if (!shouldReconnect || wsReconnectTimer) return;
		log.info('Scheduling WebSocket reconnect', { delayMs: WS_RECONNECT_DELAY_MS });
		wsReconnectTimer = setTimeout(() => {
			wsReconnectTimer = null;
			connectWebSocket();
		}, WS_RECONNECT_DELAY_MS);
	};

	async function requestJson<T>(path: string, init?: RequestInit): Promise<T> {
		log.debug('HTTP request', { path, method: init?.method ?? 'GET' });
		const res = await fetch(`${baseUrl}${path}`, init);
		if (!res.ok) {
			log.warn('HTTP request failed', {
				path,
				status: res.status,
				statusText: res.statusText
			});
			throw new Error(`Request failed: ${res.status} ${res.statusText}`);
		}
		log.debug('HTTP request succeeded', { path, status: res.status });
		return (await res.json()) as T;
	}

	async function fetchState() {
		const data = await requestJson<Partial<ControlState>>(STATUS_ENDPOINT);
		applyState(data);
	}

	const sendLightLatestOnly = createLatestOnlySender<Partial<ControlState>>(
		async (payload) => {
			await requestJson<Partial<ControlState>>(LIGHT_ENDPOINT, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify(payload)
			});
			applyState(payload);
		},
		(current, incoming) => ({ ...(current ?? {}), ...incoming })
	);

	const sendModeLatestOnly = createLatestOnlySender<Partial<ControlState>>(
		async (payload) => {
			await requestJson<Partial<ControlState>>(MODE_ENDPOINT, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify(payload)
			});
			applyState(payload);
		},
		(current, incoming) => ({ ...(current ?? {}), ...incoming })
	);

	const sendSchemaLatestOnly = createLatestOnlySender<Partial<ControlState>>(
		async (payload) => {
			await requestJson<Partial<ControlState>>(SCHEMA_ENDPOINT, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify(payload)
			});
			applyState(payload);
		},
		(current, incoming) => ({ ...(current ?? {}), ...incoming })
	);

	async function setLight(partial: Partial<ControlState>) {
		await sendLightLatestOnly(partial);
	}

	async function setMode(partial: Partial<ControlState>) {
		await sendModeLatestOnly(partial);
	}

	async function setSchema(partial: Partial<ControlState>) {
		await sendSchemaLatestOnly(partial);
	}

	function connectWebSocket() {
		if (!isBrowser || ws || !wsUrl) return;
		log.info('Connecting WebSocket', { url: wsUrl });

		const socket = new WebSocket(wsUrl);
		ws = socket;

		socket.onopen = () => {
			clearReconnectTimer();
			log.info('WebSocket connected');
			socket.send(JSON.stringify({ type: 'getStatus' }));
		};
		socket.onmessage = (event) => {
			const message = parseJson(event.data);
			if (!isStatusMessage(message)) {
				log.debug('Ignoring non-status WebSocket message');
				return;
			}
			const { type: _type, ...state } = message;
			log.debug('Applying status update from WebSocket');
			applyState(state);
		};
		socket.onclose = () => {
			if (ws === socket) ws = null;
			log.warn('WebSocket closed');
			scheduleReconnect();
		};
		socket.onerror = () => {
			log.error('WebSocket error');
			socket.close();
		};
	}

	function disconnectWebSocket() {
		shouldReconnect = false;
		clearReconnectTimer();
		ws?.close();
		ws = null;
	}

	const subscribe: typeof store.subscribe = (
		run: StoreRun,
		invalidate?: StoreInvalidate
	) => {
		subscriberCount += 1;
		if (subscriberCount === 1) {
			log.debug('First subscriber attached - starting WebSocket');
			shouldReconnect = true;
			connectWebSocket();
		}

		const unsubscribe = internalSubscribe(run, invalidate);

		return () => {
			unsubscribe();
			subscriberCount -= 1;
			if (subscriberCount === 0) {
				log.debug('Last subscriber removed - stopping WebSocket');
				disconnectWebSocket();
			}
		};
	};

	return {
		subscribe,
		fetchState,
		setLight,
		setMode,
		setSchema
	};
};

export const controlStore = createControlStore();
