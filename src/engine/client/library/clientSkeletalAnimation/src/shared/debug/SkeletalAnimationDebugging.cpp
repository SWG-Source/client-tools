// ======================================================================
//
// SkeletalAnimationDebugging.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace SkeletalAnimationDebuggingNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                  s_installed;
	ConstWatcher<Object> *s_focusObject;

}

using namespace SkeletalAnimationDebuggingNamespace;

// ======================================================================
// namespace SkeletalAnimationDebuggingNamespace
// ======================================================================

void SkeletalAnimationDebuggingNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SkeletalAnimationDebugging not installed."));
	s_installed = false;

	delete s_focusObject;
	s_focusObject = false;
}

// ======================================================================
// class SkeletalAnimationDebugging
// ======================================================================

void SkeletalAnimationDebugging::install()
{
	DEBUG_FATAL(s_installed, ("SkeletalAnimationDebugging already installed."));

	s_focusObject = new ConstWatcher<Object>();

	s_installed = true;
	ExitChain::add(remove, "SkeletalAnimationDebugging");
}

// ----------------------------------------------------------------------

void SkeletalAnimationDebugging::setFocusObject(Object const *focusObject)
{
	DEBUG_FATAL(!s_installed, ("SkeletalAnimationDebugging not installed."));
	*s_focusObject = focusObject;
}

// ----------------------------------------------------------------------

Object const *SkeletalAnimationDebugging::getFocusObject()
{	
	DEBUG_FATAL(!s_installed, ("SkeletalAnimationDebugging not installed."));
	return s_focusObject->getPointer();
}

// ======================================================================
