//======================================================================
//
// SwgCuiTrade_Panel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiTrade_Panel_H
#define INCLUDED_SwgCuiTrade_Panel_H

//======================================================================

#include "swgClientUserInterface/SwgCuiTrade.h"

class CachedNetworkId;
class ClientObject;
class CuiWidget3dObjectListViewer;
class NetworkId;
class SwgCuiContainerProviderTrade;
class SwgCuiInventoryContainer;
class SwgCuiInventoryInfo;
class UIButton;
class UICheckbox;
class UICursor;
class UIPage;
class UIText;
class UITextbox;
class UIVolumePage;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiTrade::Panel :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                Panel                     (UIPage & page);

	void                    performActivate           ();
	void                    performDeactivate         ();

	void                    OnTextboxChanged             (UIWidget * context);
	void                    OnVolumePageSelectionChanged (UIWidget * context);
	void                    OnCheckboxSet                (UIWidget * context);
	void                    OnCheckboxUnset              (UIWidget * context);
	bool                    OnMessage           (UIWidget *context, const UIMessage & msg);

	void                    reset               (int money, bool ok);
	void                    updateMoney         (int money);
	void                    updateAccepted      (bool ok);

	void                    setIsSelf           (bool self);

	void                    setName             (const Unicode::String & name);

	bool                    isAddOk             (const ClientObject & obj);

	void                    update              (float deltaTimeSecs);

	void                    setAccepted         (bool b);

	void                    onBalanceChanged    (const ClientObject & obj);
	bool                    isMoneyOk           () const;
	int                     checkMoneyValidity  ();

private:
	                       ~Panel               ();
	                        Panel               (const Panel &);
	Panel &                 operator=           (const Panel &);

	void                    updateSelectedItem  ();
	void                    updateAcceptedFromControl (bool b);

	SwgCuiInventoryInfo *   m_info;
	UICheckbox *            m_checkbox;
	UIText *                m_text;
	UITextbox *             m_textboxMoney;

	bool                    m_isSelf;

	bool                    m_ignoreCheck;
	bool                    m_ignoreMoney;

	MessageDispatch::Callback * m_callback;

	SwgCuiContainerProviderTrade * m_containerProvider;
	SwgCuiInventoryContainer     * m_container;

	UIColor                        m_colorTextboxMoney;

	bool                           m_moneyOk;
};

//----------------------------------------------------------------------

inline bool SwgCuiTrade::Panel::isMoneyOk () const
{
	return m_moneyOk;
}

//======================================================================

#endif
