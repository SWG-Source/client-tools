//===================================================================
//
// ClientServerProceduralTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientServerProceduralTerrainAppearanceTemplate.h"

#include "clientTerrain/ClientServerProceduralTerrainAppearance.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ConfigSharedTerrain.h"

//===================================================================

void ClientServerProceduralTerrainAppearanceTemplate::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientServerProceduralTerrainAppearanceTemplate::install\n"));

	AppearanceTemplateList::assignBinding (TAG (M,P,T,A), ClientServerProceduralTerrainAppearanceTemplate::create);
	AppearanceTemplateList::assignBinding (TAG (P,T,A,T), ClientServerProceduralTerrainAppearanceTemplate::create);

	ExitChain::add (remove, "ClientServerProceduralTerrainAppearanceTemplate::remove");
}

//-------------------------------------------------------------------

void ClientServerProceduralTerrainAppearanceTemplate::remove ()
{
	AppearanceTemplateList::removeBinding (TAG (M,P,T,A));
	AppearanceTemplateList::removeBinding (TAG (P,T,A,T));
}

//-------------------------------------------------------------------

AppearanceTemplate* ClientServerProceduralTerrainAppearanceTemplate::create (const char* filename, Iff* iff)
{
	return new ClientServerProceduralTerrainAppearanceTemplate (filename, iff);
}

//===================================================================

ClientServerProceduralTerrainAppearanceTemplate::ClientServerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff) :
	ProceduralTerrainAppearanceTemplate (filename, iff, false, 2, 2)
{
}

//-------------------------------------------------------------------

ClientServerProceduralTerrainAppearanceTemplate::~ClientServerProceduralTerrainAppearanceTemplate (void)
{
}

//-------------------------------------------------------------------

Appearance* ClientServerProceduralTerrainAppearanceTemplate::createAppearance () const
{
	return new ClientServerProceduralTerrainAppearance (this);
}

//===================================================================
