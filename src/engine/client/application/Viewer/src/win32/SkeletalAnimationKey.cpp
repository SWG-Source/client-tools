//
// SkeletalAnimationKey.cpp
// asommers 11-2-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstViewer.h"
#include "SkeletalAnimationKey.h"

#include "sharedFoundation/CrcLowerString.h"
#include "clientSkeletalAnimation/AnimationTrackId.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"

//-------------------------------------------------------------------

SkeletalAnimationKey::SkeletalAnimationKey(char newKey, const CString& newName, float newTimeScale, bool playLooped, bool newPlayForced, float newBlendTime, const AnimationTrackId &newTrackId)
:	key(newKey),
	playForced(newPlayForced),
	blendTime(newBlendTime),
	timeScale(newTimeScale),
	isLooping(playLooped),
	isInterruptible(false),
	weight(CONST_REAL(1.0)),
	filename(newName),
	skeletalAnimationTemplate(0),
	trackId(newTrackId)
{
	skeletalAnimationTemplate = SkeletalAnimationTemplateList::fetch(CrcLowerString(newName));
	if (!skeletalAnimationTemplate)
		return;
}

// ----------------------------------------------------------------------

SkeletalAnimationKey::~SkeletalAnimationKey(void)
{
	if (skeletalAnimationTemplate)
	{
		skeletalAnimationTemplate->release();
		skeletalAnimationTemplate = 0;
	}
}

//-------------------------------------------------------------------

