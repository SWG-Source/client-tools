// Experimental reviewed shape only. No transport or vendor implementation.
#ifndef EXPERIMENTAL_MILES_WIRE_H
#define EXPERIMENTAL_MILES_WIRE_H
#include <stdint.h>
#include <stddef.h>
namespace MilesWire {
// Private paired-endpoint version4; unrelated to the SWG network protocol.
// Encode/decode fields explicitly little-endian; these structs are schema/layout
// checks, NOT permission to send native padding or dereference received bytes.
enum { Magic=0x31534d57, Version=4, MaxFrameBytes=1048576 };
enum Kind { Request=1, Reply=2, Event=3, ReverseRequest=4, ReverseReply=5 };
enum ResourceKind { Null=0, Driver=1, OwnedSample=2, Stream=3, BorrowedSample=4, Buffer=5, File=6, Video=7 };
struct Handle { uint32_t kind, slot, generation; }; // all zero is vendor null
struct Span { uint32_t offset, length; }; // offset from frame start; checked subtraction
struct Header { uint32_t magic; uint16_t version, kind; uint32_t opcode, bytes;
 uint64_t request, causal_request, lane, lock_lease; };
// value[] positions are defined in API-MAP.md; floats retain raw uint32 bits.
// Resource buffer transfers are explicit and chunked, no native input pointers.
struct Call { Handle target, resource; uint32_t value[8]; Span bytes, text;
 uint32_t output_mask, reserved; uint64_t callback; };
struct Result { uint32_t transport_status, return_bits; Handle resource;
 uint32_t value[8]; Span bytes, text; uint32_t null_mask; uint64_t callback; };
struct SoundInfo { int32_t format; uint32_t data_offset, data_length, rate;
 int32_t bits, channels; uint32_t channel_mask, samples, block_size, initial_offset, null_mask; };
struct Eos { Handle resource; uint32_t reserved; uint64_t registration, event_sequence; };
// BufferBegin value0=totalbytes; BufferChunk target=buffer,value0=offset,bytes=chunk;
// BufferSeal validates exact complete coverage; BufferRelease retires after users.
// SessionVersion is host-side MSS_version macro query, not a forwarded export.
enum Control { Hello=0x1000, BufferBegin, BufferChunk, BufferSeal, BufferRelease,
 SessionVersion, FileOpen, FileClose, FileSeek, FileRead, EndOfSample, EndOfStream,
 CallbackAck, SessionClose, FileConsumptionAck=0x100e };
static_assert(CallbackAck==0x100c && SessionClose==0x100d && FileConsumptionAck==0x100e,
              "append-only private control opcodes");
// Private Bink controls share the paired command transport, not the SWG protocol.
enum BinkControl { BinkInitialize=0x2000, BinkOpen, BinkClose, BinkInfo,
 BinkDoFrame, BinkNextFrame, BinkWait, BinkShouldSkip, BinkService, BinkPause,
 BinkVideoOnOff, BinkSoundOnOff, BinkVolume, BinkPixelsBegin, BinkPixelsChunk,
 BinkLastError, BinkShutdown };
static_assert(BinkShutdown==0x2010, "Bink control sequence");
enum Opcode {
 AIL_WAV_info = 1,
 AIL_active_sample_count = 2,
 AIL_allocate_sample_handle = 3,
 AIL_close_stream = 4,
 AIL_digital_CPU_percent = 5,
 AIL_digital_latency = 6,
 AIL_end_sample = 7,
 AIL_file_error = 8,
 AIL_file_type = 9,
 AIL_get_preference = 10,
 AIL_get_timer_highest_delay = 11,
 AIL_last_error = 12,
 AIL_lock = 13,
 AIL_open_digital_driver = 14,
 AIL_open_stream = 15,
 AIL_register_EOS_callback = 16,
 AIL_register_stream_callback = 17,
 AIL_release_sample_handle = 18,
 AIL_room_type = 19,
 AIL_sample_ms_position = 20,
 AIL_sample_playback_rate = 21,
 AIL_sample_position = 22,
 AIL_sample_reverb_levels = 23,
 AIL_sample_status = 24,
 AIL_sample_volume_levels = 25,
 AIL_serve = 26,
 AIL_set_3D_rolloff_factor = 27,
 AIL_set_file_callbacks = 28,
 AIL_set_listener_3D_orientation = 29,
 AIL_set_listener_3D_position = 30,
 AIL_set_listener_3D_velocity_vector = 31,
 AIL_set_named_sample_file = 32,
 AIL_set_preference = 33,
 AIL_set_redist_directory = 34,
 AIL_set_room_type = 35,
 AIL_set_sample_3D_distances = 36,
 AIL_set_sample_3D_position = 37,
 AIL_set_sample_3D_velocity_vector = 38,
 AIL_set_sample_file = 39,
 AIL_set_sample_loop_block = 40,
 AIL_set_sample_loop_count = 41,
 AIL_set_sample_ms_position = 42,
 AIL_set_sample_obstruction = 43,
 AIL_set_sample_occlusion = 44,
 AIL_set_sample_playback_rate = 45,
 AIL_set_sample_position = 46,
 AIL_set_sample_reverb_levels = 47,
 AIL_set_sample_volume_levels = 48,
 AIL_set_stream_loop_block = 49,
 AIL_set_stream_loop_count = 50,
 AIL_set_stream_ms_position = 51,
 AIL_shutdown = 52,
 AIL_speaker_configuration = 53,
 AIL_start_sample = 54,
 AIL_start_stream = 55,
 AIL_startup = 56,
 AIL_stop_sample = 57,
 AIL_stream_ms_position = 58,
 AIL_stream_sample_handle = 59,
 AIL_stream_status = 60,
 AIL_unlock = 61
};
static_assert(sizeof(Handle)==12,"handle");
static_assert(sizeof(Header)==48 && offsetof(Header,request)==16,"header");
static_assert(sizeof(Call)==88 && offsetof(Call,callback)==80,"call");
static_assert(sizeof(Result)==80 && offsetof(Result,callback)==72,"result");
static_assert(sizeof(SoundInfo)==44,"soundinfo");
static_assert(sizeof(Eos)==32 && offsetof(Eos,registration)==16,"eos");
static_assert(sizeof(float)==4,"float storage");
}
#endif
