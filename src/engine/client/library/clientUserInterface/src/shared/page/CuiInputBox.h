//======================================================================
//
// CuiInputBox.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiInputBox_H
#define INCLUDED_CuiInputBox_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------

class UIButton;
class UIPage;
class UIText;
class UITextbox;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------

class CuiInputBox : public CuiMediator, public UIEventCallback
{
public:
	typedef void (*CallbackFunction)(bool response, const Unicode::String & text, const NetworkId& object);

public:
	explicit                CuiInputBox       (UIPage & page);
	virtual                 ~CuiInputBox      ();

	virtual void            OnButtonPressed   (UIWidget *context);
	virtual bool            close             ();

	void                    setQuestionText   (const Unicode::String& question);
	void                    setAnswerText     (const Unicode::String& defaultAnswer);
	void                    setCallbackFunc   (CallbackFunction);
	void                    setObject         (const NetworkId& object);
	NetworkId const &       getObject         () const;

protected:

	virtual void            performActivate   ();
	virtual void            performDeactivate ();

	virtual void            handleResult (bool affirmative, Unicode::String const & s);

	//disabled
	                        CuiInputBox (const CuiInputBox & rhs);
	                        CuiInputBox & operator= (const CuiInputBox & rhs);

private:
	UIText*                    m_questionText;
	UITextbox*                 m_answerText;
	UIButton*                  m_okayButton;
	UIButton*                  m_cancelButton;
	bool                       m_responseSent;
	CallbackFunction           m_callbackFunction;
	NetworkId                  m_object;

	MessageDispatch::Callback* m_callback;
};

//----------------------------------------------------------------------

inline NetworkId const & CuiInputBox::getObject         () const
{
	return m_object;
}

//======================================================================

#endif
