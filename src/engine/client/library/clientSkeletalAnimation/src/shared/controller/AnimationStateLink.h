// ======================================================================
//
// AnimationStateLink.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationStateLink_H
#define INCLUDED_AnimationStateLink_H

// ======================================================================

class AnimationStatePath;
class CrcLowerString;

// ======================================================================

class AnimationStateLink
{
public:

	typedef stdset<CrcLowerString>::fwd  CrcLowerStringSet;

public:

	virtual ~AnimationStateLink();

	/**
	 * Retrieve the AnimationStatePath of the destination state for this link.
	 *
	 * @return  the AnimationStatePath of the destination state for this link.
	 */
	virtual const AnimationStatePath &getDestinationPath() const = 0;

	/**
	 * Retrieve whether this link has an explicit transition animation that should be
	 * played when the link is crossed.
	 *
	 * @return  true if the link has an explicit transition animation to play
	 *          when traversed; false if not, in which case normal state-to-state
	 *          blending can be used.
	 *
	 * @see getTransitionLogicalAnimationIndex()
	 */
	virtual bool hasTransitionLogicalAnimation() const = 0;

	/**
	 * Retrieve the index for the transition logical animation.
	 *
	 * Note this function should be called only if hasTransitionLogicalAnimation()
	 * returns true.
	 *
	 * @return  the index for the transition logical animation.  The index 
	 *          is used to lookup the SkeletalAnimationTemplate within the
	 *          LogicalAnimationTable associated with the Appearance.
	 *
	 * @see  hasTransitionLogicalAnimation()
	 */
	virtual const CrcLowerString &getTransitionLogicalAnimationName() const = 0;

	virtual void                  addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const = 0;

protected:

	AnimationStateLink();

private:

	// disabled
	AnimationStateLink(const AnimationStateLink&);
	AnimationStateLink &operator =(const AnimationStateLink&);

};

// ======================================================================

inline AnimationStateLink::AnimationStateLink()
{
}

// ======================================================================

#endif
