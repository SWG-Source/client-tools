#ifndef PRIVATE_CLIENT_FILE_RUNTIME53_H
#define PRIVATE_CLIENT_FILE_RUNTIME53_H
#include "../transport/endpoint.h"
#include "../file-services/selected_services.h"
#include "../file-control/host_association_mapper.h"
#include <memory>
namespace MilesClientRuntime53 {
// Private modern-runtime composition. No object here crosses the engine STL boundary.
// Caller authenticates raw callback pipe against session before launch; no I/O yet.
// One command caller posts observations; a distinct callback thread owns all control.
// Failure retains the complete root. Only explicit paired close permits destruction.
class Runtime {
public:
    // Throws before ownership transfer on creation failure. On return handle is
    // INVALID_HANDLE_VALUE, even if asynchronous initialization later fails.
    static Runtime *launch(HANDLE &connectedAuthenticatedCallbackPipe, uint64_t session,
        uint64_t backgroundLane, std::shared_ptr<void> engineLifetime);
    bool awaitReady(); // actual Endpoint/coordinator creation, not file installation
    bool prepare(uint64_t registration, ClientMiles::FileOpenCallback,
        ClientMiles::FileCloseCallback, ClientMiles::FileSeekCallback,
        ClientMiles::FileReadCallback, std::shared_ptr<void> callbackLifetime);
    // Publish stable typed-function identity and an already verified live proxy.
    // Callback code remains pinned by this process-lifetime runtime.
    bool prepareEos(const MilesWire::Handle &, uint64_t callback,
        ClientMiles::HSAMPLE, ClientMiles::SampleCallback,
        ClientMiles::HSTREAM, ClientMiles::StreamCallback, std::shared_ptr<void>);
    // Only after genuine native release/close/shutdown and forward settlement.
    bool retireEos(const MilesWire::Handle &);
    bool retireAllEos();
    // Before command send. Resources already verified by genuine forward registry.
    // Fresh local admission chosen here. Action is derived from the trusted opcode,
    // and lease is the value returned after the previous settled command.
    bool publish(uint64_t wire, uint64_t lane,
        const std::vector<MilesWire::Handle> &resources,
        MilesCoordinator::Action action=MilesCoordinator::Ordinary, uint64_t lease=0);
    // Only after full opcode-specific reply validation. Waits for causal ACK join
    // then consumes the settled command. This does not certify SDK semantic success.
    bool returned(uint64_t wire, bool actionApplied=true, uint64_t *settledLease=0);
    // After genuine SDK shutdown and its causal joins, before SessionClose send.
    bool armClose();
    // Only after exact SessionClose success and child exit 0; consumes this on success.
    bool finishClose();
    void fail(); // async request; control thread alone mutates coordinator/Endpoint
    bool failed() const;
    HANDLE failureEvent() const; // borrowed; outer command wait must observe it
private:
    struct State;
    State *state;
    explicit Runtime(State *);
    ~Runtime(); // deliberately unavailable; no destructor-based cleanup promise
    Runtime(const Runtime &);
    Runtime &operator=(const Runtime &);
};
}
#endif
