#include "LiveChannel.h"
#include "../../bootstrap/common.h"
#include "../../file-protocol/file_protocol.h"
#include <limits>
#include "../../image/upload_policy.h"
#include "../../failure/failure_boundary.h"

namespace {
uint64_t incarnation(const std::string &text){uint64_t value=0;for(unsigned i=0;i<16;++i)value=(value<<4)|static_cast<uint64_t>(text[i]<='9'?text[i]-'0':text[i]-'a'+10);return value?value:1;}
class LiveChannel : public ClientMilesPipe::Channel {
  public:
    LiveChannel(const char *hostExecutable,const char *originalDll,std::shared_ptr<void> enginePin,
        MilesClientRuntime53::Runtime *&adopted, uint32_t uploadBudgetBytes) : runtime_(0),next_(0),lease_(0) {
        try {
        require(MilesImage93::validBudget(uploadBudgetBytes), "explicit upload byte budget");
        char budget[16];sprintf_s(budget,"%lu",static_cast<unsigned long>(uploadBudgetBytes));
        std::string random = nonce();
        std::string base = "\\\\.\\pipe\\swg-facade24-" + random;
        std::string commandName = base + "-cmd", callbackName = base + "-cb";
        PSECURITY_DESCRIPTOR descriptor = userDescriptor();
        SECURITY_ATTRIBUTES attributes = {sizeof attributes, descriptor, FALSE};
        ServerPipe command(commandName, attributes), callback(callbackName, attributes);
        LocalFree(descriptor);
        char pid[32];
        sprintf_s(pid, "%lu", GetCurrentProcessId());
        std::string args = "\"" + std::string(hostExecutable) + "\" --host " + commandName + " " +
                           callbackName + " " + random + " " + pid + " \"" + originalDll + "\" " + budget;
        std::vector<char> line(args.begin(), args.end());
        line.push_back(0);
        child_.create(hostExecutable, &line[0]);
        child_.assignAndResume(child_.job);
        command.connected();
        callback.connected();
        ULONG actual = 0;
        require(GetNamedPipeClientProcessId(command.pipe, &actual) &&
                    actual == child_.info.dwProcessId,
                "child command PID binding");
        require(GetNamedPipeClientProcessId(callback.pipe, &actual) &&
                    actual == child_.info.dwProcessId,
                "child callback PID binding");
        command_.reset(new Endpoint(command.take()));
        HANDLE raw=callback.take();
        try {runtime_=MilesClientRuntime53::Runtime::launch(raw,incarnation(random),999,enginePin);}
        catch(...){if(raw!=INVALID_HANDLE_VALUE)CloseHandle(raw);throw;}
        adopted=runtime_; // outer catch must retain it even if Hello subsequently fails
        require(runtime_->awaitReady(),"callback control owner ready");
        StartupBridge::OwnedReply hello =
            exchange(MilesWire::Hello, MilesWire::Call(),
                     MilesTransport::Bytes(random.data(), random.size()), MilesTransport::Bytes(),std::vector<MilesWire::Handle>(),0);
        require(hello.result.transport_status == StartupBridge::Success, "Hello accepted");
        }catch(const std::exception &error){
            if(runtime_){runtime_->fail();ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());}
            throw;
        }catch(...){if(runtime_){runtime_->fail();ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,"Miles connection failed after callback ownership transfer");}throw;}
    }

    StartupBridge::OwnedReply call(uint32_t opcode, const MilesWire::Call &fields,
                                   MilesTransport::Bytes payload, MilesTransport::Bytes text,const std::vector<MilesWire::Handle> &resources,
        const ClientMilesPipe::Session &replyOwner) override {
        // Known validated refusals reach the facade; only fixture probes compare
        // an expected status. Do not inherit Client.call's default-success throw.
        return exchange(opcode, fields, payload, text,resources,&replyOwner);
    }

    void finish() override {
      try {
        require(runtime_->armClose(),"callback close intent");
        StartupBridge::OwnedReply reply=exchange(MilesWire::SessionClose,MilesWire::Call(),
            MilesTransport::Bytes(),MilesTransport::Bytes(),std::vector<MilesWire::Handle>(),0);
        require(reply.result.transport_status==StartupBridge::Success,"SessionClose accepted");
        require(WaitForSingleObject(child_.info.hProcess,30000)==WAIT_OBJECT_0,"host normal exit deadline");
        DWORD code=STILL_ACTIVE;
        require(GetExitCodeProcess(child_.info.hProcess,&code) && code==0,"host normal exit status");
        // Command I/O is cancelled and drained by its issuing thread. Keep the
        // complete root on any failure; neither an ACK nor PeerClosed is enough.
        const ULONGLONG begin=GetTickCount64();
        for(;;){
            command_->pump();
            require(!command_->receiveBuffered(),"extra command bytes after SessionClose reply");
            if(command_->state()==Endpoint::Faulted && command_->failure()==Endpoint::PeerClosed)break;
            healthy(*command_);
            require(GetTickCount64()-begin<5000,"command peer close deadline");
            Sleep(2);
        }
        require(command_->drain(5000),"command close drain");
        require(runtime_->finishClose(),"callback owner and engine worker join");
        runtime_=0;command_.reset();child_.close();
      }catch(...){if(runtime_)runtime_->fail();throw;}
    }

  private:
    ChildProcess child_;
    std::unique_ptr<Endpoint> command_;
    MilesClientRuntime53::Runtime *runtime_;
    uint64_t next_,lease_;

    StartupBridge::OwnedReply exchange(uint32_t opcode,const MilesWire::Call &fields,
        MilesTransport::Bytes payload,MilesTransport::Bytes text,const std::vector<MilesWire::Handle> &resources,
        const ClientMilesPipe::Session *replyOwner){
      try {
        require(next_!=(std::numeric_limits<uint64_t>::max)(),"command ID exhausted");
        MilesWire::Header header={};header.magic=MilesWire::Magic;header.version=MilesWire::Version;
        header.kind=MilesWire::Request;header.opcode=opcode;header.request=++next_;header.lane=1;header.lock_lease=lease_;
        const MilesCoordinator::Action action=MilesCoordinator::actionForOpcode(opcode);
        std::vector<unsigned char> frame;
        require(MilesTransport::encodeCall(header,fields,payload,text,frame),"encode call");
        if(opcode!=MilesWire::Hello)require(runtime_->publish(header.request,header.lane,resources,action,lease_),"publish before send");
        require(command_->send(bytes(frame)),"command send");
        ULONGLONG begin=GetTickCount64();
        for(;;){
            require(!runtime_->failed(),"callback owner failed");
            if(opcode!=MilesWire::SessionClose)
                require(WaitForSingleObject(child_.info.hProcess,0)==WAIT_TIMEOUT,"host died during command");
            command_->pump();
            if(opcode==MilesWire::SessionClose && command_->takeFrame(frame,true))break;
            healthy(*command_);
            if(command_->takeFrame(frame))break;
            require(GetTickCount64()-begin<30000,"active command deadline");
            HANDLE events[3];DWORD count=0;events[count++]=runtime_->failureEvent();
            if(command_->readPending())events[count++]=command_->readEvent();
            if(command_->writePending())events[count++]=command_->writeEvent();
            DWORD result=WaitForMultipleObjects(count,events,FALSE,2);
            require(result!=WAIT_FAILED,"command I/O wait");
        }
        StartupBridge::OwnedReply out;
        if(opcode==MilesWire::AIL_set_file_callbacks){
            MilesFileProtocol48::InstallRequest expected;expected.header=header;expected.registration=fields.callback;
            MilesFileProtocol48::InstallStatus status;
            require(MilesFileProtocol48::decodeInstallReply(bytes(frame),expected,status),"exact install reply");
            MilesWire::Header decoded={};require(MilesTransport::decodeResult(bytes(frame),decoded,out.result),"install result projection");
        }else require(StartupBridge::decodeReply(bytes(frame),header,out),"typed reply/context");
        if(opcode!=MilesWire::Hello){
            if(opcode==MilesWire::SessionClose){
                MilesWire::Header expected=header;expected.kind=MilesWire::Reply;
                MilesWire::Result result={};std::vector<unsigned char> exact;
                require(MilesTransport::encodeResult(expected,result,MilesTransport::Bytes(),MilesTransport::Bytes(),exact) &&
                    frame==exact,"exact zero SessionClose reply");
            }else require(replyOwner && replyOwner->validateReply(opcode,fields,out),"request and owner reply validation before settlement");
            require(runtime_->returned(header.request,out.result.transport_status==StartupBridge::Success,&lease_),"forward and callback ACK join");
        }
        return out;
      }catch(...){runtime_->fail();throw;}
    }
    LiveChannel(const LiveChannel &);
    LiveChannel &operator=(const LiveChannel &);
};
} // namespace

namespace ClientMilesPipe {
Session *connectSession(const char *host,const char *dll,std::shared_ptr<void> enginePin,std::shared_ptr<void> callbackPin,uint32_t uploadBudgetBytes){
    MilesClientRuntime53::Runtime *adopted=0;
    try {
        Channel *channel=new LiveChannel(host,dll,enginePin,adopted,uploadBudgetBytes);
        return new Session(channel,*adopted,callbackPin,uploadBudgetBytes);
    }catch(const std::exception &error){
        if(adopted){adopted->fail();ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());}
        throw;
    }catch(...){
        if(adopted){adopted->fail();ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,"Miles session composition failed after ownership transfer");} // retained roots
        throw;
    }
}
}
