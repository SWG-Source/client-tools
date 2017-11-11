// ======================================================================
//
// SwgCuiVoiceFlyBar.h
// Copyright (c) 2008 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiVoiceFlyBar_H
#define INCLUDED_SwgCuiVoiceFlyBar_H

// ======================================================================

#include "clientGame/ClientObject.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

// ----------------------------------------------------------------------

class CuiMessageBox;
class UIPage;
class UIText;
class UIList;
class FlybarMessageQueue;

// ----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiVoiceFlyBar :
	public SwgCuiLockableMediator
{
public:
	struct Messages
	{
		struct VisibilityChanged {typedef bool Payload; };
	};

public:
	explicit SwgCuiVoiceFlyBar(UIPage & page);
	~SwgCuiVoiceFlyBar();

	virtual void update(float deltaTimeSecs);

	void onKeybindingsChanged();

protected:
	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnGenericSelectionChanged (UIWidget * context);
	virtual bool OnMessage (UIWidget * context, UIMessage const & msg);
	virtual void OnButtonPressed (UIWidget * context);
	virtual void OnPopupMenuSelection (UIWidget * context);

	int getRowForChannel(UIList const * const channelList, CuiVoiceChatManager::VoiceChatChannelData const & channelData);

	void addChannelToSimpleList(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void removeChannelFromSimpleList(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void refreshSimpleListSelection();

	void onChannelAddedToShortlist(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void onChannelRemovedFromShortlist(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void onChannelSpeakingToChanged(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void onCurrentSpeakerChanged(int const & dummy);
	void onPushToTalkModeChanged(bool const & on);
	void onPushToTalkKeyPressed(bool const & on);
	void onLoginStateChanged(bool const & loggedIn);
	void onExistingSessionChanged(bool const & existingSession);


private:

	void generateChannelListPopup();

	void OnToggleCharacterBoxConfirmation(const CuiMessageBox & box);

	MessageDispatch::Callback * m_callback;

	UIPage * m_flybarPage;
	UIText * m_speakerText;
	UIList * m_simpleSelectionList;
	UIButton * m_activeSpeakerButton;
	UIButton * m_talkButton;
	UIButton * m_channelSelectionButton;
	UIButton * m_toggleCharacterButton;
	UIPage * m_channelSelectionPage;
	
	std::string m_popupChannel;

	FlybarMessageQueue * m_messageQueue;

	float m_idleTime;

private: //-- disabled
	SwgCuiVoiceFlyBar();
	SwgCuiVoiceFlyBar(SwgCuiVoiceFlyBar const & rhs);
	SwgCuiVoiceFlyBar & operator= (SwgCuiVoiceFlyBar const & rhs);
};

// ======================================================================

#endif
