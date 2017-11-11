// ======================================================================
//
// EditableAnimationState.cpp
// Copyright 2002-2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableAnimationState.h"

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include "clientSkeletalAnimation/AnimationStateNameIdManager.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/EditableAnimationStateLink.h"
#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"
#include "sharedFile/Iff.h"
#include "sharedFile/IndentedFileWriter.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedXml/XmlTreeNode.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(EditableAnimationState, true, 0, 0, 0);

// ======================================================================

namespace EditableAnimationStateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ConstCharCrcString const cs_defaultLogicalAnimationName("default");

	Tag const TAG_ACTN = TAG(A,C,T,N);
	Tag const TAG_ACTS = TAG(A,C,T,S);
	Tag const TAG_CHLD = TAG(C,H,L,D);
	Tag const TAG_DAGR = TAG(D,A,G,R);
	Tag const TAG_GAGR = TAG(G,A,G,R);
	Tag const TAG_LNKS = TAG(L,N,K,S);
	Tag const TAG_LOCA = TAG(L,O,C,A);
	Tag const TAG_MVAC = TAG(M,V,A,C);
	Tag const TAG_STAT = TAG(S,T,A,T);

	char const *const  cs_nameAttributeName              = "name";

	char const *const  cs_idleAnimationElementName       = "idleAnimation";
	char const *const  cs_logicalNameAttributeName       = "logicalName";
	char const *const  cs_stateActionsElementName        = "stateActions";
	char const *const  cs_stateLinksElementName          = "stateLinks";
	char const *const  cs_stateLinkElementName           = "stateLink";
	char const *const  cs_childStatesElementName         = "childStates";
	char const *const  cs_stateElementName               = "state";
	char const *const  cs_basicActionElementName         = "basicAction";
	char const *const  cs_movementAwareActionElementName = "movementAwareAction";

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string  s_logicalName;
	std::string  s_stateName;
}

using namespace EditableAnimationStateNamespace;

// ======================================================================

struct EditableAnimationState::LessNameComparator
{
	bool operator ()(const EditableAnimationState *lhs, const EditableAnimationState *rhs) const;
	bool operator ()(const CrcLowerString &lhs, const EditableAnimationState *rhs) const;
	bool operator ()(const EditableAnimationState *lhs, const CrcLowerString &rhs) const;
};

// ======================================================================
// class EditableAnimationState::LessNameComparator
// ======================================================================

bool EditableAnimationState::LessNameComparator::operator ()(const EditableAnimationState *lhs, const EditableAnimationState *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	return lhs->getStateName() < rhs->getStateName();
}

// ----------------------------------------------------------------------

bool EditableAnimationState::LessNameComparator::operator ()(const CrcLowerString &lhs, const EditableAnimationState *rhs) const
{
	NOT_NULL(rhs);

	return lhs < rhs->getStateName();
}

// ----------------------------------------------------------------------

bool EditableAnimationState::LessNameComparator::operator ()(const EditableAnimationState *lhs, const CrcLowerString &rhs) const
{
	NOT_NULL(lhs);

	return lhs->getStateName() < rhs;
}

// ======================================================================
// class EditableAnimationState: public member functions
// ======================================================================

EditableAnimationState::EditableAnimationState(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber) :
	AnimationState(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_stateNameId(0),
	m_statePath(),
	m_parentState(0),
	m_childStates(),
	m_logicalAnimationName("", false),
	m_stateLinks(),
	m_actions(0),
	m_grantedActionGroupIndices(0),
	m_deniedActionGroupIndices(0)
{
	switch (versionNumber)
	{
		case 0:
			load_0000(iff, versionNumber);
			break;

		case 1:
			load_0001(iff, versionNumber);
			break;

		case 2:
			load_0002(iff, versionNumber);
			break;

		default:
			FATAL(true, ("EditableAnimationState: unsupported format version [%d].", versionNumber));
	}
}

// ----------------------------------------------------------------------

EditableAnimationState::EditableAnimationState(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, XmlTreeNode const &treeNode, int versionNumber) :
	AnimationState(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_stateNameId(0),
	m_statePath(),
	m_parentState(0),
	m_childStates(),
	m_logicalAnimationName("", false),
	m_stateLinks(),
	m_actions(0),
	m_grantedActionGroupIndices(0),
	m_deniedActionGroupIndices(0)
{
	switch (versionNumber)
	{
	case 1:
		loadXml_1(treeNode);
		break;

	default:
		FATAL(true, ("EditableAnimationState: unsupported version number [%d].", versionNumber));
	}
}

// ----------------------------------------------------------------------

EditableAnimationState::EditableAnimationState(const AnimationStateNameId &stateNameId, EditableAnimationStateHierarchyTemplate &hierarchyTemplate) :
	AnimationState(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_stateNameId(new AnimationStateNameId(stateNameId)),
	m_statePath(),
	m_parentState(0),
	m_childStates(),
	m_logicalAnimationName(cs_defaultLogicalAnimationName),
	m_stateLinks(),
	m_actions(new EditableAnimationActionVector()),
	m_grantedActionGroupIndices(new IntVector()),
	m_deniedActionGroupIndices(new IntVector())
{
}

// ----------------------------------------------------------------------

EditableAnimationState::~EditableAnimationState()
{
	delete m_deniedActionGroupIndices;
	delete m_grantedActionGroupIndices;

	if (m_actions)
	{
		std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
		delete m_actions;
	}
	
	std::for_each(m_stateLinks.begin(), m_stateLinks.end(), PointerDeleter());
	
	//-- Delete child states.
	removeAllChildStates();

	//-- Remove from parent (if there is one).
	if (m_parentState)
	{
		m_parentState->removeChildState(this);
		m_parentState = 0;
	}

	delete m_stateNameId;
}

// ----------------------------------------------------------------------

const AnimationStatePath &EditableAnimationState::getStatePath() const
{
	buildStatePath(m_statePath);
	return m_statePath;
}

// ----------------------------------------------------------------------

const AnimationStateNameId EditableAnimationState::getStateNameId() const
{
	NOT_NULL(m_stateNameId);
	return *m_stateNameId;
}

// ----------------------------------------------------------------------

const AnimationState *EditableAnimationState::getConstParentState() const
{
	return m_parentState;
}

// ----------------------------------------------------------------------

int EditableAnimationState::getChildStateCount() const
{
	return static_cast<int>(m_childStates.size());
}

// ----------------------------------------------------------------------

const AnimationState &EditableAnimationState::getConstChildState(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getChildStateCount());
	return *m_childStates[static_cast<EditableAnimationStateVector::size_type>(index)];
}

// ----------------------------------------------------------------------

bool EditableAnimationState::lookupChildStateIndex(const AnimationStateNameId &childNameId, int &index) const
{
	//-- lookup the child's name string from the child's id
	const CrcLowerString &childNameString = AnimationStateNameIdManager::getNameString(childNameId);

	//-- check if we've got the child
	std::pair<EditableAnimationStateVector::const_iterator, EditableAnimationStateVector::const_iterator> result = std::equal_range(m_childStates.begin(), m_childStates.end(), childNameString, LessNameComparator()); //lint !e64 // type mismatch // lint confusion?
	if (result.first == result.second)
	{
		index = -1;
		return false;
	}
	else
	{
		index = std::distance(m_childStates.begin(), result.first);
		return true;
	}
}

// ----------------------------------------------------------------------

CrcString const &EditableAnimationState::getLogicalAnimationName() const
{
	return m_logicalAnimationName;
}

// ----------------------------------------------------------------------

int EditableAnimationState::getLinkCount() const
{
	return static_cast<int>(m_stateLinks.size());
}

// ----------------------------------------------------------------------

const AnimationStateLink &EditableAnimationState::getConstLink(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getLinkCount());
	return *m_stateLinks[static_cast<EditableAnimationStateLinkVector::size_type>(index)];
}

// ----------------------------------------------------------------------

int EditableAnimationState::getActionCount() const
{
	if (m_actions)
		return static_cast<int>(m_actions->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

const AnimationAction &EditableAnimationState::getConstAction(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getActionCount());
	NOT_NULL(m_actions);
	return *(*m_actions)[static_cast<EditableAnimationActionVector::size_type>(index)];
}

// ----------------------------------------------------------------------

int EditableAnimationState::getGrantedActionGroupCount() const
{
	if (m_grantedActionGroupIndices)
		return static_cast<int>(m_grantedActionGroupIndices->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

int EditableAnimationState::getGrantedActionGroupIndex(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getGrantedActionGroupCount());
	NOT_NULL(m_grantedActionGroupIndices);

	return (*m_grantedActionGroupIndices)[static_cast<IntVector::size_type>(index)];
}

// ----------------------------------------------------------------------

int EditableAnimationState::getDeniedActionGroupCount() const
{
	if (m_deniedActionGroupIndices)
		return static_cast<int>(m_deniedActionGroupIndices->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

int EditableAnimationState::getDeniedActionGroupIndex(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getDeniedActionGroupCount());
	NOT_NULL(m_deniedActionGroupIndices);

	return (*m_deniedActionGroupIndices)[static_cast<IntVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void EditableAnimationState::setStateNameId(const AnimationStateNameId &nameId)
{
	NOT_NULL(m_stateNameId);
	*m_stateNameId = nameId;
}

// ----------------------------------------------------------------------

CrcString const &EditableAnimationState::getStateName() const
{
	NOT_NULL(m_stateNameId);
	return AnimationStateNameIdManager::getNameString(*m_stateNameId);
}

// ----------------------------------------------------------------------

EditableAnimationState *EditableAnimationState::getParentState()
{
	return m_parentState;
}

// ----------------------------------------------------------------------

void EditableAnimationState::setParentState(EditableAnimationState *parentState)
{
	m_parentState = parentState;
}

// ----------------------------------------------------------------------

EditableAnimationState &EditableAnimationState::getChildState(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getChildStateCount());
	return *m_childStates[static_cast<EditableAnimationStateVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void EditableAnimationState::addChildState(EditableAnimationState *child)
{
	NOT_NULL(child);

	//-- Add child to list of children.
	m_childStates.push_back(child);

	//-- Set child's parent to this node.
	child->setParentState(this);

	//-- Sort children.
	std::sort(m_childStates.begin(), m_childStates.end(), LessNameComparator());
}

// ----------------------------------------------------------------------
/**
 * Remove, but not delete, the specified child state from this state.
 *
 * This function will not delete the specified child state instance.
 * The caller must delete the instance if that is desired.  This function
 * can be used as part of a move operation where the child state
 * should not be deleted.
 *
 * @param child  the child state instance to remove as a child from this
 *               state.
 */

void EditableAnimationState::removeChildState(EditableAnimationState *child)
{
	NOT_NULL(child);

	//-- Find the child state.
	const EditableAnimationStateVector::iterator findIt = std::find(m_childStates.begin(), m_childStates.end(), child);
	if (findIt == m_childStates.end())
	{
		WARNING(true, ("removeChildState(): specified child [0x%08x] is not a child.", reinterpret_cast<int>(child)));
		return;
	}

	//-- Remove child state from list of children.
	IGNORE_RETURN(m_childStates.erase(findIt));

	//-- Clear the child's parent state.
	child->setParentState(0);
}

// ----------------------------------------------------------------------

void EditableAnimationState::removeAllChildStates()
{
	//-- Reset each child's parent.  If this isn't done, the child will explicitly remove itself from the parent, inefficient.
	std::for_each(m_childStates.begin(), m_childStates.end(), VoidBindSecond(VoidMemberFunction(&EditableAnimationState::setParentState), 0));

	//-- Delete each child.
	std::for_each(m_childStates.begin(), m_childStates.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void EditableAnimationState::setLogicalAnimationName(CrcString const &name)
{
	m_logicalAnimationName.set(name.getString(), name.getCrc());
}

// ----------------------------------------------------------------------

EditableAnimationStateLink &EditableAnimationState::getLink(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getLinkCount());
	return *m_stateLinks[static_cast<EditableAnimationStateLinkVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void EditableAnimationState::addLink(EditableAnimationStateLink *link)
{
	NOT_NULL(link);
	m_stateLinks.push_back(link);
}

// ----------------------------------------------------------------------

void EditableAnimationState::removeLink(EditableAnimationStateLink *link)
{
	EditableAnimationStateLinkVector::iterator findIt = std::find(m_stateLinks.begin(), m_stateLinks.end(), link);
	if (findIt == m_stateLinks.end())
		UNREF(m_stateLinks.erase(findIt));
	else
		WARNING(true, ("tried to remove EditableAnimationStateLink (0x%08x) but state doesn't know about link.", reinterpret_cast<int>(link)));
}

// ----------------------------------------------------------------------

EditableAnimationAction &EditableAnimationState::getAction(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getActionCount());
	NOT_NULL(m_actions);
	return *(*m_actions)[static_cast<EditableAnimationActionVector::size_type>(index)];
}

// ----------------------------------------------------------------------

void EditableAnimationState::addAction(EditableAnimationAction *action)
{
	//-- Check for NULL action.
	if (!action)
	{
		DEBUG_FATAL(true, ("tried to add null action to state"));
		return; //lint !e527 // Unreachable // Reachable in release.
	}

	//-- Add the action to the list.
	if (!m_actions)
		m_actions = new EditableAnimationActionVector();

	m_actions->push_back(action);

	//-- Sort list by name, abc order.
	std::sort(m_actions->begin(), m_actions->end(), EditableAnimationAction::LessNameAbcOrderPointerComparator());
}

// ----------------------------------------------------------------------

void EditableAnimationState::removeAction(EditableAnimationAction *action)
{
	//-- Check for null action.
	if (!action)
	{
		DEBUG_FATAL(true, ("tried to remove NULL action from state."));
		return; //lint !e527 // Unreachable // Reachable in release.
	}

	if (!m_actions)
	{
		WARNING(true, ("specified action [%s] does not exist for state.", action->getName().getString()));
		return;
	}

	//-- Find the specified action.
	EditableAnimationActionVector::iterator findIt = std::find(m_actions->begin(), m_actions->end(), action);
	if (findIt == m_actions->end())
	{
		WARNING(true, ("specified action [%s] does not exist for state.", action->getName().getString()));
		return;
	}

	//-- Remove it from the list of actions.
	IGNORE_RETURN(m_actions->erase(findIt));
}

// ----------------------------------------------------------------------

void EditableAnimationState::addGrantedActionGroupIndex(int actionGroupIndex)
{
	if (!m_grantedActionGroupIndices)
		m_grantedActionGroupIndices = new IntVector();

	//-- Ensure named group doesn't exist in granted action group list already.
	{
		if (std::find(m_grantedActionGroupIndices->begin(), m_grantedActionGroupIndices->end(), actionGroupIndex) != m_grantedActionGroupIndices->end())
		{
			WARNING(true, ("tried to add granted action group [%d] when already exists for this state.", actionGroupIndex));
			return;
		}
	}
	
	//-- Ensure named group doesn't exist in denied action group list.
	if (m_deniedActionGroupIndices)
	{
		if (std::find(m_deniedActionGroupIndices->begin(), m_deniedActionGroupIndices->end(), actionGroupIndex) != m_deniedActionGroupIndices->end())
		{
			WARNING(true, ("tried to add granted action group [%d] when action group is denied for this state.", actionGroupIndex));
			return;
		}
	}

	//-- Add to action group list.
	m_grantedActionGroupIndices->push_back(actionGroupIndex);
}

// ----------------------------------------------------------------------

void EditableAnimationState::deleteGrantedActionGroupIndex(int actionGroupIndex)
{
	if (!m_grantedActionGroupIndices)
	{
		WARNING(true, ("Tried to delete granted action group [%d] when does not exist for this state.", actionGroupIndex));
		return;
	}

	//-- find the specified group name
	const IntVector::iterator findIt = std::find(m_grantedActionGroupIndices->begin(), m_grantedActionGroupIndices->end(), actionGroupIndex);
	if (findIt == m_grantedActionGroupIndices->end())
	{
		WARNING(true, ("Tried to delete granted action group [%d] when does not exist for this state.", actionGroupIndex));
		return;
	}

	//-- remove the specified group name
	IGNORE_RETURN(m_grantedActionGroupIndices->erase(findIt));
}

// ----------------------------------------------------------------------

void EditableAnimationState::addDeniedActionGroupIndex(int actionGroupIndex)
{
	if (!m_deniedActionGroupIndices)
		m_deniedActionGroupIndices = new IntVector();

	//-- Ensure named group doesn't exist in granted action group list.
	if (m_grantedActionGroupIndices)
	{
		if (std::find(m_grantedActionGroupIndices->begin(), m_grantedActionGroupIndices->end(), actionGroupIndex) != m_grantedActionGroupIndices->end())
		{
			WARNING(true, ("tried to add denied action group [%d] when exists in granted action group.", actionGroupIndex));
			return;
		}
	}
	
	//-- Ensure named group doesn't exist in denied action group list already.
	{
		if (std::find(m_deniedActionGroupIndices->begin(), m_deniedActionGroupIndices->end(), actionGroupIndex) != m_deniedActionGroupIndices->end())
		{
			WARNING(true, ("tried to add denied action group [%d] multiple times to the same state.", actionGroupIndex));
			return;
		}
	}

	//-- Add to action group list.
	m_deniedActionGroupIndices->push_back(actionGroupIndex);
}

// ----------------------------------------------------------------------

void EditableAnimationState::deleteDeniedActionGroupIndex(int actionGroupIndex)
{
	if (!m_deniedActionGroupIndices)
	{
		WARNING(true, ("Tried to delete denied action group [%d] when does not exist for this state.", actionGroupIndex));
		return;
	}

	//-- Find the specified group name.
	const IntVector::iterator findIt = std::find(m_deniedActionGroupIndices->begin(), m_deniedActionGroupIndices->end(), actionGroupIndex);
	if (findIt == m_deniedActionGroupIndices->end())
	{
		WARNING(true, ("Tried to delete denied action group [%d] when does not exist for this state.", actionGroupIndex));
		return;
	}

	//-- Remove the specified group name.
	IGNORE_RETURN(m_deniedActionGroupIndices->erase(findIt));
}

// ----------------------------------------------------------------------

void EditableAnimationState::addReferencedLogicalAnimationNames(CrcLowerStringSet &set) const
{
	//-- Add state's loop logical animation name.
	// @todo fix up the CrcLowerString usage here.
	if (*(m_logicalAnimationName.getString()) != 0)
		IGNORE_RETURN(set.insert(CrcLowerString(m_logicalAnimationName.getString())));

	//-- Add actions' referenced logical animation names.
	if (m_actions)
	{
		const EditableAnimationActionVector::iterator endIt = m_actions->end();
		for (EditableAnimationActionVector::iterator it = m_actions->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->addReferencedLogicalAnimationNames(set);
		}
	}

	//-- Write links' referenced logical animations.
	{
		const EditableAnimationStateLinkVector::const_iterator endIt = m_stateLinks.end();
		for (EditableAnimationStateLinkVector::const_iterator it = m_stateLinks.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->addReferencedLogicalAnimationNames(set);
		}
	}

	//-- Get referenced animation names from child states.
	{
		const EditableAnimationStateVector::const_iterator endIt = m_childStates.end();
		for (EditableAnimationStateVector::const_iterator it = m_childStates.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->addReferencedLogicalAnimationNames(set);
		}
	}

}

// ----------------------------------------------------------------------

void EditableAnimationState::write(Iff &iff) const
{
	iff.insertForm(TAG_STAT);

		// Write general data.
		iff.insertChunk(TAG_INFO);

			// State name.
			NOT_NULL(m_stateNameId);
			iff.insertChunkString(AnimationStateNameIdManager::getNameString(*m_stateNameId).getString());

			// Loop animation.
			iff.insertChunkString(m_logicalAnimationName.getString());

		iff.exitChunk(TAG_INFO);

		// Write actions if present.
		if (m_actions && !m_actions->empty())
		{
			iff.insertForm(TAG_ACTS);

				// write # actions.
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int16>(m_actions->size()));
				iff.exitChunk(TAG_INFO);

				// write actions
				const EditableAnimationActionVector::iterator endIt = m_actions->end();
				for (EditableAnimationActionVector::iterator it = m_actions->begin(); it != endIt; ++it)
				{
					NOT_NULL(*it);
					(*it)->write(iff);
				}

			iff.exitForm(TAG_ACTS);
		}

		// Write granted action groups.
		if (m_grantedActionGroupIndices)
			writeActionGroup(iff, *m_grantedActionGroupIndices, TAG_GAGR);

		// Write denied action groups.
		if (m_deniedActionGroupIndices)
			writeActionGroup(iff, *m_deniedActionGroupIndices, TAG_DAGR);

		// Write links.
		if (!m_stateLinks.empty())
		{
			iff.insertForm(TAG_LNKS);

				// Write # links.
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int16>(m_stateLinks.size()));
				iff.exitChunk(TAG_INFO);

				// Write each link.
				const EditableAnimationStateLinkVector::const_iterator endIt = m_stateLinks.end();
				for (EditableAnimationStateLinkVector::const_iterator it = m_stateLinks.begin(); it != endIt; ++it)
				{
					NOT_NULL(*it);
					(*it)->write(iff);
				}

			iff.exitForm(TAG_LNKS);
		}

		// Write children.
		if (!m_childStates.empty())
		{
			//-- Ensure that children get written out sorted properly.
			std::sort(const_cast<EditableAnimationState*>(this)->m_childStates.begin(), const_cast<EditableAnimationState*>(this)->m_childStates.end(), LessNameComparator());

			iff.insertForm(TAG_CHLD);

				// Write # children
				iff.insertChunk(TAG_INFO);
					iff.insertChunkData(static_cast<int16>(m_childStates.size()));
				iff.exitChunk(TAG_INFO);

				// Write children.
				const EditableAnimationStateVector::const_iterator endIt = m_childStates.end();
				for (EditableAnimationStateVector::const_iterator it = m_childStates.begin(); it != endIt; ++it)
				{
					NOT_NULL(*it);
					(*it)->write(iff);
				}

			iff.exitForm(TAG_CHLD);
		}

	iff.exitForm(TAG_STAT);
}

// ----------------------------------------------------------------------

void EditableAnimationState::writeXml(IndentedFileWriter &writer) const
{
	writer.writeLineFormat("<idleAnimation logicalName='%s'/>", getLogicalAnimationName().getString());

	writeXmlActions(writer);
	writeXmlLinks(writer);
	writeXmlChildStates(writer);
}

// ======================================================================
// class EditableAnimationState: private member functions
// ======================================================================

void EditableAnimationState::buildStatePath(AnimationStatePath &path) const
{
	//-- get parent to append it's portion of the path
	if (m_parentState)
		m_parentState->buildStatePath(path);
	else
	{
		//-- this state is the root of the state path.  clear out path.
		path.clearPath();

		//-- if this state is the "root" state, skip it
		NOT_NULL(m_stateNameId);
#if 0
		if (*m_stateNameId == AnimationStateNameIdManager::getRootId()) //lint !e1702 // operator== both ordinary and member function // look into this.
			return;
#endif
	}

	//-- append this state name id to the path
	NOT_NULL(m_stateNameId);
	path.appendState(*m_stateNameId);
}

// ----------------------------------------------------------------------

void EditableAnimationState::writeActionGroup(Iff &iff, const IntVector &actionGroupIndices, const Tag &destTag) const
{
	if (!actionGroupIndices.empty())
	{
		iff.insertChunk(destTag);

			// Write # groups.
			iff.insertChunkData(static_cast<int16>(actionGroupIndices.size()));

			// Write group indices.
			const std::vector<int>::const_iterator endIt = actionGroupIndices.end();
			for (std::vector<int>::const_iterator it = actionGroupIndices.begin(); it != endIt; ++it)
			{
				iff.insertChunkData(static_cast<int16>(*it));
			}

		iff.exitChunk(destTag);
	}
}

// ----------------------------------------------------------------------

void EditableAnimationState::load_0000(Iff &iff, int versionNumber)
{
	char buffer[1024];

	//-- Get state name and logical animation name index
	iff.enterForm(TAG_STAT);

		iff.enterChunk(TAG_INFO);
		{
			// load state name id
			iff.read_string(buffer, sizeof(buffer) - 1);

			delete m_stateNameId;
			m_stateNameId = new AnimationStateNameId(AnimationStateNameIdManager::createId(CrcLowerString(buffer)));

			// load logical animation name
			const int logicalAnimationNameIndex   = static_cast<int>(iff.read_int16());
			CrcString const &logicalAnimationName = m_hierarchyTemplate.getLogicalAnimationName(logicalAnimationNameIndex);
			m_logicalAnimationName.set(logicalAnimationName.getString(), logicalAnimationName.getCrc());
		}
		iff.exitChunk(TAG_INFO);

		//-- Load locomotion animation data (and ignore.  REMOVE THIS CODE FROM VERSION 0001 and above).
		if (iff.enterChunk(TAG_LOCA, true))
		{
			IGNORE_RETURN(iff.read_int16());
			
			iff.exitChunk(TAG_LOCA);
		}

		//-- Load actions.
		if (iff.enterForm(TAG_ACTS, true))
		{
			// Get # actions.
			iff.enterChunk(TAG_INFO);
				const int count = static_cast<int>(iff.read_int16());
			iff.exitChunk(TAG_INFO);

			// Perpare actions array.
			if (m_actions)
			{
				std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
				m_actions->clear();
			}
			else
				m_actions = new EditableAnimationActionVector();

			m_actions->reserve(static_cast<EditableAnimationActionVector::size_type>(count));

			// Load actions.
			for (int i = 0; i < count; ++i)
			{
				const Tag actionType = iff.getCurrentName();
				
				switch (actionType)
				{
					case TAG_ACTN:
						m_actions->push_back(new EditableBasicAnimationAction(m_hierarchyTemplate, iff, versionNumber));
						break;

					default:
						{
							ConvertTagToString(actionType, buffer);
							DEBUG_FATAL(true, ("unsupported action type [%s].", buffer));
						}
				}
			}

			iff.exitForm(TAG_ACTS);
		}

		//-- Load granted action groups.
		if (iff.enterChunk(TAG_GAGR, true))
		{
			// Get # granted action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid granted action group count [%d].", count));

			// Setup storage.
			if (m_grantedActionGroupIndices)
				m_grantedActionGroupIndices->clear();
			else
				m_grantedActionGroupIndices = new IntVector();

			m_grantedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load granted action group names.
			for (int i = 0; i < count; ++i)
			{
				m_grantedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_GAGR);
		}

		//-- Load denied action groups.
		if (iff.enterChunk(TAG_DAGR, true))
		{
			// Get # denied action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid denied action group count [%d].", count));

			// Setup storage.
			if (m_deniedActionGroupIndices)
				m_deniedActionGroupIndices->clear();
			else
				m_deniedActionGroupIndices = new IntVector();

			m_deniedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load denied action group names.
			for (int i = 0; i < count; ++i)
			{
				m_deniedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_DAGR);
		}

		//-- Load links.
		if (iff.enterForm(TAG_LNKS, true))
		{
			// Get # links.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid state link count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_stateLinks.reserve(static_cast<size_t>(count));
			
			// Load links.
			for (int i = 0; i < count; ++i)
				m_stateLinks.push_back(new EditableAnimationStateLink(m_hierarchyTemplate, iff, versionNumber));

			iff.exitForm(TAG_LNKS);
		}

		//-- Load children.
		if (iff.enterForm(TAG_CHLD, true))
		{
			// Get # children.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid child count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_childStates.reserve(static_cast<size_t>(count));

			// Load children.
			for (int i = 0; i < count; ++i)
			{
				// Create the state.
				EditableAnimationState *const childState = new EditableAnimationState(m_hierarchyTemplate, iff, versionNumber);

				// Add to list of children.
				m_childStates.push_back(childState);

				// Set the state's parent to this state.
				childState->setParentState(this);
			}

			iff.exitForm(TAG_CHLD);
		}

	iff.exitForm(TAG_STAT);
}

// ----------------------------------------------------------------------

void EditableAnimationState::load_0001(Iff &iff, int versionNumber)
{
	char buffer[1024];

	//-- Get state name and logical animation name index
	iff.enterForm(TAG_STAT);

		iff.enterChunk(TAG_INFO);
		{
			// load state name id
			iff.read_string(buffer, sizeof(buffer) - 1);

			delete m_stateNameId;
			m_stateNameId = new AnimationStateNameId(AnimationStateNameIdManager::createId(CrcLowerString(buffer)));

			// load logical animation name
			const int        logicalAnimationNameIndex = static_cast<int>(iff.read_int16());
			CrcString const &logicalAnimationName      = m_hierarchyTemplate.getLogicalAnimationName(logicalAnimationNameIndex);
			m_logicalAnimationName.set(logicalAnimationName.getString(), logicalAnimationName.getCrc());
		}
		iff.exitChunk(TAG_INFO);

		//-- Load locomotion animation data (and ignore.  REMOVE THIS CODE FROM VERSION 0001 and above).
		if (iff.enterChunk(TAG_LOCA, true))
		{
			IGNORE_RETURN(iff.read_int16());
			
			iff.exitChunk(TAG_LOCA);
		}

		//-- Load actions.
		if (iff.enterForm(TAG_ACTS, true))
		{
			// Get # actions.
			iff.enterChunk(TAG_INFO);
				const int count = static_cast<int>(iff.read_int16());
			iff.exitChunk(TAG_INFO);

			// Perpare actions array.
			if (m_actions)
			{
				std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
				m_actions->clear();
			}
			else
				m_actions = new EditableAnimationActionVector();

			m_actions->reserve(static_cast<EditableAnimationActionVector::size_type>(count));

			// Load actions.
			for (int i = 0; i < count; ++i)
			{
				const Tag actionType = iff.getCurrentName();
				
				switch (actionType)
				{
					case TAG_ACTN:
						m_actions->push_back(new EditableBasicAnimationAction(m_hierarchyTemplate, iff, versionNumber));
						break;

					case TAG_MVAC:
						m_actions->push_back(new EditableMovementAnimationAction(m_hierarchyTemplate, iff, versionNumber));
						break;

					default:
						{
							ConvertTagToString(actionType, buffer);
							FATAL(true, ("Unsupported action type [%s].", buffer));
						}
				}
			}

			iff.exitForm(TAG_ACTS);
		}

		//-- Load granted action groups.
		if (iff.enterChunk(TAG_GAGR, true))
		{
			// Get # granted action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid granted action group count [%d].", count));

			// Setup storage.
			if (m_grantedActionGroupIndices)
				m_grantedActionGroupIndices->clear();
			else
				m_grantedActionGroupIndices = new IntVector();

			m_grantedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load granted action group names.
			for (int i = 0; i < count; ++i)
			{
				m_grantedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_GAGR);
		}

		//-- Load denied action groups.
		if (iff.enterChunk(TAG_DAGR, true))
		{
			// Get # denied action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid denied action group count [%d].", count));

			// Setup storage.
			if (m_deniedActionGroupIndices)
				m_deniedActionGroupIndices->clear();
			else
				m_deniedActionGroupIndices = new IntVector();

			m_deniedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load denied action group names.
			for (int i = 0; i < count; ++i)
			{
				m_deniedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_DAGR);
		}

		//-- Load links.
		if (iff.enterForm(TAG_LNKS, true))
		{
			// Get # links.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid state link count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_stateLinks.reserve(static_cast<size_t>(count));
			
			// Load links.
			for (int i = 0; i < count; ++i)
			{
				m_stateLinks.push_back(new EditableAnimationStateLink(m_hierarchyTemplate, iff, versionNumber));
			}

			iff.exitForm(TAG_LNKS);
		}

		//-- Load children.
		if (iff.enterForm(TAG_CHLD, true))
		{
			// Get # children.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid child count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_childStates.reserve(static_cast<size_t>(count));

			// Load children.
			for (int i = 0; i < count; ++i)
			{
				// Create the state.
				EditableAnimationState *const childState = new EditableAnimationState(m_hierarchyTemplate, iff, versionNumber);

				// Add to list of children.
				m_childStates.push_back(childState);

				// Set the state's parent to this state.
				childState->setParentState(this);
			}

			iff.exitForm(TAG_CHLD);
		}

	iff.exitForm(TAG_STAT);
}

// ----------------------------------------------------------------------

void EditableAnimationState::load_0002(Iff &iff, int versionNumber)
{
	char buffer[1024];

	//-- Get state name and logical animation name index
	iff.enterForm(TAG_STAT);

		iff.enterChunk(TAG_INFO);
		{
			// load state name id
			iff.read_string(buffer, sizeof(buffer) - 1);

			delete m_stateNameId;
			m_stateNameId = new AnimationStateNameId(AnimationStateNameIdManager::createId(CrcLowerString(buffer)));

			// load logical animation name
			iff.read_string(buffer, sizeof(buffer) - 1);
			m_logicalAnimationName.set(buffer, true);
		}
		iff.exitChunk(TAG_INFO);

		//-- Load actions.
		if (iff.enterForm(TAG_ACTS, true))
		{
			// Get # actions.
			iff.enterChunk(TAG_INFO);
				const int count = static_cast<int>(iff.read_int16());
			iff.exitChunk(TAG_INFO);

			// Perpare actions array.
			if (m_actions)
			{
				std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
				m_actions->clear();
			}
			else
				m_actions = new EditableAnimationActionVector();

			m_actions->reserve(static_cast<EditableAnimationActionVector::size_type>(count));

			// Load actions.
			for (int i = 0; i < count; ++i)
			{
				const Tag actionType = iff.getCurrentName();
				
				switch (actionType)
				{
					case TAG_ACTN:
						m_actions->push_back(new EditableBasicAnimationAction(m_hierarchyTemplate, iff, versionNumber));
						break;

					case TAG_MVAC:
						m_actions->push_back(new EditableMovementAnimationAction(m_hierarchyTemplate, iff, versionNumber));
						break;

					default:
						{
							ConvertTagToString(actionType, buffer);
							FATAL(true, ("Unsupported action type [%s].", buffer));
						}
				}
			}

			iff.exitForm(TAG_ACTS);
		}

		//-- Load granted action groups.
		if (iff.enterChunk(TAG_GAGR, true))
		{
			// Get # granted action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid granted action group count [%d].", count));

			// Setup storage.
			if (m_grantedActionGroupIndices)
				m_grantedActionGroupIndices->clear();
			else
				m_grantedActionGroupIndices = new IntVector();

			m_grantedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load granted action group names.
			for (int i = 0; i < count; ++i)
			{
				m_grantedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_GAGR);
		}

		//-- Load denied action groups.
		if (iff.enterChunk(TAG_DAGR, true))
		{
			// Get # denied action groups.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid denied action group count [%d].", count));

			// Setup storage.
			if (m_deniedActionGroupIndices)
				m_deniedActionGroupIndices->clear();
			else
				m_deniedActionGroupIndices = new IntVector();

			m_deniedActionGroupIndices->reserve(static_cast<IntVector::size_type>(count));

			// Load denied action group names.
			for (int i = 0; i < count; ++i)
			{
				m_deniedActionGroupIndices->push_back(static_cast<int>(iff.read_int16()));
			}

			iff.exitChunk(TAG_DAGR);
		}

		//-- Load links.
		if (iff.enterForm(TAG_LNKS, true))
		{
			// Get # links.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid state link count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_stateLinks.reserve(static_cast<size_t>(count));
			
			// Load links.
			for (int i = 0; i < count; ++i)
			{
				m_stateLinks.push_back(new EditableAnimationStateLink(m_hierarchyTemplate, iff, versionNumber));
			}

			iff.exitForm(TAG_LNKS);
		}

		//-- Load children.
		if (iff.enterForm(TAG_CHLD, true))
		{
			// Get # children.
			iff.enterChunk(TAG_INFO);

				const int count = static_cast<int>(iff.read_int16());
				DEBUG_FATAL(count < 0, ("invalid child count [%d].", count));

			iff.exitChunk(TAG_INFO);

			// Prepare data storage.
			m_childStates.reserve(static_cast<size_t>(count));

			// Load children.
			for (int i = 0; i < count; ++i)
			{
				// Create the state.
				EditableAnimationState *const childState = new EditableAnimationState(m_hierarchyTemplate, iff, versionNumber);

				// Add to list of children.
				m_childStates.push_back(childState);

				// Set the state's parent to this state.
				childState->setParentState(this);
			}

			iff.exitForm(TAG_CHLD);
		}

	iff.exitForm(TAG_STAT);
}

// ----------------------------------------------------------------------

void EditableAnimationState::loadXml_1(XmlTreeNode const &treeNode)
{
	FATAL(treeNode.isNull(), ("loadXml_1(): attempting to initialize a state with a NULL node."));

	int const versionNumber = 1;

	//-- Setup state name.
	treeNode.getElementAttributeAsString(cs_nameAttributeName, s_stateName);
	FATAL(s_stateName.empty(), ("loadXml_1(): attribute name is empty."));

	// @todo fixup AnimationStateNameIdManager to accept CrcString rather than CrcLowerString.
	delete m_stateNameId;
	m_stateNameId = new AnimationStateNameId(AnimationStateNameIdManager::createId(CrcLowerString(s_stateName.c_str())));

	//-- Get idle animation info.
	XmlTreeNode idleAnimationNode(treeNode.getFirstChildElementNode());
	FATAL(
		idleAnimationNode.isNull() || _stricmp(idleAnimationNode.getName(), cs_idleAnimationElementName),
		("loadXml_1(): was expecting element [%s], found [%s].",
		cs_idleAnimationElementName,
		idleAnimationNode.isNull() ? "<null node>" : idleAnimationNode.getName()
		));

	idleAnimationNode.getElementAttributeAsString(cs_logicalNameAttributeName, s_logicalName);
	m_logicalAnimationName.set(s_logicalName.c_str(), true);

	//-- Handle state actions.
	XmlTreeNode nextNode(idleAnimationNode.getNextSiblingElementNode());
	XmlTreeNode stateActionsNode(nextNode);

	if (!stateActionsNode.isNull() && !_stricmp(stateActionsNode.getName(), cs_stateActionsElementName))
	{
		IS_NULL(m_actions);
		m_actions = new EditableAnimationActionVector();

		for (XmlTreeNode actionTypeNode = stateActionsNode.getFirstChildElementNode(); !actionTypeNode.isNull(); actionTypeNode = actionTypeNode.getNextSiblingElementNode())
		{
			char const *const actionTypeName = actionTypeNode.getName();
			if (!_stricmp(actionTypeName, cs_basicActionElementName))
				m_actions->push_back(new EditableBasicAnimationAction(m_hierarchyTemplate, actionTypeNode, versionNumber));
			else if (!_stricmp(actionTypeName, cs_movementAwareActionElementName))
				m_actions->push_back(new EditableMovementAnimationAction(m_hierarchyTemplate, actionTypeNode, versionNumber));
			else
				FATAL(true, ("loadXml_1(): expecting element [%s] or [%s], found [%s].", cs_basicActionElementName, cs_movementAwareActionElementName, actionTypeName));
		}

		// Move next node forward.
		nextNode = nextNode.getNextSiblingElementNode();
	}

	//-- Handle state links.
	XmlTreeNode stateLinksNode(nextNode);
	if (!stateLinksNode.isNull() && !_stricmp(stateLinksNode.getName(), cs_stateLinksElementName))
	{
		for (XmlTreeNode linkNode = stateLinksNode.getFirstChildElementNode(); !linkNode.isNull(); linkNode = linkNode.getNextSiblingElementNode())
		{
			FATAL(_stricmp(linkNode.getName(), cs_stateLinkElementName), ("loadXml_1(): expecting element [%s], found [%s].", cs_stateLinkElementName, linkNode.getName()));
			m_stateLinks.push_back(new EditableAnimationStateLink(m_hierarchyTemplate, linkNode, versionNumber));
		}

		// Move next node forward.
		nextNode = nextNode.getNextSiblingElementNode();
	}

	//-- Handle child states.
	XmlTreeNode childStatesNode(nextNode);
	if (!childStatesNode.isNull() && !_stricmp(childStatesNode.getName(), cs_childStatesElementName))
	{
		for (XmlTreeNode childNode = childStatesNode.getFirstChildElementNode(); !childNode.isNull(); childNode = childNode.getNextSiblingElementNode())
		{
			FATAL(_stricmp(childNode.getName(), cs_stateElementName), ("loadXml_1(): expecting element [%s], found [%s].", cs_stateElementName, childNode.getName()));

			// Create the child from the node.
			EditableAnimationState *const childState = new EditableAnimationState(m_hierarchyTemplate, childNode, versionNumber);

			// Add to list of children.
			m_childStates.push_back(childState);

			// Set the state's parent to this state.
			childState->setParentState(this);
		}

		//-- Sort child states by state name in CRC order.
		std::sort(m_childStates.begin(), m_childStates.end(), LessNameComparator());
	}
}

// ----------------------------------------------------------------------

void EditableAnimationState::writeXmlActions(IndentedFileWriter &writer) const
{
	int const actionCount = getActionCount();
	if (actionCount <= 0)
		return;

	writer.writeLine("<stateActions>");
	writer.indent();
	{
		for (int actionIndex = 0; actionIndex < actionCount; ++actionIndex)
		{
			// Get the action.
			EditableAnimationAction const *const action = dynamic_cast<EditableAnimationAction const*>(&getConstAction(actionIndex));
			if (!action)
			{
				WARNING(true, ("EditableAnimationState::writeXmlActions(): failed to write action index [%d] of state [%s]: action was NULL after dynamic cast.", actionIndex, getStateName().getString()));
				continue;
			}

			// Write it.
			action->writeXml(writer);
		}
	}
	writer.unindent();
	writer.writeLine("</stateActions>");
}

// ----------------------------------------------------------------------

void EditableAnimationState::writeXmlLinks(IndentedFileWriter &writer) const
{
	//-- Skip this if no links exist.
	int const linkCount = getLinkCount();
	if (linkCount <= 0)
		return;

	//-- Write the links.
	writer.writeLine("<stateLinks>");
	writer.indent();
	{
		for (int linkIndex = 0; linkIndex < linkCount; ++linkIndex)
		{
			// Ensure link is the right type.
			EditableAnimationStateLink const *const link = dynamic_cast<EditableAnimationStateLink const*>(&getConstLink(linkIndex));
			if (!link)
			{
				WARNING(true, ("EditableAnimationState::writeXmlLinks(): failed to write link index [%d] of state [%s]: link was NULL after dynamic cast.", linkIndex, getStateName().getString()));
				continue;
			}

			// Write the link.
			link->writeXml(writer);
		}
	}
	writer.unindent();
	writer.writeLine("</stateLinks>");
}

// ----------------------------------------------------------------------

void EditableAnimationState::writeXmlChildStates(IndentedFileWriter &writer) const
{
	int const childCount = getChildStateCount();
	if (childCount <= 0)
		return;

	//-- Write out state names in ABC order for ease in XML browsing/editing.

	// Do the sort.
	typedef std::map<std::string, EditableAnimationState const*>  NameStateMap;
	NameStateMap  stateMap;
	{
		for (int childIndex = 0; childIndex < childCount; ++childIndex)
		{
			EditableAnimationState const *const childState = dynamic_cast<EditableAnimationState const*>(&getConstChildState(childIndex));
			if (!childState)
			{
				WARNING(true, ("EditableAnimationState::writeXmlChildStates(): failed to write child index [%d] of state [%s]: child state was NULL after dynamic cast.", childIndex, getStateName().getString()));
				continue;
			}

			std::pair<NameStateMap::iterator, bool> const insertResult = stateMap.insert(NameStateMap::value_type(childState->getStateName().getString(), childState));
			DEBUG_WARNING(!insertResult.second, ("Failed to insert state [%s], most probable cause is multiple state entries for the same name.", childState->getStateName().getString()));
			UNREF(insertResult);
		}
	}	

	// Write them out.
	writer.writeLine("<childStates>");
	writer.indent();
	{
		NameStateMap::iterator const endIt = stateMap.end();
		for (NameStateMap::iterator it = stateMap.begin(); it != endIt; ++it)
		{
			EditableAnimationState const *const childState = it->second;
			NOT_NULL(childState);

			writer.writeLineFormat("<state name='%s'>", childState->getStateName().getString());
			writer.indent();
			{
				childState->writeXml(writer);
			}
			writer.unindent();
			writer.writeLine("</state>");
		}
	}
	writer.unindent();
	writer.writeLine("</childStates>");
}

// ======================================================================
