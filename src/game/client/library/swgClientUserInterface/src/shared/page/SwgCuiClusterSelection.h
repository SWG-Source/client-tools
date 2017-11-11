// ======================================================================
//
// SwgCuiClusterSelection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiClusterSelection_H
#define INCLUDED_SwgCuiClusterSelection_H

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"

class UIPage;
class UIButton;
class CuiMessageBox;
class UITableModelDefault;
class UITable;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiClusterSelection :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	explicit                 SwgCuiClusterSelection    (UIPage & page);

	void                     OnButtonPressed           (UIWidget *context);
	void                     OnGenericSelectionChanged (UIWidget *context);

	void                     receiveMessage            (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void                     update                    (float deltaTimeSecs);

	void                     onClusterConnection       (bool b);
	void                     onClusterStatusChanged    (bool b);

	bool                     OnMessage                 (UIWidget * context, const UIMessage & msg);

protected:
	void                     performActivate           ();
	void                     performDeactivate         ();

	void                     clearPerformancePages ();

private:
	                        ~SwgCuiClusterSelection ();
	                         SwgCuiClusterSelection (const SwgCuiClusterSelection & rhs);
	SwgCuiClusterSelection & operator=            (const SwgCuiClusterSelection & rhs);

	void                     refreshList ();
	void                     updateServerStatus ();

private:

	UIButton *                                   m_okButton;
	UIButton *                                   m_cancelButton;
	UIButton *                                   m_changeGalaxy;
	UIPage *                                     m_main;
	UIPage *                                     m_main2;
	UIText *                                     m_recommendedGalaxy;

	CuiMessageBox *                              m_messageBox;

	bool                                         m_autoConnected;

	UITable *                                    m_table;
	UITableModelDefault *                        m_model;

	UIPage *                                     m_sampleBar;

	typedef stdvector<UIPage *>::fwd             PageVector;
	PageVector *                                 m_performancePages;

	bool                                         m_proceed;

	MessageDispatch::Callback *                  m_callback;

	bool                                         m_waitingForConnection;
	bool                                         m_dropFromCluster;
	uint32                                       m_waitingForClusterId;

	float                                        m_timeElapsed;
	bool                                         m_hasRecommended;
};

// ======================================================================

#endif
