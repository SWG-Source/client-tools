#include "eos_protocol.h"
namespace MilesEos {
namespace {
bool same(const MilesWire::Handle &a,const MilesWire::Handle &b){return a.kind==b.kind&&a.slot==b.slot&&a.generation==b.generation;}
bool envelope(const MilesWire::Header &a,const MilesWire::Header &b){return a.request==b.request&&a.causal_request==b.causal_request&&a.lane==b.lane&&a.lock_lease==b.lock_lease;}
bool zero(const MilesWire::Handle &a){return !a.kind&&!a.slot&&!a.generation;}
}
bool validEvent(const MilesWire::Header &h,const MilesWire::Eos &e){
    return h.magic==MilesWire::Magic&&h.version==MilesWire::Version&&h.kind==MilesWire::Event&&
        h.request&&h.lane&&!e.reserved&&e.event_sequence==h.request&&e.resource.slot&&e.resource.generation&&
        ((h.opcode==MilesWire::EndOfSample&&e.resource.kind==MilesWire::OwnedSample&&e.registration>=1&&e.registration<=64)||
         (h.opcode==MilesWire::EndOfStream&&e.resource.kind==MilesWire::Stream&&e.registration>=65&&e.registration<=128));
}
bool encodeCompletion(MilesWire::Header h,const MilesWire::Eos &e,std::vector<unsigned char> &out){
    if(!validEvent(h,e))return false;
    h.kind=MilesWire::ReverseReply;MilesWire::Result r={};r.resource=e.resource;r.callback=e.registration;
    return MilesTransport::encodeResult(h,r,MilesTransport::Bytes(),MilesTransport::Bytes(),out);
}
bool validateCompletion(MilesTransport::Bytes frame,const MilesWire::Header &expected,const MilesWire::Eos &e){
    MilesWire::Header h={};MilesWire::Result r={};
    if(!validEvent(expected,e)||!MilesTransport::decodeResult(frame,h,r)||!envelope(h,expected)||h.kind!=MilesWire::ReverseReply||h.opcode!=expected.opcode||
       !same(r.resource,e.resource)||r.callback!=e.registration||r.transport_status||r.return_bits||r.null_mask||r.bytes.offset||r.bytes.length||r.text.offset||r.text.length)return false;
    for(unsigned i=0;i<8;++i)if(r.value[i])return false;
    return true;
}
bool encodeConsumption(MilesWire::Header h,const MilesWire::Eos &e,std::vector<unsigned char> &out){
    if(!validEvent(h,e))return false;
    MilesWire::Call c={};c.target=e.resource;c.callback=e.registration;c.value[0]=h.opcode;
    h.kind=MilesWire::Request;h.opcode=MilesWire::CallbackAck;
    return MilesTransport::encodeCall(h,c,MilesTransport::Bytes(),MilesTransport::Bytes(),out);
}
bool validateConsumption(MilesTransport::Bytes frame,const MilesWire::Header &expected,const MilesWire::Eos &e){
    MilesWire::Header h={};MilesWire::Call c={};
    if(!validEvent(expected,e)||!MilesTransport::decodeCall(frame,h,c)||!envelope(h,expected)||h.kind!=MilesWire::Request||h.opcode!=MilesWire::CallbackAck||
       !same(c.target,e.resource)||c.callback!=e.registration||!zero(c.resource)||c.value[0]!=expected.opcode||c.output_mask||c.reserved||c.bytes.offset||c.bytes.length||c.text.offset||c.text.length)return false;
    for(unsigned i=1;i<8;++i)if(c.value[i])return false;
    return true;
}
}
