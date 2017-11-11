// ======================================================================
//
// CuiSkillManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSkillManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientEffectManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiStringIdsSkill.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/DraftSchematicGroupManager.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedObject/Controller.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include "UnicodeUtils.h"

#include <unordered_map>

// ======================================================================

namespace CuiSkillManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiSkillManager::Messages::SelectedCurrentWorkingSkillChanged::Payload &, CuiSkillManager::Messages::SelectedCurrentWorkingSkillChanged>
			selectedCurrentWorkingSkillChanged;
		MessageDispatch::Transceiver<const CuiSkillManager::Messages::ProfessionTemplateChanged::Payload &, CuiSkillManager::Messages::ProfessionTemplateChanged>
			professionTemplateChanged;
	}

	//----------------------------------------------------------------------

	class MyCallback : public MessageDispatch::Callback
	{
	public:

		MyCallback () :
		MessageDispatch::Callback ()
		{
			connect (*this, &MyCallback::onExpChanged,    static_cast<PlayerObject::Messages::ExperienceChanged  *> (0));
		}

		//----------------------------------------------------------------------

		void onExpChanged (const PlayerObject & creature)
		{
			if (&creature == static_cast<const Object *>(Game::getPlayerObject ()))
				CuiSkillManager::updateExperience (true);
		}
	};

	//----------------------------------------------------------------------

	MyCallback * s_callback = 0;

	//-----------------------------------------------------------------

	bool s_installed = false;

	//----------------------------------------------------------------------

	void testSchematicGroup (const std::string & group, const std::string & skillName, const std::string & groupType, bool verbose)
	{
		UNREF (verbose);

		static DraftSchematicGroupManager::SchematicVector drafts;
		drafts.clear ();
		
		if (!DraftSchematicGroupManager::getSchematicsForGroup (group, drafts))
		{
			WARNING (true, ("CuiSkillManager::testSkillSystem: skill [%s] calls for invalid schematic %s group [%s]", skillName.c_str (), groupType.c_str (), group.c_str ()));
			return;
		}
		
		static Unicode::String header;
		static Unicode::String desc;
		static Unicode::String attribs;				
		static std::string modifiedDraftName;

		for (DraftSchematicGroupManager::SchematicVector::const_iterator dit = drafts.begin (); dit != drafts.end (); ++dit)
		{
			const std::pair<uint32, uint32> & draftCrc = *dit;
			
			const char * const sharedDraftTemplateName = ObjectTemplateList::lookUp(draftCrc.second).getString();
			
			WARNING (!sharedDraftTemplateName, ("CuiSkillManager::testSkillSystem: skill [%s] schematic %s group [%s] [%ld] unable to lookup template name", skillName.c_str (), groupType.c_str (), group.c_str (), draftCrc.first));
			
			DEBUG_REPORT_LOG_PRINT (verbose && sharedDraftTemplateName, ("--------------- CuiSkillManager test [%s] draft [%s]\n", skillName.c_str (), sharedDraftTemplateName));
			
			const DraftSchematicInfo * const dsi = DraftSchematicManager::cacheDraftSchematic (draftCrc);
			
			if (dsi)
			{						
				header.clear ();
				desc.clear ();
				attribs.clear ();
				
				dsi->formatDescription (header, desc, attribs, false);
			}
			else
			{
				WARNING (true, ("CuiSkillManager::testSkillSystem: unable to find draft schematic [%lu] for %s group [%s] on skill [%s]", draftCrc.first, groupType.c_str (), group.c_str (), skillName.c_str ()));
			}
		}
	}

	bool s_enableEffects = false;

	const std::string s_emptyString;
}

using namespace CuiSkillManagerNamespace;

//-----------------------------------------------------------------

std::string CuiSkillManager::m_skillTemplate;
std::string CuiSkillManager::m_workingSkill;

//----------------------------------------------------------------------

const std::string CuiSkillManager::StringTables::SkillMod::names   = "stat_n";
const std::string CuiSkillManager::StringTables::SkillMod::descs   = "stat_d";
const std::string CuiSkillManager::StringTables::SkillMod::extendedDescs   = "stat_ed";

const std::string CuiSkillManager::StringTables::Skill::names   = "skl_n";
const std::string CuiSkillManager::StringTables::Skill::descs   = "skl_d";
const std::string CuiSkillManager::StringTables::Skill::titles  = "skl_t";
const std::string CuiSkillManager::StringTables::Skill::categoryNames = "skl_cat_n";

const std::string CuiSkillManager::StringTables::Exp::names     = "exp_n";
const std::string CuiSkillManager::StringTables::Exp::descs     = "exp_d";

const std::string CuiSkillManager::StringTables::Cmd::names     = "cmd_n";
const std::string CuiSkillManager::StringTables::Cmd::descs     = "cmd_d";

const std::string CuiSkillManager::StringTables::Stats::names   = "stat_n";
const std::string CuiSkillManager::StringTables::Stats::descs   = "stat_d";

//-----------------------------------------------------------------

void CuiSkillManager::install ()
{
	DEBUG_FATAL (s_installed, ("already installed"));
	s_installed = true;

	s_callback  = new MyCallback;
}

//-----------------------------------------------------------------

void CuiSkillManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	s_installed = false;

	delete s_callback;
	s_callback  = 0;

	s_enableEffects = false;
}

//-----------------------------------------------------------------

void CuiSkillManager::updateExperience (bool showMessage)
{
	UNREF(showMessage);
	//nothing to do now
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillName        (const std::string & skillName, Unicode::String & str, bool forceEnglish)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::Skill::names, skillName).localize (str, forceEnglish);
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillDescription (const std::string & skillName, Unicode::String & str)
{
	return StringId (StringTables::Skill::descs, skillName).localize (str);
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillTitle (const std::string & title, Unicode::String & str)
{
	if (title.empty())
	{
		str.clear ();
		return false;
	}
	return StringId (StringTables::Skill::titles, title).localize (str);
}

//-----------------------------------------------------------------

bool CuiSkillManager::localizeSkillName        (const SkillObject & skill, Unicode::String & str, bool forceEnglish)
{
	return localizeSkillName (skill.getSkillName (), str, forceEnglish);
}

//-----------------------------------------------------------------

bool CuiSkillManager::localizeSkillDescription (const SkillObject & skill, Unicode::String & str)
{
	return localizeSkillDescription (skill.getSkillName (), str);
}

//-----------------------------------------------------------------

bool CuiSkillManager::localizeExpName          (const std::string & expName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::Exp::names, expName).localize (str);
}

//-----------------------------------------------------------------

bool CuiSkillManager::localizeExpDescription   (const std::string & expName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::Exp::descs, expName).localize (str);
}

//---------------------------------------------------------------------

bool CuiSkillManager::localizeCmdName          (const std::string & cmdName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::Cmd::names, cmdName).localize (str);
}

//-----------------------------------------------------------------

bool CuiSkillManager::localizeCmdDescription   (const std::string & cmdName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::Cmd::descs, cmdName).localize (str);
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillModName     (const std::string & skillModName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::SkillMod::names, skillModName).localize (str);
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillModDesc     (const std::string & skillModName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::SkillMod::descs, skillModName).localize (str);
}

//----------------------------------------------------------------------

bool CuiSkillManager::localizeSkillModExtendedDesc     (const std::string & skillModName, Unicode::String & str)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return StringId (StringTables::SkillMod::extendedDescs, skillModName).localize (str);
}

//----------------------------------------------------------------------

void CuiSkillManager::formatSkillExpDescription   (const SkillObject & skill, Unicode::String & str, bool showPlayerInfo, bool useLineBreak)
{
	static StringVector sv;
	sv.clear ();

	formatSkillExpDescriptions (skill, sv, showPlayerInfo, useLineBreak);

	if (!sv.empty ())
		str = sv.front ();
}

//----------------------------------------------------------------------

void CuiSkillManager::formatSkillExpDescriptions (const SkillObject & skill, StringVector & sv, bool showPlayerInfo, bool useLineBreak)
{
	const CreatureObject * const player = Game::getPlayerCreature ();

	static Unicode::String tmp;
	static Unicode::String tmp2;
	static Unicode::String localizedExpName;

	const SkillObject::ExperienceVector & ev = skill.getPrerequisiteExperienceVector ();

	for (SkillObject::ExperienceVector::const_iterator it = ev.begin (); it != ev.end (); ++it)
	{
		const std::string & expName  = (*it).first;
		const int           expValue = (*it).second.first;

		localizedExpName.clear ();
		CuiSkillManager::localizeExpName (expName, localizedExpName);
		CuiStringVariablesData csvd;
		csvd.sourceName = localizedExpName;
		csvd.digit_i    = expValue;
		tmp.clear ();
		CuiStringVariablesManager::process (CuiStringIdsSkill::acquire_exp_prose, csvd, tmp);

		if (player && showPlayerInfo)
		{
			int playerExp = 0;
			if (!player->getExperience (expName, playerExp))
				playerExp = 0;

			csvd.digit_i    = playerExp;
			tmp2.clear ();
			CuiStringVariablesManager::process (CuiStringIdsSkill::exp_prose, csvd, tmp2);

			if (useLineBreak)
				tmp.push_back ('\n');
			else
				tmp.append (2, ' ');

			tmp.append (tmp2);
		}

		sv.push_back (tmp);
	}
}

//----------------------------------------------------------------------

void CuiSkillManager::testSkillSystem (bool verbose)
{
	UNREF (verbose);

	const SkillManager::SkillMap & skillMap = SkillManager::getInstance ().getSkillMap ();

	Unicode::String tmpStr;
	int index = 0;
	for (SkillManager::SkillMap::const_iterator skillIterator = skillMap.begin (); skillIterator != skillMap.end (); ++skillIterator, ++index)
	{
		tmpStr.clear ();

		const std::string & skillName   = (*skillIterator).first;
		const SkillObject * const skill = (*skillIterator).second;

		if (skillName.empty ())
			WARNING (true, ("CuiSkillManager::testSkillSystem: %3d empty skill name found", index));

		if (!skill)
		{
			WARNING (true, ("CuiSkillManager::testSkillSystem: %3d null skill found for [%s]", index, skillName.c_str ()));
			continue;
		}
			
		DEBUG_REPORT_LOG_PRINT (verbose, ("--------------- CuiSkillManager test [%s]\n", skillName.c_str ()));

		CuiSkillManager::localizeSkillName        (*skill, tmpStr);
		CuiSkillManager::localizeSkillDescription (*skill, tmpStr);
		
		if (skill->isTitle ())
			CuiSkillManager::localizeSkillTitle        (skillName, tmpStr);
		
		{
			const SkillObject::StringVector & cmds = skill->getCommandsProvided ();
			for (SkillObject::StringVector::const_iterator cit = cmds.begin (); cit != cmds.end (); ++cit)
			{
				const std::string & cmd = Unicode::toLower (*cit);
				
				CuiSkillManager::localizeCmdName          (cmd, tmpStr);
				CuiSkillManager::localizeCmdDescription   (cmd, tmpStr);
			}
		}
		
		{
			const SkillObject::StringVector & draftGroupsGranted = skill->getSchematicsGranted ();
			for (SkillObject::StringVector::const_iterator dit = draftGroupsGranted.begin (); dit != draftGroupsGranted.end (); ++dit)
			{
				const std::string & group = Unicode::toLower (*dit);
				testSchematicGroup (group, skillName, "granted", verbose);
			}
		}

		{
			bool found = false;
			const SkillObject::ExperienceVector & expVector = skill->getPrerequisiteExperienceVector ();
			for (SkillObject::ExperienceVector::const_iterator it = expVector.begin (); it != expVector.end (); ++it)
			{
				WARNING (found, ("CuiSkillManager::testSkillSystem: %3d skill [%s] has multiple exp requirements", index, skillName.c_str ()));
				found = true;
				
				const std::string & expName = (*it).first;
				CuiSkillManager::localizeExpName          (expName, tmpStr);
				CuiSkillManager::localizeExpDescription   (expName, tmpStr);
			}
		}
		
		{
			const SkillObject::GenericModVector & modVector = skill->getStatisticModifiers ();
			for (SkillObject::GenericModVector::const_iterator it = modVector.begin (); it != modVector.end (); ++it)
			{
				const SkillObject::GenericMod & mod = *it;
				const std::string & modName = mod.first;
				const int modValue          = mod.second;
				
				//-- skip private skill mods
				if (_strnicmp (modName.c_str (), "private_", 8))
				{					
					CuiSkillManager::localizeSkillModName     (modName, tmpStr);
					CuiSkillManager::localizeSkillModDesc     (modName, tmpStr);
				}
				
				WARNING (modValue <= 0, ("CuiSkillManager::testSkillSystem: %3d skill [%s] modifier [%s] has invalid value [%d]", index, skillName.c_str (), modName.c_str (), modValue));
			}
		}
	}

	{
		CommandTable::CommandMap const &  cmdMap = CommandTable::getCommandMap  ();
		for (CommandTable::CommandMap::const_iterator it = cmdMap.begin (); it != cmdMap.end (); ++it)
		{
			const Command & cmd = (*it).second;

			if (cmd.m_visibleToClients > 0)
			{
				const std::string & cmdName = Unicode::toLower (cmd.m_commandName);
				tmpStr.clear ();
				CuiSkillManager::localizeCmdName          (cmdName, tmpStr);
				CuiSkillManager::localizeCmdDescription   (cmdName, tmpStr);
				
				if (cmd.m_visibleToClients > 2)
				{
					CuiDragInfo cdi;
					cdi.type = CuiDragInfoTypes::CDIT_command;
					cdi.str  = std::string ("/") + cmdName;
					CuiIconManager::findIconImageStyle (cdi);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

std::string const & CuiSkillManager::getSkillTemplate()
{
	if(m_skillTemplate.empty())
	{
		PlayerObject *obj = Game::getPlayerObject();
		if(obj)
		{
			m_skillTemplate = obj->getSkillTemplate();
		}
	}
	return m_skillTemplate;
}

// ----------------------------------------------------------------------

void CuiSkillManager::clearCachedSkillTemplate()
{
	m_skillTemplate = s_emptyString;
}

// ----------------------------------------------------------------------

void CuiSkillManager::setSkillTemplate(std::string const & templateName, bool sendToServer)
{
	if (m_skillTemplate != templateName)
	{
		m_skillTemplate = templateName;

		if(sendToServer)
		{		
			Object * const player = Game::getPlayer();
			Controller * const controller = player ? player->getController() : NULL;
			if (controller)				
			{
				MessageQueueSelectProfessionTemplate * const msgSetProfessionTemplate = new MessageQueueSelectProfessionTemplate(m_skillTemplate);
				controller->appendMessage(	CM_setProfessionTemplate, 
											0.0f, 
											msgSetProfessionTemplate, 
											GameControllerMessageFlags::SEND |
											GameControllerMessageFlags::RELIABLE |
											GameControllerMessageFlags::DEST_AUTH_SERVER |
											GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}
			
		}
		triggerSkillEffects();
		Transceivers::professionTemplateChanged.emitMessage(m_skillTemplate);
	}
}

// ----------------------------------------------------------------------

std::string const & CuiSkillManager::getWorkingSkill()
{
	return m_workingSkill;
}

// ----------------------------------------------------------------------

void CuiSkillManager::setWorkingSkill(std::string const & skillName, bool sendToServer)
{
	if (m_workingSkill != skillName) 
	{
		m_workingSkill = skillName;

		if(sendToServer)
		{		
			Object * const player = Game::getPlayer();
			Controller * const controller = player ? player->getController() : NULL;
			if (controller)				
			{
				MessageQueueSelectCurrentWorkingSkill * const msgSetCurrentWorkingSkill = new MessageQueueSelectCurrentWorkingSkill(m_workingSkill);

				controller->appendMessage(	CM_setCurrentWorkingSkill, 
											0.0f, 
											msgSetCurrentWorkingSkill, 
											GameControllerMessageFlags::SEND |
											GameControllerMessageFlags::RELIABLE |
											GameControllerMessageFlags::DEST_AUTH_SERVER |
											GameControllerMessageFlags::DEST_AUTH_CLIENT);
			}

			Transceivers::selectedCurrentWorkingSkillChanged.emitMessage(m_workingSkill);

			triggerSkillEffects();
		}
	}
}

//----------------------------------------------------------------------

SkillObject const * CuiSkillManager::getWorkingSkillObject()
{
	return m_workingSkill.empty() ? NULL : SkillManager::getInstance().getSkill(m_workingSkill);
}

//----------------------------------------------------------------------

void CuiSkillManager::enableEffects(bool const enabled)
{
	s_enableEffects = enabled;
}

//----------------------------------------------------------------------

void CuiSkillManager::triggerSkillEffects()
{
	if (s_enableEffects) 
	{
		Object * const player = Game::getPlayer();
		if (player) 
		{
			static const ConstCharCrcLowerString c("clienteffect/qualify_for_skill.cef");
			ClientEffectManager::playClientEffect(c, player, CrcLowerString::empty);
		}
		s_enableEffects = false;
	}
}

// ======================================================================

