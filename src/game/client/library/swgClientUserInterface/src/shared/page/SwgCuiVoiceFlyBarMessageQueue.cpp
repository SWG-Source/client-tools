


#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "SwgCuiVoiceFlyBarMessageQueue.h"

#include "UIText.h"

#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "ClientUserInterface/CuiVoiceChatManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers.h"


FlybarMessage::FlybarMessage(Unicode::String const & text, float minLife, float maxLife, float discardTime)
:m_text(text), m_minLifetime(minLife), m_maxLifetime(maxLife), m_discardTime(discardTime),
m_timeInQueue(0.0f), m_timeActive(0.0f), m_active(false)
{
}

//----------------------------------------------------------------------

FlybarMessage::FlybarMessage(FlybarMessage const & other)
:m_text(other.m_text),
m_minLifetime(other.m_minLifetime),
m_maxLifetime(other.m_maxLifetime),
m_discardTime(other.m_discardTime),
m_timeInQueue(other.m_timeInQueue),
m_timeActive(other.m_timeActive),
m_active(other.m_active)
{
}

//----------------------------------------------------------------------

FlybarMessage& FlybarMessage::operator=(FlybarMessage const & rhs)
{
	m_text = rhs.m_text;
	m_minLifetime = rhs.m_minLifetime;
	m_maxLifetime = rhs.m_maxLifetime;
	m_discardTime = rhs.m_discardTime;
	m_timeInQueue = rhs.m_timeInQueue;
	m_timeActive = rhs.m_timeActive;
	m_active = rhs.m_active;

	return *this;
}

//----------------------------------------------------------------------

void FlybarMessage::setActive(bool active)
{
	m_active = active;
}	

//----------------------------------------------------------------------

void FlybarMessage::update(float dt)
{
	m_timeInQueue += dt;
	if(m_active)
	{
		m_timeActive += dt;
	}
}

//----------------------------------------------------------------------

void FlybarMessage::reset()
{
	m_timeInQueue = 0.0f;
	m_timeActive = 0.0f;
	m_active = false;
}

//----------------------------------------------------------------------

Unicode::String const & FlybarMessage::getText() const
{
	return m_text;
}

//----------------------------------------------------------------------

bool FlybarMessage::shouldDiscard() const
{
	if(m_active)
	{
		return m_maxLifetime > 0.0f && m_timeActive > m_maxLifetime;
	}
	else
	{
		return m_discardTime > 0.0f && m_timeInQueue > m_discardTime;
	}
}

//----------------------------------------------------------------------

bool FlybarMessage::canReplace() const
{
	return m_active && (m_minLifetime <= 0.0f || m_timeActive > m_minLifetime);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

ParticipantFlybarMessage::ParticipantFlybarMessage(CuiVoiceChatManager::ParticipantId const & who, float minLife)
: FlybarMessage(textHelper(who), minLife, -1.0f, -1.0f),
m_participant(who)
{
}

//----------------------------------------------------------------------

bool ParticipantFlybarMessage::shouldDiscard() const
{
	if(!m_active && m_timeActive < m_minLifetime)
	{
		return false;
	}
	else
	{
		return m_participant != CuiVoiceChatManager::getCurrentActiveSpeakerId();
	}
}

//----------------------------------------------------------------------

void ParticipantFlybarMessage::performAction()
{
	if(m_participant.isValid())
	{
		//activate the active speakers page and select the current active speaker
		CuiMediator * med = CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_VoiceActiveSpeakers);
		if(med)
		{
			SwgCuiVoiceActiveSpeakers *const activeSpeakers = static_cast<SwgCuiVoiceActiveSpeakers*>(med);
			activeSpeakers->setSelectedParticipant (m_participant);
		}
	}
}

//----------------------------------------------------------------------

Unicode::String ParticipantFlybarMessage::textHelper(CuiVoiceChatManager::ParticipantId const & who)
{
	CuiVoiceChatManager::ActiveSpeaker currentSpeaker;
	if(CuiVoiceChatManager::getParticipantData(who, currentSpeaker))
	{
		std::string displayName;
		if(!CuiVoiceChatManager::getChannelDisplayName(currentSpeaker.channelName, displayName))
		{
			displayName = currentSpeaker.channelName;
		}

		std::string text = currentSpeaker.displayName + "(" + displayName + ")";
		return Unicode::narrowToWide(text);
	}
	return Unicode::String();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

ChannelAddedFlybarMessage::ChannelAddedFlybarMessage(CuiVoiceChatManager::VoiceChatChannelData const & channelData, float minLife, float maxLife, float discardTime)
: FlybarMessage(textHelper(channelData), minLife, maxLife, discardTime),
  m_channelName(channelData.name)
{
}

void ChannelAddedFlybarMessage::performAction()
{
	CuiVoiceChatManager::setChannelSimpleMode (m_channelName);
}

Unicode::String ChannelAddedFlybarMessage::textHelper(CuiVoiceChatManager::VoiceChatChannelData const & channelData)
{
	static StringId const channelAddedString ("ui_voice", "flybar_shortlist_added");

	Unicode::String flybarMessage;
	CuiStringVariablesManager::process (channelAddedString, Unicode::narrowToWide(channelData.displayName), Unicode::String (), Unicode::String (), flybarMessage);
	
	return flybarMessage;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

FlybarMessageQueue::FlybarMessageQueue(UIText * text)
:m_queue(), m_text(text)
{
}

//----------------------------------------------------------------------

FlybarMessageQueue::~FlybarMessageQueue()
{
	for(QueueType::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
	{
		delete *i;
		*i = NULL;
	}
	m_queue.clear();
	m_text = NULL;
}

//----------------------------------------------------------------------

void FlybarMessageQueue::setText(UIText * text)
{
	m_text = text;
	updateText();
}

//----------------------------------------------------------------------

void FlybarMessageQueue::clear()
{
	for(QueueType::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
	{
		delete *i;
		*i = NULL;
	}
	m_queue.clear();

	if(m_text)
		m_text->Clear();
}

//----------------------------------------------------------------------

void FlybarMessageQueue::enqueue(FlybarMessage * message)
{
	NOT_NULL(message);
	m_queue.push_back( message );
	updateText();
}

//----------------------------------------------------------------------

void FlybarMessageQueue::enqueue(Unicode::String const & text, float minTime, float maxTime, float discardTime)
{
	enqueue( new FlybarMessage(text, minTime, maxTime, discardTime));
}

//----------------------------------------------------------------------

void FlybarMessageQueue::update(float dt)
{
	for(QueueType::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
	{
		FlybarMessage * msg = *i;

		msg->update(dt);

		//remove items that have expired or timed out
		if(msg->shouldDiscard())
		{
			delete msg;
			i = m_queue.erase(i);
		}
	}

	updateText();
}

//----------------------------------------------------------------------

void FlybarMessageQueue::performActionOnCurrent()
{
	QueueType::iterator front = m_queue.begin();
	if(front != m_queue.end())
	{
		(*front)->performAction();
	}
}

//----------------------------------------------------------------------

void FlybarMessageQueue::resetCurrent()
{
	QueueType::iterator front = m_queue.begin();
	if(front != m_queue.end())
	{
		(*front)->reset();
		(*front)->setActive(true);
	}
}

//----------------------------------------------------------------------

void FlybarMessageQueue::updateText()
{
	QueueType::iterator front = m_queue.begin();
	if(front != m_queue.end()) //there is at least one item in the queue
	{	
		{
			QueueType::iterator second = m_queue.begin();
			++second;
			if(second != m_queue.end())
			{	//there is someone waiting in line

				if((*front)->canReplace())
				{
					delete *front;
					m_queue.pop_front();
					front = m_queue.begin();
				}
			}
		}

		(*front)->setActive(true);

		if(m_text)
			m_text->SetText((*front)->getText());
	}
	else
	{
		if(m_text)
			m_text->Clear();
	}
}

