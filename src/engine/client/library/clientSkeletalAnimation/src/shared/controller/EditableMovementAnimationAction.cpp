// ======================================================================
//
// EditableMovementAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"
#include "sharedXml/XmlTreeNode.h"

#include <cstdio>
#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(EditableMovementAnimationAction, true, 0, 0, 0);

// ======================================================================

namespace EditableMovementAnimationActionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool isObjectLevelDebuggingEnabled(AnimationEnvironment const &animationEnvironment);

	EditableBasicAnimationAction *newActionCore(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, char const *actionName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_MVAC = TAG(M,V,A,C);

	char const *const  cs_useAddTrackAttributeName    = "useAddTrack";
	char const *const  cs_actionAnimationElementName  = "actionAnimation";
	char const *const  cs_logicalNameAttributeName    = "logicalName";
	char const *const  cs_nameAttributeName           = "name";
	char const *const  cs_movementActionElementName   = "movementAction";
	char const *const  cs_noMovementActionElementName = "noMovementAction";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_logEnabled;

	std::string  s_name;
	std::string  s_logicalName;
}

using namespace EditableMovementAnimationActionNamespace;

// ======================================================================

EditableBasicAnimationAction *EditableMovementAnimationActionNamespace::newActionCore(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, char const *actionName)
{
	EditableBasicAnimationAction *const newAction = new EditableBasicAnimationAction(actionName ? CrcLowerString(actionName) : CrcLowerString::empty, hierarchyTemplate);

	//-- Get add track disposition.
	bool  useAddTrack = false;
	IGNORE_RETURN(treeNode.getElementAttributeAsBool(cs_useAddTrackAttributeName, useAddTrack, true));
	newAction->setShouldApplyAnimationAsAdd(useAddTrack);

	//-- Get the animation name.
	XmlTreeNode const actionAnimationNode(treeNode.getFirstChildElementNode());
	FATAL(actionAnimationNode.isNull() || _stricmp(actionAnimationNode.getName(), cs_actionAnimationElementName), ("expecting element [%s], found [%s].", cs_actionAnimationElementName, actionAnimationNode.getName()));

	actionAnimationNode.getElementAttributeAsString(cs_logicalNameAttributeName, s_logicalName);
	newAction->setLogicalAnimationName(CrcLowerString(s_logicalName.c_str()));

	return newAction;
}

// ======================================================================

bool EditableMovementAnimationActionNamespace::isObjectLevelDebuggingEnabled(AnimationEnvironment const &animationEnvironment)
{
	//-- Handle no debugging focus object.
	Object const *focusObject = SkeletalAnimationDebugging::getFocusObject();
	if (!focusObject)
	{
		// There is no focus object, so enable for all objects.
		return true;
	}

	//-- Check if object associated with controller is the focus object.
	SkeletalAppearance2 const *appearance = animationEnvironment.getSkeletalAppearance();
	if (!appearance)
	{
		// Not the focus object.
		return false;
	}

	// Object level debugging is enabled if the associated appearance's owner is the animation system's focus object.
	return (appearance->getOwner() == focusObject);
}

// ======================================================================

void EditableMovementAnimationAction::install()
{
	DebugFlags::registerFlag(s_logEnabled, "ClientSkeletalAnimation/Action", "logMovementAction");

	installMemoryBlockManager();
	ExitChain::add(remove, "EditableMovementAnimationAction");
}

// ======================================================================

EditableMovementAnimationAction::EditableMovementAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber) :
	EditableAnimationAction(),
	m_zeroSpeedAction(0),
	m_speedAction(0)
{
	//-- Load iff.
	switch (versionNumber)
	{
		case 1:
			load_0001(iff, hierarchyTemplate);
			break;

		case 2:
			load_0002(iff, hierarchyTemplate);
			break;

		default:
			FATAL(true, ("EditableMovementAnimationAction: unsupported version [%d].", versionNumber));
	}

	//-- Validate construction post conditions.
	NOT_NULL(m_zeroSpeedAction);
	NOT_NULL(m_speedAction);
}

// ----------------------------------------------------------------------

EditableMovementAnimationAction::EditableMovementAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber) :
	EditableAnimationAction(),
	m_zeroSpeedAction(0),
	m_speedAction(0)
{
	//-- Load iff.
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode, hierarchyTemplate);
			break;

		default:
			FATAL(true, ("EditableMovementAnimationAction: unsupported version [%d].", versionNumber));
	}

	//-- Validate construction post conditions.
	NOT_NULL(m_zeroSpeedAction);
	NOT_NULL(m_speedAction);
}

// ----------------------------------------------------------------------

EditableMovementAnimationAction::EditableMovementAnimationAction(const CrcLowerString &actionName, EditableAnimationStateHierarchyTemplate &hierarchyTemplate) :
	EditableAnimationAction(),
	m_zeroSpeedAction(new EditableBasicAnimationAction(actionName, hierarchyTemplate)),
	m_speedAction(new EditableBasicAnimationAction(CrcLowerString::empty, hierarchyTemplate))
{
}

// ----------------------------------------------------------------------

EditableMovementAnimationAction::~EditableMovementAnimationAction()
{
	delete m_speedAction;
	delete m_zeroSpeedAction;
}

// ======================================================================

const CrcLowerString &EditableMovementAnimationAction::getName() const
{
	return m_zeroSpeedAction->getName();
}

// ----------------------------------------------------------------------

const CrcLowerString &EditableMovementAnimationAction::getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const
{
#ifdef _DEBUG
	SkeletalAppearance2 const *const appearance = animationEnvironment.getSkeletalAppearance();
	Object const              *const owner      = (appearance ? appearance->getOwner() : 0);
#endif

	if (isMoving(animationEnvironment))
	{
		CrcLowerString const &result = m_speedAction->getLogicalAnimationName(animationEnvironment);
		DEBUG_REPORT_LOG(s_logEnabled && isObjectLevelDebuggingEnabled(animationEnvironment), ("object [%s]: movement action [%s] chose moving logical animation [%s] style [%s].\n", owner ? owner->getNetworkId().getValueString().c_str() : "<NULL>", getName().getString(), result.getString(), m_speedAction->shouldApplyAnimationAsAdd(animationEnvironment) ? "add" : "replace"));
		return result;
	}
	else
	{
		CrcLowerString const &result = m_zeroSpeedAction->getLogicalAnimationName(animationEnvironment);
		DEBUG_REPORT_LOG(s_logEnabled && isObjectLevelDebuggingEnabled(animationEnvironment), ("object [%s]: movement action [%s] chose stationary logical animation [%s] style [%s].\n", owner ? owner->getNetworkId().getValueString().c_str() : "<NULL>", getName().getString(), result.getString(), m_zeroSpeedAction->shouldApplyAnimationAsAdd(animationEnvironment) ? "add" : "replace"));
		return result;
	}
}

// ----------------------------------------------------------------------

bool EditableMovementAnimationAction::shouldHideHeldItem(const AnimationEnvironment &animationEnvironment) const
{
	if (isMoving(animationEnvironment))
		return m_speedAction->shouldHideHeldItem(animationEnvironment);
	else
		return m_zeroSpeedAction->shouldHideHeldItem(animationEnvironment);
}

// ----------------------------------------------------------------------

bool EditableMovementAnimationAction::shouldApplyAnimationAsAdd(const AnimationEnvironment &animationEnvironment) const
{
	if (isMoving(animationEnvironment))
		return m_speedAction->shouldApplyAnimationAsAdd(animationEnvironment);
	else
		return m_zeroSpeedAction->shouldApplyAnimationAsAdd(animationEnvironment);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const
{
	m_zeroSpeedAction->addReferencedLogicalAnimationNames(set);
	m_speedAction->addReferencedLogicalAnimationNames(set);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::setName(const CrcLowerString &actionName)
{
	m_zeroSpeedAction->setName(actionName);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::write(Iff &iff) const
{
	iff.insertForm(TAG_MVAC);

		m_zeroSpeedAction->write(iff);
		m_speedAction->write(iff);

	iff.exitForm(TAG_MVAC);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<movementAwareAction name='%s'>", getName().getString());

	writer.indent();
	{
		getSpeedAction().writeXmlActionCore(writer, "movementAction");
		getZeroSpeedAction().writeXmlActionCore(writer, "noMovementAction");
	}
	writer.unindent();

	writer.writeLine("</movementAwareAction>");
}



// ======================================================================
// class EditableMovementAnimationAction: private static member functions
// ======================================================================

void EditableMovementAnimationAction::remove()
{
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

bool EditableMovementAnimationAction::isMoving(const AnimationEnvironment &animationEnvironment)
{
	const Vector &velocity = animationEnvironment.getConstVector(AnimationEnvironmentNames::cms_locomotionVelocity);
	return ((velocity.x != 0.0f) || (velocity.y != 0.0f) || (velocity.z != 0.0f));
}

// ======================================================================
// class EditableMovementAnimationAction: private member functions
// ======================================================================

void EditableMovementAnimationAction::load_0001(Iff &iff, EditableAnimationStateHierarchyTemplate &hierarchyTemplate)
{
	const int versionNumber = 1;

	iff.enterForm(TAG_MVAC);

		IS_NULL(m_zeroSpeedAction);
		m_zeroSpeedAction = new EditableBasicAnimationAction(hierarchyTemplate, iff, versionNumber);

		IS_NULL(m_speedAction);
		m_speedAction = new EditableBasicAnimationAction(hierarchyTemplate, iff, versionNumber);

	iff.exitForm(TAG_MVAC);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::load_0002(Iff &iff, EditableAnimationStateHierarchyTemplate &hierarchyTemplate)
{
	const int versionNumber = 2;

	iff.enterForm(TAG_MVAC);

		IS_NULL(m_zeroSpeedAction);
		m_zeroSpeedAction = new EditableBasicAnimationAction(hierarchyTemplate, iff, versionNumber);

		IS_NULL(m_speedAction);
		m_speedAction = new EditableBasicAnimationAction(hierarchyTemplate, iff, versionNumber);

	iff.exitForm(TAG_MVAC);
}

// ----------------------------------------------------------------------

void EditableMovementAnimationAction::loadXml_1(XmlTreeNode const &treeNode, EditableAnimationStateHierarchyTemplate &hierarchyTemplate)
{
	//-- Get the action name.
	treeNode.getElementAttributeAsString(cs_nameAttributeName, s_name);

	//-- Get movement-aware action.
	XmlTreeNode const movementActionNode(treeNode.getFirstChildElementNode());
	FATAL(movementActionNode.isNull() || strcmp(movementActionNode.getName(), cs_movementActionElementName), ("expecting element [%s], found [%s].", cs_movementActionElementName, movementActionNode.getName()));
	m_speedAction = newActionCore(hierarchyTemplate, movementActionNode, "");

	//-- Get zero-speed action.
	XmlTreeNode const noMovementActionNode(movementActionNode.getNextSiblingElementNode());
	FATAL(noMovementActionNode.isNull() || strcmp(noMovementActionNode.getName(), cs_noMovementActionElementName), ("expecting element [%s], found [%s].", cs_noMovementActionElementName, noMovementActionNode.getName()));
	m_zeroSpeedAction = newActionCore(hierarchyTemplate, noMovementActionNode, s_name.c_str());
}

// ======================================================================
