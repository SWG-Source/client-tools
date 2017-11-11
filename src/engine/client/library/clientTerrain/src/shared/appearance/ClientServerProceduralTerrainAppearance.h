//===================================================================
//
// ClientServerProceduralTerrainAppearance.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ClientServerProceduralTerrainAppearance_H
#define INCLUDED_ClientServerProceduralTerrainAppearance_H

//===================================================================

#include "sharedTerrain/ServerProceduralTerrainAppearance.h"

namespace DPVS
{
	class Object;
}

class Light;

//===================================================================

class ClientServerProceduralTerrainAppearance : public ServerProceduralTerrainAppearance
{
public:

	static void install ();
	static void remove ();

public:

	explicit ClientServerProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate);
	virtual ~ClientServerProceduralTerrainAppearance (void);

	virtual void render () const;

private:

	virtual DPVS::Object* getDpvsObject () const;
	virtual void createChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags);

private:

	ClientServerProceduralTerrainAppearance (void);
	ClientServerProceduralTerrainAppearance (const ClientServerProceduralTerrainAppearance&);
	ClientServerProceduralTerrainAppearance& operator= (const ClientServerProceduralTerrainAppearance&);

private:

	Light*        m_ambientLight;
	Light*        m_parallelLight;
	DPVS::Object* m_dpvsObject;

#if _DEBUG
	bool          m_noRenderTerrain;
#endif
}; 

//===================================================================

#endif
