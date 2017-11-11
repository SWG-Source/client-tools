// ======================================================================
//
// SkeletalAnimationTemplateFactory.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/SkeletalAnimationTemplateFactory.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

bool                                                           SkeletalAnimationTemplateFactory::ms_installed;
SkeletalAnimationTemplateFactory::AnimationTemplateDataVector *SkeletalAnimationTemplateFactory::ms_animationTemplateDataVector;
SkeletalAnimationTemplateFactory::HotkeyIndexMap              *SkeletalAnimationTemplateFactory::ms_hotkeyIndexMap;

// ======================================================================
// class SkeletalAnimationTemplateFactory::AnimationTemplateData
// ======================================================================

SkeletalAnimationTemplateFactory::AnimationTemplateData::~AnimationTemplateData()
{
}

// ======================================================================
// class SkeletalAnimationTemplateFactory: public static member functions
// ======================================================================

void SkeletalAnimationTemplateFactory::install()
{
	DEBUG_FATAL(ms_installed, ("SkeletalAnimationTemplateFactory already installed"));

	ms_animationTemplateDataVector = new AnimationTemplateDataVector;
	ms_hotkeyIndexMap              = new HotkeyIndexMap;

	ms_installed = true;
	ExitChain::add(remove, "SkeletalAnimationTemplateFactory");
}

// ----------------------------------------------------------------------

void SkeletalAnimationTemplateFactory::registerAnimationTemplateData(AnimationTemplateData *animationTemplateData, char hotkey)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateFactory not installed"));
	NOT_NULL(animationTemplateData);

	ms_animationTemplateDataVector->push_back(animationTemplateData);

	if (hotkey)
	{
		HotkeyIndexMap::iterator const lowerBoundIt = ms_hotkeyIndexMap->lower_bound(hotkey);
		if ((lowerBoundIt != ms_hotkeyIndexMap->end()) && !ms_hotkeyIndexMap->key_comp()(hotkey, lowerBoundIt->first))
		{
			DEBUG_FATAL(true, ("hotkey ascii index [%d] char [%c] already assigned.", static_cast<int>(hotkey), hotkey));
			return; //lint !e527 // unreachable // reachable in release.
		}

		IGNORE_RETURN(ms_hotkeyIndexMap->insert(lowerBoundIt, HotkeyIndexMap::value_type(hotkey, static_cast<int>(ms_animationTemplateDataVector->size()) - 1)));
	}
}

// ----------------------------------------------------------------------

int SkeletalAnimationTemplateFactory::getTemplateCount()
{
	return static_cast<int>(ms_animationTemplateDataVector->size());
}

// ----------------------------------------------------------------------
/**
 * @return return value is < 0 if hotkey is not mapped; otherwise, returns
 *         the index of the animation that maps to that hotkey.
 */

int SkeletalAnimationTemplateFactory::lookupIndexFromHotkey(char hotkey)
{
	HotkeyIndexMap::iterator const findIt = ms_hotkeyIndexMap->find(hotkey);
	if (findIt == ms_hotkeyIndexMap->end())
	{
		//-- No mapping.
		return -1;
	}
	else
	{
		//-- Return the index.
		return findIt->second;
	}
}

// ----------------------------------------------------------------------

const std::string &SkeletalAnimationTemplateFactory::getFriendlyClassName(int index)
{
	return getAnimationTemplateData(index).getFriendlyClassName();
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplate *SkeletalAnimationTemplateFactory::createSkeletalAnimationTemplate(int index)
{
	return getAnimationTemplateData(index).createSkeletalAnimationTemplate();
}

// ======================================================================

void SkeletalAnimationTemplateFactory::remove()
{
	DEBUG_FATAL(!ms_installed, ("SkeletalAnimationTemplateFactory not installed"));
	ms_installed = false;

	delete ms_hotkeyIndexMap;
	ms_hotkeyIndexMap = 0;

	std::for_each(ms_animationTemplateDataVector->begin(), ms_animationTemplateDataVector->end(), PointerDeleter());
	delete ms_animationTemplateDataVector;

	ms_animationTemplateDataVector = 0;
}

// ----------------------------------------------------------------------

SkeletalAnimationTemplateFactory::AnimationTemplateData &SkeletalAnimationTemplateFactory::getAnimationTemplateData(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTemplateCount());
	
	AnimationTemplateData *const atData = (*ms_animationTemplateDataVector)[static_cast<AnimationTemplateDataVector::size_type>(index)];
	NOT_NULL(atData);

	return *atData;
}

// ======================================================================
