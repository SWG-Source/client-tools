// Real coordinator regression; no SDK, engine runtime or TLS startup.
#include "../src/admission/coordinator.h"
#include <cstdio>
#include <cstdlib>

namespace {
unsigned checks = 0;
void check(bool value, int line) {
    if (!value) { std::fprintf(stderr, "video admission failed at line %d\n", line); std::exit(1); }
    ++checks;
}
#define CHECK(value) check((value), __LINE__)
}

int main() {
    using namespace MilesCoordinator;
    const Id session = 71, lane = 1;
    Coordinator c(session);
    CHECK(c.registerSessionFiles(session, 1) == Ok);
    // Open targets the driver; the next Info command targets the returned video.
    // Registry liveness is checked by the owner before this structural admission.
    MilesWire::Handle driver = { MilesWire::Driver, 1, 1 };
    MilesWire::Handle video = { MilesWire::Video, 2, 1 };
    std::vector<MilesWire::Handle> resources(1, driver);
    CHECK(c.admitGame(session, 1, lane, 0, Ordinary, resources) == Ok);
    CHECK(c.completeAdmission(session, 1) == Ok);
    resources[0] = video;
    CHECK(c.admitGame(session, 2, lane, 0, Ordinary, resources) == Ok);
    CHECK(c.activeAdmission() == 2);
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == Busy);
    CallbackId callback = {};
    CHECK(c.admitCallback(session, 1, CausalReverseIo, 2, callback) == Ok);
    CHECK(c.completeAdmission(session, 2) == PendingCallback);
    CHECK(c.activeAdmission() == 2);
    CHECK(c.acknowledge(session, callback) == Ok);
    CHECK(c.completeAdmission(session, 2) == Ok);
    CHECK(c.activeAdmission() == 0 && c.state() == Active);
    CHECK(c.completeAdmission(session, 2) == Unknown);
    CHECK(c.admitGame(session, 2, lane, 0, Ordinary, resources) == InvalidIdentity);
    // Malformed resource identities must not consume the next admission ordinal.
    resources[0].kind = 8;
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == InvalidIdentity);
    resources[0].kind = 0;
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == InvalidIdentity);
    resources[0] = video;
    resources[0].slot = 0;
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == InvalidIdentity);
    resources[0] = video;
    resources[0].generation = 0;
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == InvalidIdentity);
    CHECK(c.activeAdmission() == 0 && c.state() == Active);
    resources[0] = video;
    CHECK(c.admitGame(session, 3, lane, 0, Ordinary, resources) == Ok);
    CHECK(c.completeAdmission(session, 3) == Ok);
    CHECK(c.activeAdmission() == 0 && c.lease() == 0);
    if (checks != 22) { std::fprintf(stderr, "unexpected check count: %u\n", checks); return 1; }
    std::printf("PASS video admission: %u checks\n", checks);
}
