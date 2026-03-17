import { afterEach, describe, expect, it, vi, type Mock } from 'vitest';
import { get } from 'svelte/store';

type FetchResponse = {
	ok: boolean;
	status?: number;
	statusText?: string;
	json: () => Promise<unknown>;
};

const okResponse = (payload: unknown = {}): FetchResponse => ({
	ok: true,
	json: async () => payload
});

const deferred = <T>() => {
	let resolve!: (value: T) => void;
	let reject!: (reason?: unknown) => void;
	const promise = new Promise<T>((res, rej) => {
		resolve = res;
		reject = rej;
	});
	return { promise, resolve, reject };
};

describe('controlStore', () => {
	afterEach(() => {
		vi.restoreAllMocks();
		vi.unstubAllGlobals();
		vi.resetModules();
	});

	async function setupStore(mock: Mock = vi.fn().mockResolvedValue(okResponse())) {
		vi.stubGlobal('fetch', mock);
		const { controlStore } = await import('./controlStore');
		return { fetchMock: mock, controlStore };
	}

	it('fetchState merges server data with defaults', async () => {
		const { fetchMock, controlStore } = await setupStore(
			vi.fn().mockResolvedValue(okResponse({ on: true }))
		);
		await controlStore.fetchState();

		expect(fetchMock).toHaveBeenCalledWith(
			expect.stringMatching(/\/api\/light\/status$/),
			undefined
		);
		expect(get(controlStore)).toEqual({
			on: true,
			mode: 'day',
			schema: 'schema_01.csv',
			color: { r: 0, g: 0, b: 0 },
			clock: '00:00'
		});
	});

	it('update values posts updates and applies returned state', async () => {
		const { fetchMock, controlStore } = await setupStore(
			vi.fn().mockResolvedValueOnce(okResponse({}))
		);
		await controlStore.setMode({ mode: 'night' });

		expect(fetchMock).toHaveBeenCalledWith(expect.stringMatching(/\/api\/light\/mode$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ mode: 'night' })
		});
		expect(get(controlStore)).toEqual({
			on: false,
			mode: 'night',
			schema: 'schema_01.csv',
			color: { r: 0, g: 0, b: 0 },
			clock: '00:00'
		});
	});

	it('fetchState throws on non-ok response', async () => {
		const { controlStore } = await setupStore(
			vi.fn().mockResolvedValue({
				ok: false,
				status: 500,
				statusText: 'Internal Server Error',
				json: async () => ({})
			})
		);

		await expect(controlStore.fetchState()).rejects.toThrow(
			'Request failed: 500 Internal Server Error'
		);
	});

	it('setLight posts to /api/light/power', async () => {
		const { fetchMock, controlStore } = await setupStore();
		await controlStore.setLight({ on: true });

		expect(fetchMock).toHaveBeenCalledWith(expect.stringMatching(/\/api\/light\/power$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ on: true })
		});
	});

	it('setMode posts to /api/light/mode', async () => {
		const { fetchMock, controlStore } = await setupStore();
		await controlStore.setMode({ mode: 'night' });

		expect(fetchMock).toHaveBeenCalledWith(expect.stringMatching(/\/api\/light\/mode$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ mode: 'night' })
		});
	});

	it('setLight coalesces pending updates while request is in flight', async () => {
		const first = deferred<FetchResponse>();
		const { fetchMock, controlStore } = await setupStore(
			vi.fn()
				.mockImplementationOnce(() => first.promise)
				.mockResolvedValue(okResponse())
		);

		const p1 = controlStore.setLight({ on: true });
		const p2 = controlStore.setLight({ on: false });
		const p3 = controlStore.setLight({ on: true });

		expect(fetchMock).toHaveBeenCalledTimes(1);

		first.resolve(okResponse());
		await Promise.all([p1, p2, p3]);

		expect(fetchMock).toHaveBeenCalledTimes(2);
		expect(fetchMock).toHaveBeenNthCalledWith(2, expect.stringMatching(/\/api\/light\/power$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ on: true })
		});
	});

	it('setLight with true,false,true,false sends first and last payload only', async () => {
		const first = deferred<FetchResponse>();
		const { fetchMock, controlStore } = await setupStore(
			vi.fn()
				.mockImplementationOnce(() => first.promise)
				.mockResolvedValue(okResponse())
		);

		const p1 = controlStore.setLight({ on: true });
		const p2 = controlStore.setLight({ on: false });
		const p3 = controlStore.setLight({ on: true });
		const p4 = controlStore.setLight({ on: false });

		expect(fetchMock).toHaveBeenCalledTimes(1);
		expect(fetchMock).toHaveBeenNthCalledWith(1, expect.stringMatching(/\/api\/light\/power$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ on: true })
		});

		first.resolve(okResponse());
		await Promise.all([p1, p2, p3, p4]);

		expect(fetchMock).toHaveBeenCalledTimes(2);
		expect(fetchMock).toHaveBeenNthCalledWith(2, expect.stringMatching(/\/api\/light\/power$/), {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ on: false })
		});
	});
});
