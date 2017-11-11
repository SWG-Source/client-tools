// ======================================================================
//
// EditableAnimationStateHierarchyTemplate.cpp
// Copyright 2002-2004 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"

#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedXml/XmlTreeDocument.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <string>
#include <vector>

// ======================================================================

namespace EditableAnimationStateHierarchyTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const *const cs_hierarchyStartElementFormat =
		"<hierarchy "
			"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' "
			"xsi:schemaLocation='http://namespaces.station.sony.com/animation/state-hierarchy-template/%d %sstate-hierarchy-template-%d.xsd' "
			"xmlns='http://namespaces.station.sony.com/animation/state-hierarchy-template/%d' "
			"version='%d'>";
	char const *const cs_hierarchyEndElement = "</hierarchy>";

	/// The directory where the .ash schema can be found, must include a trailing forward slash if non-empty.
	char const *const cs_localSchemaDirectoryPrefix = "";

	/// The .ASH format version number that this code writes.
	int const         cs_writeFormatVersion = 1;

	// Element and attribute constants.
	char const *const cs_hierarchyElementName = "hierarchy";
	char const *const cs_versionAttributeName = "version";

	char const *const cs_stateElementName     = "state";

#ifdef _DEBUG
	bool              s_dumpAshXmlTree;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if 0
	void  debugWalkXmlNode(xmlNode const *node, int indentLevel);
#endif
}

using namespace EditableAnimationStateHierarchyTemplateNamespace;

const int EditableAnimationStateHierarchyTemplate::cms_maxIffSize = 1 * (1024 * 1024);

const Tag TAG_AGPS = TAG(A,G,P,S);
const Tag TAG_ASHT = TAG(A,S,H,T);
const Tag TAG_LANS = TAG(L,A,N,S);

// ======================================================================
// namespace EditableAnimationStateHierarchyTemplateNamespace
// ======================================================================

// @todo remove this code when we're confident we know how to operate on an XML DOM tree.
#if 0

void EditableAnimationStateHierarchyTemplateNamespace::debugWalkXmlNode(xmlNode const *node, int indentLevel)
{
	NOT_NULL(node);

	// Print each sibling.
	while (node)
	{
		switch (node->type)
		{
		case XML_ELEMENT_NODE:
			{
				std::string  text(static_cast<std::string::size_type>(indentLevel) * 2, ' ');

				// Capture the node name.
				IGNORE_RETURN(text.append("<["));
				if (node->name)
				{
					IGNORE_RETURN(text.append(reinterpret_cast<char const*>(node->name)));
					IGNORE_RETURN(text.append("] "));
				}
				else
					IGNORE_RETURN(text.append("<null>] "));


				// Capture attributes.
				for (xmlAttr *theAttribute = node->properties; theAttribute != NULL; theAttribute = theAttribute->next)
				{
					IGNORE_RETURN(text.append("["));
					IGNORE_RETURN(text.append(reinterpret_cast<char const*>(theAttribute->name)));
					IGNORE_RETURN(text.append("]=["));

					xmlNode *attributeValue = theAttribute->children;
					if (attributeValue->type == XML_TEXT_NODE)
					{
						IGNORE_RETURN(text.append(reinterpret_cast<char const*>(attributeValue->content)));
						IGNORE_RETURN(text.append("] "));
					}
					else
					{
						char buffer[256];
						IGNORE_RETURN(snprintf(buffer, sizeof(buffer), "<unknown theAttribute value node type %d>] ", static_cast<int>(attributeValue->type)));
						buffer[sizeof(buffer) - 1] = '\0';
					}
				}
				
				// Cap off the line.
				IGNORE_RETURN(text.append(">\n"));

				// Print out this node's info.
				REPORT_LOG(true, (text.c_str()));
			}
			break;

		default:
			// do nothing.
			break;
		} //lint !e788 // not all enum constants appeared in switch statement. // intentional.

		// Print children
		if (node->children)
			debugWalkXmlNode(node->children, indentLevel + 1);

		// Move to next sibling.
		node = node->next;
	}
}

#endif

// ======================================================================

void EditableAnimationStateHierarchyTemplate::install()
{
	AnimationStateHierarchyTemplateList::setCreateFunction(create);
	AnimationStateHierarchyTemplateList::setXmlCreateFunction(xmlCreate);

#ifdef _DEBUG
	DebugFlags::registerFlag(s_dumpAshXmlTree, "ClientSkeletalAnimation/Animation", "dumpAshXmlTree");
#endif
}

// ======================================================================
// class EditableAnimationStateHierarchyTemplate: public member functions
// ======================================================================
// ----------------------------------------------------------------------
/**
 * Constructor to create an empty instance.
 *
 * This constructor should be used by a tool preparing to fill in a new
 * AnimationStateHierarchyTemplate file.
 */

EditableAnimationStateHierarchyTemplate::EditableAnimationStateHierarchyTemplate() :
	AnimationStateHierarchyTemplate(CrcLowerString::empty),
	m_rootState(0),
	m_actionGroups(new EditableAnimationActionGroupVector()),
	m_logicalAnimationNames(0)
{
	m_rootState = new EditableAnimationState(AnimationStateNameIdManager::getRootId(), *this);
}

// ----------------------------------------------------------------------

const AnimationState *EditableAnimationStateHierarchyTemplate::getConstRootAnimationState() const
{
	return m_rootState;
}

// ----------------------------------------------------------------------

int EditableAnimationStateHierarchyTemplate::getAnimationActionGroupCount() const
{
	return static_cast<int>(m_actionGroups->size());
}

// ----------------------------------------------------------------------

const AnimationActionGroup &EditableAnimationStateHierarchyTemplate::getConstAnimationActionGroup(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getAnimationActionGroupCount());
	return *(*m_actionGroups)[static_cast<EditableAnimationActionGroupVector::size_type>(index)];
}

// ----------------------------------------------------------------------

bool EditableAnimationStateHierarchyTemplate::lookupAnimationActionGroupIndex(const CrcLowerString &groupName, int &index) const
{
	std::pair<EditableAnimationActionGroupVector::iterator, EditableAnimationActionGroupVector::iterator> result = std::equal_range(m_actionGroups->begin(), m_actionGroups->end(), groupName, AnimationActionGroup::LessNameComparator()); //lint !e64 // type mismatch // no.
	if (result.first == result.second)
	{
		index = -1;
		return false;
	}
	else
	{
		index = static_cast<int>(std::distance(m_actionGroups->begin(), result.first));
		return true;
	}
}

// ----------------------------------------------------------------------

int EditableAnimationStateHierarchyTemplate::getLogicalAnimationCount() const
{
	if (!m_logicalAnimationNames)
		return 0;
	else
		return static_cast<int>(m_logicalAnimationNames->size());
}

// ----------------------------------------------------------------------

const CrcLowerString &EditableAnimationStateHierarchyTemplate::getLogicalAnimationName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getLogicalAnimationCount());

	NOT_NULL(m_logicalAnimationNames);
	return (*m_logicalAnimationNames)[static_cast<CrcLowerStringVector::size_type>(index)];
}

// ----------------------------------------------------------------------

bool EditableAnimationStateHierarchyTemplate::lookupLogicalAnimationIndex(const CrcLowerString &animationName, int &index) const
{
	// If FATAL occurs here, it is very likely that there is some bad code out there that hasn't been
	// updated to work with recent changes to the layout of the ASH data.
	NOT_NULL(m_logicalAnimationNames);

	std::pair<CrcLowerStringVector::iterator, CrcLowerStringVector::iterator> result = std::equal_range(m_logicalAnimationNames->begin(), m_logicalAnimationNames->end(), animationName); //lint !e64 // type mismatch // no
	if (result.first == result.second)
	{
		index = -1;
		return false;
	}
	else
	{
		index = std::distance(m_logicalAnimationNames->begin(), result.first);
		return true;
	}
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const
{
	const AnimationState *const animationState = getConstRootAnimationState();
	if (animationState)
		animationState->addReferencedLogicalAnimationNames(set);
}

// ----------------------------------------------------------------------

EditableAnimationActionGroup &EditableAnimationStateHierarchyTemplate::getAnimationActionGroup(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getAnimationActionGroupCount());
	return *(*m_actionGroups)[static_cast<EditableAnimationActionGroupVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::addAnimationActionGroup(EditableAnimationActionGroup *group)
{
	//-- Check for NULL group arg.
	if (!group)
	{
		DEBUG_FATAL(true, ("null group arg specified."));
		return; //lint !e527 // unreachable // reachable in release.
	}
	
	//-- Check for group with same name as existing group.
	int        testIndex     = -1;
	const bool alreadyExists = lookupAnimationActionGroupIndex(group->getName(), testIndex);

	if (alreadyExists)
	{
		WARNING(true, ("attempted to add already-existing group node [%s]", group->getName().getString()));
		return;
	}

	//-- Add group.
	m_actionGroups->push_back(group);

	//-- Resort groups by name as required by lookup.
	std::sort(m_actionGroups->begin(), m_actionGroups->end(), AnimationActionGroup::LessNameComparator());
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::deleteAnimationActionGroup(const CrcLowerString &groupName)
{
	//-- Find group index.
	int        index  = -1;
	const bool exists = lookupAnimationActionGroupIndex(groupName, index);

	if (!exists)
	{
		WARNING(true, ("AnimationActionGroup [%s] cannot be deleted because hierarchy doesn't know about it.", groupName.getString()));
		return;
	}

	//-- erase the element
	EditableAnimationActionGroupVector::iterator deleteIt = m_actionGroups->begin() + static_cast<EditableAnimationActionGroupVector::size_type>(index);
	IGNORE_RETURN(m_actionGroups->erase(deleteIt));
}

// ----------------------------------------------------------------------

bool EditableAnimationStateHierarchyTemplate::write(const std::string &outputPathName)
{
	//-- Create the iff.
	Iff  iff(cms_maxIffSize);

	iff.insertForm(TAG_ASHT);
		iff.insertForm(TAG_0002);

			// Define action groups
			if (!m_actionGroups->empty())
			{
				iff.insertForm(TAG_AGPS);

					// Specify # groups
					iff.insertChunk(TAG_INFO);
						iff.insertChunkData(static_cast<int16>(m_actionGroups->size()));
					iff.exitChunk(TAG_INFO);

					// Add each group
					const EditableAnimationActionGroupVector::iterator endIt = m_actionGroups->end();
					for (EditableAnimationActionGroupVector::iterator it = m_actionGroups->begin(); it != endIt; ++it)
					{
						NOT_NULL(*it);
						(*it)->write(iff);
					}

				iff.exitForm(TAG_AGPS);
			}

			// Write the root state hierarchy
			m_rootState->write(iff);

		iff.exitForm(TAG_0002);
	iff.exitForm(TAG_ASHT);

	//-- Save the iff to the specified file.
	return iff.write(outputPathName.c_str(), true);
}

// ----------------------------------------------------------------------

bool EditableAnimationStateHierarchyTemplate::writeXml(char const *outputPathName) const
{
	//-- Create the file writing object.
	IndentedFileWriter *const writer = IndentedFileWriter::createWriter(outputPathName);
	if (!writer)
	{
		WARNING(true, ("writeXml(): failed to create writer for file [%s], aborting ASH write.", outputPathName));
		return false;
	}

	//-- Write XML header.
	writer->writeLine("<?xml version='1.0'?>");

	//-- Write hierarchy element.
	writer->writeLineFormat(
		cs_hierarchyStartElementFormat, 
		cs_writeFormatVersion, 
		cs_localSchemaDirectoryPrefix, 
		cs_writeFormatVersion, 
		cs_writeFormatVersion,
		cs_writeFormatVersion);

	if (m_rootState)
	{
		writer->indent();
		{
			writer->writeLineFormat("<state name='%s'>", m_rootState->getStateName().getString());
			writer->indent();
			{
				m_rootState->writeXml(*writer);
			}
			writer->unindent();
			writer->writeLine("</state>");
		}
		writer->unindent();
	}

	writer->writeLine(cs_hierarchyEndElement);

	delete writer;

	// Success.
	return true;
}

// ======================================================================
// class EditableAnimationStateHierarchyTemplate: private static member functions
// ======================================================================

AnimationStateHierarchyTemplate *EditableAnimationStateHierarchyTemplate::create(const CrcString &pathName, Iff &iff)
{
	return new EditableAnimationStateHierarchyTemplate(pathName, iff);
}

// ----------------------------------------------------------------------

AnimationStateHierarchyTemplate *EditableAnimationStateHierarchyTemplate::xmlCreate(CrcString const &pathName, XmlTreeDocument const &xmlTreeDocument)
{
	EditableAnimationStateHierarchyTemplate *newTemplate = new EditableAnimationStateHierarchyTemplate(pathName, xmlTreeDocument);
	return newTemplate;
}

// ======================================================================
// class EditableAnimationStateHierarchyTemplate: private member functions
// ======================================================================
/**
 * Constructor to load an instance from iff data.
 *
 * @param iff   the iff data for the EditableAnimationStateHierarchyTemplate.
 * @param name  the name (pathname) for the file.  Generally used by
 *              sorted list management.  Accessible via getName().
 */

EditableAnimationStateHierarchyTemplate::EditableAnimationStateHierarchyTemplate(CrcString const &name, Iff &iff) :
	AnimationStateHierarchyTemplate(name),
	m_rootState(0),
	m_actionGroups(new EditableAnimationActionGroupVector()),
	m_logicalAnimationNames(0)
{
	//-- Load data from the iff.
	iff.enterForm(TAG_ASHT);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[5];
					ConvertTagToString(version, buffer);
					FATAL(true, ("ASH file format version [%s] is unsupported.", buffer));
				}
		}

	iff.exitForm(TAG_ASHT);

	//-- Create any data structures not created via the load.
	if (!m_rootState)
		m_rootState = new EditableAnimationState(AnimationStateNameIdManager::getRootId(), *this);
}

// ----------------------------------------------------------------------

EditableAnimationStateHierarchyTemplate::EditableAnimationStateHierarchyTemplate(const CrcString &name, XmlTreeDocument const &xmlTreeDocument) :
	AnimationStateHierarchyTemplate(name),
	m_rootState(0),
	m_actionGroups(new EditableAnimationActionGroupVector()),
	m_logicalAnimationNames(0)
{
	//-- Walk the xml document.
	// Ensure the root is a hierarchy element.
	XmlTreeNode  rootTreeNode = xmlTreeDocument.getRootTreeNode();

	FATAL(!rootTreeNode.isElement(), ("EditableAnimationStateHierarchyTemplate(): document root should be an element."));
	FATAL(_stricmp(cs_hierarchyElementName, rootTreeNode.getName()), ("EditableAnimationStateHierarchyTemplate(): expecting root node to be named [%s] but found [%s].", cs_hierarchyElementName, rootTreeNode.getName()));

	int versionNumber = -1;
	rootTreeNode.getElementAttributeAsInt(cs_versionAttributeName, versionNumber);

	switch (versionNumber)
	{
		case 1:
			loadXml_1(rootTreeNode.getFirstChildElementNode());
			break;

		default:
			FATAL(true, ("Unsupported AnimationStateHierarchyTemplate version [%d] from file [%s].", versionNumber, name.getString()));
	}

	//-- Create any data structures not created via the load.
	if (!m_rootState)
		m_rootState = new EditableAnimationState(AnimationStateNameIdManager::getRootId(), *this);
}

// ----------------------------------------------------------------------

EditableAnimationStateHierarchyTemplate::~EditableAnimationStateHierarchyTemplate()
{
	delete m_logicalAnimationNames;

	std::for_each(m_actionGroups->begin(), m_actionGroups->end(), PointerDeleter());
	delete m_actionGroups;

	delete m_rootState;
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::load_0000(Iff &iff)
{
	const int versionNumber = 0;

	iff.enterForm(TAG_0000);
		
		//-- Load logical animation names.
		if (iff.enterChunk(TAG_LANS, true))
		{
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid logical animation name count value [%d].", count));

			IS_NULL(m_logicalAnimationNames);
			m_logicalAnimationNames = new CrcLowerStringVector();
			m_logicalAnimationNames->reserve(static_cast<CrcLowerStringVector::size_type>(count));

			char name[1024];
			for (int i = 0; i < count; ++i)
			{
				iff.read_string(name, sizeof(name) - 1);
				m_logicalAnimationNames->push_back(CrcLowerString(name));
			}

			iff.exitChunk(TAG_LANS);
		}

		//-- Load action groups.
		if (iff.enterForm(TAG_AGPS, true))
		{
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid action group count value [%d].", count));

			iff.exitChunk(TAG_INFO);

			m_actionGroups->clear();
			m_actionGroups->reserve(static_cast<EditableAnimationActionGroupVector::size_type>(count));

			for (int i = 0; i < count; ++i)
			{
				m_actionGroups->push_back(new EditableAnimationActionGroup(*this, iff, versionNumber));
			}

			iff.exitForm(TAG_AGPS);
		}

		//-- Load root state hierarchy.
		delete m_rootState;
		m_rootState = new EditableAnimationState(*this, iff, versionNumber);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::load_0001(Iff &iff)
{
	const int versionNumber = 1;

	iff.enterForm(TAG_0001);
		
		//-- Load logical animation names.
		if (iff.enterChunk(TAG_LANS, true))
		{
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid logical animation name count value [%d].", count));

			IS_NULL(m_logicalAnimationNames);
			m_logicalAnimationNames = new CrcLowerStringVector();
			m_logicalAnimationNames->reserve(static_cast<CrcLowerStringVector::size_type>(count));

			char name[1024];
			for (int i = 0; i < count; ++i)
			{
				iff.read_string(name, sizeof(name) - 1);
				m_logicalAnimationNames->push_back(CrcLowerString(name));
			}

			iff.exitChunk(TAG_LANS);
		}

		//-- Load action groups.
		if (iff.enterForm(TAG_AGPS, true))
		{
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid action group count value [%d].", count));

			iff.exitChunk(TAG_INFO);

			m_actionGroups->clear();
			m_actionGroups->reserve(static_cast<EditableAnimationActionGroupVector::size_type>(count));

			for (int i = 0; i < count; ++i)
			{
				m_actionGroups->push_back(new EditableAnimationActionGroup(*this, iff, versionNumber));
			}

			iff.exitForm(TAG_AGPS);
		}

		//-- Load root state hierarchy.
		delete m_rootState;
		m_rootState = new EditableAnimationState(*this, iff, versionNumber);

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::load_0002(Iff &iff)
{
	const int versionNumber = 2;

	iff.enterForm(TAG_0002);
		
		//-- Load action groups.
		if (iff.enterForm(TAG_AGPS, true))
		{
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid action group count value [%d].", count));

			iff.exitChunk(TAG_INFO);

			m_actionGroups->clear();
			m_actionGroups->reserve(static_cast<EditableAnimationActionGroupVector::size_type>(count));

			for (int i = 0; i < count; ++i)
			{
				m_actionGroups->push_back(new EditableAnimationActionGroup(*this, iff, versionNumber));
			}

			iff.exitForm(TAG_AGPS);
		}

		//-- Load root state hierarchy.
		delete m_rootState;
		m_rootState = new EditableAnimationState(*this, iff, versionNumber);

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void EditableAnimationStateHierarchyTemplate::loadXml_1(XmlTreeNode const &treeNode)
{
	//-- Clear out any existing root state.
	delete m_rootState;
	m_rootState = NULL;

	//-- Bail out if there is no state info.
	if (treeNode.isNull())
		return;

	//-- This node should be named 'state'.
	char const *const nodeName = treeNode.getName();
	FATAL(!nodeName || _stricmp(nodeName, cs_stateElementName), ("loadXml_1(): expecting element [%s], found [%s].\n", cs_stateElementName, nodeName));

	//-- Create the state.
	m_rootState = new EditableAnimationState(*this, treeNode, 1);
}

// ======================================================================
