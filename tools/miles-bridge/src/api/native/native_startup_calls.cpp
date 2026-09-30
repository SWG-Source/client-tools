#include "../private/native_startup_calls.h"
#include <windows.h>
#include <Mss.h>
#include <type_traits>
#include "native_types70.h"

#if !defined(_WIN64)
#error This private implementation targets the possessed Windows x64 SDK declarations.
#endif

static_assert(std::is_same<intptr_t, SINTa>::value, "native preference width and type");
static_assert(std::is_same<uint32_t, U32>::value, "native unsigned scalar type");
static_assert(std::is_same<int32_t, S32>::value, "native signed scalar type");
static_assert(std::is_same<decltype(&::AIL_startup), S32 (AILCALL *)()>::value, "startup declaration");
static_assert(std::is_same<decltype(&::AIL_shutdown), void (AILCALL *)()>::value, "shutdown declaration");
static_assert(std::is_same<decltype(&::AIL_get_preference), SINTa (AILCALL *)(U32)>::value, "get preference declaration");
static_assert(std::is_same<decltype(&::AIL_set_preference), SINTa (AILCALL *)(U32, SINTa)>::value, "set preference declaration");
static_assert(std::is_same<decltype(&::AIL_last_error), char *(AILCALL *)()>::value, "last error declaration");
static_assert(std::is_same<decltype(&::AIL_set_redist_directory), char *(AILCALL *)(const char *)>::value, "redist declaration");
static_assert(std::is_same<decltype(&::AIL_open_digital_driver),
    ::HDIGDRIVER (AILCALL *)(U32, S32, S32, U32)>::value, "driver declaration");
static_assert(std::is_same<decltype(&::AIL_speaker_configuration),
    MSSVECTOR3D *(AILCALL *)(::HDIGDRIVER, S32 *, S32 *, F32 *, MSS_MC_SPEC *)>::value, "speaker declaration");
static_assert(ClientMiles::MixerChannels == DIG_MIXER_CHANNELS &&
              ClientMiles::MixFragmentCount == DIG_DS_MIX_FRAGMENT_CNT, "startup preference constants");
static_assert(static_cast<int32_t>(ClientMiles::StereoSpeakerConfiguration) == static_cast<int32_t>(MSS_MC_STEREO) &&
              static_cast<int32_t>(ClientMiles::SystemSpeakerConfiguration) == static_cast<int32_t>(MSS_MC_USE_SYSTEM_CONFIG) &&
              static_cast<int32_t>(ClientMiles::HeadphoneSpeakerConfiguration) == static_cast<int32_t>(MSS_MC_HEADPHONES) &&
              static_cast<int32_t>(ClientMiles::DolbySurroundSpeakerConfiguration) == static_cast<int32_t>(MSS_MC_DOLBY_SURROUND) &&
              static_cast<int32_t>(ClientMiles::Discrete40SpeakerConfiguration) == static_cast<int32_t>(MSS_MC_40_DISCRETE) &&
              static_cast<int32_t>(ClientMiles::Discrete51SpeakerConfiguration) == static_cast<int32_t>(MSS_MC_51_DISCRETE) &&
              static_cast<int32_t>(ClientMiles::Discrete61SpeakerConfiguration) == static_cast<int32_t>(MSS_MC_61_DISCRETE) &&
              static_cast<int32_t>(ClientMiles::Discrete71SpeakerConfiguration) == static_cast<int32_t>(MSS_MC_71_DISCRETE) &&
              static_cast<int32_t>(ClientMiles::Discrete81SpeakerConfiguration) == static_cast<int32_t>(MSS_MC_81_DISCRETE), "speaker constants");

namespace ClientMilesNativeCalls52 {
int32_t startup() { return ::AIL_startup(); }
void shutdown() { ::AIL_shutdown(); }
intptr_t get_preference(uint32_t number) { return ::AIL_get_preference(number); }
intptr_t set_preference(uint32_t number, intptr_t value) { return ::AIL_set_preference(number, value); }
const char *last_error() { return ::AIL_last_error(); }
const char *set_redist_directory(const char *directory) { return ::AIL_set_redist_directory(directory); }
void MSS_version(char *destination, int32_t capacity) {
    // Use the actual private SDK macro; no reproduced resource-query body.
    AIL_MSS_version(destination, capacity);
}
ClientMiles::HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags) {
    return ::AIL_open_digital_driver(frequency, bits, channels, flags);
}
int32_t speaker_configuration_spec(ClientMiles::HDIGDRIVER driver) {
    MSS_MC_SPEC spec = MSS_MC_INVALID;
    ::AIL_speaker_configuration(driver, 0, 0, 0, &spec);
    return static_cast<int32_t>(spec);
}
}
