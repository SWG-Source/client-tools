//======================================================================
//
// SwgCuiVoiceFlyBar.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiVoiceFlyBar.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIList.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UITableModelDefault.h"
#include "UIText.h"

#include "clientGame/Game.h"
#include "clientUserInterface/CuiInputNames.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "ClientUserInterface/CuiVoiceChatManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers.h"
#include "sharedInputMap/InputMap.h"
#include "sharedInputMap/InputMap_Command.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedUtility/Callback.h"
#include "sharedUtility/CallbackReceiver.h"

#include "SwgCuiVoiceFlyBarMessageQueue.h"

namespace SwgCuiVoiceFlyBarNamespace
{
	namespace PopupIds
	{
		const std::string shortlist_remove	= "shortlist_remove";
	}

	static StringId const notConnectedMessage("ui_voice","flybar_not_connected");
	static StringId const serverDisabledMessage("ui_voice","flybar_disabled_trial");
	static StringId const connectedMessage("ui_voice", "flybar_connected");
	static StringId const talkButtonTooltipBase ("ui_voice", "flybar_talkbutton_tooltip");
	static StringId const talkButtonTooltipNoBinding ("ui_voice", "flybar_pushtotalk_button_tooltip");
	static StringId const existingSessionMessage("ui_voice", "existing_session");

	static StringId const channelRemovedString ("ui_voice", "flybar_shortlist_removed");

	static char const * const currentSpeakerText = "currentspeakertext";
	static char const * const simpleSelectionList = "simplechannelselectionlist";
	static char const * const activeSpeakerButton = "activespeakersbutton";
	static char const * const pushToTalkButton = "pushtotalkbutton";
	static char const * const channelSelectionButton = "channelSelectionButton";
	static char const * const channelSelectionPage = "channelSelectionPage";
	static char const * const flybarPage = "flybarpage";
	static char const * const pushToTalkCommandName = "CMD_pushToTalk";
	static char const * const toggleButton = "togglecharacterbutton";

	static float const currentSpeakerMinTime = 1.0f;
	static float const currentSpeakerSliderbarChangeLockTime = 1.0f;
	static float const currentSpeakerMouseOverLockTime = 1.0f;
	static float const channelJoinMessageLifetime = 2.0f;
	static float const disabledOpacity = 0.5f;
	static float const connectedMessageTime = 3.0f;
	static float const disconnectedMessageTime = 3.0f;
	static float const channelSelectionTimeout = 3.0f;

	inline int ratioToPercent(float r)
	{
		return clamp(0, static_cast<int>(100.0f * r), 100);
	}

	inline float percentToFloat(int p)
	{
		return clamp(0.0f, static_cast<float>(p) / 100.0f, 1.0f);
	}

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const SwgCuiVoiceFlyBar::Messages::VisibilityChanged::Payload &, SwgCuiVoiceFlyBar::Messages::VisibilityChanged>
			visibilityChanged;
	}


	class KeybindingChangedCallbackReceiver : public CallbackReceiver 
	{
	public:
		KeybindingChangedCallbackReceiver()
			: flybar(NULL)
		{		
		}

		void performCallback ()
		{
			if (flybar)
				flybar->onKeybindingsChanged ();
		}

		SwgCuiVoiceFlyBar * flybar;
	};

	KeybindingChangedCallbackReceiver s_keybindingsChangedCallback;

	static bool changeAccountsPending = false;
}

using namespace SwgCuiVoiceFlyBarNamespace;

SwgCuiVoiceFlyBar::SwgCuiVoiceFlyBar(UIPage & page)
: SwgCuiLockableMediator ("SwgCuiVoiceFlyBar", page),
  m_callback (new MessageDispatch::Callback),
  m_flybarPage (0),
  m_speakerText (0),
  m_simpleSelectionList (0),
  m_activeSpeakerButton (0),
  m_talkButton (0),
  m_channelSelectionButton (0),
  m_channelSelectionPage(0),
  m_toggleCharacterButton(0),
  m_popupChannel(),
  m_messageQueue(NULL),
  m_idleTime(0.0f)
{
	getCodeDataObject (TUIPage, m_flybarPage, flybarPage);
	getCodeDataObject (TUIText, m_speakerText, currentSpeakerText);
	getCodeDataObject (TUIList, m_simpleSelectionList, simpleSelectionList);
	getCodeDataObject (TUIButton, m_activeSpeakerButton, activeSpeakerButton);
	getCodeDataObject (TUIButton, m_talkButton, pushToTalkButton);
	getCodeDataObject (TUIButton, m_channelSelectionButton, channelSelectionButton);
	getCodeDataObject (TUIPage, m_channelSelectionPage, channelSelectionPage, true);
	getCodeDataObject (TUIButton, m_toggleCharacterButton, toggleButton);

	m_messageQueue = new FlybarMessageQueue(m_speakerText);

	registerMediatorObject(*m_toggleCharacterButton, true);
}

//----------------------------------------------------------------------

SwgCuiVoiceFlyBar::~SwgCuiVoiceFlyBar()
{
	delete m_messageQueue;
	m_messageQueue = NULL;

	m_flybarPage = 0;
	m_channelSelectionPage = 0;
	m_channelSelectionButton = 0;
	m_talkButton = 0;
	m_activeSpeakerButton = 0;
	m_speakerText = 0;
	m_simpleSelectionList = 0;
	delete m_callback;
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::performActivate()
{
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onChannelAddedToShortlist, static_cast<CuiVoiceChatManager::Messages::ChannelAddedToShortlist*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onChannelRemovedFromShortlist, static_cast<CuiVoiceChatManager::Messages::ChannelRemovedFromShortlist*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onChannelSpeakingToChanged, static_cast<CuiVoiceChatManager::Messages::SpeakingToChannelChanged*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onCurrentSpeakerChanged, static_cast<CuiVoiceChatManager::Messages::CurrentSpeakerChanged*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onPushToTalkModeChanged, static_cast<CuiVoiceChatManager::Messages::PushToTalkModeChanged*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onPushToTalkKeyPressed, static_cast<CuiVoiceChatManager::Messages::PushToTalkKeyPressed*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onLoginStateChanged, static_cast<CuiVoiceChatManager::Messages::LoginStateChanged*>(0));
	m_callback->connect(*this, &SwgCuiVoiceFlyBar::onExistingSessionChanged, static_cast<CuiVoiceChatManager::Messages::ExistingSessionChanged*>(0));

	if(m_simpleSelectionList)
	{
		m_simpleSelectionList->Clear();
		m_simpleSelectionList->AddRow (Unicode::narrowToWide ("None"), "None");

		std::set<std::string> const & shortlist = CuiVoiceChatManager::getShortlistedChannels();
		for(std::set<std::string>::const_iterator i = shortlist.begin(); i != shortlist.end(); ++i)
		{
			CuiVoiceChatManager::VoiceChatChannelData data;
			if(CuiVoiceChatManager::getInstance().findChannelData(*i,data))
			{
				addChannelToSimpleList(data);
			}
		}

		refreshSimpleListSelection();
	}

	if(m_talkButton)
	{
		m_talkButton->SetEnabled (CuiVoiceChatManager::getUsePushToTalk());
		m_talkButton->SetActivated (CuiVoiceChatManager::getPushToTalkKeyPressed());
	}

	m_toggleCharacterButton->SetEnabled(CuiVoiceChatManager::isOtherPlayer());

	setIsUpdating (true);

	onLoginStateChanged(CuiVoiceChatManager::isLoggedIn());

	Transceivers::visibilityChanged.emitMessage (true);

	s_keybindingsChangedCallback.flybar = this;
	CuiPreferences::getKeybindingsChangedCallback ().attachReceiver (s_keybindingsChangedCallback);
	onKeybindingsChanged();

	m_idleTime = 0.0f;	
	
	if(!CuiVoiceChatManager::getVoiceChatEnabled())
	{
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::performDeactivate()
{
	Transceivers::visibilityChanged.emitMessage (false);

	CuiPreferences::getKeybindingsChangedCallback ().detachReceiver (s_keybindingsChangedCallback);
	s_keybindingsChangedCallback.flybar = NULL;

	setIsUpdating (false);

	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onExistingSessionChanged, static_cast<CuiVoiceChatManager::Messages::ExistingSessionChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onLoginStateChanged, static_cast<CuiVoiceChatManager::Messages::LoginStateChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onPushToTalkKeyPressed, static_cast<CuiVoiceChatManager::Messages::PushToTalkKeyPressed*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onPushToTalkModeChanged, static_cast<CuiVoiceChatManager::Messages::PushToTalkModeChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onCurrentSpeakerChanged, static_cast<CuiVoiceChatManager::Messages::CurrentSpeakerChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onChannelSpeakingToChanged, static_cast<CuiVoiceChatManager::Messages::SpeakingToChannelChanged*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onChannelRemovedFromShortlist, static_cast<CuiVoiceChatManager::Messages::ChannelRemovedFromShortlist*>(0));
	m_callback->disconnect(*this, &SwgCuiVoiceFlyBar::onChannelAddedToShortlist, static_cast<CuiVoiceChatManager::Messages::ChannelAddedToShortlist*>(0));

}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_simpleSelectionList)
	{
		int const selectedRow = m_simpleSelectionList->GetLastSelectedRow();
		if(selectedRow == 0)
		{
			CuiVoiceChatManager::setChannelSimpleMode("");
		}
		else
		{
			UIData* data = m_simpleSelectionList->GetDataAtRow(selectedRow);
			if(data)
			{
				std::string channel = data->GetName();
				CuiVoiceChatManager::setChannelSimpleMode (channel);
			}
		}

		if(m_channelSelectionPage)
		{
			m_channelSelectionPage->SetVisible(false);
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiVoiceFlyBar::OnMessage(UIWidget * context, UIMessage const & msg)
{
	if(context == m_flybarPage)
	{
		if(msg.Type == UIMessage::MouseMove)
		{
			if(CuiVoiceChatManager::getVoiceChatDisabledByServer())
			{
				m_messageQueue->enqueue(serverDisabledMessage.localize(), disconnectedMessageTime, disconnectedMessageTime, 1.0f);
			}
			else if(!CuiVoiceChatManager::isLoggedIn())
			{
				m_messageQueue->enqueue(notConnectedMessage.localize(), disconnectedMessageTime, disconnectedMessageTime, 1.0f);
			}
			else if (m_toggleCharacterButton->IsEnabled())
			{
				m_messageQueue->enqueue(existingSessionMessage.localize(), connectedMessageTime, connectedMessageTime, -1.0f);
			}

			m_idleTime = 0.0f;
		}
		else if(msg.Type == UIMessage::RightMouseDown)
		{
			return false;
		}
		else if(msg.Type == UIMessage::RightMouseUp)
		{
			generateLockablePopup(context, msg);
		}
	}
	else if (context == m_talkButton)
	{
		if(msg.IsMouseDownCommand())
		{
			CuiVoiceChatManager::pushToTalkKeyPressed (true);
		}
		else if (msg.IsMouseUpCommand())
		{
			CuiVoiceChatManager::pushToTalkKeyPressed (false);
		}
	}
	else if (context == m_simpleSelectionList)
	{ 
		if (msg.Type == UIMessage::RightMouseUp)
		{
			m_popupChannel = "";
			long clickedRow = -1;
			if(m_simpleSelectionList->GetRowFromPoint(msg.MouseCoords, clickedRow) && clickedRow > 0)
			{
				UIData* data = m_simpleSelectionList->GetDataAtRow(clickedRow);
				if(data)
				{
					std::string channel = data->GetName();

					if(CuiVoiceChatManager::canChannelBeRemovedFromShortlist(channel))
					{
						m_popupChannel = channel;

						UIPopupMenu * const pop = new UIPopupMenu(&getPage()); 
						pop->SetStyle(getPage().FindPopupStyle());

						pop->AddItem(SwgCuiVoiceFlyBarNamespace::PopupIds::shortlist_remove, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::VOICE_SHORTLIST_REMOVE, 0));

						pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
						UIManager::gUIManager().PushContextWidget(*pop);
						pop->AddCallback(this);
					}
				}
			}
			return false;		
		}
		else if(msg.Type == UIMessage::RightMouseDown)
		{
			return false;
		}
		else if(msg.Type == UIMessage::MouseMove)
		{
			m_idleTime = 0.0f;
		}
	}
	else if(context == m_channelSelectionPage && m_channelSelectionPage)
	{
		if(msg.Type == UIMessage::MouseMove)
		{
			m_idleTime = 0.0f;
		}
	}
	else if(context == m_speakerText)
	{
		if(msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			m_messageQueue->performActionOnCurrent();
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::OnPopupMenuSelection (UIWidget * context)
{
	if (!context || !context->IsA(TUIPopupMenu))
		return;

	SwgCuiLockableMediator::OnPopupMenuSelection(context);

	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	if (!pop)
		return;

	if(m_popupChannel.empty())
		return;

	const std::string & selection = pop->GetSelectedName();

	//this is the only possible selection at this point...
	if (selection == PopupIds::shortlist_remove)
	{
		CuiVoiceChatManager::requestLeaveChannel(m_popupChannel);
		CuiVoiceChatManager::shortlistRemoveChannel(m_popupChannel);
		CuiVoiceChatManager::getInstance().eraseChannelData(m_popupChannel);
	}

	m_popupChannel = "";
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::OnButtonPressed(UIWidget * context)
{
	if(context == m_activeSpeakerButton)
	{
		CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_VoiceActiveSpeakers);
	}
	else if(context == m_channelSelectionButton)
	{
		m_idleTime = 0.0f;
	}
	else if(context == m_toggleCharacterButton && !changeAccountsPending)
	{
		char buffer[256];
		memset(buffer, 0, 256);
		CuiVoiceChatManager::UserInfo userInfo = CuiVoiceChatManager::getUserInfo();
		sprintf(buffer, "You are currently logged in as %s, are you sure you want to login with your current character?", userInfo.m_userName.c_str());
		CuiMessageBox * const box = CuiMessageBox::createYesNoBox(Unicode::narrowToWide(buffer));
		m_callback->connect(box->getTransceiverClosed(), *this, &SwgCuiVoiceFlyBar::OnToggleCharacterBoxConfirmation);
		changeAccountsPending = true;
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	m_idleTime += deltaTimeSecs;
	if(m_channelSelectionPage && m_channelSelectionPage->IsVisible() && m_idleTime > channelSelectionTimeout)
	{
		m_channelSelectionPage->SetVisible(false);
	}

	m_messageQueue->update (deltaTimeSecs);
}

//----------------------------------------------------------------------

int SwgCuiVoiceFlyBar::getRowForChannel(UIList const * const channelList, CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	if(channelList)
	{
		long numRows = channelList->GetRowCount();
		for(long row = 0; row < numRows; ++row)
		{
			UIData const * const data = channelList->GetDataAtRow (row);
			if(data && data->GetName() == channelData.name)
			{
				return row;
			}
		}
	}
	return -1;
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::addChannelToSimpleList(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	if(m_simpleSelectionList)
	{
		m_simpleSelectionList->AddRow (Unicode::narrowToWide (channelData.displayName), channelData.name);
		refreshSimpleListSelection();
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::removeChannelFromSimpleList(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	if(m_simpleSelectionList)
	{
		long numRows = m_simpleSelectionList->GetRowCount();
		for(long row = 0; row < numRows; ++row)
		{
			UIData const * const data = m_simpleSelectionList->GetDataAtRow (row);
			if(data && data->GetName() == channelData.name)
			{
				m_simpleSelectionList->RemoveRow (row);
			}
		}
		refreshSimpleListSelection();
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::refreshSimpleListSelection()
{
	if(m_simpleSelectionList)
	{
		std::string const & speakingTo = CuiVoiceChatManager::getSpeakingToChannel();
		m_simpleSelectionList->SelectRow (0, false);
		int numRows = m_simpleSelectionList->GetRowCount();
		for(int row = 0; row < numRows; ++row)
		{
			UIData const * const data = m_simpleSelectionList->GetDataAtRow (row);
			if(data && data->GetName() == speakingTo)
			{
				m_simpleSelectionList->SelectRow (row, false);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onChannelAddedToShortlist(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	m_messageQueue->enqueue(new ChannelAddedFlybarMessage(channelData, channelJoinMessageLifetime, channelJoinMessageLifetime, -1.0f));

	addChannelToSimpleList (channelData);
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onChannelRemovedFromShortlist(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	removeChannelFromSimpleList (channelData);

	Unicode::String flybarMessage;
	CuiStringVariablesManager::process (SwgCuiVoiceFlyBarNamespace::channelRemovedString, Unicode::narrowToWide(channelData.displayName), Unicode::String (), Unicode::String (), flybarMessage);
	m_messageQueue->enqueue(flybarMessage, channelJoinMessageLifetime, channelJoinMessageLifetime, -1.0f);
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onChannelSpeakingToChanged(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	if(m_simpleSelectionList)
	{
		int row = getRowForChannel (m_simpleSelectionList, channelData);
		if(row >= 0)
		{
			m_simpleSelectionList->SelectRow (row, false);
		}
		else
		{
			m_simpleSelectionList->SelectRow (0, false);
		}
	}

	if(m_speakerText && CuiVoiceChatManager::isLoggedIn())
	{
		std::string text;
		if (channelData.name.empty())
		{
			text = "Channel: (none)";
		}
		else
		{
			text = "Channel: " + channelData.displayName;
		}
		m_messageQueue->enqueue(Unicode::narrowToWide (text), channelJoinMessageLifetime, channelJoinMessageLifetime, -1.0f);
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onCurrentSpeakerChanged(int const &)
{
	CuiVoiceChatManager::ActiveSpeaker currentSpeaker;
	if(CuiVoiceChatManager::getCurrentSpeakerInfo (currentSpeaker))
	{
		m_messageQueue->enqueue(new ParticipantFlybarMessage(currentSpeaker.id, currentSpeakerMinTime));
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onPushToTalkModeChanged(bool const & on)
{
	if(m_talkButton)
	{
		m_talkButton->SetEnabled (on);
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onPushToTalkKeyPressed(bool const & pressed)
{
	if(m_talkButton)
	{
		m_talkButton->SetActivated (pressed);
	}
}

//----------------------------------------------------------------------

inline void safeEnable(UIWidget * widget, bool enabled)
{
	if(widget && widget->IsEnabled() != enabled)
	{
		widget->SetEnabled(enabled);
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onLoginStateChanged(bool const & loggedIn)
{
	safeEnable(m_talkButton, loggedIn && CuiVoiceChatManager::getUsePushToTalk());
	safeEnable(m_activeSpeakerButton, loggedIn);
	safeEnable(m_channelSelectionButton, loggedIn);

	if(loggedIn)
	{
		getPage().SetOpacity(1.0f);
		m_messageQueue->enqueue(connectedMessage.localize(), connectedMessageTime, connectedMessageTime, -1.0f);
	}
	else
	{
		if(m_channelSelectionPage)
		{
			m_channelSelectionPage->SetVisible(false);
		}

		getPage().SetOpacity(disabledOpacity);
		if(CuiVoiceChatManager::getVoiceChatDisabledByServer())
		{
			m_messageQueue->enqueue(serverDisabledMessage.localize(), disconnectedMessageTime, disconnectedMessageTime, -1.0f);
		}
		else
		{
			m_messageQueue->enqueue(notConnectedMessage.localize(), disconnectedMessageTime, disconnectedMessageTime, -1.0f);
		}
	}

	m_toggleCharacterButton->SetEnabled(CuiVoiceChatManager::isOtherPlayer());
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onKeybindingsChanged()
{
	Unicode::String talkButtonTooltip = talkButtonTooltipNoBinding.localize();

	InputMap const * const inputMap = Game::getGameInputMap ();
	if(inputMap)
	{
		InputMap::CommandBindInfoSet * cmdsBindInfoSets;
		const InputMap::Shifts & shifts = inputMap->getShifts ();
		CuiInputNames::setInputShifts (shifts);
		const InputMap::Command * const cmd = inputMap->findCommandByName (pushToTalkCommandName);
		inputMap->getCommandBindings (cmdsBindInfoSets, cmd);

		if (cmdsBindInfoSets != NULL)
		{
			const InputMap::CommandBindInfoSet & cbis = cmdsBindInfoSets [0];
			DEBUG_WARNING (cbis.cmd != cmd, ("Command binding does not match command"));

			if (cbis.cmd != NULL && cbis.cmd == cmd && cbis.numBinds != 0)
			{
				talkButtonTooltip = talkButtonTooltipBase.localize();
				CuiInputNames::appendInputString (cbis, talkButtonTooltip);
			}

			delete [] cmdsBindInfoSets;
			cmdsBindInfoSets = NULL;
		}
	}

	m_talkButton->SetTooltip(talkButtonTooltip);
}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::onExistingSessionChanged(bool const & existingSession)
{
	if(existingSession)
	{
		getPage().SetOpacity(1.0f);
		m_messageQueue->enqueue(existingSessionMessage.localize(), connectedMessageTime, connectedMessageTime, -1.0f);
	}
	else
		getPage().SetOpacity(disabledOpacity);


	m_toggleCharacterButton->SetEnabled(CuiVoiceChatManager::isOtherPlayer());

}

//----------------------------------------------------------------------

void SwgCuiVoiceFlyBar::OnToggleCharacterBoxConfirmation(const CuiMessageBox & box)
{
	if(box.completedAffirmative())
	{
		CuiVoiceChatManager::forceLogin();
	}

	changeAccountsPending = false;
}

//----------------------------------------------------------------------