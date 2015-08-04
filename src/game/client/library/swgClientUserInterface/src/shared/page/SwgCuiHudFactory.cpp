
//======================================================================
//
// SwgCuiHudFactory.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudFactory.h"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientUserInterface/CuiChatHistory.h"
#include "clientUserInterface/CuiIMEManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "sharedObject/Object.h"
#include "swgClientUserInterface/SwgCuiHud.h"
#include "swgClientUserInterface/SwgCuiHudGround.h"
#include "swgClientUserInterface/SwgCuiHudSpace.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIIMEManager.h"
#include "UIManager.h"
#include "UITextStyle.h"

#include <bitset>

//======================================================================

namespace SwgCuiHudFactoryNamespace
{
	char const * findMediatorNameForSceneType(Game::SceneType sceneType)
	{
		char const * mediatorName = NULL;
		switch(sceneType)
		{
		case Game::ST_ground:
			mediatorName = CuiMediatorTypes::HudGround;
			break;
		case Game::ST_space:
			mediatorName = CuiMediatorTypes::HudSpace;
			break;
		case Game::ST_reset:
		case Game::ST_numTypes:
			break;
		default:
			DEBUG_FATAL(true,("SwgCuiHudFactoryNamespace::findMediatorNameForSceneType() invalid scene type [%d]", sceneType));
			break;
		}

		return mediatorName;
	}

	UINarrowString const s_uiStyleName("ui_rebel_final");
	UINarrowString const s_uiTextureDefault("ui_rebel_final");
	UINarrowString const s_uiTextureSpace("ui_rebel_final_space");
}

using namespace SwgCuiHudFactoryNamespace;

//----------------------------------------------------------------------

void SwgCuiHudFactory::setHudActive(bool active)
{
	char const * const mediatorName = findMediatorNameForSceneType(Game::getHudSceneType());

	if (mediatorName != NULL)
	{
		if (active)
		{
			CuiMediatorFactory::activate(mediatorName, NULL, false);
		}
		else
		{
			CuiMediatorFactory::deactivate(mediatorName, false);
		}

		CuiSettings::enableAutomaticSave(active);
	}
	else
		DEBUG_FATAL(active,("SwgCuiHudFactory::setHudActive(%d) failed to activate", active ? 1 : 0));
}

//----------------------------------------------------------------------

void SwgCuiHudFactory::createHudTemplates()
{
	SwgCuiHudGround::createHudTemplate();
	SwgCuiHudSpace::createHudTemplate();
}

//----------------------------------------------------------------------

void SwgCuiHudFactory::releaseHudIfNeeded()
{
	setHudActive(false);
}

//----------------------------------------------------------------------

void SwgCuiHudFactory::createHudIfNeeded()
{
	Game::SceneType sceneType = Game::getHudSceneType();
	
	const bool needsCreate = shouldCreateInterface();

	//-- set the CuiSettings prefix string
	switch(sceneType)
	{
	case Game::ST_space:
		CuiSettings::setPrefixString ("space_");
		break;
	case Game::ST_ground:
	case Game::ST_numTypes:
		CuiSettings::setPrefixString ("");
		break;
	case Game::ST_reset:
	default:
		DEBUG_FATAL(true,("SwgCuiHudFactory::createHudTemplates() invalid last scene type [%d]", sceneType));
		break;
	}
	
	CuiSettings::load();
	CuiChatHistory::load();

	// Refresh values after a scene change.
	CuiPreferences::refreshValues();
	
	if (needsCreate)
	{		
		switch(sceneType)
		{
		case Game::ST_ground:
			if (CuiMediatorFactory::get(CuiMediatorTypes::HudGround, false))
			{
				if (!CuiMediatorFactory::attemptRelease(CuiMediatorTypes::HudGround))
				{
					WARNING(true, ("SwgCuiHudFactory unable to release HudGround"));
				}
				else
					CuiMediator::garbageCollect(true);
			}
			SwgCuiHudGround::createFreshHud ();
			break;
		case Game::ST_space:
			if (CuiMediatorFactory::get(CuiMediatorTypes::HudSpace, false))
			{
				if (!CuiMediatorFactory::attemptRelease(CuiMediatorTypes::HudSpace))
				{
					WARNING(true, ("SwgCuiHudFactory unable to release HudSpace"));
				}
				else
					CuiMediator::garbageCollect(true);
			}
			SwgCuiHudSpace::createFreshHud ();
			break;
		case Game::ST_numTypes:
		case Game::ST_reset:
			break;
		default:
			DEBUG_FATAL(true,("SwgCuiHudFactory::createHudTemplates() invalid last scene type [%d]", sceneType));
			break;
		}
	}

	GroundScene * const groundScene = dynamic_cast<GroundScene *>(Game::getScene());
	if (groundScene != NULL)
		groundScene->loadInputMap();

	char const * mediatorName = findMediatorNameForSceneType (sceneType);
	if (mediatorName)
	{
		CuiMediatorFactory::activate(mediatorName);
	}

	updateShaders();

	// Set the drop shadow stuff.
	UITextStyle::SetGlobalAutoDropShadowEnabled(!Game::isHudSceneTypeSpace());

	// Enable the IME window.
	CuiIMEManager::GetCuiIMEManager()->SetIMEIndicator(sceneType == Game::ST_space ? CuiMediatorTypes::WS_IMEIndicatorSpace : CuiMediatorTypes::WS_IMEIndicator);
	CuiIMEManager::GetCuiIMEManager()->NotifyIMEOpenStatus(true);

	// Force a save over existing data.
	if (needsCreate) 
	{
		CuiSettings::setDirty(true);
		CuiSettings::save();
		CuiChatHistory::setDirty(true);
		CuiChatHistory::save();
	}
}

//----------------------------------------------------------------------

SwgCuiHud * SwgCuiHudFactory::findMediatorForCurrentHud()
{
	char const * mediatorName = findMediatorNameForSceneType (Game::getHudSceneType());
	if (mediatorName != NULL)
		return safe_cast<SwgCuiHud *>(CuiMediatorFactory::get (mediatorName, false));
	return NULL;
}

//----------------------------------------------------------------------

bool SwgCuiHudFactory::isHudActive()
{
	SwgCuiHud const * const hud = findMediatorForCurrentHud();
	if (hud)
		return hud->isActive ();
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiHudFactory::shouldCreateInterface()
{	
	SwgCuiHud const * const hud = findMediatorForCurrentHud();
	if (NULL == hud)
		return true;

	std::string const & playerIdentificationString = SwgCuiHud::createPlayerIdentificationString();

	if (playerIdentificationString != hud->getPlayerIdentificationString())
		return true;

	return false;
}

//----------------------------------------------------------------------

void SwgCuiHudFactory::reset()
{
	// Flag the HUD for reset.
	Game::resetHud();
	UIManager::gUIManager().getUIIMEManager()->SetContextWidget(NULL);
	CuiIMEManager::GetCuiIMEManager()->SetIMEIndicator(0);
}

//----------------------------------------------------------------------

void SwgCuiHudFactory::updateShaders()
{
	// Replace textures in space and ground scenes.
	UIManager::gUIManager().replaceCanvasTexturesByName(s_uiStyleName, Game::isHudSceneTypeSpace() ? s_uiTextureSpace: s_uiTextureDefault);
}

//======================================================================
