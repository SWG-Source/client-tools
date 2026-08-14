// ======================================================================
//
// GetAuctionDetailsResponse.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetAuctionDetailsResponse_H
#define	_GetAuctionDetailsResponse_H

//-----------------------------------------------------------------------

#include "AuctionDataArchive.h"
#include "sharedNetworkMessages/AuctionData.h"
#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GetAuctionDetailsResponse : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	GetAuctionDetailsResponse(const Auction::ItemDataDetails & details);
	explicit GetAuctionDetailsResponse(Archive::ReadIterator &source);

	~GetAuctionDetailsResponse();

public: // methods

	Auction::ItemDataDetails    getDetails() const;
public: // types

private: 
	Archive::AutoVariable<Auction::ItemDataDetails>  m_details;
};

// ----------------------------------------------------------------------

inline Auction::ItemDataDetails GetAuctionDetailsResponse::getDetails() const
{
	return m_details.get();
}

// ----------------------------------------------------------------------

#endif // _GetAuctionDetailsResponse_H

