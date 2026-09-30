#include "file_protocol.h"
namespace MilesFileProtocol48 {
namespace {
bool nullHandle(const MilesWire::Handle &h){return !h.kind && !h.slot && !h.generation;}
bool zeroValues(const uint32_t *v,unsigned first){for(unsigned i=first;i<8;++i)if(v[i])return false;return true;}
bool sameOrigin(const MilesWire::Header &h,const MilesWire::Header &expected) {
    return h.request==expected.request && h.causal_request==expected.causal_request &&
           h.lane==expected.lane && h.lock_lease==expected.lock_lease;
}
bool currentOrigin(const MilesWire::Header &h) {
    return h.magic==MilesWire::Magic && h.version==MilesWire::Version && h.request && h.lane;
}
bool installHeader(const MilesWire::Header &h) {
    return currentOrigin(h) && h.kind==MilesWire::Request && h.opcode==MilesWire::AIL_set_file_callbacks &&
        !h.causal_request && !h.lock_lease;
}
bool status(uint32_t s){return s==Installed || s==Unsupported || s==InvalidFields || s==LifecycleRefused;}
bool fileExpected(const FileAckExpected &e) {
    return e.registration && currentOrigin(e.original) && e.original.kind==MilesWire::ReverseRequest &&
        e.original.opcode>=MilesWire::FileOpen && e.original.opcode<=MilesWire::FileRead;
}
bool emptyCall(const MilesWire::Call &c,unsigned usedValues) {
    return nullHandle(c.target) && nullHandle(c.resource) && zeroValues(c.value,usedValues) &&
        !c.bytes.offset && !c.bytes.length && !c.text.offset && !c.text.length && !c.output_mask && !c.reserved;
}
bool emptyResult(const MilesWire::Result &r,unsigned usedValues) {
    return !r.return_bits && zeroValues(r.value,usedValues) && !r.bytes.offset && !r.bytes.length &&
        !r.text.offset && !r.text.length && !r.null_mask;
}
}
bool encodeInstall(const MilesWire::Header &h,uint64_t registration,
                   unsigned char *buffer,size_t capacity,size_t &written) {
    if(!installHeader(h) || !registration)return false;
    MilesWire::Call c={};c.callback=registration;
    return MilesTransport::encodeCallInto(h,c,MilesTransport::Bytes(),MilesTransport::Bytes(),buffer,capacity,written);
}
bool decodeInstall(MilesTransport::Bytes bytes,const MilesWire::Header &expected,InstallRequest &out) {
    MilesWire::Header h={};MilesWire::Call c={};
    if(!installHeader(expected) || bytes.size!=InstallCallBytes || !MilesTransport::decodeCall(bytes,h,c) ||
       !installHeader(h) || !sameOrigin(h,expected) || !c.callback || !emptyCall(c,0))return false;
    out.header=h;out.registration=c.callback;return true;
}
bool encodeInstallReply(const InstallRequest &request,InstallStatus outcome,
                        unsigned char *buffer,size_t capacity,size_t &written) {
    if(!installHeader(request.header) || !request.registration || !status(outcome))return false;
    MilesWire::Header h=request.header;h.kind=MilesWire::Reply;
    MilesWire::Result r={};r.transport_status=outcome;r.callback=outcome==Installed?request.registration:0;
    return MilesTransport::encodeResultInto(h,r,MilesTransport::Bytes(),MilesTransport::Bytes(),buffer,capacity,written);
}
bool decodeInstallReply(MilesTransport::Bytes bytes,const InstallRequest &request,InstallStatus &out) {
    MilesWire::Header h={};MilesWire::Result r={};
    if(!installHeader(request.header) || !request.registration || bytes.size!=InstallReplyBytes ||
       !MilesTransport::decodeResult(bytes,h,r) || h.kind!=MilesWire::Reply ||
       h.opcode!=MilesWire::AIL_set_file_callbacks || !sameOrigin(h,request.header) || !status(r.transport_status) ||
       !emptyResult(r,0) || !nullHandle(r.resource) ||
       r.callback!=(r.transport_status==Installed?request.registration:0))return false;
    out=static_cast<InstallStatus>(r.transport_status);return true;
}
bool expectFileAck(const MilesFileChannel26::Request &request,uint64_t registration,FileAckExpected &out) {
    if(!request.valid())return false;
    FileAckExpected candidate;candidate.original=request.header();candidate.registration=registration;
    if(!fileExpected(candidate))return false;
    out=candidate;return true;
}
bool encodeFileConsumptionAck(const FileAckExpected &expected,unsigned char *buffer,size_t capacity,size_t &written) {
    if(!fileExpected(expected))return false;
    MilesWire::Header h=expected.original;h.opcode=MilesWire::FileConsumptionAck;
    MilesWire::Call c={};c.callback=expected.registration;c.value[0]=expected.original.opcode;
    return MilesTransport::encodeCallInto(h,c,MilesTransport::Bytes(),MilesTransport::Bytes(),buffer,capacity,written);
}
bool validateFileConsumptionAck(MilesTransport::Bytes bytes,const FileAckExpected &expected) {
    MilesWire::Header h={};MilesWire::Call c={};
    return fileExpected(expected) && bytes.size==ConsumptionAckBytes && MilesTransport::decodeCall(bytes,h,c) &&
        h.kind==MilesWire::ReverseRequest && h.opcode==MilesWire::FileConsumptionAck &&
        sameOrigin(h,expected.original) && c.callback==expected.registration &&
        c.value[0]==expected.original.opcode && emptyCall(c,1);
}
bool decodeStreamAliasSuccess(MilesTransport::Bytes bytes,const MilesWire::Header &expected,
                              const MilesWire::Handle &parent,MilesWire::Handle &alias) {
    MilesWire::Header h={};MilesWire::Result r={};
    if(!currentOrigin(expected) || expected.kind!=MilesWire::Request || expected.opcode!=MilesWire::AIL_stream_sample_handle ||
       parent.kind!=MilesWire::Stream || !parent.slot || !parent.generation || bytes.size!=128 ||
       !MilesTransport::decodeResult(bytes,h,r) || h.kind!=MilesWire::Reply || h.opcode!=expected.opcode ||
       !sameOrigin(h,expected) || r.transport_status || r.callback || !emptyResult(r,3) ||
       r.value[0]!=parent.kind || r.value[1]!=parent.slot || r.value[2]!=parent.generation ||
       (!nullHandle(r.resource) && r.resource.kind!=MilesWire::BorrowedSample))return false;
    alias=r.resource;return true;
}
}
