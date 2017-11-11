//======================================================================
//
// SwgCuiAuctionSell.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionSell_H
#define INCLUDED_SwgCuiAuctionSell_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

class ClientObject;
class CuiMessageBox;
class SwgCuiContainerProviderDefault;
class SwgCuiInventoryContainer;
class SwgCuiInventoryInfo;
class UIButton;
class UICheckbox;
class UIText;
class UITextbox;
class CuiWidget3dObjectListViewer;
class AuctionManagerClientData;
class SwgCuiInventoryContainerFilter;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiAuctionSell :
public CuiMediator,
public UIEventCallback
{
public:
	SwgCuiAuctionSell (UIPage & page);
	typedef AuctionManagerClientData Data;

	void                         onObjectIconDoubleClick (ClientObject & obj, bool isInventory);

	void                         OnTabbedPaneChanged (UIWidget * context);

protected:

	~SwgCuiAuctionSell ();

	void                         performActivate   ();
	void                         performDeactivate ();

	void                         OnButtonPressed         (UIWidget * context);
	void                         OnTextboxChanged        (UIWidget * context);

	void                         onCreateAuctionResponse (const std::pair<NetworkId, std::pair<int, Unicode::String> > & payload);

	void                         onSelectionChanged      (const std::pair<int, ClientObject *> & msg);

	void                         onDetailsReceived       (const Data & data);

private:

	SwgCuiAuctionSell ();
	SwgCuiAuctionSell (const SwgCuiAuctionSell & rhs);
	SwgCuiAuctionSell operator= (const SwgCuiAuctionSell & rhs);

	void                           ok                ();
	void                           handleVendorOk    ();
	void                           handleAuctionOk   ();
	void                           populateDetails   (const Data & data);


	SwgCuiInventoryContainer *     m_containerInventory;
	SwgCuiInventoryContainer *     m_containerDatapad;
	SwgCuiInventoryInfo *          m_info;

	UIText *                       m_textDesc;
	UICheckbox *                   m_checkInstantSale;
	UICheckbox *                   m_checkAuction;
	UITextbox *                    m_textboxInstantSale;
	UITextbox *                    m_textboxMinBid;
	UITextbox *                    m_textboxDays;
	UITextbox *                    m_textboxHours;
	UITextbox *                    m_textboxMins;
	UITextbox *                    m_textboxVendorPrice;
	UIButton *                     m_buttonOk;
	UIButton *                     m_buttonCancel;

	UIPage *                       m_pageRight;

	UIText *                       m_textDescStock;
	UIText *                       m_textObjectNameStock;
	CuiWidget3dObjectListViewer *  m_viewerStock;

	UICheckbox *                   m_checkPremium;

	MessageDispatch::Callback *    m_callback;

	SwgCuiContainerProviderDefault * m_containerProviderInventory;
	SwgCuiContainerProviderDefault * m_containerProviderDatapad;

	SwgCuiInventoryContainerFilter * m_containerFilterInventory;
	SwgCuiInventoryContainerFilter * m_containerFilterDatapad;

	UITabbedPane *                   m_tabs;

	stdvector<NetworkId>::fwd        m_itemsToSellFromStock;
};

//======================================================================

#endif
