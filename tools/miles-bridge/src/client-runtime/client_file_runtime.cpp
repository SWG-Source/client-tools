#include "client_file_runtime.h"
#include <process.h>
#include <mutex>
#include <stdexcept>
#include <new>
namespace MilesClientRuntime53 {
using namespace MilesFileOwner36;
using MilesFileExecutor30::EngineFileWorker;
namespace {
struct Event {
    HANDLE h;
    Event():h(CreateEventA(0,TRUE,FALSE,0)){if(!h)throw std::runtime_error("event creation");}
    ~Event(){CloseHandle(h);}
private:Event(const Event &);Event &operator=(const Event &);
};
struct Ticket {
    enum Kind { Prepare, Publish, Returned } kind;
    Event done;
    bool begun,ok;
    uint64_t wire,lane,registration;
    ClientMiles::FileOpenCallback open;
    ClientMiles::FileCloseCallback close;
    ClientMiles::FileSeekCallback seek;
    ClientMiles::FileReadCallback read;
    std::shared_ptr<void> lifetime;
    std::vector<MilesWire::Handle> resources;
    explicit Ticket(Kind k):kind(k),begun(false),ok(false),wire(0),lane(0),registration(0),
        open(0),close(0),seek(0),read(0){}
private:Ticket(const Ticket &);Ticket &operator=(const Ticket &);
};
}
struct Runtime::State {
    HANDLE raw,thread;
    const uint64_t session,background;
    Event wake,ready,terminal;
    mutable volatile LONG failureRequested;
    std::mutex slotMutex,callerMutex;
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
    uint64_t lastWire,lastAdmission,activeWire;
    State(uint64_t s,uint64_t b,std::shared_ptr<void> pin):raw(INVALID_HANDLE_VALUE),thread(0),
        session(s),background(b),failureRequested(0),engineLifetime(pin),endpoint(0),
        coordinator(0),worker(0),owner(0),mapper(0),lastWire(0),lastAdmission(0),activeWire(0){}
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
        std::lock_guard<std::mutex> caller(callerMutex);
        if(isFailed())return false;
        {std::lock_guard<std::mutex> lock(slotMutex);if(slot){requestFailure();return false;}slot=ticket;}
        if(!SetEvent(wake.h)){requestFailure();return false;}
        HANDLE events[]={ticket->done.h,terminal.h};
        DWORD result=WaitForMultipleObjects(2,events,FALSE,INFINITE);
        if(result!=WAIT_OBJECT_0){requestFailure();return false;}
        return ticket->ok && !isFailed();
    }
    void complete(const std::shared_ptr<Ticket> &ticket){
        {std::lock_guard<std::mutex> lock(slotMutex);slot.reset();}
        ticket->ok=true;
        if(!SetEvent(ticket->done.h))requestFailure();
    }
    void observation(const std::shared_ptr<Ticket> &t){
        if(t->begun){
            if(t->kind!=Ticket::Returned || !mapper)throw std::runtime_error("pending observation");
            if(mapper->commandState()==HostAssociationMapper::Settled){
                if(!mapper->consumeCommand(t->wire))throw std::runtime_error("consume");
                activeWire=0;complete(t);
            }
            return;
        }
        t->begun=true;
        if(t->kind==Ticket::Prepare){
            if(owner || activeWire || coordinator->activeAdmission() || !t->registration)
                throw std::runtime_error("prepare transition");
            MilesFileChannel26::FileServices selected=MilesSelectedFileServices44::retain(
                t->open,t->close,t->seek,t->read,t->lifetime);
            worker=EngineFileWorker::create();
            if(!worker || worker->start()!=EngineFileWorker::Started)throw std::runtime_error("engine worker start");
            context.reset(new FileSessionContext(*worker,engineLifetime,selected));
            owner=new SessionFileOwner(*coordinator,session,t->registration,context,64,64);
            mapper=new HostAssociationMapper(session,background,*coordinator,*owner,64);
            complete(t); // prepared before install send; not SDK Installed
        }else if(t->kind==Ticket::Publish){
            if(activeWire || !t->wire || t->wire<=lastWire || !t->lane || t->lane==background || lastAdmission==UINT64_MAX)
                throw std::runtime_error("command identity");
            const uint64_t admission=lastAdmission+1;
            MilesCoordinator::Error e=mapper
                ?mapper->publishCommand(t->wire,admission,t->lane,0,MilesCoordinator::Ordinary,t->resources)
                :coordinator->admitGame(session,admission,t->lane,0,MilesCoordinator::Ordinary,t->resources);
            if(e!=MilesCoordinator::Ok)throw std::runtime_error("admit command");
            lastAdmission=admission;lastWire=t->wire;activeWire=t->wire;complete(t);
        }else{
            if(!activeWire || t->wire!=activeWire)throw std::runtime_error("return identity");
            if(mapper){
                if(!mapper->observeForwardReturn(session,t->wire))throw std::runtime_error("forward return");
                if(mapper->commandState()!=HostAssociationMapper::Settled)return;
                if(!mapper->consumeCommand(t->wire))throw std::runtime_error("consume");
            }else if(coordinator->completeAdmission(session,lastAdmission)!=MilesCoordinator::Ok)
                throw std::runtime_error("preparation-free return");
            activeWire=0;complete(t);
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
                {std::lock_guard<std::mutex> lock(slotMutex);current=slot;}
                if(current)observation(current);
                if(isFailed())break;
                endpoint->pump();
                if(endpoint->state()!=MilesPipe::Endpoint::Open)throw std::runtime_error("callback endpoint");
                std::vector<unsigned char> frame;
                if(endpoint->takeFrame(frame)){
                    if(!mapper)throw std::runtime_error("reverse before table preparation");
                    HostAssociationMapper::ControlResult r=mapper->receiveControl(session,MilesTransport::Bytes(frame.data(),frame.size()));
                    if(r!=HostAssociationMapper::FileQueued && r!=HostAssociationMapper::AckConsumed)
                        throw std::runtime_error("callback control rejected");
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
        // No normal stop is implemented until quiescence/close proof is integrated.
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
Runtime::~Runtime(){} // only launch failure before thread ownership; State stays local
bool Runtime::awaitReady(){HANDLE events[]={state->ready.h,state->terminal.h};DWORD r=WaitForMultipleObjects(2,events,FALSE,INFINITE);if(r!=WAIT_OBJECT_0){state->requestFailure();return false;}return !failed();}
bool Runtime::prepare(uint64_t r,ClientMiles::FileOpenCallback o,ClientMiles::FileCloseCallback c,ClientMiles::FileSeekCallback s,ClientMiles::FileReadCallback rd,std::shared_ptr<void> pin){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::Prepare));t->registration=r;t->open=o;t->close=c;t->seek=s;t->read=rd;t->lifetime=pin;return state->post(t);}catch(...){fail();return false;}
}
bool Runtime::publish(uint64_t wire,uint64_t lane,const std::vector<MilesWire::Handle> &resources){
    try{std::shared_ptr<Ticket> t(new Ticket(Ticket::Publish));t->wire=wire;t->lane=lane;t->resources=resources;return state->post(t);}catch(...){fail();return false;}
}
bool Runtime::returned(uint64_t wire){try{std::shared_ptr<Ticket> t(new Ticket(Ticket::Returned));t->wire=wire;return state->post(t);}catch(...){fail();return false;}}
void Runtime::fail(){state->requestFailure();}
bool Runtime::failed() const{return state->isFailed();}
HANDLE Runtime::failureEvent() const{return state->terminal.h;}
}
