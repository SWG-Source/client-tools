//======================================================================
//
// CuiShipSelfInfo.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiShipSelfInfo_H
#define INCLUDED_CuiShipSelfInfo_H

//======================================================================

#include "clientUserInterface/CuiShipInfo.h"

//----------------------------------------------------------------------

class CuiShipSelfInfo :
public CuiShipInfo
{
public:

	explicit CuiShipSelfInfo (UIPage& page);
	virtual void update (float timeElapsedSecs);

private:

	CuiShipSelfInfo ();
	CuiShipSelfInfo (const CuiShipSelfInfo& rhs);
	CuiShipSelfInfo& operator= (const CuiShipSelfInfo& rhs);
};

//======================================================================

#endif
