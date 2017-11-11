// ======================================================================
//
// CharacterLodManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"

#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectWatcherList.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <map>

// ======================================================================

#define CLM_SUPPORTED_LOD_COUNT  4

// ======================================================================

namespace CharacterLodManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::multimap<float, Object*>  FloatObjectMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();
	void  computeCharacterLodSumTable();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float const cs_supportedLodCount         = CLM_SUPPORTED_LOD_COUNT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float s_characterLodSumTable[CLM_SUPPORTED_LOD_COUNT - 1];

	int const cs_characterLodSumEntryCount = sizeof(s_characterLodSumTable) / sizeof(s_characterLodSumTable[0]);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                                    s_installed;
	CharacterLodManager::ManageLodCallback  s_manageLodCallback;

	ObjectWatcherList                       s_characters;
	FloatObjectMap                          s_charactersByDistanceSquared;

	bool                                    s_isEnabled;
	float                                   s_firstLodCount;

	int                                     s_firstAssignableLodIndex;
	int                                     s_everyOtherFrameSkinningCharacterCount;
	int                                     s_hardSkinningCharacterCount;
}

using namespace CharacterLodManagerNamespace;

// ======================================================================
// namespace CharacterLodManagerNamespace
// ======================================================================

void CharacterLodManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("CharacterLodManager not installed."));
	s_installed = false;

	s_characters.removeAll(false);
	s_charactersByDistanceSquared.clear();
	s_manageLodCallback = NULL;
}

// ----------------------------------------------------------------------

void CharacterLodManagerNamespace::computeCharacterLodSumTable()
{
	//-- Compute the allowable LOD sum table.
	float lodAllowCount  = s_firstLodCount;
	float allowSum       = lodAllowCount;

	s_characterLodSumTable[0] = static_cast<float>(floor(static_cast<double>(allowSum)));

	for (int i = 1; i < cs_characterLodSumEntryCount; ++i)
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, i, cs_characterLodSumEntryCount);

		lodAllowCount *= 2;
		allowSum      += lodAllowCount;

		s_characterLodSumTable[i] = static_cast<float>(floor(static_cast<double>(allowSum)));
	}

	//-- Find the first non-zero lod index.
	for (s_firstAssignableLodIndex = 0; s_firstAssignableLodIndex < cs_characterLodSumEntryCount; ++s_firstAssignableLodIndex)
	{
		if (s_characterLodSumTable[s_firstAssignableLodIndex] > 0)
			break;
	}
}

// ======================================================================
// class CharacterLodManager: PUBLIC STATIC
// ======================================================================

void CharacterLodManager::install()
{
	DEBUG_FATAL(s_installed, ("CharacterLodManager already installed."));

	setConfiguration(
		ConfigClientSkeletalAnimation::getLodManagerEnable(), 
		ConfigClientSkeletalAnimation::getLodManagerFirstLodCount(), 
		ConfigClientSkeletalAnimation::getLodManagerEveryOtherFrameSkinningCharacterCount(), 
		ConfigClientSkeletalAnimation::getLodManagerHardSkinningCharacterCount());
	
	LocalMachineOptionManager::registerOption (s_isEnabled, "ClientGame/ClientSkeletalAnimation", "isEnabled");
	LocalMachineOptionManager::registerOption (s_firstLodCount, "ClientGame/ClientSkeletalAnimation", "firstLodCount");
	LocalMachineOptionManager::registerOption (s_everyOtherFrameSkinningCharacterCount, "ClientGame/ClientSkeletalAnimation", "everyOtherFrameSkinningCharacterCount");
	LocalMachineOptionManager::registerOption (s_hardSkinningCharacterCount, "ClientGame/ClientSkeletalAnimation", "hardSkinningCharacterCount");

	s_installed = true;
	ExitChain::add(remove, "CharacterLodManager");
}

// ----------------------------------------------------------------------

void CharacterLodManager::setManageLodCallback(ManageLodCallback callback)
{
	s_manageLodCallback = callback;
}

// ----------------------------------------------------------------------

void CharacterLodManager::getConfiguration(bool &enabled, float &firstLodCount, int &everyOtherFrameSkinningCharacterCount, int &hardSkinningCharacterCount)
{
	enabled                               = s_isEnabled;
	firstLodCount                         = s_firstLodCount;
	everyOtherFrameSkinningCharacterCount = s_everyOtherFrameSkinningCharacterCount;
	hardSkinningCharacterCount            = s_hardSkinningCharacterCount;
}

// ----------------------------------------------------------------------

void CharacterLodManager::setConfiguration(bool enabled, float firstLodCount, int everyOtherFrameSkinningCharacterCount, int hardSkinningCharacterCount)
{
	DEBUG_FATAL(firstLodCount < 0.0f, ("firstLodCount=[%g] is too low, should produce at least one allowable lod at max lod index [%d].", firstLodCount, cs_supportedLodCount));

	s_isEnabled                             = enabled;
	s_firstLodCount                         = firstLodCount;
	s_everyOtherFrameSkinningCharacterCount = everyOtherFrameSkinningCharacterCount;
	s_hardSkinningCharacterCount            = hardSkinningCharacterCount;

	if (s_isEnabled)
		computeCharacterLodSumTable();
}

// ----------------------------------------------------------------------

bool CharacterLodManager::isEnabled()
{
	return s_isEnabled;
}

// ----------------------------------------------------------------------

void CharacterLodManager::addCharacter(Object *object)
{
	DEBUG_FATAL(!s_installed, ("CharacterLodManager not installed."));

	NOT_NULL(object);
	s_characters.addObject(*object);
}

// ----------------------------------------------------------------------

void CharacterLodManager::planNextFrame(Vector const &cameraPosition_w)
{
	DEBUG_FATAL(!s_installed, ("CharacterLodManager not installed."));

	//-- Sort each character submitted by distance from camera.  The objects
	//   submitted should have been objects rendered the previous frame.
	int const objectCount = s_characters.getNumberOfObjects();
	for (int i = 0; i < objectCount; ++i)
	{
		Object *const object = s_characters.getObject(i);
		if (object)
		{
			if (s_manageLodCallback && (*s_manageLodCallback)(*object))
			{
				float const separationSquared = cameraPosition_w.magnitudeBetweenSquared(object->getPosition_w());
				IGNORE_RETURN(s_charactersByDistanceSquared.insert(FloatObjectMap::value_type(separationSquared, object)));
			}
		}
	}

	//-- Remove all characters from the list submitted this frame.
	s_characters.removeAll(false);

	//-- Assign SkeletalAppearance2 LOD indices by proximity to camera.
	int  currentAssignedLodIndex = s_firstAssignableLodIndex;
	int  assignedCount           = 0;

	FloatObjectMap::iterator const endIt = s_charactersByDistanceSquared.end();
	for (FloatObjectMap::iterator it = s_charactersByDistanceSquared.begin(); it != endIt; ++it)
	{
		NOT_NULL(it->second);
		Appearance *const baseAppearance = it->second->getAppearance();
		NOT_NULL(baseAppearance);

		SkeletalAppearance2 *const appearance = baseAppearance->asSkeletalAppearance2();
		if (appearance)
		{
			++assignedCount;

			//-- Assign the lod index for this appearance.
			appearance->setPlannedLodIndex(std::min(currentAssignedLodIndex, appearance->getDetailLevelCount() - 1));
			appearance->setEveryOtherFrameSkinningEnabled(assignedCount >= s_everyOtherFrameSkinningCharacterCount);
			appearance->setForceHardSkinningEnabled(assignedCount >= s_hardSkinningCharacterCount);

			//-- Transition to next lod index assignment as necessary.
			if ((currentAssignedLodIndex < cs_characterLodSumEntryCount) && (assignedCount >= s_characterLodSumTable[currentAssignedLodIndex]))
				++currentAssignedLodIndex;
		}
	}

	//-- Clear out map for next frame.
	s_charactersByDistanceSquared.clear();
}

// ======================================================================
