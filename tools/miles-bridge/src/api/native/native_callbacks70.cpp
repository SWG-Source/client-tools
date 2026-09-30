#include "../private/native_callbacks70.h"
#include <Mss.h>
#include "native_types70.h"
namespace ClientMilesNativeCallbacks70 {
ClientMiles::SampleCallback register_EOS_callback(ClientMiles::HSAMPLE sample, ClientMiles::SampleCallback callback) {
    return ::AIL_register_EOS_callback(sample, callback);
}
ClientMiles::StreamCallback register_stream_callback(ClientMiles::HSTREAM stream, ClientMiles::StreamCallback callback) {
    return ::AIL_register_stream_callback(stream, callback);
}
}
