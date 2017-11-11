// ======================================================================
//
// SkeletalAnimationKey.h
// Copyright 2000, 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAnimationKey_H
#define INCLUDED_SkeletalAnimationKey_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationTrackId.h"

#include <map>

class SkeletalAnimationTemplate;

// ======================================================================

struct SkeletalAnimationKey
{

public:

	typedef std::map<char, SkeletalAnimationKey*>  Map;

public:

	char                                     key;
	bool                                     playForced;
	float                                    blendTime;
	float                                    timeScale;
	bool                                     isLooping;
	bool                                     isInterruptible;
	float                                    weight;
	CString                                  filename;
	CString                                  soundFilename;
	const SkeletalAnimationTemplate*         skeletalAnimationTemplate;
	const AnimationTrackId                   trackId;

private:

	// disabled
	SkeletalAnimationKey(void);
	SkeletalAnimationKey(const SkeletalAnimationKey&);
	SkeletalAnimationKey &operator =(const SkeletalAnimationKey&);

public:

	SkeletalAnimationKey(char newKey, const CString& newName, float newTimeScale, bool newPlayLooped, bool newPlayForced, float newBlendTime, const AnimationTrackId &newTrackId);
	~SkeletalAnimationKey(void);

};

// ======================================================================

#endif
