#ifndef CLIENT_MILES_NATIVE_CALLBACKS70_H
#define CLIENT_MILES_NATIVE_CALLBACKS70_H
#include "../ClientMiles.h"
namespace ClientMilesNativeCallbacks70 {
ClientMiles::SampleCallback register_EOS_callback(ClientMiles::HSAMPLE, ClientMiles::SampleCallback);
ClientMiles::StreamCallback register_stream_callback(ClientMiles::HSTREAM, ClientMiles::StreamCallback);
}
#endif
