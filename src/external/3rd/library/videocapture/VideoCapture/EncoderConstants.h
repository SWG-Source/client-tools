/*
*   EncoderConstants declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef VIDEOCAPTURE_ENCODERCONTANTS_H
#define VIDEOCAPTURE_ENCODERCONTANTS_H

namespace VideoCapture
{

enum EncoderState
{
    cEncoderStateNotInitialized,
    cEncoderStateInitialized,
    cEncoderStateReinitialized,
    cEncoderStateStarted,
    cEncoderStateRunning,
    cEncoderStateStopped,
    cEncoderStateUninitialized,
    cEncoderStateCount
};

const char* EncoderState2String(EncoderState state);

enum EncoderThreadState
{
    cEncoderThreadStateNotStarted,
    cEncoderThreadStateStarted,
    cEncoderThreadStateRunning,
    cEncoderThreadStateStopped,
    cEncoderThreadStateDead,
    cEncoderThreadStateCount
};

const char* EncoderThreadState2String(EncoderThreadState state);

enum EncoderErr
{
    cEncoderErrNone,
    cEncoderErrFail = -1,
    cEncoderErrNoBuffers = -2,
    cEncoderErrNoEncodeJobs = -3,
    cEncoderErrNoWriteJobs = -4,
    cEncoderErrWrongState = -5,
    cEncoderErrTimeLimit = -6,
    cEncoderErrMemoryLimit = -7,
    cEncoderErrDiskLimit = -8,
    cEncoderErrBadFile = -9,
    cEncoderErrBadFileOpen = -10,
    cEncoderErrBadFileRead = -11,
    cEncoderErrBadFileWrite = -12,
    cEncoderErrBadFileData = -13,
    cEncoderErrCount = 14
};

const char* EncoderErr2String(EncoderErr err);

enum SourceFmtVid
{
    cSourceFmtVidInvalid,
    cSourceFmtVidRGB24,
    cSourceFmtVidRGB32,
    cSourceFmtVidJPEG,
    cSourceFmtVidCount
};

inline bool ValidateSourceFmtVid(int srcFmtVid)
{
    return ((cSourceFmtVidInvalid < srcFmtVid) && (srcFmtVid < cSourceFmtVidCount));
}

enum SourceFmtAud
{
    cSourceFmtAudInvalid,
    cSourceFmtAudPCM_8BitMono,
    cSourceFmtAudPCM_8BitStereo,
    cSourceFmtAudPCM_16BitMono,
    cSourceFmtAudPCM_16BitStereo,
    cSourceFmtAudCount
};

inline bool ValidateSourceFmtAud(int srcFmtAud)
{
    return ((cSourceFmtAudInvalid < srcFmtAud) && (srcFmtAud < cSourceFmtAudCount));
}

enum SourceScan
{
    cSourceScanTopBottom,
    cSourceScanBottomTop,
    cSourceScanCount
};

enum DestFmt
{
    cDestFmtInvalid,
    cDestFmtAVI,
    cDestFmtCount
};

inline bool ValidateDestFmt(int dstFmt)
{
    return ((cDestFmtInvalid < dstFmt) && (dstFmt < cDestFmtCount));
}

const char* DestFmt2Ext(DestFmt dstFmt);
DestFmt DestFile2DestFmt(const char* dstFile);
DestFmt DestExt2DestFmt(const char* dstExt);

// NOTE:
// VideoCapture code assumes EncRes is ordered by width, standard (4:3) followed by widescreen
enum EncRes
{
    cEncResInvalid,
    cEncResLow,
    cEncResLowWide,
    cEncResMed,
    cEncResMedWide,
    cEncResHi,
    cEncResHiWide,
    cEncRes160x120,
    cEncRes192x108,
    cEncRes320x240,
    cEncRes384x216,
    cEncRes640x480,
    cEncRes704x396,
    cEncRes960x720,
    cEncRes1024x768,
    cEncRes1024x576,
    cEncRes1280x1024,
    cEncRes1280x720,
    cEncRes1440x1080,
    cEncRes1600x1200,
    cEncRes1920x1080,
    cEncRes1920x1200,
    cEncResCount
};

inline bool ValidateEncRes(int encRes)
{
    return ((cEncResInvalid < encRes) && (encRes < cEncResCount));
}

enum SampleRateAud
{
    cSampleRateAudInvalid,
    cSampleRateAud8000,
    cSampleRateAud11025,
    cSampleRateAud16000,
    cSampleRateAud22050,
    cSampleRateAud32000,
    cSampleRateAud44100,
    cSampleRateAud48000,
    cSampleRateAudCount
};

inline bool ValidateSampleRateAud(int sampleRate)
{
    return ((cSampleRateAudInvalid < sampleRate) && (sampleRate < cSampleRateAudCount));
}

int SourceFmtVid2BytesPerPix(SourceFmtVid srcFmt);
int SourceFmtAud2Channels(SourceFmtAud srcFmt);
int SourceFmtAud2BitsPerSample(SourceFmtAud srcFmt);
SourceFmtAud SourceConfigAud2SourceFmtAud(int bitsPerSample, int channels);
int SampleRateAud2SamplesPerSec(SampleRateAud sampleRate);
SampleRateAud SamplesPerSec2SampleRateAud(int samplesPerSec);

} // VideoCapture

#endif // VIDEOCAPTURE_ENCODERCONTANTS_H
