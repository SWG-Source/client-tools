//======================================================================
//
// CuiMoney.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMoney_H
#define INCLUDED_CuiMoney_H

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class UIPage;
class UIText;
class ClientObject;
class PlayerObject;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

/**
* CuiMoney is a mediator that displays the bank & cash balance for an object (usually the player)
*/

class CuiMoney : 
public CuiMediator
{
public:

	explicit                      CuiMoney           (UIPage & page);
	void                          onBalanceChanged   (const ClientObject & obj);
	void                          onGalacticReserveDepositChanged(const PlayerObject & obj);
	void                          setId              (const NetworkId & id);

protected:

	virtual void                  performActivate    ();
	virtual void                  performDeactivate  ();

private:

	void                          updateValues ();

private:
	                             ~CuiMoney ();
	                              CuiMoney ();
	                              CuiMoney (const CuiMoney & rhs);
	CuiMoney &                    operator=   (const CuiMoney & rhs); 

private:

	UIText *                      m_textCash;
	UIText *                      m_textBank;

	NetworkId                     m_id;

	MessageDispatch::Callback *   m_callback;
};

//======================================================================

#endif
