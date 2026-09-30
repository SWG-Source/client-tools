// Portable unit test: real coordinator only; no SDK, engine runtime or TLS startup.
#include "../src/admission/coordinator.h"
#include <cstdio>
#include <cstdlib>

namespace {
unsigned checks = 0;
void check(bool value, int line) {
    if (!value) { std::fprintf(stderr, "lock admission failed at line %d\n", line); std::exit(1); }
    ++checks;
}
#define CHECK(value) check((value), __LINE__)
}

int main() {
    using namespace MilesCoordinator;
    const Id session = 71, lane = 1;
    std::vector<MilesWire::Handle> resources;
    Coordinator c(session);
    CHECK(actionForOpcode(MilesWire::AIL_lock) == AcquireLock);
    CHECK(actionForOpcode(MilesWire::AIL_unlock) == ReleaseLock);
    CHECK(actionForOpcode(MilesWire::AIL_serve) == Ordinary);
    CHECK(c.registerSessionFiles(session, 1) == Ok);
    CHECK(c.admitGame(session, 1, lane, 0, ReleaseLock, resources) == WrongLease);
    CHECK(c.admitGame(session, 1, lane, 0, AcquireLock, resources) == Ok);
    CallbackId causal = {}, background = {};
    CHECK(c.admitCallback(session, 1, CausalReverseIo, 1, causal) == Ok);
    CHECK(c.completeAdmission(session, 1, false) == PendingCallback);
    CHECK(c.lease() == 0 && c.leaseDepth() == 0 && c.activeAdmission() == 1);
    CHECK(c.acknowledge(session, causal) == Ok);
    CHECK(c.completeAdmission(session, 1, false) == Ok);
    CHECK(c.lease() == 0 && c.leaseOwner() == 0 && c.activeAdmission() == 0);
    CHECK(c.admitGame(session, 2, lane, 0, AcquireLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 2, true) == Ok);
    const Id lease = c.lease();
    CHECK(lease == 1 && c.leaseDepth() == 1 && c.leaseOwner() == lane);
    CHECK(c.admitGame(session, 3, lane + 1, lease, Ordinary, resources) == WrongLease);
    CHECK(c.admitGame(session, 3, lane, lease + 1, Ordinary, resources) == WrongLease);
    CHECK(c.admitGame(session, 3, lane, lease, AcquireLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 3, false) == Ok);
    CHECK(c.lease() == lease && c.leaseDepth() == 1);
    CHECK(c.admitGame(session, 4, lane, lease, AcquireLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 4, true) == Ok);
    CHECK(c.lease() == lease && c.leaseDepth() == 2);
    CHECK(c.admitGame(session, 5, lane, lease, Ordinary, resources) == Ok);
    CHECK(c.admitCallback(session, 1, CausalReverseIo, 5, causal) == Ok);
    CHECK(c.admitCallback(session, 1, Unsolicited, 0, background) == Ok);
    CHECK(c.completeAdmission(session, 5, true) == PendingCallback);
    CHECK(c.acknowledge(session, causal) == Ok);
    // Outstanding unsolicited observations neither block completion nor prove quiescence.
    CHECK(c.completeAdmission(session, 5, true) == Ok);
    Readiness ready = {};
    CHECK(c.readiness(session, 1, ready) == Ok);
    CHECK(ready.callbackPins == 1 && ready.vendorTerminationUnproven);
    CHECK(c.acknowledge(session, background) == Ok);
    CHECK(c.admitGame(session, 6, lane, lease, ReleaseLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 6, false) == Ok);
    CHECK(c.lease() == lease && c.leaseDepth() == 2);
    CHECK(c.admitGame(session, 7, lane, lease, ReleaseLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 7, true) == Ok);
    CHECK(c.lease() == lease && c.leaseDepth() == 1);
    CHECK(c.admitGame(session, 8, lane, lease, ReleaseLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 8, true) == Ok);
    CHECK(c.lease() == 0 && c.leaseDepth() == 0 && c.leaseOwner() == 0);
    CHECK(c.admitGame(session, 9, lane, lease, Ordinary, resources) == WrongLease);
    CHECK(c.admitGame(session, 9, lane, 0, AcquireLock, resources) == Ok);
    CHECK(c.completeAdmission(session, 9, true) == Ok);
    CHECK(c.lease() == lease + 1 && c.leaseDepth() == 1);
    CHECK(c.admitGame(session, 10, lane, c.lease(), ReleaseLock, resources) == Ok);
    CHECK(c.admitCallback(session, 1, CausalReverseIo, 10, causal) == Ok);
    CHECK(c.fail(session) == Ok);
    CHECK(c.completeAdmission(session, 10, true) == PendingCallback);
    CHECK(c.acknowledge(session, causal) == Ok);
    CHECK(c.completeAdmission(session, 10, true) == Ok);
    CHECK(c.state() == Failed && c.lease() == lease + 1 && c.leaseDepth() == 1 && c.activeAdmission() == 0);
    CHECK(c.admitGame(session, 11, lane, c.lease(), Ordinary, resources) == WrongState);
    if (checks != 53) { std::fprintf(stderr, "unexpected check count: %u\n", checks); return 1; }
    std::printf("PASS lock admission: %u checks\n", checks);
}
