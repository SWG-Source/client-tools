// ======================================================================
//
// EditableBasicAnimationAction.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"

#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationDebugging.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Object.h"
#include "sharedXml/XmlTreeNode.h"

#include <cstdio>
#include <set>
#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(EditableBasicAnimationAction, true, 0, 0, 0);

// ======================================================================

std::string const EditableBasicAnimationAction::cms_defaultLogicalAnimationName("default");
std::string const EditableBasicAnimationAction::cms_baseDefaultActionName("action");

// ======================================================================

int EditableBasicAnimationAction::ms_newActionCount;

// ======================================================================

namespace EditableBasicAnimationActionNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool isObjectLevelDebuggingEnabled(AnimationEnvironment const &animationEnvironment);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_ACTN = TAG(A,C,T,N);

	char const *const  cs_nameAttributeName          = "name";
	char const *const  cs_actionElementName          = "action";
	char const *const  cs_useAddTrackAttributeName   = "useAddTrack";
	char const *const  cs_actionAnimationElementName = "actionAnimation";
	char const *const  cs_logicalNameAttributeName   = "logicalName";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_logEnabled;

	std::string  s_name;
}

using namespace EditableBasicAnimationActionNamespace;

// ======================================================================

bool EditableBasicAnimationActionNamespace::isObjectLevelDebuggingEnabled(AnimationEnvironment const &animationEnvironment)
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
// Inlines.
// ======================================================================

inline void EditableBasicAnimationAction::load_0001(Iff &iff)
{
	//-- Identical to the version 0000 load code.
	load_0000(iff);
}

// ======================================================================

void EditableBasicAnimationAction::install()
{
	DebugFlags::registerFlag(s_logEnabled, "ClientSkeletalAnimation/Action", "logSimpleAction");

	installMemoryBlockManager();
	ExitChain::add(remove, "EditableBasicAnimationAction");
}

// ======================================================================

EditableBasicAnimationAction::EditableBasicAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber) :
	EditableAnimationAction(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_actionName(),
	m_logicalAnimationName(),
	m_hideHeldItem(false),
	m_applyAnimationAsAdd(false)
{
	//-- Load iff.
	switch (versionNumber)
	{
		case 0:
			load_0000(iff);
			break;

		case 1:
			load_0001(iff);
			break;

		case 2:
			load_0002(iff);
			break;

		default:
			FATAL(true, ("EditableBasicAnimationAction: unsupported version [%d].", versionNumber));
	}
}

// ----------------------------------------------------------------------

EditableBasicAnimationAction::EditableBasicAnimationAction(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber) :
	EditableAnimationAction(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_actionName(),
	m_logicalAnimationName(),
	m_hideHeldItem(false),
	m_applyAnimationAsAdd(false)
{
	//-- Load iff.
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode);
			break;

		default:
			FATAL(true, ("EditableBasicAnimationAction: unsupported version [%d].", versionNumber));
	}
}

// ----------------------------------------------------------------------

EditableBasicAnimationAction::EditableBasicAnimationAction(const CrcLowerString &actionName, EditableAnimationStateHierarchyTemplate &hierarchyTemplate) :
	EditableAnimationAction(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_actionName(*actionName.getString() ? actionName : CrcLowerString(getNewActionName().c_str())),
	m_logicalAnimationName(CrcLowerString(cms_defaultLogicalAnimationName.c_str())),
	m_hideHeldItem(false),
	m_applyAnimationAsAdd(false)
{
}

// ======================================================================

const CrcLowerString &EditableBasicAnimationAction::getName() const
{
	return m_actionName;
}

// ----------------------------------------------------------------------

const CrcLowerString &EditableBasicAnimationAction::getLogicalAnimationName(const AnimationEnvironment &animationEnvironment) const
{
	UNREF(animationEnvironment);

#ifdef _DEBUG
	SkeletalAppearance2 const *const appearance = animationEnvironment.getSkeletalAppearance();
	Object const              *const owner      = (appearance ? appearance->getOwner() : 0);

	DEBUG_REPORT_LOG(s_logEnabled && isObjectLevelDebuggingEnabled(animationEnvironment), ("object [%s]: basic action [%s] returns logical animation [%s] style [%s].\n", owner ? owner->getNetworkId().getValueString().c_str() : "<NULL>", getName().getString(), m_logicalAnimationName.getString(), shouldApplyAnimationAsAdd(animationEnvironment) ? "add" : "replace"));
#endif
	return m_logicalAnimationName;
}

// ----------------------------------------------------------------------

bool EditableBasicAnimationAction::shouldHideHeldItem(const AnimationEnvironment & /* animationEnvironment */) const
{
	return m_hideHeldItem;
}

// ----------------------------------------------------------------------

bool EditableBasicAnimationAction::shouldApplyAnimationAsAdd(const AnimationEnvironment & /* animationEnvironment */) const
{
	return m_applyAnimationAsAdd;
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const
{
	if (*(m_logicalAnimationName.getString()) != 0)
		IGNORE_RETURN(set.insert(m_logicalAnimationName));
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::setName(const CrcLowerString &actionName)
{
	m_actionName = actionName;
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::write(Iff &iff) const
{
	iff.insertChunk(TAG_ACTN);

		iff.insertChunkString(m_actionName.getString());
		iff.insertChunkString(m_logicalAnimationName.getString());
		iff.insertChunkData(static_cast<int8>(m_hideHeldItem ? 1 : 0));
		iff.insertChunkData(static_cast<int8>(m_applyAnimationAsAdd ? 1 : 0));

	iff.exitChunk(TAG_ACTN);
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<basicAction name='%s'>", getName().getString());

	writer.indent();
	{
		writeXmlActionCore(writer, "action");
	}
	writer.unindent();

	writer.writeLine("</basicAction>");
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::setLogicalAnimationName(const CrcLowerString &animationName)
{
	m_logicalAnimationName = animationName;
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::setShouldHideHeldItem(bool shouldHide)
{
	m_hideHeldItem = shouldHide;
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::setShouldApplyAnimationAsAdd(bool applyAsAdd)
{
	m_applyAnimationAsAdd = applyAsAdd;
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::writeXmlActionCore(IndentedFileWriter &writer, char const *elementName) const
{
	writer.writeLineFormat("<%s useAddTrack='%s'>", elementName, m_applyAnimationAsAdd ? "true" : "false");

	writer.indent();
	{
		writer.writeLineFormat("<actionAnimation logicalName='%s'/>", m_logicalAnimationName.getString());
	}
	writer.unindent();

	writer.writeLineFormat("</%s>", elementName);
}

// ======================================================================
// class EditableBasicAnimationAction: private static member functions
// ======================================================================

void EditableBasicAnimationAction::remove()
{
	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

std::string EditableBasicAnimationAction::getNewActionName()
{
	//-- Increment new action name count
	++ms_newActionCount;

	//-- Convert number to string.
	char numberAsString[32];
	sprintf(numberAsString, "%d", ms_newActionCount);

	//-- Return new string.
	return cms_baseDefaultActionName + numberAsString;
}

// ======================================================================
// class EditableBasicAnimationAction: private member functions
// ======================================================================

void EditableBasicAnimationAction::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_ACTN);

		//-- Load action name.
		char name[1024];
		iff.read_string(name, sizeof(name) - 1);
		m_actionName.setString(name);

		//-- Load logical animation name index.
		const int logicalAnimationNameIndex = static_cast<int>(iff.read_int16());
		strcpy(name, m_hierarchyTemplate.getLogicalAnimationName(logicalAnimationNameIndex).getString());

		//-- Strip off trailing spaces.
		int scanChar = static_cast<int>(strlen(name)) - 1;
		while ((scanChar >= 0) && (name[scanChar] == ' '))
			--scanChar;
		name[scanChar + 1] = 0;

		m_logicalAnimationName.setString(name);

		//-- Retrieve held item visibility disposition.
		m_hideHeldItem = (iff.read_int8() != 0);

		//-- Very bad practice --- I added this variable but didn't increment the
		//   version number.  Incrementing the version number for this involves incrementing
		//   it for the entire animation state hierarchy.  Take this out of future versions
		//   where we will always read/write the "apply animation as add" field.
		if (iff.getChunkLengthLeft())
			m_applyAnimationAsAdd = (iff.read_int8() != 0);
		else
		{
			// If not specified, defaults to false (priority-replacement).
			m_applyAnimationAsAdd = false;
		}

	iff.exitChunk(TAG_ACTN);
}

// ----------------------------------------------------------------------

void EditableBasicAnimationAction::load_0002(Iff &iff)
{
	iff.enterChunk(TAG_ACTN);

		//-- Load action name.
		char name[1024];
		iff.read_string(name, sizeof(name) - 1);
		m_actionName.setString(name);

		//-- Load logical animation name.
		iff.read_string(name, sizeof(name) - 1);

		//-- Strip off trailing spaces.
		int scanChar = static_cast<int>(strlen(name)) - 1;
		while ((scanChar >= 0) && (name[scanChar] == ' '))
			--scanChar;
		name[scanChar + 1] = 0;
		m_logicalAnimationName.setString(name);

		//-- Retrieve held item visibility disposition.
		m_hideHeldItem        = (iff.read_int8() != 0);
		m_applyAnimationAsAdd = (iff.read_int8() != 0);

	iff.exitChunk(TAG_ACTN);
}

void EditableBasicAnimationAction::loadXml_1(XmlTreeNode const &treeNode)
{
	//-- Get the action name.
	treeNode.getElementAttributeAsString(cs_nameAttributeName, s_name);
	m_actionName.setString(s_name.c_str());

	//-- Get the action core.
	XmlTreeNode const actionNode(treeNode.getFirstChildElementNode());
	FATAL(actionNode.isNull() || _stricmp(actionNode.getName(), cs_actionElementName), ("expecting element [%s], found [%s].", cs_actionElementName, actionNode.getName()));

	//-- Get add track disposition.
	actionNode.getElementAttributeAsBool(cs_useAddTrackAttributeName, m_applyAnimationAsAdd);

	//-- Get the animation name.
	XmlTreeNode const actionAnimationNode(actionNode.getFirstChildElementNode());
	FATAL(actionAnimationNode.isNull() || _stricmp(actionAnimationNode.getName(), cs_actionAnimationElementName), ("expecting element [%s], found [%s].", cs_actionAnimationElementName, actionAnimationNode.getName()));

	actionAnimationNode.getElementAttributeAsString(cs_logicalNameAttributeName, s_name);
	m_logicalAnimationName.setString(s_name.c_str());
}

// ======================================================================
