// ======================================================================
// videocapture_stubs.cpp
//
// No-op definitions for the videocapture/SoeUtil symbols that
// clientGraphics(SwgVideoCapture.obj) references. The real
// implementations live in the x86-only prebuilt videocapture libs
// (src\external\3rd\library\videocapture\*\lib\win32), which can never
// link into an x64 image -- video capture has effectively been dead on
// x64 since the port. Release never notices because nothing there pulls
// SwgVideoCapture.obj out of clientGraphics.lib; the Debug client does,
// so the Debug link needs these stubs. Every entry returns the "not
// available / did nothing" value; a capture started against these stubs
// simply never produces buffers and reports cStateDead.
//
// Compiled against the REAL headers so the mangled names match by
// construction. Same pattern as swg-stubs / graphics-stubs.
// ======================================================================

#include "AudioCapture/AudioCapture.h"
#include "VideoCapture/VideoCapture.h"
#include "VideoCapture/VideoCaptureManager.h"
#include "SoeUtil/ThreadLocal.h"

namespace SoeUtil
{
    static void *s_threadLocalVoidSlot; // single-slot stand-in; capture path never runs

    ThreadLocalVoid::ThreadLocalVoid() {}
    ThreadLocalVoid::~ThreadLocalVoid() {}

    void *ThreadLocalVoid::Get() const
    {
        return s_threadLocalVoidSlot;
    }

    void *ThreadLocalVoid::Set(void *rhs)
    {
        void *const old = s_threadLocalVoidSlot;
        s_threadLocalVoidSlot = rhs;
        return old;
    }
}

namespace VideoCapture
{
    SourceFmtAud SourceConfigAud2SourceFmtAud(int, int)
    {
        return static_cast<SourceFmtAud>(0);
    }

    SampleRateAud SamplesPerSec2SampleRateAud(int)
    {
        return static_cast<SampleRateAud>(0);
    }

    EncoderRes::EncoderRes() {}
    EncoderRes::~EncoderRes() {}

    const EncoderRes &EncoderRes::GetEncoderRes(EncRes)
    {
        static EncoderRes const s_stub;
        return s_stub;
    }

    const EncoderRes &FindBestEncRes(int, int, EncRes)
    {
        static EncoderRes const s_stub;
        return s_stub;
    }

    EncoderProps::EncoderProps() {}
    EncoderProps::~EncoderProps() {}

    EncoderLimits::EncoderLimits() {}
    EncoderLimits::~EncoderLimits() {}

    bool VideoCaptureManager::GetAudioBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    bool VideoCaptureManager::PutAudioBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    VideoCaptureManager_SingleUse::VideoCaptureManager_SingleUse()
    : m_pImpl(0)
    {
    }

    VideoCaptureManager_SingleUse::~VideoCaptureManager_SingleUse() {}

    bool VideoCaptureManager_SingleUse::Start(const EncoderProps &, const EncoderLimits &, const char *)
    {
        return false;
    }

    void VideoCaptureManager_SingleUse::Stop() {}

    bool VideoCaptureManager_SingleUse::GetVideoBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    bool VideoCaptureManager_SingleUse::PutVideoBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    bool VideoCaptureManager_SingleUse::GetAudioBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    bool VideoCaptureManager_SingleUse::PutAudioBuffer(CaptureCommon::IBuffer::Ptr &)
    {
        return false;
    }

    VideoCaptureManager_SingleUse::State VideoCaptureManager_SingleUse::GiveTime()
    {
        return cStateDead;
    }
}
