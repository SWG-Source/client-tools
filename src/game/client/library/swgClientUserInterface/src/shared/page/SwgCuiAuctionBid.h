//======================================================================
//
// SwgCuiAuctionBid.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionBid_H
#define INCLUDED_SwgCuiAuctionBid_H

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

class SwgCuiAuctionBid :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiAuctionBid (UIPage & page);

	virtual bool close();

protected:

	typedef std::pair<int, Unicode::String> ResultInfo;

	~SwgCuiAuctionBid ();

	virtual void update(float deltaTimeSecs);

	void                         performActivate   ();
	void                         performDeactivate ();

	void                         OnTextboxChanged        (UIWidget *context);
	void                         OnButtonPressed         (UIWidget *context);
	void                         onAuctionToBidOnChanged (const NetworkId & auctionId);

	void                         onMessageBoxClosed      (const CuiMessageBox & box);
	void                         onAuctionBidResponse    (const std::pair<NetworkId, ResultInfo> & result);

private:

	SwgCuiAuctionBid ();
	SwgCuiAuctionBid (const SwgCuiAuctionBid & rhs);
	SwgCuiAuctionBid operator= (const SwgCuiAuctionBid & rhs);

	void                         ok ();

	UIText *                 m_textItemName;
	UIText *                 m_textHighBid;
	UITextbox *              m_textboxBid;
	UITextbox *              m_textboxProxyBid;
	UIButton *               m_buttonCancel;
	UIButton *               m_buttonOk;

	bool                     m_ignoreTextbox;

	int                      m_highBid;

	MessageDispatch::Callback * m_callback;

	CuiMessageBox *             m_messageBox;
};

//======================================================================

#endif
