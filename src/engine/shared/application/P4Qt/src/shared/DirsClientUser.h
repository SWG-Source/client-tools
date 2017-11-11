// ======================================================================
//
// DirsClientUser.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DirsClientUser_H
#define INCLUDED_DirsClientUser_H

// ======================================================================

class DepotListViewItem;

#include "p4.h"

// ======================================================================

class DirsClientUser : public ClientUser
{
public:

	DirsClientUser(DepotListViewItem &parent);

	virtual void	OutputInfo(char level, const_char *data);

private:
	DirsClientUser();
	DirsClientUser(const DirsClientUser &);
	DirsClientUser &operator =(const DirsClientUser &);

protected:
	DepotListViewItem  &m_parent;
};

// ======================================================================

#endif
