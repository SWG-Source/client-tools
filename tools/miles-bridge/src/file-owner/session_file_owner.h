#ifndef SESSION_FILE_OWNER36_H
#define SESSION_FILE_OWNER36_H
#include "../admission/coordinator.h"
#include "../file-executor/FileInvocationJob.h"
#include <memory>
#include <vector>
namespace MilesFileOwner36 {
class HostAssociationMapper; // Must establish TLS provenance and session replay acceptance.
class TrustedAssociation {
    friend class HostAssociationMapper;
    friend class SessionFileOwner;
    uint64_t session, intake, admission;
    MilesFileChannel26::Association wire;
    MilesCoordinator::CallbackKind kind;
    TrustedAssociation(uint64_t s,uint64_t i,uint64_t a,
        MilesFileChannel26::Association w,MilesCoordinator::CallbackKind k)
        :session(s),intake(i),admission(a),wire(w),kind(k) {}
};
struct FileSessionContext {
    MilesFileExecutor30::EngineFileWorker &worker;
    std::shared_ptr<void> engineLifetime;
    const MilesFileChannel26::FileServices services;
    FileSessionContext(MilesFileExecutor30::EngineFileWorker &w,std::shared_ptr<void> p,
        const MilesFileChannel26::FileServices &selected)
        :worker(w),engineLifetime(p),services(selected) {}
private:
    FileSessionContext &operator=(const FileSessionContext &);
};
// Control-thread-only. Outer session owner MUST retain this object/context after
// failure and until proven vendor quiescence, all closes, ACKs and worker join.
// Destruction supplies no implicit close, ACK, worker join or quiescence proof.
class SessionFileOwner {
    friend class HostAssociationMapper; // Reads the genuine local session/registration only.
public:
    enum Intake { Queued, Rejected, FailedUnanswered };
    enum FileState { Reserved, OpenUnpublished, Published, ClosePending, ClosedAwaitingAck, Uncertain };
    SessionFileOwner(MilesCoordinator::Coordinator &,uint64_t session,uint64_t registration,
        std::shared_ptr<FileSessionContext>,size_t maxFiles,size_t maxOperations);
    ~SessionFileOwner();
    Intake receive(const TrustedAssociation &,MilesTransport::Bytes);
    void poll();
    // Borrowed stable reply view; owner/operation must remain live through use.
    MilesTransport::Bytes reply(uint64_t intake) const;
    bool acknowledge(uint64_t session,uint64_t intake); // Actual reply-consumption ACK only.
    bool fileState(MilesWire::Handle,FileState &) const;
    size_t retainedOperations() const;
    size_t retainedFiles() const;
private:
    struct FileRecord;
    struct Operation;
    MilesCoordinator::Coordinator &coordinator;
    uint64_t session,registration,lastIntake;
    std::shared_ptr<FileSessionContext> context;
    MilesTransport::ResourceRegistry registry;
    std::vector<std::shared_ptr<FileRecord> > files;
    std::vector<std::unique_ptr<Operation> > operations;
    void failure();
    void releaseFile(const std::shared_ptr<FileRecord> &);
    Operation *find(uint64_t) const;
    SessionFileOwner(const SessionFileOwner &);
    SessionFileOwner &operator=(const SessionFileOwner &);
};
}
#endif
