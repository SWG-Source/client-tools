//======================================================================
//
// InputScheme.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/InputScheme.h"

#include "clientGame/ClientStringIds.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/ClientMacroManager.h"
#include "clientUserInterface/CuiInputMessage.def"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CurrentUserOptionManager.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include <map>
#include <vector>
#include <dinput.h>

//======================================================================

namespace
{
	struct Data
	{
		int         hudSceneType;
		std::string groundInputMapName;
		uint32      flags;

		Data () :
		groundInputMapName (),
		flags              (0)
		{}

		Data (int _hudSceneType, const std::string & _g, uint32 _flags) :
		hudSceneType(_hudSceneType),
		groundInputMapName (_g),
		flags              (_flags)
		{}
	};

	Callback * s_resetCallback = 0;
	typedef stdmap<std::string, Data>::fwd DataMap;
	DataMap s_dataMap[Game::ST_numTypes];

	bool s_installed = false;

	Data * findData (const std::string & type)
	{
		int const sceneType = Game::getHudSceneType();

		if (sceneType < 0 || sceneType >= Game::ST_numTypes)
		{
			WARNING(true, ("InputScheme findData() invalid scene type [%d]", sceneType));
			return NULL;
		}

		const DataMap::iterator it = s_dataMap[sceneType].find (type);
		if (it == s_dataMap[sceneType].end ())
			return 0;
		return &(*it).second;
	}

	InputMap *  s_groundInputMap = 0;
	int         s_groundInputMapRefCount  = 0;

	struct PlayerId
	{
		std::string     loginId;
		std::string     clusterName;
		Unicode::String playerName;
		NetworkId       id;

		std::string     toString () const
		{
			return std::string ("[") + loginId + "," + clusterName + "," + Unicode::wideToNarrow (playerName) + "," + id.getValueString () + "]";
		}
		
		std::string     toPath   () const
		{
			std::string suffix;

			int const sceneType = Game::getHudSceneType();			
			if (sceneType < 0 || sceneType >= Game::ST_numTypes)
				WARNING(true, ("InputScheme findData() invalid scene type [%d]", sceneType));
			else if (sceneType == Game::ST_space)
				suffix = "_space";

			return std::string ("profiles/") + loginId + "/" + clusterName + "/" + id.getValueString () + suffix + ".inp";
		}

		bool operator != (const PlayerId & rhs) const
		{
			return !(*this == rhs);
		}

		bool operator == (const PlayerId & rhs) const
		{
			return loginId == rhs.loginId && clusterName == rhs.clusterName && id == rhs.id;
		}
	};

	PlayerId    s_playerId;

	const std::string s_groundInputMajorVersion = "swg3"; // changing the major version will cause a reset to the default keymap
	const std::string s_groundInputMinorVersion1 = "minor1"; // update mapping of primaryActionAndAttack and secondaryAttack
	const std::string s_groundInputMinorVersion2 = "minor2"; // update mapping of intended target changes
	const std::string s_groundInputMinorVersionMostRecent = s_groundInputMinorVersion2;
	std::string s_defaultGroundInputSchemeTypes[] =
	{
		s_groundInputMajorVersion + "_" + s_groundInputMinorVersionMostRecent + "_default",
		s_groundInputMajorVersion + "_" + s_groundInputMinorVersionMostRecent + "_classic",
		s_groundInputMajorVersion + "_" + s_groundInputMinorVersionMostRecent + "_modern"
	};

	std::string s_defaultSpaceInputSchemeTypes[] =
	{
		"space"
	};

	std::string s_lastInputSchemeTypes[Game::ST_numTypes] = 
	{
		"",
		s_defaultSpaceInputSchemeTypes[0]
	};

	//----------------------------------------------------------------------
	
	void install ()
	{
		if (s_installed)
			return;
		
		const Data swg_ground    (Game::ST_ground, "input/groundinputmap_swg.iff",         
			InputScheme::F_modalChat 
			| InputScheme::F_chaseCam 
			| InputScheme::F_turnStrafes 
			| InputScheme::F_canFireSecondariesFromToolbar
			);
		const Data swg_classic_ground    (Game::ST_ground, "input/groundinputmap_swg_classic.iff",         
			InputScheme::F_modalChat 
			| InputScheme::F_chaseCam 
//			| InputScheme::F_mouseMode
			| InputScheme::F_canFireSecondariesFromToolbar
			);
		const Data swg_modern_ground    (Game::ST_ground, "input/groundinputmap_swg_modern.iff",         
			InputScheme::F_modalChat 
			| InputScheme::F_chaseCam 
			| InputScheme::F_turnStrafes 
			| InputScheme::F_mouseMode
			| InputScheme::F_canFireSecondariesFromToolbar
			| InputScheme::F_mouseLeftAndRightMoves
			);

		const Data space     (Game::ST_space,  "input/spaceinputmap_default.iff",      InputScheme::F_modalChat);
		const Data spaceja101(Game::ST_space,  "input/spaceinputmap_ja101.iff",        InputScheme::F_modalChat);

		s_dataMap[Game::ST_ground].insert (DataMap::value_type (s_defaultGroundInputSchemeTypes[0], swg_ground));
		s_dataMap[Game::ST_ground].insert (DataMap::value_type (s_defaultGroundInputSchemeTypes[1], swg_classic_ground));
		s_dataMap[Game::ST_ground].insert (DataMap::value_type (s_defaultGroundInputSchemeTypes[2], swg_modern_ground));

		s_dataMap[Game::ST_space].insert (DataMap::value_type ("space", space));
		s_dataMap[Game::ST_space].insert (DataMap::value_type ("spaceja101", spaceja101));
		s_installed = true;

		CurrentUserOptionManager::registerOption (s_lastInputSchemeTypes[Game::ST_ground], "ClientGame", "lastInputSchemeType");
		CurrentUserOptionManager::registerOption (s_lastInputSchemeTypes[Game::ST_space], "ClientGame", "lastInputSchemeTypeSpace");
	}

	//----------------------------------------------------------------------

	std::string s_typeToReset;
	void onResetConfirmClosed (const CuiMessageBox & box)
	{
		if (box.completedAffirmative ())
		{
			InputScheme::resetFromType (s_typeToReset, true);

			if (!CuiPreferences::getUseModelessInterface())
			{
				CuiManager::getIoWin().setPointerToggledOn(true);
			}
		}
	}

	//----------------------------------------------------------------------

	struct RebindInfo
	{
		uint32 theShiftState;
		InputMap::InputType theType;
		int32 theValue;
		const char *commandName;
		RebindInfo(uint32 inTheShiftState, InputMap::InputType inTheType, int32 inTheValue, const char *inCommandName)
		{
			theShiftState = inTheShiftState;
			theType = inTheType;
			theValue = inTheValue;
			commandName = inCommandName;
		}
	};


	void updateToVersion1()
	{
		s_lastInputSchemeTypes[Game::ST_ground] = s_defaultGroundInputSchemeTypes[0];
		InputMap::CommandBindInfoSet * cmdsBindInfoSets = 0;

		{
			const InputMap::Command * primaryActionAndAttackCommand = s_groundInputMap->findCommandByName("CMD_primaryActionAndAttack");
			const InputMap::BindInfo leftMouseButton(0, InputMap::IT_MouseButton, 0);
			if(primaryActionAndAttackCommand)
			{
				const uint32 numCmds = s_groundInputMap->getCommandBindings (cmdsBindInfoSets, primaryActionAndAttackCommand);
				if(numCmds) 
				{
					const InputMap::CommandBindInfoSet & cbis = cmdsBindInfoSets[0];
					if (cbis.numBinds == 0) // nothing is mapped to this command
					{
						s_groundInputMap->addBinding(leftMouseButton, primaryActionAndAttackCommand);
					}
					delete [] cmdsBindInfoSets;
					cmdsBindInfoSets = 0;
				}

			}
		}

		{
			const InputMap::Command * secondaryAttackCommand = s_groundInputMap->findCommandByName("CMD_secondaryAttack");
			const InputMap::BindInfo rightMouseButton(0, InputMap::IT_MouseButton, 1);
			if(secondaryAttackCommand)
			{
				const uint32 numCmds = s_groundInputMap->getCommandBindings (cmdsBindInfoSets, secondaryAttackCommand);
				if(numCmds) 
				{
					const InputMap::CommandBindInfoSet & cbis = cmdsBindInfoSets[0];
					if (cbis.numBinds == 0) // nothing is mapped to this command
					{
						s_groundInputMap->addBinding(rightMouseButton, secondaryAttackCommand);
					}
					delete [] cmdsBindInfoSets;
					cmdsBindInfoSets = 0;
				}
			}
		}
	}

	void updateVersion1ToVersion2()
	{
		s_lastInputSchemeTypes[Game::ST_ground] = s_defaultGroundInputSchemeTypes[0];

		//we thought we were going to do something smart here, 
		//		but decided not to... so this forever will be an empty version 
		//		because it already had gone live on a public TestCenter :(
	}

	//----------------------------------------------------------------------

	void writeUpdate(const std::string &filename)
	{
		if (!s_groundInputMap->write (0, false))
		{
			WARNING (true, ("InputScheme::fetchGroundInputMap() failed to write iff [%s]", filename.c_str ()));
		}
		LocalMachineOptionManager::save ();
		CurrentUserOptionManager::save ();
	}

}

//----------------------------------------------------------------------

void  InputScheme::getTypes      (StringVector & sv)
{
	if (!s_installed)
		install ();

	int const sceneType = Game::getHudSceneType();

	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("InputScheme invalid scene type [%d]", sceneType));
		return;
	}

	sv.reserve (s_dataMap[sceneType].size ());
	for (DataMap::const_iterator it = s_dataMap[sceneType].begin (); it != s_dataMap[sceneType].end (); ++it)
	{
		const std::string & name = (*it).first;
		sv.push_back (name);
	}
}

//----------------------------------------------------------------------

bool InputScheme::resetFromType (const std::string & type, bool confirmed)
{
	if (!s_installed)
		install ();

	if (!s_groundInputMap)
	{
		WARNING (true, ("resetFromType with no input map"));
		return false;
	}
	
	const Data * const data = findData (type);
	if (!data)
	{
		WARNING (true, ("resetFromType [%s] invalid", type.c_str ()));
		return false;
	}
	
	if (confirmed)
	{		
		int const sceneType = Game::getHudSceneType();

		if (sceneType < 0 || sceneType >= Game::ST_numTypes)
		{
			WARNING(true, ("InputScheme invalid scene type [%d]", sceneType));
			return false;
		}

		s_groundInputMap->setBaseFileName (data->groundInputMapName.c_str ());
		if (!s_groundInputMap->reload (true))
		{
			WARNING (true, ("Unable to reload inputmap"));
			return false;
		}
		
		const uint32 flags = data->flags;
		
		CuiPreferences::setUseSwgMouseMap( ( flags & F_swgMouseMap ) != 0 );
		CuiPreferences::setUseModelessInterface( ( flags & F_modeless ) != 0 );
		CuiPreferences::setModalChat        ( ( flags & F_modalChat ) || ( flags & F_modeless ) );
		CuiPreferences::setMouseModeDefault ((flags & F_mouseMode) != 0);
		CuiPreferences::setTurnStrafesDuringMouseModeToggle ((flags & F_turnStrafes) != 0);
		CuiPreferences::setCanFireSecondariesFromToolbar ((flags & F_canFireSecondariesFromToolbar) != 0);
		CuiPreferences::setMouseLeftAndRightDrivesMovementToggle((flags & F_mouseLeftAndRightMoves) != 0);
		FreeChaseCamera::setCameraMode                 ((flags & F_chaseCam) != 0? FreeChaseCamera::CM_chase : FreeChaseCamera::CM_free);
		
		s_lastInputSchemeTypes[sceneType] = type;

		ClientMacroManager::synchronizeWithInputMap (s_groundInputMap);

		s_resetCallback->performCallback ();

		CuiPreferences::signalKeybindingsChanged ();

		LocalMachineOptionManager::save ();
		CurrentUserOptionManager::save ();
	}
	else
	{
		 s_typeToReset = type;
		 const Unicode::String & localizedTypeName = InputScheme::localizeTypeName (type);
		 Unicode::String result;
		 CuiStringVariablesManager::process (ClientStringIds::input_scheme_confirm_reset_prose, localizedTypeName, Unicode::emptyString, Unicode::emptyString, result);
		 CuiMessageBox::createYesNoBox (result, &onResetConfirmClosed);
	}

	return true;
}

//----------------------------------------------------------------------

InputMap * InputScheme::fetchGroundInputMap()
{
	if (!s_installed)
		install ();

	PlayerId pid;
	if (!Game::getPlayerPath (pid.loginId, pid.clusterName, pid.playerName, pid.id))
	{
		FATAL (true, ("InputScheme::fetchGroundInputMap could not Game::getPlayerPath"));
		return 0;
	}

	if (!s_groundInputMap)
	{
		int const sceneType = Game::getHudSceneType();

		if (sceneType < 0 || sceneType >= Game::ST_numTypes)
		{
			FATAL(true, ("InputScheme::fetchGroundInputMap() invalid scene type [%d]", sceneType));
			return 0;
		}

		const Data * data = findData (s_lastInputSchemeTypes[sceneType]);
	
		if(!data && sceneType == Game::ST_ground)
		{
			// try and regrab the data - it may just be a minor version change
			if(s_lastInputSchemeTypes[Game::ST_ground].find(s_groundInputMajorVersion) != std::string::npos)
			{
				data = findData (s_defaultGroundInputSchemeTypes[0]);
			}
		}

		bool needsReset = false;

		bool shouldReset = !data;

		if(!shouldReset && sceneType == Game::ST_ground) // look for a major version change in the ground input map
		{
			shouldReset = (s_lastInputSchemeTypes[Game::ST_ground].find(s_groundInputMajorVersion) == std::string::npos);
		}

		if(shouldReset)
		{
			if(sceneType == Game::ST_ground)
			{
				data = NON_NULL (findData (s_defaultGroundInputSchemeTypes[0]));
				s_lastInputSchemeTypes[Game::ST_ground] = s_defaultGroundInputSchemeTypes[0];
			}
			else
			{
				data = NON_NULL (findData (s_defaultSpaceInputSchemeTypes[0]));
				s_lastInputSchemeTypes[Game::ST_space] = s_defaultSpaceInputSchemeTypes[0];
			}
			needsReset = true;
		}

		NOT_NULL (data);

		ClientMacroManager::load ();

		const std::string & customMap = pid.toPath ();
		s_groundInputMap = new InputMap (data->groundInputMapName.c_str (), customMap.c_str (), 0);


		// look for a minor version change
		if(!needsReset && sceneType == Game::ST_ground)
		{
			bool isMinorVersion1 = (s_lastInputSchemeTypes[Game::ST_ground].find(s_groundInputMinorVersion1) != std::string::npos);
			bool isMinorVersion2 = (s_lastInputSchemeTypes[Game::ST_ground].find(s_groundInputMinorVersion2) != std::string::npos);
			if(!isMinorVersion1 && !isMinorVersion2)
			{
				updateToVersion1();
				updateVersion1ToVersion2();
				writeUpdate(data->groundInputMapName);
			}
			else if(!isMinorVersion2)
			{
				updateVersion1ToVersion2();
				writeUpdate(data->groundInputMapName);
			}
		}

		ClientMacroManager::synchronizeWithInputMap (s_groundInputMap);
		s_resetCallback  = new Callback;
		s_resetCallback->fetch ();

		if ( needsReset )
		{
			resetFromType( s_lastInputSchemeTypes[sceneType] , true );
			if (!s_groundInputMap->write (0, false))
			{
				WARNING (true, ("InputScheme::fetchGroundInputMap() failed to write iff [%s]", data->groundInputMapName.c_str ()));
			}
		}

	}
	else
	{
		if (pid != s_playerId)
		{
			FATAL (true, ("InputScheme::fetchGroundInputMap called on a new player without releasing old one [%s]", pid.toString ().c_str ()));
			return 0;
		}
	}

	if (s_groundInputMap)
		++s_groundInputMapRefCount;

	return s_groundInputMap;
}

//----------------------------------------------------------------------

void InputScheme::releaseGroundInputMap (InputMap & imap)
{
	if (!s_installed)
		install ();

	UNREF (imap);

	DEBUG_FATAL (s_groundInputMapRefCount <= 0, ("InputScheme::releaseGroundInputMap with refcount=[%d", s_groundInputMapRefCount));
	NOT_NULL (s_groundInputMap);

	DEBUG_FATAL (s_groundInputMap != &imap, ("Attempt to InputScheme::releaseGroundInputMap invalid InputMap"));
	if (!--s_groundInputMapRefCount)
	{
		delete s_groundInputMap;
		s_groundInputMap = 0;
		s_resetCallback->release ();
		s_resetCallback = 0;
	}
}

//----------------------------------------------------------------------

Callback * InputScheme::getResetCallback      ()
{
	return s_resetCallback;
}

//----------------------------------------------------------------------

Unicode::String InputScheme::localizeTypeName    (const std::string & type)
{
	return StringId ("ui", std::string ("inputscheme_") + type + "_n").localize ();
}

//----------------------------------------------------------------------

std::string const & InputScheme::getLastInputSchemeType ()
{
	int const sceneType = Game::getHudSceneType();
	
	if (sceneType < 0 || sceneType >= Game::ST_numTypes)
	{
		WARNING(true, ("InputScheme::fetchGroundInputMap() invalid scene type [%d]", sceneType));
		return s_lastInputSchemeTypes[0];
	}
	
	return s_lastInputSchemeTypes[sceneType];
}

//======================================================================
