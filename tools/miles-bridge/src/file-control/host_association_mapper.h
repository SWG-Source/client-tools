#ifndef MILES_HOST_ASSOCIATION_MAPPER45_H
#define MILES_HOST_ASSOCIATION_MAPPER45_H
#include "../file-owner/session_file_owner.h"
#include "../file-protocol/file_protocol.h"

namespace MilesFileOwner36 {
// Private single-control-thread composition. The pipe owner authenticates peers
// and posts events here; no other thread may mutate coordinator/owner directly.
class HostAssociationMapper {
public:
    enum CommandState { Empty, Executing, ReturnedWaiting, Settled };
    HostAssociationMapper(uint64_t session, uint64_t backgroundLane,
        MilesCoordinator::Coordinator &, SessionFileOwner &, size_t maxReverse);
    // Actions are chosen from trusted forward opcodes. A validated refusal must
    // report actionApplied=false; it cannot acquire or release a lease.
    // Must succeed before the command is sent. Resources have already passed
    // the session's genuine registry checks. Wire and local IDs are distinct.
    MilesCoordinator::Error publishCommand(uint64_t wireRequest, uint64_t admission,
        uint64_t lane, uint64_t lease, MilesCoordinator::Action,
        const std::vector<MilesWire::Handle> &resources, bool cleanup = false);
    // Forward reply has passed full opcode/envelope/value validation elsewhere.
    // true records it, even if causal ACKs still prevent settlement. Never waits.
    bool observeForwardReturn(uint64_t authenticatedSession, uint64_t wireRequest,
                              bool actionApplied=true);
    // Outer command result owner explicitly consumes a Settled result once.
    bool consumeCommand(uint64_t wireRequest);
    enum ControlResult { FileQueued, AckConsumed, ControlRejected, ControlFailedUnanswered };
    // Intended authenticated endpoint entry. ACK is dispatched before new-ID intake.
    // AckConsumed sends no reply; it records consumption, never a file result.
    ControlResult receiveControl(uint64_t authenticatedSession, MilesTransport::Bytes frame);
    // Trusted internal path; production endpoint code must use receiveControl.
    SessionFileOwner::Intake receiveReverse(uint64_t authenticatedSession,
                                           MilesTransport::Bytes frame);
    void poll();
    // Control owner selects a ready unsent reply from the existing reverse rows.
    bool nextUnqueuedReply(uint64_t &wire) const;
    MilesTransport::Bytes reply(uint64_t reverseRequest) const;
    // Only after Endpoint.send accepted the complete reply for this request.
    // Queueing is not consumption and does not release the operation.
    bool markReplyQueued(uint64_t reverseRequest);
    // Trusted internal path, only after exact retained ACK validation.
    // Production endpoint code uses receiveControl instead.
    // ACK carries reverse identity, NEVER a peer-selected local intake ordinal.
    bool acknowledgeConsumed(uint64_t authenticatedSession, uint64_t reverseRequest);
    void fail(); // Unknown outcomes remain pinned. Supplies no file cleanup.
    CommandState commandState() const { return command.state; }
    uint64_t currentWireRequest() const { return command.wire; }
    size_t retainedReverse() const;
private:
    struct Reverse {
        uint64_t wire, intake;
        bool queued;
        MilesFileProtocol48::FileAckExpected expected;
        Reverse() : wire(0), intake(0), queued(false) {}
    };
    struct Command {
        uint64_t wire, admission, lane, lease;
        CommandState state;
        bool actionApplied;
        Command() : wire(0), admission(0), lane(0), lease(0), state(Empty), actionApplied(false) {}
    };
    uint64_t session, backgroundLane, lastWire, lastReverse, lastIntake;
    MilesCoordinator::Coordinator &coordinator;
    SessionFileOwner &owner;
    Command command;
    std::vector<Reverse> reverse;
    Reverse *find(uint64_t);
    const Reverse *find(uint64_t) const;
    void settle();
    HostAssociationMapper(const HostAssociationMapper &);
    HostAssociationMapper &operator=(const HostAssociationMapper &);
};
}
#endif
