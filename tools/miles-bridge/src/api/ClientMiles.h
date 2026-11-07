#ifndef CLIENT_MILES_PLAIN_SURFACE70_H
#define CLIENT_MILES_PLAIN_SURFACE70_H

// Source-only proposal for the game/private-adapter boundary.
// Selected Windows SDK opaque tag names only; no vendor bodies or SDK includes.
// This nominal-type revision requires a consistent rebuild and one backend.
#include <stdint.h>
#if !defined(_WIN32) || !defined(_MSC_VER)
#error This candidate declares Windows MSVC callback calling conventions.
#endif

// Deliberate selected-SDK coupling, including its reserved global tag names.
struct _DIG_DRIVER;
struct _SAMPLE;
struct _STREAM;
namespace ClientMiles {
typedef ::_DIG_DRIVER *HDIGDRIVER;
typedef ::_SAMPLE *HSAMPLE;
typedef ::_STREAM *HSTREAM;
typedef uintptr_t FileHandle;

// Native-shaped local callbacks. FileHandle may be zero on successful open.
// Exact native sample/stream callback types; callers keep callbacks nonthrowing.
typedef uint32_t (__stdcall *FileOpenCallback)(const char *, FileHandle *);
typedef void (__stdcall *FileCloseCallback)(FileHandle);
typedef int32_t (__stdcall *FileSeekCallback)(FileHandle, int32_t, uint32_t);
typedef uint32_t (__stdcall *FileReadCallback)(FileHandle, void *, uint32_t);
typedef void (__stdcall *SampleCallback)(HSAMPLE);
typedef void (__stdcall *StreamCallback)(HSTREAM);

// Only scalar values consumed by the reviewed Audio source. Private delegates
// must assert these values against their selected SDK before adoption.
enum Preference {
    MixerChannels = 1,
    MixFragmentCount = 42
};
enum SpeakerConfiguration {
    StereoSpeakerConfiguration = 2,
    SystemSpeakerConfiguration = 0x10,
    HeadphoneSpeakerConfiguration = 0x20,
    DolbySurroundSpeakerConfiguration = 0x30,
    Discrete40SpeakerConfiguration = 0x50,
    Discrete51SpeakerConfiguration = 0x60,
    Discrete61SpeakerConfiguration = 0x70,
    Discrete71SpeakerConfiguration = 0x80,
    Discrete81SpeakerConfiguration = 0x90
};
enum SampleState {
    SampleDone = 0x0002,
    SamplePlaying = 0x0004
};
enum FileType {
    FileUnknown = 0, FilePcmWav = 1, FileAdpcmWav = 2, FileOtherWav = 3,
    FileVoc = 4, FileMidi = 5, FileXmidi = 6, FileXmidiDls = 7,
    FileXmidiMls = 8, FileDls = 9, FileMls = 10, FileMpegLayer1 = 11,
    FileMpegLayer2 = 12, FileMpegLayer3 = 13, FileOtherAsiWav = 14
};
enum FileSeekOrigin {
    FileSeekBegin = 0, FileSeekCurrent = 1, FileSeekEnd = 2
};
enum FileError {
    FileNoError = 0, FileIoError = 1, FileOutOfMemory = 2, FileNotFound = 3,
    FileCannotWrite = 4, FileCannotRead = 5, FileDiskFull = 6
};
enum RoomType {
    RoomGeneric = 0, RoomPaddedCell = 1, RoomNormal = 2, RoomBathroom = 3,
    RoomLivingRoom = 4, RoomStoneRoom = 5, RoomAuditorium = 6,
    RoomConcertHall = 7, RoomCave = 8, RoomArena = 9, RoomHangar = 10,
    RoomCarpetedHallway = 11, RoomHallway = 12, RoomStoneCorridor = 13,
    RoomAlley = 14, RoomForest = 15, RoomCity = 16, RoomMountains = 17,
    RoomQuarry = 18, RoomPlain = 19, RoomParkingLot = 20,
    RoomSewerPipe = 21, RoomUnderwater = 22, RoomDrugged = 23,
    RoomDizzy = 24, RoomPsychotic = 25
};

// Native values remain values. Private adapter failures must not fabricate null,
// zero, EOF or empty text, and must not throw modern-STL exceptions into engine TUs.
// One private implementation is selected for the entire startup lifetime.
int32_t startup();
void shutdown();
intptr_t get_preference(uint32_t number);
intptr_t set_preference(uint32_t number, intptr_t value);

// Proposed adapter-owned snapshots, not vendor-pointer lifetime promises.
// Null remains null; empty remains a nonnull empty string. A nonnull pointer is
// valid until the next call to this SAME function or completed shutdown. Copy it
// before that point; synchronize concurrent consumers/calls as required by that
// lifetime. Callers neither write nor free it. Two functions have separate storage.
const char *last_error();
const char *set_redist_directory(const char *directory);

// Explicit source adaptation of the Windows resource-query macro, not an SDK
// export. destination must cover capacity positive bytes; Audio uses 256.
// The direct implementation invokes the actual selected SDK macro. No fabricated
// scalar status, nonempty guarantee, or extra success/failure interpretation.
void MSS_version(char *destination, int32_t capacity);

HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags);
// Only the speaker-spec output consumed by Audio, retaining the current adaptation.
int32_t speaker_configuration_spec(HDIGDRIVER driver);
void set_file_callbacks(FileOpenCallback, FileCloseCallback, FileSeekCallback, FileReadCallback);
void set_listener_3D_position(HDIGDRIVER, float x, float y, float z);
void set_listener_3D_velocity_vector(HDIGDRIVER, float x, float y, float z);
void set_listener_3D_orientation(HDIGDRIVER, float faceX, float faceY, float faceZ,
                               float upX, float upY, float upZ);
void set_3D_rolloff_factor(HDIGDRIVER, float factor);
void serve();
void lock();
void unlock();
int32_t room_type(HDIGDRIVER);
void set_room_type(HDIGDRIVER, int32_t room);
int32_t digital_CPU_percent(HDIGDRIVER);
int32_t digital_latency(HDIGDRIVER);
uint32_t get_timer_highest_delay();
// Debug/diagnostic use in the reviewed Audio source; not an inferred release path.
int32_t active_sample_count(HDIGDRIVER);

HSAMPLE allocate_sample_handle(HDIGDRIVER);
int32_t set_named_sample_file(HSAMPLE, const char *suffix,
                              const void *fileImage, uint32_t fileBytes, int32_t block);
int32_t set_sample_file(HSAMPLE, const void *fileImage, int32_t block);
void release_sample_handle(HSAMPLE);
void start_sample(HSAMPLE);
void stop_sample(HSAMPLE);
void end_sample(HSAMPLE);
uint32_t sample_status(HSAMPLE);
void sample_ms_position(HSAMPLE, int32_t *totalMilliseconds, int32_t *currentMilliseconds);
void set_sample_ms_position(HSAMPLE, int32_t milliseconds);
void set_sample_position(HSAMPLE, uint32_t byteOffset);
uint32_t sample_position(HSAMPLE);
void set_sample_loop_count(HSAMPLE, int32_t count);
void set_sample_loop_block(HSAMPLE, int32_t startByte, int32_t endByte);
void set_sample_3D_position(HSAMPLE, float x, float y, float z);
void set_sample_3D_velocity_vector(HSAMPLE, float xPerMs, float yPerMs, float zPerMs);
void set_sample_3D_distances(HSAMPLE, float maximum, float minimum, int32_t autoWetAttenuation);
void set_sample_occlusion(HSAMPLE, float value);
void set_sample_obstruction(HSAMPLE, float value);
void sample_reverb_levels(HSAMPLE, float *dry, float *wet);

// The same five names accept an owned sample or a borrowed sample from a live
// stream. Other sample operations above require owned samples in this subset.
void set_sample_volume_levels(HSAMPLE, float left, float right);
void sample_volume_levels(HSAMPLE, float *left, float *right);
void set_sample_reverb_levels(HSAMPLE, float dry, float wet);
void set_sample_playback_rate(HSAMPLE, int32_t rate);
int32_t sample_playback_rate(HSAMPLE);

// Callback code/state must outlive the native callback registration and actual
// producer quiescence. Return the actual prior native callback; null is not a wire ID.
SampleCallback register_EOS_callback(HSAMPLE, SampleCallback);
StreamCallback register_stream_callback(HSTREAM, StreamCallback);

HSTREAM open_stream(HDIGDRIVER, const char *filename, int32_t streamMem);
void close_stream(HSTREAM);
HSAMPLE stream_sample_handle(HSTREAM);
void start_stream(HSTREAM);
void set_stream_loop_count(HSTREAM, int32_t count);
void set_stream_loop_block(HSTREAM, int32_t startOffset, int32_t endOffset);
int32_t stream_status(HSTREAM);
void set_stream_ms_position(HSTREAM, int32_t milliseconds);
void stream_ms_position(HSTREAM, int32_t *totalMilliseconds, int32_t *currentMilliseconds);

int32_t file_type(const void *fileImage, uint32_t fileBytes);
// Defined in an otherwise uncalled Audio helper; no runtime liveness claimed.
int32_t file_error();

// Deliberate projection of only the metadata Audio reads. This is not the SDK
// structure or a wire layout; no vendor pointers, offsets or ownership escape.
struct SampleInformation {
    int32_t format, bits, channels;
    uint32_t dataLength, rate, samples, blockSize;
};
// result is nonnull. Native return bits are preserved; populate these fields only
// on a nonzero native result, leaving result unchanged on ordinary failure.
int32_t WAV_info(const void *fileImage, SampleInformation *result);

// Sample/file-image ownership, units and nullable getter outputs remain as in
// the reviewed38 surface. No callback scheduling or allocation policy is added.
} // namespace ClientMiles
#endif
