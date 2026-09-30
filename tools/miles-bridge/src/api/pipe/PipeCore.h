#ifndef CLIENT_MILES_PRIVATE_PIPE_CORE57_H
#define CLIENT_MILES_PRIVATE_PIPE_CORE57_H
#include "../ClientMiles.h"
#include <stdexcept>
#include <string>
// Private throwing implementation only. Never included by engine/game code.
namespace ClientMilesPipeCore57 {
using ClientMiles::HDIGDRIVER;
using ClientMiles::HSAMPLE;
using ClientMiles::HSTREAM;
using ClientMiles::FileOpenCallback;
using ClientMiles::FileCloseCallback;
using ClientMiles::FileSeekCallback;
using ClientMiles::FileReadCallback;



int32_t file_type(const void *fileImage, uint32_t fileBytes);
int32_t WAV_info(const void *fileImage, ClientMiles::SampleInformation *result);

struct OwnedText {
    bool isNull;
    std::string value;
    OwnedText() : isNull(true) {
    }
};

enum class FailureReason {
    Unsupported,
    InvalidArgument,
    InvalidDriver,
    WrongState,
    InputLimit,
    ResultLimit,
    QueryFailed,
    BackendFailed
};

class Failure : public std::runtime_error {
  public:
    Failure(FailureReason reason, const char *message)
        : std::runtime_error(message), reason_(reason) {
    }
    FailureReason reason() const {
        return reason_;
    }

  private:
    FailureReason reason_;
};

// Calls remain separate; configuration and fallback decisions belong to Audio.
// A zero vendor startup result and a null vendor driver result remain values.
// Backend failures throw Failure instead of fabricating either result.
int32_t startup();
void shutdown();
intptr_t get_preference(uint32_t number);
intptr_t set_preference(uint32_t number, intptr_t value);
const char *last_error();
const char *set_redist_directory(const char *directory);
OwnedText last_errorOwned();
OwnedText set_redist_directoryOwned(const char *directory);

// Explicit adaptation of Audio's bounded 256-byte resource version query.
// Positive capacities up to the temporary paired reply budget; no clamping.
void MSS_version(char *destination, int32_t capacity);

HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags);

// Only the channel-spec output consumed by Audio. This is deliberately named
// differently: no speaker-position pointer or other outputs are promised.
int32_t speaker_configuration_spec(HDIGDRIVER driver);

// The current pipe implementation supports get1/get42 and set42=16/64.
// Driver arguments retain their native scalar widths; only one live driver is owned.
// Preference width follows the native pointer width; the pipe checks narrowing.
// Driver identities are opaque, local to the selected implementation and valid
// only through that startup lifetime. Typed EOS delivery remains unimplemented.

void set_file_callbacks(FileOpenCallback open, FileCloseCallback close,
                        FileSeekCallback seek, FileReadCallback read);
void set_listener_3D_position(HDIGDRIVER driver, float x, float y, float z);
void set_listener_3D_velocity_vector(HDIGDRIVER driver, float x, float y, float z);
void set_listener_3D_orientation(HDIGDRIVER driver, float faceX, float faceY, float faceZ,
                                 float upX, float upY, float upZ);
void set_3D_rolloff_factor(HDIGDRIVER driver, float factor);
void serve();
void lock();
void unlock();
int32_t active_sample_count(HDIGDRIVER driver);
int32_t digital_CPU_percent(HDIGDRIVER driver);
int32_t digital_latency(HDIGDRIVER driver);
uint32_t get_timer_highest_delay();
int32_t file_error();
int32_t room_type(HDIGDRIVER driver);
void set_room_type(HDIGDRIVER driver, int32_t room);


// Common sample identity. Copies alias; they never transfer ownership.
// allocate_sample_handle owns; stream_sample_handle borrows from its live stream.


// Nonnull allocation is owned: release exactly once before driver teardown.
HSAMPLE allocate_sample_handle(HDIGDRIVER driver);
int32_t set_named_sample_file(HSAMPLE sample, const char *suffix,
                             const void *fileImage, uint32_t fileBytes, int32_t block);
int32_t set_sample_file(HSAMPLE sample, const void *fileImage, int32_t block);
void sample_ms_position(HSAMPLE sample, int32_t *totalMilliseconds,
                        int32_t *currentMilliseconds);
void end_sample(HSAMPLE sample);
// Owned allocation only; never release a borrowed stream sample.
void release_sample_handle(HSAMPLE sample);

// Allocation null and named-file scalar returns stay actual SDK values. Output
// pointers preserve nullability and millisecond units. The host retains each
// image/suffix until successful replacement or genuine release; failed binds
// retain both old and attempted inputs because native failure is not rollback.
// The size-less setter requires a matching private ScopedSourceImage extent.
// Binding, millisecond query and end in this subset require an owned allocation.
// The common type also supports the explicitly documented shared stream controls.


// These controls use the common HSAMPLE type. Owned samples remain valid until
// release. Borrowed stream samples remain valid only while their parent is live.
// Borrowed use in this subset: set_sample_volume_levels, sample_volume_levels,
// set_sample_reverb_levels, set_sample_playback_rate and sample_playback_rate.
// Other sample controls here require owned samples. EOS is not implemented here.
void start_sample(HSAMPLE sample);
void stop_sample(HSAMPLE sample);
uint32_t sample_status(HSAMPLE sample);
void set_sample_loop_count(HSAMPLE sample, int32_t count);
void set_sample_loop_block(HSAMPLE sample, int32_t startByte, int32_t endByte);
void set_sample_ms_position(HSAMPLE sample, int32_t milliseconds);
void set_sample_position(HSAMPLE sample, uint32_t byteOffset);
uint32_t sample_position(HSAMPLE sample);
void set_sample_playback_rate(HSAMPLE sample, int32_t rate);
int32_t sample_playback_rate(HSAMPLE sample);
void set_sample_volume_levels(HSAMPLE sample, float left, float right);
void sample_volume_levels(HSAMPLE sample, float * left, float * right);
void set_sample_reverb_levels(HSAMPLE sample, float dry, float wet);
void sample_reverb_levels(HSAMPLE sample, float * dry, float * wet);
void set_sample_3D_position(HSAMPLE sample, float x, float y, float z);
void set_sample_3D_velocity_vector(HSAMPLE sample, float xPerMs, float yPerMs, float zPerMs);
void set_sample_3D_distances(HSAMPLE sample, float maximum, float minimum, int32_t autoWetAttenuation);
void set_sample_occlusion(HSAMPLE sample, float value);
void set_sample_obstruction(HSAMPLE sample, float value);
// Byte positions/loop offsets and millisecond positions remain distinct SDK units.
// Getter output pointers pass through unchanged, including null; this facade adds
// no pointer-validity guarantees beyond the actual SDK contract. Stop, end and
// release stay separate. No clamping, scheduling, range narrowing or replay.


// Owned by the caller after a nonnull open; close once before driver teardown.
// Copies alias one stream. This is an opaque identity, not an RAII owner.


HSTREAM open_stream(HDIGDRIVER driver, const char *filename, int32_t streamMem);
// Closing invalidates every borrowed sample obtained from this stream.
void close_stream(HSTREAM stream);
// Borrowed: do not release_sample_handle; invalid after parent close/shutdown.
HSAMPLE stream_sample_handle(HSTREAM stream);
void start_stream(HSTREAM stream);
void set_stream_loop_count(HSTREAM stream, int32_t count);
void set_stream_loop_block(HSTREAM stream, int32_t loopStartOffset, int32_t loopEndOffset);
int32_t stream_status(HSTREAM stream);
void set_stream_ms_position(HSTREAM stream, int32_t milliseconds);
void stream_ms_position(HSTREAM stream, int32_t *totalMilliseconds,
                        int32_t *currentMilliseconds);

// Signed SDK values, native offsets and nullable output pointers pass unchanged.
// Use the existing shared sample control names for borrowed samples; see
// the plain public surface for this subset. EOS delivery remains unimplemented.

}
#endif
