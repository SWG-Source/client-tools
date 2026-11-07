// Compile exclusively with actual engine headers and engine_worker_flags.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "engine_worker_context.h"
#include "../src/file-executor/EngineFileWorker.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedThread/SetupSharedThread.h"
#include <string>
#include "sharedThread/Thread.h"

namespace {
int setupState = 0;
DWORD ownerThread = 0;
enum { JobCount = 32 };
struct ProbeState;
struct Job {
    ProbeState *owner;
    unsigned ordinal;
};
struct ProbeState {
    Job jobs[JobCount];
    unsigned completed;
    bool valid;
    Thread *workerThread;
    DWORD workerId;
    ProbeState() : completed(0), valid(true), workerThread(0), workerId(0) {
        for (unsigned i = 0; i < JobCount; ++i) {
            jobs[i].owner = this;
            jobs[i].ordinal = i;
        }
    }
};
void executeJob(void *opaque) {
    Job &job = *static_cast<Job *>(opaque);
    ProbeState &state = *job.owner;
    Thread *thread = Thread::getCurrentThread();
    const DWORD id = GetCurrentThreadId();
    if (!PerThreadData::isThreadInstalled() || !thread ||
        thread == Thread::getMainThread() || id == ownerThread ||
        state.completed != job.ordinal) {
        state.valid = false;
        return;
    }
    if (!state.completed) {
        state.workerThread = thread;
        state.workerId = id;
        PerThreadData::setDebugPrintFlags(0x42);
    } else if (state.workerThread != thread || state.workerId != id ||
               PerThreadData::getDebugPrintFlags() != 0x42) {
        state.valid = false;
    }
    ++state.completed;
}
}

extern "C" int setupEngineProbe() {
    if (setupState == 1)
        return GetCurrentThreadId() == ownerThread ? 0 : 1;
    if (setupState != 0)
        return 2; // Never retry partially completed engine installation.
    setupState = -1;
    ownerThread = GetCurrentThreadId();
    try {
        // SetupSharedThread installs PerThreadData first, then Thread. Thread
        // registers its ordinary exit entry; this helper never executes it.
        if (PerThreadData::isThreadInstalled())
            return 3;
        SetupSharedThread::install();
        if (!PerThreadData::isThreadInstalled() || !Thread::getMainThread() ||
            Thread::getCurrentThread() != Thread::getMainThread())
            return 4;
        setupState = 1;
        return 0;
    } catch (...) {
        return 5;
    }
}

extern "C" int runEngineWorkerProbe() {
    if (setupState != 1 || GetCurrentThreadId() != ownerThread)
        return 10;
    try {
        const int ownerFlags = PerThreadData::getDebugPrintFlags();
        // Heap-retain work context on any uncertain join; pending work must not
        // refer to a returned stack frame. Caller exits the test on failure.
        ProbeState *state = new ProbeState;
        MilesFileExecutor30::EngineFileWorker *worker =
            MilesFileExecutor30::EngineFileWorker::create();
        if (!worker) { delete state;return 11; }
        if (worker->start() != MilesFileExecutor30::EngineFileWorker::Started) {
            MilesFileExecutor30::EngineFileWorker::destroy(worker);
            delete state;return 12;
        }
        bool accepted = true;
        for (unsigned i = 0; i < JobCount; ++i) {
            if (!worker->submit(&state->jobs[i], executeJob)) {
                accepted = false;break;
            }
        }
        if (!worker->drainAndJoin())
            return 13; // Retain worker/state until process exit on unproved join.
        const bool passed = accepted && state->valid && state->completed == JobCount &&
            PerThreadData::isThreadInstalled() && Thread::getCurrentThread() == Thread::getMainThread() &&
            PerThreadData::getDebugPrintFlags() == ownerFlags;
        // Genuine Thread::threadFunc removes worker TLS before thread termination;
        // drainAndJoin waits for that termination before releasing its owner ref.
        MilesFileExecutor30::EngineFileWorker::destroy(worker);
        delete state;
        return passed ? 0 : 14;
    } catch (...) {
        return 15;
    }
}

extern "C" bool engineProbeThreadReady() {
    return setupState == 1 && PerThreadData::isThreadInstalled() &&
        Thread::getCurrentThread() != 0;
}
