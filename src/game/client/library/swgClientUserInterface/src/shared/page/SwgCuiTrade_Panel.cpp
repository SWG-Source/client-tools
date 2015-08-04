//======================================================================
//
// SwgCuiTrade_Panel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================


#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTrade_Panel.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UICursor.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiDragInfo.h"
#include "clientUserInterface/CuiMoney.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiContainerProviderTrade.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include <list>
#include <set>

//======================================================================

SwgCuiTrade::Panel::Panel                     (UIPage & page) :
CuiMediator           ("SwgCuiTrade::Panel", page),
UIEventCallback       (),
m_info                (0),
m_checkbox            (0),
m_text                (0),
m_textboxMoney        (0),
m_isSelf              (false),
m_ignoreCheck         (false),
m_ignoreMoney         (false),
m_callback            (new MessageDispatch::Callback),
m_containerProvider   (new SwgCuiContainerProviderTrade),
m_container           (0),
m_moneyOk             (false)
{
	{
		UIPage * page = 0;
		getCodeDataObject (TUIPage, page, "container");
		m_container = new SwgCuiInventoryContainer (*page);
		m_container->fetch ();
		m_container->setContainerProvider (m_containerProvider);
	}
	
	{
		UIPage * infoPage = 0;
		getCodeDataObject (TUIPage, infoPage, "info");
		m_info = new SwgCuiInventoryInfo (*infoPage);
		m_info->fetch ();
	}

	getCodeDataObject (TUICheckbox,   m_checkbox,     "checkbox");
	getCodeDataObject (TUIText,       m_text,         "text");
	getCodeDataObject (TUITextbox,    m_textboxMoney, "textboxMoney");

	registerMediatorObject (*NON_NULL(m_container->getVolumePage ()), true);
	registerMediatorObject (*m_checkbox,                              true);
	registerMediatorObject (*m_textboxMoney,                          true);

	m_info->connectToSelectionTransceiver (m_container->getTransceiverSelection ());

	m_checkbox->SetEnabled (false);
	m_textboxMoney->SetEnabled (false);

	m_colorTextboxMoney = m_textboxMoney->GetTextColor ();
}

//----------------------------------------------------------------------

SwgCuiTrade::Panel::~Panel               ()
{
	m_info->disconnectFromSelectionTransceiver (m_container->getTransceiverSelection ());

	m_container->release ();
	m_container = 0;

	delete m_containerProvider;
	m_containerProvider = 0;

	delete m_callback;
	m_callback = 0;

	m_info->release ();
	m_checkbox     = 0;
	m_text         = 0;
	m_textboxMoney = 0;
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::performActivate           ()
{
	m_moneyOk = true;

	checkMoneyValidity ();

	m_info->activate ();
	m_container->activate ();

	if (m_isSelf)
		m_callback->connect (*this, &SwgCuiTrade::Panel::onBalanceChanged, static_cast<ClientObject::Messages::CashBalance *>(0));

	reset (0, false);
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::performDeactivate         ()
{
	m_callback->disconnect (*this, &SwgCuiTrade::Panel::onBalanceChanged, static_cast<ClientObject::Messages::CashBalance *>(0));

	m_info->deactivate ();
	m_container->deactivate ();
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::OnTextboxChanged             (UIWidget * context)
{
	if (context == m_textboxMoney)
	{
		if (!m_ignoreMoney && m_isSelf)
		{			
			const int val = checkMoneyValidity ();
			ClientSecureTradeManager::giveMoney (val);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::OnVolumePageSelectionChanged (UIWidget * context)
{
	UNREF (context);
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::OnCheckboxSet                (UIWidget * context)
{
	if (context == m_checkbox)
	{
		updateAcceptedFromControl (m_checkbox->IsChecked ());
	}
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::OnCheckboxUnset              (UIWidget * context)
{
	if (context == m_checkbox)
	{
		updateAcceptedFromControl (m_checkbox->IsChecked ());
	}
}

//----------------------------------------------------------------------

bool SwgCuiTrade::Panel::OnMessage (UIWidget *context, const UIMessage & msg )
{
	if (context->GetParent () == m_container->getVolumePage ())
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			const CuiDragInfo cdinfo (*context);
			if (cdinfo.type == CuiDragInfoTypes::CDIT_object)
			{
				const ClientObject * const clientObject = cdinfo.getClientObject ();
				if (clientObject)
					CuiActionManager::performAction  (CuiActions::examine, Unicode::narrowToWide (clientObject->getNetworkId ().getValueString ()));
			}
		}
	}
	
	return true;
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::reset (int money, bool ok)
{
	m_containerProvider->setContentDirty (true);

	updateMoney (money);
	updateAccepted (ok);
}


//----------------------------------------------------------------------

void SwgCuiTrade::Panel::updateMoney         (int money)
{
	m_ignoreMoney = true;
	Unicode::String str;
	UIUtils::FormatInteger (str, money);
	m_textboxMoney->SetLocalText (str);
	m_ignoreMoney = false;
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::updateAccepted      (bool ok)
{
	m_ignoreCheck = true;
	m_checkbox->SetChecked (ok);
	m_ignoreCheck = false;
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::setIsSelf           (bool self)
{
	m_checkbox->SetEnabled     (self);
	m_textboxMoney->SetEnabled (self);
	
	if (m_isSelf != self)
	{
		m_isSelf = self;
	}
	
	m_containerProvider->setIsSelf (self);

	if (!self)
		m_checkbox->SetText (CuiStringIdsTrade::acceptable_other.localize ());
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::updateAcceptedFromControl (bool b)
{
	if (!m_ignoreCheck && m_isSelf)
	{
		if (b)
			ClientSecureTradeManager::acceptTrade ();
		else
			ClientSecureTradeManager::unacceptTrade ();
	}
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::setName             (const Unicode::String & name)
{
	m_text->SetText (name);
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::setAccepted         (bool b)
{
	m_checkbox->SetChecked (b);
}

//----------------------------------------------------------------------

int SwgCuiTrade::Panel::checkMoneyValidity ()
{
	int val = m_textboxMoney->GetNumericIntegerValue ();
	
	if (m_isSelf)
	{
		int cash = 0;
		int bank = 0;
		
		PlayerMoneyManagerClient::getPlayerMoney (cash, bank);
		
		const int MAX_MONEY_TRADE = cash;
		
		if (val > MAX_MONEY_TRADE)
		{
			m_checkbox->SetChecked (false);
			m_checkbox->SetEnabled (false);
			//		m_textboxMoney->SetTextColor (UIColor::red);
			//		m_textboxMoney->SetTextColorOverride (true);
			
			if (m_moneyOk)
			{
				CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsTrade::err_not_enough_money.localize ());
				m_textboxMoney->Ding ();
				m_moneyOk = false;
			}
		}
		else
		{
			m_checkbox->SetEnabled               (true);
			m_textboxMoney->SetTextColorOverride (false);
			//		m_textboxMoney->SetTextColor (m_colorTextboxMoney);
			m_moneyOk = true;
		}
	}
	
	return val;
}

//----------------------------------------------------------------------

void SwgCuiTrade::Panel::onBalanceChanged   (const ClientObject &)
{
	if (m_isSelf)
		checkMoneyValidity ();
}

//======================================================================
