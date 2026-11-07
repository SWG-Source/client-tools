#include "session_version.h"
namespace MilesSessionVersion {
namespace {
bool nullHandle(const MilesWire::Handle &h) { return !h.kind && !h.slot && !h.generation; }
bool requestHeader(const MilesWire::Header &h) {
    return h.magic==MilesWire::Magic && h.version==MilesWire::Version &&
        h.kind==MilesWire::Request && h.opcode==MilesWire::SessionVersion && h.request && h.lane;
}
}
bool validCapacity(uint32_t capacity) { return capacity>0 && capacity<=CapacityLimit; }
bool validateQuery(MilesTransport::Bytes frame, MilesWire::Header &request, uint32_t &capacity) {
    MilesWire::Header h={}; MilesWire::Call c={};
    if(!MilesTransport::decodeCall(frame,h,c) || !requestHeader(h) ||
       !nullHandle(c.target) || !nullHandle(c.resource) || c.bytes.length || c.text.length ||
       c.output_mask || c.callback || c.reserved || !validCapacity(c.value[0])) return false;
    for(unsigned i=1;i<8;++i) if(c.value[i]) return false;
    request=h; capacity=c.value[0]; return true;
}
bool makeReply(const MilesWire::Header &request, MilesTransport::Bytes prefix,
               std::vector<unsigned char> &frame) {
    if(!requestHeader(request) || !prefix.data || !validCapacity(static_cast<uint32_t>(prefix.size)) ||
       prefix.size>CapacityLimit || prefix.data[prefix.size-1]) return false;
    MilesWire::Header reply=request;reply.kind=MilesWire::Reply;
    MilesWire::Result result={};
    return MilesTransport::encodeResult(reply,result,prefix,MilesTransport::Bytes(),frame);
}
}
