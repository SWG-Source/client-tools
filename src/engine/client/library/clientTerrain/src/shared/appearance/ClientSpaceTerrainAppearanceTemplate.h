// ======================================================================
//
// ClientSpaceTerrainAppearanceTemplate.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientSpaceTerrainAppearanceTemplate_H
#define INCLUDED_ClientSpaceTerrainAppearanceTemplate_H

// ======================================================================

#include "sharedTerrain/SpaceTerrainAppearanceTemplate.h"

class Iff;

// ======================================================================

class ClientSpaceTerrainAppearanceTemplate : public SpaceTerrainAppearanceTemplate
{
public:

	static void install();

	static AppearanceTemplate * create(char const * filename, Iff * iff);

public:

	ClientSpaceTerrainAppearanceTemplate(char const * filename, Iff * iff);
	virtual ~ClientSpaceTerrainAppearanceTemplate();

	virtual Appearance * createAppearance() const;

private:

	static void remove();

private:

	ClientSpaceTerrainAppearanceTemplate();
	ClientSpaceTerrainAppearanceTemplate(ClientSpaceTerrainAppearanceTemplate const &);
	ClientSpaceTerrainAppearanceTemplate & operator=(ClientSpaceTerrainAppearanceTemplate const &);
}; 

// ======================================================================

#endif
 
