//===================================================================
//
// ClientUniverse.cpp
//
// copyright 2002, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientUniverse.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedGame/ResourceClassObject.h"
#include "UnicodeUtils.h"

#include <map>

//===================================================================

void ClientUniverse::install()
{
	InstallTimer const installTimer("ClientUniverse::install");

	Universe::installDerived(new ClientUniverse);
}

//-------------------------------------------------------------------

ClientUniverse::ClientUniverse()
{
}

//-------------------------------------------------------------------

ClientUniverse::~ClientUniverse()
{
}

//-------------------------------------------------------------------

void ClientUniverse::setupResourceTree()
{
	ResourceClassObject::loadTreeFromIff();
}

// ----------------------------------------------------------------------

void ClientUniverse::resourceClassTreeLoaded(stdmap<int, ResourceClassObject*>::fwd &resourceClasses)
{
	// on the server, fractals would be loaded here
	UNREF(resourceClasses);
}

// ----------------------------------------------------------------------

ResourceClassObject *ClientUniverse::makeResourceClassObject()
{
	// on the server, we would get a more complicated class derived from ResourceClassObject
	return new ResourceClassObject;
}

//===================================================================
