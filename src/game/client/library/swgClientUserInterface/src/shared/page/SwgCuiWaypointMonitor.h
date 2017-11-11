//======================================================================
//
// SwgCuiWaypointMonitor.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiWaypointMonitor_H
#define INCLUDED_SwgCuiWaypointMonitor_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIText;
class UIImage;
class PlayerObject;
class ClientWaypointObject;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiWaypointMonitor :
public CuiMediator,
public UIEventCallback
{
public:

	                          SwgCuiWaypointMonitor    (UIPage & page, const NetworkId & waypointId);
	bool                      close                    ();
	bool                      OnMessage                (UIWidget * context, const UIMessage & msg);
	void                      update                   (float deltaTimeSecs);
	void                      refresh                  (float elapsedTimeToUse);
	void                      setTarget                (const NetworkId & waypointId);
	const NetworkId &         getTarget                () const;

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:
	                        ~SwgCuiWaypointMonitor ();
	SwgCuiWaypointMonitor ();
	SwgCuiWaypointMonitor (const SwgCuiWaypointMonitor & rhs);
	SwgCuiWaypointMonitor & operator= (const SwgCuiWaypointMonitor & rhs);

	UIText *                    m_textName;
	UIText *                    m_textDistance;
	UIImage *                   m_arrow;
	UIImage *                   m_waypointMarker;

	MessageDispatch::Callback * m_callback;

	NetworkId *                 m_waypointId;

	int                         m_lastDistance;

	float                       m_elapsedTime;
};

//----------------------------------------------------------------------

inline const NetworkId & SwgCuiWaypointMonitor::getTarget                () const
{
	return *m_waypointId;
}

//======================================================================

#endif
