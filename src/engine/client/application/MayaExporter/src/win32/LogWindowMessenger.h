// ======================================================================
//
// LogWindowMessenger.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef LOG_WINDOW_MESSENGER_H
#define LOG_WINDOW_MESSENGER_H

// ======================================================================

#include "Messenger.h"

#include <cstdio>
#include <string>
#include <vector>

// ======================================================================

class StdioFile;

// ======================================================================

class LogWindowMessenger: public Messenger
{
//lint -esym(1536, LogWindowMessenger::indentLevelValue) // exposing low access member

public:

	enum
	{
		BUFFER_SIZE = 1024
	};

public:
	LogWindowMessenger(void);
	~LogWindowMessenger(void);

	virtual int  &indentLevel(void);
	virtual void  setFileName(const char *newFilename);
	virtual void  setLineNumber(int newLineNumber);
	virtual void  enableFileAndLineDisplay(bool enableFlag);
	virtual void  enableIndent(bool enableFlag);

	virtual void  logMessageNoArgs(const char *message) const;
	virtual void  logMessageArgList(const char *format, va_list argList) const;
	virtual void  logMessage(const char *format, ...) const;
	virtual void  clearWarningsAndErrors();
	virtual void  printWarningsAndErrors() const;
    virtual void  getWarningAndErrorText(std::ostream& textstream) const;

	virtual void  logErrorMessageArgList(const char *format, va_list argList) const;
	virtual void  logErrorMessage(const char *format, ...) const;
	virtual void  logWarningMessageArgList(const char *format, va_list argList) const;
	virtual void  logWarningMessage(const char *format, ...) const;


private:
	int         m_indentLevelValue;
	const char *m_filename;
	int         m_lineNumber;
	bool        m_showFileAndLine;
	bool        m_showIndent;
	FILE       *m_outFile;
	mutable std::vector<std::string> m_errorMessages;
	mutable std::vector<std::string> m_warningMessages;
};

// ======================================================================

#endif
