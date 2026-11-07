#include "session_file_owner.h"
#include <new>
#include <stdexcept>
#include <type_traits>
namespace MilesFileOwner36 {
using namespace MilesFileChannel26;
using MilesFileExecutor30::FileInvocationJob;
struct SessionFileOwner::FileRecord {
    MilesWire::Handle identity;
    FileState state;
    bool bound,uncertain,closeAcknowledged;
    std::aligned_storage<sizeof(Binding),std::alignment_of<Binding>::value>::type storage;
    FileRecord():identity(),state(Reserved),bound(false),uncertain(false),closeAcknowledged(false) {}
    Binding *binding(){return reinterpret_cast<Binding *>(&storage);}
    ~FileRecord(){if(bound)binding()->~Binding();}
};
struct SessionFileOwner::Operation {
    uint64_t intake;
    MilesCoordinator::CallbackId callback;
    Request request;
    std::shared_ptr<FileRecord> file;
    std::shared_ptr<FileInvocationJob> job;
    std::vector<unsigned char> output;
    size_t written;
    bool ready,uncertain;
    Operation(uint64_t i,const Request &r):intake(i),callback(),request(r),
        output(128+(r.opcode()==MilesWire::FileRead?r.count():0)),written(0),ready(false),uncertain(false) {}
};
SessionFileOwner::SessionFileOwner(MilesCoordinator::Coordinator &c,uint64_t s,uint64_t r,
    std::shared_ptr<FileSessionContext> pin,size_t nf,size_t no)
    :coordinator(c),session(s),registration(r),lastIntake(0),context(pin),
     registry(static_cast<uint32_t>(nf)),files(nf),operations(no) {
    if(!nf || nf>UINT32_MAX || !no || !pin || !pin->engineLifetime || !pin->services.owner ||
       !pin->services.open || !pin->services.close || !pin->services.seek || !pin->services.read)
        throw std::invalid_argument("bounded files/operations and known installed worker/context required");
    if(coordinator.registerSessionFiles(s,r)!=MilesCoordinator::Ok)
        throw std::invalid_argument("session registration refused");
}
SessionFileOwner::~SessionFileOwner() {}
void SessionFileOwner::failure(){coordinator.fail(session);}
void SessionFileOwner::releaseFile(const std::shared_ptr<FileRecord> &f){
    registry.retire(f->identity);
    for(size_t i=0;i<files.size();++i)if(files[i]==f){files[i].reset();break;}
}
SessionFileOwner::Operation *SessionFileOwner::find(uint64_t i) const {
    for(size_t n=0;n<operations.size();++n)if(operations[n] && operations[n]->intake==i)return operations[n].get();
    return 0;
}
SessionFileOwner::Intake SessionFileOwner::receive(const TrustedAssociation &a,MilesTransport::Bytes bytes){
    if(a.session!=session || lastIntake==UINT64_MAX || a.intake!=lastIntake+1)return Rejected;
    if((a.kind==MilesCoordinator::CausalReverseIo && (!a.admission || !a.wire.causal)) ||
       (a.kind==MilesCoordinator::Unsolicited && (a.admission || a.wire.causal)))return Rejected;
    Request request;
    size_t slot=0;for(;slot<operations.size();++slot)if(!operations[slot])break;
    if(slot==operations.size()){failure();return FailedUnanswered;}
    std::shared_ptr<FileRecord> reservation;
    try {
        if(decodeRequest(bytes,a.wire,request)!=Valid)return Rejected;
        std::unique_ptr<Operation> op(new Operation(a.intake,request));
        if(coordinator.state()!=MilesCoordinator::Failed){
            if(request.opcode()==MilesWire::FileOpen){
                size_t f=0;for(;f<files.size();++f)if(!files[f])break;
                if(f==files.size()){failure();return FailedUnanswered;}
                std::shared_ptr<FileRecord> record(new FileRecord);
                if(!registry.insert(MilesWire::File,record.get(),record->identity)){failure();return FailedUnanswered;}
                files[f]=record;op->file=record;reservation=record;
            } else {
                void *raw=0;
                if(!registry.resolve(request.target(),MilesWire::File,raw))return Rejected;
                for(size_t f=0;f<files.size();++f)if(files[f].get()==raw){op->file=files[f];break;}
                if(!op->file || op->file->state!=Published)return Rejected;
            }
        }
        MilesCoordinator::Error admitted=coordinator.admitCallback(session,registration,a.kind,a.admission,op->callback);
        if(admitted!=MilesCoordinator::Ok){
            if(op->file && op->file->state==Reserved)releaseFile(op->file);
            failure();return FailedUnanswered;
        }
        lastIntake=a.intake;
        operations[slot]=std::move(op); // All retained publication storage precedes enqueue.
        reservation.reset();
        Operation &o=*operations[slot];
        if(coordinator.state()==MilesCoordinator::Failed){o.uncertain=true;return FailedUnanswered;}
        std::shared_ptr<const Binding> binding;
        if(request.opcode()!=MilesWire::FileOpen)binding=std::shared_ptr<const Binding>(o.file,o.file->binding());
        if(request.opcode()==MilesWire::FileClose){o.file->state=ClosePending;registry.beginClose(o.file->identity);}
        try {o.job=FileInvocationJob::enqueueAdmitted(context->worker,o.request,context->services,binding,context);}
        catch(...){
            if(request.opcode()==MilesWire::FileOpen){releaseFile(o.file);o.file.reset();}
            o.uncertain=true;failure();return FailedUnanswered;
        }
        if(!o.job){
            if(request.opcode()==MilesWire::FileOpen){releaseFile(o.file);o.file.reset();}
            o.uncertain=true;failure();return FailedUnanswered;
        }
        return Queued;
    } catch(...){if(reservation)releaseFile(reservation);failure();return FailedUnanswered;}
}
void SessionFileOwner::poll(){
    for(size_t i=0;i<operations.size();++i){
        if(!operations[i])continue;
        Operation &o=*operations[i];
        if(o.ready || o.uncertain || !o.job)continue;
        FileInvocationJob::Status status=o.job->status();
        if(status==FileInvocationJob::Pending)continue;
        const Invocation *inv=o.job->completedInvocation();
        if(status!=FileInvocationJob::AdapterCompleted || !inv || inv->completion().state!=Returned){
            o.uncertain=true;if(o.file){o.file->state=Uncertain;o.file->uncertain=true;}failure();continue;
        }
        MilesWire::Handle published={};
        if(o.request.opcode()==MilesWire::FileOpen){
            if(inv->completion().returnBits){
                new (&o.file->storage) Binding(o.file->identity,inv->completion().opened.handle);
                o.file->bound=true;o.file->state=OpenUnpublished;published=o.file->identity;
            }else{releaseFile(o.file);o.file.reset();}
        }else if(o.request.opcode()==MilesWire::FileClose && !o.file->uncertain)o.file->state=ClosedAwaitingAck;
        if(encodeCompletionInto(*inv,published,&o.output[0],o.output.size(),o.written)!=Valid){
            o.uncertain=true;failure();continue;
        }
        o.ready=true;
        if(published.slot)o.file->state=Published;
    }
}
MilesTransport::Bytes SessionFileOwner::reply(uint64_t i) const {
    Operation *o=find(i);return o && o->ready?MilesTransport::Bytes(&o->output[0],o->written):MilesTransport::Bytes();
}
bool SessionFileOwner::acknowledge(uint64_t s,uint64_t i){
    if(s!=session)return false;
    Operation *o=find(i);
    if(!o || !o->ready || coordinator.acknowledge(session,o->callback)!=MilesCoordinator::Ok)return false;
    std::shared_ptr<FileRecord> file=o->file;
    if(file && o->request.opcode()==MilesWire::FileClose)file->closeAcknowledged=true;
    for(size_t n=0;n<operations.size();++n)if(operations[n].get()==o){operations[n].reset();break;}
    if(file && file->closeAcknowledged && !file->uncertain){
        bool held=false;
        for(size_t n=0;n<operations.size();++n)if(operations[n] && operations[n]->file==file)held=true;
        if(!held)releaseFile(file);
    }
    return true;
}
bool SessionFileOwner::fileState(MilesWire::Handle h,FileState &out) const {
    for(size_t i=0;i<files.size();++i)if(files[i] && files[i]->identity.kind==h.kind &&
        files[i]->identity.slot==h.slot && files[i]->identity.generation==h.generation){out=files[i]->state;return true;}
    return false;
}
size_t SessionFileOwner::retainedFiles() const {
    size_t n=0;for(size_t i=0;i<files.size();++i)if(files[i])++n;return n;
}
size_t SessionFileOwner::retainedOperations() const {
    size_t n=0;for(size_t i=0;i<operations.size();++i)if(operations[i])++n;return n;
}
}
