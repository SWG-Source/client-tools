// ======================================================================
//
// P4QtClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_P4QtClientUser_H
#define INCLUDED_P4QtClientUser_H

// ======================================================================

class QApplication;
class ApplicationWindow;

#include "p4.h"

// ======================================================================

class QtClientUser : public ClientUser
{
public:

	QtClientUser(QApplication &application, ApplicationWindow &applicationWindow);

	virtual void 	OutputError(const_char *errBuf);
	virtual void	OutputInfo(char level, const_char *data);

private:
	QtClientUser();
	QtClientUser(const QtClientUser &);
	QtClientUser &operator =(const QtClientUser &);

protected:
	QApplication         &m_application;
	ApplicationWindow    &m_applicationWindow;
	bool                  m_error;
};

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------


// ======================================================================

#endif
