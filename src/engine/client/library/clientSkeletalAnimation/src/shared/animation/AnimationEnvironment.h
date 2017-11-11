// ======================================================================
//
// AnimationEnvironment.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationEnvironment_H
#define INCLUDED_AnimationEnvironment_H

// ======================================================================

class CrcLowerString;
class MemoryBlockManager;
class Object;
class SkeletalAppearance2;
class Vector;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================
/**
 * Contains lists of named variables by type.
 *
 * As planned the functionality is similar to a DynamicVariableList.
 * I chose not to use DynamicVariableList because:
 *   * I need Vector variables.  There is a DyanmicVariableList location
 *     type that contains a vector, but contains extra data I don't need.
 *   * DynamicVariableList is quite heavy.  It is geared for
 *     AutoDeltaVariable support, is way overkill for what I'm doing,
 *     and has a much larger dependency tree.
 *   * I may need to put more animation-specific functionality in the
 *     AnimationEnvironment and I want that freedom.
 */

class AnimationEnvironment
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	AnimationEnvironment();
	~AnimationEnvironment();

	void                  setSkeletalAppearance(SkeletalAppearance2 &skeletalAppearance);
	SkeletalAppearance2  *getSkeletalAppearance();
	SkeletalAppearance2 const *getSkeletalAppearance() const;

	Object const         *getOwnerObject() const;

	const Vector         &getConstVector(const CrcLowerString &name) const;
	Vector               &getVector(const CrcLowerString &name);

	const int            &getConstInt(const CrcLowerString &name) const;
	int                  &getInt(const CrcLowerString &name);

	const float          &getConstFloat(const CrcLowerString &name) const;
	float                &getFloat(const CrcLowerString &name);

	const CrcLowerString &getConstString(const CrcLowerString &name) const;
	CrcLowerString       &getString(const CrcLowerString &name);

	void                  setMostRecentVisibleGraphicsFrameNumber(int frameNumber);
	int                   getMostRecentVisibleGraphicsFrameNumber() const;

private:

	typedef stdmap<const CrcLowerString, CrcLowerString>::fwd  NamedStringMap;
	typedef stdmap<const CrcLowerString, Vector>::fwd          NamedVectorMap;
	typedef stdmap<const CrcLowerString, int>::fwd             NamedIntMap;
	typedef stdmap<const CrcLowerString, float>::fwd           NamedFloatMap;

private:

	AnimationEnvironment(const AnimationEnvironment&);
	AnimationEnvironment &operator =(const AnimationEnvironment&);

private:

	SkeletalAppearance2    *m_skeletalAppearance;
	mutable NamedStringMap *m_namedStrings;
	mutable NamedVectorMap *m_namedVectors;
	mutable NamedIntMap    *m_namedInts;
	mutable NamedFloatMap  *m_namedFloats;

	int                     m_mostRecentVisibleGraphicsFrameNumber;

};

// ======================================================================

inline void AnimationEnvironment::setSkeletalAppearance(SkeletalAppearance2 &skeletalAppearance)
{
	m_skeletalAppearance = &skeletalAppearance;
}

// ----------------------------------------------------------------------

inline SkeletalAppearance2 *AnimationEnvironment::getSkeletalAppearance()
{
	return m_skeletalAppearance;
}

// ----------------------------------------------------------------------

inline SkeletalAppearance2 const *AnimationEnvironment::getSkeletalAppearance() const
{
	return m_skeletalAppearance;
}

// ======================================================================

#endif
