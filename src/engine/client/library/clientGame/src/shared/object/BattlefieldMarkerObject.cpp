// ======================================================================
//
// BattlefieldMarkerObject.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/BattlefieldMarkerObject.h"

#include "clientGame/ClientBattlefieldMarkerObjectTemplate.h"

// ======================================================================

BattlefieldMarkerObject::BattlefieldMarkerObject(SharedBattlefieldMarkerObjectTemplate const *newTemplate) :
	TangibleObject(newTemplate)
{
}

// ----------------------------------------------------------------------
	
BattlefieldMarkerObject::~BattlefieldMarkerObject()
{
}

// ======================================================================

