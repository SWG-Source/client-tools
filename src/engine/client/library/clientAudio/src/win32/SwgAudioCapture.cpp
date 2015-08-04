// ======================================================================
//
// SwgAudioCapture.cpp
// copyright (c) 2009 Sony Online Entertainment
//
// ======================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/Audio.h"
#include "clientAudio/SwgAudioCapture.h"

#if PRODUCTION == 0

namespace AudioCapture
{

SwgAudioCaptureManager& SwgAudioCaptureManager::GetInstance()
{
	static SwgAudioCaptureManager s_swgAudioCaptureManager;
	return s_swgAudioCaptureManager;
}

SwgAudioCaptureManager::SwgAudioCaptureManager()
{
}

SwgAudioCaptureManager::~SwgAudioCaptureManager()
{
}

bool SwgAudioCaptureManager::GetConfig(int& samplesPerSec, int& bitsPerSample, int& channels)
{
	return Audio::getAudioCaptureConfig(samplesPerSec, bitsPerSample, channels);
}

bool SwgAudioCaptureManager::Start(ICallback* pCallback)
{
	return Audio::startAudioCapture(pCallback);
}

void SwgAudioCaptureManager::Stop()
{
	Audio::stopAudioCapture();
}

} // AudioCapture

#endif // PRODUCTION
