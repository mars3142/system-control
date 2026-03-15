import { writable } from 'svelte/store';

// Types for state and REST/WebSocket messages
export interface ControlState {
	on: boolean;
	mode: string;
	schema?: string;
	color?: { r: number; g: number; b: number };
	clock?: string;

	[key: string]: any;
}

const createControlStore = () => {
	const store = writable<ControlState>({
		on: false,
		mode: 'day',
		schema: 'schema_01.csv',
		color: { r: 0, g: 0, b: 0 },
		clock: '00:00'
	});
	const { subscribe, set } = store;

	// Centralized host and URL configuration
	const host = import.meta.env.DEV
		? 'system-control.local'
		: typeof window !== 'undefined'
			? window.location.host
			: '';
	const baseUrl = import.meta.env.DEV ? `http://${host}` : '';

	let ws: WebSocket | null = null;
	let wsReconnectTimer: ReturnType<typeof setTimeout> | null = null;

	async function fetchState() {
		const res = await fetch(`${baseUrl}/api/light/status`);
		if (!res.ok) throw new Error('Failed to fetch state');
		const data = await res.json();
		set(data);
	}

	async function setState(partial: Partial<ControlState>) {
		const res = await fetch(`${baseUrl}/api/status`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(partial)
		});
		if (!res.ok) throw new Error('Failed to update state');
		const data = await res.json();
		set(data);
	}

	function connectWebSocket() {
		if (typeof window === 'undefined') return;

		const wsProtocol =
			window.location.protocol === 'https:' && !import.meta.env.DEV ? 'wss:' : 'ws:';
		const wsUrl = `${wsProtocol}//${host}/ws`;

		ws = new WebSocket(wsUrl);

		ws.onopen = () => {
			if (wsReconnectTimer) clearTimeout(wsReconnectTimer);
			ws?.send(JSON.stringify({ type: 'getStatus' }));
		};
		ws.onmessage = (event) => {
			try {
				const data = JSON.parse(event.data);
				if (data.type === 'status') set(data);
			} catch (e) {
				// ignore
			}
		};
		ws.onclose = () => {
			ws = null;
			wsReconnectTimer = setTimeout(connectWebSocket, 3000);
		};
		ws.onerror = () => {
			ws?.close();
		};
	}

	if (typeof window !== 'undefined') connectWebSocket();

	return {
		...store,
		fetchState,
		setState
	};
};

export const controlStore = createControlStore();
