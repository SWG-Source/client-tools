#ifndef MILES_COORDINATOR_CANDIDATE_H
#define MILES_COORDINATOR_CANDIDATE_H
#include "../protocol/miles_wire.h"
#include <map>
#include <vector>
namespace MilesCoordinator {
typedef uint64_t Id;
enum State { Active, Draining, Failed };
enum Action { Ordinary, AcquireLock, ReleaseLock };
// Both authenticated endpoints derive the transition; peers never select an action.
inline Action actionForOpcode(uint32_t opcode) {
    return opcode==MilesWire::AIL_lock ? AcquireLock :
        opcode==MilesWire::AIL_unlock ? ReleaseLock : Ordinary;
}
enum CallbackKind { CausalReverseIo, Unsolicited };
enum Error { Ok, StaleSession, InvalidIdentity, WrongState, Busy, WrongLease,
             Capacity, Unknown, PendingCallback, ClosingResource };
struct Limits {
    size_t registrations, callbacks, requestResources;
    Id maximumId;
    Limits():registrations(64),callbacks(256),requestResources(16),maximumId(UINT64_MAX) {}
};
struct CallbackId { Id registration, sequence; };
struct Readiness {
    size_t requestPins, callbackPins;
    Id closeFrontier, acknowledgedFrontier;
    bool closing, vendorTerminationUnproven;
};
// Deterministic single-control-thread model. No vendor calls or pointer ownership.
// Input handles require structural validity; genuine registry liveness is checked
// by the central owner before admission. No general nested vendor reentry modeled.
// admissionOrdinal is minted AFTER scheduler selection, not a wire correlation ID.
// Intake replay tracking, deferred queues and fairness are outside this model.
class Coordinator {
public:
    Coordinator(Id incarnation, const Limits &limits=Limits());
    Error admitGame(Id session,Id admissionOrdinal,Id lane,Id lease,Action action,
                    const std::vector<MilesWire::Handle> &resources);
    // Trusted coordinator API; never selected directly by an untrusted wire flag.
    Error admitCleanup(Id session,Id admissionOrdinal,Id lane,Id lease,Action action,
                       const std::vector<MilesWire::Handle> &resources);
    // A validated refusal settles request pins but must not apply a lock transition.
    Error completeAdmission(Id session,Id admissionOrdinal,bool actionApplied=true);
    Error registerCallback(Id session,Id registration,const MilesWire::Handle &resource);
    // Trusted owner API: one session file table, before any driver/file exists.
    // Scope is internal; it is never selected by a wire flag or fake handle.
    Error registerSessionFiles(Id session,Id registration);
    Error admitCallback(Id session,Id registration,CallbackKind kind,Id causalAdmission,
                        CallbackId &out);
    Error acknowledge(Id session,const CallbackId &callback);
    Error beginClose(Id session,Id registration);
    Error readiness(Id session,Id registration,Readiness &out) const;
    Error beginDrain(Id session);
    Error fail(Id session); // Client outcome unknown; pins/callbacks/last lease retained.
    State state() const { return current; }
    Id activeAdmission() const { return admissionOrdinal; }
    Id lease() const { return leaseId; }
    Id leaseOwner() const { return ownerLane; }
    uint32_t leaseDepth() const { return depth; }
private:
    struct Callback { CallbackKind kind; Id cause; bool acknowledged; };
    enum RegistrationScope { ResourceScoped, SessionFiles };
    struct Registration {
        MilesWire::Handle resource; // Unused, zero storage for SessionFiles.
        RegistrationScope scope;
        Id issued,frontier,closeFrontier;
        bool closing;
        std::map<Id,Callback> callbacks;
        Registration():resource(),scope(ResourceScoped),issued(0),frontier(0),closeFrontier(0),closing(false) {}
    };
    Id incarnation,lastAdmission,lastRegistration,lastLease;
    Id sessionFilesRegistration;
    Limits bounds;
    State current;
    Id admissionOrdinal,admissionLane,admissionLease,ownerLane,leaseId;
    uint32_t depth;
    Action admissionAction;
    std::vector<MilesWire::Handle> pins;
    std::map<Id,Registration> registrations;
    size_t callbackCount;
    Error admit(Id session,Id admissionOrdinal,Id lane,Id lease,Action action,
                const std::vector<MilesWire::Handle> &resources,bool cleanup);
    bool causalPending(Id admissionOrdinal) const;
    Coordinator(const Coordinator &);
    Coordinator &operator=(const Coordinator &);
};
}
#endif
