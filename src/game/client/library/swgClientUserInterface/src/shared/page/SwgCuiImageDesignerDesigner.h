//======================================================================
//
// SwgCuiImageDesignerDesigner.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiImageDesignerDesigner_H
#define INCLUDED_SwgCuiImageDesignerDesigner_H

//======================================================================

#include "clientGame/PlayerCreatureController.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase.h"

//----------------------------------------------------------------------

class CreatureObject;
class NetworkId;
class UICheckbox;
class UIText;
class UITextbox;

#include <set>

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiImageDesignerDesigner : public SwgCuiAvatarCustomizationBase
{
public:
	explicit SwgCuiImageDesignerDesigner(UIPage & page);
	~SwgCuiImageDesignerDesigner();

	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnSliderbarChanged(UIWidget * context);
	virtual void OnVolumePageSelectionChanged(UIWidget * context);
	virtual void OnCheckboxSet(UIWidget * context);
	virtual void OnCheckboxUnset(UIWidget * context);
	virtual void OnTextboxChanged(UIWidget * context);

	virtual bool close();
	virtual void update(float delta);

	void setRecipient(NetworkId const & recipientId);
	void setTerminal(NetworkId const & terminalId);
	void setCurrentHoloemote(std::string const & currentHoloEmote);

	NetworkId const & getRecipientId() const;
	CreatureObject * getRecipientCreature() const;
	void onImageDesignerChangeReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload);
	void onImageDesignerCancelReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload);

protected:
	virtual void performActivate();
	virtual void performDeactivate();
	virtual void setGroup(const std::string & groupName);
	virtual void updateSelectedHairColorState();

private:
	void setupPage();
	void updateTimeLeft(time_t timeLeft);
	void buildAndSendUpdateToServer(bool accepted) const;
	bool isEnoughMoneyOffered() const;
	void setColorPickerColumnAndMaxIndexes() const;
	void setSingleHoloEmoteCheckbox(UICheckbox * checkbox) const;
	void checkColorPalettes() const;

private:
	//disabled
	SwgCuiImageDesignerDesigner(const SwgCuiImageDesignerDesigner & rhs);
	SwgCuiImageDesignerDesigner & operator= (const SwgCuiImageDesignerDesigner & rhs);

private:
	MessageDispatch::Callback * m_callback;
	UIText * m_timeRemaining;
	UIButton * m_buttonCancel;
	UIButton * m_buttonRevert;
	UIButton * m_buttonCommit;
	NetworkId m_recipientId;
	std::string m_originalCustomizations;
	bool m_forceUpdate;
	CuiWidget3dObjectListViewer * m_paperDollViewer;
	CreatureObject * m_paperDoll;
	UITextbox * m_costTextBox;
	UIText * m_offeredMoney;
	UICheckbox * m_holoEmoteBeehiveCheck;
	UICheckbox * m_holoEmoteBrainstormCheck;
	UICheckbox * m_holoEmoteImperialCheck;
	UICheckbox * m_holoEmoteRebelCheck;
	UICheckbox * m_holoEmoteBubbleheadCheck;
	UICheckbox * m_holoEmoteHologlitterCheck;
	UICheckbox * m_holoEmoteHolonotesCheck;
	UICheckbox * m_holoEmoteSparkyCheck;
	UICheckbox * m_holoEmoteBullhornsCheck;
	UICheckbox * m_holoEmoteChampagneCheck;
	UICheckbox * m_holoEmoteTechnokittyCheck;
	UICheckbox * m_holoEmotePhonytailCheck;
	UICheckbox * m_holoEmoteBlossomCheck;
	UICheckbox * m_holoEmoteButterfliesCheck;
	UICheckbox * m_holoEmoteHauntedCheck;
	UICheckbox * m_holoEmoteHeartsCheck;
	UICheckbox * m_holoEmoteAllCheck;
	UICheckbox * m_holoEmoteNoneCheck;
	UIPage * m_morphPageOverlay;
	UIPage * m_slider1PageOverlay;
	UIPage * m_slider2PageOverlay;
	UIPage * m_slider3PageOverlay;
	UIPage * m_slider4PageOverlay;
	UIPage * m_slider5PageOverlay;
	UIPage * m_slider6PageOverlay;
	UIPage * m_slider7PageOverlay;
	UIPage * m_colorpicker0PageOverlay;
	UIPage * m_colorpicker1PageOverlay;
	UIPage * m_hairPageOverlay;
	UIText * m_nameText;
	NetworkId m_terminalId;
	std::string m_currentHoloEmote;
	std::set<UICheckbox *> m_holoEmoteCheckboxes;
	bool m_committed;
	NetworkId m_originalHairId;
	std::string m_originalHairCustomizations;
};

//======================================================================

#endif
