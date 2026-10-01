#include "bink_service.h"
#include "bink_file_io.h"
#include "../bink/bink_protocol.h"
#include <algorithm>
namespace MilesHostBink {
namespace {
void need(bool ok,const char *why) { if(!ok)throw std::runtime_error(why); }
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b) {
    return a.kind==b.kind && a.slot==b.slot && a.generation==b.generation;
}
void metadata(Video &video,MilesWire::Result &r) {
    const Metadata m=video.metadata();
    r.value[0]=m.width;r.value[1]=m.height;r.value[2]=m.frames;r.value[3]=m.frame;
    r.value[4]=m.lastFrame;r.value[5]=m.rate;r.value[6]=m.rateDiv;
}
std::wstring dllPath() {
    wchar_t path[MAX_PATH]={};
    DWORD n=GetModuleFileNameW(0,path,MAX_PATH);
    need(n && n<MAX_PATH,"Bink host executable path");
    std::wstring value(path,n);size_t slash=value.find_last_of(L"\\/");
    need(slash!=std::wstring::npos,"Bink absolute adjacent path");
    return value.substr(0,slash+1)+L"binkw32.dll";
}
}
Service::Service(MilesTransport::ResourceRegistry &registry):registry_(registry),runtime_(0),
    driver_(),attempted_(false),ready_(false),stopped_(false),opening_(false),transfer_(0),transferId_(),offset_(0) {}
bool Service::permitsMilesShutdown() const {
    return !attempted_ || (stopped_ && !transfer_ && !opening_);
}
bool Service::intercept(const MilesWire::Header &h,const MilesWire::Call &c,
    const std::vector<unsigned char> &frame,bool started,bool shutdown,
    MilesHostRuntime50::Runtime *callbacks,bool filesInstalled,StartupBridge::OwnedReply &out) {
    using namespace MilesWire;
    using namespace StartupBridge;
    const bool bink=h.opcode>=MilesWire::BinkInitialize && h.opcode<=MilesWire::BinkShutdown;
    if(transfer_ && h.opcode!=MilesWire::BinkPixelsChunk) {
        out.result.transport_status=LifecycleRefused;return true;
    }
    if(!bink)return false;
    out.result.transport_status=InvalidFields;
    if(!MilesBinkProtocol::validCall(h,c,MilesTransport::Bytes(frame.data(),frame.size())))return true;
    out.result.transport_status=LifecycleRefused;
    if(!started || shutdown || !callbacks || !filesInstalled || opening_)return true;
    void *local=0;
    if(!MilesBinkProtocol::resolveTarget(registry_,h,c,local)) {
        out.result.transport_status=InvalidResource;return true;
    }
    if(h.opcode==MilesWire::BinkInitialize) {
        if(attempted_)return true;
        attempted_=true;driver_=c.target;
        runtime_=new Runtime(dllPath().c_str(),c.value[0]);
        int32_t status=runtime_->initialize(static_cast<HDIGDRIVER>(local),
            bindFileIo(*callbacks,runtime_->timerRead()),1024*1024);
        out.result.return_bits=static_cast<uint32_t>(status);ready_=status!=0;
        out.result.transport_status=Success;return true;
    }
    if(!attempted_ || !runtime_ || stopped_)return true;
    if(c.target.kind==Driver && !same(c.target,driver_)) {
        out.result.transport_status=InvalidResource;return true;
    }
    if(h.opcode==MilesWire::BinkLastError) {
        const char *error=runtime_->error();
        if(error) {
            size_t n=0;while(n<MilesBinkProtocol::ErrorTextBytes && error[n])++n;
            if(n==MilesBinkProtocol::ErrorTextBytes){out.result.transport_status=TextTooLong;return true;}
            out.text.assign(error,error+n+1);
        } else out.result.null_mask=MilesBinkProtocol::TextNull;
        out.result.transport_status=Success;return true;
    }
    if(h.opcode==MilesWire::BinkShutdown) {
        for(size_t i=0;i<videos_.size();++i)if(videos_[i])return true;
        // All genuine BinkClose calls have returned. Producer-stop semantics
        // still require selected-DLL runtime evidence; counters do not prove it.
        stopped_=true;out.result.transport_status=Success;return true;
    }
    if(!ready_)return true;
    if(h.opcode==MilesWire::BinkOpen) {
        size_t slot=0;while(slot<videos_.size() && videos_[slot])++slot;
        if(slot==videos_.size())videos_.push_back(std::unique_ptr<MilesHostBink::Video>());
        need(registry_.reserve(MilesWire::Video,driver_,reservation_),"Bink reserve before native open");
        opening_=true;
        videos_[slot]=runtime_->open(reinterpret_cast<const char *>(frame.data()+c.text.offset));
        if(videos_[slot]) {
            need(registry_.publish(reservation_,videos_[slot].get(),out.result.resource),"Bink publish retained native video");
            metadata(*videos_[slot],out.result);
        } else registry_.cancel(reservation_);
        opening_=false;out.result.transport_status=Success;return true;
    }
    MilesHostBink::Video *video=static_cast<MilesHostBink::Video *>(local);
    bool owned=false;size_t slot=0;
    for(;slot<videos_.size();++slot)if(videos_[slot].get()==video){owned=true;break;}
    if(!owned){out.result.transport_status=InvalidResource;return true;}
    switch(h.opcode) {
    case MilesWire::BinkClose:
        need(registry_.beginClose(c.target),"Bink close identity");
        video->close();
        need(registry_.retire(c.target),"Bink retire after native close");
        videos_[slot].reset();break;
    case MilesWire::BinkInfo: metadata(*video,out.result);break;
    case MilesWire::BinkDoFrame: out.result.return_bits=static_cast<uint32_t>(video->doFrame());break;
    case MilesWire::BinkNextFrame: video->next();break;
    case MilesWire::BinkWait: out.result.return_bits=static_cast<uint32_t>(video->wait());break;
    case MilesWire::BinkShouldSkip: out.result.return_bits=static_cast<uint32_t>(video->shouldSkip());break;
    case MilesWire::BinkService: video->service();break;
    case MilesWire::BinkPause: out.result.return_bits=static_cast<uint32_t>(video->pause(c.value[0]!=0));break;
    case MilesWire::BinkVideoOnOff: out.result.return_bits=static_cast<uint32_t>(video->videoOnOff(c.value[0]!=0));break;
    case MilesWire::BinkSoundOnOff: out.result.return_bits=static_cast<uint32_t>(video->soundOnOff(c.value[0]!=0));break;
    case MilesWire::BinkVolume: {
        int32_t volume=0;std::memcpy(&volume,&c.value[1],sizeof(volume));
        video->setVolume(c.value[0],volume);break;
    }
    case MilesWire::BinkPixelsBegin: {
        const Metadata m=video->metadata();
        out.result.return_bits=static_cast<uint32_t>(video->copyFrame(c.value[0]));
        out.result.value[0]=m.width;out.result.value[1]=m.height;
        out.result.value[2]=static_cast<uint32_t>(video->pixels().size());
        out.result.value[3]=m.frame;out.result.value[4]=c.value[0];
        transfer_=video;transferId_=c.target;offset_=0;break;
    }
    case MilesWire::BinkPixelsChunk: {
        if(transfer_!=video || !same(transferId_,c.target))return true;
        const std::vector<unsigned char> &pixels=video->pixels();
        if(c.value[0]!=offset_ || c.value[1]>pixels.size()-offset_) {
            out.result.transport_status=InvalidFields;return true;
        }
        out.bytes.assign(pixels.begin()+offset_,pixels.begin()+offset_+c.value[1]);
        offset_+=c.value[1];
        if(offset_==pixels.size()){transfer_=0;transferId_=Handle();offset_=0;}
        break;
    }
    default:return true;
    }
    out.result.transport_status=Success;return true;
}
}
