// ======================================================================
//
// OpenedClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_OpenedClientUser_H
#define INCLUDED_OpenedClientUser_H

// ======================================================================

#include "QtClientUser.h"

// ======================================================================

class OpenedClientUser : public QtClientUser
{
public:

	OpenedClientUser(QApplication &application, ApplicationWindow &applicationWindow);
	virtual void 	OutputStat(StrDict *varList);

private:
	OpenedClientUser();
	OpenedClientUser(const OpenedClientUser &);
	OpenedClientUser &operator =(const OpenedClientUser &);
};

// ======================================================================

#endif
