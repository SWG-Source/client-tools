// ======================================================================
//
// SwgCuiBiographyEdit.h
// Copyright(c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef SwgCuiBiographyEdit_H
#define SwgCuiBiographyEdit_H

//-----------------------------------------------------------------

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

#include "clientGame/PlayerCreatureController.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"

//-----------------------------------------------------------------

class UIButton;
class UIImage;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiBiographyEdit :
public CuiMediator,
public UIEventCallback
{
public:
	static SwgCuiBiographyEdit* createInto(UIPage & parent);

public:
	explicit SwgCuiBiographyEdit(UIPage & page);
	
	virtual void OnButtonPressed(UIWidget *context);
	
	void onBiographyRetrieved(PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner const &msg);
	bool requestBiography(CachedNetworkId const & playerId);

protected:
	virtual void update(float deltaTimeSecs);
	virtual void performActivate();
	virtual void performDeactivate();

private:
	~SwgCuiBiographyEdit();
	SwgCuiBiographyEdit(const SwgCuiBiographyEdit &);
	SwgCuiBiographyEdit & operator=(const SwgCuiBiographyEdit &);

private:
	CachedNetworkId m_player;

	UIText * m_biographyText;
	UIText * m_characterName;
	UIText * m_pageStatus;
	UIButton * m_saveButton;
	UIButton * m_cancelButton;

	UIColor m_editColor;
	
	enum BiographyEditStatus { BES_none, BES_failed, BES_waiting, BES_timedout, BES_success };
	BiographyEditStatus m_biographyStatus;

	Timer m_biographyTimeout;

	MessageDispatch::Callback* m_callBack;
};

//-----------------------------------------------------------------

#endif

