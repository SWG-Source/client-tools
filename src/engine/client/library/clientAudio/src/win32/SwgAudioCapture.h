// ======================================================================
//
// SwgAudioCapture.h
// copyright (c) 2009 Sony Online Entertainment
//
// ======================================================================

#ifndef AUDIOCAPTURE_SWGAUDIOCAPTURE_H
#define AUDIOCAPTURE_SWGAUDIOCAPTURE_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"

#if PRODUCTION == 0

#include "AudioCapture/AudioCapture.h"

namespace AudioCapture
{

class SwgAudioCaptureManager : public IManager
{
public:
	static SwgAudioCaptureManager& GetInstance();
	// IManager
	virtual bool GetConfig(int& samplesPerSec, int& bitsPerSample, int& channels);
	virtual bool Start(ICallback* pCallback);
	virtual void Stop();
private:
	SwgAudioCaptureManager();
	virtual ~SwgAudioCaptureManager();
	SwgAudioCaptureManager(const SwgAudioCaptureManager&);
	SwgAudioCaptureManager& operator=(const SwgAudioCaptureManager&);
};

} // AudioCapture

#endif // PRODUCTION

#endif // AUDIOCAPTURE_SWGAUDIOCAPTURE_H
