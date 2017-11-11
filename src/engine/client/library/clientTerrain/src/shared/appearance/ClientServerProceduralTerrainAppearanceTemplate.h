//===================================================================
//
// ClientServerProceduralTerrainAppearanceTemplate.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ClientServerProceduralTerrainAppearanceTemplate_h
#define INCLUDED_ClientServerProceduralTerrainAppearanceTemplate_h

//===================================================================

#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

class Iff;

//===================================================================

class ClientServerProceduralTerrainAppearanceTemplate : public ProceduralTerrainAppearanceTemplate
{
public:

	static void install ();

	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	ClientServerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~ClientServerProceduralTerrainAppearanceTemplate (void);

	virtual Appearance* createAppearance () const;

private:

	static void remove ();

private:

	ClientServerProceduralTerrainAppearanceTemplate (void);
	ClientServerProceduralTerrainAppearanceTemplate (const ClientServerProceduralTerrainAppearanceTemplate&);
	ClientServerProceduralTerrainAppearanceTemplate& operator= (const ClientServerProceduralTerrainAppearanceTemplate&);
}; 

//===================================================================

#endif
 
