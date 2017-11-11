//======================================================================
//
// SwgCuiAuctionBuy.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionBuy_H
#define INCLUDED_SwgCuiAuctionBuy_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

class UIButton;
class UIText;
class UITextbox;
class CuiMessageBox;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiAuctionBuy :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiAuctionBuy (UIPage & page);

	virtual bool close();

protected:

	typedef std::pair<int, Unicode::String> ResultInfo;

	~SwgCuiAuctionBuy ();

	virtual void update(float deltaTimeSecs);

	void                         performActivate   ();
	void                         performDeactivate ();

	void                         OnButtonPressed         (UIWidget *context);
	void                         onAuctionToBidOnChanged (const NetworkId & auctionId);

	void                         onMessageBoxClosed      (const CuiMessageBox & box);
	void                         onAuctionBidResponse    (const std::pair<NetworkId, ResultInfo> & result);

private:

	SwgCuiAuctionBuy ();
	SwgCuiAuctionBuy (const SwgCuiAuctionBuy & rhs);
	SwgCuiAuctionBuy operator= (const SwgCuiAuctionBuy & rhs);

	void                        ok ();

	UIText *                    m_textItemName;
	UIText *                    m_textInstant;

	UIButton *                  m_buttonCancel;
	UIButton *                  m_buttonOk;

	int                         m_highBid;

	MessageDispatch::Callback * m_callback;

	CuiMessageBox *             m_messageBox;
};

//======================================================================

#endif
