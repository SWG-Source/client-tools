// ======================================================================
//
// DepotsClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "DepotsClientUser.h"

#include <qstring.h>
#include "ApplicationWindow.h"

// ======================================================================

DepotsClientUser::DepotsClientUser(QApplication &application, ApplicationWindow &applicationWindow)
: QtClientUser(application, applicationWindow)
{
}

// ----------------------------------------------------------------------

void 	DepotsClientUser::OutputInfo(char /* level */, const_char *data)
{
	QString depotName(data+6);
	depotName.truncate(depotName.find(' '));
	m_applicationWindow.addDepot(depotName);
}

// ======================================================================
