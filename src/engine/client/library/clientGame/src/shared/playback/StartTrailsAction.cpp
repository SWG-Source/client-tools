// ======================================================================
//
// StartTrailsAction.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/StartTrailsAction.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/StartTrailsActionTemplate.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/HardpointObject.h"
#include "clientParticle/SwooshAppearance.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedUtility/DataTable.h"

#include <string>
#include <vector>

// ======================================================================

namespace StartTrailsActionNamespace
{
	const ConstCharCrcString cs_trailHardpointNames[] =
		{
			ConstCharCrcString("lankle"),
			ConstCharCrcString("rankle"),
			ConstCharCrcString("hold_l"),
			ConstCharCrcString("hold_r")
		};
	const int cs_trailEntryCount = static_cast<int>(sizeof(cs_trailHardpointNames) / sizeof(cs_trailHardpointNames[0]));

	struct WeaponHardpointInfo
	{
		WeaponHardpointInfo(const char *startName, const char *endName);

		ConstCharCrcString  startHardpointName; // Mandatory if we will show a trail.
		ConstCharCrcString  endHardpointName;   // Optional.  If present, we'll show a ribbon trail.  Otherwise we'll show a camera-facing single hardpoint trail.

	private:

		WeaponHardpointInfo & operator =(const WeaponHardpointInfo &);

		// Disabled.
		WeaponHardpointInfo();
	};

	WeaponHardpointInfo cs_weaponHardpointInfo[] =
		{
			WeaponHardpointInfo("tr1s", "tr1e"),
			WeaponHardpointInfo("tr2s", "tr2e")
		};
	const int cs_weaponHardpointInfoCount = static_cast<int>(sizeof(cs_weaponHardpointInfo) / sizeof(cs_weaponHardpointInfo[0]));

	const uint32 cs_weaponTrailMask = 0x10;

	const float cs_singleHardpointTrailWidth = 0.06f;

	bool s_installed = false;

	void remove();

	Tag const TAG_AWPN = TAG(A,W,P,N);
}

using namespace StartTrailsActionNamespace;

// ======================================================================

// ----------------------------------------------------------------------

StartTrailsActionNamespace::WeaponHardpointInfo::WeaponHardpointInfo(const char *startName, const char *endName) :
	startHardpointName(startName),
	endHardpointName(endName)
{
}

// ----------------------------------------------------------------------

void StartTrailsActionNamespace::remove()
{
	s_installed = false;
}

// ======================================================================
// StartTrailsAction: public member functions
// ======================================================================

// ----------------------------------------------------------------------

void StartTrailsAction::install()
{
	InstallTimer const installTimer("StartTrailsAction::install");

	DEBUG_FATAL(s_installed, ("Already installed."));

	ExitChain::add(&remove, "StartTrailsActionNamespace::install");

	s_installed = true;
}

// ----------------------------------------------------------------------

StartTrailsAction::~StartTrailsAction()
{
	if (m_objectWatcherVector)
	{
		killTrailAppearances();
		m_objectWatcherVector = 0;
	}
}

// ----------------------------------------------------------------------

void StartTrailsAction::cleanup(PlaybackScript &playbackScript)
{
	UNREF(playbackScript);

	if (m_objectWatcherVector)
		killTrailAppearances();
}

// ----------------------------------------------------------------------

bool StartTrailsAction::update(float deltaTime, PlaybackScript &script)
{
	UNREF(deltaTime);

	//-- Get the trail bitfield from the playback script.
	const StartTrailsActionTemplate &ourTemplate = getOurActionTemplate();
	
	int trailsBitfield;

	if (ConfigClientGame::getForceTrailsOnAllActions())
		trailsBitfield = -1;
	else
	{
		const bool gotVariable = script.getIntVariable(ourTemplate.getTrailBitfieldVariable(), trailsBitfield);
		if (!gotVariable || (trailsBitfield == 0))
		{
			// Nothing to do.
			return false;
		}
	}

	const int actorIndex = ourTemplate.getActorIndex();
	Object *const actorObject = script.getActor(actorIndex);
	if (!actorObject)
	{
		DEBUG_REPORT_LOG(ConfigClientGame::getLogTrailActions(), ("StartTrailsAction: skipping trails because actor object [index=%d] is NULL.\n", actorIndex));
		return false;
	}

	// get the weapon object
	Object *object = script.getObjectVariable(TAG_AWPN);
	WeaponObject * const weaponObject = dynamic_cast<WeaponObject *>(object);

	//-- Create the trails.
	m_objectWatcherVector = new ObjectWatcherVector();

	//-- Check combat_manager.mif for the swoosh name
	VectorArgb color(VectorArgb::solidWhite);
	std::string swooshFilename;

#ifdef _DEBUG
	if (   SwooshAppearanceTemplate::useFinalGameSwooshes()
	    || !Game::isParticleEditor())
	{
#endif // _DEBUG

		if (weaponObject)
		{
			// get the swoosh from the combat effects manager
			CombatEffectsManager::getSwooshEffect(*weaponObject, swooshFilename, color);
		}
		else
		{
			// get the swoosh from the script
			getSwooshFilename(script, swooshFilename);
		}

#ifdef _DEBUG
	}
#endif // _DEBUG

	createTrailObjects(static_cast<uint32>(trailsBitfield), *m_objectWatcherVector, *actorObject, swooshFilename, color);

	//-- Signal that the action is complete.
	return false;
}

// ----------------------------------------------------------------------

void StartTrailsAction::killTrailAppearances()
{
	if (!m_objectWatcherVector)
		return;

	const ObjectWatcherVector::iterator endIt = m_objectWatcherVector->end();
	for (ObjectWatcherVector::iterator it = m_objectWatcherVector->begin(); it != endIt; ++it)
	{
		ObjectWatcher *const objectWatcher = *it;
		NOT_NULL(objectWatcher);

		Object *const object = objectWatcher->getPointer();
		if (object)
		{
			bool deletedAppearance = false;

			//-- Handle SwooshAppearance.
			Appearance *const appearance = object->getAppearance();
			SwooshAppearance *const swooshAppearance = static_cast<SwooshAppearance*>(appearance);
			if (swooshAppearance)
			{
				//-- Instruct the ribbon appearance to kill itself, fading out over time.
				swooshAppearance->kill();
				deletedAppearance = true;
			}

			//-- Handle all unhandled appearance types.
			if (!deletedAppearance)
			{
				//-- Delete the object now since we don't know what type it is.
				object->kill();
			}

			//-- Ensure the dying trail object gets an alter.
			object->scheduleForAlter();
		}

		delete objectWatcher;
	}
	
	//-- Delete the vector.
	delete m_objectWatcherVector;
	m_objectWatcherVector = 0;
}

// ======================================================================
// StartTrailsAction: private static member functions
// ======================================================================

void StartTrailsAction::createTrailObjects(uint32 trailBitfield, ObjectWatcherVector &objectWatcherVector, Object &parentObject, const std::string &swooshFilename, VectorArgb const &color)
{
	//-- Scan each trail bit entry to see if it is set.  This part handles the non-weapon trails.
	uint32 testMask = 0x01;

	for (int trailIndex = 0; trailIndex < cs_trailEntryCount; ++trailIndex, testMask <<= 1)
	{
		const bool hasThisTrail = ((trailBitfield & testMask) != 0);
		if (hasThisTrail)
		{
			//-- Create the trail.
			// Get the hardpoint name.
			const CrcString &hardpointName = cs_trailHardpointNames[trailIndex];

			DEBUG_REPORT_LOG(ConfigClientGame::getLogTrailActions(), ("StartTrailsAction: trail mask [0x%04x] set for hardpoint [%s] on object [id=%s].\n", testMask, hardpointName.getString(), parentObject.getNetworkId().getValueString().c_str()));

			// Create the hardpoint object.
			Object *const childObject = new Object();
			RenderWorld::addObjectNotifications(*childObject);


			SwooshAppearance *swooshAppearance = NULL;

			if (swooshFilename.empty())
			{
				const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(SwooshAppearanceTemplate::getDefaultTemplate()));

				// Create the trail appearance.
				swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, parentObject.getAppearance(), hardpointName);
			}
			else
			{
				const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(swooshFilename.c_str()));

				// Create the trail appearance.
				swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, parentObject.getAppearance(), hardpointName);

				AppearanceTemplateList::release(swooshAppearanceTemplate);
			}

			if (swooshAppearance)
			{
				childObject->setAppearance (swooshAppearance);
				swooshAppearance->setColorModifier(color);
			}

			// Add child object to the trail object.
			parentObject.addChildObject_o(childObject);

			// Track the child object.
			objectWatcherVector.push_back(new ObjectWatcher(childObject));
		}
	}

	//-- Handle weapon trails.  This involves looking to see if a weapon has the specified hardpoints.
	if ((trailBitfield & cs_weaponTrailMask) != 0)
	{
		// Get the weapon's appearance.
		Appearance *const weaponAppearance = parentObject.getAppearance();
		DEBUG_WARNING(!weaponAppearance, ("Weapon object [id=%s] has a NULL appearance, cannot add weapon trails.", parentObject.getNetworkId().getValueString().c_str()));
		
		Transform  m_junkTransform(Transform::IF_none);

		if (weaponAppearance)
		{
			for (int infoIndex = 0; infoIndex < cs_weaponHardpointInfoCount; ++infoIndex)
			{
				const WeaponHardpointInfo &info = cs_weaponHardpointInfo[infoIndex];

				const bool hasStartHardpoint = weaponAppearance->findHardpoint(info.startHardpointName, m_junkTransform);
				if (hasStartHardpoint)
				{
					const bool hasEndHardpoint = weaponAppearance->findHardpoint(info.endHardpointName, m_junkTransform);
					if (hasEndHardpoint)
					{
						// Apply a single hardpoint-based trail.
						// Create the hardpoint object.
						Object * const childObject = new Object();
						RenderWorld::addObjectNotifications(*childObject);
						SwooshAppearance *swooshAppearance = NULL;

						if (swooshFilename.empty())
						{
							const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(SwooshAppearanceTemplate::getDefaultTemplate()));

							// Create the trail appearance.
							swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, weaponAppearance, info.startHardpointName, info.endHardpointName);
						}
						else
						{
							const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(swooshFilename.c_str()));

							// Create the trail appearance.
							swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, weaponAppearance, info.startHardpointName, info.endHardpointName);
							AppearanceTemplateList::release(swooshAppearanceTemplate);
						}

						swooshAppearance->setColorModifier(color);
						childObject->setAppearance (swooshAppearance);

						// Add child object to the trail object.
						parentObject.addChildObject_o(childObject);

						// Track the child object.
						objectWatcherVector.push_back(new ObjectWatcher(childObject));
					}
					else
					{
						// Apply a single hardpoint-based trail.
						// Create the hardpoint object.
						Object * const childObject = new Object();
						RenderWorld::addObjectNotifications(*childObject);
						SwooshAppearance *swooshAppearance = NULL;

						if (swooshFilename.empty())
						{
							const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(SwooshAppearanceTemplate::getDefaultTemplate()));

							// Create the trail appearance.
							swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, weaponAppearance, info.startHardpointName);
						}
						else
						{
							const SwooshAppearanceTemplate *swooshAppearanceTemplate = static_cast<SwooshAppearanceTemplate const *>(AppearanceTemplateList::fetch(swooshFilename.c_str()));

							// Create the trail appearance.
							swooshAppearance = new SwooshAppearance(swooshAppearanceTemplate, weaponAppearance, info.startHardpointName);
							AppearanceTemplateList::release(swooshAppearanceTemplate);
						}

						swooshAppearance->setColorModifier(color);
						childObject->setAppearance (swooshAppearance);

						// Add child object to the trail object.
						parentObject.addChildObject_o(childObject);

						// Track the child object.
						objectWatcherVector.push_back(new ObjectWatcher(childObject));
					}
				}
			}
		}
	}
}

// ======================================================================
// StartTrailsAction: private member functions
// ======================================================================

StartTrailsAction::StartTrailsAction(const StartTrailsActionTemplate &actionTemplate) :
	PlaybackAction(&actionTemplate),
	m_objectWatcherVector(0)
{
}

// ----------------------------------------------------------------------

StartTrailsActionTemplate const &StartTrailsAction::getOurActionTemplate() const
{
	return *NON_NULL(safe_cast<StartTrailsActionTemplate const*>(getPlaybackActionTemplate()));
}

// ----------------------------------------------------------------------

void StartTrailsAction::getSwooshFilename(PlaybackScript const &script, std::string swooshFilename)
{
	StartTrailsActionTemplate const &actionTemplate = getOurActionTemplate();
	
	swooshFilename.clear();
	bool const gotValue = script.getStringVariable(actionTemplate.getSwooshFilenameVariable(), swooshFilename);
	if (!gotValue)
		swooshFilename.clear();
}

// ======================================================================
