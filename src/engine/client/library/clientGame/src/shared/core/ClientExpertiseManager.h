//======================================================================
//
// ClientExpertiseManager.h
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientExpertiseManager_H
#define INCLUDED_ClientExpertiseManager_H

#include "clientGame/CreatureObject.h"

class SkillObject;

//======================================================================

class ClientExpertiseManager
{

public:

	static const int SKILL_MOD_TAG_VALUE = -9999;
	static const int MAX_NUM_EXPERTISE_RANKS = 4;
	static const int MAX_NUM_SKILL_MODS_PER_EXPERTISE = 4;

	struct ExpertiseSkillModStruct
	{
		std::string names[MAX_NUM_SKILL_MODS_PER_EXPERTISE];    // some can be empty at the end
		int values[MAX_NUM_SKILL_MODS_PER_EXPERTISE * MAX_NUM_EXPERTISE_RANKS];          // MAX_NUM_EXPERTISE_RANKS values in each of the 3 mods; some can be empty, in which case pad with zeroes
	};

	static void install();
	static void remove();

	//----------------------------------------------------------------------
	// PLAYER STATE

	static void getExpertisesForPlayer(CreatureObject::SkillList & expertiseList);

	// Expertise Points
	static int getExpertisePointsTotalForPlayer();
	static int getExpertisePointsSpentForPlayer();
	static int getExpertisePointsSpentForPlayerInTree(int tree);
	static int getExpertisePointsSpentForPlayerInTreeUpToTier(int tree, int tier);
	static int getExpertisePointsRemainingForPlayer();
	static int getExpertisePointsAllocatedForPlayer();
	static int getExpertisePointsAllocatedForPlayerInTree(int tree);

	// Expertise Trees
	typedef std::vector<int> TreeIdList;
	static TreeIdList const & getExpertiseTreesForPlayer();
	static bool hasExpertiseTrees();

	// Individual Expertises
	// if countAllocated is false, returns only skills the player actually has (spent)
	static int getExpertiseRankForPlayer(std::string const & expertiseName, bool countAllocated = true);
	// returns, for example, fs_prowess_3 given fs_prowess_1, if the player has 1 and 2
	static std::string getNextExpertiseNameGivenBaseExpertise(std::string const & baseExpertiseName);
	// returns fs_prowess_2
	static std::string getTopExpertiseNameGivenBaseExpertise(std::string const & baseExpertiseName);
	// returns the expertise that this expertise depends on, if any
	static std::string getPreviousExpertise(std::string const & baseExpertiseName);
	// returns fs_prowess_1 given any fs_prowess
	static std::string getBaseExpertiseNameForExpertise(std::string const & expertiseName);

	// Allocated (but not spent) points
	static void clearAllocatedExpertises();
	static void clearAllocatedExpertisesInTree(int tree);
	static bool hasAllocatedExpertise(std::string const & expertisName);
	static bool allocateExpertise(std::string const & expertiseName, bool checkRestrictions = true);
	static bool deallocateExpertise(std::string const & expertiseName, bool checkRestrictions = true);
	static int  getNumAllocatedExpertises();
	static bool canAllocateExpertise(std::string const & expertiseName);
	static bool canDeallocateExpertise(std::string const & expertiseName);

	static bool playerHasExpertise(std::string const & expertiseName);
	// Returns true if the player has this expertise, or has it currently allocated
	static bool playerHasExpertiseOrHasAllocated(std::string const & expertiseName);

	// Determining the type of an expertise.  Expertises come in 3 types - schematic, command, and skill mod.
	// They should only be one of the 3 at once
	static bool isExpertiseCommandType(std::string const & expertiseName);
	static std::string getExpertiseCommand(std::string const & expertiseName);
	static bool isExpertiseSchematicType(std::string const & expertiseName);
	static std::string getExpertiseSchematic(std::string const & expertiseName);
	static bool isExpertiseSkillModType(std::string const & expertiseName);
	//This version wants a base expertise, in other words a "_1" version
	static void getExpertiseSkillMods(std::string const & baseExpertiseName, ExpertiseSkillModStruct & expertiseSkillMods);

	static void sendAllocatedExpertiseListAndClear();

	//----------------------------------------------------------------------
	// UI SUPPORT

	// Expertise Strings
	static bool localizeExpertiseTreeNameFromId(int treeId, Unicode::String & localizedTreeName);
	static bool localizeExpertiseTreeDescriptionFromId(int treeId, Unicode::String & localizedTreeDescription);

	// Expertise Grid
	enum ExpertiseArrowFlags
	{
		EAF_None           = 0x0000,
		EAF_Up             = 0x0001,
		EAF_Down           = 0x0002,
		EAF_LeftIncoming   = 0x0004,
		EAF_LeftOutgoing   = 0x0008,
		EAF_RightIncoming  = 0x0010,
		EAF_RightOutgoing  = 0x0020
	};
	static ExpertiseArrowFlags const getArrowFlagsAt(int tree, int tier, int grid);

	typedef stdset<std::string>::fwd PostreqList;
	static PostreqList const & getPostreqListAt(int tree, int tier, int grid);

	static SkillObject const * getExpertisePlayerIsEligibleForAt(int tree, int tier, int grid);

	static bool localizeExpertiseName(std::string const &expertiseName, Unicode::String & localizedExpertiseName);
	static bool localizeExpertiseDescription(std::string const &expertiseName, Unicode::String & localizedExpertiseDescription);
};

//======================================================================

#endif
