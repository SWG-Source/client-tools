//======================================================================
//
// SwgCuiCredits.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCredits_H
#define INCLUDED_SwgCuiCredits_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIComposite;
class UIText;

//----------------------------------------------------------------------

class SwgCuiCredits : 
public CuiMediator,
public UIEventCallback
{

public:

	explicit                 SwgCuiCredits        (UIPage & page);

	virtual bool             OnMessage            (UIWidget *Context, const UIMessage & msg);

	virtual void             performActivate      ();
	virtual void             performDeactivate    ();

	void                     populate             ();

	void                     update               (float deltaTimeSecs);

private:
	virtual                 ~SwgCuiCredits            ();
	                         SwgCuiCredits            ();
	                         SwgCuiCredits            (const SwgCuiCredits & rhs);
	SwgCuiCredits &          operator=                (const SwgCuiCredits & rhs);

private:
	
	UIComposite *            m_comp;
	UIText *                 m_textSampleHeader;
	UIText *                 m_textSampleNames;
	UIText *                 m_textSampleMain;
	UIText *                 m_textSampleCompany;
	UIText *                 m_textSampleTitle;
	UIText *                 m_textSampleText;
	UIPage *                 m_pageSampleSpacer;

	float                    m_elapsedTime;
};

//======================================================================

#endif
