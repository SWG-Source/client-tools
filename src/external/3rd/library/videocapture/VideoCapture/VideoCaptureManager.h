/*
*   VideoCaptureManager declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef VIDEOCAPTURE_VIDEOCAPTUREMANAGER_H
#define VIDEOCAPTURE_VIDEOCAPTUREMANAGER_H

#include "Encoder.h"
#include "EncoderThread.h"
#include "EncoderUtilities.h"

namespace VideoCapture
{

// VideoCaptureManager:
// Simple wrapper class.
// Designed for use in environment where app manages the state.

class VideoCaptureManager
{
public:
    typedef Smart::SmartPtrT<VideoCaptureManager> Ptr;
    VideoCaptureManager();
    ~VideoCaptureManager();
    EncoderState GetEncoderState();
    EncoderThreadState GetEncoderThreadState();
    // Init:
    // Creates and inits encoder.
    // All internal threads are started here.
    // EncoderProps and EncoderLimits must be valid.
    // EncoderThreadHandler may be null.
    // If EncoderThreadHandler is not null it must remain valid until GetEncoderThreadState returns cEncoderThreadStateDead.
    // Init is non-blocking.
    // Result is returned via EncoderThreadHandler.
    bool Init(const EncoderProps& props, const EncoderLimits& limits, IEncoderThreadCallback* pEncoderThreadCallback);
    // Reinit:
    // Changes encoder parameters.
    // EncoderProps must be valid.
    // Reinit is non-blocking.
    // Result is returned via EncoderThreadHandler.
    bool Reinit(const EncoderProps& props);
    // Start:
    // Starts encoder processing.
    // Start is non-blocking.
    // Result is returned via EncoderThreadHandler.
    void Start(const char* dstFile);
    // Stop:
    // Stops encoder processing.
    // Stop is non-blocking.
    // Result is returned via EncoderThreadHandler.
    void Stop();
    // Uninit:
    // Stops internal threads and destroys encoder.
    // Uninit is non-blocking.
    // Result is returned via EncoderThreadHandler.
    // When GetEncoderThreadState returns cEncoderThreadStateDead then the VideoCaptureManager should be destroyed.
    void Uninit();
    // Get/PutBuffer:
    // Gets allocated buffer.
    // Should be filled in with frame data and returned with call to PutBuffer.
    // Get/PutBuffer are blocking.
    bool GetVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool PutVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool GetAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool PutAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    // GiveTime:
    // Should be called every game frame.
    // Processes internal message queue, returns state transitions and encoder status via IEncoderThreadCallback
    void GiveTime();
private:
    VideoCaptureManager(const VideoCaptureManager&);
    VideoCaptureManager& operator =(const VideoCaptureManager&);
    Encoder::Ptr m_pEncoder;
    EncoderThread::Ptr m_pEncoderThread;
};

// VideoCaptureManager_SingleUse:
// Wrapper class that manages encoder state.
// Designed for single-use (encoder is created on start and destroyed on stop), no-callback (state is managed by polling) environment.
// When GiveTime returns cStateStarted then the calling app should get/put frame buffers when convenient.
// When GiveTime returns cStateDead then the calling app should destroy the VideoCaptureManager object.

class VideoCaptureManagerImpl_SingleUse;

class VideoCaptureManager_SingleUse
{
public:
    typedef Smart::SmartPtrT<VideoCaptureManager_SingleUse> Ptr;
    enum State
    {
        cStateNone,
        cStateStarting,
        cStateStarted,
        cStateStopping,
        cStateStopped,
        cStateDead
    };
    VideoCaptureManager_SingleUse();
    ~VideoCaptureManager_SingleUse();
    // Start:
    // Creates and starts encoder during calls to GiveTime().
    bool Start(const EncoderProps& props, const EncoderLimits& limits, const char* dstFile);
    // Stop:
    // Stops and destroys encoder during calls to GiveTime().
    void Stop();
    // Get/PutBuffer:
    // Gets allocated frame buffer.
    // Should be filled in with frame data and returned with call to PutBuffer.
    bool GetVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool PutVideoBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool GetAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    bool PutAudioBuffer(CaptureCommon::IBuffer::Ptr& pBuffer);
    // GiveTime:
    // Should be called every game frame.
    // Moves the encoder through its states.
    // When GiveTime returns cStateStarted then the calling app should get/put frame buffers when convenient.
    // When GiveTime returns cStateDead then the calling app should destroy the VideoCaptureManager object.
    State GiveTime();
private:
    VideoCaptureManager_SingleUse(const VideoCaptureManager_SingleUse&);
    VideoCaptureManager_SingleUse& operator =(const VideoCaptureManager_SingleUse&);
    VideoCaptureManagerImpl_SingleUse* m_pImpl;
};

} // VideoCapture

#endif // VIDEOCAPTURE_VIDEOCAPTUREMANAGER_H
