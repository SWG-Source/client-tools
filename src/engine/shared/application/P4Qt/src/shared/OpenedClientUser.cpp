// ======================================================================
//
// OpenedClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "OpenedClientUser.h"
#include "ApplicationWindow.h"

#include <qapplication.h>

// ======================================================================

OpenedClientUser::OpenedClientUser(QApplication &application, ApplicationWindow &applicationWindow)
: QtClientUser(application, applicationWindow)
{
}

// ----------------------------------------------------------------------

void OpenedClientUser::OutputStat(StrDict *varList)
{
	// depotFile
	// clientFile
	// rev
	// action
	// change
	// type
	// user
	// client

	StrPtr *type = varList->GetVar("type");
	if (!type)
		return;
	bool binary = (strcmp(type->Text(), "binary") == 0) ? true : false;

	m_applicationWindow.addOpenedFile(varList->GetVar("change")->Text(), varList->GetVar("depotFile")->Text(), binary);
}

// ======================================================================
