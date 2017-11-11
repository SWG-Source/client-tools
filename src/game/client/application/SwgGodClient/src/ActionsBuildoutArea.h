// ======================================================================
//
// ActionsBuildoutArea.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsBuildoutArea_H
#define INCLUDED_ActionsBuildoutArea_H

// ======================================================================

#include "Singleton/Singleton.h"

class ActionHack;

// ======================================================================

class ActionsBuildoutArea: public QObject, public Singleton<ActionsBuildoutArea>
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	//lint -save
	//lint -e1925 // public data member
	ActionHack *actionRefresh;
	ActionHack *actionEdit;
	ActionHack *actionSave;

	//lint -restore

public:
	ActionsBuildoutArea();
	~ActionsBuildoutArea();

private:
	//disabled
	ActionsBuildoutArea(ActionsBuildoutArea const &);
	ActionsBuildoutArea &operator=(ActionsBuildoutArea const &);
};

// ======================================================================

#endif // INCLUDED_ActionsBuildoutArea_H
