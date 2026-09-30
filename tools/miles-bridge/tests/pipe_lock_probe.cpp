// Development process only. This is not an engine bootstrap or teardown test.
#include "../src/api/ClientMiles.h"
#include "../src/api/pipe/LiveChannel.h"
#include "../src/failure/failure_boundary.h"
#include <windows.h>
#include <cstdio>
#include <exception>
#include <memory>
#include <process.h>

namespace {
void finish(unsigned code, const char *message)
{
    std::fprintf(code ? stderr : stdout, "%s\n", message);
    std::fflush(stdout);
    std::fflush(stderr);
    // Session::close has no paired-close implementation. Retain all roots until
    // process death; this does not claim callback/worker/allocator teardown.
    ::ExitProcess(code);
}
void fatalReporter(uint32_t reason, const char *message)
{
    std::fprintf(stderr, "Miles fatal reason %lu: ", static_cast<unsigned long>(reason));
    finish(10, message ? message : "no diagnostic");
}
bool rejectedLockFields(ClientMilesPipe::Session &session, uint32_t opcode)
{
    MilesWire::Call fields = {};
    fields.value[0] = 1;
    try { session.request(opcode, fields); }
    catch (const ClientMilesPipeCore57::Failure &error) {
        return error.reason() == ClientMilesPipeCore57::FailureReason::InvalidArgument;
    }
    return false;
}
unsigned __stdcall wrongCaller(void *context)
{
    bool &rejected = *static_cast<bool *>(context);
    try { ClientMilesPipeCore57::get_preference(ClientMiles::MixFragmentCount); }
    catch (const ClientMilesPipeCore57::Failure &error) {
        rejected = error.reason() == ClientMilesPipeCore57::FailureReason::WrongState;
    }
    catch (...) {}
    return 0;
}
}

int main(int argc, char **argv)
{
    if (argc != 3)
        finish(2, "usage: miles-pipe-probe.exe <x86 host.exe> <original Mss32.dll>");
    try {
        ClientMilesPrivate52::bindFatalReporter(fatalReporter);
        // The code and engine modules are statically linked and process resident.
        // No file callbacks are installed, so the real engine worker is linked
        // but not started; this probe does not initialize engine Thread/TLS.
        std::shared_ptr<void> engineLifetime(new int(1));
        std::shared_ptr<void> callbackLifetime(new int(1));
        ClientMilesPipe::Session *session = ClientMilesPipe::connectSession(
            argv[1], argv[2], engineLifetime, callbackLifetime, 1024 * 1024);
        if (!session || !ClientMiles::startup())
            finish(3, "FAIL: session/startup");
        intptr_t const expected = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        if (!rejectedLockFields(*session, MilesWire::AIL_lock))
            finish(11, "FAIL: malformed lock not rejected as InvalidArgument");
        ClientMiles::lock();
        ClientMiles::lock();
        intptr_t const nested = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        if (!rejectedLockFields(*session, MilesWire::AIL_unlock))
            finish(12, "FAIL: malformed unlock not rejected as InvalidArgument");
        ClientMiles::unlock();
        intptr_t const outer = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        ClientMiles::unlock();
        intptr_t const after = ClientMiles::get_preference(ClientMiles::MixFragmentCount);
        bool rejected = false;
        HANDLE caller = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, wrongCaller, &rejected, 0, 0));
        if (!caller || WaitForSingleObject(caller, 10000) != WAIT_OBJECT_0)
            finish(13, "FAIL: secondary caller did not join");
        CloseHandle(caller);
        if (!rejected)
            finish(14, "FAIL: secondary caller not rejected as WrongState");
        if (ClientMiles::get_preference(ClientMiles::MixFragmentCount) != expected)
            finish(15, "FAIL: owner preference after secondary caller");
        ClientMiles::shutdown();
        if (nested != expected || outer != expected || after != expected)
            finish(4, "FAIL: preference changed during nested lock/unlock");
        std::puts("PASS: malformed lock/unlock rejected; nested sequence completed");
        std::puts("PASS: secondary caller rejected; owner sequence completed");
        std::puts("PASS: startup, nested lock/unlock, ordinary preference, shutdown");
        finish(0, "PASS: pipe lock transport probe; test-only process exit, no teardown claim");
    } catch (const std::exception &error) {
        finish(5, error.what());
    } catch (...) {
        finish(6, "FAIL: unknown exception");
    }
    return 7;
}
