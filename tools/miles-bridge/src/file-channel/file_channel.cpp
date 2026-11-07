#include "file_channel.h"
#include "../callback-guard/invocation_guard.h"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace MilesFileChannel26
{
namespace
{
bool nullHandle(const MilesWire::Handle &value)
{
    return !value.kind && !value.slot && !value.generation;
}

bool fileHandle(const MilesWire::Handle &value)
{
    return value.kind == MilesWire::File && value.slot && value.generation;
}

bool same(const MilesWire::Handle &a, const MilesWire::Handle &b)
{
    return a.kind == b.kind && a.slot == b.slot && a.generation == b.generation;
}

bool association(const MilesWire::Header &header, const Association &expected)
{
    return expected.request && expected.lane && header.request == expected.request &&
        header.causal_request == expected.causal && header.lane == expected.lane &&
        header.lock_lease == expected.lease;
}

bool zeroValues(const uint32_t *values, unsigned first)
{
    for (unsigned i = first; i < 8; ++i)
    {
        if (values[i])
            return false;
    }
    return true;
}
}

Request::Request() : envelope(), call(), validated(false) {}
uint32_t Request::opcode() const { return envelope.opcode; }
MilesWire::Handle Request::target() const { return call.target; }
uint32_t Request::count() const { return call.value[0]; }
int32_t Request::offset() const
{
    int32_t value;
    std::memcpy(&value, &call.value[0], sizeof(value));
    return value;
}
uint32_t Request::origin() const { return call.value[1]; }
const std::string &Request::name() const { return filename; }
const MilesWire::Header &Request::header() const { return envelope; }
bool Request::valid() const { return validated; }

Validation decodeRequest(MilesTransport::Bytes frame, const Association &expected,
                         Request &out)
{
    Request candidate;
    if (!MilesTransport::decodeCall(frame, candidate.envelope, candidate.call))
        return Malformed;
    const MilesWire::Header &header = candidate.envelope;
    const MilesWire::Call &call = candidate.call;
    if (header.kind != MilesWire::ReverseRequest || !association(header, expected))
        return WrongAssociation;
    if (header.opcode < MilesWire::FileOpen || header.opcode > MilesWire::FileRead ||
        !nullHandle(call.resource) || call.output_mask || call.callback || call.bytes.length)
        return InvalidFields;
    if (header.opcode == MilesWire::FileOpen)
    {
        if (!nullHandle(call.target) || !zeroValues(call.value, 0) || !call.text.length)
            return InvalidFields;
        if (call.text.length > 512)
            return UnsupportedExtent;
        const unsigned char *text = frame.data + call.text.offset;
        if (text[call.text.length - 1] ||
            std::memchr(text, 0, call.text.length - 1))
            return InvalidFields;
        candidate.filename.assign(reinterpret_cast<const char *>(text), call.text.length - 1);
    }
    else
    {
        if (!fileHandle(call.target) || call.text.length)
            return InvalidFields;
        unsigned used = 0;
        if (header.opcode == MilesWire::FileSeek)
        {
            used = 2;
            if (call.value[1] > ClientAudioFileCallbacks::SeekEnd)
                return InvalidFields;
        }
        if (header.opcode == MilesWire::FileRead)
        {
            used = 1;
            if (call.value[0] > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
                call.value[0] > MilesWire::MaxFrameBytes - 128)
                return UnsupportedExtent;
        }
        if (!zeroValues(call.value, used))
            return InvalidFields;
    }
    candidate.validated = true;
    out.filename.swap(candidate.filename);
    out.envelope = candidate.envelope;
    out.call = candidate.call;
    out.validated = true;
    return Valid;
}

Binding::Binding(MilesWire::Handle wire, ClientAudioFileCallbacks::LocalFileHandle key)
    : identity(wire), local(key) {}

Completion::Completion() : state(Returned), returnBits(0), opened() {}

Invocation::Invocation(const Request &request, const FileServices &table,
                       std::shared_ptr<const Binding> file, std::shared_ptr<void> lifetimePin)
    : operation(request), services(table), binding(file), context(lifetimePin),
      begun(false), returned(false)
{
    if (!operation.valid() || !context || !services.owner || !services.open || !services.close ||
        !services.seek || !services.read)
        throw std::invalid_argument("validated operation, services and lifetime pin required");
    if (operation.opcode() == MilesWire::FileOpen)
    {
        if (binding)
            throw std::invalid_argument("open has no existing file binding");
    }
    else if (!binding || !same(binding->identity, operation.target()))
        throw std::invalid_argument("exact live file binding required");
    if (operation.opcode() == MilesWire::FileRead)
        result.bytes.resize(operation.count() ? operation.count() : 1);
        // All allocation precedes the callback; even a zero-byte call gets storage.
}

bool Invocation::invokeOnAdmittedExecutor()
{
    if (begun)
        return false;
    begun = true;
    MilesCallbackGuard47::Scope callbackScope;
    try
    {
        if (!callbackScope.admitted())
            throw MilesCallbackGuard47::ReentryDenied();
        switch (operation.opcode())
        {
        case MilesWire::FileOpen:
            result.opened = services.open(services.owner.get(), operation.name().c_str());
            result.returnBits = result.opened.callbackResult;
            break;
        case MilesWire::FileClose:
            services.close(services.owner.get(), binding->local);
            break;
        case MilesWire::FileSeek:
            result.returnBits = static_cast<uint32_t>(
                services.seek(services.owner.get(), binding->local, operation.offset(), operation.origin()));
            break;
        case MilesWire::FileRead:
            result.returnBits = services.read(services.owner.get(), binding->local,
                result.bytes.empty() ? 0 : &result.bytes[0], operation.count());
            if (result.returnBits > operation.count())
            {
                result.state = ReadCountOutsideBuffer;
                result.bytes.clear();
            }
            else
                result.bytes.resize(result.returnBits);
            break;
        }
    }
    catch (...)
    {
        // An exception is not an observed SDK return or permission to retry.
        result.state = CallThrew;
        result.bytes.clear();
    }
    if (callbackScope.violated())
    {
        result.state = CallThrew;
        result.bytes.clear();
    }
    returned = true;
    return true;
}

bool Invocation::finished() const { return returned; }
const Completion &Invocation::completion() const
{
    if (!returned)
        throw std::logic_error("operation has not returned");
    return result;
}
const Request &Invocation::request() const { return operation; }

Validation encodeCompletion(const Invocation &work, MilesWire::Handle publishedFile,
                            std::vector<unsigned char> &frame)
{
    if (!work.finished())
        return InvalidResult;
    const Completion &completion = work.completion();
    if (completion.state != Returned)
        return InvalidResult;
    const Request &operation = work.request();
    MilesWire::Result result = {};
    result.return_bits = completion.returnBits;
    if (operation.opcode() == MilesWire::FileOpen && completion.returnBits)
    {
        if (!fileHandle(publishedFile))
            return InvalidResult;
        result.resource = publishedFile;
    }
    else if (!nullHandle(publishedFile))
        return InvalidResult;
    MilesWire::Header header = operation.header();
    header.kind = MilesWire::ReverseReply;
    MilesTransport::Bytes bytes(completion.bytes.empty() ? 0 : &completion.bytes[0],
                                completion.bytes.size());
    return MilesTransport::encodeResult(header, result, bytes, MilesTransport::Bytes(), frame)
        ? Valid : InvalidResult;
}

Validation encodeCompletionInto(const Invocation &work, MilesWire::Handle publishedFile,
                            unsigned char *buffer, size_t capacity, size_t &written)
{
    if (!work.finished())
        return InvalidResult;
    const Completion &completion = work.completion();
    if (completion.state != Returned)
        return InvalidResult;
    const Request &operation = work.request();
    MilesWire::Result result = {};
    result.return_bits = completion.returnBits;
    if (operation.opcode() == MilesWire::FileOpen && completion.returnBits)
    {
        if (!fileHandle(publishedFile))
            return InvalidResult;
        result.resource = publishedFile;
    }
    else if (!nullHandle(publishedFile))
        return InvalidResult;
    MilesWire::Header header = operation.header();
    header.kind = MilesWire::ReverseReply;
    MilesTransport::Bytes bytes(completion.bytes.empty() ? 0 : &completion.bytes[0],
                                completion.bytes.size());
    return MilesTransport::encodeResultInto(header, result, bytes, MilesTransport::Bytes(), buffer, capacity, written)
        ? Valid : InvalidResult;
}

OwnedReply::OwnedReply() : returnBits(0), file() {}

Validation decodeReply(MilesTransport::Bytes frame, const Request &request, OwnedReply &out)
{
    MilesWire::Header header;
    MilesWire::Result result;
    if (!request.valid() || !MilesTransport::decodeResult(frame, header, result))
        return Malformed;
    const MilesWire::Header &original = request.header();
    Association expected = {original.request, original.causal_request,
                            original.lane, original.lock_lease};
    if (header.kind != MilesWire::ReverseReply || header.opcode != request.opcode() ||
        !association(header, expected))
        return WrongAssociation;
    if (result.transport_status || result.text.length || result.null_mask || result.callback ||
        !zeroValues(result.value, 0))
        return InvalidResult;
    if (request.opcode() == MilesWire::FileOpen && result.return_bits)
    {
        if (!fileHandle(result.resource))
            return InvalidResult;
    }
    else if (!nullHandle(result.resource))
        return InvalidResult;
    if (request.opcode() == MilesWire::FileClose && result.return_bits)
        return InvalidResult;
    if (request.opcode() == MilesWire::FileRead)
    {
        if (result.return_bits > request.count() || result.bytes.length != result.return_bits)
            return InvalidResult;
    }
    else if (result.bytes.length)
        return InvalidResult;
    OwnedReply candidate;
    candidate.returnBits = result.return_bits;
    candidate.file = result.resource;
    if (result.bytes.length)
        candidate.bytes.assign(frame.data + result.bytes.offset,
                               frame.data + result.bytes.offset + result.bytes.length);
    out.bytes.swap(candidate.bytes);
    out.returnBits = candidate.returnBits;
    out.file = candidate.file;
    return Valid;
}

Validation copyRead(const OwnedReply &reply, uint32_t requested, void *destination,
                    size_t capacity)
{
    if (!nullHandle(reply.file) || reply.returnBits > requested ||
        reply.bytes.size() != reply.returnBits || capacity < requested ||
        (requested && !destination))
        return InvalidResult;
    if (reply.returnBits)
        std::memcpy(destination, &reply.bytes[0], reply.returnBits);
    return Valid;
}
}
