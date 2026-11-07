#include "ClientMiles.h"
#include "private/failure_boundary.h"
#include "private/native_startup_calls.h"
#include <exception>
#include <list>
#include <string>

namespace {
namespace Native = ClientMilesNativeCalls52;
namespace Boundary = ClientMilesPrivate52;

struct TextStorage {
    std::string lastError;
    std::string redistResult;
    // Each input address remains stable even if a later setter selects another
    // directory. This conservative retention is private and ends after shutdown.
    std::list<std::string> directories;
};
// The composition must serialize lifecycle/text access. This source does not
// supply that composition, a callback thread, or an SDK-spanning mutex.
TextStorage *storage = 0;
TextStorage &textStorage() {
    if (!storage)
        storage = new TextStorage;
    return *storage;
}
const char *replaceSnapshot(std::string &target, const char *nativeText) {
    if (!nativeText) {
        target.clear();
        return 0;
    }
    std::string replacement(nativeText); // Complete the copy before replacing storage.
    target.swap(replacement);
    return target.c_str(); // Nonnull even for the genuine empty string.
}
void releaseTextStorageAfterShutdown() {
    TextStorage *retired = storage;
    storage = 0;
    delete retired;
}
template<class Result, class Action>
Result guarded(Action action) {
    try {
        Boundary::requireFatalReporter();
        return action();
    } catch (const std::exception &error) {
        Boundary::fail(Boundary::PrivateException, error.what());
    } catch (...) {
        Boundary::fail(Boundary::UnknownException, "non-standard exception in Miles startup adapter");
    }
}
}

namespace ClientMiles {
int32_t startup() {
    return guarded<int32_t>([]() -> int32_t { return Native::startup(); });
}
void shutdown() {
    guarded<void>([]() -> void {
        Native::shutdown();
        // No release before the actual shutdown returns. If it throws, the
        // guard takes the nonreturning path while retained inputs stay alive.
        releaseTextStorageAfterShutdown();
    });
}
intptr_t get_preference(uint32_t number) {
    return guarded<intptr_t>([=]() -> intptr_t { return Native::get_preference(number); });
}
intptr_t set_preference(uint32_t number, intptr_t value) {
    return guarded<intptr_t>([=]() -> intptr_t { return Native::set_preference(number, value); });
}
const char *last_error() {
    return guarded<const char *>([]() -> const char * {
        TextStorage &text = textStorage();
        return replaceSnapshot(text.lastError, Native::last_error());
    });
}
const char *set_redist_directory(const char *directory) {
    return guarded<const char *>([=]() -> const char * {
        if (!directory)
            Boundary::fail(Boundary::InvalidArgument, "null Miles redistribution directory");
        TextStorage &text = textStorage();
        // Input copy and node publication precede the vendor setter. No borrowed
        // game buffer is retained by this adapter, including pre-startup calls.
        text.directories.push_back(std::string(directory));
        const char *nativeResult = Native::set_redist_directory(text.directories.back().c_str());
        // A copy failure here follows a possible vendor effect; the guard fails
        // terminally. The stable directory node remains retained; no retry.
        return replaceSnapshot(text.redistResult, nativeResult);
    });
}
void MSS_version(char *destination, int32_t capacity) {
    guarded<void>([=]() -> void {
        if (!destination || capacity <= 0)
            Boundary::fail(Boundary::InvalidArgument, "Miles version requires a writable positive extent");
        // Preserve the selected native macro's writes, including genuine empty
        // output. No fill, resource-status synthesis, or nonempty postcondition.
        Native::MSS_version(destination, capacity);
    });
}
HDIGDRIVER open_digital_driver(uint32_t frequency, int32_t bits, int32_t channels, uint32_t flags) {
    return guarded<HDIGDRIVER>([=]() -> HDIGDRIVER { return Native::open_digital_driver(frequency, bits, channels, flags); });
}
int32_t speaker_configuration_spec(HDIGDRIVER driver) {
    return guarded<int32_t>([=]() -> int32_t { return Native::speaker_configuration_spec(driver); });
}
}
