// ======================================================================
//
// ClientSpaceTerrainAppearance.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientSpaceTerrainAppearance_H
#define INCLUDED_ClientSpaceTerrainAppearance_H

// ======================================================================

#include "sharedTerrain/SpaceTerrainAppearance.h"

class ClientSpaceTerrainAppearanceTemplate;
class SpaceEnvironment;

// ======================================================================

class ClientSpaceTerrainAppearance : public SpaceTerrainAppearance
{
public:

	explicit ClientSpaceTerrainAppearance(ClientSpaceTerrainAppearanceTemplate const * clientSpaceTerrainAppearanceTemplate);
	virtual ~ClientSpaceTerrainAppearance();

	virtual float alter(float elapsedTime);
	virtual void render() const;

	virtual PackedRgb const getClearColor() const;

	SpaceEnvironment const & getSpaceEnvironment() const;
	SpaceEnvironment & getSpaceEnvironment();

private:

	ClientSpaceTerrainAppearance();
	ClientSpaceTerrainAppearance(ClientSpaceTerrainAppearance const &);
	ClientSpaceTerrainAppearance & operator=(ClientSpaceTerrainAppearance const &);

	DPVS::Object * getDpvsObject() const;

private:

	SpaceEnvironment * m_spaceEnvironment;

	DPVS::Object * m_dpvsObject;
};

// ======================================================================

#endif
