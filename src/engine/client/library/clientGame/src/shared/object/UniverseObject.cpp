// ======================================================================
//
// UniverseObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/UniverseObject.h"

#include "sharedGame/SharedUniverseObjectTemplate.h"
#include "clientGame/ClientWorld.h"

// ======================================================================

UniverseObject::UniverseObject(SharedUniverseObjectTemplate const *newTemplate) :
	ClientObject(newTemplate, ClientWorld::getIntangibleNotification())
{
}	

// ----------------------------------------------------------------------

UniverseObject::~UniverseObject()
{
}

// ======================================================================

