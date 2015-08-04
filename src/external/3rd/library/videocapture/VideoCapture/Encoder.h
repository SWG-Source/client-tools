/*
*   Encoder declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef VIDEOCAPTURE_ENCODER_H
#define VIDEOCAPTURE_ENCODER_H

#include "EncoderConstants.h"
#include "CaptureCommon/IBuffer.h"

namespace VideoCapture
{

class DestFileStats;
class EncoderImpl;
class EncoderProps;
class EncoderStatus;

class Encoder
{
public:
    typedef Smart::SmartPtrT<Encoder> Ptr;
    Encoder();
    ~Encoder();
    // Init:
    // Validates EncoderProps, allocates resources, starts threads, puts encoder into cEncoderStateInitialized.
    EncoderErr Init(const EncoderProps& props);
    // Reinit:
    // Validates EncoderProps, may reallocate resources, puts encoder into cEncoderStateReinitialized.
    EncoderErr Reinit(const EncoderProps& props);
    // Start:
    // Opens output file, puts encoder into cEncoderStateStarted.
    EncoderErr Start(const char* pDstFile);
    // Get/PutBuffer:
    // Gets allocated video/audio buffer.
    // Should be filled in with video/audio data and returned with call to PutBuffer.
    // PutVideoBuffer saves the current video frame.
    // PutAudioBuffer saves all audio data (to some internal maximum).
    // ProcessFrame sends the current video frame and all buffered audio to processing threads.
    // The first call to PutVideoBuffer puts encoder into cEncoderStateRunning.
    EncoderErr GetVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    EncoderErr PutVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    EncoderErr GetAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    EncoderErr PutAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    // ProcessFrame:
    // Sends buffered video/audio data to processing threads.
    // Call frequency determined by frame rate (fps).
    // nsecPts should be set appropriately, but is currently only used for debug.
    // nsecPts does not affect the frame rate (fps).
    EncoderErr ProcessFrame(const CaptureCommon::int64& nsecPts);
    // GetStatus:
    // May be called periodically to get encoder status.
    void GetStatus(EncoderStatus& status) const;
    // Stop:
    // Closes output file, puts encoder into cEncoderStateStopped.
    void Stop();
    // GetDestFileStats:
    // Returns DestFileStats.
    // Returned data is only valid after call to stop.
    EncoderErr GetDestFileStats(DestFileStats& stats);
    // Uninit:
    // Clears internal data, deallocates resources, stops threads, puts encoder into cEncoderStateUninitialized.
    void Uninit();
    // State:
    // Returns current encoder state.
    EncoderState State() const;
private:
    Encoder(const Encoder&);
    Encoder& operator =(const Encoder&);
    EncoderImpl* m_pImpl;
};

} // VideoCapture

#endif // VIDEOCAPTURE_ENCODER_H
