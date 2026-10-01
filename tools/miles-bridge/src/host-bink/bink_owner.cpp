#include "bink_owner.h"
#include <stdexcept>
#include <exception>
#include <new>
#include <cstring>
#ifdef _WIN64
#error Genuine Bink 1.9c owner requires x86
#endif
static_assert(sizeof(void *) == 4, "Bink host pointer width");
static_assert(BINKMAJORVERSION == 1 && BINKMINORVERSION == 9 && BINKSUBVERSION == 3,
              "Selected DLL uses Bink 1.9c ABI");
namespace MilesHostBink {
namespace {
void *RADLINK allocate(U32 bytes) {
    try { return new unsigned char[bytes]; } catch (...) { std::terminate(); }
}
void RADLINK release(void *memory) { delete [] static_cast<unsigned char *>(memory); }
Runtime *active = 0; // Bink IO/audio settings are DLL globals; one owner per host.
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template<class T> void bind(HMODULE module, T &out, const char *name) {
    FARPROC address = GetProcAddress(module, name);
    require(address != 0, name);
    static_assert(sizeof(out) == sizeof(address), "native function address size");
    std::memcpy(&out, &address, sizeof(out));
}
}
struct Runtime::Api {
    TimerRead timer;
    decltype(&::BinkSetMemory) setMemory;
    decltype(&::BinkOpen) open;
    decltype(&::BinkClose) close;
    decltype(&::BinkDoFrame) doFrame;
    decltype(&::BinkWait) wait;
    decltype(&::BinkShouldSkip) shouldSkip;
    decltype(&::BinkNextFrame) next;
    decltype(&::BinkService) service;
    decltype(&::BinkPause) pause;
    decltype(&::BinkSetVideoOnOff) video;
    decltype(&::BinkSetSoundOnOff) soundOn;
    decltype(&::BinkSetVolume) volume;
    decltype(&::BinkCopyToBuffer) copy;
    decltype(&::BinkSetIO) setIo;
    decltype(&::BinkSetIOSize) setIoSize;
    decltype(&::BinkSetSoundSystem) sound;
    decltype(&::BinkOpenMiles) miles;
    decltype(&::BinkGetError) error;
};
Runtime::Runtime(const wchar_t *path, uint32_t pixelBudget)
    : api_(new Api()), module_(0), thread_(GetCurrentThreadId()),
      io_(0), ioBytes_(0), budget_(pixelBudget), used_(0), live_(0), attempted_(false), initialized_(false) {
    require(!active && path && *path && pixelBudget,
            "Bink explicit path/budget and single owner required");
    require((wcslen(path) >= 3 && path[1] == L':' && (path[2] == L'\\' || path[2] == L'/')) ||
            (path[0] == L'\\' && path[1] == L'\\'), "Bink absolute DLL path required");
    module_ = LoadLibraryExW(path, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
    require(module_ != 0, "Load genuine Bink DLL");
    try {
#define B(member, name) bind(module_, api_->member, name)
        B(timer,"_RADTimerRead@0"); B(setMemory,"_BinkSetMemory@8");
        B(open,"_BinkOpen@8"); B(close,"_BinkClose@4"); B(doFrame,"_BinkDoFrame@4");
        B(wait,"_BinkWait@4"); B(shouldSkip,"_BinkShouldSkip@4"); B(next,"_BinkNextFrame@4");
        B(service,"_BinkService@4"); B(pause,"_BinkPause@8");
        B(video,"_BinkSetVideoOnOff@8"); B(soundOn,"_BinkSetSoundOnOff@8"); B(volume,"_BinkSetVolume@12");
        B(copy,"_BinkCopyToBuffer@28"); B(setIo,"_BinkSetIO@4"); B(setIoSize,"_BinkSetIOSize@4");
        B(sound,"_BinkSetSoundSystem@8"); B(miles,"_BinkOpenMiles@4"); B(error,"_BinkGetError@0");
#undef B
    } catch (...) { FreeLibrary(module_); module_ = 0; throw; }
    active = this;
}
int32_t Runtime::initialize(HDIGDRIVER driver, BINKIOOPEN io, uint32_t ioBytes) {
    check(); require(!attempted_ && driver && io && ioBytes, "Bink initialization inputs/one attempt");
    attempted_ = true;
    api_->setMemory(&allocate, &release);
    int32_t const status = api_->sound(api_->miles, reinterpret_cast<UINTa>(driver));
    if (!status) return status; // Keep owner/module and caller's roots on uncertain failure.
    io_ = io;
    ioBytes_ = ioBytes;
    initialized_ = true;
    return status;
}
Runtime::~Runtime() { if (module_) std::terminate(); }
void Runtime::check() const {
    require(module_ && active == this && GetCurrentThreadId() == thread_, "Bink command owner/state");
}
const char *Runtime::error() const { check(); return api_->error(); }
TimerRead Runtime::timerRead() const { check(); return api_->timer; }
std::unique_ptr<Video> Runtime::open(const char *name) {
    check(); require(initialized_ && name && *name, "Bink initialized/filename required");
    std::unique_ptr<Video> video(new Video(*this));
    // BinkOpen consumes these settings. Match BinkVideo's per-open setup so
    // later movies still use the engine's TreeFile callbacks and IO buffer.
    api_->setIo(io_);
    api_->setIoSize(ioBytes_);
    video->handle_ = api_->open(name, BINKIOPROCESSOR | BINKIOSIZE);
    if (!video->handle_) return std::unique_ptr<Video>();
    ++live_;
    return video;
}
Video::Video(Runtime &owner) : owner_(owner), handle_(0), copiedWidth_(0), copiedHeight_(0) {}
Video::~Video() { if (handle_) std::terminate(); }
void Video::check() const { owner_.check(); require(handle_ != 0, "Bink video closed"); }
Metadata Video::metadata() const {
    check(); Metadata out = {handle_->Width,handle_->Height,handle_->Frames,handle_->FrameNum,
                             handle_->LastFrameNum,handle_->FrameRate,handle_->FrameRateDiv};
    return out;
}
int32_t Video::doFrame() { check(); return owner_.api_->doFrame(handle_); }
int32_t Video::wait() { check(); return owner_.api_->wait(handle_); }
int32_t Video::shouldSkip() { check(); return owner_.api_->shouldSkip(handle_); }
void Video::next() { check(); owner_.api_->next(handle_); }
void Video::service() { check(); owner_.api_->service(handle_); }
int32_t Video::pause(bool paused) { check(); return owner_.api_->pause(handle_, paused ? 1 : 0); }
int32_t Video::videoOnOff(bool on) { check(); return owner_.api_->video(handle_,on?1:0); }
int32_t Video::soundOnOff(bool on) { check(); return owner_.api_->soundOn(handle_,on?1:0); }
void Video::setVolume(uint32_t track, int32_t volume) { check(); owner_.api_->volume(handle_,track,volume); }
int32_t Video::copyFrame(uint32_t format) {
    check();
    require(format==BINKSURFACE32A || format==BINKSURFACE565 || format==BINKSURFACE5551, "Bink native surface format");
    uint64_t const pitch = uint64_t(handle_->Width) * (format==BINKSURFACE32A ? 4 : 2);
    require(pitch && handle_->Height && pitch <= INT32_MAX, "Bink pixel dimensions");
    uint64_t const bytes = pitch * handle_->Height; // bounded pitch makes multiplication safe
    require(bytes <= owner_.budget_, "Bink pixel budget");
    uint32_t const old = static_cast<uint32_t>(pixels_.size());
    require(bytes <= owner_.budget_ - (owner_.used_ - old), "Bink aggregate pixel budget");
    require(!old || (handle_->Width == copiedWidth_ && handle_->Height == copiedHeight_),
            "Bink frame dimensions changed");
    if (bytes != old) {
        // No transfer is active when the command owner changes format. Release
        // the previous buffer first so even transient allocations obey budget.
        std::vector<unsigned char>().swap(pixels_);
        owner_.used_ -= old;
        std::vector<unsigned char> replacement(static_cast<size_t>(bytes));
        pixels_.swap(replacement);
        copiedWidth_ = handle_->Width; copiedHeight_ = handle_->Height;
        owner_.used_ += static_cast<uint32_t>(bytes);
    }
    return owner_.api_->copy(handle_, &pixels_[0], static_cast<S32>(pitch),handle_->Height,0,0,
                             format | BINKCOPYALL);
}
const std::vector<unsigned char> &Video::pixels() const { check(); return pixels_; }
void Video::close() {
    check();
    owner_.api_->close(handle_); // genuine close, before the driver's AIL shutdown
    handle_ = 0;
    --owner_.live_;
    owner_.used_ -= static_cast<uint32_t>(pixels_.size());
    std::vector<unsigned char>().swap(pixels_);
}
}
