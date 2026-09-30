// Portable protocol test. No engine, worker, vendor DLL, or transport runtime.
#include "../src/eos/eos_protocol.h"
#include <cstdio>
#include <cstdlib>
namespace {
unsigned checks=0;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"EOS protocol failed at line %d\n",line);std::exit(1);}++checks;}
#define CHECK(v) check((v),__LINE__)
MilesTransport::Bytes bytes(const std::vector<unsigned char> &v){return MilesTransport::Bytes(v.data(),v.size());}
void completionRejected(MilesWire::Header h,const MilesWire::Result &r,const MilesWire::Header &expected,const MilesWire::Eos &e){
    std::vector<unsigned char> frame;
    CHECK(MilesTransport::encodeResult(h,r,MilesTransport::Bytes(),MilesTransport::Bytes(),frame));
    CHECK(!MilesEos::validateCompletion(bytes(frame),expected,e));
}
void consumptionRejected(MilesWire::Header h,const MilesWire::Call &c,const MilesWire::Header &expected,const MilesWire::Eos &e){
    std::vector<unsigned char> frame;
    CHECK(MilesTransport::encodeCall(h,c,MilesTransport::Bytes(),MilesTransport::Bytes(),frame));
    CHECK(!MilesEos::validateConsumption(bytes(frame),expected,e));
}
void exercise(bool stream){
    using namespace MilesWire;
    Header h={};h.magic=Magic;h.version=Version;h.kind=Event;
    h.opcode=stream?EndOfStream:EndOfSample;h.request=93;h.causal_request=12;h.lane=1;h.lock_lease=47;
    Eos e={};e.resource.kind=stream?Stream:OwnedSample;e.resource.slot=7;e.resource.generation=11;
    e.registration=stream?65:1;e.event_sequence=h.request;
    CHECK(MilesEos::validEvent(h,e));
    std::vector<unsigned char> event,completion,consumption,frame;
    CHECK(MilesTransport::encodeEos(h,e,event));
    Header decoded={};Eos de={};CHECK(MilesTransport::decodeEos(bytes(event),decoded,de));
    CHECK(MilesEos::validEvent(decoded,de));
    CHECK(MilesEos::encodeCompletion(h,e,completion));CHECK(MilesEos::validateCompletion(bytes(completion),h,e));
    CHECK(MilesEos::encodeConsumption(h,e,consumption));CHECK(MilesEos::validateConsumption(bytes(consumption),h,e));
    Header rh={};Result r={};Header ah={};Call c={};
    CHECK(MilesTransport::decodeResult(bytes(completion),rh,r));CHECK(MilesTransport::decodeCall(bytes(consumption),ah,c));
    for(unsigned n=0;n<4;++n){
        Header bad=rh;
        if(n==0)++bad.request;
        if(n==1)++bad.causal_request;
        if(n==2)++bad.lane;
        if(n==3)++bad.lock_lease;
        completionRejected(bad,r,h,e);
        bad=ah;
        if(n==0)++bad.request;
        if(n==1)++bad.causal_request;
        if(n==2)++bad.lane;
        if(n==3)++bad.lock_lease;
        consumptionRejected(bad,c,h,e);
    }
    for(unsigned n=0;n<8;++n){Result bad=r;bad.value[n]=1;completionRejected(rh,bad,h,e);}
    for(unsigned n=0;n<8;++n){Call bad=c;++bad.value[n];consumptionRejected(ah,bad,h,e);}
    for(unsigned n=0;n<8;++n){
        Result bad=r;
        if(n==0)++bad.resource.generation;
        if(n==1)++bad.resource.slot;
        if(n==2)bad.resource.kind=stream?OwnedSample:Stream;
        if(n==3)bad.callback=stream?1:65;
        if(n==4)++bad.callback;
        if(n==5)bad.transport_status=1;
        if(n==6)bad.return_bits=1;
        if(n==7)bad.null_mask=1;
        completionRejected(rh,bad,h,e);
    }
    for(unsigned n=0;n<7;++n){
        Call bad=c;
        if(n==0)++bad.target.generation;
        if(n==1)++bad.target.slot;
        if(n==2)bad.target.kind=stream?OwnedSample:Stream;
        if(n==3)bad.callback=stream?1:65;
        if(n==4)++bad.callback;
        if(n==5)bad.resource=e.resource;
        if(n==6)bad.output_mask=1;
        consumptionRejected(ah,bad,h,e);
    }
    Header wrong=rh;wrong.opcode=stream?EndOfSample:EndOfStream;completionRejected(wrong,r,h,e);
    wrong=ah;wrong.opcode=FileConsumptionAck;consumptionRejected(wrong,c,h,e);
    wrong=ah;wrong.kind=ReverseRequest;consumptionRejected(wrong,c,h,e);
    for(unsigned n=0;n<7;++n){
        Eos bad=e;
        if(n==0)bad.registration=stream?1:65;
        if(n==1)bad.registration=0;
        if(n==2)bad.registration=stream?129:65;
        if(n==3)++bad.event_sequence;
        if(n==4)bad.reserved=1;
        if(n==5)bad.resource.generation=0;
        if(n==6)bad.resource.kind=BorrowedSample;
        CHECK(!MilesEos::validEvent(h,bad));CHECK(!MilesEos::encodeCompletion(h,bad,frame));CHECK(!MilesEos::encodeConsumption(h,bad,frame));
        CHECK(!MilesEos::validateCompletion(bytes(completion),h,bad));CHECK(!MilesEos::validateConsumption(bytes(consumption),h,bad));
    }
    Eos boundary=e;boundary.registration=stream?128:64;CHECK(MilesEos::validEvent(h,boundary));
    const unsigned char payload=1;
    CHECK(MilesTransport::encodeResult(rh,r,MilesTransport::Bytes(&payload,1),MilesTransport::Bytes(),frame));CHECK(!MilesEos::validateCompletion(bytes(frame),h,e));
    CHECK(MilesTransport::encodeResult(rh,r,MilesTransport::Bytes(),MilesTransport::Bytes(&payload,1),frame));CHECK(!MilesEos::validateCompletion(bytes(frame),h,e));
    CHECK(MilesTransport::encodeCall(ah,c,MilesTransport::Bytes(&payload,1),MilesTransport::Bytes(),frame));CHECK(!MilesEos::validateConsumption(bytes(frame),h,e));
    CHECK(MilesTransport::encodeCall(ah,c,MilesTransport::Bytes(),MilesTransport::Bytes(&payload,1),frame));CHECK(!MilesEos::validateConsumption(bytes(frame),h,e));
    frame=consumption;frame[124]=1;CHECK(!MilesEos::validateConsumption(bytes(frame),h,e)); // Call.reserved on wire
    frame=completion;frame.push_back(0);CHECK(!MilesEos::validateCompletion(bytes(frame),h,e));
    frame=consumption;frame.pop_back();CHECK(!MilesEos::validateConsumption(bytes(frame),h,e));
    wrong=rh;wrong.kind=Reply;
    CHECK(!MilesTransport::encodeResult(wrong,r,MilesTransport::Bytes(),MilesTransport::Bytes(),frame));
    unsigned char fixed[128];size_t written=0;
    CHECK(!MilesTransport::encodeResultInto(wrong,r,MilesTransport::Bytes(),MilesTransport::Bytes(),fixed,sizeof(fixed),written));
    frame=completion;frame[6]=Reply;frame[7]=0;CHECK(!MilesTransport::decodeResult(bytes(frame),decoded,r));
    wrong=h;wrong.kind=Request;CHECK(!MilesTransport::encodeCall(wrong,c,MilesTransport::Bytes(),MilesTransport::Bytes(),frame));
    wrong.kind=ReverseRequest;CHECK(!MilesTransport::encodeCall(wrong,c,MilesTransport::Bytes(),MilesTransport::Bytes(),frame));
    frame=consumption;const uint32_t op=h.opcode;for(unsigned i=0;i<4;++i)frame[8+i]=static_cast<unsigned char>(op>>(8*i));
    CHECK(!MilesTransport::decodeCall(bytes(frame),decoded,c));
    Header background=h;background.causal_request=0;background.lane=999;background.lock_lease=0;
    CHECK(MilesEos::encodeCompletion(background,e,frame));CHECK(MilesEos::validateCompletion(bytes(frame),background,e));
}
}
int main(){exercise(false);exercise(true);std::printf("EOS protocol passed: %u checks\n",checks);return 0;}
