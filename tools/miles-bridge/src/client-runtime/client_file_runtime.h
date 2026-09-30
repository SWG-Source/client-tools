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
// This slice has NO destroy/normal-stop API: outer root must retain returned object
// through process termination pending a future proven teardown implementation.
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
    // Before command send. Resources already verified by genuine forward registry.
    // Fresh local admission chosen here; ordinary commands only, lease must be zero.
    bool publish(uint64_t wire, uint64_t lane,
        const std::vector<MilesWire::Handle> &resources);
    // Only after full opcode-specific reply validation. Waits for causal ACK join
    // then consumes the settled command. This does not certify SDK semantic success.
    bool returned(uint64_t wire);
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
