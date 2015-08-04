// ======================================================================
//
// FstatClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FstatClientUser.h"

#include "DepotListViewItem.h"

// ======================================================================

FstatClientUser::FstatClientUser(DepotListViewItem &parent)
: ClientUser(),
	m_parent(parent)
{
}

// ----------------------------------------------------------------------

void 	FstatClientUser::OutputStat(StrDict *varList)
{
	// depotFile
	// clientFile
	// headAction
	// headType
	// headTime
	// headRev
	// headChange
	// haveRev
	// ... otherOpen0
	// ... otherAction0
	// ... otherOpen1
	// ... otherAction1

	StrPtr *type = varList->GetVar("headType");
	if (!type)
		return;
	bool binary = (strcmp(type->Text(), "binary") == 0) ? true : false;

	StrPtr *headAction = varList->GetVar("headAction");
	if (!headAction || strcmp(headAction->Text(), "delete") != 0)
		m_parent.addFile(varList->GetVar("depotFile")->Text(), binary);
}

// ======================================================================
