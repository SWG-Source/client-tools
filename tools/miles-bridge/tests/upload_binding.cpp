// Portable ownership-logic test. NativeBind is a same-module model, not an SDK
// export or evidence that a particular Miles DLL releases old binding pointers.
#include "../src/wire/resource_registry.h"
#include "../src/upload/upload_state.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
void check(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
struct Observation {
    const unsigned char *image;
    const char *suffix;
    uint32_t bytes, opcode;
    int32_t block;
    void *sample;
};
std::vector<Observation> observed;
int32_t nativeStatus = 1;
bool throwDuringBind = false;
int32_t modeledBind(uint32_t opcode, void *sample, const char *suffix,
                    const void *image, uint32_t bytes, int32_t block) {
    Observation row = {static_cast<const unsigned char *>(image), suffix,
                       bytes, opcode, block, sample};
    observed.push_back(row);
    if (throwDuringBind) throw std::runtime_error("modeled uncertain native completion");
    return nativeStatus;
}
MilesHostUpload106::QueryResult noQuery(uint32_t, const void *, uint32_t) {
    throw std::runtime_error("unexpected classification call");
}
struct Fixture {
    MilesTransport::ResourceRegistry registry;
    MilesHostUpload106::UploadState state;
    int driverLocal, sampleLocal, streamLocal, borrowedLocal;
    MilesWire::Handle driver, sample, stream, borrowed;
    explicit Fixture(uint32_t budget = 1024) : state(registry, budget),
        driverLocal(1), sampleLocal(2), streamLocal(3), borrowedLocal(4),
        driver(), sample(), stream(), borrowed() {
        observed.clear();nativeStatus = 1;throwDuringBind = false;
        check(registry.insert(MilesWire::Driver, &driverLocal, driver), "driver identity");
        publish(MilesWire::OwnedSample, &sampleLocal, sample);
        publish(MilesWire::Stream, &streamLocal, stream);
        check(registry.insertBorrowed(stream, &borrowedLocal, borrowed), "borrowed identity");
    }
    void publish(MilesWire::ResourceKind kind, void *local, MilesWire::Handle &id) {
        MilesTransport::ResourceRegistry::Reservation reservation;
        check(registry.reserve(kind, driver, reservation), "reserve child");
        check(registry.publish(reservation, local, id), "publish child");
    }
    StartupBridge::OwnedReply call(uint32_t opcode, const MilesWire::Call &fields,
                                  const std::vector<unsigned char> &frame = std::vector<unsigned char>()) {
        StartupBridge::OwnedReply reply;
        check(state.intercept(opcode, fields, frame, true, false,
                              noQuery, check, reply, modeledBind), "upload intercepted");
        return reply;
    }
    uint32_t begin(uint32_t size) {
        MilesWire::Call fields = {};fields.value[0] = size;
        return call(MilesWire::BufferBegin, fields).result.transport_status;
    }
    void sealed(unsigned char value, uint32_t size = 16) {
        check(begin(size) == StartupBridge::Success, "begin upload");
        std::vector<unsigned char> image(size, value);
        MilesWire::Call fields = {};fields.target = state.uploadId;
        fields.bytes.length = size;
        check(call(MilesWire::BufferChunk, fields, image).result.transport_status == StartupBridge::Success,
              "complete upload chunk");
        fields = MilesWire::Call();fields.target = state.uploadId;
        check(call(MilesWire::BufferSeal, fields).result.transport_status == StartupBridge::Success,
              "seal upload");
    }
    MilesWire::Call fields() const {
        MilesWire::Call result = {};result.target = sample;result.resource = state.uploadId;
        result.value[0] = 16;result.value[1] = static_cast<uint32_t>(-7);
        return result;
    }
    int32_t bind(const char *suffix = 0, bool named = true) {
        MilesWire::Call request = fields();
        std::vector<unsigned char> frame;
        if (suffix) {
            frame.assign(suffix, suffix + std::strlen(suffix) + 1);
            request.text.length = static_cast<uint32_t>(frame.size());
        }
        const uint32_t opcode = named ? MilesWire::AIL_set_named_sample_file : MilesWire::AIL_set_sample_file;
        const StartupBridge::OwnedReply reply = call(opcode, request, frame);
        check(reply.result.transport_status == StartupBridge::Success, "native status is transport success");
        int32_t status = 0;
        std::memcpy(&status, &reply.result.return_bits, sizeof status);
        return status;
    }
    void releaseUpload() {
        const MilesWire::Handle old = state.uploadId;
        MilesWire::Call fields = {};fields.target = old;
        check(call(MilesWire::BufferRelease, fields).result.transport_status == StartupBridge::Success,
              "release temporary upload");
        void *local = 0;
        check(!state.active() && state.uploadChargedBytes == 0 && state.sealedImage.empty(),
              "temporary ownership discharged");
        check(!registry.resolve(old, MilesWire::Buffer, local), "upload identity retired");
    }
};

void ownershipAndStatuses() {
    Fixture f;
    f.sealed(0x11);check(f.bind(".wav") == 1, "initial success");f.releaseUpload();
    check(f.state.retainedBytes() == 21, "image and suffix retained past BufferRelease");
    check(observed[0].image[15] == 0x11 && std::strcmp(observed[0].suffix, ".wav") == 0,
          "retained pointers remain readable");
    check(observed[0].sample == &f.sampleLocal && observed[0].block == -7 &&
          observed[0].opcode == MilesWire::AIL_set_named_sample_file, "native arguments exact");
    nativeStatus = 0;f.sealed(0x22);check(f.bind("") == 0, "native zero preserved");f.releaseUpload();
    check(f.state.retainedBytes() == 38, "zero keeps prior and attempted inputs");
    check(observed[0].image[0] == 0x11 && observed[1].image[0] == 0x22,
          "both zero-result inputs still readable");
    check(observed[1].suffix && observed[1].suffix[0] == 0, "empty suffix remains nonnull");
    nativeStatus = -3;f.sealed(0x33);check(f.bind() == -3, "negative signed status preserved");f.releaseUpload();
    check(!observed[2].suffix && f.state.retainedBytes() == 16,
          "nonnull native status prunes earlier bindings, null suffix preserved");
    check(observed[2].image[15] == 0x33, "replacement survives upload release");
    f.state.releasedSample(f.borrowed);
    check(f.state.retainedBytes() == 16, "unrelated release does not discard owned binding");
    // Model notification only: no genuine SDK call is made by this test.
    check(f.registry.beginClose(f.sample) && f.registry.retire(f.sample), "model sample release");
    f.state.releasedSample(f.sample);
    check(f.state.retainedBytes() == 0, "explicit completed-release notification discards binding");
}
void boundedSuccessAndShutdown() {
    Fixture f(64);
    for (unsigned n = 0; n < 65; ++n) {
        f.sealed(static_cast<unsigned char>(n));check(f.bind(0, false) == 1, "unnamed success");
        f.releaseUpload();
        check(f.state.retainedBytes() == 16 && observed.back().image[0] == n,
              "repeated success stays within one image charge");
    }
    nativeStatus = 0;f.sealed(0x77);check(f.bind() == 0, "failed final attempt");f.releaseUpload();
    check(f.state.retainedBytes() == 32, "two retained before shutdown");
    f.state.shutdownComplete();
    check(f.state.retainedBytes() == 0 && !f.state.active(), "completed shutdown discards retained inputs");
}
void rejectedInputs() {
    // Every refused active transaction is terminal, so isolate each malformed call.
    for (unsigned which = 0; which < 9; ++which) {
        Fixture f;f.sealed(0x44);
        MilesWire::Call c = f.fields();
        std::vector<unsigned char> text;
        uint32_t opcode = MilesWire::AIL_set_named_sample_file;
        uint32_t expected = StartupBridge::InvalidFields;
        if (which == 0) { c.target = f.borrowed;expected = StartupBridge::InvalidResource; }
        if (which == 1) { ++c.target.generation;expected = StartupBridge::InvalidResource; }
        if (which == 2) c.value[0] = 15;
        if (which == 3) c.value[2] = 1; // Block occupies value[1]; later values are unused.
        if (which == 4) { text.push_back('x');c.text.length = 1; }
        if (which == 5) { text.assign(3, 0);c.text.length = 3; }
        if (which == 6) { text.push_back(0);c.text.length = 1;opcode = MilesWire::AIL_set_sample_file; }
        if (which == 7) c.output_mask = 1;
        if (which == 8) { c.text.offset = 1;c.text.length = 1; }
        check(f.call(opcode, c, text).result.transport_status == expected, "malformed binding refused");
        check(observed.empty() && f.state.failed() && f.state.retainedBytes() == 0,
              "refusal never invokes bind or retains attempted input");
    }
}
void aggregateBudget() {
    Fixture f(48);
    f.sealed(1);f.bind();f.releaseUpload();
    check(f.begin(17) == StartupBridge::InputBudgetExceeded && !f.state.active() && !f.state.failed(),
          "retained image plus double new image charge rejects oversize begin");
    f.sealed(2); // Exactly 16 retained + 32 temporary bytes.
    check(f.state.retainedBytes() + f.state.uploadChargedBytes == 48, "aggregate admission exact boundary");
    MilesWire::Call c = f.fields();c.text.length = 1;
    const std::vector<unsigned char> emptySuffix(1, 0);
    check(f.call(MilesWire::AIL_set_named_sample_file, c, emptySuffix).result.transport_status ==
          StartupBridge::InputBudgetExceeded, "suffix byte participates in aggregate budget");
    check(observed.size() == 1 && f.state.retainedBytes() == 16 && f.state.failed(),
          "suffix budget refusal precedes native effect");
}
void uncertainCompletion() {
    Fixture f;
    f.sealed(0x41);f.bind(".wav");f.releaseUpload();
    f.sealed(0x42);throwDuringBind = true;
    bool threw = false;
    try { f.bind(".wav"); }
    catch (const std::runtime_error &) { threw = true; }
    check(threw && f.state.failed() && f.state.retainedBytes() == 42,
          "uncertain effect retains old and attempted binding");
    check(observed.size() == 2 && observed[0].image[0] == 0x41 && observed[1].image[0] == 0x42,
          "uncertain pointers remain readable");
    threw = false;
    try { f.releaseUpload(); }
    catch (const std::runtime_error &) { threw = true; }
    check(threw && f.state.active(), "uncertain transaction cannot resume to release input");
}
}
int main() {
    try {
        ownershipAndStatuses();boundedSuccessAndShutdown();rejectedInputs();aggregateBudget();uncertainCompletion();
        std::puts("PASS: modeled upload binding ownership, statuses, validation, and aggregate budget (not SDK equivalence)");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());return 1;
    }
}
