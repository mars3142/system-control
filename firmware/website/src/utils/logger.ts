type LogMeta = unknown;

type LoggerFn = (message: string, meta?: LogMeta) => void;

const DEFAULT_DEV_LOGGING_ENABLED =
	import.meta.env.DEV &&
	import.meta.env.MODE !== 'test' &&
	import.meta.env.VITE_DEV_LOGGING !== 'false';

let devLoggingEnabled = DEFAULT_DEV_LOGGING_ENABLED;

const emit = (fn: (...data: unknown[]) => void, scope: string, message: string, meta?: LogMeta) => {
	if (!devLoggingEnabled) return;
	if (meta === undefined) {
		fn(`[${scope}] ${message}`);
		return;
	}
	fn(`[${scope}] ${message}`, meta);
};

export const setDevLoggingEnabled = (enabled: boolean) => {
	devLoggingEnabled = enabled;
};

export const createLogger = (scope: string): Record<'debug' | 'info' | 'warn' | 'error', LoggerFn> => ({
	debug: (message, meta) => emit(console.debug, scope, message, meta),
	info: (message, meta) => emit(console.info, scope, message, meta),
	warn: (message, meta) => emit(console.warn, scope, message, meta),
	error: (message, meta) => emit(console.error, scope, message, meta)
});
