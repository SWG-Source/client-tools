// ======================================================================
//
// CuiActionManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiActionManager.h"

#include "clientUserInterface/CuiAction.h"

#include <map>

// ======================================================================

namespace
{
	bool s_installed = false;
	const int c_coreActionIndex = -1;
	int s_currentIndex = c_coreActionIndex;
}

//-----------------------------------------------------------------

CuiActionManager::ActionMapMap * CuiActionManager::ms_actionMapMap = NULL;

//-----------------------------------------------------------------

void CuiActionManager::setCurrentActionMap(int index)
{
	s_currentIndex = index;
}

//-----------------------------------------------------------------

void CuiActionManager::resetActionMapIndex()
{
	setCurrentActionMap(c_coreActionIndex);
}

//-----------------------------------------------------------------

CuiActionManager::ActionMap * CuiActionManager::getActionMap(int index)
{
	ActionMapMap::iterator itAction = ms_actionMapMap->find(index);
	if (itAction != ms_actionMapMap->end())
	{
		return itAction->second;
	}

	return NULL;
}

//-----------------------------------------------------------------

CuiActionManager::ActionMap * CuiActionManager::getActionMap()
{
	ActionMap * actionMap = getActionMap(s_currentIndex);

	if (actionMap == NULL)
	{
		actionMap = createActionMap(s_currentIndex);
	}

	return actionMap;
}

//-----------------------------------------------------------------

void CuiActionManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));

	ms_actionMapMap = new ActionMapMap;

	IGNORE_RETURN(createActionMap(c_coreActionIndex));

	s_installed  = true;
}

//-----------------------------------------------------------------

CuiActionManager::ActionMap * CuiActionManager::createActionMap(int index)
{
	ActionMap * actionMap = new ActionMap;
	(*ms_actionMapMap)[index] = actionMap;
	return actionMap;
}


//-----------------------------------------------------------------

void CuiActionManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	//-----------------------------------------------------------------
	//-- this map is a many-to-one mapping, so we must be careful deleting the members
	for (ActionMapMap::iterator itMap = ms_actionMapMap->begin(); itMap != ms_actionMapMap->end(); ++itMap)
	{
		ActionMap * const actionMap = itMap->second;

		for (ActionMap::iterator it = actionMap->begin (); it != actionMap->end (); ++it)
		{
			ActionInfo & info = (*it).second;
			
			if (info.m_action)
			{
				ActionMap::iterator sit = it;			
				for (++sit; sit != actionMap->end(); ++sit)
				{
					//-- null it to keep from deleting it twice...
					if ((*sit).second.m_action == info.m_action)
						(*sit).second.m_action = 0;
				}
				
				if (info.m_own)
				{
					delete info.m_action;
					info.m_action = 0;
				}
			}
		}

		delete actionMap;
	}

	delete ms_actionMapMap;
	ms_actionMapMap = 0;
	
	s_installed = false;
}

//-----------------------------------------------------------------

/**
* Perform an action by id and value.
*
* @return true if an appropriate action was found AND the action claims to have performed the action.
*
*/

bool CuiActionManager::performAction (const std::string & id, const Unicode::String & params)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	const CuiAction * action = findAction (id, s_currentIndex);

	if (!action)
	{
		action = findAction (id, c_coreActionIndex);
	}

	return action ? action->performAction (id, params) : 0;
}

//-----------------------------------------------------------------

/**
* Find an action by id.
*
*/
const CuiAction * CuiActionManager::findAction    (const std::string & id)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	CuiAction const * action = findAction(id, s_currentIndex);

	if (!action)
	{
		action = findAction(id, c_coreActionIndex);
	}

	return action;
}

//-----------------------------------------------------------------

const CuiAction * CuiActionManager::findAction    (const std::string & id, int const index)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	ActionMapMap::iterator itAction = ms_actionMapMap->find(index);
	if (itAction != ms_actionMapMap->end())
	{
		const ActionMap::iterator it = itAction->second->find(id);
		
		if (it != itAction->second->end())
			return (*it).second.m_action;
	}
	
	return 0;
}

//-----------------------------------------------------------------

/**
*
* Remove an action from the manager.  If an action is returned, CuiActionManager 
* is no longer responsible for its deletion.
*/

CuiAction * CuiActionManager::removeAction  (const std::string & id)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	const ActionMap::iterator it = getActionMap()->find (id);
	
	if (it != getActionMap()->end ())
	{
		CuiAction * const action = (*it).second.m_action;
		getActionMap()->erase (it);
		return action;
	}

	return 0;
}

//-----------------------------------------------------------------

/**
* Remove this action and all its mappings.  The caller is responsible for deleting it.
*
* @return the number of times this action was removed
*/
int CuiActionManager::removeAction  (const CuiAction * action)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	int count = 0;

	//-----------------------------------------------------------------
	//-- this map is a many-to-one mapping, so we must be careful removing the elements.

	for (ActionMapMap::iterator itMap = ms_actionMapMap->begin(); itMap != ms_actionMapMap->end(); ++itMap)
	{
		ActionMap * const actionMap = itMap->second;

		for (ActionMap::iterator it = actionMap->begin (); it != actionMap->end (); /*++it*/)
		{
			if ((*it).second.m_action == action)
			{
				actionMap->erase (it++);
				++count;
			}
			else
				++it;
		}
	}


	return count;
}

//-----------------------------------------------------------------

/**
*
* If the action is added, the CuiActionManager is responsible for deleting it, unless
* it is removeAction()'d before the CuiActionManager is ::remove()'d.
*
* @return true if the action was added, false if an action already exists for this id.
*/

bool CuiActionManager::addAction     (const std::string & id, CuiAction * action, bool own, bool replace)
{
	UNREF(replace);

	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const std::pair<ActionMap::iterator, bool> retval = getActionMap()->insert (std::make_pair (id, ActionInfo (action, own)));;

	DEBUG_FATAL (s_currentIndex != c_coreActionIndex && !replace && !retval.second, ("attempt to add multiple CuiActions for the same id: %s\n", id.c_str()));

	return retval.second;
}

// ======================================================================

