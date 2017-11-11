//
// ClientProceduralTerrainAppearance.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//

#ifndef INCLUDED_ClientProceduralTerrainAppearance_H
#define INCLUDED_ClientProceduralTerrainAppearance_H

//-------------------------------------------------------------------

#include "clientTerrain/ClientRadialFloraManager.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Volume.h"
#include "sharedSynchronization/Gate.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainQuadTree.h"
#include "sharedThread/ThreadHandle.h"

class Appearance;
class AffectorRibbon;
class Camera;
class ClientDynamicRadialFloraManager;
class ClientLocalWaterManager;
class ClientStaticRadialFloraManager;
class EnvironmentBlock;
class GroundEnvironment;
class Iff;
class InteriorEnvironmentBlock;
class Object;
class Shader;
class ShaderTemplate;
class Texture;
class VertexBuffer;
class WaterManager;

#ifdef _DEBUG
//#define RIBBON_DEBUG_FEELERS
#endif // _DEBUG
//-------------------------------------------------------------------

class ClientProceduralTerrainAppearance : public ProceduralTerrainAppearance
{
public:

	enum RotationType
	{
		RT_0,
		RT_90,
		RT_180,
		RT_270,
		RT_invalid
	};

public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Cache is used to store arrays for reuse
	//
	class Cache;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// ShaderSet currently organizes vertex arrays by shader
	//
	class ShaderSet;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// ClientChunk defines a chunk of terrain
	//
	class ClientChunk;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// ChunkRequestInfo holds the data necessary for terrain generation
	//
	class ChunkRequestInfo
	{
	public:

		ChunkRequestInfo () :
			m_x (0),
			m_z (0),
			m_size (0),
			m_priority (0),
			m_chunk (0)
		{
		}

		ChunkRequestInfo (const int x, const int z, const int size, const int priority) :
			m_x (x),
			m_z (z),
			m_size (size),
			m_priority (priority),
			m_chunk (0)
		{
		}

		~ChunkRequestInfo ()
		{
			m_chunk = 0;
		}

		bool operator== (const ChunkRequestInfo& rhs) const
		{
			return m_x == rhs.m_x && m_z == rhs.m_z && m_size == rhs.m_size;
		}

		bool operator!= (const ChunkRequestInfo& rhs) const
		{
			return !operator== (rhs);
		}

	public:

		int m_x;
		int m_z;
		int m_size;
		int m_priority;
		mutable ClientChunk* m_chunk;
	};
	
	typedef stdvector<ChunkRequestInfo>::fwd ChunkRequestInfoList;
	typedef stdmultimap<int, ChunkRequestInfo>::fwd ChunkRequestInfoMap;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LevelOfDetail;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct DynamicFloraData
	{
	public:

		//-- terrain data
		PackedRgb color;

		//-- family data

		//-- family child data
		RadialGroup::FamilyChildData const * familyChildData;

	public:

		DynamicFloraData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct EnvironmentData
	{
	public:

		int familyId;

	public:

		EnvironmentData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// ShaderData is used to construct tiles
	//
	struct ShaderData;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// ShaderCache stores actual pointers to shaders and the blend tiles
	//
	class ShaderCache;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// CreateChunkData defines the data needed to generate a chunk in the terrain system
	//
	struct ClientCreateChunkData : public ProceduralTerrainAppearance::CreateChunkData
	{
	public:

		//-- used for caching shaders
		const ShaderCache*    shaderCache;

		bool                  isLeaf;

	private:

		ClientCreateChunkData ();
		ClientCreateChunkData (const ClientCreateChunkData& rhs);
		ClientCreateChunkData& operator= (const ClientCreateChunkData& rhs);

	public:

		explicit ClientCreateChunkData (TerrainGenerator::CreateChunkBuffer* newCreateChunkBuffer);
		virtual ~ClientCreateChunkData ();

		virtual void validate () const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Radar;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static bool               ms_multiThreadedTerrainGeneration;
	static bool               ms_enableRender;
#ifdef _DEBUG
	static bool               ms_noRenderTerrain;
#endif
	static Vector             ms_directionToLight;
	static bool               ms_showChunkExtents;
	static bool               ms_showPassable;
	static int                ms_maximumNumberOfInvalidatedNodes;

private:

	enum RequestThreadMode
	{
		//-- terrain is unlocked, and can be built as needed
		RTM0_normal,  

		//-- terrain has just been locked, and we're waiting for all pending requests to be cleared
		RTM1_clearingRequests,

		//-- terrain is rebuilding the dirty chunks
		RTM2_rebuilding
	};

private:  

	typedef stdvector<ClientRadialFloraManager*>::fwd FloraManagerList;
	typedef stdvector<Rectangle2d>::fwd RegionList;
	typedef stdvector<WaterManager*>::fwd WaterManagerList;

private:

	TerrainQuadTree*                 m_chunkTree;

	//-- flora and water managers
	FloraManagerList* const          m_floraManagerList;
	ClientDynamicRadialFloraManager* m_dynamicNearFloraManager;
	ClientDynamicRadialFloraManager* m_dynamicFarFloraManager;
	ClientStaticRadialFloraManager*  m_staticNonCollidableFloraManager;
	WaterManagerList*                m_waterManagerList;
	ClientLocalWaterManager*         m_localWaterManager;
	ShaderCache*                     m_shaderCache;

	//-- level of detail
	LevelOfDetail*                   m_levelOfDetail;
	mutable Volume                   m_worldFrustum;
	mutable Vector                   m_lastRefPosition_w;
	mutable bool                     m_levelOfDetailFillComplete;
	ChunkRequestInfoList* const           m_invalidateChunkRequestInfoList;
	RegionList* const                m_invalidateRegionList;

	//-- debugging
	mutable int                      m_numberOfChunksRendered;
	float                            m_totalChunkCreationTime;
	float                            m_totalChunkGenerationTime;
	int                              m_totalNumberOfChunksCreated;

	//-- multi-threaded terrain generation
	Mutex                            m_requestCriticalSection;
	Gate                             m_requestGate;
	ThreadHandle                     m_requestThread;
	RequestThreadMode                m_requestThreadMode;
	bool                             m_quitRequestThread;
	ChunkRequestInfoMap* const            m_pendingChunkRequestInfoMap;
	ChunkRequestInfoList* const           m_completedChunkRequestInfoList;
	bool                             m_lockTerrainLevelOfDetail;

#ifdef RIBBON_DEBUG_FEELERS
	ArrayList<const AffectorRibbon*>  m_debugRibbonAffectorList;
	ArrayList<Vector>          m_debugRibbonPanelVerts;
#endif // RIBBON_DEBUG_FEELERS

	//-- misc
	DPVS::Object*                    m_dpvsObject;
	Radar*                           m_radar;
	Radar*                           m_surveyRadar;



private:

	static void remove ();

private:

	ClientChunk*          createClientChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags);
	virtual void          createChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags);
	virtual void          removeUnnecessaryChunk ();
	virtual DPVS::Object* getDpvsObject() const;

	void                  retrieveCompletedChunkCreationRequests ();
	void                  insertChunkCreationRequests (const ChunkRequestInfoList& requestInfoList);
	void                  insertChunkRebuildRequests (const ChunkRequestInfoList& requestInfoList);
	void                  clearInvalidRegionList ();

	void                  initializeLevelOfDetail (int levels);
	bool                  selectActualLevelOfDetail (const Camera* camera, const Object* referenceObject, const Volume* frustum);

	void                  buildLocalWaterTable (const TerrainGenerator::Layer* layer);
	void                  buildLocalWaterTables ();

	void                  threadRoutine();

	const TerrainQuadTree* getChunkTree () const;
	TerrainQuadTree*       getChunkTree ();

	virtual const Chunk* findChunk (int x, int z, int chunkSize) const;
	virtual const Chunk* findFirstRenderableChunk (int x, int z) const;
	virtual void addChunk (Chunk* chunk, int chunkSize);

	LevelOfDetail&       getLevelOfDetail ();
	const LevelOfDetail& getLevelOfDetail () const;
	
	void                 calculateLod () const;

private:

	ClientProceduralTerrainAppearance ();
	ClientProceduralTerrainAppearance (const ClientProceduralTerrainAppearance&);
	ClientProceduralTerrainAppearance& operator= (const ClientProceduralTerrainAppearance&);

public:

	static AppearanceTemplate* create (const char* filename, Iff* iff);
	static void          debugReport(); //lint !e1511 members hides non-virt

	static void          install ();

	static void          setEnableRender(bool enableRender);
	static bool          getEnableRender() { return ms_enableRender; }

#ifdef _DEBUG
	static bool          getNoRenderTerrain() { return ms_noRenderTerrain; }
#endif

	static const Vector& getDirectionToLight ();
	static void          setDirectionToLight (const Vector& newDirectionToLight);
	static bool          getDot3Terrain ();
	static bool          getSpecularTerrainCapable();
	static bool          getSpecularTerrainEnabled();
	static bool          getDeferredSpecularTerrainEnabled();
	static void          setDeferredSpecularTerrainEnabled(bool b);
	static bool          getUseMultiThreadedTerrainGeneration();
	static void          setUseMultiThreadedTerrainGeneration(bool useMultiThreadedTerrainGeneration);

	static void          setShowChunkExtents (bool showChunkExtents);
	static bool          getShowChunkExtents ();

	static void          setRenderRunTimeRules (bool renderRunTimeRules);
	static bool          getRenderRunTimeRules ();
	static void          setRenderOverlappingRunTimeRules (bool renderOverlappingRunTimeRules);
	static bool          getRenderOverlappingRunTimeRules ();

	static void          setDynamicNearFloraEnabled (bool dynamicNearFloraEnabled);
	static bool          getDynamicNearFloraEnabled ();
	static float         getDynamicNearFloraDistance ();
	static void          setDynamicNearFloraDistance (float dynamicNearFloraDistance);
	static void          setDynamicFarFloraEnabled (bool dynamicFarFloraEnabled);
	static bool          getDynamicFarFloraEnabled ();
	static void          setStaticNonCollidableFloraEnabled (bool staticNonCollidableFloraEnabled);
	static bool          getStaticNonCollidableFloraEnabled ();
	static float         getStaticNonCollidableFloraDistance ();
	static void          setStaticNonCollidableFloraDistance (float staticNonCollidableFloraDistance);

	static float         getMaximumThresholdHigh ();
	static float         getMaximumThreshold     ();

	static void          setReferenceCamera (Camera const * referenceCamera);

	static void          setShowPassable(bool b);
	static bool          isShowPassable();

public:

	ClientProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate);
	virtual ~ClientProceduralTerrainAppearance ();

	virtual void         addToWorld();
	virtual void         removeFromWorld();
	virtual void         render() const;
	virtual float        alter (float time);
	virtual void         preRender (const Camera* camera) const;
	virtual void         postRender () const;
	virtual bool         collideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	virtual bool         approximateCollideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual int          getNumberOfChunks () const;
	virtual bool         hasHighLevelOfDetailTerrain (const Vector& position_o) const;
	virtual bool         terrainGenerationStabilized() const;

	virtual bool            getPauseEnvironment () const;
	virtual void            setPauseEnvironment (bool pauseEnvironment);
	virtual const PackedRgb getClearColor () const;
	virtual const PackedRgb getFogColor () const;
	virtual float           getFogDensity () const;
	virtual void            getTime (int& hour, int& minute) const;
	virtual float           getTime () const;
	virtual void            setTime (float time, bool force);
	virtual bool            isDay () const;
	virtual bool            isTimeLocked() const;

	virtual float           getHighLevelOfDetailThreshold () const;
	virtual void            setHighLevelOfDetailThreshold (float highLevelOfDetailThreshold);
	virtual float           getLevelOfDetailThreshold () const;
	virtual void            setLevelOfDetailThreshold (float levelOfDetailThreshold);

	virtual void            addClearNonCollidableFloraObject (const Object* object, const ClearFloraEntryList& clearFloraEntryList);
	virtual void            removeClearNonCollidableFloraObject (const Object* object);

	virtual void            getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const;

	virtual void            invalidateRegion (const Rectangle2d& extent2d);
	virtual void            debugDump () const;

	const EnvironmentBlock* getCurrentEnvironmentBlock () const;
	const InteriorEnvironmentBlock* getCurrentInteriorEnvironmentBlock () const;

	void                 setChunkLotShader (int chunkX, int chunkZ, const Shader* lotShader) const;
	void                 clearLotShaders ();

	bool                 findStaticNonCollidableFlora (float positionX, float positionZ, StaticFloraData& data, bool& floraAllowed) const;
	bool                 findDynamicNearFlora         (float positionX, float positionZ, DynamicFloraData& data, bool& floraAllowed) const;
	bool                 findDynamicFarFlora          (float positionX, float positionZ, DynamicFloraData& data, bool& floraAllowed) const;
	bool                 findEnvironment (const Vector& position, EnvironmentData& data) const;

	int                  getNumberOfChunksRendered () const;
	void                 _onChunkRendered() const { m_numberOfChunksRendered++; }

	virtual void         purgeChunks();

	struct RadarShaderInfo
	{
		Shader * m_shader;
		Vector   m_origin;
	};

	void                  createRadarShader       (const Vector & center, real range, int maxSize, RadarShaderInfo & rinfo, bool clip, const VectorArgb & clearColor, bool drawTerrain = true) const;

	virtual bool isPassableForceChunkCreation(const Vector& position) const;
	virtual bool getHeightForceChunkCreation (const Vector& position_o, float& height) const;
	virtual const BoxExtent * getChunkExtentForceChunkCreation (const Vector& position_o) const;
};

//-------------------------------------------------------------------

#endif
