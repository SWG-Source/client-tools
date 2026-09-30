#ifndef MILES_FILE_INVOCATION_JOB30_H
#define MILES_FILE_INVOCATION_JOB30_H
#include "EngineFileWorker.h"
#include "../file-channel/file_channel.h"
#include <memory>

namespace MilesFileExecutor30
{
// Private modern-STL owner adapter. Never include this from engine/STLport TUs.
class FileInvocationJob
{
public:
    enum Status { Pending, AdapterCompleted, DispatchUncertain };
    // Admission/replay checks and resource/context pins already belong to owner.
    // Null means not queued. Exceptions before queueing stay in the owner's TU.
    static std::shared_ptr<FileInvocationJob> enqueueAdmitted(
        EngineFileWorker &, const MilesFileChannel26::Request &,
        const MilesFileChannel26::FileServices &,
        std::shared_ptr<const MilesFileChannel26::Binding>, std::shared_ptr<void> pin);
    ~FileInvocationJob();
    Status status() const;
    // Reports event signaled, not operation success; inspect status/Completion.
    bool wait() const; // NOT receive/control thread or file worker; no timeout/retry.
    // Keep a shared job owner alive while polling/waiting and using this pointer.
    // Available only after adapter completed; still inspect Completion::state.
    const MilesFileChannel26::Invocation *completedInvocation() const;
private:
    struct State;
    struct PendingContext;
    State *state;
    FileInvocationJob(const MilesFileChannel26::Request &,
        const MilesFileChannel26::FileServices &,
        std::shared_ptr<const MilesFileChannel26::Binding>, std::shared_ptr<void>);
    static void execute(void *);
    FileInvocationJob(const FileInvocationJob &);
    FileInvocationJob &operator=(const FileInvocationJob &);
};
}
#endif
