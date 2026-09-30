#include "ClientMiles.h"
#include "private/failure_boundary.h"
#include "private/native_callbacks70.h"
#include <exception>
namespace ClientMiles {
SampleCallback register_EOS_callback(HSAMPLE handle, SampleCallback callback) {
    try {
        ClientMilesPrivate52::requireFatalReporter();
        return ClientMilesNativeCallbacks70::register_EOS_callback(handle, callback);
    } catch(const std::exception &error) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
    } catch(...) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"non-standard exception in native callback registration");
    }
}
StreamCallback register_stream_callback(HSTREAM handle, StreamCallback callback) {
    try {
        ClientMilesPrivate52::requireFatalReporter();
        return ClientMilesNativeCallbacks70::register_stream_callback(handle, callback);
    } catch(const std::exception &error) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::PrivateException,error.what());
    } catch(...) {
        ClientMilesPrivate52::fail(ClientMilesPrivate52::UnknownException,"non-standard exception in native callback registration");
    }
}
}
