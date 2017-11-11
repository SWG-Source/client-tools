//======================================================================
//
// ClientExpertiseManager.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientExpertiseManager.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedNetworkMessages/ExpertiseRequestMessage.h"
#include "sharedSkillSystem/ExpertiseManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>

//======================================================================

namespace ClientExpertiseManagerNamespace
{
	bool s_installed = false;

	std::string const cs_expertiseTreeStringTable("expertise_n");
	std::string const cs_expertiseDescriptionStringTable("expertise_d");

	ExpertiseManager::ExpertiseCoord const cs_invalidCoord(0, 0, 0, 0);
	ClientExpertiseManager::PostreqList const cs_emptyPostreqList;
	DataTable const cs_unusedDataTable;

	// grid x,y,z -> arrow flags
	typedef std::map<ExpertiseManager::ExpertiseCoord, ClientExpertiseManager::ExpertiseArrowFlags> ExpertiseArrowGrid;
	ExpertiseArrowGrid s_arrowGrid;

	// grid x,y,z -> postreq skills the square leads to
	typedef std::map<ExpertiseManager::ExpertiseCoord, std::set<std::string> > ExpertisePostreqGrid;
	ExpertisePostreqGrid s_postreqGrid;

	void buildGrid(DataTable const & datatable);
	ExpertiseManager::ExpertiseCoord const connectExpertises(ExpertiseManager::ExpertiseCoord const & startCoord, SkillObject const * targetSkill, int direction);
	SkillObject const * findExpertisePostreqBelow(SkillObject const * skill, int tree, int tier, int grid);
	SkillObject const * findExpertisePostreqToLeft(SkillObject const * skill, int tree, int tier, int grid);
	SkillObject const * findExpertisePostreqToRight(SkillObject const * skill, int tree, int tier, int grid);

	std::vector<std::string> s_allocatedExpertises;

	std::string s_emptyString;	
};

using namespace ClientExpertiseManagerNamespace;

//======================================================================

void ClientExpertiseManagerNamespace::buildGrid(DataTable const & datatable)
{
	UNREF(datatable); // unused; required for callbacks only

	s_arrowGrid.clear();
	s_postreqGrid.clear();

	ExpertiseManager::TreeIdList treeIdList;
	ExpertiseManager::getExpertiseTrees(treeIdList);

	for (ExpertiseManager::TreeIdList::const_iterator i = treeIdList.begin(); i != treeIdList.end(); ++i)
	{
		int tree = *i;
		for (int tier = 1; tier <= ExpertiseManager::getNumExpertiseTiers(); ++tier)
		{
			for (int grid = 1; grid <= ExpertiseManager::getNumExpertiseColumns(); ++grid)
			{
				SkillObject const * expertise = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid);
				if (!expertise)
				{
					continue;
				}

				ExpertiseManager::ExpertiseCoord startCoord(tree, tier, grid);
				ExpertiseManager::ExpertiseCoord nextStartCoord;

				int rankMax = ExpertiseManager::getExpertiseRankMax(expertise->getSkillName());
				SkillObject const * expertiseMaxRank = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rankMax);

				SkillObject const * postreqBelow   = findExpertisePostreqBelow(expertiseMaxRank, tree, tier, grid);
				SkillObject const * postreqToLeft  = findExpertisePostreqToLeft(expertiseMaxRank, tree, tier, grid);
				SkillObject const * postreqToRight = findExpertisePostreqToRight(expertiseMaxRank, tree, tier, grid);

				if (postreqBelow)
				{
					// Draw DOWN then left or right.
					nextStartCoord = connectExpertises(startCoord, postreqBelow, ClientExpertiseManager::EAF_Down);
					if (postreqBelow == postreqToLeft && nextStartCoord != cs_invalidCoord)
					{
						IGNORE_RETURN(connectExpertises(nextStartCoord, postreqBelow, ClientExpertiseManager::EAF_LeftOutgoing));
					}
					else if (postreqBelow == postreqToRight && nextStartCoord != cs_invalidCoord)
					{
						IGNORE_RETURN(connectExpertises(nextStartCoord, postreqBelow, ClientExpertiseManager::EAF_RightOutgoing));
					}
				}
				if (postreqToLeft && postreqToLeft != postreqBelow)
				{
					// Draw LEFT then down.
					nextStartCoord = connectExpertises(startCoord, postreqToLeft, ClientExpertiseManager::EAF_LeftOutgoing);
					if (nextStartCoord != cs_invalidCoord)
					{
						IGNORE_RETURN(connectExpertises(nextStartCoord, postreqToLeft, ClientExpertiseManager::EAF_Down));
					}
				}
				if (postreqToRight && postreqToRight != postreqBelow)
				{
					// Draw RIGHT then down.
					nextStartCoord = connectExpertises(startCoord, postreqToRight, ClientExpertiseManager::EAF_RightOutgoing);
					if (nextStartCoord != cs_invalidCoord)
					{
						IGNORE_RETURN(connectExpertises(nextStartCoord, postreqToRight, ClientExpertiseManager::EAF_Down));
					}
				}
			}
		}
	}

}

//----------------------------------------------------------------------

/**
 * @param startCoord - grid square to start from
 * @param targetSkill - skill to "draw" towards
 * @param direction - direction to "draw" in
 * 
 * @return the coordinate the routine stopped drawing at
 * (if destination was NOT reached), or an invalid
 * coordinate (if destination WAS reached).
 * 
 * this allows the caller to draw an L-shape by calling
 * this routine twice: i.e. making the 2nd call with the
 * starting coordinate set to the previous call's ending
 * coordinate.
 */
ExpertiseManager::ExpertiseCoord const ClientExpertiseManagerNamespace::connectExpertises(ExpertiseManager::ExpertiseCoord const & startCoord, SkillObject const * targetSkill, int direction)
{
	ExpertiseManager::ExpertiseCoord nextCoord(startCoord);

	ExpertiseManager::ExpertiseCoord endCoord;
	std::string const & targetSkillName = targetSkill->getSkillName();
	endCoord.tree = ExpertiseManager::getExpertiseTree(targetSkillName);
	endCoord.tier = ExpertiseManager::getExpertiseTier(targetSkillName);
	endCoord.grid = ExpertiseManager::getExpertiseGrid(targetSkillName);

	bool changingDirection = false;

	do
	{
		// Mark exit point from starting square
		int arrowFlags = s_arrowGrid[nextCoord];
		arrowFlags |= direction;
		s_arrowGrid[nextCoord] = static_cast<ClientExpertiseManager::ExpertiseArrowFlags>(arrowFlags);

		// Move to next square in desired direction. Early exit possible due to the need
		// to draw L shapes (i.e. stop at the turn, requiring a draw in another direction).
		if (direction == ClientExpertiseManager::EAF_Down)
		{
			if (nextCoord.tier < endCoord.tier)
			{
				++nextCoord.tier;
			}
			else
			{
				break;
			}
		}
		else if (direction == ClientExpertiseManager::EAF_LeftOutgoing)
		{
			if (nextCoord.grid > endCoord.grid)
			{
				--nextCoord.grid;
			}
			else
			{
				break;
			}
		}
		else if (direction == ClientExpertiseManager::EAF_RightOutgoing)
		{
			if (nextCoord.grid < endCoord.grid)
			{
				++nextCoord.grid;
			}
			else
			{
				break;
			}
		}
		else
		{
			DEBUG_WARNING(true, ("ClientExpertiseManager: bad ExpertiseArrowFlag %d", direction));
			break;
		}

		// Mark square as leading to the targetSkill for highlighting purposes.
		ClientExpertiseManager::PostreqList & postreqList = s_postreqGrid[nextCoord];
		postreqList.insert(targetSkillName);

		// Set arrow flags for square.
		// Only set flag for far side of square if we know we're going further in that direction.
		arrowFlags = s_arrowGrid[nextCoord];
		if (direction & ClientExpertiseManager::EAF_Down)
		{
			arrowFlags |= ClientExpertiseManager::EAF_Up;
			if (nextCoord.tier != endCoord.tier)
			{
				arrowFlags |= ClientExpertiseManager::EAF_Down;
			}
			else
			{
				changingDirection = true;
			}
		}
		if (direction & ClientExpertiseManager::EAF_LeftOutgoing)
		{
			if (nextCoord == endCoord)
			{
				arrowFlags |= ClientExpertiseManager::EAF_RightIncoming;
			}
			else
			{
				arrowFlags |= ClientExpertiseManager::EAF_RightOutgoing;
				if (nextCoord.grid != endCoord.grid)
				{
					arrowFlags |= ClientExpertiseManager::EAF_LeftOutgoing;
				}
				else
				{
					changingDirection = true;
				}
			}
		}
		if (direction & ClientExpertiseManager::EAF_RightOutgoing)
		{
			if (nextCoord == endCoord)
			{
				arrowFlags |= ClientExpertiseManager::EAF_LeftIncoming;
			}
			else
			{
				arrowFlags |= ClientExpertiseManager::EAF_LeftOutgoing;
				if (nextCoord.grid != endCoord.grid)
				{
					arrowFlags |= ClientExpertiseManager::EAF_RightOutgoing;
				}
				else
				{
					changingDirection = true;
				}
			}
		}
		s_arrowGrid[nextCoord] = static_cast<ClientExpertiseManager::ExpertiseArrowFlags>(arrowFlags);

	} while (nextCoord != endCoord && !changingDirection);

	// If target was reached by this draw, there is no need to return
	// a valid nextCoord as the starting point for a 2nd draw.
	if (nextCoord == endCoord)
	{
		nextCoord = cs_invalidCoord;
	}

	return nextCoord;
}

//----------------------------------------------------------------------

/**
 * @param skill - expertise to start from
 * @param tree - starting tree id number
 * @param tier - starting tier number (aka "y coordinate")
 * @param grid - starting grid number (aka "x coordinate")
 * 
 * @return - skill object for closest prereq expertise searching
 *         DOWN same column, then alternating down columns to
 *         left and right. returns NULL if none found
 */
SkillObject const * ClientExpertiseManagerNamespace::findExpertisePostreqBelow(SkillObject const * skill, int tree, int tier, int grid)
{
	SkillObject const * nextSkill = 0;

	int nextGridLeft  = grid;
	int nextGridRight = grid;

	while (nextGridLeft >= 1 || nextGridRight <= ExpertiseManager::getNumExpertiseColumns())
	{
		int nextTier = tier;
		while (nextTier < ExpertiseManager::getNumExpertiseTiers())
		{
			++nextTier;
			if (nextGridLeft >= 1)
			{
				nextSkill = ExpertiseManager::getExpertiseSkillAt(tree, nextTier, nextGridLeft);
				if (nextSkill)
				{
					if (nextSkill->dependsUponSkill(*skill, true))
					{
						return nextSkill;
					}
				}
			}
			if (nextGridRight <= ExpertiseManager::getNumExpertiseColumns())
			{
				nextSkill = ExpertiseManager::getExpertiseSkillAt(tree, nextTier, nextGridRight);
				if (nextSkill)
				{
					if (nextSkill->dependsUponSkill(*skill, true))
					{
						return nextSkill;
					}
				}
			}
		}
		--nextGridLeft;
		++nextGridRight;
	}

	return 0;
}

//----------------------------------------------------------------------

/**
 * @param skill - expertise to start from
 * @param tree - starting tree id number
 * @param tier - starting tier number (aka "y coordinate")
 * @param grid - starting grid number (aka "x coordinate")
 * 
 * @return - skill object for closest prereq expertise searching
 *         LEFT then DOWN. does not search directly down (i.e.
 *         not in same column). returns NULL if none found
 */
SkillObject const * ClientExpertiseManagerNamespace::findExpertisePostreqToLeft(SkillObject const * skill, int tree, int tier, int grid)
{
	while (tier <= ExpertiseManager::getNumExpertiseTiers())
	{
		int nextGrid = grid;
		while (nextGrid > 1)
		{
			--nextGrid;
			SkillObject const * nextSkill = ExpertiseManager::getExpertiseSkillAt(tree, tier, nextGrid);
			if (nextSkill)
			{
				if (nextSkill->dependsUponSkill(*skill, true))
				{
					return nextSkill;
				}
			}
		}
		++tier;
	}

	return 0;
}

//----------------------------------------------------------------------

/**
 * @param skill - expertise to start from
 * @param tree - starting tree id number
 * @param tier - starting tier number (aka "y coordinate")
 * @param grid - starting grid number (aka "x coordinate")
 * 
 * @return - skill object for closest prereq expertise searching
 *         RIGHT then DOWN. does not search directly down (i.e.
 *         not in same column). returns NULL if none found
 */
SkillObject const * ClientExpertiseManagerNamespace::findExpertisePostreqToRight(SkillObject const * skill, int tree, int tier, int grid)
{
	while (tier <= ExpertiseManager::getNumExpertiseTiers())
	{
		int nextGrid = grid;
		while (nextGrid < ExpertiseManager::getNumExpertiseColumns())
		{
			++nextGrid;
			SkillObject const * nextSkill = ExpertiseManager::getExpertiseSkillAt(tree, tier, nextGrid);
			if (nextSkill)
			{
				if (nextSkill->dependsUponSkill(*skill, true))
				{
					return nextSkill;
				}
			}
		}
		++tier;
	}

	return 0;
}

//======================================================================

void ClientExpertiseManager::install()
{
	DEBUG_FATAL(s_installed, ("ClientExpertiseManager already installed"));
	s_installed = true;

	buildGrid(cs_unusedDataTable);
	DataTableManager::addReloadCallback(ExpertiseManager::getExpertiseDatatableName(), &buildGrid);

	ExitChain::add(ClientExpertiseManager::remove, "ClientExpertiseManager");
}

//----------------------------------------------------------------------

void ClientExpertiseManager::remove()
{
	DEBUG_FATAL(!s_installed, ("ClientExpertiseManager not installed"));
	s_installed = false;
}

//----------------------------------------------------------------------

/**
 * @param expertiseList - list to be populated with character's expertise skills
 */
void ClientExpertiseManager::getExpertisesForPlayer(CreatureObject::SkillList & expertiseList)
{
	expertiseList.clear();

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		CreatureObject::SkillList const & skillList = player->getSkills();

		for (CreatureObject::SkillList::const_iterator i = skillList.begin(); i != skillList.end(); ++i)
		{
			if (*i)
			{
				if (ExpertiseManager::isExpertise(*i))
				{
					expertiseList.insert(*i);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

/**
 * @return int - max Expertise Points available to the character
 */
int ClientExpertiseManager::getExpertisePointsTotalForPlayer()
{
	int totalPoints = 0;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		int16 level = player->getLevel();
		totalPoints = ExpertiseManager::getExpertisePointsForLevel(level);
	}

	return totalPoints;
}

//----------------------------------------------------------------------

/**
 * @return int - number of Expertise Points spent by the character
 *               (1 point per expertise possessed) includes allocated points
 */
int ClientExpertiseManager::getExpertisePointsSpentForPlayer()
{
	int spentPoints = 0;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		int count = 0;
		CreatureObject::SkillList const & skillList = player->getSkills();
		for (CreatureObject::SkillList::const_iterator i = skillList.begin(); i != skillList.end(); ++i)
		{
			const SkillObject *skill = *i;
			if(skill->getSkillName() == "expertise")
				continue;
			if(ExpertiseManager::isExpertise(skill))
				count++;
		}
		spentPoints = count;
	}

	spentPoints += getExpertisePointsAllocatedForPlayer();

	return spentPoints;
}

//----------------------------------------------------------------------

/**
 * @return int - number of Expertise Points spent by the character in specified tree
 */
int ClientExpertiseManager::getExpertisePointsSpentForPlayerInTree(int tree)
{
	int spentPoints = 0;

	CreatureObject::SkillList expertiseList;
	getExpertisesForPlayer(expertiseList);

	for (CreatureObject::SkillList::const_iterator i = expertiseList.begin(); i != expertiseList.end(); ++i)
	{
		SkillObject const * expertise = *i;
		if (expertise)
		{
			std::string const & expertiseName = expertise->getSkillName();
			if(expertiseName == "expertise")
				continue;
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				spentPoints += 1;
			}
		}
	}

	for (std::vector<std::string>::const_iterator i2 = s_allocatedExpertises.begin(); i2 != s_allocatedExpertises.end(); ++i2)
	{
		std::string const &expertiseName = *i2;
		if (!expertiseName.empty())
		{
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				spentPoints += 1;
			}
		}
	}
	return spentPoints;
}

//----------------------------------------------------------------------

int ClientExpertiseManager::getExpertisePointsSpentForPlayerInTreeUpToTier(int tree, int tier)
{
	int spentPoints = 0;

	CreatureObject::SkillList expertiseList;
	getExpertisesForPlayer(expertiseList);

	for (CreatureObject::SkillList::const_iterator i = expertiseList.begin(); i != expertiseList.end(); ++i)
	{
		SkillObject const * expertise = *i;
		if (expertise)
		{
			std::string const & expertiseName = expertise->getSkillName();
			if(expertiseName == "expertise")
				continue;
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				if(ExpertiseManager::getExpertiseTier(expertiseName) <= tier)
					spentPoints += 1;
			}
		}
	}

	for (std::vector<std::string>::const_iterator i2 = s_allocatedExpertises.begin(); i2 != s_allocatedExpertises.end(); ++i2)
	{
		std::string const &expertiseName = *i2;
		if (!expertiseName.empty())
		{
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				if(ExpertiseManager::getExpertiseTier(expertiseName) <= tier)
					spentPoints += 1;
			}
		}
	}
	return spentPoints;

}

//----------------------------------------------------------------------

/**
 * @return int - the number of Expertise Points available for
 *         the character to spend
 */
int ClientExpertiseManager::getExpertisePointsRemainingForPlayer()
{
	return getExpertisePointsTotalForPlayer() - getExpertisePointsSpentForPlayer();
}

//----------------------------------------------------------------------

/**
* @return int - the number of Expertise Points allocated by the player
*/
int ClientExpertiseManager::getExpertisePointsAllocatedForPlayer()
{
	return s_allocatedExpertises.size();
}

//----------------------------------------------------------------------

int ClientExpertiseManager::getExpertisePointsAllocatedForPlayerInTree(int tree)
{
	int spentPoints = 0;
	for (std::vector<std::string>::const_iterator i2 = s_allocatedExpertises.begin(); i2 != s_allocatedExpertises.end(); ++i2)
	{
		std::string const &expertiseName = *i2;
		if (!expertiseName.empty())
		{
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				spentPoints += 1;
			}
		}
	}
	return spentPoints;
}

//----------------------------------------------------------------------

/**
 * @return list of expertise tree id's possessed by the character
 */
ClientExpertiseManager::TreeIdList const & ClientExpertiseManager::getExpertiseTreesForPlayer()
{
	std::string const & skillTemplate = CuiSkillManager::getSkillTemplate();
	return ExpertiseManager::getExpertiseTreesForProfession(skillTemplate);
}

//----------------------------------------------------------------------

/**
 * @return bool - true if any expertise trees exist for the
 *         character's profession
 */
bool ClientExpertiseManager::hasExpertiseTrees()
{
	return !getExpertiseTreesForPlayer().empty();
}

//----------------------------------------------------------------------

/**
 * @param expertiseName - skill name of expertise
 * @return int - current rank possessed by character in given expertise
 */
int ClientExpertiseManager::getExpertiseRankForPlayer(std::string const & expertiseName, bool countAllocated)
{
	int result = 0;

	int rankMax = ExpertiseManager::getExpertiseRankMax(expertiseName);

	int tree = ExpertiseManager::getExpertiseTree(expertiseName);
	int tier = ExpertiseManager::getExpertiseTier(expertiseName);
	int grid = ExpertiseManager::getExpertiseGrid(expertiseName);
	int rank = 1;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		while (rank <= rankMax)
		{
			SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rank);
			if(!skill)
				break;
			if ( (countAllocated && playerHasExpertiseOrHasAllocated(skill->getSkillName())) ||
				 (!countAllocated && playerHasExpertise(skill->getSkillName())) )
			{
				result = rank;
				rank++;
			}
			else
			{
				break;
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

/**
 * @param treeId - int id of an expertise tree
 * @param localizedTreeName - ref to string to be populated with result
 * 
 * @return bool - false if localization failed or treeId not found
 */
bool ClientExpertiseManager::localizeExpertiseTreeNameFromId(int treeId, Unicode::String & localizedTreeName)
{
	std::string const & treeName = ExpertiseManager::getExpertiseTreeNameFromId(treeId);
	return StringId(cs_expertiseTreeStringTable, treeName).localize(localizedTreeName);
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::localizeExpertiseTreeDescriptionFromId(int treeId, Unicode::String & localizedTreeDescription)
{
	std::string const & treeName = ExpertiseManager::getExpertiseTreeNameFromId(treeId);
	return StringId(cs_expertiseDescriptionStringTable, treeName).localize(localizedTreeDescription);
}

//----------------------------------------------------------------------

/**
 * @param tree - tree id number
 * @param tier - tier number (aka "y coordinate")
 * @param grid - grid number (aka "x coordinate")
 * 
 * @return - bool flags struct indicating which tree components
 *         should be displayed at grid square
 */
ClientExpertiseManager::ExpertiseArrowFlags const ClientExpertiseManager::getArrowFlagsAt(int tree, int tier, int grid)
{
	ExpertiseManager::ExpertiseCoord expertiseCoord(tree, tier, grid);

	ExpertiseArrowGrid::const_iterator i = s_arrowGrid.find(expertiseCoord);
	if (i != s_arrowGrid.end())
	{
		return (*i).second;
	}

	return ClientExpertiseManager::EAF_None;
}

//----------------------------------------------------------------------

/**
 * @param tree - tree id number
 * @param tier - tier number (aka "y coordinate")
 * @param grid - grid number (aka "x coordinate")
 * 
 * @return - set of names of postrequisite skills that the grid square leads to
 */
ClientExpertiseManager::PostreqList const & ClientExpertiseManager::getPostreqListAt(int tree, int tier, int grid)
{
	ExpertiseManager::ExpertiseCoord expertiseCoord(tree, tier, grid);

	ExpertisePostreqGrid::const_iterator i = s_postreqGrid.find(expertiseCoord);
	if (i != s_postreqGrid.end())
	{
		return (*i).second;
	}

	return cs_emptyPostreqList;
}

//----------------------------------------------------------------------

/**
 * @param tree - tree id number
 * @param tier - tier number (aka "y coordinate")
 * @param grid - grid number (aka "x coordinate")
 * 
 * @return - skill the player is currently eligible for at that grid square (or NULL if none)
 */
SkillObject const * ClientExpertiseManager::getExpertisePlayerIsEligibleForAt(int tree, int tier, int grid)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
	{
		return 0;
	}

	SkillObject const * expertise = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid);
	if (!expertise)
	{
		return 0;
	}

	std::string nextSkillName = getNextExpertiseNameGivenBaseExpertise(expertise->getSkillName());

	if(nextSkillName.empty())
		return 0;

	if(!canAllocateExpertise(nextSkillName))
		return 0;

	
	return SkillManager::getInstance().getSkill(nextSkillName);
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::localizeExpertiseName(std::string const &expertiseName, Unicode::String & localizedExpertiseName)
{
	return CuiSkillManager::localizeSkillName(expertiseName, localizedExpertiseName);
}


//----------------------------------------------------------------------

bool ClientExpertiseManager::localizeExpertiseDescription(std::string const &expertiseName, Unicode::String & localizedExpertiseDescription)
{
	return CuiSkillManager::localizeSkillDescription(expertiseName, localizedExpertiseDescription);
}

void ClientExpertiseManager::clearAllocatedExpertises()
{
	s_allocatedExpertises.clear();
}

//----------------------------------------------------------------------

void ClientExpertiseManager::clearAllocatedExpertisesInTree(int tree)
{
	std::vector<std::string> expertisesToDeallocate;
	for (std::vector<std::string>::const_iterator i = s_allocatedExpertises.begin(); i != s_allocatedExpertises.end(); ++i)
	{
		std::string const &expertiseName = *i;
		if (!expertiseName.empty())
		{
			if (ExpertiseManager::getExpertiseTree(expertiseName) == tree)
			{
				expertisesToDeallocate.push_back(expertiseName);
			}
		}
	}
	for(std::vector<std::string>::const_iterator i2 = expertisesToDeallocate.begin(); i2 != expertisesToDeallocate.end(); ++i2)
	{
		std::string const &expertiseName = *i2;
		//We don't have to check restrictions here because there are no cross-tree restrictions
		deallocateExpertise(expertiseName, false);
	}
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::hasAllocatedExpertise(std::string const & expertiseName)
{
	return find(s_allocatedExpertises.begin(), s_allocatedExpertises.end(), expertiseName) != s_allocatedExpertises.end();
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::allocateExpertise(std::string const & expertiseName, bool checkRestrictions)
{
	if(checkRestrictions && !canAllocateExpertise(expertiseName))
		return false;
	s_allocatedExpertises.push_back(expertiseName);
	return true;
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::deallocateExpertise(std::string const & expertiseName, bool checkRestrictions)
{
	if(checkRestrictions && !canDeallocateExpertise(expertiseName))
		return false;
	s_allocatedExpertises.erase(find(s_allocatedExpertises.begin(), s_allocatedExpertises.end(), expertiseName));
	return true;
}

//----------------------------------------------------------------------

int ClientExpertiseManager::getNumAllocatedExpertises()
{
	return s_allocatedExpertises.size();
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::playerHasExpertiseOrHasAllocated(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return false;
	return (player->hasSkill(*skill) || hasAllocatedExpertise(expertiseName));
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::playerHasExpertise(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return false;
	return player->hasSkill(*skill);
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getNextExpertiseNameGivenBaseExpertise(std::string const & baseExpertiseName)
{
	int rankMax = ExpertiseManager::getExpertiseRankMax(baseExpertiseName);

	int tree = ExpertiseManager::getExpertiseTree(baseExpertiseName);
	int tier = ExpertiseManager::getExpertiseTier(baseExpertiseName);
	int grid = ExpertiseManager::getExpertiseGrid(baseExpertiseName);
	int rank = 1;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		while (rank <= rankMax)
		{
			SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rank);
			if (skill && playerHasExpertiseOrHasAllocated(skill->getSkillName()))
			{
				rank++;
			}
			else
			{
				return skill->getSkillName();
			}
		}
	}

	//Player has all the ranks of this skill
	return s_emptyString;
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getTopExpertiseNameGivenBaseExpertise(std::string const & baseExpertiseName)
{
	int rankMax = ExpertiseManager::getExpertiseRankMax(baseExpertiseName);

	int tree = ExpertiseManager::getExpertiseTree(baseExpertiseName);
	int tier = ExpertiseManager::getExpertiseTier(baseExpertiseName);
	int grid = ExpertiseManager::getExpertiseGrid(baseExpertiseName);
	int rank = 1;

	CreatureObject const * const player = Game::getPlayerCreature();
	std::string topSkill;
	if (player)
	{
		while (rank <= rankMax)
		{
			SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rank);
			if (skill && playerHasExpertiseOrHasAllocated(skill->getSkillName()))
			{
				rank++;
				topSkill = skill->getSkillName();
			}
			else
			{
				return topSkill;
			}
		}
	}

	//Player has all the ranks of this skill
	return topSkill;
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::isExpertiseCommandType(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	const std::vector<std::string> & commands = skill->getCommandsProvided();
	return !commands.empty();
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getExpertiseCommand(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	const std::vector<std::string> & commands = skill->getCommandsProvided();
	if(!commands.empty())
	{
		DEBUG_FATAL(commands.size() != 1, ("expertise %s has %d commands which is too many", expertiseName.c_str(), commands.size()));
		return commands[0];
	}
	else
		return s_emptyString;
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::isExpertiseSchematicType(std::string const & expertiseName)
{
	//@TODO
	UNREF(expertiseName);
	return false;
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getExpertiseSchematic(std::string const & expertiseName)
{
	//@TODO
	UNREF(expertiseName);
	return s_emptyString;
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::isExpertiseSkillModType(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	SkillObject::GenericModVector const & mods = skill->getStatisticModifiers();
	return !mods.empty();
}

//----------------------------------------------------------------------

void ClientExpertiseManager::getExpertiseSkillMods(std::string const & baseExpertiseName, ExpertiseSkillModStruct & expertiseSkillMods)
{
	int rankMax = ExpertiseManager::getExpertiseRankMax(baseExpertiseName);

	int tree = ExpertiseManager::getExpertiseTree(baseExpertiseName);
	int tier = ExpertiseManager::getExpertiseTier(baseExpertiseName);
	int grid = ExpertiseManager::getExpertiseGrid(baseExpertiseName);
	int rank = 1;

	int c;
	for(c = 0; c < MAX_NUM_SKILL_MODS_PER_EXPERTISE * MAX_NUM_EXPERTISE_RANKS; c++)
		expertiseSkillMods.values[c] = SKILL_MOD_TAG_VALUE;

	if(rankMax > MAX_NUM_EXPERTISE_RANKS)
	{
		DEBUG_WARNING(true, ("ClientExpertiseManager WARNING max rank for %s returned %d > %d.  This is actually super bad.", baseExpertiseName.c_str(), 
			rankMax, MAX_NUM_EXPERTISE_RANKS));
		rankMax = MAX_NUM_EXPERTISE_RANKS;
	}

	unsigned int modCount = 0;
	while (rank <= rankMax)
	{
		SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(tree, tier, grid, rank);
		if(!skill)
		{
			DEBUG_WARNING(true, ("ClientExpertiseManager getExpertiseSkillMods asked for %d %d %d %d got null.  This is actually super bad.",
				tree, tier, grid, rank));
			continue;
		}
		SkillObject::GenericModVector const & mods = skill->getStatisticModifiers();
		if(rank == 1)
		{
			modCount = mods.size();
		}
		for(unsigned int i = 0; ((i < MAX_NUM_SKILL_MODS_PER_EXPERTISE) && (i < mods.size())); ++i)
		{
			DEBUG_WARNING(mods.size() > MAX_NUM_SKILL_MODS_PER_EXPERTISE, ("expertise %s had more than %d skill mods %d", 
				baseExpertiseName.c_str(), MAX_NUM_SKILL_MODS_PER_EXPERTISE, mods.size()));
			if(rank == 1)
			{
				expertiseSkillMods.names[i] = mods[i].first;   // Fill out names on the first rank
			}
			else
			{
				DEBUG_WARNING(modCount != mods.size(), ("expertise %s had different numbers of skill mods rank 1 = %d rank %d = %d",
					baseExpertiseName.c_str(), modCount, rank, mods.size()));
				DEBUG_WARNING(expertiseSkillMods.names[i] != mods[i].first, ("expertise %s had differently named skill mods rank 1 = %s rank %d = %s",
					baseExpertiseName.c_str(), expertiseSkillMods.names[i].c_str(), rank, mods[i].first.c_str()));
			}
			expertiseSkillMods.values[i * MAX_NUM_EXPERTISE_RANKS + rank - 1] = mods[i].second; 
		}
		++rank;
	}
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::canAllocateExpertise(std::string const & expertiseName)
{
	if(playerHasExpertiseOrHasAllocated(expertiseName))
		return false;

	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return false;
	
	//Check if the player has all the prerequisites
	SkillObject::SkillVector const prereqs = skill->getPrerequisiteSkills();
	for (SkillObject::SkillVector::const_iterator i = prereqs.begin(); i != prereqs.end(); ++i)
	{
		SkillObject const * prereq = (*i);
		if (!prereq || !playerHasExpertiseOrHasAllocated(prereq->getSkillName()))
		{
			return 0;
		}
	}
	// Do you have a free point?
	int pointsRemaining = getExpertisePointsRemainingForPlayer();
	if (pointsRemaining < 1)
	{
		return false;
	}

	//Check if the player has enough points for a skill of this tier
	int tree = ExpertiseManager::getExpertiseTree(expertiseName);
	int pointsInTree = getExpertisePointsSpentForPlayerInTree(tree);
	int tier = ExpertiseManager::getExpertiseTier(expertiseName);
	if (pointsInTree < (tier - 1) * MAX_NUM_EXPERTISE_RANKS)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool ClientExpertiseManager::canDeallocateExpertise(std::string const & expertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(expertiseName);
	if(!skill)
		return false;
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return false;
	if(player->hasSkill(*skill))
		return false;
	if(!hasAllocatedExpertise(expertiseName))
		return false;

	// For use in calculating points
	int tree = ExpertiseManager::getExpertiseTree(expertiseName);
	int tier = ExpertiseManager::getExpertiseTier(expertiseName);

	//If player has any expertises allocated that depend on this one, return false
	for(std::vector<std::string>::iterator i = s_allocatedExpertises.begin(); i != s_allocatedExpertises.end(); ++i)
	{
		std::string const &s = *i;
		SkillObject const * potentialPostReq = SkillManager::getInstance().getSkill(s);
		if(potentialPostReq)
		{
			SkillObject::SkillVector const prereqs = potentialPostReq->getPrerequisiteSkills();
			for (SkillObject::SkillVector::const_iterator i = prereqs.begin(); i != prereqs.end(); ++i)
			{
				SkillObject const * prereq = (*i);
				if (prereq && (prereq->getSkillName() == expertiseName))
				{
					return false;
				}
			}
		}

		//If player has any expertises allocated that depend on having a certain number of
		//points, and deallocating this expertise would make the player go below, then return
		//false
		int subTree = ExpertiseManager::getExpertiseTree(potentialPostReq->getSkillName());
		int subTier = ExpertiseManager::getExpertiseTier(potentialPostReq->getSkillName());
		if((subTree == tree) &&(subTier > tier))
		{
			int pointsUpToTierAfter = ClientExpertiseManager::getExpertisePointsSpentForPlayerInTreeUpToTier(tree, subTier - 1) - 1;
			int pointsRequired = (subTier - 1) * MAX_NUM_EXPERTISE_RANKS;
			if(pointsRequired > pointsUpToTierAfter)
			{
				return false;
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

void ClientExpertiseManager::sendAllocatedExpertiseListAndClear()
{
	ExpertiseRequestMessage erm;
	erm.setAddExpertisesList(s_allocatedExpertises);
	erm.setClearAllExpertisesFirst(false);
	GameNetwork::send(erm, true);
	s_allocatedExpertises.clear();
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getPreviousExpertise(std::string const & baseExpertiseName)
{
	SkillObject const * skill = SkillManager::getInstance().getSkill(baseExpertiseName);
	if(!skill)
		return s_emptyString;
	SkillObject::SkillVector const prereqs = skill->getPrerequisiteSkills();
	for (SkillObject::SkillVector::const_iterator i = prereqs.begin(); i != prereqs.end(); ++i)
	{
		SkillObject const * prereq = (*i);
		if (prereq && ExpertiseManager::isExpertise(prereq))
		{
			return prereq->getSkillName();
		}
	}
	return s_emptyString;
}

//----------------------------------------------------------------------

std::string ClientExpertiseManager::getBaseExpertiseNameForExpertise(std::string const & expertiseName)
{
	if(expertiseName.empty())
		return s_emptyString;
	SkillObject const * skill = ExpertiseManager::getExpertiseSkillAt(ExpertiseManager::getExpertiseTree(expertiseName), 
		ExpertiseManager::getExpertiseTier(expertiseName), ExpertiseManager::getExpertiseGrid(expertiseName), 1);
	if(skill)
		return skill->getSkillName();
	return s_emptyString;
}


//======================================================================
