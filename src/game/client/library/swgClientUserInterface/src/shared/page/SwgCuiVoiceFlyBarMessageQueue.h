#ifndef SwgCuiVoiceFlyBarMessageQueue_h_INCLUDED
#define SwgCuiVoiceFlyBarMessageQueue_h_INCLUDED

#include "clientUserInterface/CuiVoiceChatManager.h"


class UIText;

class FlybarMessage
{
public:

	FlybarMessage(Unicode::String const & text, float minLife, float maxLife, float discardTime);
	FlybarMessage(FlybarMessage const & other);
	FlybarMessage & operator=(FlybarMessage const & rhs);

	virtual void setActive(bool active);
	virtual void update(float dt);
	virtual void reset();
	virtual void performAction() {}

	virtual Unicode::String const & getText() const;
	virtual bool shouldDiscard() const;
	virtual bool canReplace() const;

protected:
	Unicode::String m_text;
	float m_minLifetime;
	float m_maxLifetime;
	float m_discardTime;
	float m_timeInQueue;
	float m_timeActive;
	bool m_active;
};



class ParticipantFlybarMessage : public FlybarMessage
{
public:
	ParticipantFlybarMessage(CuiVoiceChatManager::ParticipantId const & who, float minLife);
	virtual bool shouldDiscard() const;
	virtual void performAction();

protected:
	CuiVoiceChatManager::ParticipantId m_participant;
	static Unicode::String textHelper(CuiVoiceChatManager::ParticipantId const & who);
};


class ChannelAddedFlybarMessage : public FlybarMessage
{
public:
	ChannelAddedFlybarMessage(CuiVoiceChatManager::VoiceChatChannelData const & channelData, float minLife, float maxLife, float discardTime);
	virtual void performAction();

protected:
	std::string m_channelName;
	static Unicode::String textHelper(CuiVoiceChatManager::VoiceChatChannelData const & channelData);
};


class FlybarMessageQueue
{
	typedef std::list<FlybarMessage*> QueueType;
public:

	FlybarMessageQueue(UIText * text);
	~FlybarMessageQueue();

	void setText(UIText * text);
	void clear();
	void enqueue(FlybarMessage * message);
	void enqueue(Unicode::String const & text, float minTime = -1.0f, float maxTime = -1.0f, float discardTime = -1.0f);
	void update(float dt);
	void performActionOnCurrent();
	void resetCurrent();

private:

	void updateText();

	FlybarMessageQueue(FlybarMessageQueue const & other);
	FlybarMessageQueue & operator=(FlybarMessageQueue const& rhs);

	QueueType m_queue;
	UIText * m_text;
};


#endif
