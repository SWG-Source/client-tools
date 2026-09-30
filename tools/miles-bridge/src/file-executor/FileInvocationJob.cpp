// Compile separately using the adapter's native modern STL, never engine STLport.
#include "FileInvocationJob.h"
#include <windows.h>
#include <cstdlib>
#include <stdexcept>

namespace MilesFileExecutor30
{
struct FileInvocationJob::State
{
    MilesFileChannel26::Invocation invocation;
    HANDLE completed;
    bool invoked, unexpected;
    State(const MilesFileChannel26::Request &request,
          const MilesFileChannel26::FileServices &services,
          std::shared_ptr<const MilesFileChannel26::Binding> binding,
          std::shared_ptr<void> pin)
        : invocation(request, services, binding, pin),
          completed(CreateEvent(NULL, TRUE, FALSE, NULL)), invoked(false), unexpected(false)
    {
        if (!completed)
            throw std::runtime_error("file completion event creation failed before admission to worker");
    }
    ~State() { CloseHandle(completed); }
};
struct FileInvocationJob::PendingContext
{
    std::shared_ptr<FileInvocationJob> job;
    explicit PendingContext(std::shared_ptr<FileInvocationJob> value) : job(value) {}
};
FileInvocationJob::FileInvocationJob(const MilesFileChannel26::Request &request,
    const MilesFileChannel26::FileServices &services,
    std::shared_ptr<const MilesFileChannel26::Binding> binding, std::shared_ptr<void> pin)
    : state(new State(request, services, binding, pin)) {}
FileInvocationJob::~FileInvocationJob() { delete state; }

std::shared_ptr<FileInvocationJob> FileInvocationJob::enqueueAdmitted(
    EngineFileWorker &worker, const MilesFileChannel26::Request &request,
    const MilesFileChannel26::FileServices &services,
    std::shared_ptr<const MilesFileChannel26::Binding> binding, std::shared_ptr<void> pin)
{
    std::shared_ptr<FileInvocationJob> job(new FileInvocationJob(request, services, binding, pin));
    std::unique_ptr<PendingContext> pending(new PendingContext(job));
    // Callback can run before submit returns; transfer pointer ownership BEFORE
    // submit, and reclaim it only on false (which guarantees no queue publication).
    PendingContext *context = pending.release();
    if (!worker.submit(context, &FileInvocationJob::execute))
    {
        delete context;
        return std::shared_ptr<FileInvocationJob>();
    }
    return job;
}
void FileInvocationJob::execute(void *opaque)
{
    // All typed ownership/destruction stays in this adapter TU, not engine code.
    std::unique_ptr<PendingContext> pending(static_cast<PendingContext *>(opaque));
    State &value = *pending->job->state;
    try { value.invoked = value.invocation.invokeOnAdmittedExecutor(); }
    catch (...) { value.unexpected = true; }
    // Event publication releases the complete immutable result to waiter/poller.
    if (!SetEvent(value.completed))
        std::abort();
    // The queued shared owner is released only after the callback has returned.
}
FileInvocationJob::Status FileInvocationJob::status() const
{
    DWORD const value = WaitForSingleObject(state->completed, 0);
    if (value == WAIT_TIMEOUT)
        return Pending;
    if (value != WAIT_OBJECT_0 || state->unexpected || !state->invoked)
        return DispatchUncertain;
    return AdapterCompleted;
}
bool FileInvocationJob::wait() const
{
    return WaitForSingleObject(state->completed, INFINITE) == WAIT_OBJECT_0;
}
const MilesFileChannel26::Invocation *FileInvocationJob::completedInvocation() const
{
    return status() == AdapterCompleted ? &state->invocation : NULL;
}
}
