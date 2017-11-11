//======================================================================
//
// SwgCuiOptVoice.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptVoice_H
#define INCLUDED_SwgCuiOptVoice_H

#include "swgClientUserInterface/SwgCuiOptBase.h"
#include "sharedMessageDispatch/Receiver.h"

//======================================================================

class UICheckbox;
class UIText;
class CuiControlsMenuBindEntry;

#include <list>

class SwgCuiOptVoice : 
public SwgCuiOptBase,
public MessageDispatch::Receiver
{
public:
	explicit SwgCuiOptVoice (UIPage & page);
	~SwgCuiOptVoice ();

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed (UIWidget *context);

	void onVoiceChatEnabledChanged(bool const & enabled);
	void onLocalSpeakerVolumeChanged(float const & volume);
	void onLocalMicVolumeChanged(float const & volume);
	//void onPushToTalkModeChanged(bool const & on);
	void onInputDeviceList(std::pair<std::string,std::vector<std::string> > const & result);
	void onInputDeviceSet(std::pair<bool, std::string> const & result);
	void onOutputDeviceList(std::pair<std::string,std::vector<std::string> > const & result);
	void onOutputDeviceSet(std::pair<bool, std::string> const & result);

	void onEchoTestRecordStart(bool const &);
	void onEchoTestRecordStop(uint64 const &length);
	void onEchoTestPlaybackStart(uint64 const &length);
	void onEchoTestPlaybackStop(uint64 const &length);
	void onVUMonitorModeChanged(bool const &started);
	void onVUMonitorUpdate(float const &energy);

	void onShowFlybarChanged(bool const & show);

	static int onGetInputCombo (const SwgCuiOptBase & , const UIComboBox &);
	static void onSetInputCombo (const SwgCuiOptBase & base, const UIComboBox & box, int index);
	static int onDefaultInputCombo (const SwgCuiOptBase & base, const UIComboBox & box);
	static int onGetOutputCombo (const SwgCuiOptBase & , const UIComboBox &);
	static void onSetOutputCombo (const SwgCuiOptBase & base, const UIComboBox & box, int index);
	static int onDefaultOutputCombo (const SwgCuiOptBase & base, const UIComboBox & box);

	virtual void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void onKeybindingsChanged();

private:

	void setButtonText(UIButton & button, std::string const & codeDataName);

	SwgCuiOptVoice & operator=(const SwgCuiOptVoice & rhs);
	SwgCuiOptVoice            (const SwgCuiOptVoice & rhs);

	UICheckbox * m_enabledCheck;
	UICheckbox * m_showFlybarCheck;
	//UICheckbox * m_pushToTalkCheck;
	UISliderbar * m_localSpeakerVolumeSlider;
	UISliderbar * m_localMicVolumeSlider;

	UIButton * m_pollDevicesButton;
	UIComboBox * m_inputDeviceCombo;
	UIComboBox * m_outputDeviceCombo;

	UIButton * m_echoRecordButton;
	UIButton * m_echoPlaybackButton;
	UIButton * m_vuButton;
	UISliderbar * m_vuSlider;

	std::list<float> m_vuSamples;

	UIButton * m_checkMicButton;
	UIButton * m_fixMicButton;
	UIText * m_checkMicResultText;

	class InputSchemeCallbackReceiver;

	InputSchemeCallbackReceiver * m_callbackReceiver;
	CuiControlsMenuBindEntry * m_pttBindEntry;
	UIPage * m_pttEntryPage;
	UIButton * m_pttRebindButton;
	UIButton * m_pttClearBindingButton;
	UIText * m_pttBindingText;

};

//======================================================================

#endif
