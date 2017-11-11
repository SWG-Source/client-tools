//======================================================================
//
// AuctionManagerClientData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AuctionManagerClientData_H
#define INCLUDED_AuctionManagerClientData_H

#include "sharedNetworkMessages/AuctionData.h"

//======================================================================

class AuctionManagerClientData
{
public:

	typedef Auction::ItemDataHeader  Header;
	typedef Auction::ItemDataDetails Details;

	AuctionManagerClientData ();

	bool  hasDetails;
	int   receiptTime;

	Header     header;
	Details    details;
	NetworkId  locationId;

	static Header                   constructSimulationHeader ();

	void                            simulateDetails        (const NetworkId & id);

	void                            constructLocalizedName (Unicode::String & str) const;

	void                            setDetails             (const Details & _details);

	void                            setHeader              (const Header & _header);

	int                             decodeAuctionItemNameString(stdvector<Unicode::String>::fwd & undecodedComponents, stdvector<Unicode::String>::fwd & decodedComponents) const;
};

//======================================================================

#endif
