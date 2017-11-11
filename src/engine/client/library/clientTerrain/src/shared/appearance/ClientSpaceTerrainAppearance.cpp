// ======================================================================
//
// ClientSpaceTerrainAppearance.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientSpaceTerrainAppearance.h"

#include "clientGraphics/RenderWorld.h"
#include "clientTerrain/ClientSpaceTerrainAppearanceTemplate.h"
#include "clientTerrain/SpaceEnvironment.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"

// ======================================================================
// PUBLIC ClientSpaceTerrainAppearance
// ======================================================================

ClientSpaceTerrainAppearance::ClientSpaceTerrainAppearance(ClientSpaceTerrainAppearanceTemplate const * const clientSpaceTerrainAppearanceTemplate) :
	SpaceTerrainAppearance(clientSpaceTerrainAppearanceTemplate),
	m_spaceEnvironment(0),
	m_dpvsObject(0)
{
	m_spaceEnvironment = new SpaceEnvironment(*this);

	m_dpvsObject = RenderWorld::createUnboundedObject(this);
}

// ----------------------------------------------------------------------

ClientSpaceTerrainAppearance::~ClientSpaceTerrainAppearance()
{
	delete m_spaceEnvironment;
	m_spaceEnvironment = 0;

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = 0;
}

// ----------------------------------------------------------------------

float ClientSpaceTerrainAppearance::alter(float const elapsedTime)
{
	IGNORE_RETURN(SpaceTerrainAppearance::alter(elapsedTime));
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void ClientSpaceTerrainAppearance::render() const
{
	NOT_NULL(m_spaceEnvironment);
	m_spaceEnvironment->render();
}

// ----------------------------------------------------------------------

PackedRgb const ClientSpaceTerrainAppearance::getClearColor() const
{
	NOT_NULL(m_spaceEnvironment);
	return m_spaceEnvironment->getClearColor();
}

//----------------------------------------------------------------------

SpaceEnvironment const & ClientSpaceTerrainAppearance::getSpaceEnvironment() const
{
	return *NON_NULL(m_spaceEnvironment);
}

//----------------------------------------------------------------------

SpaceEnvironment & ClientSpaceTerrainAppearance::getSpaceEnvironment()
{
	return *NON_NULL(m_spaceEnvironment);
}

// ======================================================================
// PRIVATE ClientSpaceTerrainAppearance
// ======================================================================

DPVS::Object * ClientSpaceTerrainAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ======================================================================

