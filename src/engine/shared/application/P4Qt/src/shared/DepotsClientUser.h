// ======================================================================
//
// DepotsClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DepotsClientUser_H
#define INCLUDED_DepotsClientUser_H

// ======================================================================

#include "QtClientUser.h"

// ======================================================================

class DepotsClientUser : public QtClientUser
{
public:

	DepotsClientUser(QApplication &application, ApplicationWindow &applicationWindow);
	virtual void 	OutputInfo(char level, const_char *data);

private:
	DepotsClientUser();
	DepotsClientUser(const DepotsClientUser &);
	DepotsClientUser &operator =(const DepotsClientUser &);
};

// ======================================================================

#endif
