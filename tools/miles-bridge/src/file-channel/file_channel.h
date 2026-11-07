#ifndef MILES_FILE_CHANNEL26_H
#define MILES_FILE_CHANNEL26_H

#include "../audio-callbacks/ClientAudioFileCallbacks.h"
#include "../wire/codec.h"
#include <memory>
#include <string>

// Private value adapter, not an engine scheduler or SDK callback table.
namespace MilesFileChannel26
{
struct Association
{
    uint64_t request, causal, lane, lease;
};

enum Validation
{
    Valid,
    Malformed,
    WrongAssociation,
    UnsupportedExtent,
    InvalidFields,
    InvalidResult
};

class Request
{
public:
    Request();
    uint32_t opcode() const;
    MilesWire::Handle target() const;
    uint32_t count() const;
    int32_t offset() const;
    uint32_t origin() const;
    const std::string &name() const;
    const MilesWire::Header &header() const;
    bool valid() const;

private:
    MilesWire::Header envelope;
    MilesWire::Call call;
    std::string filename;
    bool validated;
    friend Validation decodeRequest(MilesTransport::Bytes, const Association &, Request &);
};

Validation decodeRequest(MilesTransport::Bytes frame, const Association &expected,
                         Request &out);

// Private context-aware services. Each invocation retains this selected owner.
// These ordinary C++ thunks adapt the native callback ABI; no filesystem here.
struct FileServices
{
    std::shared_ptr<const void> owner;
    ClientAudioFileCallbacks::OpenResult (*open)(const void *, const char *);
    void (*close)(const void *, ClientAudioFileCallbacks::LocalFileHandle);
    int32_t (*seek)(const void *, ClientAudioFileCallbacks::LocalFileHandle, int32_t, uint32_t);
    uint32_t (*read)(const void *, ClientAudioFileCallbacks::LocalFileHandle, void *, uint32_t);
};
// No implicit canonical fallback: composition must supply the installed table.

struct Binding
{
    const MilesWire::Handle identity;
    const ClientAudioFileCallbacks::LocalFileHandle local;
    Binding(MilesWire::Handle wire, ClientAudioFileCallbacks::LocalFileHandle key);
private:
    Binding &operator=(const Binding &);
};

// CallThrew leaves possible side effects unknown. It authorizes neither retry
// nor retirement/cleanup of the file binding or an unpublished successful open.
enum CompletionState { Returned, ReadCountOutsideBuffer, CallThrew };
struct Completion
{
    CompletionState state;
    uint32_t returnBits;
    ClientAudioFileCallbacks::OpenResult opened;
    std::vector<unsigned char> bytes;
    Completion();
};

// Must be retained by the chosen executor until invokeOnAdmittedExecutor returns.
// Serialized executor ownership is required; concurrent entry is unsupported.
// This class supplies no thread, cancellation, file retirement or engine policy.
// The lifetime pin retains caller-owned context; it does not prove TLS/affinity.
class Invocation
{
public:
    Invocation(const Request &, const FileServices &, std::shared_ptr<const Binding>,
               std::shared_ptr<void> lifetimePin);
    bool invokeOnAdmittedExecutor(); // false means duplicate, never another call.
    bool finished() const;
    const Completion &completion() const;
    const Request &request() const;

private:
    Request operation;
    FileServices services;
    std::shared_ptr<const Binding> binding;
    std::shared_ptr<void> context;
    Completion result;
    bool begun, returned;
    Invocation(const Invocation &);
    Invocation &operator=(const Invocation &);
};

// Publication identity comes from the trusted external file owner, never local
// key bits. Successful open requires one; failed open and other results forbid it.
// Rejected completion does not alter the frame and never fabricates EOF.
Validation encodeCompletion(const Invocation &, MilesWire::Handle publishedFile,
                            std::vector<unsigned char> &frame);

Validation encodeCompletionInto(const Invocation &, MilesWire::Handle publishedFile,
                                unsigned char *, size_t capacity, size_t &written);

struct OwnedReply
{
    uint32_t returnBits;
    MilesWire::Handle file;
    std::vector<unsigned char> bytes;
    OwnedReply();
};
Validation decodeReply(MilesTransport::Bytes, const Request &, OwnedReply &);
Validation copyRead(const OwnedReply &, uint32_t requested, void *destination,
                    size_t capacity);
}

#endif
