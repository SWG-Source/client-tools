//======================================================================
//
// SwgCuiMissionBrowser.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiMissionBrowser_H
#define INCLUDED_SwgCuiMissionBrowser_H

//======================================================================

#include "sharedMessageDispatch/Receiver.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class UIPage;
class UIButton;
class UITabbedPane;
class CuiMessageBox;
class UITable;
class MessageQueueMissionGenericResponse;
class PopulateMissionBrowserMessage;
class MessageQueueMissionDetailsResponse;
class SwgCuiMissionDetails;
class MessageQueueMissionListResponseData;
class CuiWorkspace;
class ClientMissionObject;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiMissionBrowser :
public CuiMediator,
public UIEventCallback
{
public:

	class TableModel;

	explicit                SwgCuiMissionBrowser (UIPage & page);

	virtual void            performActivate   ();
	virtual void            performDeactivate ();

	virtual bool            OnMessage( UIWidget *Context, const UIMessage & msg );
	virtual void            OnButtonPressed( UIWidget *context );
	virtual void            OnTabbedPaneChanged (UIWidget * context);

	void                    onReceiveList    (const PopulateMissionBrowserMessage &);
	void                    onReceiveDetails (const MessageQueueMissionDetailsResponse &);
	void                    onMissionAccept  (const MessageQueueMissionGenericResponse &);

	void                    receiveMessage (const MessageDispatch::Emitter& , const MessageDispatch::MessageBase& message);

	void                    refreshList        ();

private:
	                       ~SwgCuiMissionBrowser ();
	                        SwgCuiMissionBrowser (const SwgCuiMissionBrowser &);
	SwgCuiMissionBrowser &  operator= (const SwgCuiMissionBrowser &);


	void onClientMissionObjectReadyForDisplay (const ClientMissionObject & creature);
	void onClientMissionObjectRemovedFromBrowser (const ClientMissionObject & creature);

	void                    acceptMission      () const;
	void                    deleteMission      () const;
	void                    viewMission        ();
	void                    displayNoSelection () const;

	UITable *               m_table;

	UIButton *              m_buttonExit;
	UIButton *              m_buttonRefresh;
	UIButton *              m_buttonAccept;
	UIButton *              m_buttonDelete;
	UIButton *              m_buttonDetails;
	UITabbedPane *          m_tabbedPane;

	MessageDispatch::Callback *  m_callback;

	TableModel *            m_tableModel;

	int                     m_requestListType;

	int                     m_currentTab;

	SwgCuiMissionDetails *  m_details;

	MessageQueueMissionListResponseData * m_detailsRequestListResponse;
	NetworkId               m_terminal;

	typedef stdvector<NetworkId>::fwd NetworkIdVector;
	typedef stdvector<const ClientMissionObject*>::fwd ClientMissionObjectVector;
};

//======================================================================

#endif
