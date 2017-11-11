//======================================================================
//
// SwgCuiTrade.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTrade_Panel.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientSecureTradeManager.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSecureTradeManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"

//======================================================================

namespace
{
	bool  s_rangeChecked = false;
	float s_containerRange = 0.0f;

	void checkRange ()
	{
		if (s_rangeChecked)
			return;

		s_rangeChecked = true;

		if (RadialMenuManager::getRangeForMenuType (Cui::MenuInfoTypes::TRADE_START, s_containerRange))
			s_containerRange *= 1.5f;
		else
			WARNING (true, ("SwgCuiInventory RadialMenuManager::getRangeForMenuType failed"));
	}
}

//----------------------------------------------------------------------

SwgCuiTrade::SwgCuiTrade               (UIPage & page) :
CuiMediator               ("SwgCuiTrade", page),
UIEventCallback           (),
m_panelSelf               (0),
m_panelOther              (0),
m_buttonCancel            (0),
m_buttonOk                (0),
m_callback                (new MessageDispatch::Callback),
m_timeSinceLastRangeCheck (0.0f),
m_textWaiting             (0)
{
	checkRange ();

	{
		UIPage * selfPage = 0;
		getCodeDataObject (TUIPage, selfPage, "self");
		
		m_panelSelf = new Panel (*selfPage);
		m_panelSelf->fetch ();
		m_panelSelf->setIsSelf (true);
	}
	
	{	
		UIPage * otherPage = 0;
		getCodeDataObject (TUIPage, otherPage, "other", true);

		if (otherPage)
		{
			m_panelOther = new Panel (*otherPage);
			m_panelOther->fetch ();
			m_panelOther->setIsSelf (false);
		}
	}
		
	getCodeDataObject (TUIButton,    m_buttonCancel, "buttonCancel");
	getCodeDataObject (TUIButton,    m_buttonOk,     "buttonOk");
	getCodeDataObject (TUIText,      m_textWaiting,  "textWaiting");

	m_textWaiting->SetPreLocalized (true);
	m_textWaiting->Clear           ();
	m_textWaiting->SetVisible      (false);
	
	setState (MS_closeable);

	setSettingsAutoSizeLocation (true, true);
}

//----------------------------------------------------------------------

SwgCuiTrade::~SwgCuiTrade               ()
{
	delete m_callback;
	m_callback = 0;

	m_panelSelf->release ();
	m_panelSelf = 0;

	if (m_panelOther)
	{
		m_panelOther->release ();
		m_panelOther = 0;
	}

	m_buttonCancel  = 0;
	m_buttonOk      = 0;
}

//----------------------------------------------------------------------

void            SwgCuiTrade::performActivate           ()
{
	CuiManager::requestPointer (true);

	m_callback->connect (*this, &SwgCuiTrade::onAbort,        static_cast<ClientSecureTradeManager::Messages::AbortTrade *>(0));
	m_callback->connect (*this, &SwgCuiTrade::onBeginTrade,   static_cast<ClientSecureTradeManager::Messages::BeginTrade *>(0));
	m_callback->connect (*this, &SwgCuiTrade::onGiveMoney,    static_cast<ClientSecureTradeManager::Messages::GiveMoney *>(0));
	m_callback->connect (*this, &SwgCuiTrade::onAccept,       static_cast<ClientSecureTradeManager::Messages::Accept *>(0));
	m_callback->connect (*this, &SwgCuiTrade::onRequestVerify,static_cast<ClientSecureTradeManager::Messages::RequestVerify *>(0));
	m_callback->connect (*this, &SwgCuiTrade::onComplete,     static_cast<ClientSecureTradeManager::Messages::Complete *>(0));

	m_buttonCancel->AddCallback (this);
	m_buttonOk->AddCallback (this);

	m_panelSelf->activate ();

	if (m_panelOther)
		m_panelOther->activate ();

	reset ();

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void            SwgCuiTrade::performDeactivate         ()
{
	setIsUpdating (false);

	CuiManager::requestPointer (false);

	m_callback->disconnect (*this, &SwgCuiTrade::onAbort,        static_cast<ClientSecureTradeManager::Messages::AbortTrade *>(0));
	m_callback->disconnect (*this, &SwgCuiTrade::onBeginTrade,   static_cast<ClientSecureTradeManager::Messages::BeginTrade *>(0));
	m_callback->disconnect (*this, &SwgCuiTrade::onGiveMoney,    static_cast<ClientSecureTradeManager::Messages::GiveMoney *>(0));
	m_callback->disconnect (*this, &SwgCuiTrade::onAccept,       static_cast<ClientSecureTradeManager::Messages::Accept *>(0));
	m_callback->disconnect (*this, &SwgCuiTrade::onRequestVerify,static_cast<ClientSecureTradeManager::Messages::RequestVerify *>(0));
	m_callback->disconnect (*this, &SwgCuiTrade::onComplete,     static_cast<ClientSecureTradeManager::Messages::Complete *>(0));

	m_buttonCancel->RemoveCallback (this);
	m_buttonOk->RemoveCallback (this);

	m_panelSelf->deactivate ();
	
	if (m_panelOther)
		m_panelOther->deactivate ();
}

//----------------------------------------------------------------------

void  SwgCuiTrade::OnButtonPressed           (UIWidget * context)
{
	if (context == m_buttonCancel)
	{
		close ();
	}
	else if (context == m_buttonOk)
	{
		if (!m_panelSelf->isMoneyOk ())
		{
			const int money = m_panelSelf->checkMoneyValidity ();
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIdsTrade::err_not_enough_money, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, money, 0.0f, result);
			CuiMessageBox::createInfoBox (CuiStringIdsTrade::err_not_enough_money.localize ());
			return;
		}

		ClientSecureTradeManager::verifyTrade ();

		const ClientObject * const other = safe_cast<const ClientObject *>(ClientSecureTradeManager::getOther ().getObject ());
		const Unicode::String & otherName = other ? other->getLocalizedName () : Unicode::String ();

		Unicode::String str;

		CuiStringVariablesManager::process (CuiStringIdsTrade::waiting_complete_prose, otherName, Unicode::String (), Unicode::String (), str);

		m_textWaiting->SetVisible (true);
		m_textWaiting->SetLocalText (str);
	}
}

//----------------------------------------------------------------------

SwgCuiTrade *    SwgCuiTrade::createInto                (UIPage & parent)
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (parent, "/pda.trade"));
	dupe->Center ();
	SwgCuiTrade * const mediator = new SwgCuiTrade (*dupe);
	return mediator;
}

//----------------------------------------------------------------------

void SwgCuiTrade::reset ()
{
	m_textWaiting->SetVisible (false);

	int moneySelf  = 0;
	int moneyOther = 0;

	bool okSelf  = false;
	bool okOther = false;

	ClientSecureTradeManager::getMoney    (moneySelf, moneyOther);
	ClientSecureTradeManager::getAccepted (okSelf, okOther);

	m_panelSelf->reset (moneySelf, okSelf);

	m_panelOther->reset (moneyOther, okOther);

	const ClientObject * const player = Game::getClientPlayer ();
	m_panelSelf->setName (player ? player->getLocalizedName () : Unicode::String ());

	if (m_panelOther)
	{
		const ClientObject * const other = dynamic_cast<const ClientObject *>(ClientSecureTradeManager::getOther ().getObject ());
		m_panelOther->setName (other ? other->getLocalizedName () : Unicode::String ());		
	}

	m_buttonOk->SetEnabled (okSelf && okOther);
}

//----------------------------------------------------------------------

void SwgCuiTrade::onBeginTrade               (const bool &      )
{
	reset ();
}

//----------------------------------------------------------------------

void SwgCuiTrade::onGiveMoney                (const int &       )
{
	int moneySelf  = 0;
	int moneyOther = 0;

	ClientSecureTradeManager::getMoney    (moneySelf, moneyOther);

//	m_panelSelf->updateMoney (moneySelf);

	if (m_panelOther)
		m_panelOther->updateMoney (moneyOther);
}

//----------------------------------------------------------------------

void SwgCuiTrade::onAccept                   (const bool &      )
{
	bool okSelf  = false;
	bool okOther = false;

	ClientSecureTradeManager::getAccepted (okSelf, okOther);

	m_panelSelf->updateAccepted (okSelf);

	if (m_panelOther)
		m_panelOther->updateAccepted (okOther);

	m_buttonOk->SetEnabled (okSelf && okOther);

	if (m_textWaiting->IsVisible ())
		m_textWaiting->SetVisible (okOther && okSelf);
}

//----------------------------------------------------------------------

void SwgCuiTrade::onRequestVerify                   (const bool &      )
{

}

//----------------------------------------------------------------------

void SwgCuiTrade::onComplete                 (const bool &     )
{
	deactivate ();
	CuiSecureTradeManager::onCompleted ();
}

//----------------------------------------------------------------------

void SwgCuiTrade::onAbort                 (const bool &     )
{
	deactivate ();
	CuiMessageBox::createInfoBox (CuiStringIdsTrade::aborted.localize ());
}

//----------------------------------------------------------------------

bool SwgCuiTrade::close    ()
{
	ClientSecureTradeManager::abortTrade ();
	deactivate ();
	return false;
}

//----------------------------------------------------------------------

void SwgCuiTrade::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_timeSinceLastRangeCheck += deltaTimeSecs;

	if (m_timeSinceLastRangeCheck > 2.4f)
	{
		m_timeSinceLastRangeCheck = 0.0f;

		const ClientObject * const other = safe_cast<ClientObject *>(ClientSecureTradeManager::getOther ().getObject ());
		
		if (!other)
		{
			Unicode::String result;
			CuiStringVariablesManager::process (CuiStringIdsTrade::target_lost, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, result);
			CuiSystemMessageManager::sendFakeSystemMessage (result);
			closeThroughWorkspace ();
		}
		else
		{
			const Object * const player = Game::getPlayer ();
			const Vector & objPos_w     = other->findPosition_w ();
			const Vector & playerPos_w  = player->findPosition_w ();
			
			if ((objPos_w.magnitudeBetween (playerPos_w) - other->getAppearanceSphereRadius ()) > s_containerRange)
			{
				const Unicode::String & localizedName = other->getLocalizedName ();
				Unicode::String result;
				CuiStringVariablesManager::process (CuiStringIdsTrade::out_of_range_prose, Unicode::emptyString, localizedName, Unicode::emptyString, result);
				CuiSystemMessageManager::sendFakeSystemMessage (result);
				
				closeThroughWorkspace ();
			}
		}
	}
}

//======================================================================
