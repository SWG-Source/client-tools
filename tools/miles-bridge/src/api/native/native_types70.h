#ifndef CLIENT_MILES_NATIVE_TYPES70_H
#define CLIENT_MILES_NATIVE_TYPES70_H
// Include after both the facade and genuine SDK; no vendor declaration bodies.
#include <type_traits>
static_assert(std::is_same<ClientMiles::HDIGDRIVER, ::HDIGDRIVER>::value,"driver identity");
static_assert(std::is_same<ClientMiles::HSAMPLE, ::HSAMPLE>::value,"sample identity");
static_assert(std::is_same<ClientMiles::HSTREAM, ::HSTREAM>::value,"stream identity");
static_assert(std::is_same<ClientMiles::SampleCallback, ::AILSAMPLECB>::value,"sample callback identity");
static_assert(std::is_same<ClientMiles::StreamCallback, ::AILSTREAMCB>::value,"stream callback identity");
static_assert(std::is_same<decltype(&::AIL_register_EOS_callback), AILSAMPLECB (AILCALL *)(HSAMPLE, AILSAMPLECB)>::value,"EOS registration signature");
static_assert(std::is_same<decltype(&::AIL_register_stream_callback), AILSTREAMCB (AILCALL *)(HSTREAM, AILSTREAMCB)>::value,"stream registration signature");
#endif
