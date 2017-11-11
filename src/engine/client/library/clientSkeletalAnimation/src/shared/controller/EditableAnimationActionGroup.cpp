// ======================================================================
//
// EditableAnimationActionGroup.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/EditableAnimationActionGroup.h"

#include "clientSkeletalAnimation/EditableBasicAnimationAction.h"
#include "clientSkeletalAnimation/EditableMovementAnimationAction.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

// ======================================================================

const std::string  EditableAnimationActionGroup::cms_newGroupNameBase("action_group");

const Tag TAG_ACTN = TAG(A,C,T,N);
const Tag TAG_AGRP = TAG(A,G,R,P);
const Tag TAG_MVAC = TAG(M,V,A,C);

int  EditableAnimationActionGroup::ms_newGroupCount;

// ======================================================================
// Inlines
// ======================================================================

inline void EditableAnimationActionGroup::load_0002(Iff &iff, int versionNumber)
{
	// Identical to version 0001.
	load_0001(iff, versionNumber);
}

// ======================================================================
/**
 * Constructor for loading from IFF data.
 */

EditableAnimationActionGroup::EditableAnimationActionGroup(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber) :
	AnimationActionGroup(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_name(0),
	m_actions(new EditableAnimationActionVector())
{
	//-- Load data from IFF.
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
			FATAL(true, ("EditableAnimationActionGroup version [%d] unsupported.", versionNumber));
	}

	//-- Create any uninitialized members.
	if (!m_name)
		m_name = new CrcLowerString(CrcLowerString::empty);
}

// ----------------------------------------------------------------------
/**
 * Constructor for creating from scratch.
 */

EditableAnimationActionGroup::EditableAnimationActionGroup(EditableAnimationStateHierarchyTemplate &hierarchyTemplate) :
	AnimationActionGroup(),
	m_hierarchyTemplate(hierarchyTemplate),
	m_name(new CrcLowerString(getNewGroupName().c_str())),
	m_actions(new EditableAnimationActionVector())
{
}

// ----------------------------------------------------------------------

EditableAnimationActionGroup::~EditableAnimationActionGroup()
{
	std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
	delete m_actions;

	delete m_name;
}

// ----------------------------------------------------------------------

const CrcLowerString &EditableAnimationActionGroup::getName() const
{
	return *m_name;
}

// ----------------------------------------------------------------------

int EditableAnimationActionGroup::getActionCount() const
{
	return static_cast<int>(m_actions->size());
}

// ----------------------------------------------------------------------

const AnimationAction &EditableAnimationActionGroup::getConstAction(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getActionCount());
	return *((*m_actions)[static_cast<EditableAnimationActionVector::size_type>(index)]);
}

// ----------------------------------------------------------------------

const AnimationAction *EditableAnimationActionGroup::findConstActionByName(const CrcLowerString &actionName) const
{
	const std::pair<EditableAnimationActionVector::iterator, EditableAnimationActionVector::iterator> result = std::equal_range(m_actions->begin(), m_actions->end(), actionName, AnimationAction::LessNameCrcOrderPointerComparator()); //lint !e64 // Lint is confused by return type.
	if (result.first != result.second)
	{
		// found it.
		return *result.first;
	}
	else
	{
		// doesn't exist.
		return 0;
	}
}

// ----------------------------------------------------------------------

void EditableAnimationActionGroup::setName(const CrcLowerString &name)
{
	*m_name = name;
}

// ----------------------------------------------------------------------

EditableAnimationAction &EditableAnimationActionGroup::getAction(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getActionCount());
	return *((*m_actions)[static_cast<EditableAnimationActionVector::size_type>(index)]);
}

// ----------------------------------------------------------------------

void EditableAnimationActionGroup::addAction(EditableAnimationAction *action)
{
	const EditableAnimationActionVector::iterator lowerBoundResult = std::lower_bound(m_actions->begin(), m_actions->end(), action, AnimationAction::LessNameCrcOrderPointerComparator());
	if ((lowerBoundResult == m_actions->end()) || AnimationAction::LessNameCrcOrderPointerComparator()(action, *lowerBoundResult))
	{
		//-- Action with same name doesn't exist, add this one.
		IGNORE_RETURN(m_actions->insert(lowerBoundResult, action));
	}
	else
	{
		WARNING(true, ("addAction(): action [%s] already exists in action group [%s].", action->getName().getString(), m_name->getString()));
	}
}

// ----------------------------------------------------------------------
/**
 * Remove, but do not delete, the specified action from the action group.
 *
 * @param action  the action to remove.
 */

void EditableAnimationActionGroup::removeAction(EditableAnimationAction *action)
{
	const std::pair<EditableAnimationActionVector::iterator, EditableAnimationActionVector::iterator> result = std::equal_range(m_actions->begin(), m_actions->end(), action, AnimationAction::LessNameCrcOrderPointerComparator()); //lint !e64 // Lint is confused by return type.
	if (result.first != result.second)
	{
		//-- Found match, remove action from list.
		IGNORE_RETURN(m_actions->erase(result.first));
	}
	else
	{
		WARNING(true, ("removeAction(): action [%s] is not an action owned by this group.", action->getName().getString()));
	}
}

// ----------------------------------------------------------------------

void EditableAnimationActionGroup::write(Iff &iff) const
{
	iff.insertForm(TAG_AGRP);

		// Specify group name and # actions.
		iff.insertChunk(TAG_INFO);
			iff.insertChunkString(m_name->getString());
			iff.insertChunkData(static_cast<int16>(m_actions->size()));
		iff.exitChunk(TAG_INFO);

		// Write actions
		const EditableAnimationActionVector::iterator endIt = m_actions->end();
		for (EditableAnimationActionVector::iterator it = m_actions->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->write(iff);
		}

	iff.exitForm(TAG_AGRP);
}

// ======================================================================

std::string EditableAnimationActionGroup::getNewGroupName()
{
	//-- Increment # created groups.
	++ms_newGroupCount;

	//-- Convert count to string.
	char numberBuffer[32];
	sprintf(numberBuffer, "%d", ms_newGroupCount);

	return cms_newGroupNameBase + numberBuffer;
}

// ======================================================================

void EditableAnimationActionGroup::load_0000(Iff &iff, int versionNumber)
{
	iff.enterForm(TAG_AGRP);

		//-- Load action group name and # actions.
		iff.enterChunk(TAG_INFO);

			//-- Load action group name.
			char name[1024];
			iff.read_string(name, sizeof(name) - 1);

			delete m_name;
			m_name = new CrcLowerString(name);

			//-- Load # actions.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid action count [%d].", count));

		iff.exitChunk(TAG_INFO);

		//-- Reserve space for actions.
		if (!m_actions->empty())
		{
			std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
			m_actions->clear();
		}
		m_actions->reserve(static_cast<EditableAnimationActionVector::size_type>(count));

		//-- Load actions.
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
						char buffer[5];

						ConvertTagToString(actionType, buffer);
						DEBUG_FATAL(true, ("unsupported action type [%s].", buffer));
					}
			}
		}

	iff.exitForm(TAG_AGRP);
}

// ----------------------------------------------------------------------

void EditableAnimationActionGroup::load_0001(Iff &iff, int versionNumber)
{
	iff.enterForm(TAG_AGRP);

		//-- Load action group name and # actions.
		iff.enterChunk(TAG_INFO);

			//-- Load action group name.
			char name[1024];
			iff.read_string(name, sizeof(name) - 1);

			delete m_name;
			m_name = new CrcLowerString(name);

			//-- Load # actions.
			const int count = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(count < 0, ("invalid action count [%d].", count));

		iff.exitChunk(TAG_INFO);

		//-- Reserve space for actions.
		if (!m_actions->empty())
		{
			std::for_each(m_actions->begin(), m_actions->end(), PointerDeleter());
			m_actions->clear();
		}
		m_actions->reserve(static_cast<EditableAnimationActionVector::size_type>(count));

		//-- Load actions.
		for (int i = 0; i < count; ++i)
		{
			const Tag actionType = iff.getCurrentName();
			
			switch (actionType)
			{
				case TAG_ACTN:
					m_actions->push_back(new EditableBasicAnimationAction(m_hierarchyTemplate, iff, versionNumber));
					break;

				// @todo add this.
				case TAG_MVAC:
					m_actions->push_back(new EditableMovementAnimationAction(m_hierarchyTemplate, iff, versionNumber));
					break;

				default:
					{
						char buffer[5];

						ConvertTagToString(actionType, buffer);
						DEBUG_FATAL(true, ("unsupported action type [%s].", buffer));
					}
			}
		}

	iff.exitForm(TAG_AGRP);
}

// ======================================================================
