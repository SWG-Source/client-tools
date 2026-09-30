#ifndef STARTUP_BRIDGE23_REPLY_H
#define STARTUP_BRIDGE23_REPLY_H
#include "../version/session_version.h"
#include "../metadata/metadata.h"
#include <cstring>
#include <stdexcept>
namespace StartupBridge {
enum Status {
    Success = 0,
    Unsupported = 1,
    InvalidResource = 2,
    InvalidFields = 3,
    LifecycleRefused = 0x1001,
    TextTooLong = 0x1002,
    InputBudgetExceeded = 0x1003,
    VersionQueryFailed = 0x1004
};
inline Status mapMetadata(MilesStartup::Status s) {
    switch (s) {
    case MilesStartup::Complete:
        return Success;
    case MilesStartup::Unsupported:
        return Unsupported;
    case MilesStartup::InvalidResource:
        return InvalidResource;
    case MilesStartup::InvalidFields:
        return InvalidFields;
    case MilesStartup::TextTooLong:
        return TextTooLong;
    case MilesStartup::InputBudgetExceeded:
        return InputBudgetExceeded;
    }
    throw std::runtime_error("unmapped metadata status");
}
inline bool knownStatus(uint32_t s) {
    return s <= InvalidFields || (s >= LifecycleRefused && s <= VersionQueryFailed);
}
inline bool metadata(uint32_t op) {
    return op == MilesWire::AIL_set_redist_directory || op == MilesWire::AIL_get_preference ||
           op == MilesWire::AIL_set_preference || op == MilesWire::AIL_last_error ||
           op == MilesWire::AIL_speaker_configuration;
}
// Scalar result fields plus owned text. Span offsets are cleared after decode;
// callers never need a hidden retained frame or a borrowed text pointer.
struct OwnedReply {
    MilesWire::Result result;
    std::vector<unsigned char> text;
    std::vector<unsigned char> bytes; // Version write prefix; never caller-buffer tail.
    OwnedReply() {
        std::memset(&result, 0, sizeof result);
    }
};
inline bool nullHandle(const MilesWire::Handle &h) {
    return !h.kind && !h.slot && !h.generation;
}
inline bool decodeReply(MilesTransport::Bytes frame, const MilesWire::Header &expected,
                        OwnedReply &out) {
    MilesWire::Header h = {};
    OwnedReply candidate;
    if (!MilesTransport::decodeResult(frame, h, candidate.result))
        return false;
    MilesWire::Result &r = candidate.result;
    if (h.kind != MilesWire::Reply || h.opcode != expected.opcode ||
        h.request != expected.request || h.lane != expected.lane ||
        h.causal_request != expected.causal_request || h.lock_lease != expected.lock_lease ||
        !knownStatus(r.transport_status))
        return false;
    const bool ok = r.transport_status == Success;
    const bool sampleRegistration=ok&&h.opcode==MilesWire::AIL_register_EOS_callback;
    const bool streamRegistration=ok&&h.opcode==MilesWire::AIL_register_stream_callback;
    if(r.callback && ((!sampleRegistration&&!streamRegistration) ||
        (sampleRegistration?r.callback>64:(r.callback<65||r.callback>128))))return false;
    const bool textOp = ok && (h.opcode == MilesWire::AIL_set_redist_directory ||
                               h.opcode == MilesWire::AIL_last_error);
    const bool versionOp = ok && h.opcode == MilesWire::SessionVersion;
    const bool wavValues=ok && h.opcode==MilesWire::AIL_WAV_info && r.return_bits!=0;
    const bool valueOp = ok && h.opcode == MilesWire::AIL_speaker_configuration;
    const bool sampleTime = ok && (h.opcode == MilesWire::AIL_sample_ms_position ||
        h.opcode == MilesWire::AIL_stream_ms_position || h.opcode == MilesWire::AIL_sample_volume_levels || h.opcode == MilesWire::AIL_sample_reverb_levels);
    const bool returnOp =
        ok && (h.opcode == MilesWire::AIL_startup || h.opcode == MilesWire::AIL_get_preference ||
               h.opcode == MilesWire::AIL_set_preference || h.opcode == MilesWire::AIL_room_type ||
               h.opcode == MilesWire::AIL_sample_status || h.opcode == MilesWire::AIL_sample_position ||
               h.opcode == MilesWire::AIL_sample_playback_rate || h.opcode == MilesWire::AIL_stream_status ||
               h.opcode == MilesWire::AIL_active_sample_count ||
               h.opcode == MilesWire::AIL_digital_CPU_percent ||
               h.opcode == MilesWire::AIL_digital_latency ||
               h.opcode == MilesWire::AIL_get_timer_highest_delay ||
               h.opcode == MilesWire::AIL_file_error || h.opcode == MilesWire::AIL_file_type ||
               h.opcode == MilesWire::AIL_WAV_info ||
               h.opcode == MilesWire::AIL_set_sample_file || h.opcode == MilesWire::AIL_set_named_sample_file);
    const bool sampleAllocation = ok && h.opcode == MilesWire::AIL_allocate_sample_handle;
    const bool aliasOp=ok && h.opcode==MilesWire::AIL_stream_sample_handle;
    const bool streamOpen=ok && h.opcode==MilesWire::AIL_open_stream;
    const bool bufferBegin=ok && h.opcode==MilesWire::BufferBegin;
    const bool resourceOp = ok && (h.opcode == MilesWire::AIL_open_digital_driver || sampleAllocation || streamOpen || aliasOp || bufferBegin);
    const uint32_t expectedKind=static_cast<uint32_t>(bufferBegin ? MilesWire::Buffer : aliasOp ? MilesWire::BorrowedSample : streamOpen ? MilesWire::Stream : sampleAllocation ? MilesWire::OwnedSample : MilesWire::Driver);
    if ((!returnOp && r.return_bits) || (!resourceOp && !nullHandle(r.resource)))return false;
    if (bufferBegin && (nullHandle(r.resource) || !r.resource.slot || !r.resource.generation))return false;
    if (resourceOp && !nullHandle(r.resource) && r.resource.kind!=expectedKind)return false;
    for (unsigned i = 0; i < 8; ++i)
        if (!(valueOp && i == 3) && !(sampleTime && i < 2) && !(aliasOp && i < 3) && !(wavValues && i < 7) && r.value[i])
            return false;
    if (!versionOp && r.bytes.length) return false;
    if (versionOp) {
        if (r.text.length || r.null_mask || !r.bytes.length ||
            r.bytes.length > MilesSessionVersion::CapacityLimit) return false;
        const unsigned char *prefix = frame.data + r.bytes.offset;
        if (prefix[r.bytes.length-1]) return false;
        candidate.bytes.assign(prefix, prefix+r.bytes.length);
    } else if (textOp) {
        if (r.null_mask & ~MilesStartup::TextNull)
            return false;
        if (r.null_mask == MilesStartup::TextNull) {
            if (r.text.length)
                return false;
        } else {
            if (!r.text.length || r.text.length > MilesStartup::ReplyTextLimit)
                return false;
            const unsigned char *text = frame.data + r.text.offset;
            if (text[r.text.length - 1] || std::memchr(text, 0, r.text.length - 1))
                return false;
            candidate.text.assign(text, text + r.text.length);
        }
    } else if (r.text.length || r.null_mask)
        return false;
    r.text = MilesWire::Span();
    r.bytes = MilesWire::Span();
    out.text.swap(candidate.text);
    out.bytes.swap(candidate.bytes);
    out.result = r;
    return true;
}
} // namespace StartupBridge
#endif
