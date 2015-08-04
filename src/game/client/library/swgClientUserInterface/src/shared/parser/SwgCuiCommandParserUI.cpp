// ======================================================================
//
// SwgCuiCommandParserUI.cpp
// copyright (c) 2001 Sony Online Entertainment
//

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCommandParserUI.h"

// ======================================================================
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/InputScheme.h"
#include "clientGame/MoodManagerClient.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientObject/MeshAppearance.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiChatBubbleManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiSocialsManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/Command.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/SocialsManager.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "swgClientUserInterface/SwgCuiChatWindow.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"
#include "swgClientUserInterface/SwgCuiLootBox.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSpaceConversation.h"
#include "swgClientUserInterface/SwgCuiWebBrowserManager.h"

#include "UIBaseObject.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPalette.h"
#include "UIRectangleStyle.h"
#include "UIWidgetRectangleStyles.h"
#include "UIWidgetStyle.h"

#include "LocalizationManager.h"

#include <cstdio>
#include <hash_map>
#include <list>
#include <map>
#include <set>
#include <vector>


// ======================================================================

namespace SwgCuiCommandParserUINamespace
{
	inline void showObjectAndChildren (const UIBaseObject * object, CommandParser::String_t & result, const Unicode::NarrowString & padding, bool showChildren, bool recurse, bool showFonts)
	{
		char buf[128];
		IGNORE_RETURN (sprintf (buf, "%s %1s--- %-30s [%20s]\n", padding.c_str (), object->GetChildCount () ? "+" : "o", object->GetName ().c_str (), object->GetTypeName ()));
		result += Unicode::narrowToWide (buf);
		
		if (object->GetChildCount () && showChildren)
		{
			Unicode::NarrowString new_padding (padding);
			IGNORE_RETURN (new_padding.append (5, ' '));
			
			UIBaseObject::UIObjectList list;
			object->GetChildren (list);
			
			for (UIBaseObject::UIObjectList::const_iterator iter = list.begin (); iter != list.end (); ++iter)
			{
				if (showFonts || !(*iter)->IsA (TUIFontCharacter))
				{
					showObjectAndChildren ( *iter, result, new_padding, recurse ? showChildren : false, recurse, showFonts);
				}
			}
		}
	}

	namespace CommandNames
	{
		const char * const action                 = "action";
		const char * const cameraInertia          = "cameraInertia";
		const char * const hudReset               = "hudReset";
		const char * const palette                = "palette";
		const char * const pointerModeMouseCamera = "pointerModeMouseCamera";
		const char * const reticleDeadZone        = "reticleDeadZone";
		const char * const inputScheme            = "inputScheme";
		const char * const reticleSelect          = "reticleSelect";
		const char * const radarSelect            = "radarSelect";
		const char * const browser             = "browser";
		const char * const url				   = "url";
#if PRODUCTION == 0
		const char * const debugPrint          = "debugPrint";
		const char * const set                 = "set";
		const char * const list                = "list";
		const char * const activate            = "activate";
		const char * const deactivate          = "deactivate";
		const char * const reset               = "reset";
		const char * const locale_reset        = "locale_reset";
		const char * const pixel_offset        = "pixel_offset";
		const char * const hudServerOutput     = "hudServerOutput";
		const char * const colorTest           = "colorTest";
		const char * const hueObjectTest       = "hueObjectTest";
		const char * const allowTargetAnything = "allowTargetAnything";
		const char * const debugExamine        = "debugExamine";
		const char * const debugClipboardExamine = "debugClipboardExamine";
		const char * const systemMessage       = "systemMessage";
		const char * const combatSpam          = "combatSpam";
		const char * const testResourceIcon    = "testResourceIcon";
		const char * const setCollideAll       = "setCollideAll";
		const char * const testSkillSystem     = "testSkillSystem";
		const char * const testTrade           = "testTrade";
		const char * const consent             = "consent";
		const char * const playUiEffect        = "playUiEffect";
		const char * const spaceConvoTest      = "spaceConvoTest";
		const char * const dumpCommandsToHtml  = "dumpCommandsToHtml";
		const char * const dumpCollectionsImages="dumpCollectionsImages";
		const char * const debugBuffs          = "debugBuffs";
		const char * const reloadPixelShader   = "reloadPixelShader";
		const char * const reloadVertexShader  = "reloadVertexShader";
		const char * const testProfession      = "testProfession";
		const char * const skinEdit            = "skinEdit";
		const char * const testLootBox         = "testLootBox";
		const char * const debugStringIds      = "debugStringIds";
		const char * const debugStringIdColor  = "debugStringIdColor";
		const char * const debugBrowserOutput  = "mozillaBrowserOutput";
#endif
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::reticleDeadZone,        0, "[<x> [y]]",   "Get or set the HUD reticle dead zone."},
		{CommandNames::cameraInertia,          0, "[float]",     "Get or set the game camera static inertia."},
#ifdef _DEBUG
		{CommandNames::action,                 0, "[string]",    "Perform a ui action by name. If string is blank or not a valid action, lists available actions."},
#else
		{CommandNames::action,                 1, "<string>",    "Perform a ui action by name."},
#endif
		{CommandNames::hudReset,               0, "",            "Reset hud positions & sizes (not toolbar etc...)."},
		{CommandNames::palette,                0, "[palette]",   "Set ui palette"},
		{CommandNames::pointerModeMouseCamera, 0, "[0/1]",       "Enable or toggle"},
		{CommandNames::inputScheme,            0, "[scheme]",    "Reset or list"},
		{CommandNames::reticleSelect,          1, "[reticle #]", "Select the current reticle."},
		{CommandNames::radarSelect,            1, "[radar #]",   "Select the current reticle."},
		{CommandNames::browser,                0, "<url>",       "Opens a web browser. If no URL is specified, you will be sent to the SWG home page."},
		{CommandNames::url,                    1, "<URL>", "Passes the specified URL to the Web Browser."		},
#if PRODUCTION == 0
		{CommandNames::debugPrint,            1, "<1|0>",       "."},
		{CommandNames::list,                  0, "",            "List all UI Pages."},
		{CommandNames::set,                   2, "<property> <value>", "Set a UI object property."},
		{CommandNames::activate,              1, "<page>",      "Activate UI Page."},
		{CommandNames::deactivate,            1, "<page>",      "Deactivate UI Page."},
		{CommandNames::reset,                 0, "",            "Reload UI from disk."},
		{CommandNames::locale_reset,          0, "",            "Reset all locale specific strings."},
		{CommandNames::pixel_offset,          0, "[offset]",    "Set or get the pixel offset."},
		{CommandNames::hudServerOutput,       0, "[1|0]",       "Get or set the HUD showing server messages."},
		{CommandNames::colorTest,             1, "<id>",        "Color Test."},
		{CommandNames::hueObjectTest,         3, "<id>, maxIndex1, maxIndex2", "Hue Object Test"},
		{CommandNames::allowTargetAnything,   1, "<1|0>",       "."},
		{CommandNames::debugExamine,          1, "<1|0>",       "."},
		{CommandNames::debugClipboardExamine, 1, "<1|0>",       "Copy debug info to clipboard"},
		{CommandNames::systemMessage,         1, "<msg>",       "fake system message"},
		{CommandNames::combatSpam,            6, "<id> <id> <id> <int> <float> <msg>",       "test combatspam"},
		{CommandNames::testResourceIcon,      3, "<viewer widget> <class/type flag> <class>", ""},
		{CommandNames::setCollideAll,         1, "<1|0>",       "collide against all geometry"},
		{CommandNames::testTrade,             0, "",            "client-side trade with lookat target"},
		{CommandNames::testSkillSystem,       1, "<1|0>",       "test. verbose or not"},
		{CommandNames::consent,               0, "",            "show a consent window"},
		{CommandNames::playUiEffect,          0, "",            "play a simulated server-triggered UI effect."},
		{CommandNames::spaceConvoTest,        0
		, "<SharedObjectTemplate name>", "Test the space conversation viewer UI"},
		{CommandNames::dumpCommandsToHtml,    2, "<html file path> <true|false>", "Dump all commands with icons to this path. false = short, true = long"},
		{CommandNames::dumpCollectionsImages, 1, "<file path>", "Dump all collection icons this path."},
		{CommandNames::debugBuffs,            1, "<buff duration>", "Creates some client-side buffs for testing."},
		{CommandNames::reloadPixelShader,     1, "<shader name>", "Reloads shader."},
		{CommandNames::reloadVertexShader,    1, "<shader name>", "Reloads shader."},
		{CommandNames::testProfession,        0, "", "Tests the profession selection window."},
		{CommandNames::skinEdit,              1, "<command> [value]", "Skin system editing tools."},
		{CommandNames::testLootBox,           1, "[objectId]...", "Test the loot box with existing object ids."},
		{CommandNames::debugStringIds,        0, "[1|0]", "Debug the source string id table and entry."},
		{CommandNames::debugStringIdColor,    0, "<ui color string>", "Set the color of the debug StringId string."},
		{CommandNames::debugBrowserOutput,    0, "", "Prints out debug information related to the Mozilla browser."		},
#endif
		{"", 0, "", ""} // this must be last
	};
	
	
	namespace ChatBubble
	{
		namespace CommandNames
		{
			const char * const durationScale = "durationScale";
			const char * const durationMin   = "durationMin";
			const char * const durationMax   = "durationMax";
			const char * const enabled       = "enabled";
			const char * const minimized     = "minimized";
		}
		
		const CommandParser::CmdInfo cmds[] =
		{
			{CommandNames::durationScale,            0, "[scale]",          "Duration Multiplier."},
			{CommandNames::durationMin,              0, "[min seconds]",    "Duration in seconds."},
			{CommandNames::durationMax,              0, "[max seconds]",    "Duration in seconds."},
			{CommandNames::enabled,                  1, "<1|0>",            "Enable."},
			{CommandNames::minimized,                1, "<1|0>",            "Minimize."},
			{"", 0, "", ""} // this must be last
		};
	}

	char buf_128 [128];

	typedef std::map<std::string /*command name*/, Command> UniqueCommands;
	Unicode::NarrowString s_commandHtmlFileName("index.html");
	int dumpStyleToFile(Unicode::NarrowString const & fileName, UIImageStyle const * const imageStyle, Gl_imageFormat const imageFormat = GLIF_png);
	int nextPowerOfTwo(int x);
	std::string getDisplayGroup(uint32 const group);
	std::string getCommandGroup(uint32 const group);
	std::vector<std::string> getProfessionsForSkill(SkillManager::SkillMap const & skillz, std::string const & commandName);
	bool dumpCommandsToHtml(std::string const & fileName, bool verbose);
	bool dumpCollectionImagesToFile(std::string const& filePath);


	typedef std::map<uint32 /*hash*/, std::string> HashStringMap;
	HashStringMap s_displayGroupNameHashMap;
	HashStringMap s_commandGroupNameHashMap;

	// client-side buff list.

#if PRODUCTION == 0
	uint32 s_clientBuffs[] = 
	{
		Crc::normalizeAndCalculate("frogBuff"),
		Crc::normalizeAndCalculate("shock"),
		Crc::normalizeAndCalculate("aim"),
		Crc::normalizeAndCalculate("giggledust")
	};	
	size_t const s_clientBuffsCount = sizeof(s_clientBuffs) / sizeof(uint32);
#else
	uint32 s_clientBuffs[] =
	{
		Crc::normalizeAndCalculate("")
	};	
	size_t const s_clientBuffsCount = 0;
#endif

	void debugBuffs(float duration);
	void exportSelectedMediatorStyles();
	void AddStylePaths(UIBaseObject * obj, std::set<UIWidgetStyle *> & styles);
	void textureRectangle(Texture::LockData & lockData, UIRect const & imageRect, UIColor const & color);
	std::string makeValidFileName(std::string const & str);

}

using namespace SwgCuiCommandParserUINamespace;

//----------------------------------------------------------------------

class SwgCuiCommandParserUI::MessageBoxParser : public CommandParser
{
public:
	MessageBoxParser ();
	virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	MessageBoxParser (const MessageBoxParser & rhs);
	MessageBoxParser & operator= (const MessageBoxParser & rhs);
};

//-----------------------------------------------------------------

class SwgCuiCommandParserUI::ShowParser : public CommandParser
{
public:
	ShowParser ();
	virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	ShowParser (const ShowParser & rhs);
	ShowParser & operator= (const ShowParser & rhs);
};


//-----------------------------------------------------------------

class SwgCuiCommandParserUI::ChatBubbleParser : public CommandParser
{
public:
	ChatBubbleParser ();
	virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	ChatBubbleParser (const ChatBubbleParser & rhs);
	ChatBubbleParser & operator= (const ChatBubbleParser & rhs);
};

//-----------------------------------------------------------------

SwgCuiCommandParserUI::SwgCuiCommandParserUI  ():
CommandParser ("ui", 0, "...", "ui commands", 0)
{
	createDelegateCommands (cmds);
	IGNORE_RETURN (addSubCommand (new MessageBoxParser ())); //lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new ShowParser ()));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new ChatBubbleParser ()));//lint !e1524 // new in ctor w no explicit dtor


	s_commandGroupNameHashMap[Crc::normalizeAndCalculate("combat_ranged")] = std::string("Combat Ranged");
	s_commandGroupNameHashMap[Crc::normalizeAndCalculate("combat_melee")] = std::string("Combat Melee");
	s_commandGroupNameHashMap[Crc::normalizeAndCalculate("combat_general")] = std::string("Combat General");
	s_commandGroupNameHashMap[Crc::normalizeAndCalculate("combat_non_loop")] = std::string("Combat Non-Loop");
	s_commandGroupNameHashMap[Crc::normalizeAndCalculate("client_only")] = std::string("Client Only");

	s_displayGroupNameHashMap[Crc::normalizeAndCalculate("jedi")] = std::string("Jedi");
	s_displayGroupNameHashMap[Crc::normalizeAndCalculate("hidden")] = std::string("Hidden");
	s_displayGroupNameHashMap[Crc::normalizeAndCalculate("csr")] = std::string("CSR");
	s_displayGroupNameHashMap[Crc::normalizeAndCalculate("ship")] = std::string("Ship");
	s_displayGroupNameHashMap[Crc::normalizeAndCalculate("astromech")] = std::string("Astromech");
}
//-----------------------------------------------------------------

bool SwgCuiCommandParserUI::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	//-----------------------------------------------------------------

	if (isCommand (argv [0],  CommandNames::reticleDeadZone))
	{
		if (argv.size () > 1)
		{
			UISize deadZoneSize;
			deadZoneSize.x = atoi (Unicode::wideToNarrow (argv [1]).c_str ());

			if (argv.size () > 2)
				deadZoneSize.y = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
			else
				deadZoneSize.y = deadZoneSize.x;

			CuiManager::getIoWin ().setDeadZone (deadZoneSize);
		}
		else
		{
			const UIRect & deadZone = CuiManager::getIoWin ().getDeadZone ();

			_snprintf (buf_128, 128, "SwgCui reticle deadzone is currently %3d,%3d.\n", deadZone.Width (), deadZone.Height ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::cameraInertia))
	{
		if (argv.size () > 1)
		{
			const float f = static_cast<float>(atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			CuiManager::setCameraInertia (f);
		}
		else
		{
			_snprintf (buf_128, 128, "SwgCui camera static inertia is %5.2f.\n", CuiPreferences::getCameraInertia ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::pointerModeMouseCamera))
	{
		bool b = false;
		if (argv.size () > 1 && !argv [1].empty ())
			b = argv [1][0] != '0';
		else
			b = !CuiPreferences::getPointerModeMouseCameraEnabled ();

		CuiPreferences::setPointerModeMouseCameraEnabled (b);

		_snprintf (buf_128, 128, "SwgCui pointerModeMouseCamera is %s", b ? "enabled" : "disabled");
		result += Unicode::narrowToWide (buf_128);

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::action))
	{
#ifdef _DEBUG
		bool listActions = argv.size() == 1;
		bool actionResult = true;
		std::string command;

		if (!listActions)
		{
			command = Unicode::wideToNarrow(argv[1]);
			Unicode::String str;
			reconstructString(argv, 2, argv.size(), true, str);
			actionResult = CuiActionManager::performAction(command, str);
		}

		if (listActions || (!actionResult && !CuiActionManager::findAction(command)))
		{
			CuiActionManager::ActionMap const * const actionMap = CuiActionManager::getActionMap();

			if (actionMap)
			{
				CuiActionManager::ActionMap::const_iterator actionMapIter = actionMap->begin();
				std::string availableActions = "The following actions are available:\n";

				for(; actionMapIter != actionMap->end(); ++actionMapIter)
					availableActions += "\t" + actionMapIter->first + "\n";

				result += Unicode::narrowToWide(availableActions);
			}
		}

		return actionResult;
#else
		const std::string command (Unicode::wideToNarrow (argv [1]));
		Unicode::String str;
		reconstructString (argv, 2, argv.size (), true, str);
		return CuiActionManager::performAction (command, str);
#endif
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::hudReset))
	{
		CuiSettings::resetForPlayer ();
		result += Unicode::narrowToWide ("HUD will reset after you disconnect and reconnect.\n");
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::palette))
	{
		if (argv.size () == 1)
		{
			const CuiPreferences::StringVector & sv = CuiPreferences::getPaletteNames (true);

			result += Unicode::narrowToWide ("Available palettes:\n");

			for (CuiPreferences::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
			{
				const std::string & paletteName = *it;
				result += Unicode::narrowToWide (paletteName);
				result.append (1, '\n');
			}

			return true;
		}

		const std::string & paletteName = Unicode::wideToNarrow (argv [1]);

		if (CuiPreferences::setPaletteName (paletteName, true))
			result += Unicode::narrowToWide ("Palette reset.");	
		else
			result += Unicode::narrowToWide ("No such palette.\n");

		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], CommandNames::inputScheme))
	{
		if (argv.size () < 2)
		{
			InputScheme::StringVector sv;
			InputScheme::getTypes (sv);

			result += Unicode::narrowToWide ("Available input schemes:\n");
			for (InputScheme::StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
			{
				result.append (4, ' ');
				result += Unicode::narrowToWide (*it);
				result.append (1, '\n');
			}
		}
		else
		{
			const std::string & type = Unicode::wideToNarrow (argv [1]);
			if (InputScheme::resetFromType (type, true))
				result += Unicode::narrowToWide ("Input Scheme reset to ") + argv [1];
			else
				result += Unicode::narrowToWide ("Input Scheme failed to set ") + argv [1];
			result.append (1, '\n');
		}
	
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], CommandNames::reticleSelect))
	{
		if (argv.size() > 1)
		{
			CuiPreferences::setReticleSelect(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
		}
		else
		{
			result += Unicode::narrowToWide("Please select a reticle style number.");
		}
		
		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0], CommandNames::radarSelect))
	{
		if (argv.size() > 1)
		{
			CuiPreferences::setRadarSelect(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
		}
		else
		{
			result += Unicode::narrowToWide("Please select a radar style number.");
		}
		
		return true;
	}
	else if(isCommand(argv[0], CommandNames::browser))
	{	
		if(argv.size() > 1)
		{
			std::string url = Unicode::wideToNarrow(argv[1]);
			SwgCuiWebBrowserManager::setURL(url);
		}

		SwgCuiWebBrowserManager::createWebBrowserPage();

		return true;
	}
	else if (isCommand(argv[0], CommandNames::url))
	{
		if(argv.size() >= 2)
		{
			SwgCuiWebBrowserManager::setURL(Unicode::wideToNarrow(argv[1]));
		}

		return true;
	}

#if PRODUCTION == 0

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::list))
	{
		result += Unicode::narrowToWide (
			"UI Mediators:\n"
			"id  name                  exists  status    path\n"
			"----------------------------------------------------------------------------------------\n");

		typedef std::vector<std::string> ResultVector;
		ResultVector rv;
		CuiMediatorFactory::getMediatorNames (rv);

		int i = 0;
		for (ResultVector::const_iterator it = rv.begin (); it != rv.end (); ++it)
		{
			const std::string & str = *it;
			const CuiMediator * mediator = CuiMediatorFactory::get (str.c_str (), false);

			_snprintf (buf_128, 128, "%2d  %-20s  %1s  %10s %-64s\n", i, (*it).c_str (), mediator ? "*" : "",
				mediator ? (mediator->isActive () ? "active" : "inactive") : "",
				mediator ? mediator->getPage().GetFullPath().c_str() : "");
			result += Unicode::narrowToWide (buf_128);
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::set))
	{
		UIBaseObject * const root = UIManager::gUIManager ().GetRootPage ();
		if (root)
		{
			root->SetProperty (UILowerString (Unicode::wideToNarrow (argv [1])), argv [2]);
		}
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::activate))
	{

		const String_t & cmdNameStr = Unicode::narrowToWide ("ui activate");

		if (argv.size () < 2)
		{
			result += getErrorMessage (cmdNameStr, ERR_INVALID_ARGUMENTS);
			return true;
		}

		const Unicode::NarrowString ns (Unicode::wideToNarrow (argv [1]));

		std::string canonicalName;
		if (!CuiMediatorFactory::findMediatorCanonicalName (ns, canonicalName))
			result += cmdNameStr + argv [1] + Unicode::narrowToWide (": No such UI Screen: ") + argv [1];
		else			
			IGNORE_RETURN (CuiMediatorFactory::activate (canonicalName.c_str ()));
		
		return true;
		
	}
	
	//-----------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::deactivate))
	{
		const String_t & cmdNameStr = Unicode::narrowToWide ("ui deactivate");

		if (argv.size () < 2)
		{
			result += getErrorMessage (cmdNameStr, ERR_INVALID_ARGUMENTS);
			return true;
		}
		
		const Unicode::NarrowString ns (Unicode::wideToNarrow (argv [1]));
		
		if (!_stricmp (ns.c_str (), "all"))
			CuiMediatorFactory::deactivateAll ();
		else
		{
			std::string canonicalName;
			if (!CuiMediatorFactory::findMediatorCanonicalName (ns, canonicalName))
				result += cmdNameStr + argv [1] + Unicode::narrowToWide (": No such UI Screen: ") + argv [1];
			else			
				IGNORE_RETURN (CuiMediatorFactory::deactivate (canonicalName.c_str ()));
		}

		return true;

	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::reset))
	{
		CuiManager::requestReset();
		SwgCuiHudFactory::reset();
		return true;
	}

	//-----------------------------------------------------------------
	
	else if (isCommand (argv [0],  CommandNames::locale_reset))
	{

		CuiManager::resetLocalizedStrings ();
		result += Unicode::narrowToWide ("UI Locale reset.");
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::pixel_offset))
	{
		if (argv.size () > 1)
		{
			real val = static_cast<real> (atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			CuiManager::setPixelOffset (val);
		}
		else
		{
			_snprintf (buf_128, 128, "SwgCui pixel offset is currently %2.2f.\n", CuiManager::getPixelOffset ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}

	//-----------------------------------------------------------------

	// @todo replace this with the chat window tab functionality
	else if (isCommand (argv [0],  CommandNames::hudServerOutput))
	{
		SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
		if (hud)
		{
			SwgCuiChatWindow * const chatWindow = NON_NULL (hud->getChatWindow  ());
			if (argv.size () == 1)
			{
				result += Unicode::narrowToWide ("HUD Server Listen: ") + Unicode::narrowToWide (chatWindow->getPrintServerOutput () ? "1\n" : "0\n");
			}
			else
			{
				const Unicode::String arg = argv [1];
				if (arg.empty ())
					return false;
				chatWindow->setPrintServerOutput (arg[0] == '0' ? false : true);
				result += Unicode::narrowToWide ("HUD Server Listen set to: ") + Unicode::narrowToWide (chatWindow->getPrintServerOutput () ? "1\n" : "0\n");
			}
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::colorTest))
	{
		const NetworkId id (Unicode::wideToNarrow (argv [1]));
		TangibleObject * const tangible = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById (id));
		if (tangible)
		{
			SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
			if (hud)
				hud->colorTest (*tangible);
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::hueObjectTest))
	{
		const NetworkId id (Unicode::wideToNarrow (argv [1]));
		const int maxIndex1 = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
		const int maxIndex2 = atoi (Unicode::wideToNarrow (argv [3]).c_str ());

		SwgCuiHud * const hud = SwgCuiHudFactory::findMediatorForCurrentHud();
		if (hud)
			hud->hueObjectTest (id, maxIndex1, maxIndex2);

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::allowTargetAnything))
	{
		CuiPreferences::setAllowTargetAnything (argv [1][0] != '0');
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::setCollideAll))
	{
		MeshAppearance::setCollideAgainstAllGeometry (argv [1][0] != '0');
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::testTrade))
	{
		CreatureObject * const player = Game::getPlayerCreature ();
		if (player)
			ClientSecureTradeManager::onBeginTrade (player->getLookAtTarget ());
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::testSkillSystem))
	{
		CuiSkillManager::testSkillSystem (argv [1][0] != '0');
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::consent))
	{
		CuiConsentManager::askLocalConsent(CuiStringIds::ok.localize(), NULL);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0],  CommandNames::debugExamine))
	{
		CuiPreferences::setDebugExamine (argv [1][0] != '0');
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::debugClipboardExamine))
	{
		CuiPreferences::setDebugClipboardExamine (argv [1][0] != '0');
		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::systemMessage))
	{
		Unicode::String str;
		CommandParser::reconstructString (argv, 1, argv.size (), true, str);
		CuiSystemMessageManager::sendFakeSystemMessage (str);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::combatSpam))
	{
		Unicode::String str;
		CommandParser::reconstructString (argv, 6, argv.size (), true, str);

		NetworkId idSource (Unicode::wideToNarrow (argv [1]));
		NetworkId idTarget (Unicode::wideToNarrow (argv [2]));
		NetworkId idOther  (Unicode::wideToNarrow (argv [3]));

		int       digit_i  = atoi (Unicode::wideToNarrow (argv [4]).c_str ());
		float     digit_f  = static_cast<float>(atof (Unicode::wideToNarrow (argv [5]).c_str ()));

		const Object * const source = NetworkIdManager::getObjectById(idSource);
		Vector position_wSource;
		if (source)
			position_wSource = source->getPosition_w();

		const Object * const target = NetworkIdManager::getObjectById(idTarget);
		Vector position_wTarget;
		if (target)
			position_wTarget = target->getPosition_w();

		CuiCombatManager::sendFakeCombatSpam (idSource, position_wSource, idTarget, position_wTarget, idOther, digit_i, digit_f, str);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], CommandNames::testResourceIcon))
	{
		const std::string & widgetPath = Unicode::wideToNarrow (argv [1]);
		const bool          isType     = argv [2][0] != '0';
		const std::string & resource   = Unicode::wideToNarrow (argv [3]);

		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer * >(UIManager::gUIManager ().GetObjectFromPath (widgetPath.c_str ()));
		if (viewer)
		{
			Object * obj = 0;

			if (isType)
			{
			}
			else
				obj = ResourceIconManager::getObjectForClass (resource);

			viewer->setObject (obj);
		}

		return true;
	}

	//----------------------------------------------------------------------
	
	else if (isCommand( argv [0], CommandNames::debugPrint))
	{
		if (argv[1][0] != '0')
			CuiMediatorFactory::activate   (CuiMediatorTypes::DebugInfoPage);
		else
			CuiMediatorFactory::deactivate (CuiMediatorTypes::DebugInfoPage);

		result += node->getFullErrorMessage (ERR_SUCCESS);
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv [0],  CommandNames::playUiEffect))
	{
		if (argv.size() < 2)
		{
			result += Unicode::narrowToWide ("Invalid number of parameters.");
			return false;
		}

		Unicode::String resultString;
		static Unicode::String const spaceString = Unicode::narrowToWide(" ");

		
		for(StringVector_t::const_iterator itString = (argv.begin() + 1); itString != argv.end(); ++itString)
		{
			resultString += *itString;
			resultString += spaceString;
		}
		
		CuiManager::playUiEffect(Unicode::wideToNarrow(resultString));

		return true;
	}

	else if(isCommand (argv [0],  CommandNames::spaceConvoTest))
	{
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_SpaceConversation);
		SwgCuiSpaceConversation * const convo = dynamic_cast<SwgCuiSpaceConversation *>(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_SpaceConversation));
		if(convo)
		{
			static std::string emptyString;
			std::string const overrideObjectTemplate = argv.size() > 1 ? Unicode::wideToNarrow(argv[1]) : emptyString;
			bool const res = convo->setTauntInfo(Game::getPlayerCreature()->getLookAtTarget(), Unicode::narrowToWide("Test Conversation"), Crc::calculate(overrideObjectTemplate.c_str()), "hack_sound", 0.0f);
			if(!res)
			{
				result += Unicode::narrowToWide ("Could not set viewer target, bad SharedObjectTemplate name?\n");
				return false;
			}
		}
	}
	else if(isCommand(argv[0], CommandNames::dumpCommandsToHtml))
	{
		std::string filePath = Unicode::wideToNarrow(argv[1]);
		filePath += "\\";

		std::string const & verbosity = Unicode::toLower(Unicode::wideToNarrow(argv[1]));
		bool const verbose = verbosity.empty() ? false : ((atoi(verbosity.c_str()) > 0) || (verbosity[0] == 't'));
		
		if (filePath.find(".") == std::string::npos) 
		{
			if (dumpCommandsToHtml(filePath, verbose))
			{
				result += node->getFullErrorMessage(ERR_SUCCESS);
				return true;
			}
		}
		else
		{
			result += Unicode::narrowToWide("Do not pass in a file name.  Only a path.");
			return true;
		}
	}
	else if(isCommand(argv[0], CommandNames::dumpCollectionsImages))
	{
		std::string filePath = Unicode::wideToNarrow(argv[1]);
		filePath += "\\";

		if (filePath.find(".") == std::string::npos) 
		{
			if (dumpCollectionImagesToFile(filePath))
			{
				result += node->getFullErrorMessage(ERR_SUCCESS);
				result += Unicode::narrowToWide("\nImages dumped, run cropimages.bat to crop them.");
				return true;
			}
			else
			{
				result += Unicode::narrowToWide("Error dumping images to folder " + filePath);
				return false;
			}
		}
		else
		{
			result += Unicode::narrowToWide("Do not pass in a file name.  Only a path.");
			return true;
		}
	}
	else if(isCommand(argv[0], CommandNames::debugBuffs))
	{
		if (Game::getSinglePlayer())
		{
			float const duration = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str())); // atof returns a double.
			debugBuffs(duration);

			if (duration) 
			{
				result += Unicode::narrowToWide("Buffs added.");
			}
			else
			{
				result += Unicode::narrowToWide("Buffs removed.");
			}
		}
		else
		{
			result += Unicode::narrowToWide("This command is available in single-player, non-production code only!");
		}

		return true;
	}
	else if(isCommand(argv[0], CommandNames::reloadPixelShader))
	{
		ShaderImplementation::reloadPixelShader(Unicode::wideToNarrow(argv[1]).c_str());
		return true;
	}
	else if(isCommand(argv[0], CommandNames::reloadVertexShader))
	{
		ShaderImplementation::reloadVertexShader(Unicode::wideToNarrow(argv[1]).c_str());
		return true;
	}
	else if(isCommand(argv[0], CommandNames::testProfession))
	{
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_ProfessionTemplateSelect);
		return true;
	}
	else if(isCommand(argv[0], CommandNames::skinEdit))
	{
		std::string const & command = Unicode::wideToNarrow(argv[1]);

		if (command == "select") 
		{
			CuiWorkspace * workspace = CuiWorkspace::getGameWorkspace();
			if (workspace) 
			{
				workspace->setShowGlow(!workspace->getShowGlow());
			}

			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide(workspace->getShowGlow() ? "Skin select mode enabled." : "Skin select mode disabled"));

			return true;
		}
		else if (command == "export")
		{
			exportSelectedMediatorStyles();
			CuiSystemMessageManager::sendFakeSystemMessage(Unicode::narrowToWide("Export complete."));
			return true;
		}
	}

	else if(isCommand(argv[0], CommandNames::testLootBox))
	{
		Unicode::String str;
		for(int i = 1; i < static_cast<int>(argv.size()); ++i)
		{
			str += argv[i] + Unicode::narrowToWide(" ");
		}
		return CuiActionManager::performAction ("lootBox", str);
	}

	else if(isCommand(argv[0], CommandNames::debugStringIds))
	{
		bool enable = true;

		if (argv.size() >= 2)
		{
			enable = atoi(Unicode::wideToNarrow(argv[1]).c_str()) > 0;
		}

		LocalizationManager::debugDisplayStrings(enable);
		return true;
	}

	else if(isCommand(argv[0], CommandNames::debugStringIdColor))
	{
		Unicode::String color(Unicode::narrowToWide(" \\#ff0000"));

		if (argv.size() >= 2)
		{
			color = argv[1];
		}

		LocalizationManager::debugDisplayStringColor(color);
		return true;
	}
	else if(isCommand(argv[0], CommandNames::debugBrowserOutput))
	{
		SwgCuiWebBrowserManager::debugOutput();
		return true;
	}


#endif
	return false;
}

//-----------------------------------------------------------------
// MessageBoxParser
//-----------------------------------------------------------------

SwgCuiCommandParserUI::MessageBoxParser::MessageBoxParser () :
CommandParser ("messagebox", 0, "...", "Manipulate messageboxes.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("list", 0, "", "List active messageboxes.", this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("create",  1, "<modal>", "Create a messagebox.", this)));//lint !e1524 // new in ctor w no explicit dtor

	CommandParser * setParser = new CommandParser ("set", 0, "[]",  "set messagebox properties", this);
	NOT_NULL (setParser);

	IGNORE_RETURN (addSubCommand (setParser));

	IGNORE_RETURN (setParser->addSubCommand (new CommandParser ("text", 2, "<index> <text>", "Set text", this)));
	IGNORE_RETURN (setParser->addSubCommand (new CommandParser ("buttons", 2, "<index> <button names...>", "Set buttons", this)));

} //lint !e429 custodial pointer setParser

//-----------------------------------------------------------------

bool SwgCuiCommandParserUI::MessageBoxParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);
	
	if (isCommand (argv[0], "list"))
	{

	}
	else if (isCommand (argv[0], "create"))
	{
		bool modal = argv[1][0] != '0';

		IGNORE_RETURN (CuiMessageBox::createMessageBox (0, modal));

		result += Unicode::narrowToWide ("Messagebox created.");
	}
	else if (isCommand (argv[0], "text"))
	{

	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------
// ShowParser
//-----------------------------------------------------------------

SwgCuiCommandParserUI::ShowParser::ShowParser () :
CommandParser ("show", 0, "...", "Show information.", 0)
{
	IGNORE_RETURN (addSubCommand (new CommandParser ("tree",       0, "[object path]", "Show child object tree.", this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("objects",    0, "[object path]", "List child objects.", this)));//lint !e1524 // new in ctor w no explicit dtor
	IGNORE_RETURN (addSubCommand (new CommandParser ("properties", 0, "",              "Show a UIBaseObject's properties.", this)));//lint !e1524 // new in ctor w no explicit dtor
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserUI::ShowParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);
	
	//-----------------------------------------------------------------

	if (isCommand (argv[0], "objects"))
	{
		const UIBaseObject * object = UIManager::gUIManager().GetRootPage();

		if (argv.size () > 1)
		{
			object = static_cast<UIBaseObject *> (object->GetObjectFromPath (Unicode::wideToNarrow (argv[1]).c_str (), TUIObject));
		}

		if (object == 0)
		{
			result += Unicode::narrowToWide ("No such page: ") + argv[1];
		}
		else
		{
			showObjectAndChildren (object, result, "", true, false, object->IsA (TUITextStyle) );
		}
	}

	//-----------------------------------------------------------------

	else if (isCommand (argv[0], "tree"))
	{
		const UIBaseObject * object = UIManager::gUIManager().GetRootPage();

		if (argv.size () > 1)
		{
			object = static_cast<UIBaseObject *> (object->GetObjectFromPath (Unicode::wideToNarrow (argv[1]).c_str (), TUIObject));
		}

		if (object == 0)
		{
			result += Unicode::narrowToWide ("No such page: ") + argv[1];
		}
		else
		{
			showObjectAndChildren (object, result, "", true, true, object->IsA (TUITextStyle));
		}
	}

	//-----------------------------------------------------------------


	else if (isCommand (argv[0], "properties"))
	{
		UIBaseObject * object = UIManager::gUIManager().GetRootPage();

		if (argv.size () > 1)
		{
			object = static_cast<UIBaseObject *> (object->GetObjectFromPath (Unicode::wideToNarrow (argv[1]).c_str (), TUIObject));
		}

		if (object == 0)
		{
			result += Unicode::narrowToWide ("No such object: ") + argv[1];
		}
		else
		{
			const UINarrowString & oname = object->GetName ();
			IGNORE_RETURN (result.append (Unicode::narrowToWide ("Properties for object: ")).append (oname.begin (), oname.end ()).append (1, '\n'));

			static const UILowerString skip_props_array [] =
			{
				UIWidget::PropertyName::Color,
					UIWidget::PropertyName::Enabled,
					UIWidget::PropertyName::GetsInput,
					UIWidget::PropertyName::Location,
					UIWidget::PropertyName::Opacity,
					UIWidget::PropertyName::ScrollExtent,
					UIWidget::PropertyName::ScrollLocation,
					UIWidget::PropertyName::Size,
					UIWidget::PropertyName::Visible,
					UIWidget::PropertyName::BackgroundTint,
					UIWidget::PropertyName::BackgroundColor,
					UIWidget::PropertyName::BackgroundOpacity,
					UIWidget::PropertyName::MinimumSize,
					UIWidget::PropertyName::MaximumSize,
					UIWidget::PropertyName::MinimumScrollExtent
			};

			static const int props_array_size = sizeof (skip_props_array) / sizeof (skip_props_array [0]);

			typedef const std::set<UILowerString> StringSet;

			static const StringSet skipStrings (skip_props_array, skip_props_array + props_array_size);

			// props
			UIBaseObject::UIPropertyNameVector pset;

			object->GetPropertyNames (pset, false);

			if (!pset.empty ())
			{
				
				for (UIBaseObject::UIPropertyNameVector::const_iterator iter = pset.begin (); iter != pset.end (); ++iter)
				{
					const UILowerString & key = *iter;

					if (skipStrings.find (key) != skipStrings.end ())
						continue;

					UIString value;						
					if (object->GetProperty (key, value))
					{
						_snprintf (buf_128, 128, "  %-20s :   ", key.c_str ());
						result += Unicode::narrowToWide (buf_128);
						IGNORE_RETURN (result.append (value));						
						IGNORE_RETURN (result.append (1, '\n'));
					}
				}
			}
			else
			{
				result += Unicode::narrowToWide ("No props for you!");
			}
			
			if (object->IsA (TUIWidget))
			{							
				const UIWidget * widget = safe_cast <UIWidget*>(object);

				IGNORE_RETURN (result.append (Unicode::narrowToWide ("Attributes for widget: ")).append (oname.begin (), oname.end ()).append (1, '\n'));
				
				// props
				
				const UIPoint & loc = widget->GetLocation ();

				UISize scrollExtent;
				widget->GetScrollExtent (scrollExtent);

				const UIPoint & scrollLoc = widget->GetScrollLocation ();
				
				const UISize minSize (widget->GetMinimumSize ());
				const UISize maxSize (widget->GetMaximumSize ());
				const UISize minExtent (widget->GetMinimumScrollExtent ());

				char buf[512];
				
				_snprintf (buf, 512,
					"           Size:  %d,%d\n"
					"   Min/Max Size:  %d,%d / %d,%d\n"
					"   ScrollExtent:  %d,%d\n"
					"MinScrollExtent:  %d,%d\n"
					"       Location:  %d,%d\n"
					" ScrollLocation:  %d,%d\n"
					"  Color/Opacity:  #%02x%02x%02x %f\n"
					"BackgroundColor:  #%02x%02x%02x %f\n"
					" BackgroundTint:  #%02x%02x%02x\n"
					"        Visible:  %d\n"
					"        Enabled:  %d\n"
					"       Selected:  %d\n"
					"     UnderMouse:  %d\n"
					"       Dragable:  %d\n"
					"      Transient:  %d\n"
					"      GetsInput:  %d\n",
					widget->GetWidth (), widget->GetHeight (),
					minSize.x, minSize.y, maxSize.x, maxSize.y,
					scrollExtent.x, scrollExtent.y,
					minExtent.x, minExtent.y,
					loc.x, loc.y,
					scrollLoc.x, scrollLoc.y,
					widget->GetColor ().r,
					widget->GetColor ().g,
					widget->GetColor ().b,
					widget->GetOpacity (),
					widget->GetBackgroundColor ().r,
					widget->GetBackgroundColor ().g,
					widget->GetBackgroundColor ().b,
					widget->GetBackgroundOpacity (),
					widget->GetBackgroundTint ().r,
					widget->GetBackgroundTint ().g,
					widget->GetBackgroundTint ().b,
					widget->IsVisible (),
					widget->IsEnabled () ,
					widget->IsSelected () ,
					widget->IsUnderMouse (), 
					widget->IsDragable () ,
					widget->IsTransient (),
					widget->GetsInput ());	

				result += Unicode::narrowToWide (buf);
			}
		}

	}

	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------
// ChatBubbleParser
//-----------------------------------------------------------------

SwgCuiCommandParserUI::ChatBubbleParser::ChatBubbleParser () :
CommandParser ("chatBubble", 0, "...", "chatBubble.", 0)
{
	createDelegateCommands (ChatBubble::cmds);

} //lint !e429 custodial pointer setParser

//-----------------------------------------------------------------

bool SwgCuiCommandParserUI::ChatBubbleParser::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	UNREF (node);
	UNREF (originalCommand);
	UNREF (userId);

	if (isCommand (argv[0], ChatBubble::CommandNames::durationScale))
	{
		if (argv.size () > 1)
		{
			const float f = static_cast<float>(atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			CuiChatBubbleManager::setBubblesDurationScale (f);
		}
		else
		{
			_snprintf (buf_128, 128, "Value %5.2f\n", CuiChatBubbleManager::getBubblesDurationScale ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}
	else if (isCommand (argv[0], ChatBubble::CommandNames::durationMin))
	{
		if (argv.size () > 1)
		{
			const float f = static_cast<float>(atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			CuiChatBubbleManager::setBubblesDurationMin (f);
		}
		else
		{
			_snprintf (buf_128, 128, "Value %5.2f\n", CuiChatBubbleManager::getBubblesDurationMin ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}
	else if (isCommand (argv[0], ChatBubble::CommandNames::durationMax))
	{
		if (argv.size () > 1)
		{
			const float f = static_cast<float>(atof (Unicode::wideToNarrow (argv [1]).c_str ()));
			CuiChatBubbleManager::setBubblesDurationMax (f);
		}
		else
		{
			_snprintf (buf_128, 128, "Value  %5.2f\n", CuiChatBubbleManager::getBubblesDurationMax ());
			result += Unicode::narrowToWide (buf_128);
		}
		return true;
	}
	else if (isCommand (argv[0], ChatBubble::CommandNames::enabled))
	{
		CuiChatBubbleManager::setBubblesEnabled (argv [1][0] != '0');
	}
	else if (isCommand (argv[0], ChatBubble::CommandNames::minimized))
	{
		CuiChatBubbleManager::setBubblesMinimized (argv [1][0] != '0');
	}
	else
	{
		result += getFullErrorMessage (ERR_INVALID_ARGUMENTS);
	}

	return true;
}

//-----------------------------------------------------------------

int SwgCuiCommandParserUINamespace::nextPowerOfTwo(int x)
{
	x--;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x++;
	return x;
}

//-----------------------------------------------------------------

int SwgCuiCommandParserUINamespace::dumpStyleToFile(Unicode::NarrowString const & fileName, UIImageStyle const * const imageStyle, Gl_imageFormat const imageFormat)
{
	UIRect const & imageRect = imageStyle->GetSourceRect();

	int const sortKey = imageRect.left + (imageRect.top * 512) + (imageRect.right * 512 * 512) + (imageRect.bottom * 512 * 512 * 512);

	CuiLayer::TextureCanvas const * const textureCanvas = dynamic_cast<CuiLayer::TextureCanvas const *>(imageStyle->GetSourceCanvas());
	if (textureCanvas) 
	{
		StaticShader const * const staticShader = textureCanvas->getShader();
		if (staticShader) 
		{
			const Texture * texture = 0;
			if (staticShader->getTexture(TAG(M,A,I,N), texture))
			{
			
				Texture::LockData lockData(TF_ARGB_8888, 0, 0, 0, texture->getWidth(), texture->getHeight(), false);
				texture->lockReadOnly(lockData);

				Rectangle2d rect2d;

				rect2d.x0 = static_cast<float>(imageRect.left);
				rect2d.y0 = static_cast<float>(imageRect.top);
				rect2d.x1 = static_cast<float>(imageRect.right);
				rect2d.y1 = static_cast<float>(imageRect.bottom);

				Graphics::writeImage(fileName.c_str(), abs(imageRect.Width()), abs(imageRect.Height()), lockData.getPitch(), static_cast<int *>(lockData.getPixelData()), imageRect.Width() <= 32, imageFormat, &rect2d);

				texture->unlock(lockData);
			}
		}
	}

	return sortKey;
}

//-----------------------------------------------------------------

std::string SwgCuiCommandParserUINamespace::getDisplayGroup(uint32 const group)
{
	HashStringMap::const_iterator const itDisp = s_displayGroupNameHashMap.find(group);
	if (itDisp != s_displayGroupNameHashMap.end()) 
	{
		return itDisp->second;
	}

	return "Unknown";
}

//-----------------------------------------------------------------

std::string SwgCuiCommandParserUINamespace::getCommandGroup(uint32 const group)
{
	HashStringMap::const_iterator const itCmd = s_commandGroupNameHashMap.find(group);
	if (itCmd != s_commandGroupNameHashMap.end()) 
	{
		return itCmd->second;
	}
	
	return "Unknown";
}

//-----------------------------------------------------------------

std::vector<std::string> SwgCuiCommandParserUINamespace::getProfessionsForSkill(SkillManager::SkillMap const & skillz, std::string const & commandName)
{
	std::vector<std::string> professions;

	for (SkillManager::SkillMap::const_iterator itSkill = skillz.begin(); itSkill != skillz.end(); ++itSkill) 
	{
		SkillObject const * const skillObject = itSkill->second;
		if (skillObject && (skillObject->hasCommand(commandName) || skillObject->hasCommand(Unicode::toLower(commandName))))
		{
			professions.push_back(skillObject->getSkillName());
		}
	}

	return professions;
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserUINamespace::dumpCommandsToHtml(std::string const & filePath, bool const verbose)
{
	FILE * htmlFile = fopen((filePath + s_commandHtmlFileName).c_str(), "wt");
	if (htmlFile) 
	{
		// Get the command map.
		CommandTable::CommandMap const & commandMap = CommandTable::getCommandMap();
		
		// Move command by name into a unique command map.
		UniqueCommands uniqueCommands;

		// Add commands.
		{
			CommandTable::CommandMap::const_iterator itCommand = commandMap.begin();
			CommandTable::CommandMap::const_iterator const itCommandEnd = commandMap.end();
			
			for (; itCommand != itCommandEnd; ++itCommand) 
			{
				Command const & cmd = itCommand->second;
				uniqueCommands.insert(std::make_pair(cmd.m_commandName, cmd));
			}
		}

		// Add moods.
		{
			MoodManager::StringIntMap sim;
			MoodManager::getCanonicalMoods(sim);
			
			for (SocialsManager::StringIntMap::const_iterator it = sim.begin (); it != sim.end (); ++it)
			{
				Command dummy;
				dummy.m_commandName = it->first;
				uniqueCommands.insert(std::make_pair(it->first, dummy));
			}
		}

		// Add socials
		{
			SocialsManager::StringIntMap sim;
			SocialsManager::getCanonicalSocials(sim);
			
			for (SocialsManager::StringIntMap::const_iterator it = sim.begin(); it != sim.end(); ++it)
			{
				Command dummy;
				dummy.m_commandName = it->first;
				uniqueCommands.insert(std::make_pair(it->first, dummy));
			}
		}

		SkillManager::SkillMap const & skillMap = SkillManager::getInstance().getSkillMap();

		// Dump html header.
		fprintf(htmlFile, "<html><head><title>SWG Player Commands</title></head><body><p>Command Listing</p><table border=\"1\">\n");

		if (verbose) 
		{
			fprintf(htmlFile, "<th>Image</th><th>Command Name</th><th>Profession</th><th>Description</th><th>Script Hook</th><th>Character Ability</th><th>Display Group</th><th>Command Group</th><th>UIBuilder Image Path</th><th>Texture Rectangle (left, top, right, bottom)</th><th>Image ID</th>\n");
		}

		// Dump default image style.
		UIImageStyle const * const fallbackStyle = CuiIconManager::getFallback();

		// Dump the image to a file.
		int const defaultSortKey = dumpStyleToFile((filePath + "default.png").c_str(), fallbackStyle);

		std::map<int, int> imageIdMap;

		// Now, find all commands that have icons and do stuff with them.
		if (!uniqueCommands.empty()) 
		{
			UniqueCommands::const_iterator itCommand = uniqueCommands.begin();
			UniqueCommands::const_iterator const itCommandEnd = uniqueCommands.end();
			
			for (; itCommand != itCommandEnd; ++itCommand) 
			{
				Unicode::NarrowString const & cmdName = itCommand->first;
				Command const & cmd = itCommand->second;
				
				CuiDragInfo dragInfo;
				
				dragInfo.type = CuiDragInfoTypes::CDIT_command;
				IGNORE_RETURN(dragInfo.str.assign(1, '/'));
				IGNORE_RETURN(dragInfo.str.append(Unicode::toLower(cmdName)));
				
				dragInfo.commandValueValid = true;
				dragInfo.commandValue      = 0;
				
				UIImageStyle const * const imageStyle  = CuiIconManager::findIconImageStyle(dragInfo);

				bool isUsingFallbackStyle = true;

				int sortKey = defaultSortKey;
				
				if (imageStyle != fallbackStyle) 
				{
					// Dump the image to a file.
					sortKey = dumpStyleToFile((filePath + cmdName + ".png").c_str(), imageStyle);
					isUsingFallbackStyle = false;
				}
			
				Unicode::String description;
				IGNORE_RETURN(CuiSkillManager::localizeCmdDescription(Unicode::toLower(cmdName), description));
				
				UINarrowString const & imagePath = imageStyle->GetFullPath();
				UIRect const & imageRect = imageStyle->GetSourceRect();

				// Begin
				fprintf(htmlFile, "<tr>");

				// Image.
				fprintf(htmlFile,"<td valign=\"middle\" align=\"left\"><img name=\"%s\" src=\"%s\" width=\"%d\" height=\"%d\" alt=\"%s\"></td>",
					cmdName.c_str(),
					isUsingFallbackStyle ? "default.png" : (cmdName + ".png").c_str(),
					nextPowerOfTwo(imageRect.Width()) * 2, nextPowerOfTwo(imageRect.Height()) * 2,
					"default.png");

				// Command Name.
				fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", cmdName.c_str());
				
				// Profession Name.
				{
					fprintf(htmlFile,"<td valign=\"middle\" align=\"middle\">");

					std::vector<std::string> const & professionNames = getProfessionsForSkill(skillMap, cmdName);
					for (std::vector<std::string>::const_iterator itProf = professionNames.begin(); itProf != professionNames.end(); ++itProf) 
					{
						fprintf(htmlFile,"%s", (*itProf).c_str());

						if ((itProf + 1) != professionNames.end()) 
						{
							fprintf(htmlFile,", ");
						}
					}

					fprintf(htmlFile,"</td>");
				}

				// Description.
				fprintf(htmlFile,"<td valign=\"middle\" align=\"justify\">%s</td>", Unicode::wideToNarrow(description).c_str());

				if (verbose) 
				{
					// Script hook.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", cmd.m_scriptHook.c_str());

					// Character ability.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", cmd.m_characterAbility.c_str());

					// Display Group.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", getDisplayGroup(cmd.m_displayGroup).c_str());

					// Command Group.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", getCommandGroup(cmd.m_commandGroup).c_str());

					// UI builder Image Path.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%s</td>", imagePath.c_str());

					// Sub-rect.
					fprintf(htmlFile,"<td valign=\"middle\" align=\"center\">%d, %d, %d, %d</td>", imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
				}


				// Image group.
				{
					int uniqueImageId = -1;
					
					std::map<int, int>::const_iterator itUniqueImage = imageIdMap.find(sortKey);
					if (itUniqueImage == imageIdMap.end()) 
					{
						uniqueImageId = static_cast<int>(imageIdMap.size());
						imageIdMap[sortKey] = uniqueImageId;
					}
					else
					{
						uniqueImageId = itUniqueImage->second;
					}
					
					fprintf(htmlFile,"<td valign=\"middle\" align=\"right\">%d</td>", uniqueImageId);
				}
				
				// End
				fprintf(htmlFile,"</tr>\n");
			}
		}

		fprintf(htmlFile, "</table></body></html>\n");	
		fclose(htmlFile);
	}
	
	return true;
}

//-----------------------------------------------------------------

bool SwgCuiCommandParserUINamespace::dumpCollectionImagesToFile(std::string const& filePath)
{
	//note to self, I don't own any of these pointers, so no messing with them

	const UIPage * const rootPage = UIManager::gUIManager ().GetRootPage ();
	FILE * cropbat = fopen((filePath + "cropimages.bat").c_str(), "wt");

	if(!cropbat)
	{
		return false;
	}

	//bat file set up stuff
	fprintf(cropbat, "REM cropimages.bat\n");
	fprintf(cropbat, "@ECHO off\n");
	fprintf(cropbat, "REM This batch file is generated by dumpCollectionsImages\n");
	fprintf(cropbat, "REM The first argument should be the path where jpegtran.exe can be found\n");
	fprintf(cropbat, "REM for example c:\\work\\swg\\all\\tools\\design\\\n");

	//add the search path we were given
	fprintf(cropbat, "IF not %%1 == \"\" (\nSETLOCAL\npath=%%1;%%path%%\n)\n");


	//grab all of the books from the collection data table
	std::vector<CollectionsDataTable::CollectionInfoBook const *> const allBooks = CollectionsDataTable::getAllBooks();
	int numBooks = allBooks.size();
	for(int bookNumber = 0; bookNumber < numBooks; ++bookNumber)
	{
		CollectionsDataTable::CollectionInfoBook const * thisBook = allBooks[bookNumber];
		std::string bookName = thisBook->name;

		fprintf(cropbat, "REM Icons in book %s:\n", bookName.c_str());

		//pages in the book...
		std::vector<CollectionsDataTable::CollectionInfoPage const *> const pages = CollectionsDataTable::getPagesInBook(bookName);
		int numPages = pages.size();
		for(int pageNumber = 0; pageNumber < numPages; ++pageNumber)
		{
			CollectionsDataTable::CollectionInfoPage const* thisPage = pages[pageNumber];
			std::string pageName = thisPage->name;
			
			fprintf(cropbat, "REM Icons in page %s:\n", pageName.c_str());

			//collections in the page...
			std::vector<CollectionsDataTable::CollectionInfoCollection const *> const collections = CollectionsDataTable::getCollectionsInPage(pageName);
			int numCollections = collections.size();
			for(int collectionNumber = 0; collectionNumber < numCollections; ++collectionNumber)
			{
				CollectionsDataTable::CollectionInfoCollection const* thisCollection = collections[collectionNumber];
				std::string collectionName = thisCollection->name;

				//Turns out we don't need to dump the icons for each slot, just for the collection rewards.
				//If we need the slots later, we can always pull it out of perforce

				std::string imageStylePath;

				std::string const baseCollectionPath = "/Styles.collection.icon.reward.";
				
				if (!thisCollection->icon.empty())
				{
					imageStylePath = thisCollection->icon;
				}
				else
				{
					imageStylePath = baseCollectionPath + collectionName;
				}
				
				const UIImageStyle *const imageStyle = safe_cast<UIImageStyle *>(rootPage->GetObjectFromPath (imageStylePath.c_str ()));

				if(imageStyle != NULL)
				{
					const UISize& size = imageStyle->GetSize();
					std::string fullFileName(filePath + collectionName + ".jpg");
					dumpStyleToFile(fullFileName, imageStyle, GLIF_jpg);
					fprintf(cropbat, "jpegtran.exe -crop %dx%d+0+0 %s %s\n", size.x, size.y, fullFileName.c_str(), fullFileName.c_str());
				}

			}//collections
		}//pages
	}//books

	fprintf(cropbat, "ECHO Cropping done, check console output for error messages\n");
	fprintf(cropbat, "PAUSE\n");
	fclose(cropbat);

	return true;
}

//-----------------------------------------------------------------

void SwgCuiCommandParserUINamespace::debugBuffs(float const duration)
{
	if (Game::getSinglePlayer())
	{
		CreatureObject * const player = Game::getPlayerCreature();
		if (player)
		{
			CreatureObject * const lookAtTarget = dynamic_cast<CreatureObject *>(player->getLookAtTarget().getObject());
			CreatureObject * const target =  lookAtTarget ? lookAtTarget : player;
			
			uint32 const timestamp = *(reinterpret_cast<uint32 const *>(&duration)) + player->getPlayedTime();
			
			for (size_t buffCount = 0; buffCount < s_clientBuffsCount; ++buffCount) 
			{
				uint32 const buffHash = s_clientBuffs[buffCount];
				
				if (duration > 0.0f) 
				{
					target->addBuffDebug(buffHash, timestamp, 500.0f);
				}
				else
				{
					target->removeBuffDebug(buffHash);
				}
			}
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCommandParserUINamespace::AddStylePaths(UIBaseObject * obj, std::set<UIWidgetStyle *> & styles)
{
	if (obj) 
	{
		UIBaseObject::UIObjectList ov;
		obj->GetChildren(ov);
		
		// Grab all the styles from the selected pages.
		for (UIBaseObject::UIObjectList::const_iterator it = ov.begin(); it != ov.end(); ++it)
		{
			UIBaseObject * const obj = *it;
			UIBaseObject::UIPropertyNameMap  const * const pmap = obj->GetPropertyMap();
			
			if (pmap)
			{
				for (UIBaseObject::UIPropertyNameMap::const_iterator pit = pmap->begin(); pit != pmap->end(); ++pit)
				{
					Unicode::String const & value = (*pit).second;
					
					if (!value.empty() && value[0] == '/')
					{
						std::string const & narrow = Unicode::toLower(Unicode::wideToNarrow(value));
						
						if (strncmp(narrow.c_str(), "/styles.", 8) != 0)
							continue;
						
						UIWidgetStyle * const styleObject = static_cast<UIWidgetStyle * const>(UIManager::gUIManager().GetObjectFromPath(narrow.c_str(), TUIWidgetStyle));
						if (styleObject && !styleObject->IsA(TUITextStyle))
						{
							styles.insert(styleObject);
						}
					}
				}
			}
			
			AddStylePaths(obj, styles);
		}
	}
}

//-----------------------------------------------------------------

std::string SwgCuiCommandParserUINamespace::makeValidFileName(std::string const & str)
{
	std::string newStr(str);
	
	if (!newStr.empty()) 
	{
		if (newStr[0] == '/') 
			newStr.erase(newStr.begin());
		
		int pos = 0;
		while ((pos = newStr.find_first_of('.')) != std::string::npos) 
			newStr[pos] = '-';
	}
	
	return newStr;
}
//-----------------------------------------------------------------

void SwgCuiCommandParserUINamespace::textureRectangle(Texture::LockData & lockData, UIRect const & imageRect, UIColor const & color)
{
	int const textureWidth = imageRect.Width();
	int const textureHeight = imageRect.Height();

	int * lockedPixels = reinterpret_cast<int *>(lockData.getPixelData());
	int const destPitch = lockData.getPitch() / sizeof(int);
	
	int const subLeft = imageRect.left;
	int const subTop = imageRect.top;

	unsigned int pixel = static_cast<unsigned int>(color.FormatRGBA()); // rls - format rgba == format argb

	for (int yp = 0; yp < textureHeight; ++yp) 
	{
		int const destPixelOffset = (yp + subTop) * destPitch;
		
		for (int xp = 0; xp < textureWidth; ++xp) 
		{
			lockedPixels[(xp + subLeft) + destPixelOffset] = pixel;
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiCommandParserUINamespace::exportSelectedMediatorStyles()
{
	CuiWorkspace * workspace = CuiWorkspace::getGameWorkspace();
	CuiMediator * mediator = workspace ? workspace->getFocusMediator() : NULL;
	if (mediator) 
	{
		UIBaseObject * selectedObject = &(mediator->getPage());
		if (selectedObject) 
		{
			std::set<UIWidgetStyle *> styles;
			AddStylePaths(selectedObject, styles);

			UISmartPointer<UICanvas> const defaultStyleCanvas(UIManager::gUIManager().GetCanvas(Unicode::narrowToWide("ui_rebel_final")));
			if (defaultStyleCanvas)
			{
				UISmartPointer<UICanvas> scratchCanvas(UIManager::gUIManager().CreateCanvas("ui_rebel_final"));
				if (scratchCanvas)
				{
					// get the size of the source style.
					UISize imageSize;
					defaultStyleCanvas->GetSize(imageSize);
					UIRect imageRect(UIPoint::zero, imageSize);
			
					// Save out all the styles.
					std::set<UIWidgetStyle *>::const_iterator itStyle = styles.begin();
					std::set<UIWidgetStyle *>::const_iterator const itStyleEnd = styles.end();
					for (; itStyle != itStyleEnd; ++itStyle)
					{
						UIWidgetStyle const * const styleObject = *itStyle;
						if (styleObject && !styleObject->IsA(TUITextStyle))
						{
							UIWidgetRectangleStyles const & rectStyles = styleObject->GetRectangleStyles();
							UIRectangleStyle const * const defaultStyle = rectStyles.GetStyle(UIWidgetRectangleStyles::RS_Default);
							
							if (defaultStyle) 
							{
								// grab each image in the style and write to disk.
								for (int image = 0; image < UIRectangleStyle::I_LastImage; ++image) 
								{
									UIImageStyle * const imageStyle = defaultStyle->GetImage(static_cast<UIRectangleStyle::Image>(image));
									if (imageStyle) 
									{
										UIRect const & sourceRect = imageStyle->GetSourceRect();

										UIRect rect(std::min(sourceRect.left, sourceRect.right),  // left
													std::min(sourceRect.top, sourceRect.bottom),  // top
													std::max(sourceRect.left, sourceRect.right),  // right
													std::max(sourceRect.top, sourceRect.bottom)); // bottom

										if (rect.Width() > 0 && rect.Height() > 0) 
										{
											CuiLayer::TextureCanvas const * const textureCanvas = dynamic_cast<CuiLayer::TextureCanvas const *>(scratchCanvas.pointer());
											if (textureCanvas) 
											{
												StaticShader const * const staticShader = textureCanvas->getShader();
												if (staticShader) 
												{
													const Texture * texture = 0;
													if (staticShader->getTexture(TAG(M,A,I,N), texture))
													{
														Texture::LockData lockData(TF_ARGB_8888, 0, 0, 0, texture->getWidth(), texture->getHeight(), false);
														texture->lockReadOnly(lockData);

														// clear out the entire rectangle.
														textureRectangle(lockData, imageRect, UIColor(0, 0, 0, 0));

														// draw the magic square!
														textureRectangle(lockData, rect, UIColor(0, 128, 0, 170));

														std::string filename = mediator->getMediatorDebugName() + "_" + makeValidFileName(styleObject->GetFullPath()) + "_" + imageStyle->GetName() + ".tga";
														
														Graphics::writeImage(filename.c_str(), abs(imageRect.Width()), abs(imageRect.Height()), lockData.getPitch(), static_cast<int *>(lockData.getPixelData()), false, GLIF_tga, NULL);
														
														texture->unlock(lockData);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// ======================================================================
