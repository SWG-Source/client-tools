#include "host_association_mapper.h"
#include <stdexcept>

namespace MilesFileOwner36 {
HostAssociationMapper::HostAssociationMapper(uint64_t s, uint64_t lane,
    MilesCoordinator::Coordinator &c, SessionFileOwner &o, size_t maximum)
    : session(s), backgroundLane(lane), lastWire(0), lastReverse(0), lastIntake(0),
      coordinator(c), owner(o), reverse(maximum) {
    if (!s || !lane || !maximum || s != o.session || &c != &o.coordinator || !o.registration)
        throw std::invalid_argument("session, background lane and reverse capacity required");
}
MilesCoordinator::Error HostAssociationMapper::publishCommand(uint64_t wire,
    uint64_t admission, uint64_t lane, uint64_t lease, MilesCoordinator::Action action,
    const std::vector<MilesWire::Handle> &resources, bool cleanup) {
    if (command.state != Empty) return MilesCoordinator::Busy;
    if (!wire || wire <= lastWire || lane == backgroundLane || action != MilesCoordinator::Ordinary)
        return MilesCoordinator::InvalidIdentity;
    MilesCoordinator::Error error = cleanup
        ? coordinator.admitCleanup(session, admission, lane, lease, action, resources)
        : coordinator.admitGame(session, admission, lane, lease, action, resources);
    if (error != MilesCoordinator::Ok) return error;
    // No allocation or fallible work after coordinator admission publication.
    command.wire = wire; command.admission = admission;
    command.lane = lane; command.lease = lease; command.state = Executing;
    lastWire = wire;
    return MilesCoordinator::Ok;
}
void HostAssociationMapper::settle() {
    if (command.state != ReturnedWaiting) return;
    const MilesCoordinator::Error result = coordinator.completeAdmission(session, command.admission);
    if (result == MilesCoordinator::Ok) command.state = Settled;
    else if (result != MilesCoordinator::PendingCallback) fail();
}
bool HostAssociationMapper::observeForwardReturn(uint64_t s, uint64_t wire) {
    if (s != session || command.state != Executing || wire != command.wire) {
        fail(); return false;
    }
    command.state = ReturnedWaiting;
    settle();
    return true;
}
bool HostAssociationMapper::consumeCommand(uint64_t wire) {
    if (command.state != Settled || command.wire != wire) return false;
    command = Command();
    return true;
}
HostAssociationMapper::Reverse *HostAssociationMapper::find(uint64_t wire) {
    if (!wire) return 0;
    for (size_t i = 0; i < reverse.size(); ++i) if (reverse[i].wire == wire) return &reverse[i];
    return 0;
}
const HostAssociationMapper::Reverse *HostAssociationMapper::find(uint64_t wire) const {
    if (!wire) return 0;
    for (size_t i = 0; i < reverse.size(); ++i) if (reverse[i].wire == wire) return &reverse[i];
    return 0;
}
HostAssociationMapper::ControlResult HostAssociationMapper::receiveControl(
    uint64_t s, MilesTransport::Bytes frame) {
    try {
        MilesWire::Header header = {};
        MilesWire::Call call = {};
        if (s != session || !MilesTransport::decodeCall(frame, header, call)) {
            fail(); return ControlRejected;
        }
        // An ACK reuses the original ID and must never enter monotonic intake.
        if (header.opcode == MilesWire::FileConsumptionAck) {
            const Reverse *record = find(header.request);
            if (!record || !MilesFileProtocol48::validateFileConsumptionAck(frame, record->expected)) {
                fail(); return ControlRejected;
            }
            return acknowledgeConsumed(s, header.request) ? AckConsumed : ControlRejected;
        }
        const SessionFileOwner::Intake result = receiveReverse(s, frame);
        if (result == SessionFileOwner::Queued) return FileQueued;
        return result == SessionFileOwner::Rejected ? ControlRejected : ControlFailedUnanswered;
    } catch (...) {
        // Retain any published row/job. External failure handling supplies no EOF.
        fail(); return ControlFailedUnanswered;
    }
}
SessionFileOwner::Intake HostAssociationMapper::receiveReverse(uint64_t s, MilesTransport::Bytes frame) {
    try {
        MilesWire::Header header = {};
        MilesWire::Call call = {};
        if (s != session || !MilesTransport::decodeCall(frame, header, call) ||
            header.kind != MilesWire::ReverseRequest || !header.request ||
            header.request <= lastReverse || lastIntake == UINT64_MAX) {
            fail(); return SessionFileOwner::Rejected;
        }
        uint64_t admission = 0;
        MilesCoordinator::CallbackKind kind = MilesCoordinator::Unsolicited;
        if (header.causal_request) {
            if ((command.state != Executing && command.state != ReturnedWaiting) ||
                header.causal_request != command.wire || header.lane != command.lane ||
                header.lock_lease != command.lease) {
                fail(); return SessionFileOwner::Rejected;
            }
            admission = command.admission; kind = MilesCoordinator::CausalReverseIo;
        } else if (header.lane != backgroundLane || header.lock_lease) {
            fail(); return SessionFileOwner::Rejected;
        }
        size_t slot = 0;
        for (; slot < reverse.size(); ++slot) if (!reverse[slot].wire) break;
        if (slot == reverse.size()) { fail(); return SessionFileOwner::FailedUnanswered; }
        const MilesFileChannel26::Association association = {
            header.request, header.causal_request, header.lane, header.lock_lease
        };
        MilesFileChannel26::Request request;
        MilesFileProtocol48::FileAckExpected expected;
        if (MilesFileChannel26::decodeRequest(frame, association, request) != MilesFileChannel26::Valid ||
            !MilesFileProtocol48::expectFileAck(request, owner.registration, expected)) {
            fail(); return SessionFileOwner::Rejected;
        }
        // Original opcode/envelope and genuine registration precede all file effects.
        // Deep rejection above publishes no reverse row and consumes no intake ID.
        // All correlation storage precedes file admission/possible worker execution.
        Reverse &record = reverse[slot];
        record.expected = expected;
        record.wire = header.request; record.intake = ++lastIntake; record.queued = false;
        lastReverse = header.request;
        const TrustedAssociation trusted(session, record.intake, admission, association, kind);
        const SessionFileOwner::Intake result = owner.receive(trusted, frame);
        if (result != SessionFileOwner::Queued) fail();
        return result;
    } catch (...) {
        // Before publication there is no file work; afterwards keep the row/job.
        fail(); return SessionFileOwner::FailedUnanswered;
    }
}
void HostAssociationMapper::poll() { owner.poll(); settle(); }
MilesTransport::Bytes HostAssociationMapper::reply(uint64_t wire) const {
    const Reverse *record = find(wire);
    return record ? owner.reply(record->intake) : MilesTransport::Bytes();
}
bool HostAssociationMapper::markReplyQueued(uint64_t wire) {
    Reverse *record = find(wire);
    if (!record || record->queued || !owner.reply(record->intake).size) {
        fail(); return false;
    }
    record->queued = true;
    return true;
}
bool HostAssociationMapper::acknowledgeConsumed(uint64_t s, uint64_t wire) {
    Reverse *record = find(wire);
    if (s != session || !record || !record->queued || !owner.acknowledge(session, record->intake)) {
        fail(); return false;
    }
    *record = Reverse();
    settle();
    return true;
}
void HostAssociationMapper::fail() { coordinator.fail(session); }
size_t HostAssociationMapper::retainedReverse() const {
    size_t count = 0;
    for (size_t i = 0; i < reverse.size(); ++i) if (reverse[i].wire) ++count;
    return count;
}
}

namespace MilesFileOwner36 {
bool HostAssociationMapper::nextUnqueuedReply(uint64_t &wire) const {
    for (size_t i=0;i<reverse.size();++i) {
        if(reverse[i].wire && !reverse[i].queued && owner.reply(reverse[i].intake).size) {
            wire=reverse[i].wire;return true;
        }
    }
    return false;
}
}
