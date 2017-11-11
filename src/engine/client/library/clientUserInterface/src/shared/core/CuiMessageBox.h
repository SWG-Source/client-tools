// ======================================================================
//
// CuiMessageBox.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiMessageBox_H
#define INCLUDED_CuiMessageBox_H

#include "sharedMessageDispatch/Message.h"
#include "UIEventCallback.h"

class UIPage;
class UIText;
class UIButton;
class UIImage;
class UIWidget;
class UIScrollbar;
class UIProgressbar;
class StringId;
class UIRunner;
class UIComposite;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Emitter;
	class Receiver;
	template <typename MessageType, typename IdentifierType> class Transceiver;
	class Callback;
}

// ======================================================================

/**
* CuiMessageBox provides modal dialog functionality.
*
* It is a wrapper for a set of widgets that compose the UI message box.
* The message box can have any of several predefined buttons, and a text
* message.  The messagebox has hooks to support a progress bar when implemented.
*
*/

class CuiMessageBox : 
public UIEventCallback
{
public:

	typedef void (*CallbackFunc) (const CuiMessageBox & );

	enum GenericButtonType
	{
		GBT_None,
		GBT_Ok,
		GBT_Cancel,
		GBT_No,
		GBT_Yes,
		GBT_Abort,
		GBT_Close,
		GBT_Max
	};

	struct Messages
	{
		static const char * const CLOSED;
		static const char * const COMPLETED;
		static const char * const PROGRESS_FINISHED;

		struct Closed
		{
			typedef CuiMessageBox Payload;
		};
	};

	struct Transceivers
	{
		typedef MessageDispatch::Transceiver <const Messages::Closed::Payload &, Messages::Closed> Closed;
	};

	//----------------------------------------------------------------------

	struct BoxMessage : public MessageDispatch::MessageBase
	{
		BoxMessage (CuiMessageBox * box, const char * type);
		virtual ~BoxMessage ();
		const CuiMessageBox * getMessageBox () const;
	private:
		CuiMessageBox *  m_box;
		BoxMessage ();

	};

	//----------------------------------------------------------------------

	struct CompletedMessage : public BoxMessage
	{
		CompletedMessage (CuiMessageBox * box, GenericButtonType type);
		GenericButtonType getButtonType () const ;
	private:
		GenericButtonType m_buttonType;
		CompletedMessage ();
	};

	//----------------------------------------------------------------------

	static StringId ms_ButtonTypeLabels[GBT_Max];
	static const char * const ms_ButtonTypeNames[GBT_Max];

	struct ButtonInfo
	{
		UIString           m_labelText;
		UINarrowString     m_name;
		bool               m_isDefault;
		bool               m_isCancel;
		GenericButtonType  m_type;

					 ButtonInfo (GenericButtonType type, bool isDefault, bool isCancel);
					 ButtonInfo ();
	private:
		             ButtonInfo (const ButtonInfo & rhs);
		ButtonInfo & operator=  (const ButtonInfo & rhs);

	};

	explicit                    CuiMessageBox       (bool modal, CallbackFunc callbackFunc);

	void                        setText             (const UIString & str, bool doLayout = true);
	void						setInputText        (const UIString & str, bool doLayout = true);

	void                        setButtons          (const ButtonInfo * buttons, const int numButtons, bool doLayout = true);
	void                        addButton           (const ButtonInfo & info, bool doLayout = false);
	void                        addButton           (GenericButtonType type, bool isDefault, bool isCancel, bool doLayout = false);
	void						enableInputField	(bool b);
	void                        setProgressbar      (bool b);

	void                        OnButtonPressed     (UIWidget * context);
	bool						OnMessage			(UIWidget * context, const UIMessage & msg);

	void                        setModal            (bool b);

	bool                        pleaseKillMe        () const;

	const std::string &         getName             () const;

	void                        layout              ();

	void                        closeMessageBox     ();

	void                        setTransitionTarget (const char * target);

	UIText &                    getUIText           ();
	UIText &					getInputText		() const;

	void                        connectToMessages      (MessageDispatch::Receiver & receiver) const;
	void                        disconnectFromMessages (MessageDispatch::Receiver & receiver) const;

	MessageDispatch::Emitter &  getEmitter ();

	static void                 install ();
	static void                 remove ();

	static CuiMessageBox *      createMessageBox   (CallbackFunc callbackFunc = 0, bool modal = true, bool runner = false, bool doLayout = true, bool inputField = false);
	static CuiMessageBox *      createMessageBox   (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true, bool runner = false, bool inputField = false);
	static CuiMessageBox *      createMessageBox   (const UIString & str, GenericButtonType type, CallbackFunc callbackFunc = 0, bool modal = true, bool runner = false, bool inputField = false);
	static CuiMessageBox *      createInfoBox      (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true);
	static CuiMessageBox *      createWaitingBox   (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true);
	static CuiMessageBox *      createOkCancelBox  (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true);
	static CuiMessageBox *      createYesNoBox     (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true);
	static CuiMessageBox *      createDialogBox    (const UIString & str, GenericButtonType buttonDefault, GenericButtonType buttonCancel, CallbackFunc callbackFunc = 0, bool modal = true);
	static CuiMessageBox *      createOkCancelBoxWithInput (const UIString & str, CallbackFunc callbackFunc = 0, bool modal = true);
	static void                 update ();

	GenericButtonType           getCompletedButtonType () const;
	UINarrowString const &		getCompletedButtonName () const;

	bool                        completedAffirmative   () const;

	Transceivers::Closed &      getTransceiverClosed ();

	static int                  getModalMessageBoxCount ();

	void                        setCallbackFunc (CallbackFunc func);

	void                        setRunner       (bool b, bool doLayout = true);

	static void                 ensureFocus     ();

	void						randomizePosition ();

	UIImage *					getImage();

	void						enableImage(bool enable);

	void						generateVerificationImage(int totalCopies, bool rotate = true, bool scale = true);

	//----------------------------------------------------------------------
	//-- only the CuiManager is allowed to delete message boxes

	class CuiManagerHelper
	{
		friend class CuiManager;
		friend class CuiMessageBox;
		static void destroyMessageBox (CuiMessageBox * box)
		{
			delete box;
		}
	};

	friend class CuiManagerHelper;

private:
	                           ~CuiMessageBox ();

	                            CuiMessageBox ();
	                            CuiMessageBox (const CuiMessageBox & rhs);
	CuiMessageBox &             operator=      (const CuiMessageBox & rhs);


private:

	bool                        m_killme;

	bool                        m_modal;

	UIPage *                    m_mainPage;
	UIPage *                    m_innerPage;
	UIPage *					m_inputPage;

	UIText *                    m_text;
	UIText *					m_inputText;
	UIScrollbar *               m_scrollbar;

	UIButton *                  m_sampleButton;

	UIProgressbar *             m_progressbar;

	typedef stdvector <UIButton *>::fwd ButtonVector;
    ButtonVector *              m_buttons;

	const char *                m_transitionTarget;

	MessageDispatch::Emitter *  m_emitter;

	GenericButtonType           m_completedButtonType;
	UINarrowString			    m_completedButtonName;

	Transceivers::Closed *      m_transceiverClosed;

	UIButton *                  m_buttonClose;

	CallbackFunc                m_callbackFunc;

	UIRunner *                  m_runner;
	UIComposite *               m_composite;
	UIComposite *               m_compositeButtons;

	UIRect *                    m_compositeMargins;
	UIImage *					m_image;
	UIPage *					m_verifyPage;
};

//-----------------------------------------------------------------


inline bool CuiMessageBox::pleaseKillMe () const
{
	return m_killme;
}

//-----------------------------------------------------------------

inline UIText & CuiMessageBox::getUIText ()
{
	return *m_text;
}

//----------------------------------------------------------------------

inline CuiMessageBox::GenericButtonType   CuiMessageBox::getCompletedButtonType () const
{
	return m_completedButtonType;
}

//----------------------------------------------------------------------

inline bool CuiMessageBox::completedAffirmative   () const
{
	return m_completedButtonType == GBT_Yes || m_completedButtonType == GBT_Ok;
}

//-----------------------------------------------------------------

inline UIText & CuiMessageBox::getInputText() const
{
	return *m_inputText;
}

//-----------------------------------------------------------------

inline UINarrowString const & CuiMessageBox::getCompletedButtonName() const
{
	return m_completedButtonName;
}

//-----------------------------------------------------------------

inline UIImage * CuiMessageBox::getImage()
{
	return m_image;
}

//-----------------------------------------------------------------

#endif
