import { createLogger } from './logger';

const isBrowser = typeof window !== 'undefined';

export const resolveHost = () => {
	if (import.meta.env.DEV) return 'system-control.local';
	return isBrowser ? window.location.host : '';
};

export const buildBaseUrl = (host: string) => {
	if (!host) return '';
	const protocol = isBrowser ? window.location.protocol : import.meta.env.DEV ? 'http:' : 'https:';
	return `${protocol}//${host}`;
};

const host = resolveHost();
export const baseUrl = buildBaseUrl(host);

const log = createLogger('apiClient');

/**
 * Führt einen fetch-Request durch, prüft auf Fehler und parst die JSON-Antwort.
 */
export async function requestJson<T>(path: string, init?: RequestInit): Promise<T> {
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
