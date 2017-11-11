//======================================================================
//
// SwgCuiVoiceFlyBar.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPopupMenu.h"
#include "UISliderbar.h"
#include "UITable.h"

#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "ClientUserInterface/CuiVoiceChatManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers_TableModel.h" 


namespace SwgCuiVoiceActiveSpeakersNamespace
{
	namespace PopupIds
	{
		const std::string channel_kick = "channel_kick";
	}

	const char * buttonClose = "buttonclose";
	const char * controlsPage = "controlspage";
	const char * sliderVolume = "slidervolume";
	const char * checkboxMute = "checkboxmuted";

	const char * iconSpeaking = "iconspeaking";
	const char * iconNotSpeaking = "iconnotspeaking";
	const char * iconMuted = "iconlocalmute";

	inline int ratioToPercent(float r)
	{
		return clamp(0, static_cast<int>(100.0f * r), 100);
	}

	inline float percentToRatio(int p)
	{
		return clamp(0.0f, static_cast<float>(p) / 100.0f, 1.0f);
	}

}

using namespace SwgCuiVoiceActiveSpeakersNamespace;

SwgCuiVoiceActiveSpeakers::SwgCuiVoiceActiveSpeakers(UIPage & page)
: CuiMediator("SwgCuiVoiceActiveSpeakers", page),
  UIEventCallback (),
  m_callback (new MessageDispatch::Callback),
  m_table (0),
  m_tableModel (0),
  m_buttonClose (0),
  m_needToRefreshData (false),
  m_controlsPage (0),
  m_volumeSlider (0),
  m_muteCheckbox (0),
  m_speakerData (),
  m_currentSelection (-1)
{
	getCodeDataObject (TUIButton,  m_buttonClose, buttonClose);
	getCodeDataObject (TUIPage, m_controlsPage, controlsPage);
	getCodeDataObject (TUISliderbar, m_volumeSlider, sliderVolume);
	getCodeDataObject (TUICheckbox, m_muteCheckbox, checkboxMute);
	getCodeDataObject (TUITable, m_table, "table");

	UIString temptext;
	if(getCodeDataString(iconSpeaking, temptext))
	{
		m_stateImageMap[SS_speaking] = temptext;
	}
	if(getCodeDataString(iconNotSpeaking, temptext))
	{
		m_stateImageMap[SS_notSpeaking] = temptext;
	}
	if(getCodeDataString(iconMuted, temptext))
	{
		m_stateImageMap[SS_muted] = temptext;
	}

	m_stateTooltipMap[SS_speaking] = StringId("ui_voice", "activespeakers_state_speaking").localize();
	m_stateTooltipMap[SS_notSpeaking] = StringId("ui_voice", "activespeakers_state_listening").localize();
	m_stateTooltipMap[SS_muted] = StringId("ui_voice", "activespeakers_state_muted").localize();

	//start voodoo incantation
	m_tableModel = new TableModel(*this);
	m_tableModel->Attach (0);
	m_tableModel->SetName ("ActiveSpeakersTable");
	getPage ().AddChild (m_tableModel);
	m_table->SetTableModel (m_tableModel);
	m_tableModel->updateTableColumnSizes (*m_table);
	m_tableModel->fireColumnsChanged ();
	m_tableModel->fireDataChanged ();
	//end voodoo incantation

}

//----------------------------------------------------------------------

SwgCuiVoiceActiveSpeakers::~SwgCuiVoiceActiveSpeakers ()
{
	m_tableModel->Detach(0);
	m_tableModel = 0;

	m_controlsPage = 0;
	m_muteCheckbox = 0;
	m_volumeSlider = 0;
	m_table = 0;
	m_buttonClose = 0;
	delete m_callback;
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::performActivate   ()
{
	m_callback->connect (*this, &SwgCuiVoiceActiveSpeakers::onSessionStarted, static_cast<CuiVoiceChatManager::Messages::SessionStarted*>(0));
	m_callback->connect (*this, &SwgCuiVoiceActiveSpeakers::onSessionEnded, static_cast<CuiVoiceChatManager::Messages::SessionEnded*>(0));
	m_callback->connect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantAdded, static_cast<CuiVoiceChatManager::Messages::ParticipantAdded*>(0));
	m_callback->connect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantRemoved, static_cast<CuiVoiceChatManager::Messages::ParticipantRemoved*>(0));
	m_callback->connect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantUpdated, static_cast<CuiVoiceChatManager::Messages::ParticipantUpdated*>(0));

	m_volumeSlider->SetUpperLimit (75);
	m_volumeSlider->SetLowerLimit (0);
	m_volumeSlider->SetValue (0, false);

	m_muteCheckbox->SetChecked (false, false);

	refreshTable ();
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantUpdated, static_cast<CuiVoiceChatManager::Messages::ParticipantUpdated*>(0));
	m_callback->disconnect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantRemoved, static_cast<CuiVoiceChatManager::Messages::ParticipantRemoved*>(0));
	m_callback->disconnect (*this, &SwgCuiVoiceActiveSpeakers::onParticipantAdded, static_cast<CuiVoiceChatManager::Messages::ParticipantAdded*>(0));
	m_callback->disconnect (*this, &SwgCuiVoiceActiveSpeakers::onSessionEnded, static_cast<CuiVoiceChatManager::Messages::SessionEnded*>(0));
	m_callback->disconnect (*this, &SwgCuiVoiceActiveSpeakers::onSessionStarted, static_cast<CuiVoiceChatManager::Messages::SessionStarted*>(0));
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::update(float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
	
	if(m_needToRefreshData)
	{
		refreshTable ();
		m_needToRefreshData = false;
		setIsUpdating (false);
	}
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::OnMessage(UIWidget *context, const UIMessage & msg )
{
	if(context == m_table && msg.Type == UIMessage::RightMouseUp)
	{
		long row, col;
		if(m_table->GetCellFromPoint(msg.MouseCoords, &row, &col))
		{
			int logicalRow = m_tableModel->getLogicalRowFromVisualRow (row);
			CuiVoiceChatManager::ActiveSpeaker data;
			if(getParticipantAtPosition (logicalRow, data) && !CuiVoiceChatManager::isParticipantMe (data.id))
			{
				if(CuiVoiceChatManager::doIHaveModeratorPowersInChannel(data.channelName))
				{
					m_radialTarget = data.displayName;

					UIPopupMenu * const pop = new UIPopupMenu(&getPage()); 
					pop->SetStyle(getPage().FindPopupStyle());

					pop->AddItem(SwgCuiVoiceActiveSpeakersNamespace::PopupIds::channel_kick, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::VOICE_KICK, 0));

					pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
					UIManager::gUIManager().PushContextWidget(*pop);
					pop->AddCallback(this);				
				}
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnPopupMenuSelection (UIWidget * context)
{
	if (!context || !context->IsA(TUIPopupMenu))
		return;

	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	if (!pop)
		return;

	if(m_radialTarget.empty())
		return;

	const std::string & selection = pop->GetSelectedName();

	//this is the only possible selection at this point...
	if (selection == SwgCuiVoiceActiveSpeakersNamespace::PopupIds::channel_kick)
	{
		CuiVoiceChatManager::channelKick(NetworkId(), m_radialTarget, CuiVoiceChatManager::getMyPrivateChannelName());
	}

	m_radialTarget = "";
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnButtonPressed (UIWidget *context)
{
	if (context == m_buttonClose)
	{
		deactivate ();
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnSliderbarChanged(UIWidget *context)
{
	if(context == m_volumeSlider)
	{
		int volume = m_volumeSlider->GetValue ();

		CuiVoiceChatManager::ActiveSpeaker data;
		if(getParticipantAtPosition (m_currentSelection, data))
		{
			CuiVoiceChatManager::setLocalParticipantVolume (data.id, percentToRatio (volume));
		}
		else
		{
			DEBUG_WARNING(true,("Sliderbar used with invalid selection %d", m_currentSelection));
			m_controlsPage->SetEnabled (false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnCheckboxSet( UIWidget *context )
{
	handleCheckboxChanged(context, true);
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnCheckboxUnset( UIWidget *context )
{
	handleCheckboxChanged(context, false);
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::handleCheckboxChanged( UIWidget *context, bool value)
{
	if(context == m_muteCheckbox)
	{
		CuiVoiceChatManager::ActiveSpeaker data;
		if(getParticipantAtPosition (m_currentSelection, data))
		{
			if(!CuiVoiceChatManager::setLocalParticipantMute (data.id, value))
			{
				m_muteCheckbox->SetChecked(!value, false);
			}
		}
		else
		{
			DEBUG_WARNING(true,("Checkbox used with invalid selection %d", m_currentSelection));
			m_controlsPage->SetEnabled (false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::OnGenericSelectionChanged (UIWidget * context)
{
	if(context == m_table)
	{
		int visualrow = m_table->GetLastSelectedRow ();
		int logicalRow = m_tableModel->getLogicalRowFromVisualRow (visualrow);

		setCurrentlySelectedParticipant (logicalRow);
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::setSelectedParticipant(CuiVoiceChatManager::ParticipantId const & speakerId)
{
	if(speakerId.isValid())
	{
		int logicalRow = getRowForParticipant (speakerId);
		m_table->SelectRow (m_tableModel->GetVisualDataRowIndex (logicalRow));
	}
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::setCurrentlySelectedParticipant(int logicalRow)
{
	m_currentSelection = logicalRow;
	updateControls ();
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::updateControls()
{
	CuiVoiceChatManager::ActiveSpeaker data;
	if(getParticipantAtPosition (m_currentSelection, data) && !CuiVoiceChatManager::isParticipantMe (data.id))
	{
		m_controlsPage->SetEnabled (true);
		m_volumeSlider->SetValue (ratioToPercent (data.volume), false);
		m_muteCheckbox->SetChecked (data.muted, false);
	}
	else
	{
		m_controlsPage->SetEnabled (false);
		m_volumeSlider->SetValue (0, false);
		m_muteCheckbox->SetChecked (false, false);
	}
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::getRowForParticipant(CuiVoiceChatManager::ActiveSpeaker const & speaker)
{
	return getRowForParticipant (speaker.id);
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::getRowForParticipant(CuiVoiceChatManager::ParticipantId const & speakerId)
{
	int numParticipants = getNumParticipants();
	for(int i = 0; i < numParticipants; ++i)
	{
		if(speakerId == m_speakerData[i])
		{
			return i;
		}
	}

	return -1;
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::addParticipantToTable(CuiVoiceChatManager::ActiveSpeaker const & speaker, bool suppressCallback)
{
	TableModel* tableModel = safe_cast<TableModel*>(m_table->GetTableModel ());

	//TODO: be smarter about mutlitple adds
	m_speakerData.push_back (speaker.id);

	if(!suppressCallback)
	{
		tableModel->fireDataChanged ();
		tableModel->fireColumnsChanged ();
	}
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::getParticipantAtPosition(int index, CuiVoiceChatManager::ActiveSpeaker & speaker)
{
	if(index < 0 || index >= getNumParticipants ())
	{
		return false;
	}

	return CuiVoiceChatManager::getParticipantData(m_speakerData[index], speaker);
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::getNumParticipants()
{
	return static_cast<uint32>(m_speakerData.size ());
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::refreshTable()
{
	UITableModel* tableModel = m_table->GetTableModel ();
	NOT_NULL(tableModel);

	typedef std::list<CuiVoiceChatManager::ActiveSpeaker> SpeakerList;
	SpeakerList allParticipants;
	CuiVoiceChatManager::getAllParticipants (allParticipants);

	CuiVoiceChatManager::ActiveSpeaker data;
	bool hadSelection = getParticipantAtPosition (m_currentSelection,data);
	int newSelection = -1;

	m_speakerData.clear ();
	int x = 0;
	for(SpeakerList::iterator i = allParticipants.begin(); i != allParticipants.end(); ++i, ++x)
	{
		addParticipantToTable (*i, true);
		if(hadSelection && data.isSameSpeaker (*i))
		{
			newSelection = x;
		}
	}

	tableModel->fireDataChanged ();
	tableModel->fireColumnsChanged ();


	m_currentSelection = newSelection;
	m_table->SelectRow (tableModel->GetVisualDataRowIndex (m_currentSelection));
	updateControls();
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::onParticipantAdded(CuiVoiceChatManager::ActiveSpeaker const & speaker)
{

	UITableModel* tableModel = m_table->GetTableModel();

	int row = getRowForParticipant (speaker);

	//already in there
	if(row >= 0)
	{
		tableModel->fireDataChanged ();
		tableModel->fireColumnsChanged ();
	}
	else
	{
		addParticipantToTable (speaker);
	}

	if(row == m_currentSelection)
	{
		updateControls ();
	}

}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::onParticipantRemoved(CuiVoiceChatManager::ActiveSpeaker const & speaker)
{
	int row = getRowForParticipant(speaker);

	if(row >= 0)
	{
		// we can't remove rows yet...

		m_needToRefreshData = true;
		setIsUpdating (true);
	}

}

void SwgCuiVoiceActiveSpeakers::onParticipantUpdated(CuiVoiceChatManager::ActiveSpeaker const & speaker)
{
	onParticipantAdded (speaker);
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::onSessionStarted(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	UNREF(channelData);
	m_needToRefreshData = true;
	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::onSessionEnded(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	UNREF(channelData);
	m_needToRefreshData = true;
	setIsUpdating (true);
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::getImageForState(SpeakingState state, UIImageStyle *& value) const
{
	std::map<SpeakingState,UIString>::const_iterator i = m_stateImageMap.find(state);
	if(i != m_stateImageMap.end())
	{
		UIString imagePath = i->second;
		value = static_cast<UIImageStyle *>(getPage().GetObjectFromPath (imagePath, TUIImageStyle));

		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::getTooltipForState(SpeakingState state, UIString & tooltip) const
{
	std::map<SpeakingState,UIString>::const_iterator i = m_stateTooltipMap.find(state);
	if(i != m_stateTooltipMap.end())
	{
		tooltip = i->second;

		return true;
	}
	return false;
}


