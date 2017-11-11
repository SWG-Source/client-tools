//
// EditorTerrain.h
// asommers 2001-01-17
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#ifndef INCLUDED_EditorTerrain_H
#define INCLUDED_EditorTerrain_H

//-------------------------------------------------------------------

class BakedTerrain;
class MapView;
class Shader;
class TerrainEditorDoc;
class Vector2d;
class VertexBuffer;
class AffectorRibbon;

//-------------------------------------------------------------------

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/ArrayList.h"
#include "sharedMath/Plane.h"
#include "sharedTerrain/TerrainGenerator.h"

//-------------------------------------------------------------------

class EditorTerrain
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Data
	{
	public:

		CString shaderFamilyName;                    //lint !e1925 //-- public data member
		CString floraStaticCollidableFamilyName;     //lint !e1925 //-- public data member
		CString floraStaticNonCollidableFamilyName;  //lint !e1925 //-- public data member
		CString floraDynamicNearFamilyName;          //lint !e1925 //-- public data member
		CString floraDynamicFarFamilyName;           //lint !e1925 //-- public data member
		CString environmentFamilyName;               //lint !e1925 //-- public data member
		real    height;                              //lint !e1925 //-- public data member

	public:

		Data ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct MetaData
	{
		const char*      name;
		Rectangle2d      extent;
		ArrayList<Plane> planeList;
		float            width;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum ResolutionType
	{
		RT_high,
		RT_medium,
		RT_low
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	EditorTerrain (TerrainEditorDoc* newDocument, const MapView* newMapView);
	~EditorTerrain ();

	bool getDataAt (const Vector2d& position, Data& data, bool heightOnly=false) const;

	void update (bool showProgress);
	void draw () const;

	void clearChunks ();
	void markDirty ();

	void showProfile (bool newShowProfile);

	bool buildHeightData (const ArrayList<MetaData>& metaDataList);
	real getHeightDataHeightAt (const Vector2d& position) const;

	void           setResolutionType (ResolutionType resolutionType);
	ResolutionType getResolutionType () const;

	void bakeTerrain ();

private:

	class Chunk;
	friend class Chunk;

private:

	bool          areValidChunkIndices (int x, int z) const;
	int           calculateChunkX (const Vector2d& position) const;
	int           calculateChunkZ (const Vector2d& position) const;
	const Chunk*  findChunk (int x, int z) const;
	const Chunk*  findChunk (const Vector2d& position) const;
	Chunk*        findChunk (int x, int z);
	Chunk*        findChunk (const Vector2d& position);
	bool          hasChunk (int x, int z) const;
	bool          hasChunk (const Vector2d& position) const;

	Chunk*        createChunk (int x, int z, bool noRiversOrRoads, bool noTexture);
	Chunk*        createChunk (const Vector2d& position, bool noRiversOrRoads, bool noTexture);
	void          removeChunk (int x, int z);
	void          removeChunk (const Vector2d& position);

	void          clearHeightDataChunkMap ();
	const Chunk*  findHeightDataChunk (int x, int z) const;
	const Chunk*  findHeightDataChunk (const Vector2d& position) const;

	bool isBelowWater (const Vector& position) const;
	void createBakedChunk (int x, int z, float& maximumHeight, float& chunkHeight, bool& hasWater);

private:

	EditorTerrain ();
	EditorTerrain (const EditorTerrain&);
	EditorTerrain& operator= (const EditorTerrain&);

private:

	typedef stdmap<uint32, Chunk*>::fwd ChunkMap;

private:

	//--
	TerrainEditorDoc*    document;
	const MapView*       mapView;

	//--
	TerrainGenerator::CreateChunkBuffer  createChunkBuffer;
	TerrainGenerator::CreateChunkBuffer  bakedCreateChunkBuffer;

	//--
	real                 chunkWidthInMeters_w;
	real                 distanceBetweenPoles_w;
	int                  originOffset;
	int                  upperPad;
	int                  numberOfPoles;

	//-- list of all created chunks
	ChunkMap*            m_chunkMap;

	//-- shader for invalid chunks
	Shader*              invalidShader;

	ArrayList<const TerrainGenerator::Boundary*> m_localWaterTableList;
	ArrayList<const AffectorRibbon*> m_affectorRibbonList;
	ChunkMap*            m_heightDataChunkMap;

	ResolutionType       m_resolutionType;
};

//-------------------------------------------------------------------

#endif
