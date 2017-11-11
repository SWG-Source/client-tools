// ======================================================================
//
// ContentSyncClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstSwgContentSync.h"
#include "ContentSyncClientUser.h"
#include "ApplicationWindow.h"

#include <qapplication.h>
#include <qtextedit.h>
#include <qdatetime.h>

// ======================================================================

ContentSyncClientUser::ContentSyncClientUser(ApplicationWindow &applicationWindow)
: ClientUser(),
	m_applicationWindow(applicationWindow),
	m_error(false)
{
}

// ----------------------------------------------------------------------

void ContentSyncClientUser::OutputError(const_char *data)
{
	// make these messages look like info, not an error
	if (strstr(data, "- file(s) up-to-date.") || strstr(data, "- file(s) not in client view.") || strstr(data, "- no permission for operation on file(s)."))
	{
		OutputInfo(0, data);
		return;
	}

	if (!m_error)
	{
		m_applicationWindow.appendMessage(ApplicationWindow::MT_error, data);
		m_errors.push_back(QString(data));
		m_error = true;
	}
}

// ----------------------------------------------------------------------

void ContentSyncClientUser::OutputInfo(char /* level */, const_char *data)
{
	m_applicationWindow.appendMessage(ApplicationWindow::MT_normal, data);
	m_error = false;
}

// ----------------------------------------------------------------------

void ContentSyncClientUser::OutputText(const_char * /* data */, int /* length */)
{
}

// ----------------------------------------------------------------------

ContentSyncClientUser::ErrorList::const_iterator ContentSyncClientUser::errorsBegin() const
{
	return m_errors.begin();
}

// ----------------------------------------------------------------------

ContentSyncClientUser::ErrorList::const_iterator ContentSyncClientUser::errorsEnd() const
{
	return m_errors.end();
}

// ======================================================================
