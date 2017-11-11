//======================================================================
//
// RoadmapManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/RoadmapManager.h"

#include "clientAudio/Audio.h"
#include "clientAudio/SoundId.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedSkillSystem/LevelManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>
#include <vector>

namespace RoadmapManagerNamespace
{
	const int MAX_NUM_BRANCHES = 4;
	const int NUM_PHASES = 4; //If this changes, have to change many things	

	struct SkillClientDataRecord
	{
		std::string iconPath;
		std::string activityType;
	};

	struct TemplateRecord
	{
		std::vector<std::string> skills;
		std::string startingProfession;
		std::string professionName;
		int interfacePriority;
		bool levelBased;
	};

	std::string m_defaultStartingProfession("commando");

	struct RoadmapRecord
	{
		std::string roadmapName;
		std::vector<std::string> templateNames;
		int phaseCounts[MAX_NUM_BRANCHES][NUM_PHASES];
		std::vector<std::string> phaseIconPaths;
	};
	std::string const s_emptyString;

	std::map<uint32, SkillClientDataRecord> m_skillClientData;
	std::map<uint32, TemplateRecord> m_templateData;
	std::map<uint32, RoadmapRecord> m_roadmapData;

	struct XPRecord
	{
		int maxXP;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
	std::vector<XPRecord> m_xpRecordData;

	struct ItemRewardRecord
	{
		std::string appearanceName;
		std::string stringId;
	};
	std::map<uint32, ItemRewardRecord> m_itemRewardData;

	std::string const m_roadmapIconPath("/Styles.Icon.Roadmap.");

	int m_numberOfRoadmaps;

	char *NEW_CHARACTER_WORKING_SKILL = "new_character_no_skill";

	const int NUM_SPACE_TEMPLATES = 3;
	std::string const s_spaceTemplateNames[] = {"space_pilot_rebel", "space_pilot_imperial", "space_pilot_neutral"};

	std::string const s_politicianTemplateName = "politician";
	std::string const s_storytellerTemplateName = "chronicles";

	//@TODO: These should be in a datatable somewhere.  phase limits are the level boundaries for the phases
	const int s_phaseLimits[] = { 0, 21, 45, 69, 90 };
	//skill ups are the levels at which we get a new skill
	const int s_skillUps[] = { 1, 4, 7, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66, 70, 74, 78, 82, 86, 90 };
		
}

using namespace RoadmapManagerNamespace;

//----------------------------------------------------------------------

//----------------------------------------------------------------------

void RoadmapManager::install()
{
	loadSkillClientDatatable();
	loadTemplateDatatable();
	loadRoadmapDatatable();
	loadColorForSkillDatatable();
	loadItemRewardsDatatable();
}

//----------------------------------------------------------------------

void RoadmapManager::remove()
{
	m_xpRecordData.clear();
}

//----------------------------------------------------------------------

std::string const &RoadmapManager::getIconPathForSkill(std::string const &skillName)
{
	std::map<uint32, SkillClientDataRecord>::iterator i = m_skillClientData.find(Crc::normalizeAndCalculate(skillName.c_str()));
	if(i == m_skillClientData.end())
	{
		static std::string tmpResult;
		//If no data, get the player's current roadmap and append a set path to that
		std::string const &roadmapName = getRoadmapNameForTemplateName(getPlayerSkillTemplate());
		tmpResult = m_roadmapIconPath;
		tmpResult.append(roadmapName);
		return tmpResult;
	}
	return i->second.iconPath;
}

//----------------------------------------------------------------------

std::string const &RoadmapManager::getActivityTypeForSkill(std::string const & skillName)
{
	std::map<uint32, SkillClientDataRecord>::iterator i = m_skillClientData.find(Crc::normalizeAndCalculate(skillName.c_str()));
	if(i == m_skillClientData.end())
	{
		return s_emptyString;
	}
	return i->second.activityType;
}

//----------------------------------------------------------------------

void RoadmapManager::loadSkillClientDatatable()
{
	DataTable *datatable = DataTableManager::getTable("datatables/roadmap/skill_to_icon_mapping.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(datatable->getNumRows());
	int skillNameColumn = datatable->findColumnNumber("SKILLNAME");
	int iconColumn = datatable->findColumnNumber("ICON");
	int activityTypeColumn = datatable->findColumnNumber("ACTIVITY_TYPE");

	for(unsigned int r = 0; r < numRows; ++r)
	{
		const std::string & skillName = datatable->getStringValue(skillNameColumn, r);
		const std::string & iconPath = datatable->getStringValue(iconColumn, r);
		const std::string & activityType = datatable->getStringValue(activityTypeColumn, r);
		
		SkillClientDataRecord scdr;
		scdr.iconPath = iconPath;
		scdr.activityType = activityType;
		m_skillClientData.insert(std::make_pair(Crc::normalizeAndCalculate(skillName.c_str()), scdr));
	}
}

//----------------------------------------------------------------------

std::string RoadmapManager::getRoadmapNameForTemplateName(const std::string &templateName)
{
	std::string::size_type underscore;

	underscore = templateName.find_last_of('_');
	if (underscore==std::string::npos)
	{
		return templateName;
	}
	std::string result = templateName.substr(0, underscore);	
	return result;
}

//----------------------------------------------------------------------

void RoadmapManager::loadTemplateDatatable()
{
	DataTable *datatable = DataTableManager::getTable("datatables/skill_template/skill_template.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(datatable->getNumRows());
	int const templateNameColumn = datatable->findColumnNumber("templateName");
	int const templateColumn = datatable->findColumnNumber("template");
	int const templateStartingColumn  = datatable->findColumnNumber("startingTemplateName");
	int const templateUIPriorityColumn = datatable->findColumnNumber("userInterfacePriority");
	int const levelBasedColumn = datatable->findColumnNumber("levelBased");

	std::string const & defaultStartingProfession = datatable->getStringDefaultForColumn(templateStartingColumn);
	if (!defaultStartingProfession.empty()) 
	{
		m_defaultStartingProfession = defaultStartingProfession;
	}

	for(unsigned int r = 0; r < numRows; ++r)
	{
		std::string const & templateName = datatable->getStringValue(templateNameColumn, r);
		std::string const & templateList = datatable->getStringValue(templateColumn, r);
		std::string const & templateStarting = datatable->getStringValue(templateStartingColumn, r);
		int const templateUIPriority = datatable->getIntValue(templateUIPriorityColumn, r);
		bool const levelBased = (datatable->getIntValue(levelBasedColumn, r) != 0);
		
		TemplateRecord tr;

		tr.startingProfession = templateStarting;
		tr.professionName = templateName;
		tr.interfacePriority = templateUIPriority;
		tr.levelBased = levelBased;

		splitString(templateList, tr.skills);

		uint32 const templateCRC = Crc::normalizeAndCalculate(templateName.c_str());
		DEBUG_FATAL(m_templateData.find(templateCRC) != m_templateData.end(), ("RoadmapManager::loadTemplateDatatable(): Duplicate profession template entry: %s.", templateName.c_str()));
		m_templateData.insert(std::make_pair(templateCRC, tr));
	}
}

//----------------------------------------------------------------------

void RoadmapManager::loadRoadmapDatatable()
{
	char const * const clientRoadmapDataTable = "datatables/roadmap/client_roadmap_data.iff";
	DataTable *datatable = DataTableManager::getTable(clientRoadmapDataTable, true);

	unsigned int const numRows = static_cast<unsigned int>(datatable->getNumRows());
	int roadmapNameColumn = datatable->findColumnNumber("roadmapName");
	int templateListColumn = datatable->findColumnNumber("templateList");
	int phaseCount1Column = datatable->findColumnNumber("phase1Count");
	int phaseCountsColumn = datatable->findColumnNumber("branchPhaseCounts");
	int phaseIconsColumn = datatable->findColumnNumber("phaseIcons");
	
	for(unsigned int r = 0; r < numRows; ++r)
	{
		const std::string & roadmapName = datatable->getStringValue(roadmapNameColumn, r);

		RoadmapRecord rr;
		rr.roadmapName = roadmapName;
		const std::string & templateList = datatable->getStringValue(templateListColumn, r);
		splitString(templateList, rr.templateNames);
		int phase1Count = datatable->getIntValue(phaseCount1Column, r);
		unsigned int b;
		for(b = 0; b < MAX_NUM_BRANCHES; ++b)
			rr.phaseCounts[b][0] = phase1Count;
		const std::string & phaseCountsString = datatable->getStringValue(phaseCountsColumn, r);
		std::vector<std::string> tmp;
		splitString(phaseCountsString, tmp);

		uint const phaseCountsStringSize = rr.templateNames.size() * (NUM_PHASES - 1);
		FATAL(tmp.size() != phaseCountsStringSize, ("Invalid number of comma separated values in data table (%s) row (%d) column (%s) expected (%d) got (%d).",
			clientRoadmapDataTable, r + 3, "branchPhaseCounts", phaseCountsStringSize, tmp.size()));

		int c = 0;
		for(b = 0; b < rr.templateNames.size(); ++b)
		{
			for(int p = 0; p < NUM_PHASES - 1; ++p)			
				rr.phaseCounts[b][p + 1] = atoi(tmp[c++].c_str());			
		}
		
		const std::string &phaseIconsString = datatable->getStringValue(phaseIconsColumn, r);
		splitString(phaseIconsString, rr.phaseIconPaths);

		m_roadmapData.insert(std::make_pair(Crc::normalizeAndCalculate(roadmapName.c_str()), rr));		
	}
	m_numberOfRoadmaps = numRows;
}

//----------------------------------------------------------------------

void RoadmapManager::loadColorForSkillDatatable()
{
	DataTable *datatable = DataTableManager::getTable("datatables/roadmap/color_for_skills.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(datatable->getNumRows());
	int maxXPColumn = datatable->findColumnNumber("maxXPDividedByLevel");
	int colorRColumn = datatable->findColumnNumber("colorR");
	int colorGColumn = datatable->findColumnNumber("colorG");
	int colorBColumn = datatable->findColumnNumber("colorB");

	for(unsigned int r = 0; r < numRows; ++r)
	{
		const int maxXP = datatable->getIntValue(maxXPColumn, r);
		const int colorR = datatable->getIntValue(colorRColumn, r);
		const int colorG = datatable->getIntValue(colorGColumn, r);
		const int colorB = datatable->getIntValue(colorBColumn, r);

		XPRecord xpr;

		xpr.maxXP = maxXP;
		xpr.r = static_cast<unsigned char>(colorR);
		xpr.g = static_cast<unsigned char>(colorG);
		xpr.b = static_cast<unsigned char>(colorB);
		m_xpRecordData.push_back(xpr);
	}
}

void RoadmapManager::loadItemRewardsDatatable()
{
	DataTable *datatable = DataTableManager::getTable("datatables/roadmap/item_rewards.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(datatable->getNumRows());
	int roadmapTemplateNameColumn = datatable->findColumnNumber("roadmapTemplateName");
	int roadmapSkillNameColumn = datatable->findColumnNumber("roadmapSkillName");
	int appearanceNameColumn = datatable->findColumnNumber("appearanceName");
	int stringIdColumn = datatable->findColumnNumber("stringId");
	
	if((roadmapSkillNameColumn == -1) || (appearanceNameColumn == -1) || (stringIdColumn == -1))
		return;
	
	for(unsigned int r = 0; r < numRows; ++r)
	{
		const std::string & roadmapTemplateName = datatable->getStringValue(roadmapTemplateNameColumn, r);
		const std::string & roadmapSkillName = datatable->getStringValue(roadmapSkillNameColumn, r);
		const std::string & appearanceName = datatable->getStringValue(appearanceNameColumn, r);
		const std::string & stringIdName = datatable->getStringValue(stringIdColumn, r);

		ItemRewardRecord irr;
		char tmp[512];
		sprintf(tmp, "%s_%s", roadmapTemplateName.c_str(), roadmapSkillName.c_str());
		irr.appearanceName = appearanceName;
		irr.stringId = stringIdName;

		m_itemRewardData.insert(std::make_pair(Crc::normalizeAndCalculate(tmp), irr));		
	}
	m_numberOfRoadmaps = numRows;
}

//----------------------------------------------------------------------

int RoadmapManager::getSkillIndex(const std::string &templateName, const std::string &workingSkill)
{
	std::map<uint32, TemplateRecord>::iterator i = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(i == m_templateData.end())
		return -1;
	TemplateRecord const &tr = i->second;
	int c = 0;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv, ++c)
	{
		std::string const &s = *iv;
		if(_stricmp(s.c_str(), workingSkill.c_str()) == 0)
			return c;
	}
	return -1;
}

//----------------------------------------------------------------------

int RoadmapManager::getCurrentPhase(const std::string &roadmapName, const std::string &templateName, const std::string &workingSkill, bool addOne)
{
	int index = getSkillIndex(templateName, workingSkill);
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
		return -1;
	int branchNumber = -1;
	RoadmapRecord const &rr = i->second;
	int c = 0;
	for(std::vector<std::string>::const_iterator bi = rr.templateNames.begin(); bi != rr.templateNames.end(); ++bi)
	{
		std::string const &curTemplateName = *bi;
		if(_stricmp(templateName.c_str(), curTemplateName.c_str()) == 0)
		{
			branchNumber = c;
			break;
		}
		c++;
	}
	if(branchNumber == -1)
		return -1;

	int targetValue = addOne ? 1 : 0;
	for(int p = 0; p < NUM_PHASES; ++p)
	{
		index -= rr.phaseCounts[branchNumber][p];
		if(index < targetValue)
			return p;
	}
	return NUM_PHASES;
}

//----------------------------------------------------------------------

bool RoadmapManager::hasPlayerBranched()
{
	return true;
}

//----------------------------------------------------------------------

bool RoadmapManager::hasPlayerBranched(const std::string &roadmapName, const std::string &templateName, const std::string &workingSkill)
{
	UNREF(roadmapName);
	UNREF(templateName);
	UNREF(workingSkill);
	return true;
}

//----------------------------------------------------------------------

int RoadmapManager::getPlayerBranch()
{
	if(Game::getSinglePlayer())
		return 1;

	PlayerObject *player = Game::getPlayerObject();
	if(!player)
		return -1;
	std::string const & templateName = CuiSkillManager::getSkillTemplate();
	return getPlayerBranch(getRoadmapNameForTemplateName(templateName), templateName);
}

//----------------------------------------------------------------------

int RoadmapManager::getPlayerBranch(const std::string &roadmapName, const std::string &templateName)
{
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
	{
		DEBUG_WARNING(true, ("getPlayerBranch processed '%s' and '%s', returning 0, couldn't find roadmap", roadmapName.c_str(), templateName.c_str()));
		return -1;
	}
	RoadmapRecord const &rr = i->second;
	int c = 0;
	for(std::vector<std::string>::const_iterator bi = rr.templateNames.begin(); bi != rr.templateNames.end(); ++bi)
	{
		std::string const &curTemplateName = *bi;
		if(_stricmp(templateName.c_str(), curTemplateName.c_str()) == 0)		
			return c;		
		c++;
	}
	DEBUG_WARNING(true, ("getPlayerBranch processed '%s' and '%s' returning -1, couldn't find template in list", roadmapName.c_str(), templateName.c_str()));
	return -1;
}

//----------------------------------------------------------------------

//Returns a list of all the skills in that phase of that template in the vector out
void RoadmapManager::getSkillList(const std::string &roadmapName, int track, int phase, std::vector<std::string> &out, std::vector<int> &xpOut,
								  int &xpToFirstSkill)
{
	std::string const & templateName = getTemplateForRoadmapTrack(roadmapName, track);
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
		return;
	RoadmapRecord const &rr = i->second;
	
	int startIndex = 0;
	int finishIndex = 0;
	for(int indexCounter = 0; indexCounter < phase; ++indexCounter)
	{
		startIndex += rr.phaseCounts[track][indexCounter];
	}
	finishIndex = startIndex + rr.phaseCounts[track][phase];

	
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return;
	TemplateRecord const &tr = it->second;
	
	int c = 0;
	xpToFirstSkill = 0;
	int xpSoFar = 0;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv, ++c)
	{
		if(c == finishIndex)
			break;
		std::string const &skillName = *iv;
		CrcLowerString crcSkillName(skillName.c_str());
		if(c < startIndex)
		{
			int xpValue = LevelManager::getSkillXpValue(crcSkillName);
			xpSoFar += xpValue;			
			xpToFirstSkill += xpValue;
		}
		else
		{		
			out.push_back(skillName);
			xpSoFar += LevelManager::getSkillXpValue(crcSkillName);
			xpOut.push_back(xpSoFar);
		}
	}
	return;
}

//----------------------------------------------------------------------

std::string const &RoadmapManager::getPlayerSkillTemplate()
{
#if PRODUCTION == 0
	static const std::string sp("smuggler_2a");
	if(Game::getSinglePlayer())
		return sp;
#endif

	PlayerObject *player = Game::getPlayerObject();
	if(!player)
		return s_emptyString;
	return CuiSkillManager::getSkillTemplate();
}

//----------------------------------------------------------------------

std::string const &RoadmapManager::getPlayerWorkingSkill()
{
#if PRODUCTION == 0
	static const std::string sp("combat_marksman_pistol_01");
	if(Game::getSinglePlayer())
		return sp;
#endif
	
	PlayerObject *player = Game::getPlayerObject();
	if(!player)
		return s_emptyString;

	return CuiSkillManager::getWorkingSkill();
}

//----------------------------------------------------------------------

std::string const &RoadmapManager::getTemplateForRoadmapTrack(std::string const &roadmapName, int track)
{
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
		return s_emptyString;
	RoadmapRecord const &rr = i->second;
	if(track >= static_cast<int>(rr.templateNames.size()))
		return s_emptyString;
	return rr.templateNames[track];
}

//----------------------------------------------------------------------

void RoadmapManager::splitString(std::string const &source, stdvector<std::string>::fwd &out)
{
	std::string::size_type curPos = 0;
	while(curPos != std::string::npos)
	{
		std::string::size_type nextPos = source.find(',', curPos);
		std::string skillName = source.substr(curPos, nextPos - curPos);
		out.push_back(skillName);
		curPos = nextPos;
		if(curPos != std::string::npos)
			curPos++;
	}
}

//----------------------------------------------------------------------

void RoadmapManager::getCurrentColorForSkill(const std::string &skillName, unsigned char &r, unsigned char &g, unsigned char &b)
{
	const SkillObject * const skill = SkillManager::getInstance ().getSkill (skillName);
	if(!skill)
	{
		r = g = b = 255;
		return;
	}
	const SkillObject::ExperiencePair * xpInfo = skill->getPrerequisiteExperience();
	if(!xpInfo)
	{
		r = g = b = 255;
		return;
	}
	const int expValue = xpInfo->second.first;

	CreatureObject *player = Game::getPlayerCreature();
	int level = player->getLevel();
	float lookupValue = (expValue + 0.0f) / level;

	for(std::vector<XPRecord>::iterator i = m_xpRecordData.begin(); i != m_xpRecordData.end(); ++i)
	{
		const XPRecord &xpr = *i;
		if(xpr.maxXP >= lookupValue)
		{
			r = xpr.r;
			g = xpr.g;
			b = xpr.b;
			return;
		}
	}
	return;
}

//----------------------------------------------------------------------
	
int RoadmapManager::getNumberOfBranchesInRoadmap(const std::string &roadmapName)
{
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
		return -1;
	RoadmapRecord const &rr = i->second;
	return rr.templateNames.size();
}


//----------------------------------------------------------------------

void RoadmapManager::getStartingProfessions(std::vector<std::string> & startingProfessionVector, bool const prioritizeList)
{
	if (prioritizeList) 
	{
		std::multimap<int, uint32> priorityMap;

		for (std::map<uint32, TemplateRecord>::const_iterator i = m_templateData.begin(); i != m_templateData.end(); ++i)
		{
			TemplateRecord const & rec = i->second;

			if (rec.startingProfession.empty()) 
				continue;

			priorityMap.insert(std::make_pair(rec.interfacePriority, i->first));
		}

		for (std::multimap<int, uint32>::const_iterator itPri = priorityMap.begin(); itPri != priorityMap.end(); ++itPri) 
		{
			uint32 crc = itPri->second;
			TemplateRecord const & rec = m_templateData[ crc ];
			startingProfessionVector.push_back(rec.startingProfession);
		}
	}
	else
	{
		for (std::map<uint32, TemplateRecord>::const_iterator i = m_templateData.begin(); i != m_templateData.end(); ++i)
		{
			TemplateRecord const & rec = i->second;

			if (rec.startingProfession.empty()) 
				continue;

			startingProfessionVector.push_back(rec.startingProfession);
		}
	}
}

//----------------------------------------------------------------------
// Be warned, this is O(n).

void RoadmapManager::getStartingProfessionTemplateAndSkill(std::string const & startingProfessionName, std::string & startingTemplate, std::string & startingSkill)
{
	for (std::map<uint32, TemplateRecord>::const_iterator i = m_templateData.begin(); i != m_templateData.end(); ++i)
	{
		TemplateRecord const & rec = i->second;
		if (startingProfessionName == rec.startingProfession)
		{
			startingTemplate = rec.professionName;

			if (!rec.skills.empty()) 
			{
				startingSkill = rec.skills[0];
			}

			break;
		}
	}
}

//----------------------------------------------------------------------

void RoadmapManager::getStartingProfessionSkill(std::string & startingTemplate, std::string & startingSkill)
{
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(startingTemplate.c_str()));
	if(it == m_templateData.end())
		return;
	TemplateRecord const &tr = it->second;
	startingSkill = tr.skills[0];
}

//----------------------------------------------------------------------

std::string RoadmapManager::getDefaultStartingProfession()
{
	return m_defaultStartingProfession;
}
	
int RoadmapManager::getNumberOfRoadmaps()
{
	return m_numberOfRoadmaps;
}

//----------------------------------------------------------------------
	
void RoadmapManager::getRoadmapList(stdvector<std::string>::fwd &out)
{
	std::map<uint32, RoadmapRecord>::iterator i;
	for(i = m_roadmapData.begin(); i != m_roadmapData.end(); ++i)
	{
		RoadmapRecord const &rr = i->second;
		out.push_back(rr.roadmapName);
	}
	return;
}

//----------------------------------------------------------------------
	
bool RoadmapManager::playerIsOnRoadmap()
{
	PlayerObject const * player = Game::getPlayerObject();
	if(!player)
	{
		return false;
	}
	std::string const & templateName = CuiSkillManager::getSkillTemplate();

	
	if(templateName.empty() || RoadmapManager::getNumberOfBranchesInRoadmap(RoadmapManager::getRoadmapNameForTemplateName(templateName)) == -1 ||
		RoadmapManager::getPlayerBranch() == -1)
	{
		return false;
	}
	return true;
}

//----------------------------------------------------------------------
	
std::vector<std::string> const &RoadmapManager::getPhaseIconList(const std::string &roadmapName)
{
	static std::vector<std::string> emptyVector;
	std::map<uint32, RoadmapRecord>::iterator i = m_roadmapData.find(Crc::normalizeAndCalculate(roadmapName.c_str()));
	if(i == m_roadmapData.end())
		return emptyVector;
	return i->second.phaseIconPaths;
}

//----------------------------------------------------------------------

const std::string &RoadmapManager::getItemRewardAppearance(const std::string &templateName, const std::string &skillName)
{
	char tmp[512];
	sprintf(tmp, "%s_%s", templateName.c_str(), skillName.c_str());
	std::map<uint32, ItemRewardRecord>::iterator i = m_itemRewardData.find(Crc::normalizeAndCalculate(tmp));
	if(i == m_itemRewardData.end())
		return s_emptyString;
	return i->second.appearanceName;
}

//----------------------------------------------------------------------
	
const std::string &RoadmapManager::getItemRewardTextId(const std::string &templateName, const std::string &skillName)
{
	char tmp[512];
	sprintf(tmp, "%s_%s", templateName.c_str(), skillName.c_str());
	std::map<uint32, ItemRewardRecord>::iterator i = m_itemRewardData.find(Crc::normalizeAndCalculate(tmp));
	if(i == m_itemRewardData.end())
		return s_emptyString;
	return i->second.stringId;
}

//----------------------------------------------------------------------

bool RoadmapManager::playerIsNewCharacter()
{
	std::string const &workingSkill = getPlayerWorkingSkill();
	if(_stricmp(workingSkill.c_str(), NEW_CHARACTER_WORKING_SKILL) == 0)
		return true;
	return false;
}

//----------------------------------------------------------------------

const std::string &RoadmapManager::getPlayerSpaceTemplateName()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return s_emptyString;
	for(int i = 0; i < NUM_SPACE_TEMPLATES; ++i)
	{
		const std::string &templateName = s_spaceTemplateNames[i];
		std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
		if(it == m_templateData.end())
			return s_emptyString;
		TemplateRecord const &tr = it->second;
		if(tr.skills.size() == 0)
			return s_emptyString;
		SkillObject const *skill = SkillManager::getInstance().getSkill(*tr.skills.begin());
		if(skill && player->hasSkill(*skill))
		{
			return templateName;
		}		
	}
	return s_emptyString;
}


//----------------------------------------------------------------------

const std::string &RoadmapManager::getPlayerSpaceWorkingSkill()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return s_emptyString;
	
	const std::string &templateName = getPlayerSpaceTemplateName();
	
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return s_emptyString;
	TemplateRecord const &tr = it->second;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv)
	{
		std::string const &skillName = *iv;
		SkillObject const *skill = SkillManager::getInstance().getSkill(skillName);
		if(skill && !player->hasSkill(*skill))
			return skillName;
	}
	return s_emptyString;
}

void RoadmapManager::getSkillList(const std::string &templateName, std::vector<std::string> &out, std::vector<int> &xpOut, int &xpToFirstSkill)
{
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return;
	TemplateRecord const &tr = it->second;
	
	xpToFirstSkill = 0;
	int xpSoFar = 0;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv)
	{
		std::string const &skillName = *iv;
		CrcLowerString crcSkillName(skillName.c_str());
		out.push_back(skillName);
		xpSoFar += LevelManager::getSkillXpValue(crcSkillName);		
		xpOut.push_back(xpSoFar);
	}
	return;
}

//----------------------------------------------------------------------

const std::string &RoadmapManager::getPlayerPoliticianWorkingSkill()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return s_emptyString;
	
	const std::string &templateName = s_politicianTemplateName;
	
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return s_emptyString;
	TemplateRecord const &tr = it->second;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv)
	{
		std::string const &skillName = *iv;
		SkillObject const *skill = SkillManager::getInstance().getSkill(skillName);
		if(skill && !player->hasSkill(*skill))
			return skillName;
	}
	return s_emptyString;
}	

//----------------------------------------------------------------------

const std::string &RoadmapManager::getPlayerStorytellerWorkingSkill()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return s_emptyString;

	const std::string &templateName = s_storytellerTemplateName;

	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return s_emptyString;
	TemplateRecord const &tr = it->second;
	for(std::vector<std::string>::const_iterator iv = tr.skills.begin(); iv != tr.skills.end(); ++iv)
	{
		std::string const &skillName = *iv;
		SkillObject const *skill = SkillManager::getInstance().getSkill(skillName);
		if(skill && !player->hasSkill(*skill))
			return skillName;
	}
	return s_emptyString;
}	


//----------------------------------------------------------------------

const void RoadmapManager::getLevelLimits(int phase, int &outStartLevel, int &outEndLevel)
{
	if((phase < 0) || (phase > 3))
		DEBUG_FATAL(true, ("bad phase in RoadmapManager::getLevelLimits %d", phase));
	outStartLevel = s_phaseLimits[phase];
	outEndLevel = s_phaseLimits[phase + 1];
}

//----------------------------------------------------------------------

const std::string RoadmapManager::getSkillForLevel(const std::string &templateName, int level)
{
	std::vector<std::string> skills;
	std::vector<int> dummy;
	int dummy2;
	int indx = 0;
	while(s_skillUps[indx] < level)
		indx++;
	if(s_skillUps[indx] != level)
		return s_emptyString;	
	getSkillList(templateName, skills, dummy, dummy2);
	return skills[indx];
}

//----------------------------------------------------------------------

bool RoadmapManager::isLevelBasedTemplate(const std::string &templateName)
{
	std::map<uint32, TemplateRecord>::iterator it = m_templateData.find(Crc::normalizeAndCalculate(templateName.c_str()));
	if(it == m_templateData.end())
		return false;
	TemplateRecord const &tr = it->second;
	return tr.levelBased;
}

//----------------------------------------------------------------------

int RoadmapManager::getPlayerCurrentPhase()
{	
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return 0;
	int level = player->getLevel();
	for(int i = 0; i < 4; ++i)
	{
		if(level < s_phaseLimits[i+1])
			return i;
	}
	return 3;
}

//----------------------------------------------------------------------

float RoadmapManager::getPlayerXpProgressForCurrentPhase()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return 0.0f;
	int level = player->getLevel();
	int expCurrent = player->getLevelXp();

	LevelManager::LevelData levelData;
	LevelManager::setLevelDataFromXp(levelData, expCurrent);

	int16 const nextLevel = static_cast<int16>(levelData.currentLevel + 1);

	int const currentCap = LevelManager::getRequiredXpToReachLevel(levelData.currentLevel);
	int const nextCap = LevelManager::getRequiredXpToReachLevel(nextLevel);

	int curPhase = getPlayerCurrentPhase();
	int startLevel, endLevel;
	getLevelLimits(curPhase, startLevel, endLevel);
	if(endLevel <= startLevel)
		return 1.0f;
	float amountPerLevel = 1.0f / (endLevel - startLevel);
	float amountForLevel;
	if(nextCap > currentCap)
		amountForLevel = (expCurrent - currentCap) / (nextCap - currentCap + 0.0f);
	else  //This happens when nextCap == currentCap, which is true when the player is at the level cap
		amountForLevel = 0.0f;
	return (level - startLevel) * amountPerLevel + amountForLevel * amountPerLevel;
}

//----------------------------------------------------------------------

const std::string RoadmapManager::getNextSkillForLevelTemplate()
{
	CreatureObject *player = Game::getPlayerCreature();
	if(!player)
		return s_emptyString;
	int level = player->getLevel();
	const std::string &templateName = CuiSkillManager::getSkillTemplate();;
	while(level < LevelManager::getMaxLevel())
	{
		std::string skillName = getSkillForLevel(templateName, level);
		if(!skillName.empty())
			return skillName;
		level++;
	}
	return s_emptyString;
}
