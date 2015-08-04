//======================================================================
//
// CuiConversationMenu.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiConversationMenu_H
#define INCLUDED_CuiConversationMenu_H

//======================================================================

#include "UIEventCallback.h"
#include "UINotification.h"
#include "UITypes.h"

class UIPage;
class UIText;
class UIPopupMenu;
class Camera;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class CuiConversationMenu :
public UIEventCallback,
public UINotification
{
public:
	explicit       CuiConversationMenu (UIPage & parent);
	              ~CuiConversationMenu ();

	void           OnPopupMenuSelection (UIWidget * context);
	void           Notify               (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	bool           OnMessage            (UIWidget * context, const UIMessage & msg);

	void           onResponsesChanged (const bool &);
	void           onTargetChanged    (const bool &);

	bool           updatePosition (const Camera & camera);
	bool           isValid () const;

	bool isClientOnlyMode() const;
	void setCloseTimer(bool enabled, float timer);
	void onConversationEnded(bool const & b);

private:	

	void           setValid   (bool b);
	void           setVisible (bool b);

	void           updateResponses ();

	void requestPointer(bool pointer);

	UIPage       * m_frame;
	UIText       * m_text;
	UIPopupMenu  * m_popup;

	bool           m_valid;

	bool           m_responsesDirty;
	bool           m_targetDirty;

	MessageDispatch::Callback * m_callback;

	UIFloatPoint   m_lastHeadScreenPoint;
	UIFloatPoint   m_lastDesiredHeadScreenPoint;
	bool           m_headPointsInitialized;
	float          m_lastUpdateSecs;

	bool           m_canSee;
	bool m_requestPointer;
};

//----------------------------------------------------------------------

inline bool CuiConversationMenu::isValid () const
{
	return m_valid;
}

//======================================================================

#endif
