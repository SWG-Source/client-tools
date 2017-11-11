// ======================================================================
//
// AnimationEnvironment.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"

#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

#include <map>
#include <limits>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(AnimationEnvironment, true, 0, 0, 0);

// ======================================================================
// class AnimationEnvironment: public member functions
// ======================================================================

AnimationEnvironment::AnimationEnvironment() :
	m_skeletalAppearance(0),
	m_namedStrings(0),
	m_namedVectors(0),
	m_namedInts(0),
	m_namedFloats(0),
	m_mostRecentVisibleGraphicsFrameNumber(std::numeric_limits<int>::min())
{
}

// ----------------------------------------------------------------------

AnimationEnvironment::~AnimationEnvironment()
{
	delete m_namedFloats;
	m_namedFloats=0;

	delete m_namedVectors;
	m_namedVectors=0;

	delete m_namedInts;
	m_namedInts=0;

	delete m_namedStrings;
	m_namedStrings=0;

	m_skeletalAppearance = 0;
}

// ----------------------------------------------------------------------

Object const *AnimationEnvironment::getOwnerObject() const
{
	return (m_skeletalAppearance ? m_skeletalAppearance->getOwner() : NULL);
}

// ----------------------------------------------------------------------

const Vector &AnimationEnvironment::getConstVector(const CrcLowerString &name) const
{
	if (!m_namedVectors)
		m_namedVectors = new NamedVectorMap();

	NamedVectorMap::iterator lowerBoundResult = m_namedVectors->lower_bound(name);
	if ((lowerBoundResult != m_namedVectors->end()) && !m_namedVectors->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedVectorMap::iterator newIt = m_namedVectors->insert(lowerBoundResult, NamedVectorMap::value_type(name, Vector::zero));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

Vector &AnimationEnvironment::getVector(const CrcLowerString &name)
{
	if (!m_namedVectors)
		m_namedVectors = new NamedVectorMap();

	NamedVectorMap::iterator lowerBoundResult = m_namedVectors->lower_bound(name);
	if ((lowerBoundResult != m_namedVectors->end()) && !m_namedVectors->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedVectorMap::iterator newIt = m_namedVectors->insert(lowerBoundResult, NamedVectorMap::value_type(name, Vector::zero));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

const int &AnimationEnvironment::getConstInt(const CrcLowerString &name) const
{
	if (!m_namedInts)
		m_namedInts = new NamedIntMap();

	NamedIntMap::iterator lowerBoundResult = m_namedInts->lower_bound(name);
	if ((lowerBoundResult != m_namedInts->end()) && !m_namedInts->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedIntMap::iterator newIt = m_namedInts->insert(lowerBoundResult, NamedIntMap::value_type(name, 0));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

int &AnimationEnvironment::getInt(const CrcLowerString &name)
{
	if (!m_namedInts)
		m_namedInts = new NamedIntMap();

	NamedIntMap::iterator lowerBoundResult = m_namedInts->lower_bound(name);
	if ((lowerBoundResult != m_namedInts->end()) && !m_namedInts->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedIntMap::iterator newIt = m_namedInts->insert(lowerBoundResult, NamedIntMap::value_type(name, 0));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

const float &AnimationEnvironment::getConstFloat(const CrcLowerString &name) const
{
	if (!m_namedFloats)
		m_namedFloats = new NamedFloatMap();

	NamedFloatMap::iterator lowerBoundResult = m_namedFloats->lower_bound(name);
	if ((lowerBoundResult != m_namedFloats->end()) && !m_namedFloats->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedFloatMap::iterator newIt = m_namedFloats->insert(lowerBoundResult, NamedFloatMap::value_type(name, 0.0f));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

float &AnimationEnvironment::getFloat(const CrcLowerString &name)
{
	if (!m_namedFloats)
		m_namedFloats = new NamedFloatMap();

	NamedFloatMap::iterator lowerBoundResult = m_namedFloats->lower_bound(name);
	if ((lowerBoundResult != m_namedFloats->end()) && !m_namedFloats->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedFloatMap::iterator newIt = m_namedFloats->insert(lowerBoundResult, NamedFloatMap::value_type(name, 0.0f));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

const CrcLowerString &AnimationEnvironment::getConstString(const CrcLowerString &name) const
{
	if (!m_namedStrings)
		m_namedStrings = new NamedStringMap();

	NamedStringMap::iterator lowerBoundResult = m_namedStrings->lower_bound(name);
	if ((lowerBoundResult != m_namedStrings->end()) && !m_namedStrings->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedStringMap::iterator newIt = m_namedStrings->insert(lowerBoundResult, NamedStringMap::value_type(name, CrcLowerString::empty));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

CrcLowerString &AnimationEnvironment::getString(const CrcLowerString &name)
{
	if (!m_namedStrings)
		m_namedStrings = new NamedStringMap();

	NamedStringMap::iterator lowerBoundResult = m_namedStrings->lower_bound(name);
	if ((lowerBoundResult != m_namedStrings->end()) && !m_namedStrings->key_comp()(name, lowerBoundResult->first))
	{
		//-- Return existing entry.
		return lowerBoundResult->second;
	}
	else
	{
		//-- Entry doesn't exist, create it.
		NamedStringMap::iterator newIt = m_namedStrings->insert(lowerBoundResult, NamedStringMap::value_type(name, CrcLowerString::empty));
		return newIt->second;
	}
}

// ----------------------------------------------------------------------

void AnimationEnvironment::setMostRecentVisibleGraphicsFrameNumber(int frameNumber)
{
	m_mostRecentVisibleGraphicsFrameNumber = frameNumber;
}

// ----------------------------------------------------------------------

int AnimationEnvironment::getMostRecentVisibleGraphicsFrameNumber() const
{
	return m_mostRecentVisibleGraphicsFrameNumber;
}

// ======================================================================
