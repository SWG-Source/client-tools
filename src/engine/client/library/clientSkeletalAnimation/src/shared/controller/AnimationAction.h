// ======================================================================
//
// AnimationAction.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationAction_H
#define INCLUDED_AnimationAction_H

// ======================================================================

class AnimationEnvironment;
class CrcLowerString;

// ======================================================================

class AnimationAction
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessNameCrcOrderPointerComparator
	{
		bool operator ()(const AnimationAction *lhs, const AnimationAction *rhs) const;
		bool operator ()(const CrcLowerString &lhs, const AnimationAction *rhs) const;
		bool operator ()(const AnimationAction *lhs, const CrcLowerString &rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessNameAbcOrderPointerComparator
	{
		bool operator ()(const AnimationAction *lhs, const AnimationAction *rhs) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdset<CrcLowerString>::fwd  CrcLowerStringSet;

public:

	virtual ~AnimationAction();
	
	/**
	 * Retrieve the name of the action.
	 *
	 * @return  the name of the action.
	 */
	virtual const CrcLowerString &getName() const = 0;

	/**
	 * Retrieve the logical animation name to be played when
	 * this action is triggered.
	 *
	 * @return  the logical animation name to be played when
	 *          this action is triggered.
	 */
	virtual const CrcLowerString &getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const = 0;

	/**
	 * Indicate whether playing this action should force the
	 * visuals to hide any hand-held items.
	 *
	 * @return  true if the visual playback system should temporarily
	 *          hide any handheld items while this animation plays;
	 *          false otherwise.
	 */
	virtual bool shouldHideHeldItem(const AnimationEnvironment &animationEnvironment) const = 0;

	/**
	 * Retrieve whether the animation associated with the action
	 * should be applied as an add animation (on the add track) or as
	 * a priority-based replacement animation (on the play-once track).
	 *
	 * @return  if true, the animation should be played on an add track;
	 *          otherwise, the animation should be played on a priority-based replacement
	 *          track.
	 */
	virtual bool shouldApplyAnimationAsAdd(const AnimationEnvironment &animationEnvironment) const = 0;

	// virtual bool shouldHideHeldItem(const AnimationStateNameId &heldItemClass) const = 0;

	virtual void addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const = 0;

protected:

	AnimationAction();
	
private:

	// disabled
	AnimationAction(const AnimationAction &);
	AnimationAction &operator =(const AnimationAction&);
	
};

// ======================================================================

#endif
