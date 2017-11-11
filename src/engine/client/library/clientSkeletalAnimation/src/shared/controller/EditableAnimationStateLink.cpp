// ======================================================================
//
// EditableAnimationStateLink.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableAnimationStateLink.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/AnimationStatePath.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedXml/XmlTreeNode.h"

#include <set>
#include <string>

// ======================================================================

namespace EditableAnimationStateLinkNamespace
{	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const          TAG_LINK                    = TAG(L,I,N,K);

	char const *const  cs_statePathElementName     = "statePath";
	char const *const  cs_relativeAttributeName    = "relative";

	char const *const  cs_stateRefElementName      = "stateRef";
	char const *const  cs_nameAttributeName        = "name";

	char const *const  cs_logicalNameAttributeName = "logicalName";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string   s_name;
}

using namespace EditableAnimationStateLinkNamespace;

// ======================================================================
// Inlines.
// ======================================================================

inline void EditableAnimationStateLink::load_0001(Iff &iff)
{
	// Same as version 0000.
	load_0000(iff);
}

// ======================================================================

EditableAnimationStateLink::EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber) :
	AnimationStateLink(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_destinationPath(new AnimationStatePath()),
	m_transitionLogicalAnimationName(0)
{
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
			FATAL(true, ("EditableAnimationStateLink: unsupported version [%d].", versionNumber));
	}
}

// ----------------------------------------------------------------------

EditableAnimationStateLink::EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber) :
	AnimationStateLink(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_destinationPath(new AnimationStatePath()),
	m_transitionLogicalAnimationName(0)
{
	switch (versionNumber)
	{
		case 1:
			loadXml_1(treeNode);
			break;

		default:
			FATAL(true, ("EditableAnimationStateLink: unsupported version [%d].", versionNumber));
	}
}


// ----------------------------------------------------------------------

EditableAnimationStateLink::EditableAnimationStateLink(EditableAnimationStateHierarchyTemplate &hierarchyTemplate) :
	AnimationStateLink(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_destinationPath(new AnimationStatePath()),
	m_transitionLogicalAnimationName(new CrcLowerString())
{
}

// ----------------------------------------------------------------------

EditableAnimationStateLink::~EditableAnimationStateLink()
{
	delete m_transitionLogicalAnimationName;
	delete m_destinationPath;
}

// ----------------------------------------------------------------------

const AnimationStatePath &EditableAnimationStateLink::getDestinationPath() const
{
	return *m_destinationPath;
}

// ----------------------------------------------------------------------

bool EditableAnimationStateLink::hasTransitionLogicalAnimation() const
{
	if (!m_transitionLogicalAnimationName)
		return false;

	const char *const nameString = m_transitionLogicalAnimationName->getString();
	return (nameString && *nameString);
}

// ----------------------------------------------------------------------

const CrcLowerString &EditableAnimationStateLink::getTransitionLogicalAnimationName() const
{
	if (m_transitionLogicalAnimationName)
		return *m_transitionLogicalAnimationName;
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const
{
	if (m_transitionLogicalAnimationName && (*(m_transitionLogicalAnimationName->getString()) != 0))
		IGNORE_RETURN(set.insert(*m_transitionLogicalAnimationName));
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::setDestinationPath(const AnimationStatePath &path)
{
	path.copyTo(*m_destinationPath);
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::setTransitionLogicalAnimationName(const CrcLowerString &name)
{
	if (!m_transitionLogicalAnimationName)
		m_transitionLogicalAnimationName = new CrcLowerString(name);
	else
		*m_transitionLogicalAnimationName = name;
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::write(Iff &iff) const
{
	iff.insertChunk(TAG_LINK);
	
		//-- Write path info.
		const int pathLength = m_destinationPath->getPathLength();
		iff.insertChunkData(static_cast<int16>(pathLength));

		//-- Write path data.
		for (int i = 0; i < pathLength; ++i)
		{
			const CrcLowerString &stateName = AnimationStateNameIdManager::getNameString(m_destinationPath->getState(i));
			iff.insertChunkString(stateName.getString());
		}

		//-- Write transitional logical animation name data.
		const bool hasAnimation = hasTransitionLogicalAnimation();

		iff.insertChunkData(static_cast<int8>(hasAnimation ? 1 : 0));
		iff.insertChunkString((hasAnimation && m_transitionLogicalAnimationName) ? m_transitionLogicalAnimationName->getString() : "");

	iff.exitChunk(TAG_LINK);
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::writeXml(IndentedFileWriter &writer) const
{
	AnimationStatePath const &path = getDestinationPath();
	int const pathLength = path.getPathLength();
	if (pathLength <= 0)
	{
		WARNING(true, ("Ignoring writing of path to an empty state set."));
		return;
	}

	writer.writeLine("<stateLink>");

	writer.indent();
	{
		// Write absolute state path.
		writer.writeLine("<statePath relative='false'>");
		writer.indent();
		{
			for (int stateIndex = 0; stateIndex < pathLength; ++stateIndex)
			{
				writer.writeLineFormat("<stateRef name='%s'/>", AnimationStateNameIdManager::getNameString(path.getState(stateIndex)).getString());
			}
		}
		writer.unindent();
		writer.writeLine("</statePath>");

		// Write link animation.
		if (hasTransitionLogicalAnimation())
			writer.writeLineFormat("<linkAnimation logicalName='%s'/>", getTransitionLogicalAnimationName().getString());
	}
	writer.unindent();

	writer.writeLine("</stateLink>");
}

// ======================================================================
// EditableAnimationStateLink: private member functions
// ======================================================================

void EditableAnimationStateLink::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_LINK);

		//-- Retrieve # states in path.
		const int stateCount = static_cast<int>(iff.read_int16());
		DEBUG_FATAL(stateCount < 0, ("invalid state count [%d]", stateCount));

		//-- Construct the path.
		m_destinationPath->clearPath();
		
		// NOTE: Remove this check for zero-path handling from future
		//       versions of format.  It is here solely to fix the fact that
		//       initially I would omit the root from the path.

		if (stateCount == 0)
			m_destinationPath->appendState(AnimationStateNameIdManager::getRootId());
		else
		{
			char buffer[1024];
			for (int i = 0; i < stateCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer) - 1);

	#if 1
				//-- Check if root is first state.  If not, append root.  This check should go away in
				//   future version formats.  Initially I didn't embed the root node in the path --- it was
				//   assumed.
				if (i == 0)
				{
					if (_stricmp(buffer, "root") != 0)
					{
						// root is not the first node, append it.
						m_destinationPath->appendState(AnimationStateNameIdManager::getRootId());
					}
				}
	#endif

				m_destinationPath->appendState(AnimationStateNameIdManager::createId(CrcLowerString(buffer)));
			}
		}

		//-- Get transition logical animation information.
		const bool playLogicalAnimation  = (iff.read_int8() != 0);
		const int  logicalAnimationIndex = static_cast<int>(iff.read_int16());

		delete m_transitionLogicalAnimationName;
		if (playLogicalAnimation)
			m_transitionLogicalAnimationName = new CrcLowerString(m_hierarchyTemplate.getLogicalAnimationName(logicalAnimationIndex));
		else
			m_transitionLogicalAnimationName = 0;

	iff.exitChunk(TAG_LINK);
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::load_0002(Iff &iff)
{
	iff.enterChunk(TAG_LINK);

		//-- Retrieve # states in path.
		const int stateCount = static_cast<int>(iff.read_int16());
		DEBUG_FATAL(stateCount < 0, ("invalid state count [%d]", stateCount));

		//-- Construct the path.
		m_destinationPath->clearPath();
		
		// NOTE: Remove this check for zero-path handling from future
		//       versions of format.  It is here solely to fix the fact that
		//       initially I would omit the root from the path.
		char buffer[1024];

		if (stateCount == 0)
			m_destinationPath->appendState(AnimationStateNameIdManager::getRootId());
		else
		{
			for (int i = 0; i < stateCount; ++i)
			{
				iff.read_string(buffer, sizeof(buffer) - 1);
				m_destinationPath->appendState(AnimationStateNameIdManager::createId(CrcLowerString(buffer)));
			}
		}

		//-- Get transition logical animation information.
		const bool playLogicalAnimation  = (iff.read_int8() != 0);
		iff.read_string(buffer, sizeof(buffer) - 1);

		IS_NULL(m_transitionLogicalAnimationName);
		if (playLogicalAnimation)
			m_transitionLogicalAnimationName = new CrcLowerString(buffer);

	iff.exitChunk(TAG_LINK);
}

// ----------------------------------------------------------------------

void EditableAnimationStateLink::loadXml_1(XmlTreeNode const &treeNode)
{	
	//-- Handle state path element
	XmlTreeNode statePathNode(treeNode.getFirstChildElementNode());
	FATAL(!statePathNode.isElement() || _stricmp(statePathNode.getName(), cs_statePathElementName), ("loadXml_1(): expecting element [%s], instead found [%s].", cs_statePathElementName, statePathNode.getName()));

	bool relativePath = false;
	IGNORE_RETURN(statePathNode.getElementAttributeAsBool(cs_relativeAttributeName, relativePath, true));
	FATAL(relativePath, ("relative animation state path support is not yet implemented."));

	//-- Construct the path.
	m_destinationPath->clearPath();

	for (XmlTreeNode stateRefNode = statePathNode.getFirstChildElementNode(); !stateRefNode.isNull(); stateRefNode = stateRefNode.getNextSiblingElementNode())
	{
		FATAL(_stricmp(stateRefNode.getName(), cs_stateRefElementName), ("loadXml_1(): expecting element [%s], found [%s].", cs_stateRefElementName, stateRefNode.getName()));

		stateRefNode.getElementAttributeAsString(cs_nameAttributeName, s_name);
		FATAL(s_name.empty(), ("loadXml_1(): stateRef element specified a zero-length state name."));

		m_destinationPath->appendState(AnimationStateNameIdManager::createId(CrcLowerString(s_name.c_str())));
	}

	//-- Handle optional linkAnimation element
	XmlTreeNode linkAnimationNode(statePathNode.getNextSiblingElementNode());
	if (!linkAnimationNode.isNull())
	{
		linkAnimationNode.getElementAttributeAsString(cs_logicalNameAttributeName, s_name);
		FATAL(s_name.empty(), ("linkAnimation element specified a zero-length logical animation name."));

		m_transitionLogicalAnimationName = new CrcLowerString(s_name.c_str());
	}
}

// ======================================================================
