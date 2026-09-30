#include "../ClientMiles.h"
#include "PipeCore.h"
#include "../../failure/failure_boundary.h"
#include "../../callback-guard/invocation_guard.h"
#include <exception>

namespace {
namespace Boundary = ClientMilesPrivate52;
namespace Core = ClientMilesPipeCore57;
template<class Result, class Action> Result guarded(Action action) {
    try {
        Boundary::requireFatalReporter();
        // Before selected Session access, any request or local proxy mutation.
        MilesCallbackGuard47::requireForwardAllowed();
        return action();
    } catch (const std::exception &error) {
        Boundary::fail(Boundary::PrivateException,error.what());
    } catch (...) {
        Boundary::fail(Boundary::UnknownException,"exception in private Miles pipe adapter");
    }
}
}
// Partial plain surface: 58 guarded definitions backed by this selected Core.
// EOS registration and both sample-file setters remain unimplemented.
namespace ClientMiles {
int32_t WAV_info(const void *fileImage,SampleInformation *result) {
    return guarded<int32_t>([=]() -> int32_t { return Core::WAV_info(fileImage,result); });
}
void MSS_version(char *destination, int32_t capacity) {
    guarded<void>([=]() -> void { Core::MSS_version(destination, capacity); });
}
int32_t file_type(const void *fileImage,uint32_t fileBytes) {
    return guarded<int32_t>([=]() -> int32_t { return Core::file_type(fileImage,fileBytes); });
}
const char *last_error() {
    return guarded<const char *>([]() -> const char * { return Core::last_error(); });
}
const char *set_redist_directory(const char *directory) {
    return guarded<const char *>([=]() -> const char * { return Core::set_redist_directory(directory); });
}
int32_t startup() {
    return guarded<int32_t>([=]() -> int32_t { return Core::startup(); });
}
void shutdown() {
    guarded<void>([=]() -> void { Core::shutdown(); });
}
intptr_t get_preference(uint32_t number) {
    return guarded<intptr_t>([=]() -> intptr_t { return Core::get_preference(number); });
}
intptr_t set_preference(uint32_t number, intptr_t value) {
    return guarded<intptr_t>([=]() -> intptr_t { return Core::set_preference(number, value); });
}
HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags) {
    return guarded<HDIGDRIVER>([=]() -> HDIGDRIVER { return Core::open_digital_driver(frequency, bits, channels, flags); });
}
int32_t speaker_configuration_spec(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Core::speaker_configuration_spec(driver); });
}
// Current pipe composition supports one nonnull table after startup with a
// private-root lifetime pin. Core rejects null/replacement; no callback substitution.
void set_file_callbacks(FileOpenCallback open, FileCloseCallback close,
                        FileSeekCallback seek, FileReadCallback read) {
    guarded<void>([=]() -> void { Core::set_file_callbacks(open, close, seek, read); });
}
void set_listener_3D_position(HDIGDRIVER driver, float x, float y, float z) {
    guarded<void>([=]() -> void { Core::set_listener_3D_position(driver, x, y, z); });
}
void set_listener_3D_velocity_vector(HDIGDRIVER driver, float x, float y, float z) {
    guarded<void>([=]() -> void { Core::set_listener_3D_velocity_vector(driver, x, y, z); });
}
void set_listener_3D_orientation(HDIGDRIVER driver, float x, float y, float z, float upX, float upY, float upZ) {
    guarded<void>([=]() -> void { Core::set_listener_3D_orientation(driver, x, y, z, upX, upY, upZ); });
}
void set_3D_rolloff_factor(HDIGDRIVER driver, float factor) {
    guarded<void>([=]() -> void { Core::set_3D_rolloff_factor(driver, factor); });
}
void lock() {
    guarded<void>([]() -> void { Core::lock(); });
}
void unlock() {
    guarded<void>([]() -> void { Core::unlock(); });
}
void serve() {
    guarded<void>([=]() -> void { Core::serve(); });
}
int32_t active_sample_count(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Core::active_sample_count(driver); });
}
int32_t digital_CPU_percent(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Core::digital_CPU_percent(driver); });
}
int32_t digital_latency(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Core::digital_latency(driver); });
}
uint32_t get_timer_highest_delay() {
    return guarded<uint32_t>([=]() -> uint32_t { return Core::get_timer_highest_delay(); });
}
int32_t file_error() {
    return guarded<int32_t>([=]() -> int32_t { return Core::file_error(); });
}
int32_t room_type(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Core::room_type(driver); });
}
void set_room_type(HDIGDRIVER driver, int32_t room) {
    guarded<void>([=]() -> void { Core::set_room_type(driver, room); });
}
HSTREAM open_stream(HDIGDRIVER driver,const char *filename,int32_t streamMem) {
    return guarded<HSTREAM>([=]() -> HSTREAM { return Core::open_stream(driver, filename, streamMem); });
}
void close_stream(HSTREAM stream) {
    guarded<void>([=]() -> void { Core::close_stream(stream); });
}
HSAMPLE stream_sample_handle(HSTREAM stream) {
    return guarded<HSAMPLE>([=]() -> HSAMPLE { return Core::stream_sample_handle(stream); });
}
void start_stream(HSTREAM stream) {
    guarded<void>([=]() -> void { Core::start_stream(stream); });
}
void set_stream_loop_count(HSTREAM stream,int32_t count) {
    guarded<void>([=]() -> void { Core::set_stream_loop_count(stream, count); });
}
void set_stream_loop_block(HSTREAM stream,int32_t first,int32_t last) {
    guarded<void>([=]() -> void { Core::set_stream_loop_block(stream, first, last); });
}
int32_t stream_status(HSTREAM stream) {
    return guarded<int32_t>([=]() -> int32_t { return Core::stream_status(stream); });
}
void set_stream_ms_position(HSTREAM stream,int32_t value) {
    guarded<void>([=]() -> void { Core::set_stream_ms_position(stream, value); });
}
void stream_ms_position(HSTREAM stream,int32_t *total,int32_t *current) {
    guarded<void>([=]() -> void { Core::stream_ms_position(stream, total, current); });
}
HSAMPLE allocate_sample_handle(HDIGDRIVER driver) {
    return guarded<HSAMPLE>([=]() -> HSAMPLE { return Core::allocate_sample_handle(driver); });
}
void sample_ms_position(HSAMPLE sample, int32_t *total, int32_t *current) {
    guarded<void>([=]() -> void { Core::sample_ms_position(sample, total, current); });
}
void end_sample(HSAMPLE sample) {
    guarded<void>([=]() -> void { Core::end_sample(sample); });
}
void release_sample_handle(HSAMPLE sample) {
    guarded<void>([=]() -> void { Core::release_sample_handle(sample); });
}
void start_sample(HSAMPLE sample) {
    guarded<void>([=]() -> void { Core::start_sample(sample); });
}
void stop_sample(HSAMPLE sample) {
    guarded<void>([=]() -> void { Core::stop_sample(sample); });
}
uint32_t sample_status(HSAMPLE sample) {
    return guarded<uint32_t>([=]() -> uint32_t { return Core::sample_status(sample); });
}
void set_sample_loop_count(HSAMPLE sample, int32_t count) {
    guarded<void>([=]() -> void { Core::set_sample_loop_count(sample, count); });
}
void set_sample_loop_block(HSAMPLE sample, int32_t startByte, int32_t endByte) {
    guarded<void>([=]() -> void { Core::set_sample_loop_block(sample, startByte, endByte); });
}
void set_sample_ms_position(HSAMPLE sample, int32_t milliseconds) {
    guarded<void>([=]() -> void { Core::set_sample_ms_position(sample, milliseconds); });
}
void set_sample_position(HSAMPLE sample, uint32_t byteOffset) {
    guarded<void>([=]() -> void { Core::set_sample_position(sample, byteOffset); });
}
uint32_t sample_position(HSAMPLE sample) {
    return guarded<uint32_t>([=]() -> uint32_t { return Core::sample_position(sample); });
}
void set_sample_playback_rate(HSAMPLE sample, int32_t rate) {
    guarded<void>([=]() -> void { Core::set_sample_playback_rate(sample, rate); });
}
int32_t sample_playback_rate(HSAMPLE sample) {
    return guarded<int32_t>([=]() -> int32_t { return Core::sample_playback_rate(sample); });
}
void set_sample_volume_levels(HSAMPLE sample, float left, float right) {
    guarded<void>([=]() -> void { Core::set_sample_volume_levels(sample, left, right); });
}
void sample_volume_levels(HSAMPLE sample, float * left, float * right) {
    guarded<void>([=]() -> void { Core::sample_volume_levels(sample, left, right); });
}
void set_sample_reverb_levels(HSAMPLE sample, float dry, float wet) {
    guarded<void>([=]() -> void { Core::set_sample_reverb_levels(sample, dry, wet); });
}
void sample_reverb_levels(HSAMPLE sample, float * dry, float * wet) {
    guarded<void>([=]() -> void { Core::sample_reverb_levels(sample, dry, wet); });
}
void set_sample_3D_position(HSAMPLE sample, float x, float y, float z) {
    guarded<void>([=]() -> void { Core::set_sample_3D_position(sample, x, y, z); });
}
void set_sample_3D_velocity_vector(HSAMPLE sample, float xPerMs, float yPerMs, float zPerMs) {
    guarded<void>([=]() -> void { Core::set_sample_3D_velocity_vector(sample, xPerMs, yPerMs, zPerMs); });
}
void set_sample_3D_distances(HSAMPLE sample, float maximum, float minimum, int32_t autoWetAttenuation) {
    guarded<void>([=]() -> void { Core::set_sample_3D_distances(sample, maximum, minimum, autoWetAttenuation); });
}
void set_sample_occlusion(HSAMPLE sample, float value) {
    guarded<void>([=]() -> void { Core::set_sample_occlusion(sample, value); });
}
void set_sample_obstruction(HSAMPLE sample, float value) {
    guarded<void>([=]() -> void { Core::set_sample_obstruction(sample, value); });
}
}
