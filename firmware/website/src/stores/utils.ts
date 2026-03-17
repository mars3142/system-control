type Deferred = {
    resolve: () => void;
    reject: (error: unknown) => void;
};

export const createLatestOnlySender = <T extends object>(
    send: (payload: T) => Promise<void>,
    merge: (current: T | null, incoming: T) => T
) => {
    let inFlight = false;
    let pending: T | null = null;
    const waiters: Deferred[] = [];

    const flush = async () => {
        if (inFlight) return;
        inFlight = true;

        try {
            while (pending) {
                const nextPayload = pending;
                pending = null;
                await send(nextPayload);
            }

            const done = waiters.splice(0);
            done.forEach(({ resolve }) => resolve());
        } catch (error) {
            const failed = waiters.splice(0);
            pending = null;
            failed.forEach(({ reject }) => reject(error));
        } finally {
            inFlight = false;
            if (pending) {
                void flush();
            }
        }
    };

    return (incoming: T): Promise<void> => {
        pending = merge(pending, incoming);
        return new Promise<void>((resolve, reject) => {
            waiters.push({ resolve, reject });
            void flush();
        });
    };
};
