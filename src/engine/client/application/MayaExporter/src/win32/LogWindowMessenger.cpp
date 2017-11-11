// ======================================================================
//
// LogWindowMessenger.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "LogWindowMessenger.h"

#include "sharedStatusWindow/LogWindow.h"

#include <stdio.h>
#include <sstream>

// ======================================================================

namespace
{
	const std::string banner1 = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	const std::string banner2 = "!! ERRORS ENCOUNTERED, PLEASE EXAMINE         !!\n";
	const std::string banner3 = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
}

// ======================================================================

LogWindowMessenger::LogWindowMessenger(void)
:	Messenger(),
	m_indentLevelValue(0),
	m_filename(0),
	m_lineNumber(0),
	m_showFileAndLine(false),
	m_showIndent(true),
	m_outFile(0)
{
	//create a new file if needed, but append if possible
	m_outFile = fopen("c:/mayaExporterLog.txt", "at");
}

// ----------------------------------------------------------------------

LogWindowMessenger::~LogWindowMessenger(void)
{
	if(m_outFile != 0)
		fclose(m_outFile);
	m_outFile = 0;

	m_filename = 0;
}

// ----------------------------------------------------------------------

int &LogWindowMessenger::indentLevel(void)
{
	return m_indentLevelValue;
}

// ----------------------------------------------------------------------

void LogWindowMessenger::setFileName(const char *newFilename)
{
	m_filename = newFilename;
}

// ----------------------------------------------------------------------

void LogWindowMessenger::setLineNumber(int newLineNumber)
{
	m_lineNumber = newLineNumber;
}

// ----------------------------------------------------------------------

void LogWindowMessenger::enableFileAndLineDisplay(bool enableFlag)
{
	m_showFileAndLine = enableFlag;
}

// ----------------------------------------------------------------------

void LogWindowMessenger::enableIndent(bool enableFlag)
{
	m_showIndent = enableFlag;
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logMessageNoArgs(const char *message) const
{
	DEBUG_FATAL(!message, ("null message arg"));

	char  buffer[BUFFER_SIZE];
	char *writePtr;

	// handle indent
	if (m_showIndent)
	{
		memset(buffer, 32, m_indentLevelValue);
		writePtr = buffer + m_indentLevelValue;
	}
	else
		writePtr = buffer;

	// handle file and line display
	if (m_showFileAndLine)
	{
		sprintf(writePtr, "%s(%d) : ", m_filename, m_lineNumber);
		writePtr += strlen(writePtr);
	}

	// copy in the rest of the message
	strcpy(writePtr, message);

	// display it
	LogWindow::printNoArgs(buffer);
//	REPORT_LOG(true, (buffer)); Double print from LogWindow::printNoArgs

	if(m_outFile != 0)
	{
		fwrite(buffer, sizeof(char), strlen(buffer), m_outFile);
		fflush(m_outFile);
	}
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logMessageArgList(const char *format, va_list argList) const
{
	DEBUG_FATAL(!format, ("null format arg"));
	DEBUG_FATAL(!argList, ("null argList arg"));

	char buffer[BUFFER_SIZE];

	// -TRF- potential buffer overrun
	static_cast<void> (vsprintf (buffer, format, argList));

	LogWindowMessenger::logMessageNoArgs(buffer);
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logErrorMessageArgList(const char *format, va_list argList) const
{
	DEBUG_FATAL(!format, ("null format arg"));
	DEBUG_FATAL(!argList, ("null argList arg"));

	char buffer[BUFFER_SIZE];

	// -TRF- potential buffer overrun
	static_cast<void> (vsprintf (buffer, format, argList));

	LogWindowMessenger::logMessageNoArgs(buffer);
	m_errorMessages.push_back(buffer);
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logWarningMessageArgList(const char *format, va_list argList) const
{
	DEBUG_FATAL(!format, ("null format arg"));
	DEBUG_FATAL(!argList, ("null argList arg"));

	char buffer[BUFFER_SIZE];

	// -TRF- potential buffer overrun
	static_cast<void> (vsprintf (buffer, format, argList));

	LogWindowMessenger::logMessageNoArgs(buffer);
	m_warningMessages.push_back(buffer);
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logMessage(const char *format, ...) const
{
	va_list argList;
	
	va_start (argList, format);
	LogWindowMessenger::logMessageArgList(format, argList);
	va_end (argList);	
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logErrorMessage(const char *format, ...) const
{
	va_list argList;
	
	va_start (argList, format);
	LogWindowMessenger::logErrorMessageArgList(format, argList);
	va_end (argList);	
}

// ----------------------------------------------------------------------

void LogWindowMessenger::logWarningMessage(const char *format, ...) const
{
	va_list argList;
	
	va_start (argList, format);
	LogWindowMessenger::logWarningMessageArgList(format, argList);
	va_end (argList);	
}

// ----------------------------------------------------------------------

/** Clear out the list of errors and warnings.  Do NOT do this if we're in a multi-export.
 */
void LogWindowMessenger::clearWarningsAndErrors()
{
	if(!m_multiExport)
	{
		m_warningMessages.clear();
		m_errorMessages.clear();
	}
}

// ----------------------------------------------------------------------

/** Print out the list of errors and warnings.  Do NOT do this if we're in a multi-export.
 */
void LogWindowMessenger::printWarningsAndErrors() const
{
	if(!m_multiExport)
	{
		if(!m_errorMessages.empty() || !m_warningMessages.empty())
		{
			LogWindowMessenger::logMessage("\n");
			LogWindowMessenger::logMessageNoArgs(banner1.c_str());
			LogWindowMessenger::logMessageNoArgs(banner2.c_str());
			LogWindowMessenger::logMessageNoArgs(banner3.c_str());
			for(std::vector<std::string>::iterator i = m_warningMessages.begin(); i != m_warningMessages.end(); ++i)
				LogWindowMessenger::logMessage("%s%s", "WARNING: ", i->c_str());
			for(std::vector<std::string>::iterator j = m_errorMessages.begin(); j != m_errorMessages.end(); ++j)
				LogWindowMessenger::logMessage("%s%s", "ERROR: ", j->c_str());
		}
		LogWindowMessenger::logMessage("\n");
		LogWindowMessenger::logMessage("Export process - %d error(s), %d warning(s)\n", m_errorMessages.size(), m_warningMessages.size());
	}
}

// ======================================================================

/** Print out the list of errors and warnings.  Do NOT do this if we're in a multi-export.
 */
void LogWindowMessenger::getWarningAndErrorText(std::ostream& textstream) const
{
	if(!m_multiExport)
	{
		if(!m_errorMessages.empty() || !m_warningMessages.empty())
		{
            textstream << std::endl;
            textstream << banner2;

            for(std::vector<std::string>::iterator i = m_warningMessages.begin(); i != m_warningMessages.end(); ++i)
            {
                textstream << "WARNING: " << i->c_str();
            }

			for(std::vector<std::string>::iterator j = m_errorMessages.begin(); j != m_errorMessages.end(); ++j)
            {
				textstream << "ERROR: " << j->c_str();
            }
		}
		
        textstream << std::endl << "Export process - " 
                    << m_errorMessages.size() << " error(s), " 
                    << m_warningMessages.size() << " warning(s)" << std::endl;
	}
}
