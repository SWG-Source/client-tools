// ============================================================================
//
// AwayFromKeyBoardManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/AwayFromKeyBoardManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiStringIdsAwayFromKeyBoard.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/CurrentUserOptionManager.h"

// ============================================================================
//
// AwayFromKeyBoardManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace AwayFromKeyBoardManagerNamespace
{
	bool            s_installed = false;
	time_t          s_touchTime = 0;
	bool            s_autoAwayFromKeyBoard = false;
	Unicode::String s_automaticResponseMessage;
	int const       s_automaticAwayFromKeyBoardMinutesMin = 1;
	int const       s_automaticAwayFromKeyBoardMinutesMax = 30;
	int const       s_automaticAwayFromKeyBoardMinutesDefault = 10;
	int             s_automaticAwayFromKeyBoardMinutes = s_automaticAwayFromKeyBoardMinutesDefault;
	bool            s_autoAwayFromKeyBoardEnabled = true;
	bool            s_resetAwayFromKeyBoard = false;
	Unicode::String s_defaultAFKMessage;
}

using namespace AwayFromKeyBoardManagerNamespace;

// ============================================================================
//
// AwayFromKeyBoardManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::install()
{
	InstallTimer const installTimer("AwayFromKeyBoardManager::install");

	DEBUG_FATAL(s_installed, ("AwayFromKeyBoardManager::install() - Already installed."));
	s_installed = true;
	ExitChain::add(AwayFromKeyBoardManager::remove, "AwayFromKeyBoardManager::remove", 0, false);

	s_automaticResponseMessage = CuiStringIdsAwayFromKeyBoard::away_from_keyboard_default_message.localize();
	s_defaultAFKMessage = s_automaticResponseMessage;
	CurrentUserOptionManager::registerOption(s_autoAwayFromKeyBoardEnabled, "ClientGame", "AwayFromKeyBoardAuto");
	CurrentUserOptionManager::registerOption(s_automaticResponseMessage, "ClientGame", "AwayFromKeyBoardResponseMessage");
	CurrentUserOptionManager::registerOption(s_automaticAwayFromKeyBoardMinutes, "ClientGame", "AwayFromKeyBoardMinutes");
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::remove()
{
	s_installed = false;
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::alter(float const deltaTime)
{
	UNREF(deltaTime);

	if (s_touchTime == 0)
	{
		s_touchTime = CuiUtils::GetSystemSeconds();
	}

	if (   !s_autoAwayFromKeyBoard
	    && (s_automaticAwayFromKeyBoardMinutes > 0)
		&& s_autoAwayFromKeyBoardEnabled)
	{
		time_t secondsElapsed = CuiUtils::GetSystemSeconds() - s_touchTime;

		if (secondsElapsed > s_automaticAwayFromKeyBoardMinutes * 60)
		{
			PlayerObject const * const playerObject = Game::getConstPlayerObject();

			if (   (playerObject != NULL)
			    && !playerObject->isAwayFromKeyBoard())
			{
				s_autoAwayFromKeyBoard = true;
				CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_on.localize());
				toggleAwayFromKeyBoard();
			}
		}
	}

	// If this is a new player and they just logged in and they are away from
	// the keyboard, change them to not-afk

	if (s_resetAwayFromKeyBoard)
	{
		PlayerObject *playerObject = Game::getPlayerObject();

		if (playerObject != NULL)
		{
			s_resetAwayFromKeyBoard = false;

			if (playerObject->isAwayFromKeyBoard())
			{
				playerObject->toggleAwayFromKeyBoard();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::touch()
{
	if (s_autoAwayFromKeyBoard)
	{
		s_autoAwayFromKeyBoard = false;

		// If the user was automatically put away from the keyboard, remove the
		// the status

		CuiSystemMessageManager::sendFakeSystemMessage(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_off.localize());
		toggleAwayFromKeyBoard();
	}

	s_touchTime = CuiUtils::GetSystemSeconds();
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::toggleAwayFromKeyBoard()
{
	PlayerObject const * const playerObject = Game::getConstPlayerObject();

	if (playerObject != NULL)
	{
		playerObject->toggleAwayFromKeyBoard();
	}
}

//-----------------------------------------------------------------------------
bool AwayFromKeyBoardManager::isAwayFromKeyBoard()
{
	bool result = false;

	PlayerObject const * const playerObject = Game::getConstPlayerObject();

	if ((playerObject != NULL) &&
		playerObject->isAwayFromKeyBoard())
	{
		result = true;
	}

	if (s_autoAwayFromKeyBoard)
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::setAutomaticAwayFromKeyBoardMinutes(int const automaticAwayFromKeyBoardMinutes)
{
	int min = getAutomaticAwayFromKeyBoardMinutesMin();
	int max = getAutomaticAwayFromKeyBoardMinutesMax();

	s_automaticAwayFromKeyBoardMinutes = clamp(min, automaticAwayFromKeyBoardMinutes, max);
}

//-----------------------------------------------------------------------------
int AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutes()
{
	return s_automaticAwayFromKeyBoardMinutes;
}

//-----------------------------------------------------------------------------
int AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesDefault()
{
	return s_automaticAwayFromKeyBoardMinutesDefault;
}

//-----------------------------------------------------------------------------
void AwayFromKeyBoardManager::setAutomaticResponseMessage(Unicode::String const &message)
{
	s_automaticResponseMessage = message;
}

//-----------------------------------------------------------------------------
Unicode::String const &AwayFromKeyBoardManager::getAutomaticResponseMessage()
{
	if(s_automaticResponseMessage.empty())
	{
		return s_defaultAFKMessage;
	}
	else
	{	
		return s_automaticResponseMessage;
	}
}

//-----------------------------------------------------------------------------
Unicode::String AwayFromKeyBoardManager::getAutomaticResponseMessagePrefix()
{
	return CuiStringIdsAwayFromKeyBoard::away_from_keyboard_automatic_response_prefix.localize();
}

//----------------------------------------------------------------------
bool AwayFromKeyBoardManager::isAutoAwayFromKeyBoardEnabled()
{
	return s_autoAwayFromKeyBoardEnabled;
}

//----------------------------------------------------------------------
bool AwayFromKeyBoardManager::isAutoAwayFromKeyBoardEnabledDefault()
{
	return true;
}

//----------------------------------------------------------------------
void AwayFromKeyBoardManager::setAutoAwayFromKeyBoardEnabled(bool enabled)
{
	s_autoAwayFromKeyBoardEnabled = enabled;
}

//----------------------------------------------------------------------
Unicode::String AwayFromKeyBoardManager::getAutomaticResponseMessageDefault()
{
	return s_defaultAFKMessage;
}

//----------------------------------------------------------------------
int AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesMin()
{
	return s_automaticAwayFromKeyBoardMinutesMin;
}

//----------------------------------------------------------------------
int AwayFromKeyBoardManager::getAutomaticAwayFromKeyBoardMinutesMax()
{
	return s_automaticAwayFromKeyBoardMinutesMax;
}

//----------------------------------------------------------------------
void AwayFromKeyBoardManager::resetAwayFromKeyBoard()
{
	s_resetAwayFromKeyBoard = true;
}

// ============================================================================
