// Compile with the engine's actual STLport/FirstSharedFoundation settings.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "EngineFileWorker.h"
#include "sharedSynchronization/Gate.h"
#include "sharedSynchronization/Mutex.h"
#include <string>
#include "sharedThread/Thread.h"
#include <stdlib.h>

namespace MilesFileExecutor30
{
class EngineFileThread : public Thread
{
public:
    explicit EngineFileThread(EngineFileWorker &value) : Thread("MilesFile"), owner(value)
    {
        handle = NULL;
        id = 0;
    }
    bool checkedJoin()
    {
        return WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
    }
    bool startOwned()
    {
        ref(); // Retain owner's reference in addition to Thread's initial self ref.
        Thread::start();
        return handle != NULL;
    }
private:
    EngineFileThread(const EngineFileThread &);
    EngineFileThread &operator=(const EngineFileThread &);
    void run() { owner.run(); }
    EngineFileWorker &owner;
};

struct EngineFileWorker::Impl
{
    struct Item
    {
        void *context;
        Execute execute;
        Item *next;
        Item(void *c, Execute e) : context(c), execute(e), next(NULL) {}
    };
    Mutex mutex;
    Gate ready, pending;
    EngineFileThread *thread;
    Item *first, *last;
    bool accepting, stopping, used;
    Impl() : ready(false), pending(false), thread(NULL), first(NULL), last(NULL),
             accepting(false), stopping(false), used(false) {}
};

EngineFileWorker *EngineFileWorker::create()
{
    try { return new EngineFileWorker; }
    catch (...) { return NULL; }
}
void EngineFileWorker::destroy(EngineFileWorker *worker) { delete worker; }
EngineFileWorker::EngineFileWorker() : impl(new Impl) {}
EngineFileWorker::~EngineFileWorker()
{
    // Joining here could deadlock a shutdown dependency. Refuse premature destroy.
    if (impl->thread || impl->first)
        ::abort();
    delete impl;
}

EngineFileWorker::StartResult EngineFileWorker::start()
{
    // Owner-only start/join; submit is not exposed to intake before Started.
    if (impl->used)
        return AlreadyStarted;
    EngineFileThread *thread;
    try { thread = new EngineFileThread(*this); }
    catch (...) { return AllocationFailed; }
    impl->used = true;
    impl->thread = thread;
    if (!thread->startOwned())
    {
        impl->thread = NULL;
        thread->deref(); // No native thread exists to release its initial self ref.
        thread->deref(); // Release owner ref; destruction occurs in this TU/module.
        return ThreadCreationFailed;
    }
    impl->ready.wait(); // Signaled only inside run, AFTER Thread::threadFunc TLS setup.
    impl->mutex.enter();
    impl->accepting = true;
    impl->mutex.leave();
    return Started;
}

bool EngineFileWorker::submit(void *context, Execute execute)
{
    if (!context || !execute)
        return false;
    Impl::Item *item;
    try { item = new Impl::Item(context, execute); }
    catch (...) { return false; }
    impl->mutex.enter();
    if (!impl->accepting)
    {
        impl->mutex.leave();
        delete item;
        return false;
    }
    if (impl->last)
        impl->last->next = item;
    else
        impl->first = item;
    impl->last = item;
    impl->pending.open();
    impl->mutex.leave();
    return true;
}

void EngineFileWorker::run()
{
    impl->ready.open();
    for (;;)
    {
        impl->pending.wait();
        impl->mutex.enter();
        Impl::Item *item = impl->first;
        if (!item)
        {
            bool const stop = impl->stopping;
            if (!stop)
                impl->pending.close();
            impl->mutex.leave();
            if (stop)
                return; // Thread::threadFunc removes TLS, then releases self ref.
            continue;
        }
        impl->first = item->next;
        if (!impl->first)
        {
            impl->last = NULL;
            if (!impl->stopping)
                impl->pending.close();
        }
        impl->mutex.leave();
        // No queue lock, receive-loop lock, or engine FileStreamer gate held here.
        // Only this worker invokes queued operations; no nested pumping during read.
        try { item->execute(item->context); }
        catch (...) { ::abort(); } // ABI contract breach; never escape Thread::run.
        delete item; // Only the engine-owned queue node, never typed caller context.
    }
}

bool EngineFileWorker::drainAndJoin()
{
    if (!impl->thread)
        return true;
    if (Thread::getCurrentThread() == impl->thread)
        return false;
    impl->mutex.enter();
    impl->accepting = false;
    impl->stopping = true;
    impl->pending.open();
    impl->mutex.leave();
    if (!impl->thread->checkedJoin())
        return false; // Keep owner ref/thread/resources: termination is unproved.
    impl->thread->deref(); // Join completed, including PerThreadData::threadRemove.
    impl->thread = NULL;
    return true;
}
}
