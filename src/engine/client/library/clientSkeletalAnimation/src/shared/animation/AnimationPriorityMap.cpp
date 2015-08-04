// ======================================================================
//
// AnimationPriorityMap.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationPriorityMap.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"

#include <algorithm>
#include <map>

// ======================================================================

const Tag TAG_APRM = TAG(A,P,R,M);

// ======================================================================

const int                           AnimationPriorityMap::cms_defaultPriority = 0;

bool                                AnimationPriorityMap::ms_installed;
AnimationPriorityMap::StringIntMap *AnimationPriorityMap::ms_priorityGroupMap;

// ======================================================================
// class AnimationPriorityMap: public static member functions
// ======================================================================

void AnimationPriorityMap::install(const char *priorityMapFileName)
{
	DEBUG_FATAL(ms_installed, ("AnimationPriorityMap already installed."));

	//-- Initialize data structures.
	ms_priorityGroupMap = new StringIntMap();

	//-- Load data.
	Iff  iff;

	const bool success = iff.open(priorityMapFileName, true);
	if (!success)
	{
		DEBUG_WARNING(true, ("AnimationPriorityMap data file [%s] not found, defaulting to no entries.", priorityMapFileName));
	}
	else
	{
		iff.enterForm(TAG_APRM);
		
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
						DEBUG_FATAL(true, ("unsupported AnimationPriorityMap file version [%s].", buffer));
					}
			}

		iff.exitForm(TAG_APRM);
	}

	ms_installed = true;
	ExitChain::add(remove, "AnimationPriorityMap");
}

// ----------------------------------------------------------------------

int AnimationPriorityMap::getPriorityValue(const CrcLowerString &priorityGroupName)
{
	DEBUG_FATAL(!ms_installed, ("AnimationPriorityMap not installed."));

	const StringIntMap::iterator findIt = ms_priorityGroupMap->find(priorityGroupName);
	if (findIt != ms_priorityGroupMap->end())
	{
		// Found it, return it.
		return findIt->second;
	}
	else
	{
		// Did not find it, return default.
		DEBUG_WARNING(true, ("AnimationPriorityMap: attempted to look up non-existing priority group name [%s].", priorityGroupName.getString()));
		return cms_defaultPriority;
	}
}

// ----------------------------------------------------------------------

int AnimationPriorityMap::getPriorityGroupCount()
{
	DEBUG_FATAL(!ms_installed, ("AnimationPriorityMap not installed."));
	return static_cast<int>(ms_priorityGroupMap->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &AnimationPriorityMap::getPriorityGroupName(int index)
{
	DEBUG_FATAL(!ms_installed, ("AnimationPriorityMap not installed."));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getPriorityGroupCount());

	// @todo use a sorted vector for efficiency.
	StringIntMap::iterator it = ms_priorityGroupMap->begin();
	std::advance(it, index);

	return it->first;
}

// ----------------------------------------------------------------------

int AnimationPriorityMap::getPriorityGroupPriority(int index)
{
	DEBUG_FATAL(!ms_installed, ("AnimationPriorityMap not installed."));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getPriorityGroupCount());

	// @todo use a sorted vector for efficiency.
	StringIntMap::iterator it = ms_priorityGroupMap->begin();
	std::advance(it, index);

	return it->second;
}

// ======================================================================
// class AnimationPriorityMap: private static member functions
// ======================================================================

void AnimationPriorityMap::remove()
{
	DEBUG_FATAL(!ms_installed, ("AnimationPriorityMap not installed."));

	delete ms_priorityGroupMap;
	ms_priorityGroupMap = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void AnimationPriorityMap::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
			
			char buffer[1024];

			while (iff.getChunkLengthLeft() > 0)
			{
				//-- Load an entry.
				iff.read_string(buffer, sizeof(buffer) - 1);
				const int priority = static_cast<int>(iff.read_int16());

				IGNORE_RETURN(ms_priorityGroupMap->insert(StringIntMap::value_type(CrcLowerString(buffer), priority)));
			}

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
