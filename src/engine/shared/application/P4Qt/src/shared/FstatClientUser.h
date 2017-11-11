// ======================================================================
//
// FstatClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FstatClientUser_H
#define INCLUDED_FstatClientUser_H

// ======================================================================

class DepotListViewItem;

#include "p4.h"

// ======================================================================

class FstatClientUser : public ClientUser
{
public:

	FstatClientUser(DepotListViewItem &parent);

	virtual void 	OutputStat(StrDict *varList);

private:
	FstatClientUser();
	FstatClientUser(const FstatClientUser &);
	FstatClientUser &operator =(const FstatClientUser &);

protected:
	DepotListViewItem  &m_parent;
};

// ======================================================================

#endif
