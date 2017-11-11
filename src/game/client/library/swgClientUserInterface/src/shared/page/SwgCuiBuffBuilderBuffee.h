//======================================================================
//
// SwgCuiBuffBuilderBuffee.h
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiBuffBuilderBuffee_H
#define INCLUDED_SwgCuiBuffBuilderBuffee_H

#include "clientGame/PlayerCreatureController.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

namespace MessageDispatch
{
	class Callback;
};

class NetworkId;
class UIList;
class UIText;

// ======================================================================

class SwgCuiBuffBuilderBuffee :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiBuffBuilderBuffee (UIPage & page);

	virtual void                performActivate           ();
	virtual void                performDeactivate         ();

	virtual void                OnButtonPressed           (UIWidget *context );
	virtual void                OnGenericSelectionChanged (UIWidget * context);

	virtual bool close();
	virtual void update(float deltaTimeSecs);

	void setBufferId(NetworkId const & bufferId);
	NetworkId const & getBufferId() const;

	void onBuffBuilderChangeReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload);
	void onBuffBuilderCancelReceived(PlayerCreatureController::Messages::BuffBuilderChangeReceived::Payload const & payload);

private:

	~SwgCuiBuffBuilderBuffee ();
	SwgCuiBuffBuilderBuffee (const SwgCuiBuffBuilderBuffee &);
	SwgCuiBuffBuilderBuffee &  operator= (const SwgCuiBuffBuilderBuffee &);

	CreatureObject * getBufferCreature() const;
	void buildAndSendUpdateToServer(bool const accepted) const;
	bool hasEnoughMoney() const;

	MessageDispatch::Callback *  m_callback;

	NetworkId m_bufferId;
	UIButton * m_cancelButton;
	UIButton * m_acceptButton;
	UIList * m_effectList;
	UIText * m_coverCharge;
	UIText * m_entertainerName;

	bool m_committed;

};

// ======================================================================

#endif

