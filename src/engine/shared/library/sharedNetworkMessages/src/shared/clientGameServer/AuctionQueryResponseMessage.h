// ======================================================================
//
// AuctionQueryResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryResponseMessage_H
#define	_AuctionQueryResponseMessage_H

//-----------------------------------------------------------------------

#include "AuctionDataArchive.h"
#include "AuctionQueryTypes.h"
#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class AuctionQueryResponseMessage : public GameNetworkMessage
{
public:
	typedef AuctionQueryResponse_AuctionData AuctionData;
	typedef stdvector<AuctionData>::fwd AuctionDataVector;

	AuctionQueryResponseMessage(int requestId, int typeFlag, AuctionDataVector const &auctionData);
	explicit AuctionQueryResponseMessage(Archive::ReadIterator &source);

	~AuctionQueryResponseMessage();

public: // methods

	int                       getRequestId() const;
	int                       getTypeFlag() const;
	AuctionDataVector const & getAuctionData() const;

public: // types

private: 
	Archive::AutoVariable<int>       m_requestId;
	Archive::AutoVariable<int>       m_typeFlag;
	Archive::AutoArray<AuctionData>  m_auctionData;
};

// ----------------------------------------------------------------------

inline int AuctionQueryResponseMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryResponseMessage::getTypeFlag() const
{
	return m_typeFlag.get();
}

// ----------------------------------------------------------------------

inline AuctionQueryResponseMessage::AuctionDataVector const & AuctionQueryResponseMessage::getAuctionData() const
{
	return m_auctionData.get();
}

// ----------------------------------------------------------------------

#endif // _AuctionQueryResponseMessage_H

