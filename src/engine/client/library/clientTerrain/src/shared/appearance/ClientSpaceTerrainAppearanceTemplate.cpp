// ======================================================================
//
// ClientSpaceTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientSpaceTerrainAppearanceTemplate.h"

#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"

// ======================================================================
// STATIC PUBLIC ClientSpaceTerrainAppearanceTemplate
// ======================================================================

void ClientSpaceTerrainAppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(cms_spaceTerrainAppearanceTemplateTag, ClientSpaceTerrainAppearanceTemplate::create);

	ExitChain::add(ClientSpaceTerrainAppearanceTemplate::remove, "ClientSpaceTerrainAppearanceTemplate::remove");
}

// ----------------------------------------------------------------------

AppearanceTemplate * ClientSpaceTerrainAppearanceTemplate::create(char const * const filename, Iff * const iff)
{
	return new ClientSpaceTerrainAppearanceTemplate(filename, iff);
}

// ======================================================================
// PUBLIC ClientSpaceTerrainAppearanceTemplate
// ======================================================================

ClientSpaceTerrainAppearanceTemplate::ClientSpaceTerrainAppearanceTemplate(char const * const filename, Iff * const iff) :
	SpaceTerrainAppearanceTemplate(filename, iff)
{
}

// ----------------------------------------------------------------------

ClientSpaceTerrainAppearanceTemplate::~ClientSpaceTerrainAppearanceTemplate()
{
}

// ----------------------------------------------------------------------

Appearance * ClientSpaceTerrainAppearanceTemplate::createAppearance() const
{
	return new ClientSpaceTerrainAppearance(this);
}

// ======================================================================
// STATIC PRIVATE ClientSpaceTerrainAppearanceTemplate
// ======================================================================

void ClientSpaceTerrainAppearanceTemplate::remove()
{
	AppearanceTemplateList::removeBinding(cms_spaceTerrainAppearanceTemplateTag);
}

// ======================================================================

