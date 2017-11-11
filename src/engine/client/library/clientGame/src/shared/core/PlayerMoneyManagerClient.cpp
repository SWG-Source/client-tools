//======================================================================
//
// PlayerMoneyManagerClient.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerMoneyManagerClient.h"

#include "clientGame/Game.h"
#include "clientGame/ClientObject.h"

//======================================================================

namespace PlayerMoneyManagerClientNamespace
{

}

//----------------------------------------------------------------------

using namespace PlayerMoneyManagerClientNamespace;

//----------------------------------------------------------------------

bool PlayerMoneyManagerClient::getPlayerMoney (int & cash, int & bank)
{
	const ClientObject * const player = Game::getClientPlayer ();

	if (player)
	{
		cash = player->getCashBalance ();
		bank = player->getBankBalance ();
		return true;
	}

	cash = 0;
	bank = 0;
	return false;
}

//======================================================================

