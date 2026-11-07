#ifndef CLIENT_MILES_PRIVATE_NATIVE_CALLS52_H
#define CLIENT_MILES_PRIVATE_NATIVE_CALLS52_H
#include "../ClientMiles.h"

// Statically linked implementation seam, not a function table or runtime plugin.
// Production definitions are in native/native_startup_calls.cpp and call the SDK.
// A later explicitly scoped portable gate may link scripted definitions instead
// to exercise the real public snapshot/catch code without a vendor or engine.
namespace ClientMilesNativeCalls52 {
int32_t startup();
void shutdown();
intptr_t get_preference(uint32_t number);
intptr_t set_preference(uint32_t number, intptr_t value);
const char *last_error();
const char *set_redist_directory(const char *retainedDirectory);
void MSS_version(char *destination, int32_t capacity);
ClientMiles::HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits,
                                           int32_t channels, uint32_t flags);
int32_t speaker_configuration_spec(ClientMiles::HDIGDRIVER driver);
}
#endif
