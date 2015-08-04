// ======================================================================
//
// SwgVideoCapture.h
// copyright (c) 2009 Sony Online Entertainment
//
// ======================================================================

#ifndef VIDEOCAPTURE_SWGVIDEOCAPTURE_H
#define VIDEOCAPTURE_SWGVIDEOCAPTURE_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"

#if PRODUCTION == 0

namespace AudioCapture
{

class IManager;

} // AudioCapture

namespace VideoCapture
{

void install(); // Installs SoeUtilMemoryAdapter

namespace SingleUse
{

class ICallback
{
public:
	virtual ~ICallback(){}
	virtual void OnStart() = 0;
	virtual void OnStop() = 0;
};

void config(int resolution, int seconds, int quality, const char* filename, AudioCapture::IManager* pAudioCaptureManager);
void start(VideoCapture::SingleUse::ICallback* pVideoCaptureCallback, AudioCapture::IManager* pAudioCaptureManager);
void stop();
void run();

} // SingleUse

} // VideoCapture

#endif // PRODUCTION

#endif // VIDEOCAPTURE_SWGVIDEOCAPTURE_H
