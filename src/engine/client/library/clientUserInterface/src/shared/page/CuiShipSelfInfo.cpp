//======================================================================
//
// CuiShipSelfInfo.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiShipSelfInfo.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipStation.h"
#include "sharedGame/ShipChassisSlotType.h"

//======================================================================

CuiShipSelfInfo::CuiShipSelfInfo(UIPage& page) :
CuiShipInfo("CuiShipSelfInfo", page)
{
}

//----------------------------------------------------------------------

void CuiShipSelfInfo::update(const float timeElapsedSecs)
{
	UNREF(timeElapsedSecs);

	CreatureObject const * const player = Game::getPlayerCreature();

	if (player == NULL)
	{
		return;
	}
	
	ShipObject const * shipObject = NULL;
	if (player->getShipStation() != ShipStation::ShipStation_None)
	{
		shipObject = ShipObject::getContainingShip(*player);
	}

	setTitle("Current Ship Info", Unicode::wideToNarrow(player->getObjectName()).c_str());

	setCommonShipInfo( shipObject );
}

//======================================================================
