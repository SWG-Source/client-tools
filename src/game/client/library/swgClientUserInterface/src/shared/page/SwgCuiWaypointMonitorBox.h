//======================================================================
//
// SwgCuiWaypointMonitorBox.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiWaypointMonitorBox_H
#define INCLUDED_SwgCuiWaypointMonitorBox_H

//======================================================================

class ClientWaypointObject;
class PlayerObject;
class SwgCuiWaypointMonitor;
class UIComposite;
class UIPage;
class UIOpacityEffector;

namespace MessageDispatch
{
	class Callback;
}

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class SwgCuiWaypointMonitorBox :
public CuiMediator,
public UIEventCallback
{
public:
	
	explicit SwgCuiWaypointMonitorBox     (UIPage & page);
	void     onPlayerWaypointsChanged     (const PlayerObject & );
	void     onWaypointChanged            (const ClientWaypointObject & );
	void     reset                        ();
	void     update                       (float);
	bool     OnMessage                    (UIWidget * context, const UIMessage & msg);
	void     OnPopupMenuSelection         (UIWidget * context);
	bool     close                        ();

protected:

	void  performActivate   ();
	void  performDeactivate ();
	
private:

	SwgCuiWaypointMonitorBox (const SwgCuiWaypointMonitorBox & rhs);
	SwgCuiWaypointMonitorBox & operator= (const SwgCuiWaypointMonitorBox & rhs);

	~SwgCuiWaypointMonitorBox ();

private:

	UIComposite *   m_comp;
	UIPage *        m_sample;

	typedef stdvector<SwgCuiWaypointMonitor *>::fwd MonitorVector;
	MonitorVector * m_monitorVector;

	MessageDispatch::Callback * m_callback;
	bool                        m_needsReset;
	UISize                      m_minSize;
	UIOpacityEffector *         m_effectorFadeIn;
};

//======================================================================

#endif
