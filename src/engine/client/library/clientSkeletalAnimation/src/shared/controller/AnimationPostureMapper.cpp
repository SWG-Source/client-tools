// ======================================================================
//
// AnimationPostureMapper.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationPostureMapper.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <vector>

// ======================================================================

const Tag TAG_ANPM = TAG(A,N,P,M);

// ======================================================================

bool                                    AnimationPostureMapper::ms_installed;
AnimationPostureMapper::MapEntryVector  AnimationPostureMapper::ms_mapEntryVector;

// ======================================================================

class AnimationPostureMapper::MapEntry
{
public:

	MapEntry();
	MapEntry(bool hasAssociatedState, const AnimationStateNameId &animationStateNameId);

	bool                        hasAssociatedState() const;
	const AnimationStateNameId &getStateNameId() const;

private:

	// disabled
	//MapEntry(const MapEntry&);             //lint -esym(754, MapEntry::MapEntry)  // defensive hiding
	//MapEntry &operator =(const MapEntry&); //lint -esym(754, MapEntry::operator=) // defensive hiding

private:

	bool                  m_hasAssociatedState;
	AnimationStateNameId  m_animationStateNameId;

};

// ======================================================================

AnimationPostureMapper::MapEntry::MapEntry() :
	m_hasAssociatedState(false),
	m_animationStateNameId()
{
}

// ----------------------------------------------------------------------

AnimationPostureMapper::MapEntry::MapEntry(bool newHasAssociatedState, const AnimationStateNameId &animationStateNameId) :
	m_hasAssociatedState(newHasAssociatedState),
	m_animationStateNameId(animationStateNameId)
{
}

// ----------------------------------------------------------------------

inline bool AnimationPostureMapper::MapEntry::hasAssociatedState() const
{
	return m_hasAssociatedState;
}

// ----------------------------------------------------------------------

inline const AnimationStateNameId &AnimationPostureMapper::MapEntry::getStateNameId() const
{
	return m_animationStateNameId;
}

// ======================================================================

void AnimationPostureMapper::install(const char *postureMapperFile)
{
	DEBUG_FATAL(ms_installed, ("AnimationPostureMapper already installed."));

	if (postureMapperFile)
	{
		Iff  iff;

		const bool loadSuccess = iff.open(postureMapperFile, true);
		if (loadSuccess)
			load(iff);
		else
			DEBUG_WARNING(true, ("AnimationPostureMapper failed to open posture mapper file [%s], game posture will not influence animation posture.", postureMapperFile));
	}

	ms_installed = true;
	ExitChain::add(remove, "AnimationPostureMapper");
}

// ----------------------------------------------------------------------

bool AnimationPostureMapper::mapPosture(int postureEnumValue, AnimationStateNameId &postureStateId)
{
	DEBUG_FATAL(!ms_installed, ("AnimationPostureMapper not installed."));

	//-- Handle unknown postures.
	if ((postureEnumValue < 0) || (postureEnumValue >= static_cast<int>(ms_mapEntryVector.size())))
	{
		// Unknown posture.  Indicate there is no associated state with this (i.e. we're standing).
		DEBUG_WARNING(true, ("posture [enum val=%d] specified, animation system doesn't have mapped.", postureEnumValue));
		return false;
	}

	//-- Get map entry for specified posture.
	// VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, postureEnumValue, static_cast<int>(ms_mapEntryVector.size()));
	MapEntry &mapEntry = ms_mapEntryVector[static_cast<MapEntryVector::size_type>(postureEnumValue)];

	const bool hasAssociatedState = mapEntry.hasAssociatedState();
	if (hasAssociatedState)
		postureStateId = mapEntry.getStateNameId();

	return hasAssociatedState;
}

// ======================================================================

void AnimationPostureMapper::remove()
{
	//-- Clear out the old map entry vector.
	MapEntryVector().swap(ms_mapEntryVector);
}

// ----------------------------------------------------------------------

void AnimationPostureMapper::load(Iff &iff)
{
	iff.enterForm(TAG_ANPM);

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
					DEBUG_WARNING(true, ("unsupported AnimationPostureMapper file version [%s], animation postures will be out of whack.", buffer));

					iff.exitForm(true);
					return;
				}
		}

	iff.exitForm(TAG_ANPM);
}

// ----------------------------------------------------------------------

void AnimationPostureMapper::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_INFO);

			//-- Load # game postures.
			const int gamePostureCount = static_cast<int>(iff.read_int16());

			//-- Size posture lookup array.  Default constructor is fine for
			//   any game posture that maps to no animation state (i.e. standing).
			ms_mapEntryVector.resize(static_cast<MapEntryVector::size_type>(gamePostureCount));

			//-- Load all entries.
			char buffer[1024];

			while (iff.getChunkLengthLeft())
			{
				const int gamePostureIndex = static_cast<int>(iff.read_int16());
				iff.read_string(buffer, sizeof(buffer) - 1);

				if (strlen(buffer) > 0)
				{
					// Set mapper entry.
					ms_mapEntryVector[static_cast<MapEntryVector::size_type>(gamePostureIndex)] = MapEntry(true, AnimationStateNameIdManager::createId(CrcLowerString(buffer)));
				}
			}

		iff.exitChunk(TAG_INFO);
	iff.exitForm(TAG_0000);
}

// ======================================================================
