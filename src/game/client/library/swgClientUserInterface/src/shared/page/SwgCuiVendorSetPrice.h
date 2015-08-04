//======================================================================
//
// SwgCuiVendorSetPrice.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiVendorSetPrice_H
#define INCLUDED_SwgCuiVendorSetPrice_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class NetworkId;
class UIButton;
class UIText;
class UITextbox;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiVendorSetPrice :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiVendorSetPrice (UIPage & page);

protected:

	typedef std::pair<int, Unicode::String> ResultInfo;

	~SwgCuiVendorSetPrice ();

	void                         performActivate   ();
	void                         performDeactivate ();

	void                         OnButtonPressed              (UIWidget *context);
	void                         onAuctionToSetPriceOnChanged (const NetworkId & auctionId);

	void                         onAuctionSetPriceResponse    (const std::pair<NetworkId, ResultInfo> & result);

private:

	SwgCuiVendorSetPrice ();
	SwgCuiVendorSetPrice (const SwgCuiVendorSetPrice & rhs);
	SwgCuiVendorSetPrice operator= (const SwgCuiVendorSetPrice & rhs);

	void                        ok () const;

	UIText *                    m_textItemName;
	UITextbox *                 m_textboxPrice;

	UIButton *                  m_buttonCancel;
	UIButton *                  m_buttonOk;

	MessageDispatch::Callback * m_callback;
};

//======================================================================

#endif
