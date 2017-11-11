// ======================================================================
//
// Messenger.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef MESSENGER_H
#define MESSENGER_H

// ======================================================================

#include <stdarg.h>
#include <ostream>

// ======================================================================

class Messenger
{
public:

	class Indenter
	{
	//lint -esym(1725, Indenter::indentVariable)   // member is a reference
	public:
		Indenter(int &newIndentVariable, int newIndentLevel);
		~Indenter(void);

	private:
		// disabled
		Indenter(void);
		Indenter(const Indenter&);
		Indenter &operator =(const Indenter&);

	private:
		int &m_indentVariable;
		int  m_indentLevel;
	};

public:
	Messenger();
	virtual ~Messenger(void);

	virtual int  &indentLevel(void) = 0;
	virtual void  setFileName(const char *filename) = 0;
	virtual void  setLineNumber(int lineNumber) = 0;
	virtual void  enableFileAndLineDisplay(bool enableFlag) = 0;
	virtual void  enableIndent(bool enableFlag) = 0;

	virtual void  logMessageNoArgs(const char *message) const = 0;
	virtual void  logMessageArgList(const char *format, va_list argList) const = 0;
	virtual void  logMessage(const char *format, ...) const = 0;

	virtual void  clearWarningsAndErrors() = 0;
	virtual void  printWarningsAndErrors() const = 0;
    virtual void  getWarningAndErrorText(std::ostream& text) const = 0;
	virtual void  logErrorMessageArgList(const char *format, va_list argList) const = 0;
	virtual void  logErrorMessage(const char *format, ...) const = 0;
	virtual void  logWarningMessageArgList(const char *format, va_list argList) const = 0;
	virtual void  logWarningMessage(const char *format, ...) const = 0;
	void          startMultiExport();
	void          endMultiExport();

	// change user feedback for silent mode
	const int     displayUserFeedback(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
	void          startSilentExport();
	void          endSilentExport();

protected:
		bool m_multiExport;
		bool m_silentExport;
};

// ======================================================================

inline Messenger::Indenter::Indenter(int &newIndentVariable, int newIndentLevel)
: m_indentVariable(newIndentVariable),
  m_indentLevel(newIndentLevel)
{
	m_indentVariable += m_indentLevel;
}

// ----------------------------------------------------------------------

inline Messenger::Indenter::~Indenter(void)
{
	m_indentVariable -= m_indentLevel;
}

// ======================================================================

inline Messenger::Messenger()
: m_multiExport(false),
  m_silentExport(false)
{
}

// ----------------------------------------------------------------------

inline void Messenger::startMultiExport()
{
	m_multiExport = true;
}

// ----------------------------------------------------------------------

inline void Messenger::endMultiExport()
{
	m_multiExport = false;
}

// ----------------------------------------------------------------------

inline void Messenger::startSilentExport()
{
	m_silentExport = true;
}

// ----------------------------------------------------------------------

inline void Messenger::endSilentExport()
{
	m_silentExport = false;
}

// ======================================================================
// some useful defines, assuming a non-const Messenger* called messenger
// is within scope when the define is called

#define MESSENGER_MESSAGE_BOX(a,b,c,d) (messenger->displayUserFeedback(a,b,c,d))

#define USE_STATUS_WINDOW 1

#if USE_STATUS_WINDOW

	// accepts a printf-style paren-enclosed format and argument list
	#define MESSENGER_LOG(a) messenger->enableFileAndLineDisplay(false), messenger->enableIndent(true), messenger->logMessage a

	// accepts a printf-style paren-enclosed format and argument list
	#define MESSENGER_LOG_ERROR(a) messenger->enableFileAndLineDisplay(false), messenger->enableIndent(true), messenger->logErrorMessage a

	// accepts a printf-style paren-enclosed format and argument list
	#define MESSENGER_LOG_WARNING(a) messenger->enableFileAndLineDisplay(false), messenger->enableIndent(true), messenger->logWarningMessage a

	// a = test condition.  if true, disaplay printf-style formatted arg list error b and return false
	#define MESSENGER_REJECT(a,b)                  \
		if (a)                                       \
		{                                            \
			messenger->enableIndent(false);            \
			messenger->enableFileAndLineDisplay(true); \
			messenger->setFileName(__FILE__);          \
			messenger->setLineNumber(__LINE__);        \
			messenger->logErrorMessage b;              \
			return false;                              \
		}

	// a = test condition.  if true, disaplay printf-style formatted warning arg list b and return false
	#define MESSENGER_REJECT_WARNING(a,b)          \
		if (a)                                       \
		{                                            \
			messenger->enableIndent(false);            \
			messenger->enableFileAndLineDisplay(true); \
			messenger->setFileName(__FILE__);          \
			messenger->setLineNumber(__LINE__);        \
			messenger->logWarningMessage b;            \
			return false;                              \
		}

	#define MESSENGER_REJECT_VOID(a,b)             \
		if (a)                                       \
		{                                            \
			messenger->enableIndent(false);            \
			messenger->enableFileAndLineDisplay(true); \
			messenger->setFileName(__FILE__);          \
			messenger->setLineNumber(__LINE__);        \
			messenger->logErrorMessage b;              \
			return;                                    \
		}

	#define MESSENGER_REJECT_STATUS(a,b)           \
		if (a)                                       \
		{                                            \
			messenger->enableIndent(false);            \
			messenger->enableFileAndLineDisplay(true); \
			messenger->setFileName(__FILE__);          \
			messenger->setLineNumber(__LINE__);        \
			messenger->logErrorMessage b;              \
			return MS::kFailure;                       \
		}

	/**
	 * Use this error reporting routine like this:
	 *   STATUS_REJECT(status, "Some error");
	 *
	 * If status indicates a failed status code, prints "Some error", 
	 * then the error string associated with status.
	 */
	#define STATUS_REJECT(status, preErrorMessage) \
		MESSENGER_REJECT(!status, ( preErrorMessage ## " [%s].\n", status.errorString().asChar() ))

	#define MESSENGER_INDENT Messenger::Indenter  indenter(messenger->indentLevel(), 2)

#else

	#define MESSENGER_LOG(a)                  {UNREF(a);}
	#define MESSENGER_LOG_ERROR(a)            {UNREF(a);}
	#define MESSENGER_LOG_WARNING(a)          {UNREF(a);}
	#define MESSENGER_REJECT(a,b)             {UNREF(a); UNREF(b);}
	#define MESSENGER_REJECT_WARNING(a,b)     {UNREF(a); UNREF(b);}
	#define MESSENGER_REJECT_VOID(a,b)        {UNREF(a); UNREF(b);}
	#define MESSENGER_REJECT_STATUS(a,b)      {UNREF(a); UNREF(b);}
	#define STATUS_REJECT(a, b)               {UNREF(a); UNREF(b);}
	#define MESSENGER_INDENT                  NOP

#endif

inline const int Messenger::displayUserFeedback(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	if (m_silentExport)
	{	
		// just log the message and return NO
		enableFileAndLineDisplay(false);
		enableIndent(true);
		logMessage(lpText);
		return IDNO;
	}
	else
	{
		// display the message
		return MessageBox(hWnd, lpText, lpCaption, uType);
	}
}

// ======================================================================

// ======================================================================

#endif
