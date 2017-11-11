//======================================================================
//
// CuiShipTargetInfo.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiShipTargetInfo_H
#define INCLUDED_CuiShipTargetInfo_H

//======================================================================

#include "clientUserInterface/CuiShipInfo.h"

//----------------------------------------------------------------------

class CuiShipTargetInfo :
public CuiShipInfo
{
public:

	explicit CuiShipTargetInfo (UIPage& page);
	virtual void update (float timeElapsedSecs);


private:
	//disabled
	CuiShipTargetInfo ();
	CuiShipTargetInfo (const CuiShipTargetInfo& rhs);
	CuiShipTargetInfo& operator= (const CuiShipTargetInfo& rhs);

private:
	UIText * m_textTargetedComponent;
	static char ms_textBuffer[ CuiShipInfo::TextBufferLength ];
};

//======================================================================

#endif
