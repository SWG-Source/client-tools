//======================================================================
//
// SwgCuiImageDesignerRecipient.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiImageDesignerRecipient_H
#define INCLUDED_SwgCuiImageDesignerRecipient_H

//======================================================================

#include "clientGame/PlayerCreatureController.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

class ClientObject;
class CreatureObject;
class CuiWidget3dObjectListViewer;
class NetworkId;
class TangibleObject;
class UIButton;
class UIComposite;
class UIText;
class UITextbox;

//----------------------------------------------------------------------

class SwgCuiImageDesignerRecipient : public CuiMediator, public UIEventCallback
{
public:
	explicit SwgCuiImageDesignerRecipient (UIPage & page);
	virtual void OnButtonPressed(UIWidget * context);
	virtual void OnTextboxChanged(UIWidget * context);
	virtual void OnCheckboxSet(UIWidget * context);

	virtual bool close();
	virtual void update(float deltaTimeSecs);

	void setDesigner(NetworkId const & designerId);
	void setTerminal(NetworkId const & terminalId);
	void setCurrentHoloemote(std::string const & currentHoloEmote);
	NetworkId const & getDesignerId() const;
	CreatureObject * getDesignerCreature() const;
	void onImageDesignerChangeReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload);
	void onImageDesignerCancelReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload);
	void setAccepted(bool accepted);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	//disabled
	~SwgCuiImageDesignerRecipient ();
	SwgCuiImageDesignerRecipient (const SwgCuiImageDesignerRecipient & rhs);
	SwgCuiImageDesignerRecipient & operator= (const SwgCuiImageDesignerRecipient & rhs);

private:
	void updateTimeLeft(time_t timeLeft);
	void buildAndSendUpdateToServer(bool accepted) const;
	bool isEnoughMoneyOffered() const;
	void setViewersView(std::string const & rootBone, float zoomFactor);
	int checkMoney() const;

private:
	MessageDispatch::Callback* m_callback;
	NetworkId m_designerId;
	CuiWidget3dObjectListViewer * m_viewerBefore;
	CuiWidget3dObjectListViewer * m_viewerAfter;
	CreatureObject * m_beforeDoll;
	CreatureObject * m_afterDoll;
	UIButton * m_rejectButton;
	UIButton * m_acceptButton;
	std::string m_originalCustomizations;
	UIText * m_timeLeft;
	UIText * m_acceptedByDesigner;
	UIText * m_cost;
	UIText * m_purchasedHoloEmote;
	UITextbox * m_offeredMoneyTextBox;
	UICheckbox * m_bodyCheckBox;
	UICheckbox * m_faceCheckBox;
	UIComposite * m_dataComposite;
	NetworkId m_terminalId;
	std::string m_currentHoloEmote;
	bool m_acceptedByDesignerBool;
	bool m_committed;
	NetworkId m_afterDollOriginalHairId;
	std::string m_originalHairCustomizations;
	UIText * m_caption;
};

//======================================================================

#endif
