// ======================================================================
//
// GetAuctionDetailsResponse.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "AuctionDataArchive.h"
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetAuctionDetailsResponse.h"

// ======================================================================

const char * const GetAuctionDetailsResponse::MessageType = "GetAuctionDetailsResponse";

//----------------------------------------------------------------------

GetAuctionDetailsResponse::GetAuctionDetailsResponse(const Auction::ItemDataDetails & details) :
	GameNetworkMessage(MessageType),
	m_details(details)
{
	AutoByteStream::addVariable(m_details);
}

// ----------------------------------------------------------------------

GetAuctionDetailsResponse::GetAuctionDetailsResponse(Archive::ReadIterator & source) :
	GameNetworkMessage(MessageType),
	m_details()
{
	AutoByteStream::addVariable(m_details);
	unpack(source);
}

// ----------------------------------------------------------------------

GetAuctionDetailsResponse::~GetAuctionDetailsResponse()
{
}

// ======================================================================

