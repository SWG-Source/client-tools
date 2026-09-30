#include "ClientMiles.h"
#include "private/failure_boundary.h"
#include "private/native_calls60.h"
#include <exception>
namespace {
namespace Boundary=ClientMilesPrivate52;
namespace Native=ClientMilesNativeCalls60;
template<class Result,class Action> Result guarded(Action action) {
    try {Boundary::requireFatalReporter();return action();}
    catch(const std::exception &error){Boundary::fail(Boundary::PrivateException,error.what());}
    catch(...){Boundary::fail(Boundary::UnknownException,"non-standard exception in Miles native adapter");}
}
}
namespace ClientMiles {
void set_listener_3D_position(HDIGDRIVER handle, float x, float y, float z) {
    return guarded<void>([=]() -> void {
        return Native::set_listener_3D_position(handle, x, y, z);
    });
}
void set_listener_3D_velocity_vector(HDIGDRIVER handle, float x, float y, float z) {
    return guarded<void>([=]() -> void {
        return Native::set_listener_3D_velocity_vector(handle, x, y, z);
    });
}
void set_listener_3D_orientation(HDIGDRIVER handle, float faceX, float faceY, float faceZ, float upX, float upY, float upZ) {
    return guarded<void>([=]() -> void {
        return Native::set_listener_3D_orientation(handle, faceX, faceY, faceZ, upX, upY, upZ);
    });
}
void set_3D_rolloff_factor(HDIGDRIVER handle, float factor) {
    return guarded<void>([=]() -> void {
        return Native::set_3D_rolloff_factor(handle, factor);
    });
}
void serve() {
    return guarded<void>([=]() -> void {
        return Native::serve();
    });
}
void lock() {
    return guarded<void>([=]() -> void {
        return Native::lock();
    });
}
void unlock() {
    return guarded<void>([=]() -> void {
        return Native::unlock();
    });
}
int32_t room_type(HDIGDRIVER handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::room_type(handle);
    });
}
void set_room_type(HDIGDRIVER handle, int32_t room) {
    return guarded<void>([=]() -> void {
        return Native::set_room_type(handle, room);
    });
}
int32_t digital_CPU_percent(HDIGDRIVER handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::digital_CPU_percent(handle);
    });
}
int32_t digital_latency(HDIGDRIVER handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::digital_latency(handle);
    });
}
uint32_t get_timer_highest_delay() {
    return guarded<uint32_t>([=]() -> uint32_t {
        return Native::get_timer_highest_delay();
    });
}
int32_t active_sample_count(HDIGDRIVER handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::active_sample_count(handle);
    });
}
HSAMPLE allocate_sample_handle(HDIGDRIVER handle) {
    return guarded<HSAMPLE>([=]() -> HSAMPLE {
        return Native::allocate_sample_handle(handle);
    });
}
int32_t set_named_sample_file(HSAMPLE handle, const char * suffix, const void * fileImage, uint32_t fileBytes, int32_t block) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::set_named_sample_file(handle, suffix, fileImage, fileBytes, block);
    });
}
int32_t set_sample_file(HSAMPLE handle, const void * fileImage, int32_t block) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::set_sample_file(handle, fileImage, block);
    });
}
void release_sample_handle(HSAMPLE handle) {
    return guarded<void>([=]() -> void {
        return Native::release_sample_handle(handle);
    });
}
void start_sample(HSAMPLE handle) {
    return guarded<void>([=]() -> void {
        return Native::start_sample(handle);
    });
}
void stop_sample(HSAMPLE handle) {
    return guarded<void>([=]() -> void {
        return Native::stop_sample(handle);
    });
}
void end_sample(HSAMPLE handle) {
    return guarded<void>([=]() -> void {
        return Native::end_sample(handle);
    });
}
uint32_t sample_status(HSAMPLE handle) {
    return guarded<uint32_t>([=]() -> uint32_t {
        return Native::sample_status(handle);
    });
}
void sample_ms_position(HSAMPLE handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds) {
    return guarded<void>([=]() -> void {
        return Native::sample_ms_position(handle, totalMilliseconds, currentMilliseconds);
    });
}
void set_sample_ms_position(HSAMPLE handle, int32_t milliseconds) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_ms_position(handle, milliseconds);
    });
}
void set_sample_position(HSAMPLE handle, uint32_t byteOffset) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_position(handle, byteOffset);
    });
}
uint32_t sample_position(HSAMPLE handle) {
    return guarded<uint32_t>([=]() -> uint32_t {
        return Native::sample_position(handle);
    });
}
void set_sample_loop_count(HSAMPLE handle, int32_t count) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_loop_count(handle, count);
    });
}
void set_sample_loop_block(HSAMPLE handle, int32_t startByte, int32_t endByte) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_loop_block(handle, startByte, endByte);
    });
}
void set_sample_3D_position(HSAMPLE handle, float x, float y, float z) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_3D_position(handle, x, y, z);
    });
}
void set_sample_3D_velocity_vector(HSAMPLE handle, float xPerMs, float yPerMs, float zPerMs) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_3D_velocity_vector(handle, xPerMs, yPerMs, zPerMs);
    });
}
void set_sample_3D_distances(HSAMPLE handle, float maximum, float minimum, int32_t autoWetAttenuation) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_3D_distances(handle, maximum, minimum, autoWetAttenuation);
    });
}
void set_sample_occlusion(HSAMPLE handle, float value) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_occlusion(handle, value);
    });
}
void set_sample_obstruction(HSAMPLE handle, float value) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_obstruction(handle, value);
    });
}
void sample_reverb_levels(HSAMPLE handle, float * dry, float * wet) {
    return guarded<void>([=]() -> void {
        return Native::sample_reverb_levels(handle, dry, wet);
    });
}
void set_sample_volume_levels(HSAMPLE handle, float left, float right) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_volume_levels(handle, left, right);
    });
}
void sample_volume_levels(HSAMPLE handle, float * left, float * right) {
    return guarded<void>([=]() -> void {
        return Native::sample_volume_levels(handle, left, right);
    });
}
void set_sample_reverb_levels(HSAMPLE handle, float dry, float wet) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_reverb_levels(handle, dry, wet);
    });
}
void set_sample_playback_rate(HSAMPLE handle, int32_t rate) {
    return guarded<void>([=]() -> void {
        return Native::set_sample_playback_rate(handle, rate);
    });
}
int32_t sample_playback_rate(HSAMPLE handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::sample_playback_rate(handle);
    });
}
HSTREAM open_stream(HDIGDRIVER handle, const char * filename, int32_t streamMem) {
    return guarded<HSTREAM>([=]() -> HSTREAM {
        return Native::open_stream(handle, filename, streamMem);
    });
}
void close_stream(HSTREAM handle) {
    return guarded<void>([=]() -> void {
        return Native::close_stream(handle);
    });
}
HSAMPLE stream_sample_handle(HSTREAM handle) {
    return guarded<HSAMPLE>([=]() -> HSAMPLE {
        return Native::stream_sample_handle(handle);
    });
}
void start_stream(HSTREAM handle) {
    return guarded<void>([=]() -> void {
        return Native::start_stream(handle);
    });
}
void set_stream_loop_count(HSTREAM handle, int32_t count) {
    return guarded<void>([=]() -> void {
        return Native::set_stream_loop_count(handle, count);
    });
}
void set_stream_loop_block(HSTREAM handle, int32_t startOffset, int32_t endOffset) {
    return guarded<void>([=]() -> void {
        return Native::set_stream_loop_block(handle, startOffset, endOffset);
    });
}
int32_t stream_status(HSTREAM handle) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::stream_status(handle);
    });
}
void set_stream_ms_position(HSTREAM handle, int32_t milliseconds) {
    return guarded<void>([=]() -> void {
        return Native::set_stream_ms_position(handle, milliseconds);
    });
}
void stream_ms_position(HSTREAM handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds) {
    return guarded<void>([=]() -> void {
        return Native::stream_ms_position(handle, totalMilliseconds, currentMilliseconds);
    });
}
int32_t file_type(const void * fileImage, uint32_t fileBytes) {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::file_type(fileImage, fileBytes);
    });
}
int32_t file_error() {
    return guarded<int32_t>([=]() -> int32_t {
        return Native::file_error();
    });
}
int32_t WAV_info(const void * fileImage, SampleInformation * result) {
    return guarded<int32_t>([=]() -> int32_t {
        if(!result)Boundary::fail(Boundary::InvalidArgument,"WAV_info requires result storage");
        return Native::WAV_info(fileImage, result);
    });
}
}
