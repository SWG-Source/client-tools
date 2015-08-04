// ======================================================================
//
// ChangePostureActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ChangePostureActionTemplate.h"

#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientGame/CreatureObject.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "swgSharedUtility/Postures.def"

// ======================================================================

bool ChangePostureActionTemplate::ms_installed;

// ======================================================================

const Tag TAG_CPSA = TAG(C,P,S,A);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================
// class ChangePostureActionTemplate: public static member functions
// ======================================================================

void ChangePostureActionTemplate::install()
{
	InstallTimer const installTimer("ChangePostureActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("ChangePostureActionTemplate already installed"));

	PlaybackScriptTemplate::registerActionTemplate(TAG_CPSA, create);

	ms_installed = true;
	ExitChain::add(remove, "ChangePostureActionTemplate");
}

// ======================================================================
// class ChangePostureActionTemplate: public member functions
// ======================================================================

PlaybackAction *ChangePostureActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool ChangePostureActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Retrieve the actor Object instance.
	CreatureObject *const object = dynamic_cast<CreatureObject*>(script.getActor(getActorIndex()));
	if (!object)
	{
		// Nothing to do.
		return false;
	}

	//-- Get the new game posture.
	const int newPosture = getDestinationPosture(script);
	if (newPosture < 0)
	{
		//-- Specifying a -1 posture change means the caller doesn't really want a posture change.
		return false;
	}

	//-- Set the new posture.
	object->setVisualPosture(static_cast<Postures::Enumerator>(newPosture));

	//-- Nothing more to do.
	return false;
}

// ======================================================================
// class ChangePostureActionTemplate: private static member functions
// ======================================================================

void ChangePostureActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("ChangePostureActionTemplate not installed."));

	ms_installed = false;
	PlaybackScriptTemplate::deregisterActionTemplate(TAG_CPSA);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *ChangePostureActionTemplate::create(Iff &iff)
{
	return new ChangePostureActionTemplate(iff);
}

// ======================================================================
// class ChangePostureActionTemplate: private member functions
// ======================================================================

ChangePostureActionTemplate::ChangePostureActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_actorIndex(0),
	m_destinationPostureVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("ChangePostureActionTemplate not installed."));

	iff.enterForm(TAG_CPSA);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					DEBUG_FATAL(true, ("ChangePostureActionTemplate: unsupported version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_CPSA);
}

// ----------------------------------------------------------------------

void ChangePostureActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_actorIndex                 = static_cast<int>(iff.read_int16());
			m_destinationPostureVariable = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

int ChangePostureActionTemplate::getActorIndex() const
{
	return m_actorIndex;
}

// ----------------------------------------------------------------------

int ChangePostureActionTemplate::getDestinationPosture(const PlaybackScript &script) const
{
	int posture = 0;

	IGNORE_RETURN(script.getIntVariable(m_destinationPostureVariable, posture));
	return posture;
}

// ======================================================================
