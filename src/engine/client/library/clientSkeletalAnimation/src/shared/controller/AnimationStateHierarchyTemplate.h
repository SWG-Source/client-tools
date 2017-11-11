// ======================================================================
//
// AnimationStateHierarchyTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationStateHierarchyTemplate_H
#define INCLUDED_AnimationStateHierarchyTemplate_H

// ======================================================================

class AnimationActionGroup;
class AnimationState;
class AnimationStatePath;
class CrcLowerString;
class CrcString;

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"

// ======================================================================
/**
 * An interface for accessing animation state hierarchy data.
 *
 * There are two known implementations of this class.
 * BasicAnimationStateHierarchyTemplate is what the engine runtime
 * uses within a game.  EditableAnimationStateHierarchyTemplate is
 * used by the AnimationEditor and allows on-the-fly editing.
 */

class AnimationStateHierarchyTemplate
{
public:

	typedef stdset<CrcLowerString>::fwd  CrcLowerStringSet;

public:

	const CrcString &getName() const;

	void                  fetch() const;
	void                  release() const;
	int                   getReferenceCount() const;

	/**
	 * Retrieve the root AnimationState instance for this hierarchy.
	 *
	 * @return  the root AnimationState instance associated with the state hierarchy.
	 */
	virtual const AnimationState *getConstRootAnimationState() const = 0;

	/**
	 * Retrieve the number of AnimationActionGroup instances associated with
	 * this hierarchy.
	 *
	 * @return  the number of AnimationActionGroup instances associated with
	 *          this hierarchy.
	 */
	virtual int getAnimationActionGroupCount() const = 0;

	/**
	 * Retrieve the specified AnimationActionGroup instance associated with
	 * this hierarchy.
	 *
	 * @param index  a value within the range 0 <= index < getAnimationActionGroupCount().
	 *
	 * @return  the specified AnimationActionGroup instance associated with
	 *          this hierarchy.
	 */
	virtual const AnimationActionGroup &getConstAnimationActionGroup(int index) const = 0;

	/**
	 * Find the index of the animation action group associated with
	 * the given name.
	 *
	 * @param groupName  name of the AnimationActionGroup to lookup.
	 * @param index      the index of the specified AnimationActionGroup is
	 *                   returned in this parameter upon successful return.
	 *
	 * @return  true if the group was found and the index returned successfully;
	 *          false if an error occurred.
	 */
	virtual bool lookupAnimationActionGroupIndex(const CrcLowerString &groupName, int &index) const = 0;

	/**
	 * Retrieve the number of logical animations referenced by this hierarchy.
	 *
	 * @return  the number of logical animations referenced by this hierarchy.
	 *
	 * @deprecated this function will disappear when version 0001 and earlier
	 *             formats are no longer supported.
	 */
	virtual int getLogicalAnimationCount() const = 0;

	/**
	 * Retrieve the name of the specified logical animation referenced by this hierarchy.
	 *
	 * @param index  the logical animation name to lookup, must be in the
   *               range 0 <= index < getLogicalAnimationCount().
	 *
	 * @return  the name of the specified logical animation referenced by this hierarchy.
	 *
	 * @deprecated this function will disappear when version 0001 and earlier
	 *             formats are no longer supported.
	 */
	virtual const CrcLowerString &getLogicalAnimationName(int index) const = 0;

	/**
	 * Find the index of the logical animation with the given animation name.
	 *
	 * @param animationName  the name of the logical animation for which the
	 *                       index will be retrieved.
	 * @param index          the index of the specified logical animation will
	 *                       be returned in this parameter if the function returns
	 *                       successfully.
	 *
	 * @return  true if the animationName was found and the index returned successfully;
	 *          false otherwise.
	 *
	 * @deprecated this function will disappear when version 0001 and earlier
	 *             formats are no longer supported.
	 */
	virtual bool lookupLogicalAnimationIndex(const CrcLowerString &animationName, int &index) const = 0;

	virtual void          addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const = 0;

	const AnimationState *getConstAnimationState(const AnimationStatePath &path) const;
	const AnimationState *getConstAnimationState(const AnimationStatePath &path, int partialPathLength) const;

protected:

	AnimationStateHierarchyTemplate(const CrcString &name);
	virtual ~AnimationStateHierarchyTemplate();

private:

	// disabled
	AnimationStateHierarchyTemplate();
	AnimationStateHierarchyTemplate(const AnimationStateHierarchyTemplate&);
	AnimationStateHierarchyTemplate &operator =(const AnimationStateHierarchyTemplate&);

private:

	PersistentCrcString const  m_name;
	mutable int                m_referenceCount;

};

// ======================================================================

inline const CrcString &AnimationStateHierarchyTemplate::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline void AnimationStateHierarchyTemplate::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

inline int AnimationStateHierarchyTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ======================================================================

#endif
