//======================================================================
//
// CuiSecureTradeManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSecureTradeManager.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedObject/CachedNetworkId.h"

//======================================================================

void CuiSecureTradeManager::initiateTrade     (const NetworkId & player, const NetworkId & item)
{
	if (ClientSecureTradeManager::initiateTrade (player, item, true))
	{
		const ClientObject * const obj = safe_cast<const ClientObject *>(CachedNetworkId (player).getObject ());
	
		if (obj)
		{
			Unicode::String msg;
			CuiStringVariablesManager::process (CuiStringIdsTrade::request_sent_prose, obj->getLocalizedName (), Unicode::String (), Unicode::String (), msg);
			CuiSystemMessageManager::sendFakeSystemMessage (msg);
		}
	}
}

//----------------------------------------------------------------------

void CuiSecureTradeManager::receiveRequest    (const ClientObject & obj)
{	
	Unicode::String msg;
	CuiStringVariablesManager::process (CuiStringIdsTrade::requested_prose, obj.getLocalizedName (), Unicode::String (), Unicode::String (), msg);
	CuiSystemMessageManager::sendFakeSystemMessage (msg);
}

//----------------------------------------------------------------------

void CuiSecureTradeManager::onCompleted    ()
{
	CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsTrade::complete.localize ());
}
	

//======================================================================
