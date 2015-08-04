// ======================================================================
//
// AnimationHeldItemMapper.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationHeldItemMapper.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

const Tag TAG_AHIM = TAG(A,H,I,M);

// ======================================================================

bool                                    AnimationHeldItemMapper::ms_installed;
AnimationHeldItemMapper::MapEntryVector AnimationHeldItemMapper::ms_mapEntryVector;

// ======================================================================

struct AnimationHeldItemMapper::MapEntry
{
public:

	MapEntry(const char *startAppearanceTemplateName, const char *animationStateName);

public:

	std::string           m_startAppearanceTemplateName;
	AnimationStateNameId  m_animationStateNameId;

private:

	// disabled
	MapEntry();

};

// ======================================================================

AnimationHeldItemMapper::MapEntry::MapEntry(const char *startAppearanceTemplateName, const char *animationStateName) :
	m_startAppearanceTemplateName(startAppearanceTemplateName),
	m_animationStateNameId(AnimationStateNameIdManager::createId(CrcLowerString(animationStateName)))
{
}

// ======================================================================

void AnimationHeldItemMapper::install(const char *heldItemMapperFileName)
{
	DEBUG_FATAL(ms_installed, ("AnimationHeldItemMapper already installed."));

	if (heldItemMapperFileName)
	{
		Iff  iff;

		const bool loadSuccess = iff.open(heldItemMapperFileName, true);
		if (loadSuccess)
			load(iff);
		else
			DEBUG_WARNING(true, ("AnimationHeldItemMapper failed to open held item mapper file [%s], items held in hand will not influence animation selection.", heldItemMapperFileName));
	}

	ms_installed = true;
	ExitChain::add(remove, "AnimationHeldItemMapper");
}

// ======================================================================

bool AnimationHeldItemMapper::mapHeldItem(const std::string &heldItemAppearanceTemplateName, AnimationStateNameId &heldItemStateId)
{
	DEBUG_FATAL(!ms_installed, ("AnimationHeldItemMapper not installed"));

	const MapEntryVector::iterator endIt = ms_mapEntryVector.end();
	for (MapEntryVector::iterator it = ms_mapEntryVector.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		MapEntry &entry = *(*it);

		const std::string::size_type  matchPosition = heldItemAppearanceTemplateName.find(entry.m_startAppearanceTemplateName);
		if (static_cast<int>(matchPosition) != static_cast<int>(std::string::npos))
		{
			//-- The entry's entire start string matched the some portion of the appearance template name.  Return the
			//   state for it.
			heldItemStateId = entry.m_animationStateNameId;

			return true;
		}
	}

	//-- Didn't find a mapping.
	return false;
}

// ======================================================================

void AnimationHeldItemMapper::remove()
{
	DEBUG_FATAL(!ms_installed, ("AnimationHeldItemMapper not installed"));

	std::for_each(ms_mapEntryVector.begin(), ms_mapEntryVector.end(), PointerDeleter());
	MapEntryVector().swap(ms_mapEntryVector);
}

// ======================================================================

void AnimationHeldItemMapper::load(Iff &iff)
{
	iff.enterForm(TAG_AHIM);

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
				WARNING_STRICT_FATAL(true, ("unsupported AnimationHeldItemMapper file format [%s].", buffer));

				iff.exitForm(TAG_AHIM, true);
			}
	}

	iff.exitForm(TAG_AHIM);
}

// ----------------------------------------------------------------------

void AnimationHeldItemMapper::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);
			
			char appearanceNamePrefix[MAX_PATH];
			char animationStateName[MAX_PATH];

			while (iff.getChunkLengthLeft() > 0)
			{
				// Read mapping entry.
				iff.read_string(appearanceNamePrefix, sizeof(appearanceNamePrefix) - 1);
				iff.read_string(animationStateName, sizeof(animationStateName) - 1);

				// If both items are not empty strings, create a mapping entry.
				if ((appearanceNamePrefix[0] != '\0') && (animationStateName[0] != '\0'))
					ms_mapEntryVector.push_back(new MapEntry(appearanceNamePrefix, animationStateName));
			}

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
