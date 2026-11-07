#ifndef CLIENT_MILES_NATIVE_CALLS60_H
#define CLIENT_MILES_NATIVE_CALLS60_H
#include "../ClientMiles.h"
namespace ClientMilesNativeCalls60 {
using ClientMiles::HDIGDRIVER;
using ClientMiles::HSAMPLE;
using ClientMiles::HSTREAM;
using ClientMiles::SampleInformation;
void set_listener_3D_position(HDIGDRIVER handle, float x, float y, float z);
void set_listener_3D_velocity_vector(HDIGDRIVER handle, float x, float y, float z);
void set_listener_3D_orientation(HDIGDRIVER handle, float faceX, float faceY, float faceZ, float upX, float upY, float upZ);
void set_3D_rolloff_factor(HDIGDRIVER handle, float factor);
void serve();
void lock();
void unlock();
int32_t room_type(HDIGDRIVER handle);
void set_room_type(HDIGDRIVER handle, int32_t room);
int32_t digital_CPU_percent(HDIGDRIVER handle);
int32_t digital_latency(HDIGDRIVER handle);
uint32_t get_timer_highest_delay();
int32_t active_sample_count(HDIGDRIVER handle);
HSAMPLE allocate_sample_handle(HDIGDRIVER handle);
int32_t set_named_sample_file(HSAMPLE handle, const char * suffix, const void * fileImage, uint32_t fileBytes, int32_t block);
int32_t set_sample_file(HSAMPLE handle, const void * fileImage, int32_t block);
void release_sample_handle(HSAMPLE handle);
void start_sample(HSAMPLE handle);
void stop_sample(HSAMPLE handle);
void end_sample(HSAMPLE handle);
uint32_t sample_status(HSAMPLE handle);
void sample_ms_position(HSAMPLE handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds);
void set_sample_ms_position(HSAMPLE handle, int32_t milliseconds);
void set_sample_position(HSAMPLE handle, uint32_t byteOffset);
uint32_t sample_position(HSAMPLE handle);
void set_sample_loop_count(HSAMPLE handle, int32_t count);
void set_sample_loop_block(HSAMPLE handle, int32_t startByte, int32_t endByte);
void set_sample_3D_position(HSAMPLE handle, float x, float y, float z);
void set_sample_3D_velocity_vector(HSAMPLE handle, float xPerMs, float yPerMs, float zPerMs);
void set_sample_3D_distances(HSAMPLE handle, float maximum, float minimum, int32_t autoWetAttenuation);
void set_sample_occlusion(HSAMPLE handle, float value);
void set_sample_obstruction(HSAMPLE handle, float value);
void sample_reverb_levels(HSAMPLE handle, float * dry, float * wet);
void set_sample_volume_levels(HSAMPLE handle, float left, float right);
void sample_volume_levels(HSAMPLE handle, float * left, float * right);
void set_sample_reverb_levels(HSAMPLE handle, float dry, float wet);
void set_sample_playback_rate(HSAMPLE handle, int32_t rate);
int32_t sample_playback_rate(HSAMPLE handle);
HSTREAM open_stream(HDIGDRIVER handle, const char * filename, int32_t streamMem);
void close_stream(HSTREAM handle);
HSAMPLE stream_sample_handle(HSTREAM handle);
void start_stream(HSTREAM handle);
void set_stream_loop_count(HSTREAM handle, int32_t count);
void set_stream_loop_block(HSTREAM handle, int32_t startOffset, int32_t endOffset);
int32_t stream_status(HSTREAM handle);
void set_stream_ms_position(HSTREAM handle, int32_t milliseconds);
void stream_ms_position(HSTREAM handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds);
int32_t file_type(const void * fileImage, uint32_t fileBytes);
int32_t file_error();
int32_t WAV_info(const void * fileImage, SampleInformation * result);
}
#endif
