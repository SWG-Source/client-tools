#ifndef MILES_HOST_BINK_OWNER_H
#define MILES_HOST_BINK_OWNER_H
#include <windows.h>
#include <Mss.h>
#include <bink.h>
#include <stdint.h>
#include <vector>
#include <memory>

namespace MilesHostBink {
typedef U32 (RADLINK *TimerRead)();
struct Metadata {
    uint32_t width, height, frames, frame, lastFrame, rate, rateDiv;
};
class Video;
// Command-thread owner. The caller retains driver and real IO callback state
// until every Video is explicitly closed. Runtime/DLL remain until process exit.
// The budget bounds aggregate copied pixel buffers, not Bink's internal heap.
class Runtime {
public:
    Runtime(const wchar_t *absoluteDllPath, uint32_t pixelBudget);
    // One attempt only; exact native sound-init status. Zero leaves a terminal
    // retained owner: caller must not retry or release uncertain callback roots.
    int32_t initialize(HDIGDRIVER driver, BINKIOOPEN io, uint32_t ioBytes);
    ~Runtime();
    std::unique_ptr<Video> open(const char *name);
    const char *error() const;
    TimerRead timerRead() const;
private:
    friend class Video;
    struct Api;
    std::unique_ptr<Api> api_;
    HMODULE module_;
    DWORD thread_;
    uint32_t budget_, used_, live_;
    bool attempted_, initialized_;
    void check() const;
    Runtime(const Runtime &);
    Runtime &operator=(const Runtime &);
};
class Video {
public:
    ~Video();
    Metadata metadata() const;
    int32_t doFrame();
    int32_t wait();
    int32_t shouldSkip();
    void next();
    void service();
    int32_t pause(bool paused);
    int32_t videoOnOff(bool enabled);
    int32_t soundOnOff(bool enabled);
    void setVolume(uint32_t track, int32_t volume);
    // Native 32A/565/5551 with BINKCOPYALL and tight pitch. Native signed status
    // is preserved, including zero. Pixel view is invalidated by next copy/close.
    int32_t copyFrame(uint32_t format);
    const std::vector<unsigned char> &pixels() const;
    void close();
private:
    friend class Runtime;
    explicit Video(Runtime &);
    Runtime &owner_;
    HBINK handle_;
    uint32_t copiedWidth_, copiedHeight_;
    std::vector<unsigned char> pixels_;
    void check() const;
    Video(const Video &);
    Video &operator=(const Video &);
};
}
#endif
