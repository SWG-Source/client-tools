// ======================================================================
//
// SkeletalAnimationTemplateFactory.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SkeletalAnimationTemplateFactory_H
#define INCLUDED_SkeletalAnimationTemplateFactory_H

// ======================================================================

class SkeletalAnimationTemplate;

// ======================================================================

class SkeletalAnimationTemplateFactory
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class AnimationTemplateData
	{
	public:

		virtual ~AnimationTemplateData();

		virtual const std::string         &getFriendlyClassName() const = 0;
		virtual SkeletalAnimationTemplate *createSkeletalAnimationTemplate() const = 0;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void                       install();
	static void                       registerAnimationTemplateData(AnimationTemplateData *animationTemplateData, char hotkey);

	static int                        getTemplateCount();
	static int                        lookupIndexFromHotkey(char hotkey);

	static const std::string         &getFriendlyClassName(int index);
	static SkeletalAnimationTemplate *createSkeletalAnimationTemplate(int index);

private:

	typedef stdvector<AnimationTemplateData*>::fwd  AnimationTemplateDataVector;
	typedef stdmap<char, int>::fwd                  HotkeyIndexMap;

private:

	static void                       remove();

	static AnimationTemplateData     &getAnimationTemplateData(int index);

private:

	static bool                         ms_installed;
	static AnimationTemplateDataVector *ms_animationTemplateDataVector;
	static HotkeyIndexMap              *ms_hotkeyIndexMap;

};

// ======================================================================

#endif
