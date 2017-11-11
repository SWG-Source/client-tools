//======================================================================
//
// SwgCuiCraftSummary.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftSummary_H
#define INCLUDED_SwgCuiCraftSummary_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "UINotification.h"

class UIPage;
class UIButton;
class SwgCuiInventoryInfo;
class UITable;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftSummary :
public CuiMediator,
public UIEventCallback,
public UINotification
{
public:

	explicit                    SwgCuiCraftSummary (UIPage & page);

	virtual void                performActivate    ();
	virtual void                performDeactivate  ();

	virtual void                OnButtonPressed    (UIWidget *context );

	virtual void                Notify             (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	bool                        close              ();

private:
	                           ~SwgCuiCraftSummary ();
	                            SwgCuiCraftSummary (const SwgCuiCraftSummary &);
	SwgCuiCraftSummary &        operator=          (const SwgCuiCraftSummary &);

	void                        onSessionEnded            (const bool &);

	UIButton *                  m_buttonNext;
	bool                        m_transition;

	SwgCuiInventoryInfo *       m_mediatorInfo;
	UITable *                   m_table;
	UIPage *                    m_pageSampleBar;

	typedef stdvector<UIPage *>::fwd PageVector;
	PageVector *                m_bars;

	MessageDispatch::Callback *   m_callback;
	UIText *                      m_textSuccess;

	bool                          m_sessionEnded;
};

//======================================================================

#endif
