// ======================================================================
//
// DirsClientUser.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "DirsClientUser.h"

#include "DepotListViewItem.h"

// ======================================================================

DirsClientUser::DirsClientUser(DepotListViewItem &parent)
: ClientUser(),
	m_parent(parent)
{
}

// ----------------------------------------------------------------------

void DirsClientUser::OutputInfo(char /* level */, const_char *data)
{
	m_parent.addDirectory(data);
}

// ======================================================================
