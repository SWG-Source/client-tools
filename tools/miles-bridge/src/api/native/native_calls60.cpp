#include "../private/native_calls60.h"
#include <Mss.h>
#include <type_traits>
#include "native_types70.h"
#if !defined(_WIN64) || _MSC_VER != 1800
#error Actual possessed v120 x64 SDK declarations required; no native library is supplied.
#endif
static_assert(std::is_same<int32_t,S32>::value,"native signed scalar");
static_assert(std::is_same<uint32_t,U32>::value,"native unsigned scalar");
static_assert(std::is_same<float,F32>::value,"native float");
static_assert(std::is_same<decltype(&::AIL_set_listener_3D_position), void (AILCALL *)(::HDIGDRIVER, F32, F32, F32)>::value,"actual set_listener_3D_position declaration");
static_assert(std::is_same<decltype(&::AIL_set_listener_3D_velocity_vector), void (AILCALL *)(::HDIGDRIVER, F32, F32, F32)>::value,"actual set_listener_3D_velocity_vector declaration");
static_assert(std::is_same<decltype(&::AIL_set_listener_3D_orientation), void (AILCALL *)(::HDIGDRIVER, F32, F32, F32, F32, F32, F32)>::value,"actual set_listener_3D_orientation declaration");
static_assert(std::is_same<decltype(&::AIL_set_3D_rolloff_factor), void (AILCALL *)(::HDIGDRIVER, F32)>::value,"actual set_3D_rolloff_factor declaration");
static_assert(std::is_same<decltype(&::AIL_serve), void (AILCALL *)()>::value,"actual serve declaration");
static_assert(std::is_same<decltype(&::AIL_lock), void (AILCALL *)()>::value,"actual lock declaration");
static_assert(std::is_same<decltype(&::AIL_unlock), void (AILCALL *)()>::value,"actual unlock declaration");
static_assert(std::is_same<decltype(&::AIL_room_type), S32 (AILCALL *)(::HDIGDRIVER)>::value,"actual room_type declaration");
static_assert(std::is_same<decltype(&::AIL_set_room_type), void (AILCALL *)(::HDIGDRIVER, S32)>::value,"actual set_room_type declaration");
static_assert(std::is_same<decltype(&::AIL_digital_CPU_percent), S32 (AILCALL *)(::HDIGDRIVER)>::value,"actual digital_CPU_percent declaration");
static_assert(std::is_same<decltype(&::AIL_digital_latency), S32 (AILCALL *)(::HDIGDRIVER)>::value,"actual digital_latency declaration");
static_assert(std::is_same<decltype(&::AIL_get_timer_highest_delay), U32 (AILCALL *)()>::value,"actual get_timer_highest_delay declaration");
static_assert(std::is_same<decltype(&::AIL_active_sample_count), S32 (AILCALL *)(::HDIGDRIVER)>::value,"actual active_sample_count declaration");
static_assert(std::is_same<decltype(&::AIL_allocate_sample_handle), ::HSAMPLE (AILCALL *)(::HDIGDRIVER)>::value,"actual allocate_sample_handle declaration");
static_assert(std::is_same<decltype(&::AIL_set_named_sample_file), S32 (AILCALL *)(::HSAMPLE, const char *, const void *, U32, S32)>::value,"actual set_named_sample_file declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_file), S32 (AILCALL *)(::HSAMPLE, const void *, S32)>::value,"actual set_sample_file declaration");
static_assert(std::is_same<decltype(&::AIL_release_sample_handle), void (AILCALL *)(::HSAMPLE)>::value,"actual release_sample_handle declaration");
static_assert(std::is_same<decltype(&::AIL_start_sample), void (AILCALL *)(::HSAMPLE)>::value,"actual start_sample declaration");
static_assert(std::is_same<decltype(&::AIL_stop_sample), void (AILCALL *)(::HSAMPLE)>::value,"actual stop_sample declaration");
static_assert(std::is_same<decltype(&::AIL_end_sample), void (AILCALL *)(::HSAMPLE)>::value,"actual end_sample declaration");
static_assert(std::is_same<decltype(&::AIL_sample_status), U32 (AILCALL *)(::HSAMPLE)>::value,"actual sample_status declaration");
static_assert(std::is_same<decltype(&::AIL_sample_ms_position), void (AILCALL *)(::HSAMPLE, S32 *, S32 *)>::value,"actual sample_ms_position declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_ms_position), void (AILCALL *)(::HSAMPLE, S32)>::value,"actual set_sample_ms_position declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_position), void (AILCALL *)(::HSAMPLE, U32)>::value,"actual set_sample_position declaration");
static_assert(std::is_same<decltype(&::AIL_sample_position), U32 (AILCALL *)(::HSAMPLE)>::value,"actual sample_position declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_loop_count), void (AILCALL *)(::HSAMPLE, S32)>::value,"actual set_sample_loop_count declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_loop_block), void (AILCALL *)(::HSAMPLE, S32, S32)>::value,"actual set_sample_loop_block declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_3D_position), void (AILCALL *)(::HSAMPLE, F32, F32, F32)>::value,"actual set_sample_3D_position declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_3D_velocity_vector), void (AILCALL *)(::HSAMPLE, F32, F32, F32)>::value,"actual set_sample_3D_velocity_vector declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_3D_distances), void (AILCALL *)(::HSAMPLE, F32, F32, S32)>::value,"actual set_sample_3D_distances declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_occlusion), void (AILCALL *)(::HSAMPLE, F32)>::value,"actual set_sample_occlusion declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_obstruction), void (AILCALL *)(::HSAMPLE, F32)>::value,"actual set_sample_obstruction declaration");
static_assert(std::is_same<decltype(&::AIL_sample_reverb_levels), void (AILCALL *)(::HSAMPLE, F32 *, F32 *)>::value,"actual sample_reverb_levels declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_volume_levels), void (AILCALL *)(::HSAMPLE, F32, F32)>::value,"actual set_sample_volume_levels declaration");
static_assert(std::is_same<decltype(&::AIL_sample_volume_levels), void (AILCALL *)(::HSAMPLE, F32 *, F32 *)>::value,"actual sample_volume_levels declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_reverb_levels), void (AILCALL *)(::HSAMPLE, F32, F32)>::value,"actual set_sample_reverb_levels declaration");
static_assert(std::is_same<decltype(&::AIL_set_sample_playback_rate), void (AILCALL *)(::HSAMPLE, S32)>::value,"actual set_sample_playback_rate declaration");
static_assert(std::is_same<decltype(&::AIL_sample_playback_rate), S32 (AILCALL *)(::HSAMPLE)>::value,"actual sample_playback_rate declaration");
static_assert(std::is_same<decltype(&::AIL_open_stream), ::HSTREAM (AILCALL *)(::HDIGDRIVER, const char *, S32)>::value,"actual open_stream declaration");
static_assert(std::is_same<decltype(&::AIL_close_stream), void (AILCALL *)(::HSTREAM)>::value,"actual close_stream declaration");
static_assert(std::is_same<decltype(&::AIL_stream_sample_handle), ::HSAMPLE (AILCALL *)(::HSTREAM)>::value,"actual stream_sample_handle declaration");
static_assert(std::is_same<decltype(&::AIL_start_stream), void (AILCALL *)(::HSTREAM)>::value,"actual start_stream declaration");
static_assert(std::is_same<decltype(&::AIL_set_stream_loop_count), void (AILCALL *)(::HSTREAM, S32)>::value,"actual set_stream_loop_count declaration");
static_assert(std::is_same<decltype(&::AIL_set_stream_loop_block), void (AILCALL *)(::HSTREAM, S32, S32)>::value,"actual set_stream_loop_block declaration");
static_assert(std::is_same<decltype(&::AIL_stream_status), S32 (AILCALL *)(::HSTREAM)>::value,"actual stream_status declaration");
static_assert(std::is_same<decltype(&::AIL_set_stream_ms_position), void (AILCALL *)(::HSTREAM, S32)>::value,"actual set_stream_ms_position declaration");
static_assert(std::is_same<decltype(&::AIL_stream_ms_position), void (AILCALL *)(::HSTREAM, S32 *, S32 *)>::value,"actual stream_ms_position declaration");
static_assert(std::is_same<decltype(&::AIL_file_type), S32 (AILCALL *)(const void *, U32)>::value,"actual file_type declaration");
static_assert(std::is_same<decltype(&::AIL_file_error), S32 (AILCALL *)()>::value,"actual file_error declaration");
static_assert(std::is_same<decltype(&::AIL_WAV_info), S32 (AILCALL *)(void const *, AILSOUNDINFO *)>::value,"actual WAV_info declaration");

static_assert(static_cast<int32_t>(ClientMiles::SampleDone)==static_cast<int32_t>(SMP_DONE),"SampleDone SDK value");
static_assert(static_cast<int32_t>(ClientMiles::SamplePlaying)==static_cast<int32_t>(SMP_PLAYING),"SamplePlaying SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileSeekBegin)==static_cast<int32_t>(AIL_FILE_SEEK_BEGIN),"FileSeekBegin SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileSeekCurrent)==static_cast<int32_t>(AIL_FILE_SEEK_CURRENT),"FileSeekCurrent SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileSeekEnd)==static_cast<int32_t>(AIL_FILE_SEEK_END),"FileSeekEnd SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileUnknown)==static_cast<int32_t>(AILFILETYPE_UNKNOWN),"FileUnknown SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FilePcmWav)==static_cast<int32_t>(AILFILETYPE_PCM_WAV),"FilePcmWav SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileAdpcmWav)==static_cast<int32_t>(AILFILETYPE_ADPCM_WAV),"FileAdpcmWav SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileOtherWav)==static_cast<int32_t>(AILFILETYPE_OTHER_WAV),"FileOtherWav SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileVoc)==static_cast<int32_t>(AILFILETYPE_VOC),"FileVoc SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileMidi)==static_cast<int32_t>(AILFILETYPE_MIDI),"FileMidi SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileXmidi)==static_cast<int32_t>(AILFILETYPE_XMIDI),"FileXmidi SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileXmidiDls)==static_cast<int32_t>(AILFILETYPE_XMIDI_DLS),"FileXmidiDls SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileXmidiMls)==static_cast<int32_t>(AILFILETYPE_XMIDI_MLS),"FileXmidiMls SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileDls)==static_cast<int32_t>(AILFILETYPE_DLS),"FileDls SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileMls)==static_cast<int32_t>(AILFILETYPE_MLS),"FileMls SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileMpegLayer1)==static_cast<int32_t>(AILFILETYPE_MPEG_L1_AUDIO),"FileMpegLayer1 SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileMpegLayer2)==static_cast<int32_t>(AILFILETYPE_MPEG_L2_AUDIO),"FileMpegLayer2 SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileMpegLayer3)==static_cast<int32_t>(AILFILETYPE_MPEG_L3_AUDIO),"FileMpegLayer3 SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileOtherAsiWav)==static_cast<int32_t>(AILFILETYPE_OTHER_ASI_WAV),"FileOtherAsiWav SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileNoError)==static_cast<int32_t>(AIL_NO_ERROR),"FileNoError SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileIoError)==static_cast<int32_t>(AIL_IO_ERROR),"FileIoError SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileOutOfMemory)==static_cast<int32_t>(AIL_OUT_OF_MEMORY),"FileOutOfMemory SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileNotFound)==static_cast<int32_t>(AIL_FILE_NOT_FOUND),"FileNotFound SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileCannotWrite)==static_cast<int32_t>(AIL_CANT_WRITE_FILE),"FileCannotWrite SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileCannotRead)==static_cast<int32_t>(AIL_CANT_READ_FILE),"FileCannotRead SDK value");
static_assert(static_cast<int32_t>(ClientMiles::FileDiskFull)==static_cast<int32_t>(AIL_DISK_FULL),"FileDiskFull SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomGeneric)==static_cast<int32_t>(ENVIRONMENT_GENERIC),"RoomGeneric SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomPaddedCell)==static_cast<int32_t>(ENVIRONMENT_PADDEDCELL),"RoomPaddedCell SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomNormal)==static_cast<int32_t>(ENVIRONMENT_ROOM),"RoomNormal SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomBathroom)==static_cast<int32_t>(ENVIRONMENT_BATHROOM),"RoomBathroom SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomLivingRoom)==static_cast<int32_t>(ENVIRONMENT_LIVINGROOM),"RoomLivingRoom SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomStoneRoom)==static_cast<int32_t>(ENVIRONMENT_STONEROOM),"RoomStoneRoom SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomAuditorium)==static_cast<int32_t>(ENVIRONMENT_AUDITORIUM),"RoomAuditorium SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomConcertHall)==static_cast<int32_t>(ENVIRONMENT_CONCERTHALL),"RoomConcertHall SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomCave)==static_cast<int32_t>(ENVIRONMENT_CAVE),"RoomCave SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomArena)==static_cast<int32_t>(ENVIRONMENT_ARENA),"RoomArena SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomHangar)==static_cast<int32_t>(ENVIRONMENT_HANGAR),"RoomHangar SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomCarpetedHallway)==static_cast<int32_t>(ENVIRONMENT_CARPETEDHALLWAY),"RoomCarpetedHallway SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomHallway)==static_cast<int32_t>(ENVIRONMENT_HALLWAY),"RoomHallway SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomStoneCorridor)==static_cast<int32_t>(ENVIRONMENT_STONECORRIDOR),"RoomStoneCorridor SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomAlley)==static_cast<int32_t>(ENVIRONMENT_ALLEY),"RoomAlley SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomForest)==static_cast<int32_t>(ENVIRONMENT_FOREST),"RoomForest SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomCity)==static_cast<int32_t>(ENVIRONMENT_CITY),"RoomCity SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomMountains)==static_cast<int32_t>(ENVIRONMENT_MOUNTAINS),"RoomMountains SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomQuarry)==static_cast<int32_t>(ENVIRONMENT_QUARRY),"RoomQuarry SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomPlain)==static_cast<int32_t>(ENVIRONMENT_PLAIN),"RoomPlain SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomParkingLot)==static_cast<int32_t>(ENVIRONMENT_PARKINGLOT),"RoomParkingLot SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomSewerPipe)==static_cast<int32_t>(ENVIRONMENT_SEWERPIPE),"RoomSewerPipe SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomUnderwater)==static_cast<int32_t>(ENVIRONMENT_UNDERWATER),"RoomUnderwater SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomDrugged)==static_cast<int32_t>(ENVIRONMENT_DRUGGED),"RoomDrugged SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomDizzy)==static_cast<int32_t>(ENVIRONMENT_DIZZY),"RoomDizzy SDK value");
static_assert(static_cast<int32_t>(ClientMiles::RoomPsychotic)==static_cast<int32_t>(ENVIRONMENT_PSYCHOTIC),"RoomPsychotic SDK value");

namespace ClientMilesNativeCalls60 {
void set_listener_3D_position(HDIGDRIVER handle, float x, float y, float z) {
    return ::AIL_set_listener_3D_position(handle, x, y, z);
}
void set_listener_3D_velocity_vector(HDIGDRIVER handle, float x, float y, float z) {
    return ::AIL_set_listener_3D_velocity_vector(handle, x, y, z);
}
void set_listener_3D_orientation(HDIGDRIVER handle, float faceX, float faceY, float faceZ, float upX, float upY, float upZ) {
    return ::AIL_set_listener_3D_orientation(handle, faceX, faceY, faceZ, upX, upY, upZ);
}
void set_3D_rolloff_factor(HDIGDRIVER handle, float factor) {
    return ::AIL_set_3D_rolloff_factor(handle, factor);
}
void serve() {
    return ::AIL_serve();
}
void lock() {
    return ::AIL_lock();
}
void unlock() {
    return ::AIL_unlock();
}
int32_t room_type(HDIGDRIVER handle) {
    return ::AIL_room_type(handle);
}
void set_room_type(HDIGDRIVER handle, int32_t room) {
    return ::AIL_set_room_type(handle, room);
}
int32_t digital_CPU_percent(HDIGDRIVER handle) {
    return ::AIL_digital_CPU_percent(handle);
}
int32_t digital_latency(HDIGDRIVER handle) {
    return ::AIL_digital_latency(handle);
}
uint32_t get_timer_highest_delay() {
    return ::AIL_get_timer_highest_delay();
}
int32_t active_sample_count(HDIGDRIVER handle) {
    return ::AIL_active_sample_count(handle);
}
HSAMPLE allocate_sample_handle(HDIGDRIVER handle) {
    return ::AIL_allocate_sample_handle(handle);
}
int32_t set_named_sample_file(HSAMPLE handle, const char * suffix, const void * fileImage, uint32_t fileBytes, int32_t block) {
    return ::AIL_set_named_sample_file(handle, suffix, fileImage, fileBytes, block);
}
int32_t set_sample_file(HSAMPLE handle, const void * fileImage, int32_t block) {
    return ::AIL_set_sample_file(handle, fileImage, block);
}
void release_sample_handle(HSAMPLE handle) {
    return ::AIL_release_sample_handle(handle);
}
void start_sample(HSAMPLE handle) {
    return ::AIL_start_sample(handle);
}
void stop_sample(HSAMPLE handle) {
    return ::AIL_stop_sample(handle);
}
void end_sample(HSAMPLE handle) {
    return ::AIL_end_sample(handle);
}
uint32_t sample_status(HSAMPLE handle) {
    return ::AIL_sample_status(handle);
}
void sample_ms_position(HSAMPLE handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds) {
    return ::AIL_sample_ms_position(handle, totalMilliseconds, currentMilliseconds);
}
void set_sample_ms_position(HSAMPLE handle, int32_t milliseconds) {
    return ::AIL_set_sample_ms_position(handle, milliseconds);
}
void set_sample_position(HSAMPLE handle, uint32_t byteOffset) {
    return ::AIL_set_sample_position(handle, byteOffset);
}
uint32_t sample_position(HSAMPLE handle) {
    return ::AIL_sample_position(handle);
}
void set_sample_loop_count(HSAMPLE handle, int32_t count) {
    return ::AIL_set_sample_loop_count(handle, count);
}
void set_sample_loop_block(HSAMPLE handle, int32_t startByte, int32_t endByte) {
    return ::AIL_set_sample_loop_block(handle, startByte, endByte);
}
void set_sample_3D_position(HSAMPLE handle, float x, float y, float z) {
    return ::AIL_set_sample_3D_position(handle, x, y, z);
}
void set_sample_3D_velocity_vector(HSAMPLE handle, float xPerMs, float yPerMs, float zPerMs) {
    return ::AIL_set_sample_3D_velocity_vector(handle, xPerMs, yPerMs, zPerMs);
}
void set_sample_3D_distances(HSAMPLE handle, float maximum, float minimum, int32_t autoWetAttenuation) {
    return ::AIL_set_sample_3D_distances(handle, maximum, minimum, autoWetAttenuation);
}
void set_sample_occlusion(HSAMPLE handle, float value) {
    return ::AIL_set_sample_occlusion(handle, value);
}
void set_sample_obstruction(HSAMPLE handle, float value) {
    return ::AIL_set_sample_obstruction(handle, value);
}
void sample_reverb_levels(HSAMPLE handle, float * dry, float * wet) {
    return ::AIL_sample_reverb_levels(handle, dry, wet);
}
void set_sample_volume_levels(HSAMPLE handle, float left, float right) {
    return ::AIL_set_sample_volume_levels(handle, left, right);
}
void sample_volume_levels(HSAMPLE handle, float * left, float * right) {
    return ::AIL_sample_volume_levels(handle, left, right);
}
void set_sample_reverb_levels(HSAMPLE handle, float dry, float wet) {
    return ::AIL_set_sample_reverb_levels(handle, dry, wet);
}
void set_sample_playback_rate(HSAMPLE handle, int32_t rate) {
    return ::AIL_set_sample_playback_rate(handle, rate);
}
int32_t sample_playback_rate(HSAMPLE handle) {
    return ::AIL_sample_playback_rate(handle);
}
HSTREAM open_stream(HDIGDRIVER handle, const char * filename, int32_t streamMem) {
    return ::AIL_open_stream(handle, filename, streamMem);
}
void close_stream(HSTREAM handle) {
    return ::AIL_close_stream(handle);
}
HSAMPLE stream_sample_handle(HSTREAM handle) {
    return ::AIL_stream_sample_handle(handle);
}
void start_stream(HSTREAM handle) {
    return ::AIL_start_stream(handle);
}
void set_stream_loop_count(HSTREAM handle, int32_t count) {
    return ::AIL_set_stream_loop_count(handle, count);
}
void set_stream_loop_block(HSTREAM handle, int32_t startOffset, int32_t endOffset) {
    return ::AIL_set_stream_loop_block(handle, startOffset, endOffset);
}
int32_t stream_status(HSTREAM handle) {
    return ::AIL_stream_status(handle);
}
void set_stream_ms_position(HSTREAM handle, int32_t milliseconds) {
    return ::AIL_set_stream_ms_position(handle, milliseconds);
}
void stream_ms_position(HSTREAM handle, int32_t * totalMilliseconds, int32_t * currentMilliseconds) {
    return ::AIL_stream_ms_position(handle, totalMilliseconds, currentMilliseconds);
}
int32_t file_type(const void * fileImage, uint32_t fileBytes) {
    return ::AIL_file_type(fileImage, fileBytes);
}
int32_t file_error() {
    return ::AIL_file_error();
}
int32_t WAV_info(const void * fileImage, SampleInformation * result) {
    AILSOUNDINFO info={};
    const S32 status=::AIL_WAV_info(fileImage,&info);
    if(status) {
        SampleInformation projected={};
        projected.format=info.format;projected.bits=info.bits;projected.channels=info.channels;
        projected.dataLength=info.data_len;projected.rate=info.rate;
        projected.samples=info.samples;projected.blockSize=info.block_size;
        *result=projected;
    }
    return status;
}
}
