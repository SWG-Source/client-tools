// ======================================================================
//
// EditableAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EditableAnimationAction_H
#define INCLUDED_EditableAnimationAction_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationAction.h"

class EditableAnimationStateHierarchyTemplate;
class Iff;
class IndentedFileWriter;

// ======================================================================

class EditableAnimationAction: public AnimationAction
{
public:

	EditableAnimationAction();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const CrcLowerString &getName() const = 0;
	virtual const CrcLowerString &getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const = 0;
	virtual bool                  shouldHideHeldItem(const AnimationEnvironment &animationEnvironment) const = 0;
	virtual bool                  shouldApplyAnimationAsAdd(const AnimationEnvironment &animationEnvironment) const = 0;
	virtual void                  addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual void                  setName(const CrcLowerString &actionName) = 0;
	virtual void                  writeXml(IndentedFileWriter &writer) const = 0;

	virtual void                  write(Iff &iff) const = 0;

private:

	// Disabled.
	EditableAnimationAction(const EditableAnimationAction&);
	EditableAnimationAction &operator =(const EditableAnimationAction&);

};

// ======================================================================

inline EditableAnimationAction::EditableAnimationAction()
{
};

// ======================================================================

#endif
