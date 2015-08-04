//======================================================================
//
// SwgCuiAuctionList.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionList_H
#define INCLUDED_SwgCuiAuctionList_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAuctionListBase.h"

//----------------------------------------------------------------------

class SwgCuiAuctionList :
public SwgCuiAuctionListBase
{
public:
	SwgCuiAuctionList (UIPage & page);
protected:

	virtual void performActivate   ();
	virtual void performDeactivate ();

private:

	SwgCuiAuctionList (const SwgCuiAuctionList& rhs);
	SwgCuiAuctionList & operator= (const SwgCuiAuctionList & rhs);
	SwgCuiAuctionList ();

	NetworkId             m_marketObjectId;
};

//======================================================================

#endif
