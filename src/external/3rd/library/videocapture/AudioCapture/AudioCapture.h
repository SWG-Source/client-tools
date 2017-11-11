/*
*   AudioCapture declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef AUDIOCAPTURE_AUDIOCAPTURE_H
#define AUDIOCAPTURE_AUDIOCAPTURE_H

// AudioCapture is built using default 8 byte alignment
#pragma pack(push)
#pragma pack(8)
#include "CaptureCommon/IBuffer.h"

namespace AudioCapture
{

enum Prop
{
    cPropName,
    cPropCallback,
    cPropSamplesPerSec,
    cPropBitsPerSample,
    cPropChannels
};

inline const char* Prop2String(Prop prop)
{
    switch(prop)
    {
        case cPropName: return "AudioCapture Filter";
        case cPropCallback: return "Callback";
        case cPropSamplesPerSec: return "SamplesPerSec";
        case cPropBitsPerSample: return "BitsPerSample";
        case cPropChannels: return "Channels";
        default: break;
    }
    return "Invalid";
}

class ICallback
{
public:
    virtual ~ICallback(){}
    virtual bool GetBuffer(CaptureCommon::IBuffer::Ptr& pBuffer) = 0;
    virtual void PutBuffer(CaptureCommon::IBuffer::Ptr& pBuffer) = 0;
};

class IManager
{
public:
    virtual ~IManager(){}
    virtual bool GetConfig(int& samplesPerSec, int& bitsPerSample, int& channels) = 0;
    virtual bool Start(ICallback* pCallback) = 0;
    virtual void Stop() = 0;
};

} // AudioCapture

#pragma pack(pop)

#endif // AUDIOCAPTURE_AUDIOCAPTURE_H
