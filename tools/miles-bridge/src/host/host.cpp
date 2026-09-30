// Paired x86 host. SDK shutdown precedes explicit callback transport stop.
#include "../bootstrap/common.h"
#include "../admission/coordinator.h"
#include "../backend/backend.h"
#include "../host-runtime/host_file_runtime.h"
#include "../eos/host_eos.h"
#include <limits>
#ifdef _WIN64
#error Original Miles host requires Win32
#endif
namespace {
uint64_t incarnation(const std::string &n){uint64_t value=0;for(unsigned i=0;i<16;++i)value=(value<<4)|static_cast<uint64_t>(n[i]<='9'?n[i]-'0':n[i]-'a'+10);return value?value:1;}
bool nullHandle59(const MilesWire::Handle &h){return !h.kind&&!h.slot&&!h.generation;}
bool emptyCloseCall(const MilesWire::Call &c){
    if(!nullHandle59(c.target)||!nullHandle59(c.resource)||c.callback||c.reserved||c.output_mask||
       c.bytes.offset||c.bytes.length||c.text.offset||c.text.length)return false;
    for(unsigned i=0;i<8;++i)if(c.value[i])return false;
    return true;
}
std::vector<unsigned char> nextCommand(Endpoint &command){
    for(;;){command.pump();healthy(command);std::vector<unsigned char> frame;
        if(command.takeFrame(frame))return frame;
        // No idle command deadline. Only pending events are waited; a fully
        // synchronous partial pump immediately continues when there is no event.
        if(command.readPending())require(WaitForSingleObject(command.readEvent(),INFINITE)==WAIT_OBJECT_0,"idle command wait");
    }
}
void host(int argc,char **argv){
    require(argc==8,"host arguments");
    uint32_t uploadBudgetBytes=0;
    require(MilesImage93::parseBudget(argv[7],uploadBudgetBytes),"explicit upload byte budget");
    const std::string nonceText=argv[4];require(nonceText.size()==32,"nonce extent");
    for(size_t n=0;n<nonceText.size();++n)require((nonceText[n]>='0'&&nonceText[n]<='9')||(nonceText[n]>='a'&&nonceText[n]<='f'),"nonce alphabet");
    char *end=0;unsigned long expected=strtoul(argv[5],&end,10);require(expected && end && !*end,"parent PID");
    HANDLE rawCommand=CreateFileA(argv[2],GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
    HANDLE rawCallback=CreateFileA(argv[3],GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
    require(rawCommand!=INVALID_HANDLE_VALUE && rawCallback!=INVALID_HANDLE_VALUE,"connected pipes");
    ULONG pid=0;
    require(GetNamedPipeServerProcessId(rawCommand,&pid)&&pid==expected,"command parent PID");
    require(GetNamedPipeServerProcessId(rawCallback,&pid)&&pid==expected,"callback parent PID");
    // Heap-retained owners prevent exception unwind from invoking emergency SDK
    // shutdown or Endpoint cleanup after callbacks may have been adopted.
    Endpoint *command=new Endpoint(rawCommand,17);
    Backend *backend=new Backend(argv[6],uploadBudgetBytes);
    MilesHostRuntime50::Runtime *callbacks=0;
    const uint64_t session=incarnation(nonceText);
    MilesCoordinator::Coordinator coordinator(session);
    uint64_t last=0,ordinal=0;bool hello=false;
    for(;;){
        std::vector<unsigned char> frame=nextCommand(*command);
        MilesWire::Header h={};MilesWire::Call c={};
        require(MilesTransport::decodeCall(bytes(frame),h,c),"decode command");
        require(last!=(std::numeric_limits<uint64_t>::max)() && h.request==last+1 &&
            h.kind==MilesWire::Request && h.lane==1 && !h.causal_request,"command correlation");
        require(!backend->uploadFailed(),"terminal image upload cannot resume");
        last=h.request;StartupBridge::OwnedReply out;std::vector<unsigned char> encoded;bool closing=false;
        if(!hello){
            require(!h.lock_lease && h.opcode==MilesWire::Hello && c.bytes.length==32 && !c.text.length &&
                nullHandle59(c.target)&&nullHandle59(c.resource)&&!c.output_mask&&!c.callback&&!c.reserved,"hello shape");
            for(unsigned i=0;i<8;++i)require(!c.value[i],"hello fields");
            require(!memcmp(&frame[c.bytes.offset],nonceText.data(),32),"full nonce");hello=true;
            HANDLE transferred=rawCallback;rawCallback=INVALID_HANDLE_VALUE;
            callbacks=new MilesHostRuntime50::Runtime(transferred,session,1,999,64);
            MilesHostEos::initialize(*callbacks);
        }else{

            std::vector<MilesWire::Handle> resources;bool live=true;
            const MilesWire::Handle handles[]={c.target,c.resource};
            for(unsigned i=0;i<2;++i)if(!nullHandle59(handles[i])){
                void *local=0;live=live&&backend->registry.resolve(handles[i],static_cast<MilesWire::ResourceKind>(handles[i].kind),local);
                if(resources.empty() || handles[i].kind!=resources.front().kind || handles[i].slot!=resources.front().slot || handles[i].generation!=resources.front().generation)resources.push_back(handles[i]);
            }
            if(!live)out.result.transport_status=StartupBridge::InvalidResource;
            else {
                require(ordinal!=(std::numeric_limits<uint64_t>::max)(),"admission exhausted");
                require(coordinator.admitGame(session,ordinal+1,h.lane,h.lock_lease,
                    MilesCoordinator::actionForOpcode(h.opcode),resources)==MilesCoordinator::Ok,"host admission");++ordinal;
                MilesHostContext::Origin origin={session,h.request,h.lane,h.lock_lease,ordinal};
                require(!backend->uploadActive() || h.opcode!=MilesWire::AIL_set_file_callbacks,
                    "file installation during image transaction refused before SDK effect");
                if(h.opcode==MilesWire::SessionClose){
                    if(!emptyCloseCall(c))out.result.transport_status=StartupBridge::InvalidFields;
                    else if(!backend->shutdown || backend->started || backend->driver || backend->streamOpenPending ||
                            backend->uploadActive() || coordinator.leaseDepth() || !backend->registry.empty() || !MilesHostEos::empty())
                        out.result.transport_status=StartupBridge::LifecycleRefused;
                    else {
                        require(callbacks!=0,"callback runtime exists before paired stop");
                        callbacks->stopAfterSdkShutdown();closing=true;
                    }
                }else if(h.opcode==MilesWire::AIL_set_file_callbacks){
                    MilesFileProtocol48::InstallRequest request;
                    require(MilesFileProtocol48::decodeInstall(bytes(frame),h,request),"exact install request");
                    require(callbacks && request.registration==callbacks->registration(),"immutable file registration");
                    encoded.resize(MilesFileProtocol48::InstallReplyBytes);size_t written=0;
                    // installAdmitted owns Scope41; do not nest another scope.
                    require(MilesHostRuntime50::installAdmitted(*callbacks,bytes(frame),h,origin,
                        backend->started,backend->shutdown,encoded.data(),encoded.size(),written),"admitted installation");
                    encoded.resize(written);
                }else{
                    MilesHostContext::Scope scope(origin);
                    require(scope.result()==MilesHostContext::Entered,"admitted SDK origin");
                    // Shutdown cannot discard a held vendor counter or its lease.
                    if(h.opcode==MilesWire::AIL_shutdown && coordinator.leaseDepth())
                        out.result.transport_status=StartupBridge::LifecycleRefused;
                    else out=backend->execute(h,c,frame);
                }
                require(coordinator.completeAdmission(session,ordinal,out.result.transport_status==StartupBridge::Success)==MilesCoordinator::Ok,"observed SDK return");
            }
        }
        backend->observeUploadRefusal(out.result.transport_status);
        if(encoded.empty()){
            MilesWire::Header reply=h;reply.kind=MilesWire::Reply;
            require(MilesTransport::encodeResult(reply,out.result,bytes(out.bytes),bytes(out.text),encoded),"result encoding");
        }
        require(command->send(bytes(encoded)),"command reply");sent(*command);
        if(closing){
            // Command reply is fully written before orderly process exit. Owners
            // and vendor module remain heap-retained; this does not claim unload.
            command->cancel();require(command->drain(30000),"command stop drain");
            return;
        }
    }
}
}
int main(int argc,char **argv){
    try {host(argc,argv);}
    catch(const std::exception &error){
        OutputDebugStringA("Miles host failure: ");
        OutputDebugStringA(error.what());
        OutputDebugStringA("\n");
        MilesHostRuntime50::fatal();
    }
    catch(...){MilesHostRuntime50::fatal();}
    return 0;
}
