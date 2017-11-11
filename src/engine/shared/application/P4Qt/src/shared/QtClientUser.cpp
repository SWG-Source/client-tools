// ======================================================================
//
// QtClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "QtClientUser.h"
#include "ApplicationWindow.h"

#include <qapplication.h>
#include <qtextedit.h>
#include <qdatetime.h>

// ======================================================================

QtClientUser::QtClientUser(QApplication &application, ApplicationWindow &applicationWindow)
: ClientUser(),
	m_application(application),
	m_applicationWindow(applicationWindow),
	m_error(false)
{
}

// ----------------------------------------------------------------------

void QtClientUser::OutputError(const_char *data)
{
	// make these messages look like info, not an error
	if (strstr(data, "- file(s) up-to-date.") || strstr(data, "- file(s) not in client view."))
	{
		OutputInfo(0, data);
		return;
	}

	if (!m_error)
	{
		m_applicationWindow.outputMessage(ApplicationWindow::MT_error, data);
		m_error = true;
	}
}

// ----------------------------------------------------------------------

void QtClientUser::OutputInfo(char /* level */, const_char *data)
{
	m_applicationWindow.outputMessage(ApplicationWindow::MT_normal, data);
	m_error = false;
	m_application.processEvents();
}

// ======================================================================
