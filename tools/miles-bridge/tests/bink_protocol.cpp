// Portable protocol/registry checks. No Bink DLL, engine, or runtime mocks.
#include "../src/bink/bink_protocol.h"
#include "../src/backend/reply.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace {
unsigned checks=0;
void check(bool value,int line) { if(!value){std::fprintf(stderr,"Bink protocol failure line %d\n",line);std::exit(1);}++checks; }
#define CHECK(x) check((x),__LINE__)
using namespace MilesWire;
using namespace MilesTransport;
Bytes bytes(const std::vector<unsigned char> &v){return Bytes(v.data(),v.size());}
Header header(uint32_t op,uint16_t kind){Header h={};h.magic=Magic;h.version=Version;h.kind=kind;h.opcode=op;h.request=18;h.lane=2;h.lock_lease=7;return h;}
Handle driver={Driver,1,3}, video={Video,2,8};
const char filename[]="movie/original.bik";
const char emptyText[]={0};
const unsigned char pixel[]={1,2,3,4};
Call call(uint32_t op){
    Call c={};c.target=MilesBinkProtocol::targetKind(op)==Driver?driver:video;
    if(op==BinkInitialize)c.value[0]=1024*1024;
    if(op==BinkPause || op==BinkVideoOnOff || op==BinkSoundOnOff)c.value[0]=1;
    if(op==BinkVolume){c.value[0]=2;c.value[1]=0xffffffffU;}
    if(op==BinkPixelsBegin)c.value[0]=MilesBinkProtocol::Surface32A;
    if(op==BinkPixelsChunk){c.value[0]=4;c.value[1]=4;}
    return c;
}
unsigned used(uint32_t op){
    return op==BinkVolume || op==BinkPixelsChunk?2:
        op==BinkInitialize || op==BinkPause || op==BinkVideoOnOff || op==BinkSoundOnOff || op==BinkPixelsBegin?1:0;
}
bool validCall(Header h,Call c,Bytes a=Bytes(),Bytes b=Bytes()){
    std::vector<unsigned char> frame;
    if(!encodeCall(h,c,a,b,frame))return false;
    Header decoded={};Call actual={};
    CHECK(decodeCall(bytes(frame),decoded,actual));
    return MilesBinkProtocol::validCall(decoded,actual,bytes(frame));
}
Result result(uint32_t op){
    Result r={};
    if(op==BinkInitialize || op==BinkDoFrame || op==BinkWait || op==BinkShouldSkip ||
       op==BinkPause || op==BinkVideoOnOff || op==BinkSoundOnOff)r.return_bits=0xffffffffU;
    if(op==BinkOpen || op==BinkInfo){
        const uint32_t metadata[]={64,32,20,2,1,30,1};std::memcpy(r.value,metadata,sizeof(metadata));
        if(op==BinkOpen)r.resource=video;
    }
    if(op==BinkPixelsBegin){r.return_bits=0xffffffffU;r.value[0]=64;r.value[1]=32;r.value[2]=8192;r.value[3]=2;r.value[4]=MilesBinkProtocol::Surface32A;}
    return r;
}
bool validReply(Header h,Result r,const Header &expected,Bytes a=Bytes(),Bytes b=Bytes()){
    std::vector<unsigned char> frame;
    if(!encodeResult(h,r,a,b,frame))return false;
    StartupBridge::OwnedReply out;
    return StartupBridge::decodeReply(bytes(frame),expected,out);
}
void calls(){
    for(uint32_t op=BinkInitialize;op<=BinkShutdown;++op){
        Header h=header(op,Request);Call c=call(op);Bytes name=op==BinkOpen?Bytes(filename,sizeof(filename)):Bytes();
        CHECK(validCall(h,c,Bytes(),name));
        for(unsigned i=used(op);i<8;++i){Call bad=c;bad.value[i]=1;CHECK(!validCall(h,bad,Bytes(),name));}
        Call bad=c;bad.resource=driver;CHECK(!validCall(h,bad,Bytes(),name));
        bad=c;bad.callback=1;CHECK(!validCall(h,bad,Bytes(),name));
        bad=c;bad.output_mask=1;CHECK(!validCall(h,bad,Bytes(),name));
        bad=c;bad.reserved=1;CHECK(!validCall(h,bad,Bytes(),name));
        bad=c;bad.target.kind=Stream;CHECK(!validCall(h,bad,Bytes(),name));
        bad=c;bad.target=Handle();CHECK(!validCall(h,bad,Bytes(),name));
        CHECK(!validCall(h,c,Bytes(pixel,4),name));
        CHECK(!validCall(h,c,Bytes(),Bytes(emptyText,1)));
        Header wrong=h;wrong.kind=ReverseRequest;CHECK(!validCall(wrong,c,Bytes(),name));
        wrong=h;wrong.kind=Event;CHECK(!validCall(wrong,c,Bytes(),name));
        wrong=h;wrong.version=3;CHECK(!validCall(wrong,c,Bytes(),name));
        wrong=h;wrong.request=0;CHECK(!validCall(wrong,c,Bytes(),name));
        wrong=h;wrong.lane=0;CHECK(!validCall(wrong,c,Bytes(),name));
    }
    for(uint32_t op=BinkPause;op<=BinkSoundOnOff;++op){Call c=call(op);c.value[0]=2;CHECK(!validCall(header(op,Request),c));}
    Call c=call(BinkInitialize);c.value[0]=0;CHECK(!validCall(header(BinkInitialize,Request),c));
    c=call(BinkPixelsBegin);c.value[0]=6;CHECK(!validCall(header(BinkPixelsBegin,Request),c));
    c=call(BinkPixelsChunk);c.value[1]=0;CHECK(!validCall(header(BinkPixelsChunk,Request),c));
    c.value[1]=MilesBinkProtocol::ChunkBytes+1;CHECK(!validCall(header(BinkPixelsChunk,Request),c));
    c.value[0]=UINT32_MAX;c.value[1]=1;CHECK(!validCall(header(BinkPixelsChunk,Request),c));
    c.value[0]=0;c.value[1]=MilesBinkProtocol::ChunkBytes;CHECK(validCall(header(BinkPixelsChunk,Request),c));
    std::vector<unsigned char> text(512,'x');text.back()=0;c=call(BinkOpen);
    CHECK(validCall(header(BinkOpen,Request),c,Bytes(),bytes(text)));
    text.push_back(0);CHECK(!validCall(header(BinkOpen,Request),c,Bytes(),bytes(text)));
    text.resize(512);text.back()='x';CHECK(!validCall(header(BinkOpen,Request),c,Bytes(),bytes(text)));
    text.back()=0;text[1]=0;CHECK(!validCall(header(BinkOpen,Request),c,Bytes(),bytes(text)));
    std::vector<unsigned char> frame;CHECK(encodeCall(header(BinkInfo,Request),call(BinkInfo),Bytes(),Bytes(),frame));
    frame[124]=1;Header h={};Call decoded={};CHECK(!decodeCall(bytes(frame),h,decoded));
}
void replies(){
    for(uint32_t op=BinkInitialize;op<=BinkShutdown;++op){
        Header h=header(op,Reply);Result r=result(op);
        Bytes a=op==BinkPixelsChunk?Bytes(pixel,4):Bytes();Bytes b=op==BinkLastError?Bytes(emptyText,1):Bytes();
        CHECK(validReply(h,r,h,a,b));
        for(unsigned field=0;field<4;++field){Header wrong=h;if(field==0)++wrong.request;if(field==1)++wrong.lane;if(field==2)++wrong.lock_lease;if(field==3)++wrong.causal_request;CHECK(!validReply(wrong,r,h,a,b));}
        Header wrong=h;wrong.opcode=op==BinkShutdown?uint32_t(BinkInitialize):op+1;CHECK(!validReply(wrong,r,h,a,b));
        wrong=h;wrong.kind=ReverseReply;CHECK(!validReply(wrong,r,h,a,b));
        wrong=h;wrong.version=3;CHECK(!validReply(wrong,r,h,a,b));
        Result bad=r;bad.callback=1;CHECK(!validReply(h,bad,h,a,b));
        bad=r;bad.null_mask=2;CHECK(!validReply(h,bad,h,a,b));
        bad=r;bad.value[7]=1;CHECK(!validReply(h,bad,h,a,b));
        bad=r;bad.resource=driver;CHECK(!validReply(h,bad,h,a,b));
        bad=r;bad.transport_status=StartupBridge::InvalidFields;CHECK(!validReply(h,bad,h,Bytes(pixel,4),b));
        Result refusal={};refusal.transport_status=StartupBridge::InvalidFields;CHECK(validReply(h,refusal,h));
        refusal.return_bits=1;CHECK(!validReply(h,refusal,h));
        refusal=Result();refusal.transport_status=99;CHECK(!validReply(h,refusal,h));
        if(op!=BinkPixelsChunk)CHECK(!validReply(h,r,h,Bytes(pixel,4),b));
        if(op!=BinkLastError)CHECK(!validReply(h,r,h,a,Bytes(emptyText,1)));
    }
    Result r={};Header h=header(BinkOpen,Reply);CHECK(validReply(h,r,h));r.value[0]=1;CHECK(!validReply(h,r,h));
    r=result(BinkOpen);r.resource.kind=OwnedSample;CHECK(!validReply(h,r,h));
    r=result(BinkPixelsBegin);h=header(BinkPixelsBegin,Reply);++r.value[2];CHECK(!validReply(h,r,h));
    r=result(BinkPixelsBegin);r.value[0]=UINT32_MAX;r.value[1]=UINT32_MAX;CHECK(!validReply(h,r,h));
    const uint32_t formats[]={MilesBinkProtocol::Surface32A,MilesBinkProtocol::Surface565,MilesBinkProtocol::Surface5551};
    for(unsigned i=0;i<3;++i){r=result(BinkPixelsBegin);r.value[4]=formats[i];r.value[2]=64*32*MilesBinkProtocol::pixelBytes(formats[i]);CHECK(validReply(h,r,h));}
    h=header(BinkLastError,Reply);r=Result();r.null_mask=MilesBinkProtocol::TextNull;CHECK(validReply(h,r,h));CHECK(!validReply(h,r,h,Bytes(),Bytes(emptyText,1)));
    r=Result();CHECK(!validReply(h,r,h));CHECK(validReply(h,r,h,Bytes(),Bytes(emptyText,1)));
    std::vector<unsigned char> text(MilesBinkProtocol::ErrorTextBytes,'x');text.back()=0;
    CHECK(validReply(h,r,h,Bytes(),bytes(text)));text.push_back(0);CHECK(!validReply(h,r,h,Bytes(),bytes(text)));
    text.resize(MilesBinkProtocol::ErrorTextBytes);text[2]=0;CHECK(!validReply(h,r,h,Bytes(),bytes(text)));
    h=header(BinkPixelsChunk,Reply);CHECK(!validReply(h,Result(),h));
    std::vector<unsigned char> chunk(MilesBinkProtocol::ChunkBytes,42);CHECK(validReply(h,Result(),h,bytes(chunk)));chunk.push_back(42);CHECK(!validReply(h,Result(),h,bytes(chunk)));
    // Empty/scalar operations must not inherit metadata fields from another opcode.
    h=header(BinkClose,Reply);r=Result();r.value[0]=1;CHECK(!validReply(h,r,h));r=Result();r.return_bits=1;CHECK(!validReply(h,r,h));
}
void registry(){
    ResourceRegistry registry(8);int d=0,d2=0,v=0;Handle parent={},other={},child={};void *local=0;
    CHECK(registry.insert(Driver,&d,parent));CHECK(registry.insert(Driver,&d2,other));
    CHECK(!registry.insert(Video,&v,child));ResourceRegistry::Reservation pending;
    CHECK(!registry.reserve(Video,Handle(),pending));Handle bad=parent;++bad.generation;CHECK(!registry.reserve(Video,bad,pending));bad=parent;bad.kind=Stream;CHECK(!registry.reserve(Video,bad,pending));
    CHECK(registry.reserve(Video,parent,pending));CHECK(registry.publish(pending,&v,child));
    Header h=header(BinkInfo,Request);Call c={};c.target=child;
    CHECK(MilesBinkProtocol::resolveTarget(registry,h,c,local));CHECK(local==&v);
    ++c.target.generation;CHECK(!MilesBinkProtocol::resolveTarget(registry,h,c,local));CHECK(!local);
    c.target=parent;CHECK(!MilesBinkProtocol::resolveTarget(registry,h,c,local));
    CHECK(registry.retire(other));CHECK(registry.resolve(child,Video,local));
    CHECK(registry.beginClose(parent));CHECK(!registry.resolve(child,Video,local));
    CHECK(registry.retire(parent));CHECK(!registry.resolve(child,Video,local));CHECK(!registry.retire(child));
    CHECK(registry.insert(Driver,&d,parent));CHECK(registry.reserve(Video,parent,pending));CHECK(registry.publish(pending,&v,child));
    Handle old=child;CHECK(registry.beginClose(child));CHECK(!registry.resolve(child,Video,local));CHECK(registry.retire(child));
    CHECK(registry.reserve(Video,parent,pending));CHECK(registry.publish(pending,&v,child));CHECK(!registry.resolve(old,Video,local));
    ResourceRegistry::Reservation unpub;CHECK(registry.reserve(Video,parent,unpub));CHECK(registry.retire(parent));CHECK(!registry.publish(unpub,&v,old));registry.cancel(unpub);CHECK(registry.empty());
}
}
int main(){static_assert(MilesWire::Version==4,"paired Bink protocol");static_assert(MilesBinkProtocol::TextNull==int(MilesStartup::TextNull),"shared null flag");calls();replies();registry();std::printf("Bink protocol: %u checks passed\n",checks);}
