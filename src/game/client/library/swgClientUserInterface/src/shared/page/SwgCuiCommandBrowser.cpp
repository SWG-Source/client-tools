///======================================================================
//
// SwgCuiCommandBrowser.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandBrowser.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "utf8.h"
#include "clientGame/ClientMacroManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatterSpeechManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiDragManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiMessageQueueManager.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "fileInterface/StdioFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/SocialsManager.h"
#include "sharedInputMap/InputMap.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiNewMacro.h"
#include <algorithm>
#include <ctype.h>
#include <list>
#include <map>

//======================================================================

namespace
{
	static const char * s_command_breaker = "+";
	static const char * s_chatterSpeech = "chatterSpeech";

	uint32 const s_displayGroupAstromech = Crc::normalizeAndCalculate ("astromech");
	uint32 const s_displayGroupCostume = Crc::normalizeAndCalculate("costume");

	bool isCommandForPage(SwgCuiCommandBrowser::TabType tabType, uint32 const displayGroupSpace, std::string const & cmd)
	{
		size_t endpos = 0;
		std::string cmd_first_word = cmd;
		IGNORE_RETURN(Unicode::getFirstToken (cmd, 0, endpos, cmd_first_word, s_command_breaker));

		const Command & command = CommandTable::getCommand (Crc::normalizeAndCalculate (cmd_first_word.c_str ()));
		
		if (command.isNull () || command.m_visibleToClients < 2)
			return false;

		const bool isCombatCommand = CuiCombatManager::isCombatCommand (command);
		const bool isSpaceCommand = (command.m_displayGroup == displayGroupSpace) ? true : false;
		const bool isAstromechCommand = (command.m_displayGroup == s_displayGroupAstromech) ? true : false;
		const bool isCostumeCommand = (command.m_displayGroup == s_displayGroupCostume) ? true : false;
		const bool isOtherCommand = !isCombatCommand && !isSpaceCommand && !isAstromechCommand && !isCostumeCommand;

		if(tabType == SwgCuiCommandBrowser::TT_combat && isCombatCommand)
			return true;

		if(tabType == SwgCuiCommandBrowser::TT_space && isSpaceCommand)
			return true;

		if(tabType == SwgCuiCommandBrowser::TT_astromech && isAstromechCommand)
			return true;

		if (tabType == SwgCuiCommandBrowser::TT_costume && isCostumeCommand)
			return true;

		if(tabType == SwgCuiCommandBrowser::TT_other && isOtherCommand)
			return true;

		return false;
	}

	bool findCommand (const UIWidget & page, CuiDragInfo & dragInfo)
	{
		const UIBaseObject * const icon = page.GetObjectFromPath ("icon");
		if (icon)
		{
			dragInfo.set (*icon);
			return true;
		}
		return false;
	}

	void appendCommandTableCommands (SwgCuiCommandBrowser::TabType const tabType, uint32 const displayGroupSpace, std::map<std::string, int> & sv)
	{
		const CommandTable::CommandMap & cmap = CommandTable::getCommandMap  ();

		for (CommandTable::CommandMap::const_iterator it = cmap.begin (); it != cmap.end (); ++it)
		{
			const Command & cmd = (*it).second;

			if ((cmd.m_visibleToClients > 2) && (isCommandForPage(tabType, displayGroupSpace, cmd.m_commandName)))
			{
				sv.insert(std::make_pair(cmd.m_commandName, 1));
			}
		}
	}

	namespace Properties
	{
		const UILowerString LowerDisplayName = UILowerString ("LowerDisplayName");
	}

	namespace TabNames
	{			
		const std::string combat = "@ui_command:command_tab_combat";
		const std::string social = "@ui_command:command_tab_social";
		const std::string mood   = "@ui_command:command_tab_mood";
		const std::string space  = "@ui_command:command_tab_space";
		const std::string astromech  = "@ui_command:command_tab_astromech";
		const std::string chatter  = "@ui_command:command_tab_chatter";
		const std::string other  = "@ui_command:command_tab_other";
		const std::string macro  = "@ui_command:command_tab_macro";
		const std::string costume = "@ui_command:command_tab_costume";
	}
}

//----------------------------------------------------------------------

SwgCuiCommandBrowser::SwgCuiCommandBrowser (UIPage & page) :
CuiMediator           ("SwgCuiCommandBrowser", page),
UIEventCallback       (),
m_callback            (new MessageDispatch::Callback),
m_newMacroButton      (0),
m_editMacroButton     (0),
m_deleteMacroButton   (0),
m_volume              (0),
m_macroVolume         (0),
m_tabs                (0),
m_textName            (0),
m_textDesc            (0),
m_sample              (0),
m_pageInfo            (0),
m_tabType             (TT_combat),
m_displayGroupSpace   (Crc::normalizeAndCalculate ("ship"))
{
	getCodeDataObject (TUIButton,     m_newMacroButton,    "buttonNewMacro");
	getCodeDataObject (TUIButton,     m_editMacroButton,   "buttonEditMacro");
	getCodeDataObject (TUIButton,     m_deleteMacroButton, "buttonDeleteMacro");
	getCodeDataObject (TUIVolumePage, m_volume,            "volume");
	getCodeDataObject (TUIVolumePage, m_macroVolume,       "macrovolume");
	getCodeDataObject (TUITabbedPane, m_tabs,              "tabs");
	getCodeDataObject (TUIText,       m_textName,          "textName");
	getCodeDataObject (TUIText,       m_textDesc,          "textDesc");
	getCodeDataObject (TUIPage,       m_sample,            "sample");
	getCodeDataObject (TUIPage,       m_pageInfo,          "info");

	m_sample->SetVisible (false);

	setState    (MS_closeable);
	setState    (MS_closeDeactivates);

	m_tabs->SetActiveTab (-1);

	m_volume->Clear      ();
	m_macroVolume->Clear ();
	m_textName->Clear    ();
	m_textDesc->Clear    ();

	m_textName->SetPreLocalized (true);
	m_textDesc->SetPreLocalized (true);

	//- remove space tabs on the ground
	if (Game::ST_ground == Game::getHudSceneType())
	{
		UIDataSource * const ds = m_tabs->GetDataSource();
		if (NULL != ds)
		{
			{
				UIBaseObject * const child = ds->GetChild(TabNames::space.c_str());
				if (NULL != child)
					ds->RemoveChild(child);
			}

			{
				UIBaseObject * const child = ds->GetChild(TabNames::astromech.c_str());
				if (NULL != child)
					ds->RemoveChild(child);
			}
			{
				UIBaseObject * const child = ds->GetChild(TabNames::chatter.c_str());
				if (NULL != child)
					ds->RemoveChild(child);
			}
		}
	}
}

//----------------------------------------------------------------------

SwgCuiCommandBrowser::~SwgCuiCommandBrowser ()
{
	delete m_callback;
	m_callback = 0;

	m_newMacroButton      = 0;
	m_editMacroButton     = 0;
	m_deleteMacroButton   = 0;
	m_volume        = 0;
	m_macroVolume   = 0;
	m_tabs          = 0;
	m_textName      = 0;
	m_textDesc      = 0;
	m_sample        = 0;
	m_pageInfo      = 0;
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::performActivate   ()
{
	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiCommandBrowser::onCommandsChanged,        static_cast<CreatureObject::Messages::CommandsChanged *>  (0));
	m_callback->connect (*this, &SwgCuiCommandBrowser::onCommandAdded,           static_cast<CreatureObject::Messages::CommandAdded *>     (0));
	m_callback->connect (*this, &SwgCuiCommandBrowser::onCommandRemoved,         static_cast<CreatureObject::Messages::CommandRemoved *>   (0));
	m_callback->connect (*this, &SwgCuiCommandBrowser::onMacroAdded,             static_cast<ClientMacroManager::Messages::Added*>         (0));
	m_callback->connect (*this, &SwgCuiCommandBrowser::onMacroRemoved,           static_cast<ClientMacroManager::Messages::Removed*>       (0));
	m_callback->connect (*this, &SwgCuiCommandBrowser::onMacroModified,          static_cast<ClientMacroManager::Messages::Modified*>      (0));

	m_volume->AddCallback      (this);
	m_macroVolume->AddCallback (this);
	m_tabs->AddCallback        (this);

	m_newMacroButton->AddCallback    (this);
	m_editMacroButton->AddCallback   (this);
	m_deleteMacroButton->AddCallback (this);

	if (m_tabs->GetActiveTab () < 0)
	{
		m_tabs->SetActiveTab (0);
		update ();
	}
	else
	{
		reset ();
		update ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onCommandsChanged,        static_cast<CreatureObject::Messages::CommandsChanged *>  (0));
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onCommandAdded,           static_cast<CreatureObject::Messages::CommandAdded *>     (0));
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onCommandRemoved,         static_cast<CreatureObject::Messages::CommandRemoved *>   (0));
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onMacroAdded,             static_cast<ClientMacroManager::Messages::Added*>         (0));
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onMacroRemoved,           static_cast<ClientMacroManager::Messages::Removed*>       (0));
	m_callback->disconnect (*this, &SwgCuiCommandBrowser::onMacroModified,          static_cast<ClientMacroManager::Messages::Modified*>      (0));

	m_volume->RemoveCallback      (this);
	m_macroVolume->RemoveCallback (this);
	m_tabs->RemoveCallback        (this);

	m_newMacroButton->RemoveCallback    (this);
	m_editMacroButton->RemoveCallback   (this);
	m_deleteMacroButton->RemoveCallback (this);

	CuiManager::requestPointer     (false);
}

//----------------------------------------------------------------------

bool SwgCuiCommandBrowser::OnMessage (UIWidget * context, const UIMessage & msg)
{
	NOT_NULL (context);

	if (context->GetParent () == m_volume)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			if (context->IsA (TUIPage))
			{
				UIPage * const page = NON_NULL(dynamic_cast<UIPage *>(context));
				CuiDragInfo dragInfo;

				if (!findCommand (*page, dragInfo))
					WARNING (true, ("Unable to find drag command"));
				else
				{
					//DEBUG_WARNING(true, ("%s %s\n", dragInfo.cmd.c_str(), dragInfo.str.c_str()));
					IGNORE_RETURN(CuiDragManager::handleAction (dragInfo));
					return false;
				}
			}
		}
	}

	if (context->GetParent () == m_macroVolume)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			if (context->IsA (TUIPage))
			{
				UIPage * const page = NON_NULL(dynamic_cast<UIPage *>(context));
				CuiDragInfo dragInfo;

				if (!findCommand (*page, dragInfo))
					WARNING (true, ("Unable to find drag command"));
				else
				{
					IGNORE_RETURN(CuiDragManager::handleAction (dragInfo));
					return false;
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabs)
	{
		const long index = m_tabs->GetActiveTab ();
		
		std::string tabname;
		IGNORE_RETURN(m_tabs->GetTabName (index, tabname));
		
		if (!_stricmp (tabname.c_str (), TabNames::combat.c_str()))
			m_tabType = TT_combat;
		else if (!_stricmp (tabname.c_str (), TabNames::social.c_str()))
			m_tabType = TT_social;
		else if (!_stricmp (tabname.c_str (), TabNames::mood.c_str()))
			m_tabType = TT_mood;
		else if (!_stricmp (tabname.c_str (), TabNames::space.c_str()))
			m_tabType = TT_space;
		else if (!_stricmp (tabname.c_str (), TabNames::astromech.c_str()))
			m_tabType = TT_astromech;
		else if (!_stricmp (tabname.c_str (), TabNames::chatter.c_str()))
			m_tabType = TT_chatter;
		else if (!_stricmp (tabname.c_str (), TabNames::other.c_str()))
			m_tabType = TT_other;
		else if (!_stricmp (tabname.c_str (), TabNames::macro.c_str()))
			m_tabType = TT_macro;
		else if (!_stricmp (tabname.c_str (), TabNames::costume.c_str()))
			m_tabType = TT_costume;
		else
			m_tabType = TT_none;

		reset ();
		update ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::OnVolumePageSelectionChanged (UIWidget * context)
{
	if (context == m_volume)
	{
		update ();
	}

	if (context == m_macroVolume)
	{
		update ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::OnButtonPressed   (UIWidget * context)
{
	if (context == m_newMacroButton)
	{
		IGNORE_RETURN(CuiActionManager::performAction  (CuiActions::newMacro, Unicode::emptyString));
	}
	else if (context == m_editMacroButton)
	{
		UIPage * const selected = dynamic_cast<UIPage *>(m_macroVolume->GetLastSelectedChild ());
		if (selected && selected->IsVisible())
		{
			const std::string & macroName = selected->GetName ();

			const ClientMacroManager::Data * data = ClientMacroManager::findMacroData (macroName);
			const std::string & forcePauseCommand = ClientMacroManager::getForcePauseCommand();

			

			if (data)
			{
				//pull off forced pause for display
				Unicode::UTF8String command = data->commandString;
				std::string::size_type pos = command.find(forcePauseCommand);
				if(pos == 0)
				{
					command = command.substr(forcePauseCommand.size());
				}

				Unicode::UTF8String s = macroName;
				IGNORE_RETURN(s.append (1, ' '));
				s += data->userDefinedName;
				IGNORE_RETURN(s.append (1, ' '));
				s += data->icon;
				IGNORE_RETURN(s.append (1, ' '));
				s += data->color;
				IGNORE_RETURN(s.append (1, ' '));
				s += command;

				IGNORE_RETURN(CuiActionManager::performAction  (CuiActions::newMacro, Unicode::utf8ToWide (s)));
			}
			else
				WARNING (true, ("SwgCuiCommandBrowser unable to find macro data to edit [%s]", macroName.c_str ()));
		}
		else
			WARNING (true, ("SwgCuiCommandBrowser Attempt to edit macro with nothing selected."));
	}
	else if (context == m_deleteMacroButton)
	{
		UIPage * const selected = dynamic_cast<UIPage *>(m_macroVolume->GetLastSelectedChild ());
		if (selected && selected->IsVisible())
		{
			const std::string & macroName = selected->GetName ();
			IGNORE_RETURN(ClientMacroManager::eraseMacro (macroName));
			reset ();
			update();
			IGNORE_RETURN(ClientMacroManager::save());
		}
		else
			WARNING (true, ("SwgCuiCommandBrowser attempt to delete macro with nothing selected."));
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::update ()
{
	UIPage * selected = 0;
		
	if (m_tabType == TT_macro)
	{
		selected = dynamic_cast<UIPage *>(m_macroVolume->GetLastSelectedChild ());
		m_editMacroButton->SetEnabled   ((selected != 0) && (selected->IsVisible()));
		m_deleteMacroButton->SetEnabled ((selected != 0) && (selected->IsVisible()));
	}
	else
	{
		selected = dynamic_cast<UIPage *>(m_volume->GetLastSelectedChild ());
	}
	
	m_textName->Clear ();
	m_textDesc->Clear ();
	
	if (selected)
	{
		m_pageInfo->SetVisible (true);
		CuiDragInfo dragInfo;
		
		if (findCommand (*selected, dragInfo))
		{
			if (!dragInfo.name.empty ())
			{
				if(selected->IsVisible())
				{
					m_textName->SetLocalText (dragInfo.name);
				}

				if (m_tabType == TT_mood)
				{	
					Unicode::String result;
					CuiStringVariablesManager::process (CuiStringIds::mood_desc_prose, Unicode::emptyString, dragInfo.name, Unicode::emptyString, result);
					
					const uint32 moodType = MoodManagerClient::getMoodByName (dragInfo.name);
//					if (moodType)
					{
						std::string emoteName;
						const bool hasAnimation = MoodManagerClient::getMoodEmoteAnimation (moodType, emoteName);
						
						if (hasAnimation)
						{
							IGNORE_RETURN(result.append (1, '\n'));
							result += CuiStringIds::mood_desc_has_animation.localize ();
						}						
						m_textDesc->SetLocalText (result);
					}
				}
				else if (m_tabType == TT_macro)
				{
					//pull off forced pause for display
					Unicode::UTF8String command = dragInfo.str;
					const std::string & forcePauseCommand = ClientMacroManager::getForcePauseCommand();
					std::string::size_type pos = command.find(forcePauseCommand);
					if(pos == 0)
					{
						command = command.substr(forcePauseCommand.size());
					}
					if(selected->IsVisible())
					{
						m_textDesc->SetLocalText (Unicode::utf8ToWide (command));
					}
				}
			}
			else if (!dragInfo.cmd.empty ())
			{
				const std::string & lowerCmd = Unicode::toLower (dragInfo.cmd);
				Unicode::String tmp;
				if (CuiSkillManager::localizeCmdName (lowerCmd, tmp))
				{
					m_textName->SetLocalText (tmp);
					if (CuiSkillManager::localizeCmdDescription (lowerCmd, tmp))
						m_textDesc->SetLocalText (tmp);
				}
				else
				{
					m_textName->SetLocalText (tmp);
				}
			}
			else
			{
				Unicode::String tmp;
				
				std::string str = Unicode::toLower (dragInfo.str);
				if (!str.empty () && str [0] == '/')
					IGNORE_RETURN(str.erase (size_t (0), 1));
				
				IGNORE_RETURN(CuiSkillManager::localizeCmdName (str, tmp));
				m_textName->SetLocalText (tmp);
				
				IGNORE_RETURN(CuiSkillManager::localizeCmdDescription (str, tmp));
				m_textDesc->SetLocalText (tmp);
			}
			
			safe_cast<UIPage *>(m_textDesc->GetParentWidget ())->Pack ();
		}
	}
	else
	{
		m_pageInfo->SetVisible (false);
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::clearCommands (UIVolumePage & vol)
{
	const UIBaseObject::UIObjectList & olist = vol.GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPage * const wid = safe_cast<UIPage *>(*it);
		wid->SetVisible (false);
		wid->SetEnabled (false);
	}

	vol.SetPackDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::reset  ()
{
	m_textName->Clear    ();
	m_textDesc->Clear    ();

	clearCommands (*m_volume);
	clearCommands (*m_macroVolume);

	const CreatureObject * const player = Game::getPlayerCreature ();

	if (!player)
		return;
	
	CuiDragInfo dragInfo;
	
	if (m_tabType == TT_combat || m_tabType == TT_space || m_tabType == TT_other || m_tabType == TT_astromech || m_tabType == TT_costume)
	{
		std::map<std::string, int> sv = player->getCommands ();

		{
			for (std::map<std::string, int>::iterator it = sv.begin (); it != sv.end (); )
			{
				std::string const commandName = it->first;

				//DEBUG_WARNING(true, ("%s\n", commandName.c_str()));

				if (isCommandForPage(m_tabType, m_displayGroupSpace, commandName))
				{
					std::map<std::string, int>::iterator itNext = it;
					++itNext;

					while (itNext != sv.end())
					{
						std::string const nextCommandName = itNext->first;
						int const nextCommandNameSize = nextCommandName.size();

						char const * const occurance = strstr(nextCommandName.c_str(), commandName.c_str());

						if (occurance == 0)
						{
							break;
						}

						else if ((nextCommandNameSize > 2)
								&& (nextCommandName[nextCommandNameSize - 2] == '_')
								&& (isdigit(nextCommandName[nextCommandNameSize - 1])))
						{
							std::map<std::string, int>::iterator temp = itNext;
							--temp;

							//DEBUG_WARNING(true, ("%s %s\n", temp->first.c_str(), nextCommandName.c_str()));

							sv.erase (temp);
							itNext++;
						}
						else
						{
							break;
						}
					}

					it = itNext;
				}
				else
				{
					std::map<std::string, int>::iterator temp = it++;
					sv.erase (temp);
				}
			}
		}

		appendCommandTableCommands(m_tabType, m_displayGroupSpace, sv);
		
		//build, sort a list based on the localized name of the social
		std::map<Unicode::String, std::string> commandNameMap;
		std::vector<Unicode::String> localizedCommandList;
		for (std::map<std::string, int>::const_iterator it = sv.begin (); it != sv.end (); ++it)
		{
			Unicode::String displayName;
			IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower ((*it).first), displayName));
			localizedCommandList.push_back(displayName);
			commandNameMap[displayName] = (*it).first;

			//DEBUG_WARNING(true, ("%s %s\n", (*it).first.c_str(), Unicode::wideToNarrow(displayName).c_str()));

		}
		std::sort (localizedCommandList.begin (), localizedCommandList.end ());
		IGNORE_RETURN(localizedCommandList.erase (std::unique (localizedCommandList.begin (), localizedCommandList.end ()), localizedCommandList.end ()));

		//add the commands to the socials tab
		for(std::vector<Unicode::String>::iterator i = localizedCommandList.begin(); i != localizedCommandList.end(); ++i)
		{
			const std::string & cmd = commandNameMap[*i];
			addCommand (cmd, 0);
		}
	}
	else if (m_tabType == TT_social)
	{
		SocialsManager::StringIntMap sim;
		SocialsManager::getCanonicalSocials (sim);

		//build, sort a list based on the localized name of the social
		std::map<Unicode::String, std::string> commandNameMap;
		std::vector<Unicode::String> localizedCommandList;
		for (SocialsManager::StringIntMap::const_iterator it = sim.begin (); it != sim.end (); ++it)
		{
			Unicode::String displayName;
			IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower (it->first), displayName));
			localizedCommandList.push_back(displayName);
			commandNameMap[displayName] = it->first;
		}
		std::sort (localizedCommandList.begin (), localizedCommandList.end ());
		IGNORE_RETURN(localizedCommandList.erase (std::unique (localizedCommandList.begin (), localizedCommandList.end ()), localizedCommandList.end ()));

		//add the commands to the socials tab
		for(std::vector<Unicode::String>::iterator i = localizedCommandList.begin(); i != localizedCommandList.end(); ++i)
		{
			const std::string & name = commandNameMap[*i];
			const uint32        id   = sim[name];
			addSocial (name, id);
		}
	}
	else if (m_tabType == TT_mood)
	{
		MoodManager::StringIntMap sim;
		MoodManager::getCanonicalMoods (sim);

		//build, sort a list based on the localized name of the mood
		std::map<Unicode::String, std::string> commandNameMap;
		std::vector<Unicode::String> localizedCommandList;
		for (SocialsManager::StringIntMap::const_iterator it = sim.begin (); it != sim.end (); ++it)
		{
			Unicode::String displayName;
			IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower (it->first), displayName));
			localizedCommandList.push_back(displayName);
			commandNameMap[displayName] = it->first;
		}
		std::sort (localizedCommandList.begin (), localizedCommandList.end ());
		IGNORE_RETURN(localizedCommandList.erase (std::unique (localizedCommandList.begin (), localizedCommandList.end ()), localizedCommandList.end ()));

		//add the commands to the mood tab
		for(std::vector<Unicode::String>::iterator i = localizedCommandList.begin(); i != localizedCommandList.end(); ++i)
		{
			const std::string & name = commandNameMap[*i];
			const uint32        id   = sim[name];
			Unicode::String displayName;
			IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower (name), displayName));
			addMood (name, displayName, id);
		}
	}
	else if (m_tabType == TT_macro)
	{
		const ClientMacroManager::MacroDataVector & mdv = ClientMacroManager::getMacroDataVector ();

		for (ClientMacroManager::MacroDataVector::const_iterator it = mdv.begin (); it != mdv.end (); ++it)
		{
			const ClientMacroManager::Data & data = *it;
			UIColor color;
			IGNORE_RETURN(UIUtils::ParseColor (data.color, color));
			addMacro (data.name, data.userDefinedName, data.commandString, data.icon, color, 0);
		}
	}
	else if (m_tabType == TT_chatter)
	{
		std::vector<std::string> const & data = CuiChatterSpeechManager::getData();
		for(std::vector<std::string>::const_iterator i = data.begin(); i != data.end(); ++i)
		{
			std::string finalCommand = s_chatterSpeech;
			finalCommand += s_command_breaker;
			finalCommand += *i;
			addChatterSpeech(finalCommand);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::addSocial (const std::string & cmd, uint32 id)
{
	if (id == CuiSocialsManager::getWhisperSocialId ())
		return;

	CuiDragInfo dragInfo;
	dragInfo.type              = CuiDragInfoTypes::CDIT_command;
	IGNORE_RETURN(dragInfo.str.assign (1, '/'));
	IGNORE_RETURN(dragInfo.str.append (cmd));

	dragInfo.commandValueValid = true;
	dragInfo.commandValue      = 0;

	IGNORE_RETURN(addEntry (cmd, dragInfo, 0, *m_volume, 0));
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::addChatterSpeech (std::string const & cmd)
{
	CuiDragInfo dragInfo;
	dragInfo.type              = CuiDragInfoTypes::CDIT_command;
	IGNORE_RETURN(dragInfo.str.assign (1, '/'));
	IGNORE_RETURN(dragInfo.str.append (cmd));

	dragInfo.commandValueValid = true;
	dragInfo.commandValue      = 0;

	IGNORE_RETURN(addEntry (cmd, dragInfo, 0, *m_volume, 0));
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::addMood (const std::string & cmd, const Unicode::String & displayName, uint32)
{
	static const std::string prefix = std::string ("/") + Unicode::wideToNarrow (MoodManagerClient::getMoodCommand ()) + std::string (" ");

	CuiDragInfo dragInfo;
	dragInfo.type              = CuiDragInfoTypes::CDIT_command;
	IGNORE_RETURN(dragInfo.str.assign (prefix));
	IGNORE_RETURN(dragInfo.str.append (cmd));

	dragInfo.commandValueValid = true;
	dragInfo.commandValue      = 0;
	dragInfo.name              = displayName;

	IGNORE_RETURN(addEntry (cmd, dragInfo, 0, *m_volume, 0));
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::addMacro    (const std::string & name, const std::string & userDefinedName, const std::string & text, const std::string& imageStyle, const UIColor & color, UIPage * insertBefore)
{
	UNREF (color);

	CuiDragInfo dragInfo;
	dragInfo.type              = CuiDragInfoTypes::CDIT_macro;
	dragInfo.cmd               = name;
	dragInfo.str               = text;
	dragInfo.name              = Unicode::utf8ToWide (userDefinedName);

	UIImageStyle * const s = CuiIconManager::findIcon (imageStyle);

	UIPage* const p = addEntry (userDefinedName, dragInfo, insertBefore, *m_macroVolume, s);
	NOT_NULL (p);
	p->SetName (name);
}


//----------------------------------------------------------------------

void SwgCuiCommandBrowser::addCommand (const std::string & cmd, UIPage * const insertBefore)
{
	CuiDragInfo dragInfo;
	dragInfo.type = CuiDragInfoTypes::CDIT_command;
	IGNORE_RETURN(dragInfo.str.assign (1, '/'));
	IGNORE_RETURN(dragInfo.str.append (cmd));

	IGNORE_RETURN(addEntry (cmd, dragInfo, insertBefore, *m_volume, 0));
}

//----------------------------------------------------------------------

UIPage * SwgCuiCommandBrowser::selectEntry (const std::string & cmd, const std::string & str, UIVolumePage & vol)
{
	UNREF (str);

	const UIBaseObject::UIObjectList & olist = vol.GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPage * const p = safe_cast<UIPage *>((*it));
		NOT_NULL (p);

		std::string test( p->GetName().c_str() );

		// do case insensitive check
		if (!_stricmp (test.c_str(), cmd.c_str ()))
		{
			vol.SetSelection (p);
			vol.ForcePackChildren ();
			vol.EnsureChildVisible (*p);
			return p;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

UIPage* SwgCuiCommandBrowser::addEntry (const std::string & cmd, const CuiDragInfo & dragInfo, UIPage * insertBefore, UIVolumePage & vol, UIImageStyle* iconStyle) const
{
	UIPage * displayPage = NULL;
	if (insertBefore && !insertBefore->IsVisible ())
		displayPage = insertBefore;

	UIPage * actualInsertBefore = 0;
	UIPage * const page = createCommandPage (cmd, dragInfo, displayPage, vol, actualInsertBefore, iconStyle);
	if (page)
	{
		if (!insertBefore)
			insertBefore = actualInsertBefore;
		
		if (insertBefore != page)
		{
			page->Attach (0);
			
			if (page->GetParent () == &vol)
				vol.RemoveChild       (page);
			
			IGNORE_RETURN(vol.InsertChildBefore (page, insertBefore));
			page->Detach (0);
		}

		page->Link ();

		page->SetOpacity (0.0f);
		page->SetEnabled (false);
		page->SetEnabled (true);
	}
	return page;
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onMacroAdded      (const ClientMacroManager::Messages::Added::Payload & name)
{
	if (m_tabType != TT_macro)
		return;

	const ClientMacroManager::Data * const data = ClientMacroManager::findMacroData (name);
	if (data)
	{
		//-- we don't care about duplicates, the macro system should prevent it
		bool duplicate = false;
		UIPage * const insertBefore = findInsertionPoint (*m_macroVolume, Unicode::utf8ToWide (data->userDefinedName), duplicate);

		//pull off forced pause for display
		std::string command = data->commandString;
		const std::string & forcePauseCommand = ClientMacroManager::getForcePauseCommand();
		std::string::size_type pos = command.find(forcePauseCommand);
		if(pos == 0)
		{
			command = command.substr(forcePauseCommand.size());
		}

		UIColor color;
		IGNORE_RETURN(UIUtils::ParseColor (data->color, color));
		addMacro (data->name, data->userDefinedName, command, data->icon, color, insertBefore);
		m_macroVolume->SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onMacroRemoved    (const ClientMacroManager::Messages::Removed::Payload & name)
{
	if (m_tabType != TT_macro)
		return;

	removeEntry (name, *m_macroVolume);
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onMacroModified   (const ClientMacroManager::Messages::Modified::Payload & name)
{
	if (m_tabType != TT_macro)
		return;

	onMacroRemoved (name);
	onMacroAdded   (name);

	update();
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onMacrosReset     (const bool & )
{
	if (m_tabType == TT_macro)
	{
		reset  ();
		update ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onCommandsChanged (const CreatureObject::Messages::CommandsChanged::Payload &)
{
	//-- command changes are handled in onCommandAdded/Removed
} //lint !e1762 stfu noob

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onCommandAdded    (const CreatureObject::Messages::CommandAdded::Payload & msg)
{
	const std::string & cmd = msg.second;
	if (!isCommandForPage(m_tabType, m_displayGroupSpace, cmd))
		return;

	Unicode::String displayString;
	IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower (cmd), displayString));

	bool duplicate = false;
	UIPage * const insertBefore = findInsertionPoint (*m_volume, displayString, duplicate);

	if (duplicate)
		return;

	addCommand (cmd, insertBefore);
	m_volume->SetPackDirty (true);
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::onCommandRemoved  (const CreatureObject::Messages::CommandRemoved::Payload & msg)
{
	const std::string & cmd = msg.second;
	if (!isCommandForPage(m_tabType, m_displayGroupSpace, cmd))
		return;

	removeEntry (cmd, *m_volume);
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::removeEntry        (const std::string & name, UIVolumePage & vol)
{
	const UIBaseObject::UIObjectList & olist = vol.GetChildrenRef ();
	unsigned int visibleListLength = 0;
	unsigned int removedEntryIndex = 0;
	for (UIBaseObject::UIObjectList::const_iterator it2 = olist.begin (); it2 != olist.end (); ++it2)
	{
		UIPage * const wid = safe_cast<UIPage *>(*it2);
		if (wid->IsVisible ())
		{
			visibleListLength++;
		}
	}
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPage * const wid = safe_cast<UIPage *>(*it);

		//-- invisible widgets are all stored at the end of the page
		//-- widget name is already lower cased

		if (!wid->IsVisible ())
			break;

		if (wid->GetName () == name)
		{
			wid->SetVisible (false);
			IGNORE_RETURN(vol.MoveChild (wid, UIBaseObject::Bottom));
			m_macroVolume->SetSelectionIndex(-1);
			if(visibleListLength == 1)
			{
				//do nothing because -1 is the right index to have selected
			}
			else if(visibleListLength == (removedEntryIndex + 1))
			{
				m_macroVolume->SetSelectionIndex(static_cast<int>(removedEntryIndex) - 1);
			}
			else
			{
				m_macroVolume->SetSelectionIndex(static_cast<int>(removedEntryIndex));
			}			
			break;
		}
		removedEntryIndex++;
	}
	vol.SetPackDirty (true);
}

//----------------------------------------------------------------------
 
UIPage * SwgCuiCommandBrowser::findInsertionPoint (UIVolumePage const & vol, const Unicode::String & displayText, bool & duplicate) const
{
	const Unicode::String & lowerDisplayText = Unicode::toLower (displayText);
	Unicode::String lowerWidgetDisplayName;

	const UIBaseObject::UIObjectList & olist = vol.GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIPage * const wid = safe_cast<UIPage *>(*it);

		//-- invisible widgets are all stored at the end of the page
		//-- widget name is already lower cased

		if (!wid->IsVisible ())
			return wid;

		lowerWidgetDisplayName.clear ();

		if (wid->GetProperty (Properties::LowerDisplayName, lowerWidgetDisplayName))
		{
			if (lowerWidgetDisplayName > lowerDisplayText)
				return wid;

			if (lowerWidgetDisplayName == lowerDisplayText)
			{
				duplicate = true;
				return wid;
			}
		}
		else
			return wid;
	}

	return 0;
}

//----------------------------------------------------------------------

UIPage * SwgCuiCommandBrowser::createCommandPage (const std::string & cmd, const CuiDragInfo & dragInfo, UIPage * displayPage, UIVolumePage & vol, UIPage * & actualInsertBefore, UIImageStyle* iconStyle) const
{
	actualInsertBefore = 0;

	if (!displayPage)
	{
		const UIBaseObject::UIObjectList & olist = vol.GetChildrenRef ();

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIPage * const wid = safe_cast<UIPage *>(*it);

			//-- invisible widgets are all stored at the end of the page
			//-- widget name is already lower cased

			if (!wid->IsVisible ())
			{
				displayPage = wid;
				actualInsertBefore = displayPage;
				break;
			}
		}

		if (!displayPage)
			displayPage = dynamic_cast<UIPage *>(m_sample->DuplicateObject ());
	}

	NOT_NULL (displayPage);

	if (displayPage) //lint !e774 bool always true, not in release 
	{
		dragInfo.setWidget (*displayPage);
		UIImage * const icon = safe_cast<UIImage *>(displayPage->GetChild ("icon"));

		displayPage->SetCustomDragWidget (icon);

		if (icon)
		{
			bool tmpParent = !displayPage->GetParent ();

			if (tmpParent)
				displayPage->SetParent (&vol);

			dragInfo.setWidget (*icon);
			icon->SetDragable (false);

			if(iconStyle)
				icon->SetStyle (iconStyle);
			else
			{
				UIImageStyle * const imageStyle  = CuiIconManager::findIconImageStyle (dragInfo);
				icon->SetStyle (imageStyle);
			}

			UIButtonStyle * const buttonStyle = CuiIconManager::findButtonStyle (dragInfo);
			if (buttonStyle)
			{
				buttonStyle->ApplyToWidget (*icon);
				icon->SetBackgroundOpacity (1.0f);
			}
			else
				icon->SetBackgroundOpacity (0.0f);

			displayPage->Link ();

			if (tmpParent)
				displayPage->SetParent (0);
		}
		
		Unicode::String displayName;

		if (!dragInfo.name.empty ())
			displayName = dragInfo.name;
		else
		{
			size_t endpos = 0;
			std::string cmd_first_word = cmd;
			IGNORE_RETURN(Unicode::getFirstToken (cmd, 0, endpos, cmd_first_word, s_command_breaker));
			IGNORE_RETURN(CuiSkillManager::localizeCmdName (Unicode::toLower (cmd), displayName));
//			displayName = Unicode::narrowToWide (cmd);
		}

		UIText * const text = safe_cast<UIText *>(displayPage->GetChild ("text"));
		if (text)
		{
			text->SetPreLocalized (true);
			text->SetLocalText (displayName);
		}
		
		displayPage->SetProperty (Properties::LowerDisplayName, Unicode::toLower (displayName));

		displayPage->SetActivated (false);
		displayPage->SetVisible   (true);
		displayPage->SetName      (cmd);
		Unicode::String description;			
		if (CuiSkillManager::localizeCmdDescription(Unicode::toLower(cmd), description))
		{
			displayPage->SetLocalTooltip(description);
			if(icon)
				icon->SetLocalTooltip(description);
		}
		else if (m_tabType == TT_mood)
		{
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIds::mood_desc_prose, Unicode::emptyString, Unicode::narrowToWide(cmd), Unicode::emptyString, result);
			
			const uint32 moodType = MoodManagerClient::getMoodByName (Unicode::narrowToWide(cmd));
			{
				std::string emoteName;
				const bool hasAnimation = MoodManagerClient::getMoodEmoteAnimation (moodType, emoteName);
				
				if (hasAnimation)
				{
					IGNORE_RETURN(result.append (1, '\n'));
					result += CuiStringIds::mood_desc_has_animation.localize ();
				}						
				displayPage->SetLocalTooltip(result);
				if(icon)
					icon->SetLocalTooltip(result);
			}
		}
		else
		{
			displayPage->SetLocalTooltip(Unicode::emptyString);
			if(icon)
				icon->SetLocalTooltip(Unicode::emptyString);
		}
	}
	
	return displayPage;
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::selectCommand     (const std::string & cmd)
{
	std::string strCopy = cmd;

	if (strCopy.empty ())
		return;

	if (strCopy [0] == '/')
		IGNORE_RETURN(strCopy.erase (0, 1));

	std::string cmdStr;

	if (CuiMessageQueueManager::findCommandString (strCopy, cmdStr, true))
	{
		selectString (cmdStr);
		return;
	}

	//-- hmm this should probably just be warning & stop processing
	selectString (strCopy);
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::selectString      (const std::string & str)
{
	reset();
	update();

	std::string strCopy = str;
	
	if (strCopy.empty ())
		return;

	if (strCopy [0] == '/')
		IGNORE_RETURN(strCopy.erase (0, 1));

	
	size_t endpos = 0;
	std::string token;
	if (Unicode::getFirstToken (strCopy, 0, endpos, token))
	{
		static const std::string moodPrefix = Unicode::wideToNarrow (MoodManagerClient::getMoodCommand ());
		if (!_stricmp (token.c_str (), moodPrefix.c_str ()))
		{
			strCopy = Unicode::getTrim (strCopy.substr (endpos));
			m_tabs->SetActiveTab (TabNames::mood);
			if (!selectEntry (strCopy, std::string (), *m_volume))
				WARNING (true, ("SwgCuiCommandBrowser unable to select mood entry [%s]", strCopy.c_str ()));

			return;
		}
	}
	
	const uint32 social = SocialsManager::getSocialTypeByName (strCopy);
	if (social)
	{
		m_tabs->SetActiveTab (TabNames::social);
		if (!selectEntry (strCopy, std::string (), *m_volume))
			WARNING (true, ("SwgCuiCommandBrowser unable to select social entry [%s]", strCopy.c_str ()));
		return;
	}
	
	{
		size_t cmd_endpos = 0;
		std::string cmd_first_word = strCopy;
		IGNORE_RETURN(Unicode::getFirstToken (strCopy, 0, cmd_endpos, cmd_first_word, s_command_breaker));

//		DEBUG_REPORT_LOG( true, ( "selectString: %s %s", str.c_str(), cmd_first_word.c_str() ) );

		const Command & command = CommandTable::getCommand (Crc::normalizeAndCalculate(cmd_first_word.c_str ()));
		
		if (!command.isNull ())
		{
			if (command.m_visibleToClients < 2)
			{
				WARNING (true, ("SwgCuiCommandBrowser tried to select invalid cmd [%s] original=(%s)", strCopy.c_str (), str.c_str ()));
				return;
			}
			
			const bool isCombatCommand = CuiCombatManager::isCombatCommand (command);
			
			if (isCombatCommand)
			{
				m_tabs->SetActiveTab (TabNames::combat);
				if (!selectEntry (strCopy, std::string (), *m_volume))
					WARNING (true, ("SwgCuiCommandBrowser unable to select command table entry [%s]", strCopy.c_str ()));
				return;
			}

			if (command.m_displayGroup == s_displayGroupCostume)
			{
				m_tabs->SetActiveTab(TabNames::costume);
				if (!selectEntry(strCopy, std::string(), *m_volume))
					WARNING(true, ("SwgCuiCommandBrowser unable to select command table entry [%s]", strCopy.c_str ()));
				return;
			}
		}
	}

	m_tabs->SetActiveTab (TabNames::other);
	if (!selectEntry (strCopy, std::string (), *m_volume))
		WARNING (true, ("SwgCuiCommandBrowser unable to select other cmd entry [%s]", strCopy.c_str ()));
}

//----------------------------------------------------------------------

void SwgCuiCommandBrowser::selectMacro       (const std::string & macro)
{	
	m_tabs->SetActiveTab (TabNames::macro);
	if (!selectEntry (macro, std::string (), *m_macroVolume))
		WARNING (true, ("SwgCuiCommandBrowser unable to select macro entry [%s]", macro.c_str ()));
}

//======================================================================

