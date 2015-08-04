// ======================================================================
//
// AnimationStatePath.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"

#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <string>

// ======================================================================
// class AnimationStatePath: public static member functions
// ======================================================================
/**
 * Return the length of the path that is in common between the two
 * paths.
 *
 * @return  the length of the path that is in common between the two
 *          paths.  A return value of zero indicates that only the
 *          root of the hierarchy is common between the two states.
 */

int AnimationStatePath::getCommonPathLength(const AnimationStatePath &path1, const AnimationStatePath &path2)
{
	int commonLength = 0;

	const AnimationStateNameIdVector::size_type  length1 = path1.m_path.size();
	const AnimationStateNameIdVector::size_type  length2 = path2.m_path.size();

	//-- Compare only over the number of elements equal to the smaller of the paths.
	const AnimationStateNameIdVector::size_type  compareLength = std::min(length1, length2);

	for (AnimationStateNameIdVector::size_type i = 0; i < compareLength; ++i)
	{
		if (path1.m_path[i] == path2.m_path[i])
			++commonLength;
		else
			break;
	}

	return commonLength;
}

// ======================================================================
// class AnimationStatePath: public member functions
// ======================================================================

AnimationStatePath::AnimationStatePath() :
	m_path()
{
}

// ----------------------------------------------------------------------

void AnimationStatePath::clearPath()
{
	m_path.clear();
}

// ----------------------------------------------------------------------

void AnimationStatePath::appendState(const AnimationStateNameId &stateNameId)
{
	m_path.push_back(stateNameId);
}

// ----------------------------------------------------------------------

void AnimationStatePath::removeLastState()
{
	if (m_path.empty())
		WARNING(true, ("removeLastState() called on AnimationStatePath of zero length."));
	else
		IGNORE_RETURN(m_path.erase(m_path.end()));
}

// ----------------------------------------------------------------------

bool AnimationStatePath::hasState(const AnimationStateNameId &stateNameId) const
{
	int const pathLength = getPathLength();
	for (int i = 0; i < pathLength; ++i)
	{
		if (getState(i) == stateNameId)
		{
			// Found it.
			return true;
		}
	}

	// Did not find it.
	return false;
}

// ----------------------------------------------------------------------

int AnimationStatePath::getPathLength() const
{
	return static_cast<int>(m_path.size());
}

// ----------------------------------------------------------------------

const AnimationStateNameId &AnimationStatePath::getState(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getPathLength());
	return m_path[static_cast<AnimationStateNameIdVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void AnimationStatePath::copyTo(AnimationStatePath &destinationPath) const
{
	destinationPath.m_path = m_path;
} //lint !e1764 // destinationPath could be declared const ref // hmm, I'm modifying destinationPath.

// ----------------------------------------------------------------------

bool AnimationStatePath::operator ==(const AnimationStatePath &rhs) const
{
	return m_path == rhs.m_path;
}

// ----------------------------------------------------------------------

bool AnimationStatePath::isEqualTo(const AnimationStatePath &rhs, int rhsLength) const
{
	DEBUG_FATAL(rhsLength > static_cast<int>(rhs.m_path.size()), ("Caller passed rhsLength greater than the length of the path [%d/%u].", rhsLength, static_cast<int>(rhs.m_path.size())));

	//-- Trivially reject: if lengths are the same, they can't be equal.
	const AnimationStateNameIdVector::size_type lhsSize = m_path.size();
	if (static_cast<int>(lhsSize) != rhsLength)
		return false;

	//-- Check each element.  If there are any differences, the paths are not equal.
	for (AnimationStateNameIdVector::size_type i = 0; i < lhsSize; ++i)
	{
		if (m_path[i] != rhs.m_path[i])
			return false;
	}

	//-- The paths are the same.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a string in dot notation for the complete path.
 *
 * @return  the string in dot notation for the complete path.
 */

std::string AnimationStatePath::getPathString() const
{
	std::string  pathString;

	for (AnimationStateNameIdVector::size_type i = 0; i < m_path.size(); ++i)
	{
		if (i)
			pathString += '.';

		pathString += AnimationStateNameIdManager::getNameString(m_path[i]).getString();
	}

	return pathString;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a string in dot notation for a portion of the complete path.
 *
 * @param partialPathLength  the number of states to print.
 *
 * @return  the string in dot notation for a portion of the complete path.
 */

std::string AnimationStatePath::getPathString(int partialPathLength) const
{
	std::string  pathString;

	const AnimationStateNameIdVector::size_type  size = std::min(static_cast<AnimationStateNameIdVector::size_type>(partialPathLength), m_path.size());
	for (AnimationStateNameIdVector::size_type i = 0; i < size; ++i)
	{
		if (i)
			pathString += '.';

		pathString += AnimationStateNameIdManager::getNameString(m_path[i]).getString();
	}

	return pathString;
}

// ======================================================================
