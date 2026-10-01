#ifndef MILES_BINK_PROTOCOL_H
#define MILES_BINK_PROTOCOL_H
#include "../wire/codec.h"
#include <cstring>
namespace MilesBinkProtocol {
// Native Bink 1.9c surface numbers; no SDK header crosses into the x64 client.
enum { Surface32A=5, Surface5551=8, Surface565=10,
       FilenameBytes=512, ErrorTextBytes=4096, TextNull=1, ChunkBytes=MilesWire::MaxFrameBytes-128 };
// Initialize: Driver, value0=pixel budget. Open: Driver, text=filename+NUL.
// LastError/Shutdown: Driver. All remaining targets: Video.
// Pause/VideoOnOff/SoundOnOff: value0 boolean. Volume: value0 track,value1 S32 bits.
// PixelsBegin: value0 surface. PixelsChunk: value0 offset,value1 positive count.
// Open/Info results: value0..6 width,height,frames,frame,lastFrame,rate,rateDiv;
// only Open returns a nullable Video resource (null requires zero metadata).
// PixelsBegin: native return_bits and value0..4 width,height,total,frame,surface.
// PixelsChunk: raw bytes only. LastError: bounded NUL text or TextNull (same as metadata).
// Initialize/DoFrame/Wait/ShouldSkip/Pause/VideoOnOff/SoundOnOff: return_bits only.
// Close/NextFrame/Service/Volume/Shutdown: empty success. Refusals: status only.
inline bool opcode(uint32_t op) { return op>=MilesWire::BinkInitialize && op<=MilesWire::BinkShutdown; }
inline bool nullHandle(const MilesWire::Handle &h) { return !h.kind && !h.slot && !h.generation; }
inline bool surface(uint32_t f) { return f==Surface32A || f==Surface5551 || f==Surface565; }
inline unsigned pixelBytes(uint32_t f) { return f==Surface32A ? 4 : surface(f) ? 2 : 0; }
inline bool zeros(const uint32_t *values,unsigned begin) {
    for(unsigned i=begin;i<8;++i) if(values[i])return false;
    return true;
}
inline bool boundedText(MilesTransport::Bytes frame,const MilesWire::Span &s,uint32_t limit) {
    return s.length && s.length<=limit && s.offset<=frame.size &&
        s.length<=frame.size-s.offset && frame.data &&
        !frame.data[s.offset+s.length-1] &&
        !std::memchr(frame.data+s.offset,0,s.length-1);
}
inline MilesWire::ResourceKind targetKind(uint32_t op) {
    return op==MilesWire::BinkInitialize || op==MilesWire::BinkOpen ||
        op==MilesWire::BinkLastError || op==MilesWire::BinkShutdown ? MilesWire::Driver : MilesWire::Video;
}
// Call only after the real codec validates canonical spans/frame length. These
// checks describe admissible fields; native row/copy state remains host-owned.
inline bool validCall(const MilesWire::Header &h,const MilesWire::Call &c,MilesTransport::Bytes frame) {
    if(h.magic!=MilesWire::Magic || h.version!=MilesWire::Version || h.kind!=MilesWire::Request ||
       !h.request || !h.lane || !opcode(h.opcode) || h.bytes!=frame.size ||
       c.target.kind!=static_cast<uint32_t>(targetKind(h.opcode)) ||
       !MilesTransport::validHandle(c.target) || !nullHandle(c.resource) ||
       c.callback || c.reserved || c.output_mask || c.bytes.length || c.bytes.offset)return false;
    unsigned used=0;
    switch(h.opcode) {
    case MilesWire::BinkInitialize: if(!c.value[0])return false;used=1;break;
    case MilesWire::BinkPause: case MilesWire::BinkVideoOnOff: case MilesWire::BinkSoundOnOff:
        if(c.value[0]>1)return false;
        used=1;break;
    case MilesWire::BinkVolume: used=2;break;
    case MilesWire::BinkPixelsBegin: if(!surface(c.value[0]))return false;used=1;break;
    case MilesWire::BinkPixelsChunk:
        if(!c.value[1] || c.value[1]>ChunkBytes || c.value[0]>UINT32_MAX-c.value[1])return false;
        used=2;break;
    default:break;
    }
    if(!zeros(c.value,used))return false;
    if(h.opcode==MilesWire::BinkOpen)
        return c.text.length>1 && boundedText(frame,c.text,FilenameBytes);
    return !c.text.length && !c.text.offset;
}
inline bool resolveTarget(const MilesTransport::ResourceRegistry &registry,
        const MilesWire::Header &h,const MilesWire::Call &c,void *&local) {
    local=0;
    return opcode(h.opcode) && registry.resolve(c.target,targetKind(h.opcode),local);
}
inline bool validResult(const MilesWire::Header &h,const MilesWire::Result &r,MilesTransport::Bytes frame) {
    if(h.magic!=MilesWire::Magic || h.version!=MilesWire::Version || h.kind!=MilesWire::Reply ||
       !h.request || !h.lane || !opcode(h.opcode) || h.bytes!=frame.size || r.callback)return false;
    if(r.null_mask && (r.transport_status || h.opcode!=MilesWire::BinkLastError || r.null_mask!=TextNull))return false;
    if(r.transport_status) {
        if(!(r.transport_status<=3 || (r.transport_status>=0x1001 && r.transport_status<=0x1004)))return false;
        return !r.return_bits && nullHandle(r.resource) && zeros(r.value,0) &&
            !r.bytes.length && !r.bytes.offset && !r.text.length && !r.text.offset;
    }
    bool scalar=false;unsigned values=0;
    switch(h.opcode) {
    case MilesWire::BinkInitialize: case MilesWire::BinkDoFrame: case MilesWire::BinkWait:
    case MilesWire::BinkShouldSkip: case MilesWire::BinkPause: case MilesWire::BinkVideoOnOff:
    case MilesWire::BinkSoundOnOff: scalar=true;break;
    case MilesWire::BinkOpen: case MilesWire::BinkInfo: values=7;break;
    case MilesWire::BinkPixelsBegin: scalar=true;values=5;break;
    default:break;
    }
    if((!scalar && r.return_bits) || !zeros(r.value,values))return false;
    if(h.opcode==MilesWire::BinkOpen) {
        if(nullHandle(r.resource)) { if(!zeros(r.value,0))return false; }
        else if(r.resource.kind!=MilesWire::Video || !MilesTransport::validHandle(r.resource))return false;
    } else if(!nullHandle(r.resource))return false;
    if(h.opcode==MilesWire::BinkPixelsBegin) {
        uint64_t bytes=uint64_t(r.value[0])*r.value[1];
        unsigned bpp=pixelBytes(r.value[4]);
        // Check division before multiplication to keep malformed extents bounded.
        if(!bpp || !bytes || bytes>UINT32_MAX/bpp || bytes*bpp!=r.value[2])return false;
    }
    if(h.opcode==MilesWire::BinkPixelsChunk) {
        if(!r.bytes.length || r.bytes.length>ChunkBytes || r.bytes.offset>frame.size ||
           r.bytes.length>frame.size-r.bytes.offset)return false;
    } else if(r.bytes.length || r.bytes.offset)return false;
    if(h.opcode==MilesWire::BinkLastError)
        return r.null_mask==TextNull ? !r.text.length && !r.text.offset : boundedText(frame,r.text,ErrorTextBytes);
    return !r.text.length && !r.text.offset;
}
}
#endif
