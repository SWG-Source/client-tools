#include "../ClientBink.h"
#include "Session.h"
#include "VideoState.h"
#include "../../bink/bink_protocol.h"
#include "../../callback-guard/invocation_guard.h"
#include "../../failure/failure_boundary.h"
#include <cstring>
#include <limits>
#include <algorithm>
namespace {
void require(bool value,const char *message) { if(!value)throw std::runtime_error(message); }
uint32_t bpp(uint32_t format) {
    if(format==ClientBink::Surface32A)return 4;
    if(format==ClientBink::Surface565 || format==ClientBink::Surface5551)return 2;
    return 0;
}
ClientMilesPipe::Session &running() {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();s.requireRunning();
    require(s.videos->started && !s.videos->stopped && !s.videos->copying,"Bink lifecycle/transfer state");
    return s;
}
MilesWire::Call target(ClientMilesPipe::Session &s,ClientBink::Handle handle) {
    require(handle!=0,"Bink null video");
    for(auto i=s.videos->rows.begin();i!=s.videos->rows.end();++i)
        if(i->get()==handle && (*i)->live) {
            MilesWire::Call call={};call.target=(*i)->wire;return call;
        }
    throw std::runtime_error("Bink video is not owned by this session");
}
int32_t scalar(uint32_t bits) {int32_t value;std::memcpy(&value,&bits,sizeof(value));return value;}
ClientBink::Info metadata(const MilesWire::Result &r) {
    ClientBink::Info info={r.value[0],r.value[1],r.value[2],r.value[3],r.value[4],r.value[5],r.value[6]};return info;
}
template<class R,class F> R guarded(F f) {
    try {
        ClientMilesPrivate52::requireFatalReporter();
        MilesCallbackGuard47::requireForwardAllowed();return f();
    } catch(const std::exception &e) {ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,e.what());}
      catch(...) {ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"Bink private adapter failure");}
}
int32_t control(ClientBink::Handle h,uint32_t opcode,uint32_t first=0,uint32_t second=0) {
    ClientMilesPipe::Session &s=running();MilesWire::Call c=target(s,h);
    c.value[0]=first;c.value[1]=second;return scalar(s.request(opcode,c).result.return_bits);
}
}
namespace ClientBink {
int32_t initialize(ClientMiles::HDIGDRIVER driver,uint32_t budget) {
    return guarded<int32_t>([=]() {
        ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();s.requireRunning();
        require(s.fileCallbacksInstalled() && !s.videos->attempted && budget,"Bink initialization prerequisites");
        MilesWire::Call c={};c.target=s.verifiedDriver(driver);c.value[0]=budget;
        s.videos->attempted=true;s.videos->driver=c.target;s.videos->budget=budget;
        int32_t result=scalar(s.request(MilesWire::BinkInitialize,c).result.return_bits);
        s.videos->started=result!=0;return result;
    });
}
void shutdown() {guarded<void>([](){
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();s.requireRunning();
    require(s.videos->attempted && !s.videos->stopped && !s.videos->copying &&
        !s.videos->anyLive(),"Bink shutdown state/videos");
    MilesWire::Call c={};c.target=s.videos->driver;s.request(MilesWire::BinkShutdown,c);
    s.videos->started=false;s.videos->stopped=true;
});}
Handle open(const char *filename) {return guarded<Handle>([=]() -> Handle {
    ClientMilesPipe::Session &s=running();require(filename!=0,"Bink filename");
    size_t length=0;while(length<512 && filename[length])++length;
    require(length && length<512,"Bink filename extent");
    // Allocate the local identity before any native open effect; never recycle
    // stale pointer identities during this session.
    s.videos->rows.push_back(std::unique_ptr<VideoToken>(new VideoToken));
    VideoToken *row=s.videos->rows.back().get();
    MilesWire::Call c={};c.target=s.videos->driver;
    StartupBridge::OwnedReply r=s.request(MilesWire::BinkOpen,c,MilesTransport::Bytes(filename,length+1));
    if(!r.result.resource.kind){s.videos->rows.pop_back();return 0;}
    row->wire=r.result.resource;row->live=true;return row;
});}
void close(Handle h) {guarded<void>([=](){
    ClientMilesPipe::Session &s=running();MilesWire::Call c=target(s,h);
    s.request(MilesWire::BinkClose,c);h->live=false;
});}
Info info(Handle h) {return guarded<Info>([=](){
    ClientMilesPipe::Session &s=running();return metadata(s.request(MilesWire::BinkInfo,target(s,h)).result);
});}
int32_t doFrame(Handle h){return guarded<int32_t>([=](){return control(h,MilesWire::BinkDoFrame);});}
void nextFrame(Handle h){guarded<void>([=](){control(h,MilesWire::BinkNextFrame);});}
int32_t wait(Handle h){return guarded<int32_t>([=](){return control(h,MilesWire::BinkWait);});}
int32_t shouldSkip(Handle h){return guarded<int32_t>([=](){return control(h,MilesWire::BinkShouldSkip);});}
void service(Handle h){guarded<void>([=](){control(h,MilesWire::BinkService);});}
int32_t pause(Handle h,bool on){return guarded<int32_t>([=](){return control(h,MilesWire::BinkPause,on?1:0);});}
int32_t setVideoOnOff(Handle h,bool on){return guarded<int32_t>([=](){return control(h,MilesWire::BinkVideoOnOff,on?1:0);});}
int32_t setSoundOnOff(Handle h,bool on){return guarded<int32_t>([=](){return control(h,MilesWire::BinkSoundOnOff,on?1:0);});}
void setVolume(Handle h,uint32_t track,int32_t volume){guarded<void>([=](){
    uint32_t bits;std::memcpy(&bits,&volume,sizeof(bits));control(h,MilesWire::BinkVolume,track,bits);
});}
int32_t copyToBuffer(Handle h,void *destination,int32_t stride,uint32_t height,PixelFormat format) {
    return guarded<int32_t>([=](){
        ClientMilesPipe::Session &s=running();MilesWire::Call c=target(s,h);
        Info dimensions=metadata(s.request(MilesWire::BinkInfo,c).result);
        uint32_t const pixelBytes=bpp(format);
        uint64_t const pitch=uint64_t(dimensions.width)*pixelBytes;
        require(destination && stride>0 && pixelBytes && dimensions.width && dimensions.height &&
            pitch<=static_cast<uint32_t>(stride) && height>=dimensions.height,"Bink destination extent");
        uint64_t const total=pitch*dimensions.height; // pitch is bounded by positive int32 stride
        require(total<=s.videos->budget && total<=UINT32_MAX,"Bink pixel budget");
        require(uint64_t(stride)*height <= (std::numeric_limits<size_t>::max)(),"Bink destination address extent");
        // Fully receive/validate into private memory before touching the caller's
        // texture. A partial reply or crash is never a completed frame.
        std::vector<unsigned char> pixels(static_cast<size_t>(total));
        s.videos->copying=true;s.videos->copyingVideo=c.target;s.videos->copyingInfo=dimensions;
        s.videos->format=format;s.videos->total=static_cast<uint32_t>(total);s.videos->offset=0;
        c.value[0]=format;
        int32_t result=scalar(s.request(MilesWire::BinkPixelsBegin,c).result.return_bits);
        while(s.videos->offset<s.videos->total) {
            c.value[0]=s.videos->offset;
            c.value[1]=(std::min)(s.videos->total-s.videos->offset,uint32_t(MilesWire::MaxFrameBytes-128));
            StartupBridge::OwnedReply r=s.request(MilesWire::BinkPixelsChunk,c);
            std::memcpy(&pixels[s.videos->offset],r.bytes.data(),r.bytes.size());
            s.videos->offset+=c.value[1];
        }
        s.videos->copying=false;
        for(uint32_t y=0;y<dimensions.height;++y)
            std::memcpy(static_cast<unsigned char *>(destination)+size_t(y)*stride,
                        &pixels[size_t(y)*static_cast<size_t>(pitch)],static_cast<size_t>(pitch));
        return result;
    });
}
const char *lastError(){return guarded<const char *>([]() -> const char * {
    ClientMilesPipe::Session &s=ClientMilesPipe::Session::selected();s.requireRunning();
    require(s.videos->attempted && !s.videos->stopped && !s.videos->copying,"Bink error query lifecycle");
    MilesWire::Call c={};c.target=s.videos->driver;
    StartupBridge::OwnedReply r=s.request(MilesWire::BinkLastError,c);
    if(r.result.null_mask){s.videos->error.clear();return 0;}
    require(!r.text.empty() && !r.text.back(),"Bink error reply");
    s.videos->error.assign(reinterpret_cast<const char *>(r.text.data()),r.text.size()-1);
    return s.videos->error.c_str();
});}
}
