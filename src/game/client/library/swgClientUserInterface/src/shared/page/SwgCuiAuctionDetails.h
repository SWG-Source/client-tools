//======================================================================
//
// SwgCuiAuctionDetails.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionDetails_H
#define INCLUDED_SwgCuiAuctionDetails_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class CuiWidget3dObjectListViewer;
class UIButton;
class UIComposite;
class UIText;
class UITextbox;
class AuctionManagerClientData;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiAuctionDetails :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiAuctionDetails (UIPage & page);
	typedef AuctionManagerClientData Data;

	// buy is enabled by default
	void setBuyButtonEnabled(bool const enabled);

protected:

	~SwgCuiAuctionDetails ();

	void                         performActivate        ();
	void                         performDeactivate      ();

	void                         OnButtonPressed        (UIWidget * context);
	void                         onDetailsReceived      (const Data & data);
	void                         onAuctionToViewChanged (const NetworkId & auctionId);

	void                         update                 (float deltaTimeSecs);
	void                         updateTimeRemaining    (int secsRemaining);

private:

	SwgCuiAuctionDetails ();
	SwgCuiAuctionDetails (const SwgCuiAuctionDetails & rhs);
	SwgCuiAuctionDetails operator= (const SwgCuiAuctionDetails & rhs);

	void                          populate        (const Data & data);
	void                          populateDetails (const Data & data);

	UIButton *                    m_buttonExit;
	UIButton *                    m_buttonBid;
	UIButton *                    m_buttonRetrieve;
	UIButton *                    m_buttonSell;
	UIButton *                    m_buttonWithdraw;
	UIButton *                    m_buttonAccept;
	UIButton *                    m_buttonNext;
	UIButton *                    m_buttonPrev;

	UIComposite *                 m_compositeText;
	UIComposite *                 m_compositeDesc;

	UIText *                      m_labelPrice;
	UIText *                      m_textPrice;
	UIText *                      m_textItemName;
	UIText *                      m_textLocation;
	UIText *                      m_textSeller;
	UIText *                      m_textTime;
	UIText *                      m_textType;
	UIText *                      m_textDesc;
	UIText *                      m_textAttribs;

	CuiWidget3dObjectListViewer * m_viewer;

	MessageDispatch::Callback *   m_callback;

	NetworkId                     m_itemId;

	int                           m_lastSecsRemaining;

	float                         m_elapsedTime;

	bool m_buyButtonEnabled;
};

//======================================================================

#endif
