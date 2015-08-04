//======================================================================
//
// SwgCuiNetStatus.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiNetStatus_H
#define INCLUDED_SwgCuiNetStatus_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "sharedMessageDispatch/Receiver.h"

class UIText;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiNetStatus :
public SwgCuiLockableMediator,
public MessageDispatch::Receiver
{
public:

	explicit                 SwgCuiNetStatus               (UIPage & page);
	void                     update                        (float deltaTimeSecs);
	virtual void             receiveMessage                (const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:

	                       ~SwgCuiNetStatus ();
	SwgCuiNetStatus ();
	SwgCuiNetStatus (const SwgCuiNetStatus & rhs);
	SwgCuiNetStatus & operator= (const SwgCuiNetStatus & rhs);

	UIText *                           m_textPing;
	UIText *                           m_textPacketLoss;
	UIText *                           m_textBandwidth;
	UIText *                           m_textFps;
	UIText *                           m_textActivity;

	int                                m_cachedPing;
	int                                m_cachedPacketLoss;
	int                                m_cachedFps;

	float                              m_elapsedUpdate;

	bool                               m_performanceClear;

	time_t                             m_timeToRequestGalaxyLoopTime;

	void updateBandwidth();
};

//======================================================================

#endif
