//======================================================================
//
// SwgCuiVendor.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiVendor_H
#define INCLUDED_SwgCuiVendor_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAuctionListBase.h"

//----------------------------------------------------------------------

class SwgCuiVendor :
public SwgCuiAuctionListBase
{
public:
	SwgCuiVendor (UIPage & page);
protected:

	virtual void performActivate ();
	virtual void performDeactivate ();

private:

	SwgCuiVendor (const SwgCuiVendor& rhs);
	SwgCuiVendor & operator= (const SwgCuiVendor & rhs);
	SwgCuiVendor ();

	bool          m_hasActivated;
	bool          m_isOwner;
	NetworkId     m_vendorId;
};

//======================================================================

#endif
