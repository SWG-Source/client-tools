/*
*   EncoderThread declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef VIDEOCAPTURE_ENCODERTHREAD_H
#define VIDEOCAPTURE_ENCODERTHREAD_H

#include "EncoderConstants.h"
#include "Smart/SmartPtr.h"

namespace VideoCapture
{

class EncoderStatus;
class DestFileStats;

class IEncoderThreadCallback
{
public:
    typedef Smart::SmartPtrT<IEncoderThreadCallback> Ptr;
    virtual ~IEncoderThreadCallback(){};
    // OnEncoderThreadStateChange:
    // Returns previous and current encoder thread states, and any error that occurred during the state transition.
    virtual void OnEncoderThreadStateChange(EncoderThreadState prevState, EncoderThreadState currState) = 0;
    // OnEncoderStateChange:
    // Returns previous, target and current encoder states, and any error that occurred during the state transition.
    virtual void OnEncoderStateChange(EncoderState prevState, EncoderState targetState, EncoderState currState, EncoderErr err) = 0;
    // OnEncoderStatus:
    // Returns EncoderStatus for each call to EncoderThread::GiveTime().
    // All EncoderStatus fields are only updated periodically according to EncoderProps::DstMsecPerStatus()
    virtual void OnEncoderStatus(const EncoderStatus& status) = 0;
    // OnDestFileDone:
    // Returns DestFileStats.
    virtual void OnDestFileDone(const DestFileStats& stats) = 0;
};

class Encoder;
class EncoderLimits;
class EncoderProps;
class EncoderThreadImpl;

class EncoderThread
{
public:
    typedef Smart::SmartPtrT<EncoderThread> Ptr;
    // EncoderThread:
    // Encoder is the encoder object to be managed.
    // EncoderThreadHandler is the callback interface, and may be null.
    // nsecPerFrame sets the rate for calling Encoder::ProcessFrame. Must not be zero.
    // msecPerStatus sets the rate for calling IEncoderThreadCallback::OnEncoderStatus. Zero disables callback.
    // EncoderLimits set limits on thread operation.
    EncoderThread(Encoder& encoder, IEncoderThreadCallback* pEncoderThreadCallback, const CaptureCommon::int64& nsecPerFrame, int msecPerStatus, const EncoderLimits& limits);
    ~EncoderThread();
    // Start:
    // Starts thread.
    void Start();
    // WaitForStart:
    // Blocks until thread starts.
    // Returns false if thread does not start in msecTimeout. Set msecTimeout to -1 for infinite timeout.
    bool WaitForStart(int msecTimeout);
    // Start:
    // Stops thread.
    void Stop();
    // WaitForStop:
    // Blocks until thread stops.
    void WaitForStop();
    // State:
    // Returns EncoderThreadState.
    EncoderThreadState State() const;
    // EncoderInit:
    // Non-blocking call to Encoder::Init.
    void EncoderInit(const EncoderProps& props);
    // EncoderReinit:
    // Non-blocking call to Encoder::Reinit.
    void EncoderReinit(const EncoderProps& props);
    // EncoderStart:
    // Non-blocking call to Encoder::Start.
    void EncoderStart(const char* dstFile);
    // EncoderStop:
    // Non-blocking call to Encoder::Stop.
    void EncoderStop();
    // EncoderUninit:
    // Non-blocking call to Encoder::Uninit.
    void EncoderUninit();
    // GiveTime:
    // Blocking call that internal message queue and returns state transitions and encoder status via IEncoderThreadCallback
    void GiveTime();
    // Note:
    // Encoder::Get/Put*Buffer are called directly on the encoder since those calls are meant to block
private:
    EncoderThread(const EncoderThread&);
    EncoderThread& operator =(const EncoderThread&);
    EncoderThreadImpl* m_pImpl;
};

} // VideoCapture

#endif  // VIDEOCAPTURE_ENCODERTHREAD_H
