#include "client_file_runtime.h"
#include "../eos/client_eos.h"
#include "../eos/eos_protocol.h"
#include <process.h>
#include <stdexcept>
#include <new>
namespace MilesClientRuntime53 {
using namespace MilesFileOwner36;
using MilesFileExecutor30::EngineFileWorker;
namespace {
// Keep synchronization storage inside this owner. VS2013 std::mutex uses
// ConcRT's debug allocation with ordinary global delete, which the engine
// replaces; that allocation/deallocation pair crosses incompatible heaps.
class ControlMutex {
public:
    ControlMutex(){InitializeSRWLock(&lock_);}
    void acquire(){AcquireSRWLockExclusive(&lock_);}
    void release(){ReleaseSRWLockExclusive(&lock_);}
private:
    SRWLOCK lock_;
    ControlMutex(const ControlMutex &);
    ControlMutex &operator=(const ControlMutex &);
};
class ControlLock {
public:
    explicit ControlLock(ControlMutex &mutex):mutex_(mutex){mutex_.acquire();}
    ~ControlLock(){mutex_.release();}
private:
    ControlMutex &mutex_;
    ControlLock(const ControlLock &);
    ControlLock &operator=(const ControlLock &);
};
struct Event {
    HANDLE h;
    Event():h(CreateEventA(0,TRUE,FALSE,0)){if(!h)throw std::runtime_error("event creation");}
    ~Event(){CloseHandle(h);}
private:Event(const Event &);Event &operator=(const Event &);
};
struct Ticket {
    enum Kind { Prepare, PrepareEos, RetireEos, RetireAllEos, ArmClose, FinishClose, Publish, Returned } kind;
    Event done;
    bool begun,ok;
    uint64_t wire,lane,registration,lease;
    MilesCoordinator::Action action;
    bool actionApplied;
    ClientMiles::FileOpenCallback open;
    ClientMiles::FileCloseCallback close;
    ClientMiles::FileSeekCallback seek;
    ClientMiles::FileReadCallback read;
    MilesWire::Handle resource;
    ClientMiles::HSAMPLE sample;
    ClientMiles::SampleCallback sampleCallback;
    ClientMiles::HSTREAM stream;
    ClientMiles::StreamCallback streamCallback;
    std::shared_ptr<void> lifetime;
    std::vector<MilesWire::Handle> resources;
    explicit Ticket(Kind k):kind(k),begun(false),ok(false),wire(0),lane(0),registration(0),lease(0),
        action(MilesCoordinator::Ordinary),actionApplied(true),
        open(0),close(0),seek(0),read(0),resource(),sample(0),sampleCallback(0),stream(0),streamCallback(0){}
private:Ticket(const Ticket &);Ticket &operator=(const Ticket &);
};
}
namespace {
bool sameHandle(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
struct EosFunction {
    ClientMiles::SampleCallback sample;
    ClientMiles::StreamCallback stream;
    std::shared_ptr<void> lifetime;
    EosFunction():sample(0),stream(0){}
};
struct EosResource {
    MilesWire::Handle wire;
    ClientMiles::HSAMPLE sample;
    ClientMiles::HSTREAM stream;
};
struct EosPending {
    MilesWire::Header header;
    MilesWire::Eos event;
    std::shared_ptr<MilesEos::ClientJob> job;
    std::vector<unsigned char> completion;
    bool queued;
    EosPending():header(),event(),queued(false){}
};
}
struct Runtime::State {
    HANDLE raw,thread;
    const uint64_t session,background;
    Event wake,ready,terminal;
    bool closeArmed,peerClosed,cleanStopped;
    mutable volatile LONG failureRequested;
    ControlMutex slotMutex,callerMutex;
    std::shared_ptr<Ticket> slot;
    std::shared_ptr<void> engineLifetime;
    // These owners are created/read/mutated only on the control thread. Retained
    // on failure, including open FileRecords with zero outstanding callbacks.
    MilesPipe::Endpoint *endpoint;
    MilesCoordinator::Coordinator *coordinator;
    EngineFileWorker *worker;
    std::shared_ptr<FileSessionContext> context;
    SessionFileOwner *owner;
    HostAssociationMapper *mapper;
    uint64_t lastWire,lastAdmission,activeWire,activeLane,activeLease,lastReverse;
    EosFunction functions[128];
    std::vector<EosResource> eosResources;
    std::unique_ptr<EosPending> eos;
    State(uint64_t s,uint64_t b,std::shared_ptr<void> pin):raw(INVALID_HANDLE_VALUE),thread(0),
        session(s),background(b),closeArmed(false),peerClosed(false),cleanStopped(false),failureRequested(0),engineLifetime(pin),endpoint(0),
        coordinator(0),worker(0),owner(0),mapper(0),lastWire(0),lastAdmission(0),activeWire(0),activeLane(0),activeLease(0),lastReverse(0){}
    void requestFailure(){InterlockedExchange(&failureRequested,1);SetEvent(wake.h);}
    bool isFailed() const {return InterlockedCompareExchange(&failureRequested,0,0)!=0;}
    void terminalFailure(){
        InterlockedExchange(&failureRequested,1);
        if(coordinator)coordinator->fail(session);
        // Cancellation and draining happen on the issuing thread. A timed-out
        // drain retains Endpoint storage; terminal signal is never a close proof.
        if(endpoint){endpoint->cancel();endpoint->drain(0);}
        SetEvent(terminal.h);
    }
    bool post(const std::shared_ptr<Ticket> &ticket){
        ControlLock caller(callerMutex);
        if(isFailed())return false;
        {ControlLock lock(slotMutex);if(slot){requestFailure();return false;}slot=ticket;}
        if(!SetEvent(wake.h)){requestFailure();return false;}
        HANDLE events[]={ticket->done.h,terminal.h};
        DWORD result=WaitForMultipleObjects(2,events,FALSE,30000);
        if(result!=WAIT_OBJECT_0){requestFailure();return false;}
        return ticket->ok && !isFailed();
    }
    void complete(const std::shared_ptr<Ticket> &ticket){
        {ControlLock lock(slotMutex);slot.reset();}
        ticket->ok=true;
        if(!SetEvent(ticket->done.h))requestFailure();
    }
    void ensureWorker(){
        if(worker)return;
        worker=EngineFileWorker::create();
        if(!worker || worker->start()!=EngineFileWorker::Started)
            throw std::runtime_error("engine worker start");
    }
    void prepareEos(const Ticket &t){
        const bool sample=t.resource.kind==MilesWire::OwnedSample && t.sample && !t.stream && !t.streamCallback;
        const bool stream=t.resource.kind==MilesWire::Stream && t.stream && !t.sample && !t.sampleCallback;
        if((!sample && !stream) || !t.resource.slot || !t.resource.generation || !t.lifetime ||
           (sample && (t.registration>64 || (!!t.registration != !!t.sampleCallback))) ||
           (stream && (t.registration && (t.registration<65 || t.registration>128))) ||
           (stream && (!!t.registration != !!t.streamCallback)))
            throw std::runtime_error("typed EOS preparation");
        if(t.registration){
            EosFunction &function=functions[t.registration-1];
            if((function.sample || function.stream) &&
               (function.sample!=t.sampleCallback || function.stream!=t.streamCallback))
                throw std::runtime_error("EOS function identity reused");
            function.sample=t.sampleCallback;function.stream=t.streamCallback;function.lifetime=t.lifetime;
        }
        bool found=false;
        for(size_t i=0;i<eosResources.size();++i)if(sameHandle(eosResources[i].wire,t.resource)){
            if(eosResources[i].sample!=t.sample || eosResources[i].stream!=t.stream)
                throw std::runtime_error("EOS proxy identity changed");
            found=true;
        }
        if(!found){EosResource resource={t.resource,t.sample,t.stream};eosResources.push_back(resource);}
        ensureWorker();
    }
    void receiveEos(const std::vector<unsigned char> &frame){
        if(eos || !worker)throw std::runtime_error("overlapping or unprepared EOS");
        std::unique_ptr<EosPending> pending(new EosPending);
        if(!MilesTransport::decodeEos(MilesTransport::Bytes(frame.data(),frame.size()),pending->header,pending->event) ||
           !MilesEos::validEvent(pending->header,pending->event))throw std::runtime_error("EOS envelope");
        const MilesWire::Header &h=pending->header;
        if(h.causal_request ? (!activeWire || h.causal_request!=activeWire || h.lane!=activeLane || h.lock_lease!=activeLease)
                            : (h.lane!=background || h.lock_lease))throw std::runtime_error("EOS command association");
        const MilesWire::Eos &event=pending->event;
        const EosFunction &function=functions[event.registration-1];
        const EosResource *resource=0;
        for(size_t i=0;i<eosResources.size();++i)if(sameHandle(eosResources[i].wire,event.resource))resource=&eosResources[i];
        if(!resource || !function.lifetime ||
           (h.opcode==MilesWire::EndOfSample ? (!function.sample || !resource->sample) : (!function.stream || !resource->stream)))
            throw std::runtime_error("EOS callback or live proxy unknown");
        if(!MilesEos::encodeCompletion(h,event,pending->completion))throw std::runtime_error("EOS completion encoding");
        // Capture immutable typed function/proxy before effect. Session retires
        // proxy storage only after native release and this consumption ACK.
        eos=std::move(pending);
        eos->job=MilesEos::ClientJob::submit(*worker,resource->sample,function.sample,
            resource->stream,function.stream,function.lifetime);
    }
    bool quiescent() const {
        return !activeWire && !coordinator->activeAdmission() && !coordinator->leaseDepth() &&
            !eos && eosResources.empty() && (!mapper || !mapper->retainedReverse()) &&
            (!owner || (!owner->retainedOperations() && !owner->retainedFiles()));
    }
    void observation(const std::shared_ptr<Ticket> &t){
        if(t->kind==Ticket::ArmClose){
            if(closeArmed || !quiescent() || endpoint->sendBusy() || endpoint->receiveBuffered())
                throw std::runtime_error("close requires settled shutdown and no live files");
            closeArmed=true;complete(t);return;
        }
        if(t->kind==Ticket::FinishClose){
            if(!closeArmed || !quiescent())throw std::runtime_error("close settlement missing");
            if(!peerClosed)return;
            // Drain only after SessionClose admission settled, so our own final
            // command is not rejected as new game work by the coordinator.
            if(coordinator->beginDrain(session)!=MilesCoordinator::Ok)
                throw std::runtime_error("coordinator close drain");
            if(!endpoint->drain(5000))throw std::runtime_error("callback close drain");
            if(worker && !worker->drainAndJoin())throw std::runtime_error("engine worker join");
            if(isFailed())throw std::runtime_error("close deadline expired");
            // All these roots were created on this control thread. The worker
            // has joined through actual engine TLS removal before their release.
            delete mapper;mapper=0;delete owner;owner=0;context.reset();
            if(worker){EngineFileWorker::destroy(worker);worker=0;}
            for(size_t i=0;i<128;++i)functions[i].lifetime.reset();
            engineLifetime.reset();delete coordinator;coordinator=0;
            delete endpoint;endpoint=0;cleanStopped=true;complete(t);return;
        }
        if(closeArmed && t->kind!=Ticket::Publish && t->kind!=Ticket::Returned)
            throw std::runtime_error("observation after close intent");
        // Native completion and bridge consumption are distinct. Forward return,
        // new admission and retirement wait for the EOS consumption ACK too.
        if(eos)return;
        if(t->begun){
            if(t->kind!=Ticket::Returned || !mapper)throw std::runtime_error("pending observation");
            if(mapper->commandState()==HostAssociationMapper::Settled){
                if(!mapper->consumeCommand(t->wire))throw std::runtime_error("consume");
                activeWire=0;t->lease=coordinator->lease();complete(t);
            }
            return;
        }
        t->begun=true;
        if(t->kind==Ticket::PrepareEos){
            if(activeWire)throw std::runtime_error("EOS preparation during command");
            prepareEos(*t);complete(t);
        }else if(t->kind==Ticket::RetireEos || t->kind==Ticket::RetireAllEos){
            if(activeWire)throw std::runtime_error("EOS retirement during command");
            for(size_t i=0;i<eosResources.size();)
                if(t->kind==Ticket::RetireAllEos || sameHandle(eosResources[i].wire,t->resource))
                    eosResources.erase(eosResources.begin()+i);
                else ++i;
            complete(t);
        }else if(t->kind==Ticket::Prepare){
            if(owner || activeWire || coordinator->activeAdmission() || !t->registration)
                throw std::runtime_error("prepare transition");
            MilesFileChannel26::FileServices selected=MilesSelectedFileServices44::retain(
                t->open,t->close,t->seek,t->read,t->lifetime);
            ensureWorker();
            context.reset(new FileSessionContext(*worker,engineLifetime,selected));
            owner=new SessionFileOwner(*coordinator,session,t->registration,context,64,64);
            mapper=new HostAssociationMapper(session,background,*coordinator,*owner,64);
            complete(t); // prepared before install send; not SDK Installed
        }else if(t->kind==Ticket::Publish){
            if(activeWire || !t->wire || t->wire<=lastWire || !t->lane || t->lane==background || lastAdmission==UINT64_MAX)
                throw std::runtime_error("command identity");
            const uint64_t admission=lastAdmission+1;
            MilesCoordinator::Error e=mapper
                ?mapper->publishCommand(t->wire,admission,t->lane,t->lease,t->action,t->resources)
                :coordinator->admitGame(session,admission,t->lane,t->lease,t->action,t->resources);
            if(e!=MilesCoordinator::Ok)throw std::runtime_error("admit command");
            lastAdmission=admission;lastWire=t->wire;activeWire=t->wire;activeLane=t->lane;activeLease=t->lease;complete(t);
        }else{
            if(!activeWire || t->wire!=activeWire)throw std::runtime_error("return identity");
            if(mapper){
                if(!mapper->observeForwardReturn(session,t->wire,t->actionApplied))throw std::runtime_error("forward return");
                if(mapper->commandState()!=HostAssociationMapper::Settled)return;
                if(!mapper->consumeCommand(t->wire))throw std::runtime_error("consume");
            }else if(coordinator->completeAdmission(session,lastAdmission,t->actionApplied)!=MilesCoordinator::Ok)
                throw std::runtime_error("preparation-free return");
            activeWire=0;t->lease=coordinator->lease();complete(t);
        }
    }
    void run(){
        try {
            // Allocation occurs before transfer to constructor, which consumes the
            // handle even when event creation throws. Retain raw on allocation fail.
            void *storage=::operator new(sizeof(MilesPipe::Endpoint));
            HANDLE transferred=raw;raw=INVALID_HANDLE_VALUE;
            try {endpoint=new (storage) MilesPipe::Endpoint(transferred);}
            catch(...){::operator delete(storage);throw;}
            coordinator=new MilesCoordinator::Coordinator(session);
            if(!SetEvent(ready.h))throw std::runtime_error("ready signal");
            for(;;){
                if(isFailed())break;
                ResetEvent(wake.h);
                std::shared_ptr<Ticket> current;
                {ControlLock lock(slotMutex);current=slot;}
                if(current)observation(current);
                if(cleanStopped)return;
                if(isFailed())break;
                endpoint->pump();
                if(closeArmed && endpoint->receiveBuffered())
                    throw std::runtime_error("callback bytes after close intent");
                if(endpoint->state()!=MilesPipe::Endpoint::Open){
                    if(!closeArmed || endpoint->failure()!=MilesPipe::Endpoint::PeerClosed)
                        throw std::runtime_error("callback endpoint");
                    peerClosed=true;
                }
                std::vector<unsigned char> frame;
                if(endpoint->takeFrame(frame)){
                    if(closeArmed)throw std::runtime_error("callback after close intent");
                    MilesWire::Header header={};MilesWire::Call fields={};MilesWire::Eos event={};
                    const MilesTransport::Bytes data(frame.data(),frame.size());
                    const bool eventFrame=MilesTransport::decodeEos(data,header,event);
                    if(!eventFrame && !MilesTransport::decodeCall(data,header,fields))throw std::runtime_error("reverse decode");
                    // File consumption ACKs retain ReverseRequest kind but reuse
                    // their original ID; they are not new reverse operations.
                    if(eventFrame || (header.kind==MilesWire::ReverseRequest &&
                       header.opcode!=MilesWire::FileConsumptionAck)){
                        if(lastReverse==UINT64_MAX || header.request!=lastReverse+1)
                            throw std::runtime_error("shared reverse sequence");
                        lastReverse=header.request;
                    }
                    if(eventFrame)receiveEos(frame);
                    else if(header.opcode==MilesWire::CallbackAck){
                        if(!eos || !eos->queued || !MilesEos::validateConsumption(data,eos->header,eos->event))
                            throw std::runtime_error("EOS consumption ACK");
                        eos.reset();
                    }else{
                        if(!mapper || eos)throw std::runtime_error("file callback without prepared idle owner");
                        HostAssociationMapper::ControlResult r=mapper->receiveControl(session,data);
                        if(r!=HostAssociationMapper::FileQueued && r!=HostAssociationMapper::AckConsumed)
                            throw std::runtime_error("callback control rejected");
                    }
                }
                if(eos && !eos->queued){
                    const MilesEos::ClientJob::Status status=eos->job->status();
                    if(status==MilesEos::ClientJob::Failed)throw std::runtime_error("EOS callback failed");
                    if(status==MilesEos::ClientJob::Completed && !endpoint->sendBusy()){
                        if(!endpoint->send(MilesTransport::Bytes(eos->completion.data(),eos->completion.size())))
                            throw std::runtime_error("EOS completion send");
                        eos->queued=true;
                    }
                }
                if(mapper){
                    mapper->poll();
                    if(coordinator->state()==MilesCoordinator::Failed)throw std::runtime_error("file work uncertain");
                    uint64_t wire=0;
                    if(!endpoint->sendBusy() && mapper->nextUnqueuedReply(wire)){
                        if(!endpoint->send(mapper->reply(wire)) || !mapper->markReplyQueued(wire))
                            throw std::runtime_error("reply queue failure");
                    }
                }
                // Bounded idle tick polls jobs; never waits on their worker. Endpoint
                // pump is nonblocking. No idle command watchdog or second ACK table.
                DWORD w=WaitForSingleObject(wake.h,2);
                if(w!=WAIT_OBJECT_0 && w!=WAIT_TIMEOUT)throw std::runtime_error("control wait");
            }
        }catch(...){requestFailure();}
        terminalFailure();
        // Retain control thread/Endpoint issuing lifetime and all engine ownership.
        // Only explicit paired close returns from run().
        for(;;){if(endpoint)endpoint->drain(0);Sleep(20);}
    }
    static unsigned __stdcall entry(void *p){static_cast<State *>(p)->run();return 0;}
private:State(const State &);State &operator=(const State &);
};
Runtime::Runtime(State *s):state(s){}
Runtime *Runtime::launch(HANDLE &raw,uint64_t session,uint64_t background,std::shared_ptr<void> pin){
    if(!raw || raw==INVALID_HANDLE_VALUE || !session || !background || !pin)throw std::invalid_argument("runtime binding");
    std::unique_ptr<State> s(new State(session,background,pin));
    Runtime *result=new Runtime(s.get());
    s->raw=raw;
    uintptr_t thread=_beginthreadex(0,0,&State::entry,s.get(),0,0);
    if(!thread){delete result;throw std::runtime_error("control thread creation");}
    s->thread=reinterpret_cast<HANDLE>(thread);raw=INVALID_HANDLE_VALUE;s.release();return result;
}
Runtime::~Runtime(){} // launch failure, or explicit successful finishClose after join
bool Runtime::awaitReady(){HANDLE events[]={state->ready.h,state->terminal.h};DWORD r=WaitForMultipleObjects(2,events,FALSE,30000);if(r!=WAIT_OBJECT_0){state->requestFailure();return false;}return !failed();}
bool Runtime::prepare(uint64_t r,ClientMiles::FileOpenCallback o,ClientMiles::FileCloseCallback c,ClientMiles::FileSeekCallback s,ClientMiles::FileReadCallback rd,std::shared_ptr<void> pin){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::Prepare));t->registration=r;t->open=o;t->close=c;t->seek=s;t->read=rd;t->lifetime=pin;return state->post(t);}catch(...){fail();return false;}
}
bool Runtime::prepareEos(const MilesWire::Handle &resource,uint64_t id,ClientMiles::HSAMPLE sample,
    ClientMiles::SampleCallback sc,ClientMiles::HSTREAM stream,ClientMiles::StreamCallback tc,std::shared_ptr<void> pin){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::PrepareEos));t->resource=resource;t->registration=id;
        t->sample=sample;t->sampleCallback=sc;t->stream=stream;t->streamCallback=tc;t->lifetime=pin;
        return state->post(t);}catch(...){fail();return false;}
}
bool Runtime::retireEos(const MilesWire::Handle &resource){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::RetireEos));t->resource=resource;return state->post(t);}
    catch(...){fail();return false;}
}
bool Runtime::retireAllEos(){
    try{return state->post(std::shared_ptr<Ticket>(new Ticket(Ticket::RetireAllEos)));}catch(...){fail();return false;}
}
bool Runtime::publish(uint64_t wire,uint64_t lane,const std::vector<MilesWire::Handle> &resources,
    MilesCoordinator::Action action,uint64_t lease){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::Publish));t->wire=wire;t->lane=lane;t->resources=resources;t->action=action;t->lease=lease;return state->post(t);}catch(...){fail();return false;}
}
bool Runtime::returned(uint64_t wire,bool actionApplied,uint64_t *settledLease){
    try {
        std::shared_ptr<Ticket> t(new Ticket(Ticket::Returned));
        t->wire=wire;t->actionApplied=actionApplied;
        if(!state->post(t))return false;
        if(settledLease)*settledLease=t->lease;
        return true;
    }catch(...){fail();return false;}
}
bool Runtime::armClose(){
    try{return state->post(std::shared_ptr<Ticket>(new Ticket(Ticket::ArmClose)));}
    catch(...){fail();return false;}
}
bool Runtime::finishClose(){
    try{
        if(!state->post(std::shared_ptr<Ticket>(new Ticket(Ticket::FinishClose))))return false;
        if(WaitForSingleObject(state->thread,5000)!=WAIT_OBJECT_0){fail();return false;}
        CloseHandle(state->thread);delete state;state=0;delete this;return true;
    }catch(...){fail();return false;}
}
void Runtime::fail(){state->requestFailure();}
bool Runtime::failed() const{return state->isFailed();}
HANDLE Runtime::failureEvent() const{return state->terminal.h;}
}
