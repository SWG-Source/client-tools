//======================================================================
//
// CuiConsent.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiConsent_H
#define INCLUDED_CuiConsent_H

//======================================================================

/** This widget is a resusable "ask a question, get a yes/no answer" widget.  
    It will most often be used to ask consent of the player.  This widget is 
    specifically for the server to ask a question, and get a response.  Corpse 
    loot consent, image designer consent, and other	non-modal type questions.  
    The widget hides in the corner of the screen until dealt with, to help avoid 
    griefing by consent-asking players.
*/

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

class UIButton;
class UIPage;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------

class CuiConsent : public CuiMediator, public UIEventCallback
{
public:
	explicit                CuiConsent     (UIPage & page);
	void                    performActivate   ();
	void                    performDeactivate ();
	void                    OnButtonPressed   (UIWidget *context);
	void                    setQuestion       (const Unicode::String& question);
	void                    setClientOnly     (bool clientOnly);
	int                     getId             ();
	void                    setId             (int id);
	bool                    close             ();

public:
	static CuiConsent *  createInto (UIPage & parent);

private:
	//disabled
	                       ~CuiConsent ();
	                        CuiConsent (const CuiConsent & rhs);
	                        CuiConsent & operator= (const CuiConsent & rhs);

private:
	UIText*                    m_questionText; 
	UIButton*                  m_yesButton; 
	UIButton*                  m_noButton; 
	MessageDispatch::Callback* m_callback;
	Unicode::String            m_question;
	int                        m_id;
	bool                       m_responseSent;
	bool                       m_clientOnly;
};

//======================================================================

#endif
