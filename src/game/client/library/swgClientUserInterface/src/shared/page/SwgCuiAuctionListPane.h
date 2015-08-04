//======================================================================
//
// SwgCuiAuctionListPane.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionListPane_H
#define INCLUDED_SwgCuiAuctionListPane_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class AuctionManagerClientData;
class NetworkId;
class SwgCuiAuctionFilter;
class SwgCuiAuctionListBase;
class SwgCuiAuctionListView;
class UIButton;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiAuctionListPane :
public CuiMediator,
public UIEventCallback
{
public:

	typedef SwgCuiAuctionListView      View;
	typedef AuctionManagerClientData   Data;

	SwgCuiAuctionListPane              (const char * const debugName, UIPage & page, int type, SwgCuiAuctionListBase & listRoot, bool const disableBuyInDetails);

	void                   onTraverseAuctionToView   (const bool & forward);

	void                   onViewDoubleClick         (const View & view);
	void                   onViewSelectionChanged    (const View & view);
	void                   onViewContextRequest      (const std::pair<const View *, std::pair<NetworkId, UIPoint> > & payload);

	void                   requestUpdate             (bool optional, int incrementIndex);

	typedef stdvector<NetworkId>::fwd NetworkIdVector;

	const NetworkId &       findSelectedAuction       (bool warn) const;
	const NetworkIdVector & findSelectedAuctions      (bool warn) const;

	void                   showDetails               (const NetworkId & id) const;

	void                   OnButtonPressed           (UIWidget * context);
	void                   OnPopupMenuSelection      (UIWidget * context);
	
	void                   startSale                 () const;
	void                   retrieve                  (const NetworkId & id) const;
	void                   retrieve                  (const NetworkIdVector & ids) const;
	void                   withdraw                  (const NetworkId & id) const;
	void                   withdraw                  (const NetworkIdVector & ids) const;
	void                   acceptBid                 (const NetworkId & id);
	void                   enterBid                  () const;
	void                   enterBid                  (const NetworkId & auctionId) const;
	void                   setPrice                  (const NetworkId & id) const;
	void                   reject                    (const NetworkId & id) const;
	void                   createWaypoint            (const NetworkId & id, bool includePriceInWaypointName) const;
	void                   sellFromStock             (const NetworkIdVector & ids) const;
	void                   relistFromStock           (const NetworkIdVector & ids) const;

	void                   update                    (float deltaTimeSecs);

	void                   updateButtonStates        ();
	void                   updateTextListStatus      ();

	void                   saveSettings              () const;
	void                   loadSettings              ();

	void                   onListViewUpdated         ();

	bool                   isSelectionWithdrawable   () const;
	bool                   isSelectionAcceptable     () const;
	bool                   isSelectionRetrievable    () const;

protected:

	virtual ~SwgCuiAuctionListPane ();

	virtual void                performActivate   ();
	virtual void                performDeactivate ();

	SwgCuiAuctionListBase *     m_listRoot;
	MessageDispatch::Callback * m_callback;
	SwgCuiAuctionFilter *       m_filter;
	int                         m_type;
	int                         m_auctionListType;
	View *                      m_view;

	UIButton *                  m_buttonBid;
	UIButton *                  m_buttonDetails;
	UIButton *                  m_buttonWithdraw;
	UIButton *                  m_buttonAccept;
	UIButton *                  m_buttonBidAll;
	UIButton *                  m_buttonBidMyBids;
	UIButton *                  m_buttonRetrieve;
	UIButton *                  m_buttonSell;
	UIButton *                  m_buttonSellFromStock;
	UIButton *                  m_buttonRelistFromStock;
	UIButton *                  m_buttonOffer;
	UIButton *                  m_buttonChangePrice;
	UIButton *                  m_buttonReject;
	UIButton *                  m_buttonCreateWaypoint;

	UIText *                    m_textListStatus;

	UIText *                    m_textPaging;
	UIButton *                  m_buttonPagePrev;
	UIButton *                  m_buttonPageNext;

	bool m_disableBuyInDetails;
private:

	SwgCuiAuctionListPane ();
	SwgCuiAuctionListPane (const SwgCuiAuctionListPane & rhs);
	SwgCuiAuctionListPane & operator= (const SwgCuiAuctionListPane & rhs);
};

//======================================================================

#endif
