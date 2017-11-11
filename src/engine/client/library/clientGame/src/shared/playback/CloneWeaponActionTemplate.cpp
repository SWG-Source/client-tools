// ======================================================================
//
// CloneWeaponActionTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CloneWeaponActionTemplate.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/PassthroughPlaybackAction.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGraphics/RenderWorld.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"

// ======================================================================

const Tag TAG_AWPN = TAG(A,W,P,N);
const Tag TAG_CWAT = TAG(C,W,A,T);
const Tag TAG_NONE = TAG(N,O,N,E);

// ======================================================================

bool CloneWeaponActionTemplate::ms_installed;

// ======================================================================
// class CloneWeaponActionTemplate: public static member functions
// ======================================================================

void CloneWeaponActionTemplate::install()
{
	InstallTimer const installTimer("CloneWeaponActionTemplate::install");

	DEBUG_FATAL(ms_installed, ("CloneWeaponActionTemplate already installed."));

	PlaybackScriptTemplate::registerActionTemplate(TAG_CWAT, create);

	ms_installed = true;
	ExitChain::add(remove, "CloneWeaponActionTemplate");
}

// ======================================================================
// class CloneWeaponActionTemplate: public member functions
// ======================================================================

PlaybackAction *CloneWeaponActionTemplate::createPlaybackAction() const
{
	return new PassthroughPlaybackAction(*this);
}

// ----------------------------------------------------------------------

bool CloneWeaponActionTemplate::update(float deltaTime, PlaybackScript &script) const
{
	UNREF(deltaTime);

	//-- Clone the weapon object.
	// Get the appropriate ObjectTemplate to use.
	const ObjectTemplate *const objectTemplate = fetchWeaponObjectTemplate(script);
	if (!objectTemplate)
	{
		DEBUG_WARNING(true, ("CWAT: failed to fetch the weapon's object template for cloning purposes."));
		return false;
	}

	// Create the object from the object template.
	Object *const object = objectTemplate->createObject();

	// Release the object template.
	objectTemplate->releaseReference();

	if (object)
	{
		ClientObject *const clientObject = object->asClientObject();
		if (clientObject)
			clientObject->endBaselines();
	}
	else
	{
		DEBUG_WARNING(true, ("objectTemplate [%s]'s createObject() returned NULL.", objectTemplate->getName()));
		return false;
	}

	// Set notifications on the object.
	RenderWorld::addObjectNotifications(*object);
	//object->addNotification(ClientWorld::getIntangibleNotification());

	//-- Get the appearance for the attacking object.
	SkeletalAppearance2 *const appearance = getActorAppearance(script);
	if (!appearance)
	{
		DEBUG_WARNING(true, ("the actor (index = %d) does not have a skeletal appearance.", getActorIndex()));
		return false;
	}

	//-- Attach the object to the specified hardpoint.
	// Get the hardpoint name.
	CrcLowerString  hardpointName;
	if (!getHardpointName(script, hardpointName))
	{
		DEBUG_WARNING(true, ("failed to get hardpoint name."));
		return false;
	}

	// Attach object to the hardpoint.
	appearance->attach(object, hardpointName);

	//-- Success, no more processing.
	return false;
}

// ======================================================================
// class CloneWeaponActionTemplate: private static member functions
// ======================================================================

void CloneWeaponActionTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("CloneWeaponActionTemplate not installed."));
	ms_installed = false;

	PlaybackScriptTemplate::deregisterActionTemplate(TAG_CWAT);
}

// ----------------------------------------------------------------------

PlaybackActionTemplate *CloneWeaponActionTemplate::create(Iff &iff)
{
	return new CloneWeaponActionTemplate(iff);
}

// ======================================================================
// class CloneWeaponActionTemplate: private member functions
// ======================================================================

CloneWeaponActionTemplate::CloneWeaponActionTemplate(Iff &iff) :
	PlaybackActionTemplate(),
	m_actorIndex(-1),
	m_fallbackObjectTemplateNameVariable(TAG_NONE),
	m_hardpointNameVariable(TAG_NONE)
{
	DEBUG_FATAL(!ms_installed, ("CloneWeaponActionTemplate not installed."));

	iff.enterForm(TAG_CWAT);

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
					DEBUG_FATAL(true, ("unsupported CloneWeaponActionTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_CWAT);
}

// ----------------------------------------------------------------------

void CloneWeaponActionTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			m_actorIndex                         = static_cast<int>(iff.read_int16());
			m_fallbackObjectTemplateNameVariable = static_cast<Tag>(iff.read_uint32());
			m_hardpointNameVariable              = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

const ObjectTemplate *CloneWeaponActionTemplate::fetchWeaponObjectTemplate(const PlaybackScript &script) const
{
	//-- Get the weapon object from the playback script.
	const Object *weaponObject = script.getObjectVariable(TAG_AWPN);
	
	//if an FBOT is specified in the script, use it
	std::string fbotTemplate;
	if (script.getStringVariable(getFallbackObjectTemplateNameVariable(), fbotTemplate) && !fbotTemplate.empty())
	{
		// Ensure the file exists.
		if (!TreeFile::exists(fbotTemplate.c_str()))
		{
			DEBUG_WARNING(true, ("CWAT: object template file [%s] does not exist.", fbotTemplate.c_str()));
			return 0;
		}

		// Load the object template.
		DEBUG_REPORT_LOG(ConfigClientGame::getLogCloneWeaponAction(), ("CWAT: weapon object template specified in script [%s].\n", fbotTemplate.c_str()));
		return ObjectTemplateList::fetch(fbotTemplate);
	}
	else if (weaponObject)
	{
		//-- Fetch the ObjectTemplate for the weapon object.
		const ObjectTemplate *const objectTemplate = weaponObject->getObjectTemplate();
		if (objectTemplate)
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getLogCloneWeaponAction(), ("CWAT: weapon object cloned from given weapon's template [%s].\n", objectTemplate->getName()));

			// Fetch reference for caller.
			objectTemplate->addReference();
		}
		else
		{
			DEBUG_REPORT_LOG(ConfigClientGame::getLogCloneWeaponAction(), ("CWAT: given weapon object has a NULL object template, can't clone.\n"));
		}

		return objectTemplate;
	}

	return 0;
}

// ----------------------------------------------------------------------

SkeletalAppearance2 *CloneWeaponActionTemplate::getActorAppearance(PlaybackScript &script) const
{
	//-- Get the actor object.
	Object *const object = script.getActor(getActorIndex());
	if (!object)
		return 0;

	//-- Return the appearance.
	return dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
}

// ----------------------------------------------------------------------

bool CloneWeaponActionTemplate::getHardpointName(const PlaybackScript &script, CrcLowerString &name) const
{
	//-- Retrieve the hardpoint name.
	std::string  hardpointName;

	const bool gotName = script.getStringVariable(getHardpointNameVariable(), hardpointName);
	if (!gotName)
		return false;

	//-- Set the string.
	name.setString(hardpointName.c_str());
	return true;
}

// ======================================================================
