// ======================================================================
//
// AnimationStateNameIdManager.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/UniqueNameList.h"

#include "boost/smart_ptr.hpp"

// ======================================================================

bool                  AnimationStateNameIdManager::ms_installed;

UniqueNameList        AnimationStateNameIdManager::ms_stateNameList;
AnimationStateNameId  AnimationStateNameIdManager::ms_rootId;

// ======================================================================
// class AnimationStateNameIdManager: public static member functions
// ======================================================================

void AnimationStateNameIdManager::install()
{
	DEBUG_FATAL(ms_installed, ("AnimationStateNameIdManager already installed"));

	const int rootId = ms_stateNameList.submitName(boost::shared_ptr<CrcLowerString>(new ConstCharCrcLowerString("root")));
	ms_rootId        = AnimationStateNameId(rootId);

	ms_installed = true;
	ExitChain::add(remove, "AnimationStateNameIdManager");
}

// ----------------------------------------------------------------------

AnimationStateNameId AnimationStateNameIdManager::createId(const CrcLowerString &stateName)
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateNameIdManager not installed"));

	return AnimationStateNameId(ms_stateNameList.submitName(boost::shared_ptr<CrcLowerString>(new CrcLowerString(stateName))));
}

// ----------------------------------------------------------------------

const AnimationStateNameId &AnimationStateNameIdManager::getRootId()
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateNameIdManager not installed"));

	return ms_rootId;
}

// ----------------------------------------------------------------------

const CrcLowerString &AnimationStateNameIdManager::getNameString(const AnimationStateNameId &id)
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateNameIdManager not installed"));

	const int idValue = id.getId();

	if (idValue >= 0)
		return ms_stateNameList.getName(idValue);
	else
		return CrcLowerString::empty;
}

// ======================================================================
// class AnimationStateNameIdManager: private static member functions
// ======================================================================

void AnimationStateNameIdManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("AnimationStateNameIdManager not installed"));

	ms_stateNameList.reset();

	ms_installed = false;
}

// ======================================================================
