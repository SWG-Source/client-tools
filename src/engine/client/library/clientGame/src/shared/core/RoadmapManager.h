//======================================================================
//
// RoadmapManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_RoadmapManager_H
#define INCLUDED_RoadmapManager_H

//======================================================================

class DataTable;
class PlayerObject;

#include "Unicode.h"

//----------------------------------------------------------------------

class RoadmapManager
{
public:
	
	static void                        install();
	static void                        remove();

	static std::string const &getIconPathForSkill   (std::string const & skillName);
	static std::string const &getActivityTypeForSkill   (std::string const & skillName);
	static std::string getRoadmapNameForTemplateName(const std::string &templateName);
	static int getSkillIndex(const std::string &templateName, const std::string &workingSkill);	
	static int getCurrentPhase(const std::string &roadmapName, const std::string &templateName, const std::string &workingSkill, bool addOne = false);
	static bool hasPlayerBranched();
	static bool hasPlayerBranched(const std::string &roadmapName, const std::string &templateName, const std::string &workingSkill);
	static int getPlayerBranch(); //0-based
	static int getPlayerBranch(const std::string &roadmapName, const std::string &templateName);

	static void getSkillList(const std::string &roadmapName, int track, int phase, stdvector<std::string>::fwd &out, stdvector<int>::fwd &xpOut, int &xpToFirstSkill);
	static std::string const &getPlayerSkillTemplate();
	static std::string const &getPlayerWorkingSkill();	
	static std::string const &getTemplateForRoadmapTrack(std::string const &roadmapName, int track);

	static void getCurrentColorForSkill(const std::string &skillName, unsigned char &r, unsigned char &g, unsigned char &b);

	static int getNumberOfBranchesInRoadmap(const std::string &roadmapName);

	static void getStartingProfessions(stdvector<std::string>::fwd & startingProfessionVector, bool prioritizeList);
	static void getStartingProfessionTemplateAndSkill(std::string const & startingProfessionName, std::string & startingTemplate, std::string & startingSkill);
	static void getStartingProfessionSkill(std::string & startingTemplate, std::string & startingSkill);
	static std::string getDefaultStartingProfession();
	static int getNumberOfRoadmaps();
	static void getRoadmapList(stdvector<std::string>::fwd &out);

	//This is false if the player is not on a roadmap, which generally means that they are working on their own chosen skill; but see next function
	static bool playerIsOnRoadmap();
	//This function determines if a player is a brand new character, without any working skill or template.  This should only be true during NPE.
	static bool playerIsNewCharacter();

	static const stdvector<std::string>::fwd &getPhaseIconList(const std::string &roadmapName);

	static const std::string &getItemRewardAppearance(const std::string &templateName, const std::string &skillName);
	static const std::string &getItemRewardTextId(const std::string &templateName, const std::string &skillName);

	//Space template functions

	//Get the name of the template that the player's on, be it novice, imperial, rebel
	static const std::string &getPlayerSpaceTemplateName();

	//Get the name of the space skill the player is working on (currently, the first skill they don't have)
	static const std::string &getPlayerSpaceWorkingSkill();

	//Get the full list of skills for a template
	static void getSkillList(const std::string &templateName, stdvector<std::string>::fwd &out, stdvector<int>::fwd &xpOut, int &xpToFirstSkill);
	
	//Get the name of the politician skill the player is working on (currently, the first skill they don't have)
	static const std::string &getPlayerPoliticianWorkingSkill();

	//Get the name of the Storyteller skill the player is working on (currently, the first skill they don't have)
	static const std::string &getPlayerStorytellerWorkingSkill();


	// -- New functions for the level-based form of roadmaps

	// Is this template level-based on skill-based
	static bool isLevelBasedTemplate(const std::string &templateName);

	//outStartLevel is the first level at which you are in this phase, outEndLevel is the level at which you transition.
	//outEndLevel of phase x == outStartLevel of phase x+1
	static const void getLevelLimits(int phase, int &outStartLevel, int &outEndLevel);

	//Returns the "representative skill" for this level, or empty string if there is none
	static const std::string getSkillForLevel(const std::string &templateName, int level);

	//Returns the current phase for the player based on the level number
	static int getPlayerCurrentPhase();
	
	//Returns a float from 0.0 to 1.0 indicating how far that player has come towards their current phase
	static float getPlayerXpProgressForCurrentPhase();

	//Returns the next skill that the player will get, if any, or empty string if they are at the cap
	static const std::string getNextSkillForLevelTemplate();

private:
	
	static void                loadSkillClientDatatable();
	static void                loadTemplateDatatable();
	static void                loadRoadmapDatatable();
	static void                loadColorForSkillDatatable();
	static void                loadItemRewardsDatatable();
	
	static void                splitString(std::string const &source, stdvector<std::string>::fwd &out);
};

//======================================================================

#endif
