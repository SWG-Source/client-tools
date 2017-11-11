//======================================================================
//
// CuiSecureTradeManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSecureTradeManager_H
#define INCLUDED_CuiSecureTradeManager_H

//======================================================================

class NetworkId;
class ClientObject;

//----------------------------------------------------------------------

class CuiSecureTradeManager
{
public:

	static void initiateTrade     (const NetworkId & player, const NetworkId & item);

	static void receiveRequest    (const ClientObject & obj);

	static void onCompleted       ();
};

//======================================================================

#endif
