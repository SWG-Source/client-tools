#include "host_file_runtime.h"
#include <cstring>
#include <limits>
namespace MilesHostRuntime50 {
namespace {
__declspec(thread) bool insideCallback=false;
const DWORD ActiveTimeout=30000;
void signal(HANDLE event) { if(!SetEvent(event))fatal(); }
void await(HANDLE event) { if(WaitForSingleObject(event,ActiveTimeout)!=WAIT_OBJECT_0)fatal(); }
MilesTransport::Bytes bytes(const std::vector<unsigned char> &v) {
    return MilesTransport::Bytes(v.empty()?0:&v[0],v.size());
}
// Called exclusively by callback I/O owner. Include only pending I/O events;
// completed manual-reset events must not turn this into a spinning loop.
void pumpWait(MilesPipe::Endpoint &ep,HANDLE event,DWORD timeout) {
    HANDLE handles[3]; DWORD count=0;
    if(event)handles[count++]=event;
    if(ep.readPending())handles[count++]=ep.readEvent();
    if(ep.writePending())handles[count++]=ep.writeEvent();
    if(!count)fatal();
    DWORD result=WaitForMultipleObjects(count,handles,FALSE,timeout);
    if(result==WAIT_FAILED || result==WAIT_ABANDONED_0)fatal();
}
void healthy(MilesPipe::Endpoint &ep) { if(ep.state()!=MilesPipe::Endpoint::Open)fatal(); }
void send(MilesPipe::Endpoint &ep,MilesTransport::Bytes frame) {
    if(!ep.send(frame))fatal();
    const ULONGLONG start=GetTickCount64();
    while(ep.sendBusy()) {
        ep.pump();healthy(ep);
        if(!ep.sendBusy())break;
        if(GetTickCount64()-start>=ActiveTimeout)fatal();
        if(ep.readPending() || ep.writePending())pumpWait(ep,0,1000);
    }
}
}
__declspec(noreturn) void fatal() throw() {
    TerminateProcess(GetCurrentProcess(),0xE050);
    // Fail-fast fallback does not run DLL detach or C++ unwind.
    RaiseFailFastException(0,0,0);
    for(;;) Sleep(INFINITE);
}
Runtime::Runtime(HANDLE pipe,uint64_t session,uint64_t registration,uint64_t backgroundLane,
                 uint32_t liveFiles)
 :pipe_(pipe),thread_(0),ready_(0),work_(0),reply_(0),consumed_(0),done_(0),
  session_(session),registration_(registration),backgroundLane_(backgroundLane),
  lastRequest_(0),files_(liveFiles),ticket_(0) {
    if(!session_ || !registration_ || !backgroundLane_)fatal();
    InitializeCriticalSection(&producer_);
    ready_=CreateEventA(0,FALSE,FALSE,0);work_=CreateEventA(0,TRUE,FALSE,0);
    reply_=CreateEventA(0,FALSE,FALSE,0);consumed_=CreateEventA(0,FALSE,FALSE,0);
    done_=CreateEventA(0,FALSE,FALSE,0);
    if(!ready_ || !work_ || !reply_ || !consumed_ || !done_)fatal();
    thread_=CreateThread(0,0,&Runtime::entry,this,0,0);
    if(!thread_)fatal();
    await(ready_);
}
Runtime::~Runtime(){fatal();}
DWORD WINAPI Runtime::entry(void *context) {
    try {static_cast<Runtime *>(context)->run();}catch(...){fatal();}
    fatal();
}
void Runtime::run() {
    MilesPipe::Endpoint endpoint(pipe_); // construction, all I/O and lifetime here
    signal(ready_);
    for(;;) {
        endpoint.pump();healthy(endpoint);
        std::vector<unsigned char> unexpected;
        if(endpoint.takeFrame(unexpected))fatal();
        if(WaitForSingleObject(work_,0)!=WAIT_OBJECT_0) {
            if(endpoint.readPending() || endpoint.writePending())
                pumpWait(endpoint,work_,INFINITE);
            continue;
        }
        if(!ResetEvent(work_))fatal();
        Ticket *const current=ticket_;if(!current || !current->transaction)fatal();
        send(endpoint,bytes(current->request));
        const ULONGLONG start=GetTickCount64();
        while(!endpoint.takeFrame(current->reply)) {
            endpoint.pump();healthy(endpoint);
            if(GetTickCount64()-start>=ActiveTimeout)fatal();
            // pump may have completed a whole synchronous frame without pending I/O.
            if(endpoint.readPending())pumpWait(endpoint,0,1000);
        }
        signal(reply_);
        // Producer consumes on its own callback stack. It alone touches FileTokens.
        // No Endpoint access on that stack. This wait cannot hold a state mutex.
        await(consumed_);
        send(endpoint,current->transaction->ack());
        if(!current->transaction->observeAckWriteComplete())fatal();
        ticket_=0; // last access to stack ticket precedes the release event
        signal(done_);
    }
}
uint32_t Runtime::invoke(uint32_t opcode,uint32_t token,const char *name,int32_t offset,
                        uint32_t countOrOrigin,void *destination,uint32_t &openedToken) {
    if(insideCallback)fatal(); // before producer mutex: same-thread reentry cannot deadlock
    insideCallback=true;
    MilesHostContext::Origin origin={};
    MilesHostContext::SnapshotResult context=MilesHostContext::snapshot(session_,origin);
    if(context!=MilesHostContext::Ready && context!=MilesHostContext::Unsolicited)fatal();
    EnterCriticalSection(&producer_);
    if(lastRequest_==(std::numeric_limits<uint64_t>::max)())fatal();
    MilesWire::Header h={};h.magic=MilesWire::Magic;h.version=MilesWire::Version;h.kind=MilesWire::ReverseRequest;h.opcode=opcode;
    h.request=++lastRequest_;
    h.causal_request=context==MilesHostContext::Ready?origin.wireRequest:0;
    h.lane=context==MilesHostContext::Ready?origin.lane:backgroundLane_;
    h.lock_lease=context==MilesHostContext::Ready?origin.lease:0;
    MilesWire::Call call={};size_t nameBytes=0;
    if(opcode==MilesWire::FileOpen) {
        if(!name)fatal();
        while(nameBytes<512 && name[nameBytes])++nameBytes;
        if(nameBytes==512)fatal();++nameBytes;
    } else {
        if(!files_.resolve(token,call.target))fatal();
        if(opcode==MilesWire::FileRead)call.value[0]=countOrOrigin;
        if(opcode==MilesWire::FileSeek) {
            std::memcpy(&call.value[0],&offset,sizeof(offset));call.value[1]=countOrOrigin;
        }
    }
    Ticket ticket;
    if(!MilesTransport::encodeCall(h,call,MilesTransport::Bytes(),
            MilesTransport::Bytes(name,nameBytes),ticket.request))fatal();
    MilesFileChannel26::Request request;
    MilesFileChannel26::Association association={h.request,h.causal_request,h.lane,h.lock_lease};
    if(MilesFileChannel26::decodeRequest(bytes(ticket.request),association,request)!=MilesFileChannel26::Valid)fatal();
    MilesHostFiles49::ReplyTransaction transaction(files_,request,registration_,token,
        destination,opcode==MilesWire::FileRead?countOrOrigin:0);
    ticket.transaction=&transaction;
    if(!transaction.beginSend())fatal();
    ticket_=&ticket;signal(work_);await(reply_);
    if(!transaction.consume(bytes(ticket.reply)))fatal();
    signal(consumed_);await(done_);
    uint32_t result=0;
    if(!transaction.result(result,openedToken))fatal();
    LeaveCriticalSection(&producer_);insideCallback=false;
    return result;
}
}
