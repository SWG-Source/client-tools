// ======================================================================
//
// EditableMovementAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EditableMovementAnimationAction_H
#define INCLUDED_EditableMovementAnimationAction_H

// ======================================================================

#include "clientSkeletalAnimation/EditableAnimationAction.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class EditableAnimationStateHierarchyTemplate;
class EditableBasicAnimationAction;
class Iff;
class MemoryBlockManager;
class XmlTreeNode;

// ======================================================================

class EditableMovementAnimationAction: public EditableAnimationAction
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	EditableMovementAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber);
	EditableMovementAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber);
	EditableMovementAnimationAction(const CrcLowerString &actionName, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	virtual ~EditableMovementAnimationAction();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Inherited interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const CrcLowerString &getName() const;
	virtual const CrcLowerString &getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const;
	virtual bool                  shouldHideHeldItem(const AnimationEnvironment &animationEnvironment) const;
	virtual bool                  shouldApplyAnimationAsAdd(const AnimationEnvironment &animationEnvironment) const;
	virtual void                  addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const;

	virtual void                  setName(const CrcLowerString &actionName);

	virtual void                  write(Iff &iff) const;
	virtual void                  writeXml(IndentedFileWriter &writer) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// New functionality interface.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	EditableBasicAnimationAction &getZeroSpeedAction();
	EditableBasicAnimationAction &getSpeedAction();

	EditableBasicAnimationAction const &getZeroSpeedAction() const;
	EditableBasicAnimationAction const &getSpeedAction() const;

private:

	static void                   remove();
	static bool                   isMoving(const AnimationEnvironment &animationEnvironment);

private:

	void                          load_0001(Iff &iff, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	void                          load_0002(Iff &iff, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);

	void                          loadXml_1(XmlTreeNode const &treeNode, EditableAnimationStateHierarchyTemplate &hierarchyTemplate);

	// Disabled.
	EditableMovementAnimationAction();
	EditableMovementAnimationAction(const EditableMovementAnimationAction&);
	EditableMovementAnimationAction &operator =(const EditableMovementAnimationAction&);

private:

	/// Animation action used when character is moving at zero speed (i.e. is idle).
	EditableBasicAnimationAction            *m_zeroSpeedAction;

	/// Animation action used when character is moving at some non-zero (positive) speed.
	EditableBasicAnimationAction            *m_speedAction;

};

// ======================================================================

inline EditableBasicAnimationAction &EditableMovementAnimationAction::getZeroSpeedAction()
{
	NOT_NULL(m_zeroSpeedAction);
	return *m_zeroSpeedAction;
}

// ----------------------------------------------------------------------

inline EditableBasicAnimationAction &EditableMovementAnimationAction::getSpeedAction()
{
	NOT_NULL(m_speedAction);
	return *m_speedAction;
}

// ----------------------------------------------------------------------

inline EditableBasicAnimationAction const &EditableMovementAnimationAction::getZeroSpeedAction() const
{
	NOT_NULL(m_zeroSpeedAction);
	return *m_zeroSpeedAction;
}

// ----------------------------------------------------------------------

inline EditableBasicAnimationAction const &EditableMovementAnimationAction::getSpeedAction() const
{
	NOT_NULL(m_speedAction);
	return *m_speedAction;
}

// ======================================================================

#endif
