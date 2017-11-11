// ======================================================================
//
// RoleIconManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/RoleIconManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "sharedFoundation/Exitchain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/MessageQueueChangeRoleIconChoice.h"
#include "sharedObject/Controller.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <vector>

// ======================================================================

namespace RoleIconManagerNamespace
{
	bool m_installed = false;

	DataTable *ms_roleIconDatatable = NULL;
	
	struct RoleIconRecord
	{
		int index;
		std::string roleIcon;
		std::string qualifyingSkill;
	};

	const std::string ms_columnIndex("INDEX");
	const std::string ms_columnRoleIcon("ROLE_ICON");
	const std::string ms_columnQualifyingSkill("QUALIFYING_SKILL");
	
	std::vector<RoleIconRecord> ms_iconRecords;
	std::vector<int> ms_validIndices;	

	std::string emptyString;
	std::string NONE_STRING = "none";
};

using namespace RoleIconManagerNamespace;

// ======================================================================

void RoleIconManager::install()
{
	if(m_installed)
		return;
	m_installed = true;

	ms_roleIconDatatable = DataTableManager::getTable("datatables/role/role.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(ms_roleIconDatatable->getNumRows());
	int const indexColumn = ms_roleIconDatatable->findColumnNumber(ms_columnIndex);
	int const roleIconColumn = ms_roleIconDatatable->findColumnNumber(ms_columnRoleIcon);
	int const qualifyingSkillColumn = ms_roleIconDatatable->findColumnNumber(ms_columnQualifyingSkill);

	unsigned int i;
	int lastIndex = -1;
	for(i = 0; i < numRows; ++i)
	{
		RoleIconRecord roleIconRecord;

		roleIconRecord.index = ms_roleIconDatatable->getIntValue(indexColumn, i);
		roleIconRecord.roleIcon = ms_roleIconDatatable->getStringValue(roleIconColumn, i);
		roleIconRecord.qualifyingSkill = ms_roleIconDatatable->getStringValue(qualifyingSkillColumn, i);
		
		ms_iconRecords.push_back(roleIconRecord);
		
		if(roleIconRecord.index != lastIndex)
		{
			ms_validIndices.push_back(roleIconRecord.index);
			lastIndex = roleIconRecord.index;
		}
	}

	ExitChain::add(RoleIconManager::remove, "RoleIconManager::remove", 0, false);
}

// ----------------------------------------------------------------------

void RoleIconManager::remove()
{
	
}

// ----------------------------------------------------------------------

void RoleIconManager::cycleToNextQualifyingIcon(bool const includeWorkingSkill)
{
	PlayerObject const * const player = Game::getPlayerObject();
	CreatureObject * const creature = Game::getPlayerCreature();

	if (!player || !creature)
		return;

	int const curChoice = includeWorkingSkill ? 0 : player->getRoleIconChoice();
	int nextChoice = getNextPossibleIconChoice(curChoice);
	bool doesQualify = doesQualifyForIcon(creature, nextChoice, includeWorkingSkill);

	while (!doesQualify)
	{
		if (nextChoice == curChoice)
			return;

		nextChoice = getNextPossibleIconChoice(nextChoice);
		doesQualify = doesQualifyForIcon(creature, nextChoice, includeWorkingSkill);		
	}

	Controller * const controller = creature->getController();

	if (controller)				
	{
		MessageQueueChangeRoleIconChoice * const mqcric = new MessageQueueChangeRoleIconChoice(nextChoice, 0);
		uint32 const flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER;

		controller->appendMessage(CM_changeRoleIconChoice, 0.0f, mqcric, flags);
	}
}

// ----------------------------------------------------------------------

const std::string & RoleIconManager::getRoleIconName(int choice)
{
	for(std::vector<RoleIconRecord>::iterator i = ms_iconRecords.begin(); i != ms_iconRecords.end(); ++i)
	{
		const RoleIconRecord & record = *i;
		if(record.index == choice)
		{
			return record.roleIcon;
		}
	}
	//If none work, return none
	return NONE_STRING;
}

// ----------------------------------------------------------------------

bool RoleIconManager::doesQualifyForIcon(CreatureObject const * const creature, int const curChoice, bool const includeWorkingSkill)
{
	if (!creature)
		return false;

	if (creature->getMasterId() != NetworkId::cms_invalid)
	{
		for (std::vector<RoleIconRecord>::iterator i = ms_iconRecords.begin(); i != ms_iconRecords.end(); ++i)
		{
			RoleIconRecord const & record = *i;

			if (record.index == curChoice)
			{
				std::string const & skillName = record.qualifyingSkill;

				if (strncmp(skillName.c_str(), "pet_special", 11) == 0)
					return true;
			}
		}

		return false;			
	}

	PlayerObject const * const player = includeWorkingSkill ? Game::getPlayerObject() : 0;

	for (std::vector<RoleIconRecord>::iterator i = ms_iconRecords.begin(); i != ms_iconRecords.end(); ++i)
	{
		RoleIconRecord const & record = *i;

		if (record.index == curChoice)
		{
			std::string const & skillName = record.qualifyingSkill;
			SkillObject const * const skill = SkillManager::getInstance().getSkill(skillName);

			if (creature && skill && creature->hasSkill(*skill))
				return true;

			if (player && player->getWorkingSkill() == skillName)
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

int RoleIconManager::getNextPossibleIconChoice(int curChoice)
{
	std::vector<int>::iterator i = std::find(ms_validIndices.begin(), ms_validIndices.end(), curChoice);
	if(i == ms_validIndices.end())
	{
		i = ms_validIndices.begin();
	}
	else
	{
		++i;
		if(i == ms_validIndices.end())
			i = ms_validIndices.begin();
	}
	if(i != ms_validIndices.end())
	{
		return *i;
	}
	return 0;
}

// ----------------------------------------------------------------------

int RoleIconManager::getPetRoleIconChoice()
{
	static int petIndex = -1;
	if(petIndex == -1)
	{	
		for(std::vector<RoleIconRecord>::iterator i = ms_iconRecords.begin(); i != ms_iconRecords.end(); ++i)
		{
			const RoleIconRecord & record = *i;
			const std::string & skillName = record.qualifyingSkill;
			if(strncmp(skillName.c_str(), "pet_special", 11) == 0)
				petIndex = record.index;			
		}
	}
	return petIndex;
}

// ======================================================================
