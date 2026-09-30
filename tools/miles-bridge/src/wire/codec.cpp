#include "codec.h"
namespace MilesTransport {
namespace {
using namespace MilesWire;
struct Writer {
    std::vector<unsigned char> b;
    void put(uint64_t x, unsigned n) {
        for (unsigned i = 0; i < n; ++i) {
            b.push_back(static_cast<unsigned char>(x & 255));
            x >>= 8;
        }
    }
    void handle(const Handle &h) {
        put(h.kind, 4);
        put(h.slot, 4);
        put(h.generation, 4);
    }
    void span(const Span &s) {
        put(s.offset, 4);
        put(s.length, 4);
    }
    void payload(Bytes v) {
        if (v.size)
            b.insert(b.end(), v.data, v.data + v.size);
    }
};
struct Reader {
    const unsigned char *p;
    explicit Reader(Bytes b) : p(b.data) {}
    uint64_t get(unsigned n) {
        uint64_t x = 0;
        for (unsigned i = 0; i < n; ++i)
            x |= static_cast<uint64_t>(*p++) << (8 * i);
        return x;
    }
    uint32_t u() { return static_cast<uint32_t>(get(4)); }
    Handle handle() {
        Handle h = {u(), u(), u()};
        return h;
    }
    Span span() {
        Span s = {u(), u()};
        return s;
    }
};
bool opcode(uint32_t o) { return (o >= 1 && o <= 61) || (o >= Hello && o <= FileConsumptionAck); }
bool envelopeOpcode(uint32_t o) { return opcode(o) && o != EndOfSample && o != EndOfStream; }
bool resultOpcode(const Header &h) { return envelopeOpcode(h.opcode) ||
    (h.kind==ReverseReply && (h.opcode==EndOfSample || h.opcode==EndOfStream)); }
bool header(const Header &h) {
    return h.magic == Magic && h.version == Version && opcode(h.opcode);
}
template<class W> void putHeader(W &w, const Header &h) {
    w.put(h.magic, 4);
    w.put(h.version, 2);
    w.put(h.kind, 2);
    w.put(h.opcode, 4);
    w.put(h.bytes, 4);
    w.put(h.request, 8);
    w.put(h.causal_request, 8);
    w.put(h.lane, 8);
    w.put(h.lock_lease, 8);
}
struct FixedWriter {
    unsigned char *p;
    explicit FixedWriter(unsigned char *buffer):p(buffer) {}
    void put(uint64_t x,unsigned n) { for(unsigned i=0;i<n;++i){*p++=static_cast<unsigned char>(x & 255);x>>=8;} }
    void handle(const Handle &h){put(h.kind,4);put(h.slot,4);put(h.generation,4);}
    void span(const Span &s){put(s.offset,4);put(s.length,4);}
    void payload(Bytes b){for(size_t i=0;i<b.size;++i)*p++=b.data[i];}
};
template<class W> void callFields(W &w,const Header &h,const Call &c,const Span &sa,const Span &sb,Bytes a,Bytes b) {
    putHeader(w, h);
    w.handle(c.target);
    w.handle(c.resource);
    for (unsigned i = 0; i < 8; ++i)
        w.put(c.value[i], 4);
    w.span(sa);
    w.span(sb);
    w.put(c.output_mask, 4);
    w.put(0, 4);
    w.put(c.callback, 8);
    w.payload(a);
    w.payload(b);
}
template<class W> void resultFields(W &w,const Header &h,const Result &c,const Span &sa,const Span &sb,Bytes a,Bytes b) {
    putHeader(w, h);
    w.put(c.transport_status, 4);
    w.put(c.return_bits, 4);
    w.handle(c.resource);
    for (unsigned i = 0; i < 8; ++i)
        w.put(c.value[i], 4);
    w.span(sa);
    w.span(sb);
    w.put(c.null_mask, 4);
    w.put(c.callback, 8);
    w.payload(a);
    w.payload(b);
}
Header getHeader(Reader &r) {
    Header h;
    h.magic = r.u();
    h.version = static_cast<uint16_t>(r.get(2));
    h.kind = static_cast<uint16_t>(r.get(2));
    h.opcode = r.u();
    h.bytes = r.u();
    h.request = r.get(8);
    h.causal_request = r.get(8);
    h.lane = r.get(8);
    h.lock_lease = r.get(8);
    return h;
}
bool input(Bytes b, size_t minimum) {
    return b.data && b.size >= minimum && b.size <= MaxFrameBytes;
}
bool plan(Header &h, size_t fixed, Bytes a, Bytes b, Span &sa, Span &sb) {
    if ((a.size && !a.data) || (b.size && !b.data) || a.size > MaxFrameBytes - fixed ||
        b.size > MaxFrameBytes - fixed - a.size)
        return false;
    sa.offset = a.size ? static_cast<uint32_t>(fixed) : 0;
    sa.length = static_cast<uint32_t>(a.size);
    sb.offset = b.size ? static_cast<uint32_t>(fixed + a.size) : 0;
    sb.length = static_cast<uint32_t>(b.size);
    h.bytes = static_cast<uint32_t>(fixed + a.size + b.size);
    return true;
}
bool spans(size_t total, size_t fixed, const Span &a, const Span &b) {
    const Span s[2] = {a, b};
    size_t cursor = fixed;
    for (unsigned i = 0; i < 2; ++i) {
        if (!s[i].length) {
            if (s[i].offset)
                return false;
            continue;
        }
        if (s[i].offset != cursor || s[i].length > total - cursor)
            return false;
        cursor += s[i].length;
    }
    return cursor == total;
}
bool eosValid(const Header &h, const Eos &e) {
    return h.kind == Event && e.reserved == 0 && validHandle(e.resource) && e.resource.slot &&
           ((h.opcode == EndOfSample &&
             (e.resource.kind == OwnedSample || e.resource.kind == BorrowedSample)) ||
            (h.opcode == EndOfStream && e.resource.kind == Stream));
}
} // namespace
bool encodeCall(Header h, const Call &c, Bytes a, Bytes b, std::vector<unsigned char> &out) {
    if (!header(h) || !envelopeOpcode(h.opcode) ||
        (h.kind != Request && h.kind != ReverseRequest) || c.reserved || !validHandle(c.target) ||
        !validHandle(c.resource))
        return false;
    Span sa, sb;
    if (!plan(h, 136, a, b, sa, sb))
        return false;
    Writer w;
    callFields(w,h,c,sa,sb,a,b);
    out.swap(w.b);
    return true;
}
bool encodeCallInto(Header h,const Call &c,Bytes a,Bytes b,unsigned char *buffer,size_t capacity,size_t &written) {
    if(!header(h) || !envelopeOpcode(h.opcode) || (h.kind!=Request && h.kind!=ReverseRequest) ||
       c.reserved || !validHandle(c.target) || !validHandle(c.resource))return false;
    Span sa,sb;
    if(!plan(h,136,a,b,sa,sb) || !buffer || capacity<h.bytes)return false;
    FixedWriter w(buffer);callFields(w,h,c,sa,sb,a,b);written=h.bytes;return true;
}
bool decodeCall(Bytes b, Header &outH, Call &outC) {
    if (!input(b, 136))
        return false;
    Reader r(b);
    Header h = getHeader(r);
    if (!header(h) || !envelopeOpcode(h.opcode) || h.bytes != b.size ||
        (h.kind != Request && h.kind != ReverseRequest))
        return false;
    Call c;
    c.target = r.handle();
    c.resource = r.handle();
    for (unsigned i = 0; i < 8; ++i)
        c.value[i] = r.u();
    c.bytes = r.span();
    c.text = r.span();
    c.output_mask = r.u();
    c.reserved = r.u();
    c.callback = r.get(8);
    if (c.reserved || !validHandle(c.target) || !validHandle(c.resource) ||
        !spans(b.size, 136, c.bytes, c.text))
        return false;
    outH = h;
    outC = c;
    return true;
}
bool encodeResult(Header h, const Result &c, Bytes a, Bytes b, std::vector<unsigned char> &out) {
    if (!header(h) || !resultOpcode(h) || (h.kind != Reply && h.kind != ReverseReply) ||
        !validHandle(c.resource))
        return false;
    Span sa, sb;
    if (!plan(h, 128, a, b, sa, sb))
        return false;
    Writer w;
    resultFields(w,h,c,sa,sb,a,b);
    out.swap(w.b);
    return true;
}
bool encodeResultInto(Header h,const Result &c,Bytes a,Bytes b,unsigned char *buffer,size_t capacity,size_t &written) {
    if(!header(h) || !resultOpcode(h) || (h.kind!=Reply && h.kind!=ReverseReply) || !validHandle(c.resource))return false;
    Span sa,sb;
    if(!plan(h,128,a,b,sa,sb) || !buffer || capacity<h.bytes)return false;
    FixedWriter w(buffer);resultFields(w,h,c,sa,sb,a,b);written=h.bytes;return true;
}
bool decodeResult(Bytes b, Header &outH, Result &outC) {
    if (!input(b, 128))
        return false;
    Reader r(b);
    Header h = getHeader(r);
    if (!header(h) || !resultOpcode(h) || h.bytes != b.size ||
        (h.kind != Reply && h.kind != ReverseReply))
        return false;
    Result c;
    c.transport_status = r.u();
    c.return_bits = r.u();
    c.resource = r.handle();
    for (unsigned i = 0; i < 8; ++i)
        c.value[i] = r.u();
    c.bytes = r.span();
    c.text = r.span();
    c.null_mask = r.u();
    c.callback = r.get(8);
    if (!validHandle(c.resource) || !spans(b.size, 128, c.bytes, c.text))
        return false;
    outH = h;
    outC = c;
    return true;
}
bool encodeEos(Header h, const Eos &e, std::vector<unsigned char> &out) {
    if (!header(h) || !eosValid(h, e))
        return false;
    h.bytes = 80;
    Writer w;
    putHeader(w, h);
    w.handle(e.resource);
    w.put(0, 4);
    w.put(e.registration, 8);
    w.put(e.event_sequence, 8);
    out.swap(w.b);
    return true;
}
bool decodeEos(Bytes b, Header &outH, Eos &outE) {
    if (!input(b, 80) || b.size != 80)
        return false;
    Reader r(b);
    Header h = getHeader(r);
    Eos e;
    e.resource = r.handle();
    e.reserved = r.u();
    e.registration = r.get(8);
    e.event_sequence = r.get(8);
    if (!header(h) || h.bytes != 80 || !eosValid(h, e))
        return false;
    outH = h;
    outE = e;
    return true;
}
} // namespace MilesTransport
