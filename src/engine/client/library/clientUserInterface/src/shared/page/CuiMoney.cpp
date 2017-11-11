//======================================================================
//
// CuiMoney.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMoney.h"

#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIData.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================

CuiMoney::CuiMoney (UIPage & page) :
CuiMediator ("CuiMoney", page),
m_textCash  (0),
m_textBank  (0),
m_id        (),
m_callback  (new MessageDispatch::Callback)
{
	getCodeDataObject (TUIText, m_textCash, "textCash");
	getCodeDataObject (TUIText, m_textBank, "textBank");

	m_textCash->SetPreLocalized (true);
	m_textBank->SetPreLocalized (true);
}

//----------------------------------------------------------------------

CuiMoney::~CuiMoney ()
{
	delete m_callback;
	m_callback = 0;
	m_id = NetworkId::cms_invalid;
	m_textCash = 0;
	m_textBank = 0;
}

//----------------------------------------------------------------------

void CuiMoney::performActivate    ()
{
	m_callback->connect (*this, &CuiMoney::onBalanceChanged, static_cast<ClientObject::Messages::BankBalance *>(0));
	m_callback->connect (*this, &CuiMoney::onBalanceChanged, static_cast<ClientObject::Messages::CashBalance *>(0));
	m_callback->connect (*this, &CuiMoney::onGalacticReserveDepositChanged, static_cast<PlayerObject::Messages::GalacticReserveDepositChanged *>(0));

	updateValues ();
}

//----------------------------------------------------------------------

void CuiMoney::performDeactivate  ()
{
	m_callback->disconnect (*this, &CuiMoney::onBalanceChanged, static_cast<ClientObject::Messages::BankBalance *>(0));
	m_callback->disconnect (*this, &CuiMoney::onBalanceChanged, static_cast<ClientObject::Messages::CashBalance *>(0));
	m_callback->disconnect (*this, &CuiMoney::onGalacticReserveDepositChanged, static_cast<PlayerObject::Messages::GalacticReserveDepositChanged *>(0));
}

//----------------------------------------------------------------------

void CuiMoney::onBalanceChanged   (const ClientObject & obj)
{
	if (obj.getNetworkId () == m_id)
		updateValues ();
}

//----------------------------------------------------------------------

void CuiMoney::onGalacticReserveDepositChanged(const PlayerObject & obj)
{
	PlayerObject const * const po = Game::getPlayerObject();
	if (po && (po->getNetworkId() == obj.getNetworkId()) && m_id.isValid() && (Game::getPlayerNetworkId() == m_id))
		updateValues ();
}

//----------------------------------------------------------------------

void CuiMoney::updateValues ()
{
	const ClientObject * const obj = safe_cast<const ClientObject *>(NetworkIdManager::getObjectById (m_id));
	if (obj)
	{
		Unicode::String s;
		UIUtils::FormatInteger (s, obj->getBankBalance ());
		if(obj->getBankBalance() > 100)
			s = UIUtils::FormatDelimitedInteger(s);

		// if this is the primary player character, include the galactic reserve balance, if any
		if (Game::getPlayerNetworkId() == m_id)
		{
			PlayerObject const * const po = Game::getPlayerObject();
			if (po && (po->getGalacticReserveDeposit() > 0))
			{
				Unicode::String reserve;
				UIUtils::FormatInteger(reserve, static_cast<int>(po->getGalacticReserveDeposit()));

				s += Unicode::narrowToWide(" (");
				s += reserve;
				s += Unicode::narrowToWide("B GR)");
			}
		}

		m_textBank->SetLocalText (s);
		UIUtils::FormatInteger (s, obj->getCashBalance ());
		
		if(obj->getCashBalance() > 100)
			s = UIUtils::FormatDelimitedInteger(s);

		m_textCash->SetLocalText (s);
		getPage ().ForcePackChildren ();
	}
	else
	{
		m_textCash->Clear ();
		m_textBank->Clear ();
	}
}

//----------------------------------------------------------------------

void CuiMoney::setId (const NetworkId & id)
{
	m_id = id;
	if (isActive ())
	{
		updateValues ();
	}
}

//======================================================================
