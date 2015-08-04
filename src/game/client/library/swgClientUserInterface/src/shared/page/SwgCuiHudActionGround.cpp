//======================================================================
//
// SwgCuiHudActionGround.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudActionGround.h"

#include "clientGame/ClientWorld.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiSurveyManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerGround.h"

//======================================================================

namespace SwgCuiHudActionGroundNamespace
{
	float const cms_maxDistanceToUseSpaceTerminal = 16.0f;
}

using namespace SwgCuiHudActionGroundNamespace;

//======================================================================

SwgCuiHudActionGround::SwgCuiHudActionGround (SwgCuiHud & mediator) :
SwgCuiHudAction (mediator)
{
	CuiActionManager::addAction (CuiActions::showSurvey,               this, false);
	CuiActionManager::addAction (CuiActions::survey,                   this, false);
	CuiActionManager::addAction (CuiActions::shipChoose,               this, false);
	CuiActionManager::addAction (CuiActions::shipView,                 this, false);
	CuiActionManager::addAction (CuiActions::droidCommand,             this, false);
	CuiActionManager::addAction (CuiActions::spawnGroupLootLottery, this, false);
	CuiActionManager::addAction (CuiActions::closeGroupLootLottery, this, false);
	CuiActionManager::addAction (CuiActions::cybernetics,           this, false);
}

//----------------------------------------------------------------------

SwgCuiHudActionGround::~SwgCuiHudActionGround ()
{
}

//----------------------------------------------------------------------

SwgCuiHudWindowManagerGround & SwgCuiHudActionGround::getWindowManagerGround () const
{
	return *NON_NULL(safe_cast<SwgCuiHudWindowManagerGround *>(&getWindowManager()));
}

//----------------------------------------------------------------------

bool  SwgCuiHudActionGround::performAction (const std::string & id, const Unicode::String & params) const
{
	if (id == CuiActions::survey)
	{
		CuiSurveyManager::doSurvey();
	}
#if PRODUCTION == 0
	//access the survey ui
	else if (id == CuiActions::showSurvey)
	{
		getWindowManagerGround().spawnSurvey ();
	}
#endif

	else if (id == CuiActions::shipChoose)
	{
		ClientWorld::ObjectVector result;
		ClientWorld::findObjectsInRange(Game::getPlayerCreature()->getPosition_w(), cms_maxDistanceToUseSpaceTerminal, result);
		Object * const player = Game::getPlayer();
		if(player)
		{
			ClientObject const * closestSpaceTerminal = NULL;
			for(ClientWorld::ObjectVector::const_iterator i = result.begin(); i != result.end(); ++i)
			{
				Object const * const o = *i;
				ClientObject const * const co = o ? o->asClientObject() : NULL;
				if(co && (co->getGameObjectType() == SharedObjectTemplate::GOT_terminal_space))
				{
					if(!closestSpaceTerminal)
					{
						closestSpaceTerminal = co;
						DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiHudActionGround::performAction - found terminal [%s].\n", closestSpaceTerminal->getNetworkId().getValueString().c_str()));
					}
					else
					{
						float const currentTerminalRangeSquared	= (player->getPosition_w() - closestSpaceTerminal->getPosition_w()).magnitudeSquared();
						float const newTerminalRangeSquared	= (player->getPosition_w() - co->getPosition_w()).magnitudeSquared();
						if(newTerminalRangeSquared < currentTerminalRangeSquared)
						{
							closestSpaceTerminal = co;
							DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiHudActionGround::performAction - found closer terminal [%s].\n", closestSpaceTerminal->getNetworkId().getValueString().c_str()));
						}
					}
				}
			}

			if(closestSpaceTerminal)
			{
				Controller * const controller = NON_NULL (Game::getPlayer()->getController());
				if(controller)
				{
					MessageQueueGenericValueType<NetworkId> * const msg = new MessageQueueGenericValueType<NetworkId>(closestSpaceTerminal->getNetworkId());
					controller->appendMessage (CM_spaceTerminalRequest, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
				} //lint !e429 msg not freed (controller owns it)

				DEBUG_REPORT_LOG_PRINT(true, ("SwgCuiHudActionGround::performAction - spawning ShipChoose with terminal [%s].\n", closestSpaceTerminal->getNetworkId().getValueString().c_str()));
				getWindowManagerGround().spawnShipChoose(closestSpaceTerminal->getNetworkId());
			}
			else
				CuiSystemMessageManager::sendFakeSystemMessage (Unicode::narrowToWide("You must be near a terminal to do this.")); //TODO localize
		}
	}

	else if (id == CuiActions::shipView)
	{
		std::vector<Unicode::String> result;
		IGNORE_RETURN(Unicode::tokenize(params, result));
		if(result.size() == 2)
		{
			std::string const & shipIdStr = Unicode::wideToNarrow(result[0]);
			NetworkId const shipId(shipIdStr);
			std::string const & terminalIdStr = Unicode::wideToNarrow(result[1]);
			NetworkId const terminalId(terminalIdStr);
			getWindowManagerGround().spawnShipView(shipId, terminalId);
		}
	}

	else if (id == CuiActions::droidCommand)
	{
		std::string const & droidControlDeviceIdStr = Unicode::wideToNarrow(params);
		NetworkId const droidControlDeviceId(droidControlDeviceIdStr);
		getWindowManagerGround().spawnDroidCommand(droidControlDeviceId);
	}

	else if (id == CuiActions::spawnGroupLootLottery)
	{
		std::string const & inventoryIdStr = Unicode::wideToNarrow(params);
		NetworkId const inventoryId(inventoryIdStr);
		getWindowManagerGround().spawnGroupLootLottery(inventoryId);
	}

	else if (id == CuiActions::closeGroupLootLottery)
	{
		std::string const & inventoryIdStr = Unicode::wideToNarrow(params);
		NetworkId const inventoryId(inventoryIdStr);
		getWindowManagerGround().closeGroupLootLottery(inventoryId);
	}

	else if (id == CuiActions::cybernetics)
	{
		MessageQueueCyberneticsOpen::OpenType openType = MessageQueueCyberneticsOpen::OT_VIEW;
		NetworkId npc = NetworkId::cms_invalid;
		if(!params.empty())
		{
			std::vector<Unicode::String> result;
			IGNORE_RETURN(Unicode::tokenize(params, result));
			if(result.size() >= 2)
			{
				npc = NetworkId(Unicode::wideToNarrow(result[0]));
				openType = static_cast<MessageQueueCyberneticsOpen::OpenType>(atoi(Unicode::wideToNarrow(result[1]).c_str()));
			}
		}
		getWindowManagerGround().spawnCybernetics(npc, openType);
	}

	else
		return SwgCuiHudAction::performAction (id, params);

	return true;
}

//======================================================================
