#ifndef CLIENT_MILES_PIPE_SESSION_H
#define CLIENT_MILES_PIPE_SESSION_H

#include "PipeCore.h"
#include "ScopedSourceImage.h"
#include "Channel.h"
#include <memory>
#include "../../client-runtime/client_file_runtime.h"

namespace ClientMilesPipe {
struct DriverProxy;
struct SampleState;
struct VideoState;
// Composition-root-only owner; not included by game policy. Bootstrap/Hello
// occurs in the concrete channel before selecting this sole session. Explicit
// close follows genuine SDK shutdown and paired joins; no public protocol API.
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
    void prepareEos(const MilesWire::Handle &, uint64_t, ClientMiles::HSAMPLE,
        ClientMiles::SampleCallback, ClientMiles::HSTREAM, ClientMiles::StreamCallback);
    void retireEos(const MilesWire::Handle &);
    void retireAllEos();
    int32_t classifyImage(const void *image, uint32_t bytes);
    int32_t queryWav(const void *image, ClientMiles::SampleInformation *result);
    int32_t bindSampleImage(const MilesWire::Handle &sample, const void *image,
        uint32_t bytes, uint32_t opcode, int32_t block, const char *suffix);
    MilesWire::Handle verifiedDriver(ClientMiles::HDIGDRIVER);
    bool fileCallbacksInstalled() const { return filesInstalled_; }
    bool started;
    bool stopped;
    std::unique_ptr<DriverProxy> driver;
    std::unique_ptr<SampleState> samples;
    std::unique_ptr<VideoState> videos;
    // Caller must serialize text access with the existing Session command path.
    // Retained with this owner until explicit paired close.
    std::string lastErrorSnapshot;
    std::string redistSnapshot;
    static Session &selected();

  private:
    const DWORD commandThread_; // bound command caller; callback owner is separate
    void requireCommandThread() const;
    Channel *channel_; // retained on failure; explicit close only
    bool closed_;
    bool faulted_;
    MilesClientRuntime53::Runtime &runtime_; // consumed by channel finish on successful close
    std::shared_ptr<void> callbackCodeLifetime_;
    bool filesPrepared_, filesInstalled_;
    uint32_t uploadBudgetBytes_;
    enum UploadPhase { UploadIdle, UploadBeginning, UploadChunks, UploadSealing,
        UploadClassifying, UploadReleasing };
    UploadPhase uploadPhase_;
    MilesWire::Handle uploadId_;
    uint32_t uploadBytes_;
    uint32_t uploadOpcode_;
    MilesWire::Handle uploadTarget_;
    uint32_t uploadBlock_;
    const ScopedSourceImage *sourceView_; // lexical borrow, never a pin or allocation owner
    friend class ScopedSourceImage;
    StartupBridge::OwnedReply queryImage(const void *,uint32_t,uint32_t,
        const MilesWire::Handle &target = MilesWire::Handle(), uint32_t block = 0,
        MilesTransport::Bytes text = MilesTransport::Bytes());
    std::vector<MilesWire::Handle> verifiedResources(const MilesWire::Call &) const;
    Session(const Session &);
    Session &operator=(const Session &);
};
} // namespace ClientMilesPipe
#endif
