// ======================================================================
//
// SwgCuiPermissionList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef SwgCuiPermissionList_H
#define SwgCuiPermissionList_H

//-----------------------------------------------------------------

#include "sharedMessageDispatch/Receiver.h"

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"
#include "UINotification.h"

#include <vector>

//-----------------------------------------------------------------

class PermissionListCreateMessage;
class UIButton;
class UIComboBox;
class UIList;
class UIPage;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiPermissionList
: public CuiMediator,
  public UIEventCallback,
  public UINotification,
  public MessageDispatch::Receiver
{
public:
	static SwgCuiPermissionList* createInto              (UIPage & parent);

public:
	explicit                   SwgCuiPermissionList      (UIPage & page);
	void                       Notify                    (UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode);
	void                       receiveMessage            (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                       setData                   (const PermissionListCreateMessage& msg);

protected:
	void                       performActivate           ();
	void                       performDeactivate         ();
	bool                       close                     ();
	void                       OnButtonPressed           (UIWidget *context);
	void                       OnGenericSelectionChanged (UIWidget * context);

private:
	                          ~SwgCuiPermissionList      ();
	                           SwgCuiPermissionList      (const SwgCuiPermissionList &);
	SwgCuiPermissionList&      operator=                 (const SwgCuiPermissionList &);

private:
	UIText*                    m_titleText;
	UIList*                    m_memberList;
	Unicode::String            m_currentName;
	Unicode::String            m_listName;
	UIButton*                  m_addButton;
	UIButton*                  m_removeButton;
	UIButton*                  m_closeButton;

	UIPage*                    m_addPage;
	UIButton*                  m_addOk;
	UIButton*                  m_addCancel;
	UIComboBox*                m_addCombo;
	std::vector<Unicode::String> m_nearbyPeople;
};

//-----------------------------------------------------------------

#endif

