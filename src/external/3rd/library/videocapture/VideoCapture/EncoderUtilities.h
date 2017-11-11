/*
*   EncoderUtilities declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef VIDEOCAPTURE_ENCODERUTILITIES_H
#define VIDEOCAPTURE_ENCODERUTILITIES_H

#include "EncoderConstants.h"
#include "Types.h"
#include "SoeUtil/String.h"
#include "SoeUtil/Time.h"

namespace VideoCapture
{

const int c_minRate = 5;
const int c_maxRate = 30;

inline bool ValidateRate(int r)
{
    return ((c_minRate <= r) && (r <= c_maxRate));
}

const int c_minQuality = 0;
const int c_maxQuality = 10000;

inline bool ValidateQuality(int q)
{
    return ((c_minQuality <= q) && (q <= c_maxQuality));
}

inline int LinearTransform(int x, int x1, int x2, int y1, int y2)
{
    return (y1 + ((x - x1) * (y2 - y1) / (x2 - x1))); // returns y
}

inline int Quality2Native(int min, int max, int q)
{
    if(!ValidateQuality(q))
    {
        return max;
    }
    // (max - min) may be neg
    // Ex: min=2 max=20 q=0 (min), (2 + (0 * 18 / 10000)=2 (min)
    // Ex: min=2 max=20 q=10000 (max), (2 + (10000 * 18 / 10000)=20 (max)
    // Ex: min=20 max=2 q=0 (min), (20 + (0 * -18 / 10000)=20 (min)
    // Ex: min=20 max=2 q=10000 (max), (20 + (10000 * -18 / 10000)=2 (max)
    return LinearTransform(q, c_minQuality, c_maxQuality, min, max);
}

inline int Native2Quality(int min, int max, int n)
{
    // (max - min) may be neg
    // Ex: min=2 max=20 q=2 (min), (0 + ((2 - 2) * 10000 / 18))=0 (min)
    // Ex: min=2 max=20 q=20 (max), (0 + ((20 - 2) * 10000 / 18))=10000 (max)
    // Ex: min=20 max=2 q=20 (min), (0 + ((20 - 20) * 10000 / -18))=0 (min)
    // Ex: min=20 max=2 q=2 (max), (0 + ((2 - 20) * 10000 / -18))=10000 (max)
    return LinearTransform(n, min, max, c_minQuality, c_maxQuality);
}

class EncoderComp
{
public:
    virtual ~EncoderComp();
    unsigned long FourCC() const
    {
        return m_fcc;
    }
    int MapQuality(int q) // (c_minQuality - c_maxQuality) to native
    {
        return Quality2Native(m_minQuality, m_maxQuality, q);
    }
    void Clear();
    void Print(SoeUtil::String& s) const;
    void Print() const;
protected:
    EncoderComp();
    EncoderComp(unsigned long fcc, int minQuality, int maxQuality);
private:
    unsigned long m_fcc;
    int m_minQuality;
    int m_maxQuality;
    int m_qualityRange;
};

class EncoderRes
{
public:
    static const EncoderRes& GetEncoderRes(EncRes encRes);
    EncoderRes();
    virtual ~EncoderRes();
    EncRes Res() const
    {
        return m_encRes;
    }
    int Width() const
    {
        return m_width;
    }
    int Height() const
    {
        return m_height;
    }
    bool WideScreen() const
    {
        return m_wideScreen;
    }
    bool Compressed() const
    {
        return m_compressed;
    }
    void Clear();
    void Print(SoeUtil::String& s) const;
    void Print() const;
private:
    EncoderRes(EncRes encRes, int width, int height, bool wideScreen, bool compressed);
    EncRes m_encRes;
    int m_width;
    int m_height;
    bool m_wideScreen;
    bool m_compressed;
};

const EncoderRes& FindBestEncRes(int srcWidth, int srcHeight, EncRes targetRes);

inline int64 CalcTimePerFrame(int rate, int scale) // 100nsec
{
    return ((scale * 10000000LL) / rate); // 100nsec
}

inline int64 CalcNsecPerFrame(int rate, int scale) // nsec
{
    return ((scale * 1000000000LL) / rate);
}

inline int CalcBlockAlignAud(int channels, int bitsPerSample)
{
    return (channels * bitsPerSample / 8);
}

inline int CalcAvgBytesPerSecAud(int channels, int samplesPerSec, int bitsPerSample)
{
    const int blockAlign = CalcBlockAlignAud(channels, bitsPerSample);
    return (samplesPerSec * blockAlign);
}

inline int CalcAudSize(int channels, int samplesPerSec, int bitsPerSample)
{
    return CalcAvgBytesPerSecAud(channels, samplesPerSec, bitsPerSample);
}

class EncoderProps
{
public:
    EncoderProps();
    ~EncoderProps();
    // SrcFmtVid:
    // Video source format
    // Read/Write
    // Required
    void SrcFmtVid(SourceFmtVid srcFmtVid)
    {
        m_srcFmtVid = srcFmtVid;
    }
    SourceFmtVid SrcFmtVid() const
    {
        return m_srcFmtVid;
    }
    // SrcScan:
    // Video source scan order
    // Read/Write
    // Required
    void SrcScan(SourceScan srcScan)
    {
        m_srcScan = srcScan;
    }
    SourceScan SrcScan() const
    {
        return m_srcScan;
    }
    // Res:
    // Desired output frame resolution
    // Read/Write
    // Required
    void Res(EncRes encRes)
    {
        m_encRes = encRes;
    }
    EncRes Res() const
    {
        return m_encRes;
    }
    // SrcWidth:
    // Width of source frame
    // Read/Write
    // Required
    void SrcWidth(int srcWidth)
    {
        m_srcWidth = srcWidth;
    }
    int SrcWidth() const
    {
        return m_srcWidth;
    }
    // SrcHeight:
    // Height of source frame
    // Read/Write
    // Required
    void SrcHeight(int srcHeight)
    {
        m_srcHeight = srcHeight;
    }
    int SrcHeight() const
    {
        return m_srcHeight;
    }
    // SrcBytesPerPix:
    // Video source bytes per pix, determined by SrcFmtVid
    // Read only
    int SrcBytesPerPix() const
    {
        return m_srcBytesPerPix;
    }
    // SrcBytesVid:
    // Video source bytes per frame, determined by SrcBytesPerPix, SrcWidth and SrcHeight
    // Read only
    int SrcBytesVid() const
    {
        return m_srcBytesVid;
    }
    // SrcFmtAud:
    // Audio source format
    // Read/Write
    // Required to enable audio
    void SrcFmtAud(SourceFmtAud srcFmtAud)
    {
        m_srcFmtAud = srcFmtAud;
    }
    SourceFmtAud SrcFmtAud() const
    {
        return m_srcFmtAud;
    }
    // SrcSampleRateAud:
    // Audio samples per second
    // Read/Write
    // Required if audio enabled
    void SrcSampleRateAud(SampleRateAud srcSampleRateAud)
    {
        m_srcSampleRateAud = srcSampleRateAud;
    }
    SampleRateAud SrcSampleRateAud() const
    {
        return m_srcSampleRateAud;
    }
    // SrcChannels:
    // Audio source channel count (1 mono, 2 sterio)
    // Read only
    int SrcChannels() const
    {
        return SourceFmtAud2Channels(m_srcFmtAud);
    }
    // SrcBitsPerSample:
    // Audio source bits per sample
    // Read only
    int SrcBitsPerSample() const
    {
        return SourceFmtAud2BitsPerSample(m_srcFmtAud);
    }
    // SrcSamplesPerSec:
    // Audio source samples per second
    // Read only
    int SrcSamplesPerSec() const
    {
        return SampleRateAud2SamplesPerSec(m_srcSampleRateAud);
    }
    // SrcBytesAud:
    // Audio source bytes per sec, determined by SrcFmtAud, SrcChannels, SrcSamplesPerSec and SrcBitsPerSample
    // Read only
    int SrcBytesAud() const
    {
        return m_srcBytesAud;
    }
    // CompWidth:
    // Width of compressor input frame
    // Equal to DstWidth if prescaled by calling app, equal to SrcWidth otherwise
    // Read only
    int CompWidth() const
    {
        return m_compWidth;
    }
    // CompHeight:
    // Height of compressor input frame
    // Equal to DstHeight if prescaled by calling app, equal to SrcHeight otherwise
    // Read only
    int CompHeight() const
    {
        return m_compHeight;
    }
    // CompBytesPerPix:
    // Bytes per pixel of compressor input frame
    // Currently always equal to SrcBytesPerPix
    // Read only
    int CompBytesPerPix() const
    {
        return m_compBytesPerPix;
    }
    // CompBytes:
    // Bytes of compressor input frame, determined by CompBytesPerPix, CompWidth and CompHeight
    // Read only
    int CompBytes() const
    {
        return m_compBytes;
    }
    // DstFmt
    // Output file format
    // Read/Write
    // Required
    void DstFmt(DestFmt dstFmt)
    {
        m_dstFmt = dstFmt;
    }
    DestFmt DstFmt() const
    {
        return m_dstFmt;
    }
    // DstMaxFileSize
    // Max allowable output file size
    // Currently only determines the preallocated index size of the avi header
    // Read/Write
    // Required
    // Typical value: 100 (GBytes)
    void DstMaxFileGBytes(int gbytes) // GBytes
    {
        m_dstMaxFileGBytes = gbytes;
    }
    const int DstMaxFileGBytes() const
    {
        return m_dstMaxFileGBytes;
    }
    // DstWidth:
    // Width of output frame, determined by Res
    // Read only
    int DstWidth() const
    {
        return m_dstWidth;
    }
    // DstHeight:
    // Height of output frame, determined by Res
    // Read only
    int DstHeight() const
    {
        return m_dstHeight;
    }
    // DstRate:
    // Desired output frames per sec numerator
    // Read/Write
    // Required
    // Typical value: 30
    void DstRate(int dstRate)
    {
        m_dstRate = dstRate;
    }
    int DstRate() const
    {
        return m_dstRate;
    }
    // DstScale:
    // Desired output frames per sec denominator
    // Read/Write
    // Required
    // Typical value: 1
    void DstScale(int dstScale)
    {
        m_dstScale = dstScale;
    }
    int DstScale() const
    {
        return m_dstScale;
    }
    // DstNsecPerFrame:
    // Desired output nsec per frame
    // Read/Write
    // Only required as a replacement for DstRate and DstScale
    // Typical value: 33333333
    void DstNsecPerFrame(const int64& dstNsecPerFrame)
    {
        m_dstNsecPerFrame = dstNsecPerFrame;
    }
    const int64& DstNsecPerFrame() const
    {
        return m_dstNsecPerFrame;
    }
    // DstMsecPerStatus:
    // Desired status callback rate
    // Read/Write
    // Typical value: 100
    void DstMsecPerStatus(int dstMsecPerStatus)
    {
        m_dstMsecPerStatus = dstMsecPerStatus;
    }
    int DstMsecPerStatus() const
    {
        return m_dstMsecPerStatus;
    }
    // DstQuality:
    // Desired output video compression quality
    // Read/Write
    // Required
    // Valid range: c_minQuality to c_maxQuality
    // Set to zero to use system default
    // Typical value: 0
    void DstQuality(int q)
    {
        m_dstQuality = q;
    }
    int DstQuality() const
    {
        return m_dstQuality;
    }
    // PreScale:
    // Indicates whether calling app will prescale frames (true) to desired output size
    // Read/Write
    // Required
    // Must be true for now
    // Current rescale algorithm is simple subsample designed for testing
    // It should not be used in production
    void PreScale(bool state)
    {
        m_preScale = state;
    }
    bool PreScale() const
    {
        return m_preScale;
    }
    // MemoryLimit:
    // Enables or disables system memory limits (Disk and RAM)
    // If enabled, encoder will stop when limits are reached
    // Read/Write
    // Required
    // Must be false for now
    //void MemoryLimit(bool state)
    //{
    //    m_memoryLimit = state;
    //}
    //bool MemoryLimit() const
    //{
    //    return m_memoryLimit;
    //}
    // SaveFirstFrame:
    // Enables or disables saving first frame to file
    // Read/Write
    void SaveFirstFrame(bool state)
    {
        m_saveFirstFrame = state;
    }
    bool SaveFirstFrame() const
    {
        return m_saveFirstFrame;
    }
    // PrivateData:
    // Encoder specific private data
    // Calling app must ignore
    void PrivateData(void* p)
    {
        m_pPrivateData = p;
    }
    void* PrivateData() const
    {
        return m_pPrivateData;
    }
    // HasVideo:
    // Returns true for valid SrcFmtVid, otherwise false
    bool HasVideo() const
    {
        return ValidateSourceFmtVid(m_srcFmtVid);
    }
    // HasAudio:
    // Returns true for valid SrcFmtAud, otherwise false
    bool HasAudio() const
    {
        return ValidateSourceFmtAud(m_srcFmtAud);
    }
    // Clear:
    // Clears data
    void Clear();
    // Validate:
    // Validates data
    bool Validate() const;
    // Print:
    // Prints data in human readable form
    void Print(SoeUtil::String& s) const;
    // Print:
    // Prints data to console and debug output window
    void Print() const;
private:
    SourceFmtVid m_srcFmtVid;
    SourceScan m_srcScan;
    EncRes m_encRes;
    int m_srcWidth;
    int m_srcHeight;
    mutable int m_srcBytesPerPix;
    mutable int m_srcBytesVid;
    SourceFmtAud m_srcFmtAud;
    SampleRateAud m_srcSampleRateAud;
    mutable int m_srcBytesAud;
    mutable int m_compWidth;
    mutable int m_compHeight;
    mutable int m_compBytesPerPix;
    mutable int m_compBytes;
    DestFmt m_dstFmt;
    int m_dstMaxFileGBytes;
    mutable int m_dstWidth;
    mutable int m_dstHeight;
    int m_dstRate;
    int m_dstScale;
    mutable int64 m_dstNsecPerFrame;
    int m_dstMsecPerStatus;
    int m_dstQuality;
    bool m_preScale; // Client will prescale frames, currently must be true
    bool m_memoryLimit; // memoryLimit=false disables memory limit for expert user, currently unused
    bool m_saveFirstFrame;
    void* m_pPrivateData;
};

class EncoderLimits
{
public:
    EncoderLimits();
    ~EncoderLimits();
    void SecDuration(int sec)
    {
        m_secDuration = sec;
    }
    int SecDuration() const
    {
        return m_secDuration;
    }
    void MinPcntMemoryAvailable(int pcnt)
    {
        m_minPcntMemoryAvailable = pcnt;
    }
    int MinPcntMemoryAvailable() const
    {
        return m_minPcntMemoryAvailable;
    }
    void MinDiskMBytesAvailable(int MBytes)
    {
        m_minDiskMBytesAvailable = MBytes;
    }
    int MinDiskMBytesAvailable() const
    {
        return m_minDiskMBytesAvailable;
    }
    void Clear();
    bool Validate() const;
    void Print(SoeUtil::String& s) const;
    void Print() const;
private:
    int m_secDuration;
    int m_minPcntMemoryAvailable;
    int m_minDiskMBytesAvailable;
};

class EncoderStatus
{
public:
    EncoderStatus();
    ~EncoderStatus();
    void State(EncoderState state)
    {
        m_state = state;
    }
    EncoderState State() const
    {
        return m_state;
    }
    void FrameCount(const int64& cnt)
    {
        m_frameCount = cnt;
    }
    const int64& FrameCount() const
    {
        return m_frameCount;
    }
    void NsecElapsed(const int64& nsec)
    {
        m_nsecElapsed = nsec;
    }
    const int64& NsecElapsed() const
    {
        return m_nsecElapsed;
    }
    void PcntMemoryAvailable(int pcnt)
    {
        m_pcntMemoryAvail = pcnt;
    }
    int PcntMemoryAvailable() const
    {
        return m_pcntMemoryAvail;
    }
    void DiskMBytesAvailable(int MBytes)
    {
        m_diskMBytesAvail = MBytes;
    }
    int DiskMBytesAvailable() const
    {
        return m_diskMBytesAvail;
    }
    void Clear();
    void Print(SoeUtil::String& s) const;
    void Print() const;
private:
    EncoderState m_state;
    int64 m_frameCount;
    int64 m_nsecElapsed;
    int m_pcntMemoryAvail;
    int m_diskMBytesAvail;
};

class DestFileStats
{
public:
    DestFileStats();
    ~DestFileStats();
    void Filename(const SoeUtil::String& filename)
    {
        m_filename = filename;
    }
    const SoeUtil::String& Filename() const
    {
        return m_filename;
    }
    void Width(int width)
    {
        m_width = width;
    }
    int Width() const
    {
        return m_width;
    }
    void Height(int height)
    {
        m_height = height;
    }
    int Height() const
    {
        return m_height;
    }
    void Size(const int64& size)
    {
        m_size = size;
    }
    const int64& Size() const
    {
        return m_size;
    }
    void CreationTime(const SoeUtil::Time::SecondsStamp& creationTime)
    {
        m_creationTime = creationTime;
    }
    const SoeUtil::Time::SecondsStamp& CreationTime() const
    {
        return m_creationTime;
    }
    void ModifiedTime(const SoeUtil::Time::SecondsStamp& modifiedTime)
    {
        m_modifiedTime = modifiedTime;
    }
    const SoeUtil::Time::SecondsStamp& ModifiedTime() const
    {
        return m_modifiedTime;
    }
    void NsecPerFrame(const int64& nsecPerFrame)
    {
        m_nsecPerFrame = nsecPerFrame;
    }
    const int64& NsecPerFrame() const
    {
        return m_nsecPerFrame;
    }
    void FrameCount(const int64& frameCount)
    {
        m_frameCount = frameCount;
    }
    const int64& FrameCount() const
    {
        return m_frameCount;
    }
    void AudSampleCount(const int64& audCount)
    {
        m_audCount = audCount;
    }
    const int64& AudSampleCount() const
    {
        return m_audCount;
    }
    void Clear()
    {
        m_filename.Clear();
        m_width = 0;
        m_height = 0;
        m_size = 0;
        m_creationTime = SoeUtil::Time::cSecondsNull;
        m_modifiedTime = SoeUtil::Time::cSecondsNull;
        m_nsecPerFrame = 0;
        m_frameCount = 0;
        m_audCount = 0;
    }
    void Print(SoeUtil::String& s) const;
    void Print() const;
private:
    SoeUtil::String m_filename;
    int m_width;
    int m_height;
    int64 m_size;
    SoeUtil::Time::SecondsStamp m_creationTime; // Seconds since 01/01/1970 UTC
    SoeUtil::Time::SecondsStamp m_modifiedTime; // Seconds since 01/01/1970 UTC
    int64 m_nsecPerFrame;
    int64 m_frameCount;
    int64 m_audCount;
};

bool DestFmt2DestFile(DestFmt fmt, SoeUtil::String& dstFile);

} // VideoCapture

#endif // VIDEOCAPTURE_ENCODERUTILITIES_H
