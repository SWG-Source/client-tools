//===================================================================
//
// ClientProceduralTerrainAppearanceTemplate.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ClientProceduralTerrainAppearanceTemplate_h
#define INCLUDED_ClientProceduralTerrainAppearanceTemplate_h

//===================================================================

#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

class Iff;

//===================================================================

class ClientProceduralTerrainAppearanceTemplate : public ProceduralTerrainAppearanceTemplate
{
public:

	static void install ();

	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	ClientProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~ClientProceduralTerrainAppearanceTemplate (void);

	virtual Appearance* createAppearance () const;
	virtual void        preloadAssets () const;
	virtual void        garbageCollect () const;

	// CONSULT-59: incremental preload. preloadAssets() only captures the asset
	// name lists (cheap); preloadAssetsStep() performs fetches under the
	// [ClientTerrain] terrainPreloadBudgetMs per-frame budget and returns true
	// once everything is fetched. Pumped by ClientProceduralTerrainAppearance.
	bool                preloadAssetsStep () const;
	bool                isPreloadComplete () const;

private:

	static void remove ();

private:

	ClientProceduralTerrainAppearanceTemplate (void);
	ClientProceduralTerrainAppearanceTemplate (const ClientProceduralTerrainAppearanceTemplate&);
	ClientProceduralTerrainAppearanceTemplate& operator= (const ClientProceduralTerrainAppearanceTemplate&);

private:

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager* m_preloadManager;
}; 

//===================================================================

#endif
 
