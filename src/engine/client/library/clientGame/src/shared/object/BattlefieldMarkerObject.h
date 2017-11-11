// ======================================================================
//
// BattlefieldMarkerObject.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================


#ifndef INCLUDED_BattlefieldMarkerObject_H
#define INCLUDED_BattlefieldMarkerObject_H

// ======================================================================

class SharedBattlefieldMarkerObjectTemplate;

// ======================================================================

#include "clientGame/TangibleObject.h"

// ======================================================================

class BattlefieldMarkerObject: public TangibleObject
{
public:
	explicit BattlefieldMarkerObject(SharedBattlefieldMarkerObjectTemplate const *newTemplate);
	virtual ~BattlefieldMarkerObject();

private:
	BattlefieldMarkerObject();
	BattlefieldMarkerObject(BattlefieldMarkerObject const &);
	BattlefieldMarkerObject &operator=(BattlefieldMarkerObject const &);
};

// ======================================================================

#endif

