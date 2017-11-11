// ======================================================================
//
// CuiModifierManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiModifierManager.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "clientGame/Game.h"

#include "StringId.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace CuiModifierManagerNamespace
{
	bool s_installed;

	const std::string s_tableName("datatables/modifiers/modifiers.iff");

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiModifierManager::Messages::ModifierAdded::Payload &, CuiModifierManager::Messages::ModifierAdded> s_modifierAdded;
		MessageDispatch::Transceiver<const CuiModifierManager::Messages::ModifierRemoved::Payload &, CuiModifierManager::Messages::ModifierRemoved> s_modifierRemoved;
		MessageDispatch::Transceiver<bool, CuiModifierManager::Messages::AllModifiersRemoved> s_allModifiersRemoved;
	}

	struct ModifierNode
	{
		ModifierNode()
		 : m_name()
		 , m_crc(0)
		 , m_validModifier(false)
		 , m_childList()
		{
		}

		typedef std::map<CuiModifierManager::ModId, ModifierNode> ChildList;

		std::string m_name;
		uint32 m_crc;
		bool m_validModifier;
		ChildList m_childList;
	};

	ModifierNode s_modifierTree;
	CuiModifierManager::ModIdList s_modifierChildren;

	void addModifierNode(ModifierNode &parentNode, std::string const &modifierName, const int depth);
	void getModifierNodes(const CuiModifierManager::ModId modId, const ModifierNode &parentNode, const bool parentFound);

#if PRODUCTION == 0
	void printModifierTree(ModifierNode const &parentNode, const int depth);
#endif // PRODUCTION == 0
}

using namespace CuiModifierManagerNamespace;

//-----------------------------------------------------------------

void CuiModifierManagerNamespace::addModifierNode(ModifierNode &parentNode, std::string const &modifierName, const int depth)
{
	// Find the correct position depending on our depth

	int position = std::string::npos;
	
	for (int i = 0; i < depth; ++i)
	{
		position = modifierName.find(".", ++position);
	}

	if (position != std::string::npos)
	{
		// This is a parent node

		std::string parsedName(modifierName.substr(0, position));

		if (!parsedName.empty())
		{
			ModifierNode modifierNode;
			modifierNode.m_name = parsedName;
			modifierNode.m_crc = Crc::calculate(parsedName.c_str());
			modifierNode.m_validModifier = false;

			std::pair<ModifierNode::ChildList::iterator, bool> result = parentNode.m_childList.insert(std::make_pair(modifierNode.m_crc, modifierNode));

			// Keep searching for any children of this parent

			addModifierNode(result.first->second, modifierName, depth + 1);
		}
	}
	else
	{
		// This is a child node

		ModifierNode modifierNode;
		modifierNode.m_name = modifierName;
		modifierNode.m_crc = Crc::calculate(modifierName.c_str());
		modifierNode.m_validModifier = true;

		std::pair<ModifierNode::ChildList::iterator, bool> result = parentNode.m_childList.insert(std::make_pair(modifierNode.m_crc, modifierNode));
		UNREF(result);

#if PRODUCTION == 0
		if (!result.second)
		{
			DEBUG_WARNING(true, ("CRC collision occured inserting modifier(%s) crc(%u). Dumping full CRC list so you can tell what else has the same CRC.", modifierNode.m_name, modifierNode.m_crc));

			printModifierTree(s_modifierTree, 0);
		}
#endif // PRODUCTION == 0
	}
}

#if PRODUCTION == 0
//-----------------------------------------------------------------

void CuiModifierManagerNamespace::printModifierTree(ModifierNode const &parentNode, const int depth)
{
	if (depth == 0)
	{
		DEBUG_REPORT_LOG(true, ("Modifier tree entries are in CRC order\n"));
	}

	std::string tabs;

	for (int i = 0; i < depth; ++i)
	{
		tabs.append("\t");
	}

	DEBUG_REPORT_LOG(true, ("%d%s%s(%u)\n", depth, tabs.c_str(), parentNode.m_name.empty() ? "root" : parentNode.m_name.c_str(), Crc::calculate(parentNode.m_name.c_str())));

	ModifierNode::ChildList::const_iterator iterChildList = parentNode.m_childList.begin();

	for (; iterChildList != parentNode.m_childList.end(); ++iterChildList)
	{
		printModifierTree((*iterChildList).second, depth + 1);
	}
}
#endif // PRODUCTION == 0

//-----------------------------------------------------------------

void CuiModifierManagerNamespace::getModifierNodes(const CuiModifierManager::ModId modId, const ModifierNode &parentNode, const bool parentFound)
{
	if (   parentFound
	    && parentNode.m_validModifier)
	{
		s_modifierChildren.push_back(parentNode.m_crc);
	}

	ModifierNode::ChildList::const_iterator iterChildList = parentNode.m_childList.begin();

	for (; iterChildList != parentNode.m_childList.end(); ++iterChildList)
	{
		if (parentFound)
		{
			// Parent found somewhere above this branch, keep collecting leaves

			getModifierNodes(modId, iterChildList->second, true);
		}
		else
		{
			const CuiModifierManager::ModId currentModId = iterChildList->first;

			if (currentModId == modId)
			{
				// Parent found, this is the last branch we are going down

				getModifierNodes(modId, iterChildList->second, true);
				break;
			}
			else
			{
				// No parent found, keep searching the tree

				getModifierNodes(modId, iterChildList->second, false);
			}
		}
	}
}

// ======================================================================
//
// CuiModifierManager
//
// ======================================================================

CuiModifierManager::CurrentModifiers * CuiModifierManager::ms_currentModifierMap;
CuiModifierManager::ModifiersInfo * CuiModifierManager::ms_modifiersInfo;

//-----------------------------------------------------------------

void CuiModifierManager::install ()
{
	DEBUG_FATAL (s_installed, ("CuiModifierManager::install: already installed.\n"));
	if(s_installed)
		return;

	ms_currentModifierMap = new CurrentModifiers;
	ms_modifiersInfo = new ModifiersInfo;
	s_installed  = true;
	loadData();
}

//-----------------------------------------------------------------

void CuiModifierManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("CuiModifierManager::remove: not installed.\n"));
	if(!s_installed)
		return;

	delete ms_currentModifierMap;
	ms_currentModifierMap = NULL;

	if(ms_modifiersInfo)
		std::for_each (ms_modifiersInfo->begin (), ms_modifiersInfo->end (), PointerDeleterPairSecond ());
	delete ms_modifiersInfo;
	ms_modifiersInfo = NULL;
	s_installed = false;
}

//-----------------------------------------------------------------

void CuiModifierManager::update (float deltaTime)
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::update: not installed"));
	if(!s_installed || !ms_currentModifierMap)
		return;

	for(CurrentModifiers::iterator i = ms_currentModifierMap->begin(); i != ms_currentModifierMap->end(); )
	{
		if (i->second != -1.0f)
		{
			i->second -= deltaTime;
			//If we have a modifier that is more that 5 seconds expired, and the server has not told us about it, expire it ourselves.
			if (i->second <= -5.0f)
			{
				CurrentModifiers::iterator oldIt = i++;
				removeModifier(oldIt->first);
			}
			else
			{
				// we need to make sure the time isn't mistaken to an infinite mod
				if (i->second == -1.0f)
					i->second = -1.1f;
				++i;
			}
		}
		else
			++i;
	}
}

//-----------------------------------------------------------------

void CuiModifierManager::loadData()
{
	DEBUG_FATAL (!s_installed, ("CuiModifierManager::loadData: not installed"));
	if(!s_installed || !ms_modifiersInfo)
		return;

	const DataTable * const fileTable = DataTableManager::getTable (s_tableName.c_str(), true);
	if (fileTable)
	{
		s_modifierTree.m_childList.clear();
		
		const int numRows = fileTable->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			const std::string & modName     = fileTable->getStringValue (0, i);
			const std::string & nameid      = fileTable->getStringValue (1, i);
			const std::string & descid      = fileTable->getStringValue (2, i);
			const std::string & iconstyle   = fileTable->getStringValue (3, i);

			const ModId crc     = getModId(modName.c_str());
			ModifierInfo * mod  = new ModifierInfo;
			mod->m_modId        = crc;
			mod->m_name         = StringId(nameid);
			mod->m_description  = StringId(descid);
			mod->m_iconStyle    = iconstyle;
			mod->m_internalName = modName;

			addModifierNode(s_modifierTree, modName, 1);

			IGNORE_RETURN(ms_modifiersInfo->insert(std::make_pair(crc, mod)));
		}
	}
}

//-----------------------------------------------------------------
/*
const CuiModifierManager::CurrentModifiers * CuiModifierManager::getCurrentModifierMap()
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::getCurrentModifierMap: not installed"));
	if(!s_installed)
		return NULL;

	return ms_currentModifierMap;
}
*/
//-----------------------------------------------------------------

void CuiModifierManager::addModifier(CuiModifierManager::ModId modId, float duration)
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::addModifier: not installed"));
	if(!s_installed || !ms_currentModifierMap)
		return;

	(*ms_currentModifierMap)[modId] = duration;
	Transceivers::s_modifierAdded.emitMessage (modId);
}

//-----------------------------------------------------------------

void CuiModifierManager::removeModifier(CuiModifierManager::ModId modId)
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::removeModifier: not installed"));
	if(!s_installed || !ms_currentModifierMap)
		return;

	ms_currentModifierMap->erase(modId);
	Transceivers::s_modifierRemoved.emitMessage (modId);
}

//-----------------------------------------------------------------

void CuiModifierManager::removeAllModifiers()
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::removeAllModifiers: not installed"));
	if(!s_installed || !ms_currentModifierMap)
		return;

	ms_currentModifierMap->clear();
	Transceivers::s_allModifiersRemoved.emitMessage(true);
}

//-----------------------------------------------------------------

/** Given a modifier Id, return the display information for it.  The works on any modifier, not just those
    currently active on the avatar.
 */
const CuiModifierManager::ModifierInfo * CuiModifierManager::getModifierInfo(CuiModifierManager::ModId modId)
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::getModifierInfo: not installed"));
	if(!s_installed || !ms_modifiersInfo)
		return NULL;

	ModifiersInfo::iterator i = ms_modifiersInfo->find(modId);
	if(i == ms_modifiersInfo->end())
		return NULL;
	else
	{
		ModifierInfo * m = i->second;
		return m;
	}
}

//-----------------------------------------------------------------

/** Get the remaining time for a modifier currently applied to a player.  If the given modifier is not
    on the avatar, return 0.0f.
 *
float CuiModifierManager::getRemainingDuration(ModId modId)
{
	DEBUG_FATAL(!s_installed, ("CuiModifierManager::addModifier: not installed"));
	if(!s_installed || !ms_currentModifierMap)
		return 0.0f;

	CurrentModifiers::iterator i = ms_currentModifierMap->find(modId);
	if(i != ms_currentModifierMap->end())
			return i->second;
	else
		return 0.0f;
}
*/
//-----------------------------------------------------------------

CuiModifierManager::ModId CuiModifierManager::getModId(const std::string& internalName)
{
	return Crc::calculate(internalName.c_str());
}

//-----------------------------------------------------------------

bool CuiModifierManager::getModifierList(const ModId modId, ModIdList &modifierChildren)
{
	s_modifierChildren.clear();
	getModifierNodes(modId, s_modifierTree, false);

	modifierChildren = s_modifierChildren;

	return !modifierChildren.empty();
}

// ======================================================================

