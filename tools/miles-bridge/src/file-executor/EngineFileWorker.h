#ifndef MILES_ENGINE_FILE_WORKER30_H
#define MILES_ENGINE_FILE_WORKER30_H

namespace MilesFileExecutor30
{
class EngineFileThread;

// Private engine-side mechanism, not an admission/registry/lifecycle policy.
// No STL objects, allocation ownership, or exceptions cross this interface.
class EngineFileWorker
{
public:
    enum StartResult { Started, AlreadyStarted, AllocationFailed, ThreadCreationFailed };
    typedef void (*Execute)(void *); // Must not throw; context owner retains it.

    static EngineFileWorker *create(); // NULL on caught allocation exception.
    // Engine MemoryManager may instead FATAL; recovery is not guaranteed.
    static void destroy(EngineFileWorker *); // Same-TU allocation/destruction.
    // Owner MUST drainAndJoin before destroy; never implicit teardown.
    StartResult start(); // Owner thread only; returns after run() signals TLS-ready.
    bool submit(void *context, Execute execute); // Nonblocking; false = not queued.
    // Owner only, after vendor production ended and every required close queued.
    // Closes intake, drains FIFO, joins through TLS removal; never call on worker.
    // Failed join returns false and RETAINS ownership; do not destroy resources.
    bool drainAndJoin();

private:
    EngineFileWorker();
    ~EngineFileWorker();
    struct Impl;
    Impl *impl;
    void run();
    friend class EngineFileThread;
    EngineFileWorker(const EngineFileWorker &);
    EngineFileWorker &operator=(const EngineFileWorker &);
};
}
#endif
