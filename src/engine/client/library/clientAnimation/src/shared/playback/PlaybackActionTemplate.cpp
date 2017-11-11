// ======================================================================
//
// PlaybackActionTemplate.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientAnimation/FirstClientAnimation.h"
#include "clientAnimation/PlaybackActionTemplate.h"

// ======================================================================

bool PlaybackActionTemplate::update(float /* deltaTime */, PlaybackScript & /* script */) const
{
	// this implementation should never be called.  So then, why is it
	// provided?  I provide it because of this.  Consider a PlaybackActionTemplate that 
	// creates an associated PlaybackAction class.  The PlaybackAction class handles
	// the update and never passes it on to the PlaybackActionTemplate.  Should the
	// PlaybackActionTemplate-derived class be forced to implement a do-nothing function
	// in this case?  I'd prefer that not to be the case.
	//
	// So then, why provide this function at all if the PlaybackAction-derived class
	// should always know what kind of PlaybackActionTemplate it has?  I have a
	// PassthroughAction that just calls the PlaybackActionTemplate's update, and this
	// is convenient for any Action that does not require per-action unique state.
	DEBUG_FATAL(true, ("default action template should never be called\n"));
	return false;
}

// ----------------------------------------------------------------------
/**
 * Call this function when the playback action template is controlling activity
 * that needs to stop such that the next call to update will return false.
 * Any state associated with the action template can be cleared at this call time.
 */

void PlaybackActionTemplate::stop(PlaybackScript & /* script */) const
{
	// Default implementation does nothing.
}

// ----------------------------------------------------------------------

void PlaybackActionTemplate::release() const
{
	--m_referenceCount;

	if (m_referenceCount < 1)
	{
		DEBUG_WARNING(m_referenceCount < 0, ("bad reference handling logic [%d].\n", m_referenceCount));
		delete const_cast<PlaybackActionTemplate*>(this);
	}
}

// ======================================================================

PlaybackActionTemplate::PlaybackActionTemplate() :
	m_referenceCount(0)
{
}


// ----------------------------------------------------------------------

PlaybackActionTemplate::~PlaybackActionTemplate()
{
}

// ======================================================================
