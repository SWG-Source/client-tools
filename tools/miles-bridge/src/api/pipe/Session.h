#ifndef CLIENT_MILES_PIPE_SESSION_H
#define CLIENT_MILES_PIPE_SESSION_H

#include "PipeCore.h"
#include "Channel.h"
#include <memory>
#include "../../client-runtime/client_file_runtime.h"

namespace ClientMilesPipe {
struct DriverProxy;
struct SampleState;
class ScopedSourceImage;
// Composition-root-only owner; not included by game policy. Bootstrap/Hello
// occurs in the concrete channel before selecting this sole session. Explicit
// normal close remains unavailable pending paired shutdown proof; no public protocol API.
class Session {
  public:
    Session(Channel *channel, MilesClientRuntime53::Runtime &,
            std::shared_ptr<void> callbackCodeLifetime, uint32_t uploadBudgetBytes);
    ~Session();
    void close();

    // Private implementation operations, not an application backend interface.
    StartupBridge::OwnedReply request(uint32_t opcode, const MilesWire::Call &fields,
                                      MilesTransport::Bytes text = MilesTransport::Bytes(),
                                      MilesTransport::Bytes payload = MilesTransport::Bytes());
    bool uncertain() const { return faulted_; }
    size_t sampleProxyCount() const; // private composition/test observation
    void rejectResult(); // terminal uncertainty; no retry after an invalid sample result
    void requireRunning() const;
    void requireAvailable() const;
    // Pure owner/request checks after wire decoding, before admission settlement.
    bool validateReply(uint32_t opcode,const MilesWire::Call &,
        const StartupBridge::OwnedReply &) const;
    void installFiles(ClientMiles::FileOpenCallback, ClientMiles::FileCloseCallback,
        ClientMiles::FileSeekCallback, ClientMiles::FileReadCallback);
    int32_t classifyImage(const void *image, uint32_t bytes);
    int32_t queryWav(const void *image, ClientMiles::SampleInformation *result);
    bool started;
    bool stopped;
    std::unique_ptr<DriverProxy> driver;
    std::unique_ptr<SampleState> samples;
    // Caller must serialize text access with the existing Session command path.
    // Retained with this owner; normal paired destruction remains unavailable.
    std::string lastErrorSnapshot;
    std::string redistSnapshot;
    static Session &selected();

  private:
    Channel *channel_; // retained private root; no automatic teardown in59
    bool closed_;
    bool faulted_;
    MilesClientRuntime53::Runtime &runtime_; // retained by private root, never deleted here
    std::shared_ptr<void> callbackCodeLifetime_;
    bool filesPrepared_, filesInstalled_;
    uint32_t uploadBudgetBytes_;
    enum UploadPhase { UploadIdle, UploadBeginning, UploadChunks, UploadSealing,
        UploadClassifying, UploadReleasing };
    UploadPhase uploadPhase_;
    MilesWire::Handle uploadId_;
    uint32_t uploadBytes_;
    uint32_t uploadOpcode_;
    const ScopedSourceImage *sourceView_; // lexical borrow, never a pin or allocation owner
    friend class ScopedSourceImage;
    StartupBridge::OwnedReply queryImage(const void *,uint32_t,uint32_t);
    std::vector<MilesWire::Handle> verifiedResources(const MilesWire::Call &) const;
    Session(const Session &);
    Session &operator=(const Session &);
};
// Private root/adoption only. Caller keeps allocation readable/stable and uses
// the same serialized owner through scope destruction. Not part of public70.
class ScopedSourceImage {
public:
    ScopedSourceImage(const void *base,uint32_t bytes);
    ~ScopedSourceImage(); // clears only this token; never calls transport
private:
    Session &owner_;
    const void *base_;
    uint32_t bytes_;
    friend class Session;
    ScopedSourceImage(const ScopedSourceImage &);
    ScopedSourceImage &operator=(const ScopedSourceImage &);
};
} // namespace ClientMilesPipe
#endif
