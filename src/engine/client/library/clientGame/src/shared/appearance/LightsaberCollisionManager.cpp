// ======================================================================
//
// LightsaberCollisionManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/LightsaberCollisionManager.h"

#include "clientGame/ClientEventManager.h"
#include "clientGame/LightsaberAppearance.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Segment3d.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"

#include <unordered_map>
#include <limits>
#include <string>
#include <vector>

// ======================================================================

namespace LightsaberCollisionManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class BladeInfo
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	public:

		static void install();
		static void remove();

	public:

		BladeInfo(LightsaberAppearance const &appearance, int bladeIndex);

		Object const               *getObject() const;
		LightsaberAppearance const *getLightsaberAppearance() const;
		int                         getBladeIndex() const;

		void                        alter();

		Segment3d const            &getMostRecentBladeSegment() const;
		Segment3d const            &getLeastRecentBladeSegment() const;

	private:

		BladeInfo();

	private:

		ConstWatcher<Object>        m_lightsaberWatcher;
		LightsaberAppearance const *m_appearance;
		int                         m_bladeIndex;

		Segment3d                   m_segment1;
		Segment3d                   m_segment2;
		bool                        m_segment2IsMostRecent;

	};

	typedef std::vector<BladeInfo*>  BladeInfoVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void                      remove();

	BladeInfoVector::iterator findBladeInfo(LightsaberAppearance const &appearance, int bladeIndex);
	void                      getMinMax4(float a, float b, float c, float d, float &min, float &max);

	bool                      testBladeIntersection(float elapsedTime, Segment3d const &saberAStart, Segment3d const &saberAEnd, Segment3d const &saberBStart, Segment3d const &saberBEnd);
	bool                      testForDimensionOverlap(float blade1_start1, float blade1_end1, float blade1_start2, float blade1_end2, float blade2_start1, float blade2_end1, float blade2_start2, float blade2_end2);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const CrcLowerString &getClashName();

	ConstCharCrcLowerString const cs_clashLightEventName("clash_light");
	ConstCharCrcLowerString const cs_clashMediumEventName("clash_medium");
	ConstCharCrcLowerString const cs_clashHeavyEventName("clash_heavy");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool             s_installed;
	BladeInfoVector  s_blades;

	typedef std::unordered_map<NetworkId, float, NetworkId::Hash> ClashList;
	ClashList s_clashList;
	const float s_clashTimeMin = 0.2f;
	const float s_clashTimeMax = 0.5f;
}

using namespace LightsaberCollisionManagerNamespace;

// ======================================================================
// namespace LightsaberCollisionManagerNamespace
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(LightsaberCollisionManagerNamespace::BladeInfo, true, 0, 0, 0);

// ======================================================================

void LightsaberCollisionManagerNamespace::BladeInfo::install()
{
	InstallTimer const installTimer("LightsaberCollisionManagerNamespace::BladeInfo");

	installMemoryBlockManager();
}

// ----------------------------------------------------------------------

void LightsaberCollisionManagerNamespace::BladeInfo::remove()
{
	removeMemoryBlockManager();
}

// ======================================================================

LightsaberCollisionManagerNamespace::BladeInfo::BladeInfo(LightsaberAppearance const &appearance, int bladeIndex) :
	m_lightsaberWatcher(appearance.getOwner()),
	m_appearance(&appearance),
	m_bladeIndex(bladeIndex),
	m_segment1(Vector::zero, Vector::zero),
	m_segment2(Vector::zero, Vector::zero),
	m_segment2IsMostRecent(true)
{
	//-- Initialize both segments (old and new) with current blade info.
	appearance.getBladeSegment(bladeIndex, m_segment1);
	m_segment2 = m_segment1;
}

// ----------------------------------------------------------------------

Object const *LightsaberCollisionManagerNamespace::BladeInfo::getObject() const
{
	return m_lightsaberWatcher.getPointer();
}

// ----------------------------------------------------------------------

LightsaberAppearance const *LightsaberCollisionManagerNamespace::BladeInfo::getLightsaberAppearance() const
{
	return m_appearance;
}

// ----------------------------------------------------------------------

int LightsaberCollisionManagerNamespace::BladeInfo::getBladeIndex() const
{
	return m_bladeIndex;
}

// ----------------------------------------------------------------------

void LightsaberCollisionManagerNamespace::BladeInfo::alter()
{
	//-- Flip where we write the most recent segment info.
	m_segment2IsMostRecent = !m_segment2IsMostRecent;

	//-- Retrieve the most recent segment info.
	Segment3d &writeSegment = (m_segment2IsMostRecent ? m_segment2 : m_segment1);
	m_appearance->getBladeSegment(m_bladeIndex, writeSegment);
}

// ----------------------------------------------------------------------

Segment3d const &LightsaberCollisionManagerNamespace::BladeInfo::getMostRecentBladeSegment() const
{
	return (m_segment2IsMostRecent ? m_segment2 : m_segment1);
}

// ----------------------------------------------------------------------

Segment3d const &LightsaberCollisionManagerNamespace::BladeInfo::getLeastRecentBladeSegment() const
{
	return (m_segment2IsMostRecent ? m_segment1 : m_segment2);
}

// ======================================================================

void LightsaberCollisionManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("LightsaberCollisionManager not installed."));
	s_installed = false;

	BladeInfoVector::iterator const endIt = s_blades.end();
	for (BladeInfoVector::iterator it = s_blades.begin(); it != endIt; ++it)
	{
		BladeInfo *bladeInfo = *it;
		if (bladeInfo)
		{
			delete bladeInfo;
			*it = NULL;
		}
	}

	BladeInfoVector().swap(s_blades);

	BladeInfo::remove();
}

// ----------------------------------------------------------------------

LightsaberCollisionManagerNamespace::BladeInfoVector::iterator LightsaberCollisionManagerNamespace::findBladeInfo(LightsaberAppearance const &appearance, int bladeIndex)
{
	//-- Search list for blade.
	BladeInfoVector::iterator const endIt = s_blades.end();
	for (BladeInfoVector::iterator it = s_blades.begin(); it != endIt; ++it)
	{
		BladeInfo *bladeInfo = *it;
		NOT_NULL(bladeInfo);

		// Check if lightsaber still exists.
		if ((bladeInfo->getObject() != NULL) && (bladeInfo->getLightsaberAppearance() == &appearance) && (bladeInfo->getBladeIndex() == bladeIndex))
			return it;
	}

	//-- Not found.
	return endIt;
}

// ----------------------------------------------------------------------

void LightsaberCollisionManagerNamespace::getMinMax4(float a, float b, float c, float d, float &min, float &max)
{
	max = -std::numeric_limits<float>::max();
	min =  std::numeric_limits<float>::max();

	if (a > max)
		max = a;
	if (a < min)
		min = a;

	if (b > max)
		max = b;
	else if (b < min)
		min = b;

	if (c > max)
		max = c;
	else if (c < min)
		min = c;

	if (d > max)
		max = d;
	else if (d < min)
		min = d;
}

// ----------------------------------------------------------------------

bool LightsaberCollisionManagerNamespace::testForDimensionOverlap(
		float blade1_start1, float blade1_end1, float blade1_start2, float blade1_end2,
		float blade2_start1, float blade2_end1, float blade2_start2, float blade2_end2)
{
	//-- Get the intervals for the coordinate.
	float blade1Min;
	float blade1Max;

	getMinMax4(blade1_start1, blade1_end1, blade1_start2, blade1_end2, blade1Min, blade1Max);

	float blade2Min;
	float blade2Max;

	getMinMax4(blade2_start1, blade2_end1, blade2_start2, blade2_end2, blade2Min, blade2Max);

	//-- Check if there's any overlap of the intervals.
	if ((blade1Max < blade2Min) || (blade1Min > blade2Max))
	{
		// They don't overlap at all.
		return false;
	}
	else
	{
		// The do overlap.
		return true;
	}
}

// ----------------------------------------------------------------------

bool LightsaberCollisionManagerNamespace::testBladeIntersection(float elapsedTime, Segment3d const &blade1Start, Segment3d const &blade1End, Segment3d const &blade2Start, Segment3d const &blade2End)
{
	UNREF(elapsedTime);

	//-- Break down into three one-dimensional problems looking for trivial rejection.
	// Check x.
	bool const overlapInX = testForDimensionOverlap(
			blade1Start.getBegin().x, blade1Start.getEnd().x, blade1End.getBegin().x, blade1End.getEnd().x,
			blade2Start.getBegin().x, blade2Start.getEnd().x, blade2End.getBegin().x, blade2End.getEnd().x);
	if (!overlapInX)
		return false;

	// Check y.
	bool const overlapInY = testForDimensionOverlap(
			blade1Start.getBegin().y, blade1Start.getEnd().y, blade1End.getBegin().y, blade1End.getEnd().y,
			blade2Start.getBegin().y, blade2Start.getEnd().y, blade2End.getBegin().y, blade2End.getEnd().y);
	if (!overlapInY)
		return false;

	// Check z.
	bool const overlapInZ = testForDimensionOverlap(
			blade1Start.getBegin().z, blade1Start.getEnd().z, blade1End.getBegin().z, blade1End.getEnd().z,
			blade2Start.getBegin().z, blade2Start.getEnd().z, blade2End.getBegin().z, blade2End.getEnd().z);
	if (!overlapInZ)
		return false;

	// There was overlap in x, y and z ignoring time.  Let's call this a collision for now and see if it's good enough.
	return true;
}

// ----------------------------------------------------------------------

const CrcLowerString & LightsaberCollisionManagerNamespace::getClashName()
{
	float const percent = Random::randomReal();

	if (percent < 0.7f)
	{
		return cs_clashLightEventName;
	}
	else if (percent < 0.97f)
	{
		return cs_clashMediumEventName;
	}

	return cs_clashHeavyEventName;
}

// ======================================================================
// class LightsaberCollisionManager: PUBLIC STATIC
// ======================================================================

void LightsaberCollisionManager::install()
{
	InstallTimer const installTimer("LightsaberCollisionManager::install");

	DEBUG_FATAL(s_installed, ("LightsaberCollisionManager already installed."));

	BladeInfo::install();

	s_installed = true;
	ExitChain::add(LightsaberCollisionManagerNamespace::remove, "LightsaberCollisionManager");
}

// ----------------------------------------------------------------------

void LightsaberCollisionManager::alter(float elapsedTime)
{
	DEBUG_FATAL(!s_installed, ("LightsaberCollisionManager not installed."));

	//-- Remove deleted blades and update blade segment info for each blade.
	int bladeCount = static_cast<int>(s_blades.size());
	for (int k = 0; k < bladeCount; )
	{
		BladeInfo *const bladeInfo = s_blades[static_cast<BladeInfoVector::size_type>(k)];
		NOT_NULL(bladeInfo);

		if (bladeInfo->getObject() != NULL)
		{
			//-- Update blade position, move on to next blade.
			bladeInfo->alter();
			++k;
		}
		else
		{
			//-- This blade segment is dead, remove from list.
			if (k < bladeCount - 1)
			{
				// Move last vector entry into its place...
				s_blades[static_cast<BladeInfoVector::size_type>(k)] = s_blades[static_cast<BladeInfoVector::size_type>(bladeCount - 1)];
				s_blades.pop_back();

				// Don't increment k --- we've just moved the end of the array into the slot.
			}

			//-- Decrement blade count to account for removed blade.
			--bladeCount;
		}
	}

	//-- Test each lightsaber against every other lightsaber.  This is an N!
	//   test, so we don't want N growing too large.  If it does, we may need
	//   to add some kind of subdivision into the equation.
	for (int i = 0; i < bladeCount - 1; ++i)
	{
		BladeInfo const *bladeInfo1 = s_blades[static_cast<BladeInfoVector::size_type>(i)];
		NOT_NULL(bladeInfo1);
		if (!bladeInfo1->getObject())
			continue;

		for (int j = i + 1; j < bladeCount; ++j)
		{
			BladeInfo const *bladeInfo2 = s_blades[static_cast<BladeInfoVector::size_type>(j)];
			NOT_NULL(bladeInfo2);
			if (!bladeInfo2->getObject())
				continue;

			//-- Don't compare a multi-bladed lightsaber to itself.
			if (bladeInfo1->getLightsaberAppearance() == bladeInfo2->getLightsaberAppearance())
				continue;

			//-- Test blade 1 against blade 2.
			bool const isCollision = testBladeIntersection(elapsedTime, bladeInfo1->getLeastRecentBladeSegment(), bladeInfo1->getMostRecentBladeSegment(), bladeInfo2->getLeastRecentBladeSegment(), bladeInfo2->getMostRecentBladeSegment());
			if (isCollision)
			{
				Object const *const wielder1 = bladeInfo1->getLightsaberAppearance()->getWielderObject();

				//-- @todo fixup ClientEvent and ClientEffect to take hardpoints as CrcString rather than CrcLowerString for hardpoint names.
				// See if this fight already has an outstanding clash

				if (wielder1 != NULL)
				{
					ClashList::iterator iterClashList = s_clashList.find(wielder1->getNetworkId());

					if ((iterClashList == s_clashList.end()) || (iterClashList->second <= 0.0f))
					{
						//-- Send a clash event to each participating blade's wielder, put on the blade midpoint hardpoint.
						Object const *const lightsaberObject1 = bladeInfo1->getLightsaberAppearance()->getOwner();

						if (lightsaberObject1)
						{
							IGNORE_RETURN( ClientEventManager::playEvent(getClashName(), const_cast<Object*>(wielder1), lightsaberObject1, ConstCharCrcLowerString(LightsaberAppearance::getMidpointHardpointName(bladeInfo1->getBladeIndex()).getString())) );

							if (iterClashList == s_clashList.end())
							{
								s_clashList.insert(std::make_pair(wielder1->getNetworkId(), Random::randomReal(s_clashTimeMin, s_clashTimeMax)));
							}
							else
							{
								iterClashList->second = Random::randomReal(s_clashTimeMin, s_clashTimeMax);
							}
						}
					}
				}

				Object const *const wielder2 = bladeInfo2->getLightsaberAppearance()->getWielderObject();

				if (wielder2 != NULL)
				{
					ClashList::iterator iterClashList = s_clashList.find(wielder2->getNetworkId());

					if ((iterClashList == s_clashList.end()) || (iterClashList->second <= 0.0f))
					{
						Object const *const lightsaberObject2 = bladeInfo2->getLightsaberAppearance()->getOwner();

						if (lightsaberObject2)
						{
							IGNORE_RETURN( ClientEventManager::playEvent(getClashName(), const_cast<Object*>(wielder2), lightsaberObject2, ConstCharCrcLowerString(LightsaberAppearance::getMidpointHardpointName(bladeInfo2->getBladeIndex()).getString())) );

							if (iterClashList == s_clashList.end())
							{
								s_clashList.insert(std::make_pair(wielder2->getNetworkId(), Random::randomReal(s_clashTimeMin, s_clashTimeMax)));
							}
							else
							{
								iterClashList->second = Random::randomReal(s_clashTimeMin, s_clashTimeMax);
							}
						}
					}
				}
			}
		}
	}

	{
		// Age the clashes

		ClashList::iterator iterClashList = s_clashList.begin();

		for (; iterClashList != s_clashList.end(); ++iterClashList)
		{
			iterClashList->second -= elapsedTime;
		}
	}

	{
		// Clean out the clash list

		ClashList::iterator iterClashList = s_clashList.begin();

		for (; iterClashList != s_clashList.end(); ++iterClashList)
		{
			if (iterClashList->second <= -60.0f)
			{
				s_clashList.erase(iterClashList);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void LightsaberCollisionManager::addLightsaberBlade(LightsaberAppearance const &appearance, int bladeIndex)
{
	DEBUG_FATAL(!s_installed, ("LightsaberCollisionManager not installed."));

	//-- Check if we're already tracking this blade.
	BladeInfoVector::iterator it = findBladeInfo(appearance, bladeIndex);
	if ((it != s_blades.end()) && *it)
	{
		//-- We're already tracking this blade.
		DEBUG_WARNING(true, ("LightsaberCollisionManager::addLightsaberBlade(): tried to add lightsaber object [%s], blade [%d] when already tracked.", (*it)->getObject() ? (*it)->getObject()->getNetworkId().getValueString().c_str() : "<null object>", bladeIndex));
		return;
	}

	//-- Add blade to list.
	s_blades.push_back(new BladeInfo(appearance, bladeIndex));
}

// ----------------------------------------------------------------------

void LightsaberCollisionManager::removeLightsaberBlade(LightsaberAppearance const &appearance, int bladeIndex)
{
	DEBUG_FATAL(!s_installed, ("LightsaberCollisionManager not installed."));

	//-- Check if we're already tracking this blade.
	BladeInfoVector::iterator it = findBladeInfo(appearance, bladeIndex);
	if ((it == s_blades.end()) || !*it)
		return;

	//-- Dispose of blade info.
	delete *it;
	IGNORE_RETURN(s_blades.erase(it));
}

// ======================================================================
