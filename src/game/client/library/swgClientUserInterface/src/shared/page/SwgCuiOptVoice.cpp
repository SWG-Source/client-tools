//======================================================================
//
// SwgCuiOptVoice.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptVoice.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"

#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiControlsMenuBindEntry.h"
#include "clientUserInterface/CuiInputNames.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedInputMap/InputMap.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiVoiceFlyBar.h"

#include <algorithm>
#include <vector>

//======================================================================

namespace SwgCuiOptVoiceNamespace
{
	static StringId const problemsRemaining("ui_voice", "checkmic_problems_remain");
	static StringId const allProblemsFixed("ui_voice", "checkmic_problems_fixed");

	static char const * const echoTestStartText = "stringechorecordstart";
	static char const * const echoTestStopText = "stringechorecordstop";
	static char const * const echoTestPlaybackStartText = "stringechoplaybackstart";
	static char const * const echoTestPlaybackStopText = "stringechoplaybackstop";
	static char const * const vuMonitorStartText = "stringvumonitorstart";
	static char const * const vuMonitorStopText = "stringvumonitorstop";
	static char const * const pushToTalkCommandName = "CMD_pushToTalk";

	static const uint32 vuMonitorSampleAverageCount = 5;

	int   getDefaultIndexZero  (const SwgCuiOptBase & , const UIComboBox &) { return 0; }

	int getIndexForThisRow(UIComboBox const & combo, std::string const & rowName)
	{
		const int numRows = combo.GetItemCount();
		for(int i = 0; i < numRows; ++i)
		{
			std::string indexName;
			if(combo.GetIndexName(i, indexName) && indexName == rowName)
			{
				return i;
			}
		}
		return -1;
	}

	bool selectThisRow(UIComboBox & combo, std::string const & rowName)
	{
		int index = getIndexForThisRow(combo,rowName);
		combo.SetSelectedIndex(index);
		return index != -1;
	}

	float addSampleAndCalcAverage(std::list<float> & samples, float newSample, uint32 maxSamples = vuMonitorSampleAverageCount)
	{
		float total = 0.0f;
		samples.push_back(newSample);
		while(samples.size() > maxSamples)
		{
			samples.pop_front();
		}

		float count = static_cast<float>(samples.size());
		if(count < 1.0f)
		{
			//safeguard against divide by zero with a reasonable guess at what would be a valid value
			return newSample;
		}

		for(std::list<float>::const_iterator i = samples.begin(); i != samples.end(); ++i)
		{
			total += *i;
		}

		return total / count;
	}

	void setSpeakerVolume(float volume)
	{
		CuiPreferences::setSpeakerVolume(volume);
		CuiVoiceChatManager::setLocalSpeakerVolume(CuiPreferences::getSpeakerVolume());
	}

	void setMicVolume(float volume)
	{
		CuiPreferences::setMicVolume(volume);
		CuiVoiceChatManager::setLocalMicVolume(CuiPreferences::getMicVolume());
	}

}


class SwgCuiOptVoice::InputSchemeCallbackReceiver : public CallbackReceiver 
{
public:
	InputSchemeCallbackReceiver()
		: optPage(NULL)
	{		
	}

	void performCallback ()
	{
		if (optPage)
			optPage->onKeybindingsChanged ();
	}

	SwgCuiOptVoice * optPage;
};


using namespace SwgCuiOptVoiceNamespace;

SwgCuiOptVoice::SwgCuiOptVoice (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptVoice", page),
MessageDispatch::Receiver(),
m_enabledCheck (0),
m_showFlybarCheck (0),
//m_pushToTalkCheck (0),
m_localSpeakerVolumeSlider (0),
m_localMicVolumeSlider (0),
m_pollDevicesButton (0),
m_inputDeviceCombo (0),
m_outputDeviceCombo (0),
m_echoRecordButton (0),
m_echoPlaybackButton (0),
m_vuButton (0),
m_vuSlider (0),
m_vuSamples(),
m_checkMicButton(0),
m_fixMicButton(0),
m_checkMicResultText(0),
m_callbackReceiver(new InputSchemeCallbackReceiver()),
m_pttBindEntry(0),
m_pttEntryPage(0),
m_pttRebindButton(0),
m_pttClearBindingButton(0),
m_pttBindingText(0)
{
	m_callbackReceiver->optPage = this;
	connectToMessage (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING);


	getCodeDataObject (TUICheckbox, m_enabledCheck, "checkEnable");
	registerCheckbox (*m_enabledCheck, CuiVoiceChatManager::setVoiceChatEnabled, CuiVoiceChatManager::getVoiceChatEnabled, CuiVoiceChatManager::getVoiceChatEnabled);
	
	//-- restore checkbox & enabled states
	//this may not be necessary
	m_enabledCheck->SetChecked (!CuiVoiceChatManager::getVoiceChatEnabled ());
	m_enabledCheck->SetChecked (CuiVoiceChatManager::getVoiceChatEnabled ());

	getCodeDataObject (TUICheckbox, m_showFlybarCheck, "checkshowflybar");
	registerCheckbox (*m_showFlybarCheck, CuiVoiceChatManager::setShowFlybar, CuiVoiceChatManager::getShowFlybar, getTrue);

	//push to talk mode is always on until vivox adds thresholding to the non-push to talk mode
	//getCodeDataObject (TUICheckbox, m_pushToTalkCheck, "checkpushtotalk");
	//registerCheckbox (*m_pushToTalkCheck, CuiVoiceChatManager::setUsePushToTalk, CuiVoiceChatManager::getUsePushToTalk, CuiVoiceChatManager::getUsePushToTalkDefault);

	getCodeDataObject(TUISliderbar, m_localSpeakerVolumeSlider, "slidervolumespeaker");
	registerSlider (*m_localSpeakerVolumeSlider,SwgCuiOptVoiceNamespace::setSpeakerVolume,CuiPreferences::getSpeakerVolume,CuiVoiceChatManager::getLocalSpeakerVolumeDefault,0.0f,1.0f);

	getCodeDataObject (TUISliderbar, m_localMicVolumeSlider, "slidervolumemic");
	registerSlider (*m_localMicVolumeSlider,SwgCuiOptVoiceNamespace::setMicVolume,CuiPreferences::getMicVolume,CuiVoiceChatManager::getLocalMicVolumeDefault,0.0f,0.8f);

	UISliderbar * tmpSlider;
	getCodeDataObject (TUISliderbar, tmpSlider, "slideraudiofade");
	registerSlider (*tmpSlider, Audio::setFadeAllFactor, Audio::getFadeAllFactor, Audio::getDefaultFadeAllFactor, 0.0f, 1.0f);

	//UICheckbox * tmpCheck = 0;
	//getCodeDataObject (TUICheckbox, tmpCheck, "checkadvancedchannelselection");
	//registerCheckbox (*tmpCheck, CuiVoiceChatManager::setUseAdvancedChannelSelection, CuiVoiceChatManager::getUseAdvancedChannelSelection, getFalse);

	getCodeDataObject (TUIComboBox, m_inputDeviceCombo, "comboinputdevice");
	m_inputDeviceCombo->Clear();
	registerComboBox(*m_inputDeviceCombo, SwgCuiOptVoice::onSetInputCombo, SwgCuiOptVoice::onGetInputCombo, SwgCuiOptVoice::onDefaultInputCombo);

	getCodeDataObject (TUIComboBox, m_outputDeviceCombo, "combooutputdevice");
	m_outputDeviceCombo->Clear();
	registerComboBox(*m_outputDeviceCombo, SwgCuiOptVoice::onSetOutputCombo, SwgCuiOptVoice::onGetOutputCombo, SwgCuiOptVoice::onDefaultOutputCombo);


	getCodeDataObject(TUIButton, m_pollDevicesButton, "buttonpolldevices");

	getCodeDataObject(TUIButton, m_echoRecordButton, "buttonechorecord");
	getCodeDataObject(TUIButton, m_echoPlaybackButton, "buttonechoplayback");

	getCodeDataObject(TUIButton, m_vuButton, "buttonvumonitor");
	getCodeDataObject(TUISliderbar, m_vuSlider, "slidervumonitor");

	getCodeDataObject(TUIButton, m_checkMicButton, "buttoncheckmic");
	getCodeDataObject(TUIButton, m_fixMicButton, "buttonfixmic");
	getCodeDataObject(TUIText, m_checkMicResultText, "textcheckmicresult");

	UICheckbox * tempCheck = 0;
	getCodeDataObject(TUICheckbox, tempCheck, "checkautodeclineinvites");
	registerCheckbox(*tempCheck, CuiPreferences::setVoiceAutoDeclineInvites, CuiPreferences::getVoiceAutoDeclineInvites, getFalse);
	
	tempCheck = 0;
	getCodeDataObject(TUICheckbox, tempCheck, "checkautojoin");
	registerCheckbox(*tempCheck, CuiPreferences::setVoiceAutoJoinChannels, CuiPreferences::getVoiceAutoJoinChannels, getTrue);

	getCodeDataObject(TUIButton, m_pttRebindButton, "buttonpttrebind");
	getCodeDataObject(TUIButton, m_pttClearBindingButton, "buttonpttclearbinding");
	getCodeDataObject(TUIText, m_pttBindingText, "textpttbinding");
	getCodeDataObject(TUIPage, m_pttEntryPage, "entrypage");
	m_pttEntryPage->SetVisible(false);


	m_callback->connect(*this, &SwgCuiOptVoice::onShowFlybarChanged, static_cast<CuiVoiceChatManager::Messages::ShowFlybarChanged*>(0));
}

//----------------------------------------------------------------------

SwgCuiOptVoice::~SwgCuiOptVoice()
{
	m_callback->disconnect(*this, &SwgCuiOptVoice::onShowFlybarChanged, static_cast<CuiVoiceChatManager::Messages::ShowFlybarChanged*>(0));

	disconnectFromMessage (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING);

	m_callbackReceiver->optPage = NULL;
	delete m_callbackReceiver;
	m_callbackReceiver = 0;

	if (m_pttBindEntry)
	{
		m_pttBindEntry->deactivate ();
		m_pttBindEntry->release ();
		m_pttBindEntry = 0;
	}

	m_pttBindEntry = 0;
	m_pttEntryPage = 0;
	m_pttBindingText = 0;
	m_pttClearBindingButton = 0;
	m_pttRebindButton = 0;
	m_enabledCheck = 0;
	m_showFlybarCheck = 0;
	m_localSpeakerVolumeSlider = 0;
	m_localMicVolumeSlider = 0;
	m_inputDeviceCombo = 0;
	m_outputDeviceCombo = 0;
	m_echoRecordButton = 0;
	m_echoPlaybackButton = 0;
	m_vuButton = 0;
	m_vuSlider = 0;
	m_checkMicButton = 0;
	m_fixMicButton = 0;
	m_checkMicResultText = 0;
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::performActivate   ()
{
	SwgCuiOptBase::performActivate();

	m_callback->connect(*this, &SwgCuiOptVoice::onVoiceChatEnabledChanged, static_cast<CuiVoiceChatManager::Messages::VoiceChatEnabledChanged*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onLocalSpeakerVolumeChanged, static_cast<CuiVoiceChatManager::Messages::LocalSpeakerVolumeChanged*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onLocalMicVolumeChanged, static_cast<CuiVoiceChatManager::Messages::LocalMicVolumeChanged*>(0));
	//m_callback->connect(*this, &SwgCuiOptVoice::onPushToTalkModeChanged, static_cast<CuiVoiceChatManager::Messages::PushToTalkModeChanged*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onInputDeviceList, static_cast<CuiVoiceChatManager::Messages::InputDeviceList*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onInputDeviceSet, static_cast<CuiVoiceChatManager::Messages::InputDeviceSet*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onOutputDeviceList, static_cast<CuiVoiceChatManager::Messages::OutputDeviceList*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onOutputDeviceSet, static_cast<CuiVoiceChatManager::Messages::OutputDeviceSet*>(0));

	m_callback->connect(*this, &SwgCuiOptVoice::onEchoTestRecordStart, static_cast<CuiVoiceChatManager::Messages::EchoTestRecordStart*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onEchoTestRecordStop, static_cast<CuiVoiceChatManager::Messages::EchoTestRecordStop*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onEchoTestPlaybackStart, static_cast<CuiVoiceChatManager::Messages::EchoTestPlaybackStart*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onEchoTestPlaybackStop, static_cast<CuiVoiceChatManager::Messages::EchoTestPlaybackStop*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onVUMonitorModeChanged, static_cast<CuiVoiceChatManager::Messages::OnVUMonitorModeChange*>(0));
	m_callback->connect(*this, &SwgCuiOptVoice::onVUMonitorUpdate, static_cast<CuiVoiceChatManager::Messages::OnVUMonitorUpdate*>(0));

	m_inputDeviceCombo->Clear();
	m_outputDeviceCombo->Clear();
	CuiVoiceChatManager::requestInputDeviceList();
	CuiVoiceChatManager::requestOutputDeviceList();

	setButtonText(*m_echoRecordButton, echoTestStartText);
	setButtonText(*m_echoPlaybackButton, echoTestPlaybackStartText);

	m_echoRecordButton->SetEnabled(true);
	m_echoPlaybackButton->SetEnabled(false);

	bool const vuMonitorActive = CuiVoiceChatManager::isVUMonitorActive();
	setButtonText(*m_vuButton, vuMonitorActive ? vuMonitorStopText : vuMonitorStartText);
	m_vuSlider->SetEnabled(vuMonitorActive);
	m_vuSlider->SetValue(0, true);
	m_vuSamples.clear();

	m_checkMicResultText->SetText(UIString());
	m_fixMicButton->SetEnabled(false);

	CuiPreferences::getKeybindingsChangedCallback ().attachReceiver (*m_callbackReceiver);
	onKeybindingsChanged();
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::performDeactivate ()
{
	CuiPreferences::getKeybindingsChangedCallback ().detachReceiver (*m_callbackReceiver);

	m_callback->disconnect(*this, &SwgCuiOptVoice::onVUMonitorUpdate, static_cast<CuiVoiceChatManager::Messages::OnVUMonitorUpdate*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onVUMonitorModeChanged, static_cast<CuiVoiceChatManager::Messages::OnVUMonitorModeChange*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onEchoTestPlaybackStop, static_cast<CuiVoiceChatManager::Messages::EchoTestPlaybackStop*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onEchoTestPlaybackStart, static_cast<CuiVoiceChatManager::Messages::EchoTestPlaybackStart*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onEchoTestRecordStop, static_cast<CuiVoiceChatManager::Messages::EchoTestRecordStop*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onEchoTestRecordStart, static_cast<CuiVoiceChatManager::Messages::EchoTestRecordStart*>(0));

	m_callback->disconnect(*this, &SwgCuiOptVoice::onOutputDeviceSet, static_cast<CuiVoiceChatManager::Messages::OutputDeviceSet*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onOutputDeviceList, static_cast<CuiVoiceChatManager::Messages::OutputDeviceList*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onInputDeviceSet, static_cast<CuiVoiceChatManager::Messages::InputDeviceSet*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onInputDeviceList, static_cast<CuiVoiceChatManager::Messages::InputDeviceList*>(0));
	//m_callback->disconnect(*this, &SwgCuiOptVoice::onPushToTalkModeChanged, static_cast<CuiVoiceChatManager::Messages::PushToTalkModeChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onLocalMicVolumeChanged, static_cast<CuiVoiceChatManager::Messages::LocalMicVolumeChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onLocalSpeakerVolumeChanged, static_cast<CuiVoiceChatManager::Messages::LocalSpeakerVolumeChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiOptVoice::onVoiceChatEnabledChanged, static_cast<CuiVoiceChatManager::Messages::VoiceChatEnabledChanged*>(0));

	if(CuiVoiceChatManager::isVUMonitorActive())
	{
		CuiVoiceChatManager::endVUMonitor();
	}

	if(CuiVoiceChatManager::isPerformingEchoTest() || CuiVoiceChatManager::isPlayingEchoTest())
	{
		CuiVoiceChatManager::stopEchoTest();
	}

	m_echoPlaybackButton->SetEnabled(false);
	m_vuSlider->SetEnabled(false);
	m_vuSlider->SetValue(0, true);

	SwgCuiOptBase::performDeactivate();
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::OnButtonPressed (UIWidget *context)
{
	SwgCuiOptBase::OnButtonPressed(context);

	if(context == m_pollDevicesButton)
	{
		m_inputDeviceCombo->Clear();
		m_outputDeviceCombo->Clear();

		CuiVoiceChatManager::requestInputDeviceList();
		CuiVoiceChatManager::requestOutputDeviceList();
	}

	if(context == m_echoRecordButton)
	{
		if(CuiVoiceChatManager::isPerformingEchoTest())
		{
			CuiVoiceChatManager::stopEchoTest();
		}
		else
		{
			CuiVoiceChatManager::beginEchoTest();
		}
	}

	if(context == m_echoPlaybackButton)
	{
		if(CuiVoiceChatManager::isPlayingEchoTest())
		{
			CuiVoiceChatManager::stopEchoTest();
		}
		else
		{
			if(CuiVoiceChatManager::isEchoTestPlaybackAvailable())
			{
				CuiVoiceChatManager::startEchoTestPlayback();
			}
		}
	}

	if(context == m_vuButton)
	{
		if(CuiVoiceChatManager::isVUMonitorActive())
		{
			CuiVoiceChatManager::endVUMonitor();
		}
		else
		{
			CuiVoiceChatManager::beginVUMonitor();
		}
	}

	if(context == m_checkMicButton)
	{
		uint32 result = CuiVoiceChatManager::checkWindowsMicSettings(CuiVoiceChatManager::getCurrentInputDevice(), false);
		Unicode::String resultText = CuiVoiceChatManager::getCheckMicString(result);
		m_checkMicResultText->SetText(resultText);
		m_fixMicButton->SetEnabled(CuiVoiceChatManager::canWindowsSettingsBeCorrected(result));
	}

	if(context == m_fixMicButton)
	{
		CuiVoiceChatManager::checkWindowsMicSettings(CuiVoiceChatManager::getCurrentInputDevice(), true);
		uint32 result = CuiVoiceChatManager::checkWindowsMicSettings(CuiVoiceChatManager::getCurrentInputDevice(), false);
		Unicode::String resultText;
		if(result)
		{
			resultText = problemsRemaining.localize() + CuiVoiceChatManager::getCheckMicString(result);
		}
		else
		{
			resultText = allProblemsFixed.localize();
		}
		m_checkMicResultText->SetText(resultText);
		m_fixMicButton->SetEnabled(CuiVoiceChatManager::canWindowsSettingsBeCorrected(result));
	}

	if(context == m_pttRebindButton)
	{
		CuiVoiceChatManager::setUsePushToTalk(true);

		InputMap * inputMap = Game::getGameInputMap();
		if(inputMap)
		{
			const InputMap::Command * const cmd = inputMap->findCommandByName (pushToTalkCommandName);
			NOT_NULL (cmd);

			if (!m_pttBindEntry)
			{
				m_pttBindEntry = new CuiControlsMenuBindEntry (*m_pttEntryPage);
				m_pttBindEntry->fetch ();
			}

			NOT_NULL (m_pttBindEntry);
			m_pttBindEntry->activate ();
			m_pttBindEntry->setCommand (inputMap, cmd);

			if (getContainingWorkspace ())
				getContainingWorkspace ()->cancelFocusForMediator (*this);
		}
	}

	if(context == m_pttClearBindingButton)
	{
		InputMap * inputMap = Game::getGameInputMap();
		if(inputMap)
		{
			const InputMap::Command * const cmd = inputMap->findCommandByName (pushToTalkCommandName);
			NOT_NULL (cmd);

			inputMap->removeBindings (cmd);
			CuiPreferences::signalKeybindingsChanged ();
			inputMap->handleInputReset();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onVoiceChatEnabledChanged(bool const & enabled)
{
	m_enabledCheck->SetChecked (enabled, false);

	if(enabled)
	{
		if(CuiPreferences::getVoiceShowFlybar())
		{
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_VoiceFlyBar);
		}
	}
	else
	{
		CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_VoiceFlyBar);
		CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_VoiceActiveSpeakers);

		if (m_pttBindEntry)
		{
			m_pttBindEntry->deactivate ();
			m_pttBindEntry->release ();
			m_pttBindEntry = 0;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onLocalSpeakerVolumeChanged(float const & volume)
{
	m_localSpeakerVolumeSlider->SetValue (static_cast<int>(100.0f*volume), false);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onLocalMicVolumeChanged(float const & volume)
{
	m_localMicVolumeSlider->SetValue (static_cast<int>(100.0f*volume), false);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onShowFlybarChanged(bool const & show)
{
	if(show)
	{
		CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_VoiceFlyBar);
	}
	else
	{
		CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_VoiceFlyBar);
	}
}

//----------------------------------------------------------------------

//void SwgCuiOptVoice::onPushToTalkModeChanged(bool const & on)
//{
//	m_pushToTalkCheck->SetChecked (on, false);
//
//	if(!on && m_pttBindEntry)
//	{
//		m_pttBindEntry->deactivate ();
//		m_pttBindEntry->release ();
//		m_pttBindEntry = 0;
//	}
//}

//======================================================================

int SwgCuiOptVoice::onGetInputCombo (const SwgCuiOptBase & , const UIComboBox & box)
{
	std::string const & current = CuiVoiceChatManager::getCurrentInputDevice();
	return getIndexForThisRow(box, current);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onSetInputCombo (const SwgCuiOptBase & , const UIComboBox & box, int )
{
	std::string selectedString;
	box.GetSelectedIndexName(selectedString);
	if(!selectedString.empty())
		CuiVoiceChatManager::setInputDevice(selectedString);
}

//----------------------------------------------------------------------

int SwgCuiOptVoice::onDefaultInputCombo (const SwgCuiOptBase &, const UIComboBox & box)
{
	std::string const & defaultDevice = CuiVoiceChatManager::getDefaultInputDevice();
	return getIndexForThisRow(box, defaultDevice);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onInputDeviceList(std::pair<std::string,std::vector<std::string> > const & result)
{
	std::string const & currentDevice = result.first;
	std::vector<std::string> const & deviceList = result.second;

	for(std::vector<std::string>::const_iterator i = deviceList.begin(); i != deviceList.end(); ++i)
	{
		m_inputDeviceCombo->AddItem(Unicode::narrowToWide(*i),*i);
	}

	SwgCuiOptVoiceNamespace::selectThisRow(*m_inputDeviceCombo,currentDevice);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onInputDeviceSet(std::pair<bool, std::string> const & result)
{
	UNREF(result);
	std::string const & deviceName = CuiVoiceChatManager::getCurrentInputDevice();
	SwgCuiOptVoiceNamespace::selectThisRow(*m_inputDeviceCombo, deviceName);
}

//----------------------------------------------------------------------

int SwgCuiOptVoice::onGetOutputCombo (const SwgCuiOptBase & , const UIComboBox & box)
{
	std::string const & current = CuiVoiceChatManager::getCurrentOutputDevice();
	return getIndexForThisRow(box, current);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onSetOutputCombo (const SwgCuiOptBase & , const UIComboBox & box, int )
{
	std::string selectedString;
	box.GetSelectedIndexName(selectedString);
	if(!selectedString.empty())
		CuiVoiceChatManager::setOutputDevice(selectedString);
}

//----------------------------------------------------------------------

int SwgCuiOptVoice::onDefaultOutputCombo (const SwgCuiOptBase &, const UIComboBox & box)
{
	std::string const & defaultDevice = CuiVoiceChatManager::getDefaultOutputDevice();
	return getIndexForThisRow(box, defaultDevice);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onOutputDeviceList(std::pair<std::string,std::vector<std::string> > const & result)
{
	std::string const & currentDevice = result.first;
	std::vector<std::string> const & deviceList = result.second;

	for(std::vector<std::string>::const_iterator i = deviceList.begin(); i != deviceList.end(); ++i)
	{
		m_outputDeviceCombo->AddItem(Unicode::narrowToWide(*i),*i);
	}

	SwgCuiOptVoiceNamespace::selectThisRow(*m_outputDeviceCombo,currentDevice);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onOutputDeviceSet(std::pair<bool, std::string> const & result)
{
	UNREF(result);
	std::string const & deviceName = CuiVoiceChatManager::getCurrentOutputDevice();
	SwgCuiOptVoiceNamespace::selectThisRow(*m_outputDeviceCombo, deviceName);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onEchoTestRecordStart(bool const &)
{
	setButtonText(*m_echoRecordButton, echoTestStopText);
	m_echoPlaybackButton->SetEnabled(false);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onEchoTestRecordStop(uint64 const & /*length*/)
{
	setButtonText(*m_echoRecordButton, echoTestStartText);
	m_echoPlaybackButton->SetEnabled(CuiVoiceChatManager::isEchoTestPlaybackAvailable());
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onEchoTestPlaybackStart(uint64 const & /*length*/)
{
	setButtonText(*m_echoPlaybackButton, echoTestPlaybackStopText);
	m_echoRecordButton->SetEnabled(false);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onEchoTestPlaybackStop(uint64 const & /*length*/)
{
	setButtonText(*m_echoPlaybackButton, echoTestPlaybackStartText);
	m_echoRecordButton->SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onVUMonitorModeChanged(bool const & started)
{
	if(started)
	{
		setButtonText(*m_vuButton, vuMonitorStopText);
		m_vuSlider->SetValue(0, true);
		m_vuSlider->SetEnabled(true);
	}
	else
	{
		setButtonText(*m_vuButton, vuMonitorStartText);
		m_vuSlider->SetValue(0, true);
		m_vuSlider->SetEnabled(false);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onVUMonitorUpdate(float const & energy)
{
	int percent = static_cast<int>(addSampleAndCalcAverage(m_vuSamples, energy) * 100.0f);
	m_vuSlider->SetValue(percent, true);
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::setButtonText(UIButton & button, std::string const & codeDataName)
{
	UIString tempText;
	if(getCodeDataString(codeDataName.c_str(), tempText))
	{
		button.SetText(tempText);
	}
	else
	{
		button.SetText(Unicode::narrowToWide(codeDataName));
	}
}

//----------------------------------------------------------------------

void SwgCuiOptVoice::onKeybindingsChanged()
{
	if(m_pttBindingText)
	{
		InputMap const * const inputMap = Game::getGameInputMap ();
		if(inputMap)
		{
			Unicode::String bindingText;
			CuiInputNames::getInputValueString(*inputMap, pushToTalkCommandName, bindingText);
			m_pttBindingText->SetText(bindingText);
		}
	}
}

//-----------------------------------------------------------------
// taken mostly from SwgCuiOptKeymap
void SwgCuiOptVoice::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType (CuiControlsMenuBindEntry::Messages::UPDATE_BINDING))
	{
		InputMap const * const inputMap = Game::getGameInputMap ();
		if (inputMap)
		{
			if (!inputMap->write (0, false))
			{
				// TODO: localize
				CuiMessageBox::createInfoBox (Unicode::narrowToWide ("Unable to save configuration.\n"));			
			}
			else
			{
				CuiSettings::save();
			}
		}

		CuiPreferences::signalKeybindingsChanged ();
	}
}

//-----------------------------------------------------------------



