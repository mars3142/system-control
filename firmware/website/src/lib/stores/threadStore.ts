import { writable } from 'svelte/store';
import { createLogger } from '../utils/logger';
import { requestJson, resolveHost } from '../utils/apiClient';

export interface ThreadNetworkInfo {
	networkName: string;
	panId: string;
	channel: number;
	status: 'offline' | 'active';
}

export interface ThreadState {
	networkInfo?: ThreadNetworkInfo;
	isPermitJoinActive: boolean;
	permitJoinTimeLeft: number;
	joinedDevicesCount: number;
}

const DEFAULT_THREAD_STATE: ThreadState = {
	isPermitJoinActive: false,
	permitJoinTimeLeft: 0,
	joinedDevicesCount: 0
};

const STATUS_ENDPOINT = '/api/thread/status';
const PERMIT_JOIN_ENDPOINT = '/api/thread/permit-join';
const WS_ENDPOINT = '/ws';
const WS_RECONNECT_DELAY_MS = 3000;

const isBrowser = typeof window !== 'undefined';

const buildWebSocketUrl = (host: string) => {
	if (!isBrowser) return '';
	const wsProtocol = window.location.protocol === 'https:' && !import.meta.env.DEV ? 'wss:' : 'ws:';
	return `${wsProtocol}//${host}${WS_ENDPOINT}`;
};

const parseJson = (raw: string): any => {
	try {
		return JSON.parse(raw);
	} catch {
		return null;
	}
};

const createThreadStore = () => {
	const log = createLogger('threadStore');
	const store = writable<ThreadState>({ ...DEFAULT_THREAD_STATE });
	const { subscribe: internalSubscribe, set, update } = store;

	const host = resolveHost();
	const wsUrl = buildWebSocketUrl(host);

	let ws: WebSocket | null = null;
	let wsReconnectTimer: ReturnType<typeof setTimeout> | null = null;
	let shouldReconnect = true;
	let subscriberCount = 0;

	const applyState = (nextState: Partial<ThreadState>) => {
		update(current => ({ ...current, ...nextState }));
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

	async function fetchState() {
		try {
			const data = await requestJson<Partial<ThreadState>>(STATUS_ENDPOINT);
			applyState(data);
		} catch (error) {
			log.error('Failed to fetch initial thread state', error);
		}
	}

	async function startPermitJoin(durationSeconds: number = 60) {
		try {
			// Optimistic UI update
			applyState({ 
				isPermitJoinActive: true, 
				permitJoinTimeLeft: durationSeconds,
				joinedDevicesCount: 0
			});
			
			await requestJson(PERMIT_JOIN_ENDPOINT, { 
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({ duration: durationSeconds })
			});
		} catch (error) {
			log.error('Failed to start permit join', error);
			applyState({ isPermitJoinActive: false, permitJoinTimeLeft: 0 });
		}
	}

	function connectWebSocket() {
		if (!isBrowser || ws || !wsUrl) return;
		log.info('Connecting WebSocket', { url: wsUrl });

		const socket = new WebSocket(wsUrl);
		ws = socket;

		socket.onopen = () => {
			clearReconnectTimer();
			log.info('WebSocket connected');
		};

		socket.onmessage = (event) => {
			const message = parseJson(event.data);
			if (!message || !message.type) return;

			if (message.type === 'thread_permit_join_status') {
				log.debug('Received permit join status');
				applyState({ 
					isPermitJoinActive: message.active,
					permitJoinTimeLeft: message.timeLeft || 0
				});
			} else if (message.type === 'thread_device_joined') {
				log.debug('A new device joined the Thread network!');
				update(state => ({
					...state,
					joinedDevicesCount: state.joinedDevicesCount + 1
				}));
			} else if (message.type === 'thread_network_status') {
				log.debug('Received thread network status update');
				applyState({ networkInfo: message.networkInfo });
			}
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

	const subscribe: typeof store.subscribe = (run, invalidate) => {
		subscriberCount += 1;
		if (subscriberCount === 1) {
			log.debug('First subscriber attached - starting WebSocket');
			shouldReconnect = true;
			connectWebSocket();
			fetchState();
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
		startPermitJoin
	};
};

export const threadStore = createThreadStore();
