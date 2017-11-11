// ======================================================================
//
// AnimationMessageActionTemplate.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationMessageActionTemplate.h"

#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientSkeletalAnimation/AnimationMessageAction.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"

// ======================================================================

const Tag TAG_AMSG = TAG(A,M,S,G);

// ======================================================================

bool AnimationMessageActionTemplate::ms_installed;

// ======================================================================
// public static member functions
// ======================================================================

void AnimationMessageActionTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("AnimationMessageActionTemplate already installed"));

	//-- register the trigger template creation function
	PlaybackScriptTemplate::registerActionTemplate(TAG_AMSG, create);

	ms_installed = true;
	ExitChain::add(remove, "AnimationMessageActionTemplate");
}

// ======================================================================
// public member functions
// ======================================================================

PlaybackAction *AnimationMessageActionTemplate::createPlaybackAction() const
{
	return new AnimationMessageAction(*this);
}

// ----------------------------------------------------------------------

const Tag &AnimationMessageActionTemplate::getMessageNameVariable() const
{
	return m_messageNameVariable;
}

// ----------------------------------------------------------------------

int AnimationMessageActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ======================================================================
// private static member functions
// ======================================================================

void AnimationMessageActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("AnimationMessageActionTemplate not installed"));

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_AMSG);

	ms_installed = false;
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *AnimationMessageActionTemplate::create(Iff &iff)
{
	return new AnimationMessageActionTemplate(iff);
}

// ======================================================================
// private member functions
// ======================================================================

AnimationMessageActionTemplate::AnimationMessageActionTemplate(Iff &iff)
:	PlaybackActionTemplate(),
	m_messageNameVariable(TAG_0000),
	m_actorIndex(-1)
{
	iff.enterForm(TAG_AMSG);

		const Tag version = iff.getCurrentName();
		if (version == TAG_0000)
			load_0000(iff);
		else
		{
			char buffer[5];
			ConvertTagToString(version, buffer);
			FATAL(true, ("unsupported AnimationMessageActionTemplate version [%s]", buffer)); 
		}

	iff.exitForm(TAG_AMSG);
}

// ----------------------------------------------------------------------

void AnimationMessageActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		iff.enterChunk(TAG_INFO);

			m_actorIndex = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(m_actorIndex < 0, ("invalid actor index %d", m_actorIndex));

			m_messageNameVariable = Tag(iff.read_uint32());

		iff.exitChunk(TAG_INFO);
	}
	iff.exitForm(TAG_0000);
}

// ======================================================================
