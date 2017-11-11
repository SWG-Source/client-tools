// ======================================================================
//
// SwgCuiVoiceActiveSpeakers.h
// Copyright (c) 2008 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiVoiceActiveSpeakers_H
#define INCLUDED_SwgCuiVoiceActiveSpeakers_H

// ======================================================================

#include "clientGame/ClientObject.h"
#include "clientUserInterface/CuiVoiceChatManager.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

// ----------------------------------------------------------------------

class UIPage;
class UIText;
class UIList;
class UISliderbar;
class UICheckbox;
class UITable;
class UITableModel;

// ----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiVoiceActiveSpeakers :
	public CuiMediator,
	public UIEventCallback
{

public:

	class TableModel;
	enum SpeakingState
	{
		SS_notSpeaking,
		SS_speaking,
		SS_muted,
	};

	explicit SwgCuiVoiceActiveSpeakers(UIPage & page);
	~SwgCuiVoiceActiveSpeakers();

	virtual void update(float deltaTimeSecs);

	void setSelectedParticipant(CuiVoiceChatManager::ParticipantId const & speaker);

	bool getImageForState(SpeakingState state, UIImageStyle *& value) const;
	bool getTooltipForState(SpeakingState state, UIString & tooltip) const;


protected:
	virtual void performActivate();
	virtual void performDeactivate();

	virtual bool OnMessage(UIWidget *Context, const UIMessage & msg );
	virtual void OnGenericSelectionChanged (UIWidget * context);
	virtual void OnButtonPressed (UIWidget * context);
	virtual void OnSliderbarChanged(UIWidget *context);
	virtual void OnCheckboxSet( UIWidget *context );
	virtual void OnCheckboxUnset( UIWidget *context );
	virtual void OnPopupMenuSelection (UIWidget * context);

	void handleCheckboxChanged( UIWidget *context, bool value);

	int getRowForParticipant(CuiVoiceChatManager::ActiveSpeaker const & speaker);
	int getRowForParticipant(CuiVoiceChatManager::ParticipantId const & speakerId);
	void addParticipantToTable(CuiVoiceChatManager::ActiveSpeaker const & speaker, bool suppressCallback = false);
	bool getParticipantAtPosition(int row, CuiVoiceChatManager::ActiveSpeaker & speaker);
	int getNumParticipants();

	void setCurrentlySelectedParticipant(int logicalRow);
	void updateControls();

	void refreshTable();

	void onParticipantAdded(CuiVoiceChatManager::ActiveSpeaker const & speaker);
	void onParticipantRemoved(CuiVoiceChatManager::ActiveSpeaker const & speaker);
	void onParticipantUpdated(CuiVoiceChatManager::ActiveSpeaker const & speaker);
	void onSessionStarted(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
	void onSessionEnded(CuiVoiceChatManager::VoiceChatChannelData const & channelData);

private:

	MessageDispatch::Callback * m_callback;

	UITable *               m_table;
	TableModel *            m_tableModel;


	UIButton * m_buttonClose;

	bool m_needToRefreshData;

	UIPage * m_controlsPage;
	UISliderbar * m_volumeSlider;
	UICheckbox * m_muteCheckbox;

	std::vector<CuiVoiceChatManager::ParticipantId> m_speakerData;
	int m_currentSelection;

	std::string m_radialTarget;

	std::map<SpeakingState, UIString> m_stateImageMap;
	std::map<SpeakingState, UIString> m_stateTooltipMap;

private: //-- disabled
	SwgCuiVoiceActiveSpeakers();
	SwgCuiVoiceActiveSpeakers(SwgCuiVoiceActiveSpeakers const & rhs);
	SwgCuiVoiceActiveSpeakers & operator= (SwgCuiVoiceActiveSpeakers const & rhs);
};

// ======================================================================

#endif
