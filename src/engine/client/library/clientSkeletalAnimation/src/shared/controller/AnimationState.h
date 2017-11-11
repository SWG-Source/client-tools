// ======================================================================
//
// AnimationState.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationState_H
#define INCLUDED_AnimationState_H

// ======================================================================

class AnimationAction;

class AnimationStateLink;
class AnimationStateNameId;
class AnimationStatePath;
class CrcLowerString;
class CrcString;

// ======================================================================
/**
 * An interface (pure virtual class) for accessing information about
 * the hierarchy node.
 *
 * There are two known implementations: BasicAnimationState used by the
 * game runtime and EditableAnimationState used by AnimationEditor.
 */

class AnimationState
{
public:

	typedef stdset<CrcLowerString>::fwd  CrcLowerStringSet;

public:

	AnimationState();
	virtual ~AnimationState();

	/**
	 * Retrieve the full path of this state.
	 *
	 * @return  an AnimationStatePath instance for the full path
	 *          to this state.
	 */
	virtual const AnimationStatePath &getStatePath() const = 0;

	/**
	 * Retrieve the state name for this state.
	 *
	 * The state name is the short name for the node.  
	 * E.g. The state "std.rifle.standing" has a StateNameId
	 *      for the name "standing".  The AnimationStatePath
	 *      for the same name points to an ordered list of
	 *      AnimationStateNameId entries "std", "rifle", "standing".
	 *
	 * @return  the state name for this state.
	 */
	virtual const AnimationStateNameId getStateNameId() const = 0;

	/**
	 * Retrieve the parent state of this state.
	 *
	 * @return  NULL if this state doesn't have a parent; otherwise
	 *          it returns the AnimationState instance for the parent
	 *          of this state.
	 */
	virtual const AnimationState *getConstParentState() const = 0;

	/**
	 * Retrieve the number of child states this state has.
	 *
	 * @return  the number of child states this state has.
	 */
	virtual int getChildStateCount() const = 0;

	/**
	 * Retrieve the AnimationState instance for the child state.
	 *
	 * @param index  uniquely specifies the child state, must be in the
	 *               range 0 <= index < getChildStateCount().
	 *
	 * @return  the AnimationState instance for the child state.
	 */
	virtual const AnimationState &getConstChildState(int index) const = 0;

	/**
	 * Find the index of the child state that corresponds to a
	 * given state name.
	 *
	 * @param childNameId  the name of the child state whose index should
	 *                     be found.
	 * @param index        will be filled with the index of the child state
	 *                     that matches the given name if the function
	 *                     returns successfully.
	 *
	 * @return  true upon successful completion; false otherwise.
	 */
	virtual bool lookupChildStateIndex(const AnimationStateNameId &childNameId, int &index) const = 0;

	/**
	 * Retrieve the logical animation name for the loop animation pertaining to this
	 * state.
	 *
	 * @return  the logical animation name for the loop animation pertaining to this
	 *          state.
	 */
	virtual CrcString const &getLogicalAnimationName() const = 0;

	/**
	 * Retrieve the number of links associated with this state.
	 *
	 * @return  the number of links associated with this state.
	 */
	virtual int getLinkCount() const = 0;

	/**
	 * Retrieve the specified AnimationStateLink instance.
	 *
	 * @param index  uniquely identifies the state link data to retrieve.
	 *               Must be in range 0 <= index < getLinkCount().
	 *
	 * @return  the specified AnimationStateLink instance.
	 */
	virtual const AnimationStateLink &getConstLink(int index) const = 0;

	/**
	 * Retrieve the number of non-group actions defined for this state.
	 *
	 * @return  the number of non-group actions defined for this state.
	 */
	virtual int getActionCount() const = 0;

	/**
	 * Retrieve the specified non-group AnimationAction instance defined
	 * for this state.
	 *
	 * @param index  uniquely identifies the non-group AnimationAction
	 *               to retrieve.  Must be in range 0 <= index < getActionCount().
	 *
	 * @return  the specified non-group AnimationAction instance defined
	 *          for this state.
	 */
	virtual const AnimationAction &getConstAction(int index) const = 0;

	/**
	 * Retrieve the number of granted action groups associated with this state.
	 *
	 * @return  the number of granted action groups associated with this state.
	 */
	virtual int getGrantedActionGroupCount() const = 0;

	/**
	 * Retrieve the specified granted AnimationActionGroup index associated
	 * with this state.
	 *
	 * The AnimationmActionGroup index references the table of 
	 * AnimationActionGroup instances associated with the
	 * AnimationStateHierarchyTemplate of which this AnimationState
	 * is a part.
	 *
	 * @param index  uniquely identifies the granted AnimationActionGroup
	 *               to retrieve.  Must be in range 0 <= index < getGrantedActionGroupCount().
	 *
	 * @return  the specified granted AnimationActionGroup index.
	 */
	virtual int getGrantedActionGroupIndex(int index) const = 0;

	/**
	 * Retrieve the number of denied action groups associated with this state.
	 *
	 * @return  the number of denied action groups associated with this state.
	 */
	virtual int getDeniedActionGroupCount() const = 0;

	/**
	 * Retrieve the specified denied AnimationActionGroup index associated
	 * with this state.
	 *
	 * The AnimationmActionGroup index references the table of 
	 * AnimationActionGroup instances associated with the
	 * AnimationStateHierarchyTemplate of which this AnimationState
	 * is a part.
	 *
	 * @param index  uniquely identifies the denied AnimationActionGroup
	 *               to retrieve.  Must be in range 0 <= index < getDeniedActionGroupCount().
	 *
	 * @return  the specified denied AnimationActionGroup index.
	 */
	virtual int getDeniedActionGroupIndex(int index) const = 0;

	virtual void              addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const = 0;

	const AnimationState     *getConstChildState(const AnimationStateNameId &childStateId) const;
	const AnimationStateLink *getConstLink(const AnimationStatePath &path, int pathLengthToConsider) const;
	const AnimationAction    *getConstActionByName(const CrcLowerString &actionName) const;

private:

	// disabled
	AnimationState(const AnimationState&);
	AnimationState &operator =(const AnimationState&);

};

// ======================================================================

#endif
