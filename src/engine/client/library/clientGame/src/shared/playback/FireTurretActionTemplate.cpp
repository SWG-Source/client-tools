// ======================================================================
//
// FireTurretActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/FireTurretActionTemplate.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientAnimation/WatcherAction.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/TurretObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"

// ======================================================================

const Tag TAG_FTAT = TAG(F,T,A,T);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool FireTurretActionTemplate::ms_installed;

// ======================================================================
// class FireTurretActionTemplate: public static member functions
// ======================================================================

void FireTurretActionTemplate::install()
{
	InstallTimer const installTimer("FireTurretActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("FireTurretActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_FTAT, create);

	ms_installed = true;
	ExitChain::add(remove, "FireTurretActionTemplate");
}

// ======================================================================
// class FireTurretActionTemplate: public member functions
// ======================================================================

PlaybackAction *FireTurretActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool FireTurretActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Get attacker actor.  This should be the TurretObject.
	InstallationObject *const installationObject = dynamic_cast<InstallationObject*>(script.getActor(getAttackerActorIndex()));
	if (!installationObject)
	{
		DEBUG_REPORT_LOG(true, ("FTAT: aborting attack, attacker is not an installation object.\n"));
		script.abortPlayback();

		return false;
	}

#if 1
	TurretObject *turretObject = 0;

	const int childObjectCount = installationObject->getNumberOfChildObjects();
	for (int i = 0; i < childObjectCount; ++i)
	{
		turretObject = dynamic_cast<TurretObject*>(installationObject->getChildObject(i));
		if (turretObject)
			break;
	}

#else
	TurretObject *const turretObject = dynamic_cast<TurretObject*>(installationObject->getTurretObject());
#endif
	if (!turretObject)
	{
		DEBUG_REPORT_LOG(true, ("FTAT: aborting attack, attacker installation does not have a turret object.\n"));
		script.abortPlayback();

		return false;
	}

	//-- Get defender actor.
	Object *const defenderObject = script.getActor(getDefenderActorIndex());
	if (!defenderObject)
	{
		DEBUG_REPORT_LOG(true, ("FTAT: aborting attack, defender object is NULL.\n"));
		script.abortPlayback();

		return false;
	}

	//-- Determine if defender is hit.
	int damageAmount = 0;

	const bool gotIsHit = script.getIntVariable(getIsHitVariable(), damageAmount);
	DEBUG_WARNING(!gotIsHit, ("failed to get isHit variable, did the .pst forget to declare this?"));
	UNREF(gotIsHit);

	const bool isHit = gotIsHit && (damageAmount > 0);

	//-- Instruct turret to fire.
	Object *const projectileObject = const_cast<Object *>(turretObject->fire(defenderObject, isHit));

	//-- Setup the watcher object.
	if ((m_watcherThreadIndex >= 0) && (m_watcherActionIndex >= 0))
	{
		WatcherAction *const watcherAction = safe_cast<WatcherAction*>(script.getAction(m_watcherThreadIndex, m_watcherActionIndex));
		if (watcherAction)
			watcherAction->setWatcherObject(projectileObject);
		else
			DEBUG_WARNING(true, ("FTAT: thread [%d] action [%d] is not a watcher.", m_watcherThreadIndex, m_watcherActionIndex));
	}

	//-- Success, no more processing.
	return false;
}

// ======================================================================
// class FireTurretActionTemplate: private static member functions
// ======================================================================

void FireTurretActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("FireTurretActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_FTAT);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *FireTurretActionTemplate::create(Iff &iff)
{
	return new FireTurretActionTemplate(iff);
}

// ======================================================================
// class FireTurretActionTemplate: private member functions
// ======================================================================

FireTurretActionTemplate::FireTurretActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_attackerActorIndex(-1),
	m_defenderActorIndex(-1),
	m_watcherThreadIndex(-1),
	m_watcherActionIndex(-1),
	m_isHitVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("FireTurretActionTemplate not installed."));

	iff.enterForm(TAG_FTAT);

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
					DEBUG_FATAL(true, ("unsupported FireTurretActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_FTAT);
}

// ----------------------------------------------------------------------

void FireTurretActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_attackerActorIndex                 = static_cast<int>(iff.read_int16());
			m_defenderActorIndex                 = static_cast<int>(iff.read_int16());

			m_watcherThreadIndex                 = static_cast<int>(iff.read_int16());
			m_watcherActionIndex                 = static_cast<int>(iff.read_int16());

			m_isHitVariable                      = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
