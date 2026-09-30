#ifndef CLIENT_MILES_DEVELOPMENT_AUDIO_SELECTION_H
#define CLIENT_MILES_DEVELOPMENT_AUDIO_SELECTION_H
#ifndef CLIENT_MILES_DEV_FACADE
#error Development Audio facade selection requires explicit opt-in
#endif
// Include after genuine Miles/engine headers. This selects source calls only;
// it defines no SDK exports and is not a production backend selection.
#include "../api/ClientMiles.h"
#include "../api/pipe/ScopedSourceImage.h"
#include "../failure/failure_boundary.h"
namespace ClientMilesDevelopment {
inline void speakerConfiguration(HDIGDRIVER driver, S32 *physical, S32 *logical,
                                 F32 *falloff, MSS_MC_SPEC *spec) {
    if (physical || logical || falloff || !spec)
        ClientMilesPrivate52::fail(ClientMilesPrivate52::InvalidArgument,
            "development speaker query supports only the required spec output");
    *spec = static_cast<MSS_MC_SPEC>(ClientMiles::speaker_configuration_spec(driver));
}
inline S32 wavInfo(const void *image, AILSOUNDINFO *info) {
    if (!info)
        ClientMilesPrivate52::fail(ClientMilesPrivate52::InvalidArgument,
            "development WAV query requires output");
    ClientMiles::SampleInformation projected = {};
    const S32 result = ClientMiles::WAV_info(image, &projected);
    if (result) {
        info->format = projected.format;
        info->bits = projected.bits;
        info->channels = projected.channels;
        info->data_len = projected.dataLength;
        info->rate = projected.rate;
        info->samples = projected.samples;
        info->block_size = projected.blockSize;
    }
    return result;
}
}
#undef AIL_MSS_version
#define AIL_MSS_version ClientMiles::MSS_version
#define AIL_WAV_info ClientMilesDevelopment::wavInfo
#define AIL_speaker_configuration ClientMilesDevelopment::speakerConfiguration
#define AIL_active_sample_count ClientMiles::active_sample_count
#define AIL_allocate_sample_handle ClientMiles::allocate_sample_handle
#define AIL_close_stream ClientMiles::close_stream
#define AIL_digital_CPU_percent ClientMiles::digital_CPU_percent
#define AIL_digital_latency ClientMiles::digital_latency
#define AIL_end_sample ClientMiles::end_sample
#define AIL_file_error ClientMiles::file_error
#define AIL_file_type ClientMiles::file_type
#define AIL_get_preference ClientMiles::get_preference
#define AIL_get_timer_highest_delay ClientMiles::get_timer_highest_delay
#define AIL_last_error ClientMiles::last_error
#define AIL_lock ClientMiles::lock
#define AIL_open_digital_driver ClientMiles::open_digital_driver
#define AIL_open_stream ClientMiles::open_stream
#define AIL_register_EOS_callback ClientMiles::register_EOS_callback
#define AIL_register_stream_callback ClientMiles::register_stream_callback
#define AIL_release_sample_handle ClientMiles::release_sample_handle
#define AIL_room_type ClientMiles::room_type
#define AIL_sample_ms_position ClientMiles::sample_ms_position
#define AIL_sample_playback_rate ClientMiles::sample_playback_rate
#define AIL_sample_position ClientMiles::sample_position
#define AIL_sample_reverb_levels ClientMiles::sample_reverb_levels
#define AIL_sample_status ClientMiles::sample_status
#define AIL_sample_volume_levels ClientMiles::sample_volume_levels
#define AIL_serve ClientMiles::serve
#define AIL_set_3D_rolloff_factor ClientMiles::set_3D_rolloff_factor
#define AIL_set_file_callbacks ClientMiles::set_file_callbacks
#define AIL_set_listener_3D_orientation ClientMiles::set_listener_3D_orientation
#define AIL_set_listener_3D_position ClientMiles::set_listener_3D_position
#define AIL_set_listener_3D_velocity_vector ClientMiles::set_listener_3D_velocity_vector
#define AIL_set_named_sample_file ClientMiles::set_named_sample_file
#define AIL_set_preference ClientMiles::set_preference
#define AIL_set_redist_directory ClientMiles::set_redist_directory
#define AIL_set_room_type ClientMiles::set_room_type
#define AIL_set_sample_3D_distances ClientMiles::set_sample_3D_distances
#define AIL_set_sample_3D_position ClientMiles::set_sample_3D_position
#define AIL_set_sample_3D_velocity_vector ClientMiles::set_sample_3D_velocity_vector
#define AIL_set_sample_file ClientMiles::set_sample_file
#define AIL_set_sample_loop_block ClientMiles::set_sample_loop_block
#define AIL_set_sample_loop_count ClientMiles::set_sample_loop_count
#define AIL_set_sample_ms_position ClientMiles::set_sample_ms_position
#define AIL_set_sample_obstruction ClientMiles::set_sample_obstruction
#define AIL_set_sample_occlusion ClientMiles::set_sample_occlusion
#define AIL_set_sample_playback_rate ClientMiles::set_sample_playback_rate
#define AIL_set_sample_position ClientMiles::set_sample_position
#define AIL_set_sample_reverb_levels ClientMiles::set_sample_reverb_levels
#define AIL_set_sample_volume_levels ClientMiles::set_sample_volume_levels
#define AIL_set_stream_loop_block ClientMiles::set_stream_loop_block
#define AIL_set_stream_loop_count ClientMiles::set_stream_loop_count
#define AIL_set_stream_ms_position ClientMiles::set_stream_ms_position
#define AIL_shutdown ClientMiles::shutdown
#define AIL_start_sample ClientMiles::start_sample
#define AIL_start_stream ClientMiles::start_stream
#define AIL_startup ClientMiles::startup
#define AIL_stop_sample ClientMiles::stop_sample
#define AIL_stream_ms_position ClientMiles::stream_ms_position
#define AIL_stream_sample_handle ClientMiles::stream_sample_handle
#define AIL_stream_status ClientMiles::stream_status
#define AIL_unlock ClientMiles::unlock
#endif
