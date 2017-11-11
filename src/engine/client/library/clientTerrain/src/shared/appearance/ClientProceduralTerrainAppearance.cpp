//===================================================================
//
// ClientProceduralTerrainAppearance.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientTerrain/ClientDynamicRadialFloraManager.h"
#include "clientTerrain/ClientGlobalWaterManager2.h"
#include "clientTerrain/ClientLocalWaterManager.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Cache.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ClientChunk.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_LevelOfDetail.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_Radar.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ShaderCache.h"
#include "clientTerrain/ClientStaticRadialFloraManager.h"
#include "clientTerrain/ClientTerrainSorter.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "clientTerrain/GroundEnvironment.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedThread/RunThread.h"
#include "sharedUtility/FileName.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include "dpvsObject.hpp"

#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <string>

//===================================================================

bool        ClientProceduralTerrainAppearance::ms_multiThreadedTerrainGeneration;

bool        ClientProceduralTerrainAppearance::ms_enableRender = true;
#ifdef _DEBUG
bool        ClientProceduralTerrainAppearance::ms_noRenderTerrain = false;
#endif

Vector      ClientProceduralTerrainAppearance::ms_directionToLight;
bool        ClientProceduralTerrainAppearance::ms_showChunkExtents;
bool        ClientProceduralTerrainAppearance::ms_showPassable;
int         ClientProceduralTerrainAppearance::ms_maximumNumberOfInvalidatedNodes;

namespace
{
#ifdef _DEBUG
	bool ms_reportCreationTime;
	bool ms_showBadWaterLocations;
	bool ms_showBadSlopeLocations;
	bool ms_renderClearFloraMap;
	const Shader* ms_badLocationShader = 0;
#endif

	bool  ms_renderRunTimeRules = false;
	bool  ms_renderOverlappingRunTimeRules = false;
	bool  ms_staticNonCollidableFloraEnabled = true;
	float ms_staticNonCollidableFloraDistance = 0.f;
	bool  ms_dynamicFarFloraEnabled = true;
	float ms_dynamicFarFloraDistance = 0.f;
	bool  ms_dynamicNearFloraEnabled = true;
	float ms_dynamicNearFloraDistance = 0.f;
	int   ms_maximumNumberOfInvalidatedNodes = 0;
	bool ms_specularTerrainEnabled = true;
	bool ms_deferredSpecularTerrainEnabled = true; // spec terrain changes are deferred until the terrain is re-created

	float ms_maximumThresholdHigh = 0.f;
	float ms_maximumThreshold     = 1.0f;

	const Tag TAG_DOT3 = TAG (D,O,T,3);

	Camera const * ms_referenceCamera;
}

//===================================================================
//
// ClientProceduralTerrainAppearance::ClientCreateChunkData
//

ClientProceduralTerrainAppearance::ClientCreateChunkData::ClientCreateChunkData (TerrainGenerator::CreateChunkBuffer* newCreateChunkBuffer) :
	ProceduralTerrainAppearance::CreateChunkData (newCreateChunkBuffer),
	shaderCache (0),
	isLeaf (false)
{
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::ClientCreateChunkData::~ClientCreateChunkData ()
{
	shaderCache = 0;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::ClientCreateChunkData::validate () const
{
	CreateChunkData::validate ();

	NOT_NULL (shaderCache);
}

//===================================================================

ClientProceduralTerrainAppearance::DynamicFloraData::DynamicFloraData () :
	color (PackedRgb::solidWhite),
	familyChildData ()
{
}

//===================================================================

ClientProceduralTerrainAppearance::EnvironmentData::EnvironmentData () :
	familyId (0)
{
}

//===================================================================
//
// ClientProceduralTerrainAppearance
//

void ClientProceduralTerrainAppearance::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientProceduralTerrainAppearance::install\n"));

	ms_multiThreadedTerrainGeneration = ConfigClientTerrain::getTerrainMultiThreaded ();

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_reportCreationTime, "ClientTerrain", "reportCreationTime");
	DebugFlags::registerFlag (ms_noRenderTerrain, "ClientTerrain", "noRenderTerrain");
	DebugFlags::registerFlag (ms_renderRunTimeRules, "ClientTerrain", "renderRunTimeRules");
	DebugFlags::registerFlag (ms_renderOverlappingRunTimeRules, "ClientTerrain", "renderOverlappingRunTimeRules");
	DebugFlags::registerFlag (ms_showBadWaterLocations, "ClientTerrain", "showBadWaterLocations");
	DebugFlags::registerFlag (ms_showBadSlopeLocations, "ClientTerrain", "showBadSlopeLocations");
	DebugFlags::registerFlag (ms_renderClearFloraMap, "ClientTerrain", "renderClearFloraMap");
	ms_badLocationShader = ShaderTemplateList::fetchShader ("shader/placement_red.sht");
#endif
	
	if (MemoryManager::getLimit () >= 750)
	{
		ms_maximumThresholdHigh = 30.f;
		ms_maximumThreshold     = 8.0f;
	}
	else
	{
		if (MemoryManager::getLimit () > 562)
		{
			ms_maximumThresholdHigh = 16.f;
			ms_maximumThreshold     = 5.0f;
		}
		else
		{
			if (MemoryManager::getLimit () > 375)
			{
				ms_maximumThresholdHigh = 12.f;
				ms_maximumThreshold     = 4.0f;
			}
			else
			{
				if (MemoryManager::getLimit () > 262)
				{
					ms_maximumThresholdHigh = 8.f;
					ms_maximumThreshold     = 3.0f;
				}
				else
				{
					ms_maximumThresholdHigh = 4.f;
					ms_maximumThreshold     = 2.0f;
				}
			}
		}
	}

	DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportInstall (), ("Terrain: ms_maximumThresholdHigh = %1.2f\n", ms_maximumThresholdHigh));

	ms_staticNonCollidableFloraEnabled = ConfigClientTerrain::getStaticNonCollidableFloraEnabled ();
	ms_staticNonCollidableFloraDistance = ConfigClientTerrain::getStaticNonCollidableFloraDistanceDefault ();
	ms_dynamicNearFloraEnabled = ConfigClientTerrain::getDynamicNearFloraEnabled ();
	ms_dynamicNearFloraDistance = ConfigClientTerrain::getDynamicNearFloraDistanceDefault ();
	ms_dynamicFarFloraEnabled = ConfigClientTerrain::getDynamicFarFloraEnabled ();

	LocalMachineOptionManager::registerOption (ms_staticNonCollidableFloraEnabled, "ClientTerrain", "staticNonCollidableFloraEnabled");
	LocalMachineOptionManager::registerOption (ms_staticNonCollidableFloraDistance, "ClientTerrain", "staticNonCollidableFloraDistance");
	LocalMachineOptionManager::registerOption (ms_dynamicFarFloraEnabled, "ClientTerrain", "dynamicFarFloraEnabled");
	LocalMachineOptionManager::registerOption (ms_dynamicNearFloraEnabled, "ClientTerrain", "dynamicNearFloraEnabled");
	LocalMachineOptionManager::registerOption (ms_dynamicNearFloraDistance, "ClientTerrain", "dynamicNearFloraDistance");
	LocalMachineOptionManager::registerOption (ms_specularTerrainEnabled, "ClientTerrain", "specularTerrainEnabled");
	ms_deferredSpecularTerrainEnabled = ms_specularTerrainEnabled;
	LocalMachineOptionManager::registerOption (ms_deferredSpecularTerrainEnabled, "ClientTerrain", "deferredSpecularTerrainEnabled");

	ExitChain::add (remove, "ClientProceduralTerrainAppearance::remove");
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_reportCreationTime);
	DebugFlags::unregisterFlag (ms_noRenderTerrain);
	DebugFlags::unregisterFlag (ms_renderRunTimeRules);
	DebugFlags::unregisterFlag (ms_renderOverlappingRunTimeRules);
	DebugFlags::unregisterFlag (ms_showBadWaterLocations);
	DebugFlags::unregisterFlag (ms_showBadSlopeLocations);
	DebugFlags::unregisterFlag (ms_renderClearFloraMap);
	ms_badLocationShader->release ();
#endif
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setEnableRender(bool const enableRender)
{
	ms_enableRender = enableRender;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setDynamicNearFloraEnabled (bool dynamicNearFloraEnabled)
{
	ms_dynamicNearFloraEnabled = dynamicNearFloraEnabled;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getDynamicNearFloraEnabled ()
{
	return ms_dynamicNearFloraEnabled;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setDynamicNearFloraDistance (float const dynamicNearFloraDistance)
{
	ms_dynamicNearFloraDistance = dynamicNearFloraDistance;
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getDynamicNearFloraDistance ()
{
	return ms_dynamicNearFloraDistance;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setDynamicFarFloraEnabled (bool dynamicFarFloraEnabled)
{
	ms_dynamicFarFloraEnabled = dynamicFarFloraEnabled;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getDynamicFarFloraEnabled ()
{
	return ms_dynamicFarFloraEnabled;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setStaticNonCollidableFloraEnabled (bool staticNonCollidableFloraEnabled)
{
	ms_staticNonCollidableFloraEnabled = staticNonCollidableFloraEnabled;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getStaticNonCollidableFloraEnabled ()
{
	return ms_staticNonCollidableFloraEnabled;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setStaticNonCollidableFloraDistance (float const staticNonCollidableFloraDistance)
{
	ms_staticNonCollidableFloraDistance = staticNonCollidableFloraDistance;
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getStaticNonCollidableFloraDistance ()
{
	return ms_staticNonCollidableFloraDistance;
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getMaximumThresholdHigh ()
{
	return ms_maximumThresholdHigh;
}

//----------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getMaximumThreshold ()
{
	return ms_maximumThreshold;
}

//----------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setReferenceCamera (Camera const * const referenceCamera)
{
	ms_referenceCamera = referenceCamera;
}

//===================================================================
// PUBLIC ClientProceduralTerrainAppearance
//===================================================================

ClientProceduralTerrainAppearance::ClientProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate) :
	ProceduralTerrainAppearance (appearanceTemplate),
	m_chunkTree (0),
	m_floraManagerList (NON_NULL (new FloraManagerList)),
	m_dynamicNearFloraManager (0),
	m_dynamicFarFloraManager (0),
	m_staticNonCollidableFloraManager (0),
	m_waterManagerList (NON_NULL (new WaterManagerList)),
	m_localWaterManager (0),
	m_shaderCache (0),
	m_levelOfDetail (0),
	m_worldFrustum (6),
	m_lastRefPosition_w(0,0,0),
	m_levelOfDetailFillComplete (false),
	m_invalidateChunkRequestInfoList (NON_NULL (new ChunkRequestInfoList)),
	m_invalidateRegionList (NON_NULL (new RegionList)),
	m_numberOfChunksRendered (0),
	m_totalChunkCreationTime (0),
	m_totalChunkGenerationTime (0),
	m_totalNumberOfChunksCreated (0),
	m_requestCriticalSection (),
	m_requestGate (false),
	m_requestThread (0),
	m_requestThreadMode (RTM0_normal),
	m_quitRequestThread (false),
	m_pendingChunkRequestInfoMap (NON_NULL (new ChunkRequestInfoMap)),
	m_completedChunkRequestInfoList (NON_NULL (new ChunkRequestInfoList)),
	m_lockTerrainLevelOfDetail (false),
#ifdef RIBBON_DEBUG_FEELERS
	m_debugRibbonAffectorList (),
	m_debugRibbonPanelVerts (),
#endif // RIBBON_DEBUG_FEELERS
	
	m_dpvsObject(NULL),
	m_radar (0),
	m_surveyRadar (0)
{
	// set spec if deferred value is different
	if(ms_deferredSpecularTerrainEnabled != ms_specularTerrainEnabled) 
	{
		ms_specularTerrainEnabled = ms_deferredSpecularTerrainEnabled;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- to ensure that memory isn't claimed on the client for server terrain (this must be first)
	{
		Cache::install ();
		ShaderSet::install ();
		ClientChunk::install ();
		ClientTerrainSorter::install ();
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	setClient ();

	const TerrainGenerator* const generator                    = appearanceTemplate->getTerrainGenerator ();
	const float             mapWidthInMeters                   = appearanceTemplate->getMapWidthInMeters ();
	const float             chunkWidthInMeters                 = appearanceTemplate->getChunkWidthInMeters ();
	const float             nonCollidableMinimumDistance       = appearanceTemplate->getNonCollidableMinimumDistance ();
	const float             nonCollidableMaximumDistance       = appearanceTemplate->getNonCollidableMaximumDistance ();
	const float             nonCollidableTileSize              = appearanceTemplate->getNonCollidableTileSize ();
	const float             nonCollidableTileBorder            = appearanceTemplate->getNonCollidableTileBorder ();
	const uint32            nonCollidableSeed                  = appearanceTemplate->getNonCollidableSeed ();
	const float             radialMinimumDistance              = appearanceTemplate->getRadialMinimumDistance ();
	const float             radialMaximumDistance              = appearanceTemplate->getRadialMaximumDistance ();
	const float             radialTileSize                     = appearanceTemplate->getRadialTileSize ();
	const float             radialTileBorder                   = appearanceTemplate->getRadialTileBorder ();
	const uint32            radialSeed                         = appearanceTemplate->getRadialSeed ();
	const float             farRadialMinimumDistance           = appearanceTemplate->getFarRadialMinimumDistance ();
	const float             farRadialMaximumDistance           = appearanceTemplate->getFarRadialMaximumDistance ();
	const float             farRadialTileSize                  = appearanceTemplate->getFarRadialTileSize ();
	const float             farRadialTileBorder                = appearanceTemplate->getFarRadialTileBorder ();
	const uint32            farRadialSeed                      = appearanceTemplate->getFarRadialSeed ();
	const bool              useGlobalWaterTable                = appearanceTemplate->getUseGlobalWaterTable ();
	const char* const       globalWaterTableShaderTemplateName = appearanceTemplate->getGlobalWaterTableShaderTemplateName ();
	const float             globalWaterTableShaderSize         = appearanceTemplate->getGlobalWaterTableShaderSize ();
	const float             globalWaterTableHeight             = appearanceTemplate->getGlobalWaterTableHeight ();
	const int               numberOfTilesPerChunk              = appearanceTemplate->getNumberOfTilesPerChunk ();
	const float             environmentCycleTime               = appearanceTemplate->getEnvironmentCycleTime ();

	const_cast<ShaderGroup&> (generator->getShaderGroup ()).loadSurfaceProperties ();

	//--
	m_shaderCache = new ShaderCache (generator->getShaderGroup ());

	if (ConfigClientTerrain::getEnableFlora ())
	{
		//-- first far radial
		if (generator->getRadialGroup ().getNumberOfFamilies () > 0)
		{
			if (farRadialMaximumDistance > farRadialMinimumDistance)
			{
				ms_dynamicFarFloraDistance = farRadialMaximumDistance;

				m_dynamicFarFloraManager = new ClientDynamicRadialFloraManager (*this, ms_dynamicFarFloraEnabled, farRadialMinimumDistance, ms_dynamicFarFloraDistance, &ClientProceduralTerrainAppearance::findDynamicFarFlora, false);
				m_dynamicFarFloraManager->setDebugName ("FloraDynamicFar");
				m_dynamicFarFloraManager->initialize (farRadialTileSize, farRadialTileBorder, farRadialSeed);
				m_floraManagerList->push_back (m_dynamicFarFloraManager);
			}
		}

		//-- then non-collidable
		if (generator->getFloraGroup ().getNumberOfFamilies () > 0)
		{
			if (nonCollidableMaximumDistance > nonCollidableMinimumDistance)
			{
				m_staticNonCollidableFloraManager = new ClientStaticRadialFloraManager (*this, ms_staticNonCollidableFloraEnabled, nonCollidableMinimumDistance, ms_staticNonCollidableFloraDistance, &ClientProceduralTerrainAppearance::findStaticNonCollidableFlora);
				m_staticNonCollidableFloraManager->setDebugName ("FloraStaticNonCollidable");
				m_staticNonCollidableFloraManager->initialize (nonCollidableTileSize, nonCollidableTileBorder, nonCollidableSeed);
				m_floraManagerList->push_back (m_staticNonCollidableFloraManager);
			}
		}

		//-- then near radial
		if (generator->getRadialGroup ().getNumberOfFamilies () > 0)
		{
			if (radialMaximumDistance > radialMinimumDistance)
			{
				m_dynamicNearFloraManager = new ClientDynamicRadialFloraManager (*this, ms_dynamicNearFloraEnabled, radialMinimumDistance, ms_dynamicNearFloraDistance, &ClientProceduralTerrainAppearance::findDynamicNearFlora, true);
				m_dynamicNearFloraManager->setDebugName ("FloraDynamicNear");
				m_dynamicNearFloraManager->initialize (radialTileSize, radialTileBorder, radialSeed);
				m_floraManagerList->push_back (m_dynamicNearFloraManager);
			}
		}
	}

	//-- build the tree
	m_chunkTree = new TerrainQuadTree (static_cast<int> (mapWidthInMeters / chunkWidthInMeters), chunkWidthInMeters);

	//-- build water
	if (GraphicsOptionTags::get (TAG3 (D,X,9)))
	{
		DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportLogPrint () && useGlobalWaterTable, ("ClientGlobalWaterManager2: global water table\n"));
		DEBUG_REPORT_LOG (ConfigSharedTerrain::getDebugReportLogPrint () && !useGlobalWaterTable, ("ClientGlobalWaterManager2: no global water table\n"));

		if (useGlobalWaterTable)
		{
			m_waterManagerList->push_back (new ClientGlobalWaterManager2 (*this, mapWidthInMeters, FileName (FileName::P_shader, globalWaterTableShaderTemplateName), globalWaterTableShaderSize, globalWaterTableHeight, 0));
		}

		//-- build local water tables
		m_localWaterManager = new ClientLocalWaterManager (*this);
		m_waterManagerList->push_back (m_localWaterManager);
	}

	buildLocalWaterTables ();

	//-- create the environment
	GroundEnvironment::getInstance().setClientProceduralTerrainAppearance(this, environmentCycleTime);
	ClientChunk::setTerrainCloudShader(GroundEnvironment::getInstance().getTerrainCloudShader());

	// create the thread to build the terrain
	MemberFunctionThreadZero<ClientProceduralTerrainAppearance> * memberFunction = new MemberFunctionThreadZero<ClientProceduralTerrainAppearance>("ClientTerrain", *this, &ClientProceduralTerrainAppearance::threadRoutine);
	m_requestThread = MemberFunctionThreadZero<ClientProceduralTerrainAppearance>::Handle (memberFunction);
	m_requestThread->setPriority(Thread::kNormal);

	m_radar       = new Radar (*this, *getChunkTree (), numberOfTilesPerChunk, originOffset);
	m_surveyRadar = new Radar (*this, *getChunkTree (), numberOfTilesPerChunk, originOffset);

	m_levelOfDetail = new LevelOfDetail (*this);

	m_dpvsObject = RenderWorld::createUnboundedObject(this);
}  //lint !e429  //-- memberFunction has not been freed or returned

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::~ClientProceduralTerrainAppearance ()
{
	m_levelOfDetail->removeAllObjectsFromWorld(getChunkTree()->getTopNode());

	delete m_levelOfDetail;
	m_levelOfDetail = 0;

	delete m_radar;
	m_radar = 0;

	delete m_surveyRadar;
	m_surveyRadar = 0;

	// wait for the thread to die
	m_requestCriticalSection.enter ();
		m_quitRequestThread = true;
		m_requestGate.open();
	m_requestCriticalSection.leave ();
	m_requestThread->wait();

	// free up the memory used to communicate with the thread
	m_pendingChunkRequestInfoMap->clear();
	for (ChunkRequestInfoList::iterator iter = m_completedChunkRequestInfoList->begin (); iter != m_completedChunkRequestInfoList->end (); ++iter)
	{
		const ChunkRequestInfo& requestInfo = (*iter);
		if (requestInfo.m_chunk)
		{
			delete requestInfo.m_chunk;
			requestInfo.m_chunk = 0;
		}
	}

	//-- delete shader cache
	delete m_shaderCache;
	m_shaderCache = 0;

	//-- delete flora managers
	m_dynamicNearFloraManager         = 0;
	m_dynamicFarFloraManager          = 0;
	m_staticNonCollidableFloraManager = 0;

	std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), PointerDeleter ());
	delete m_floraManagerList;

	//-- delete the water
	std::for_each (m_waterManagerList->begin (), m_waterManagerList->end (), PointerDeleter ());
	delete m_waterManagerList;

	GroundEnvironment::destroyInstance();
	ClientChunk::setTerrainCloudShader(0);

	delete m_pendingChunkRequestInfoMap;

	delete m_completedChunkRequestInfoList;

	IGNORE_RETURN (m_dpvsObject->release());
	m_dpvsObject = NULL;

	delete m_invalidateChunkRequestInfoList;
	delete m_invalidateRegionList;

	delete m_chunkTree;
	m_chunkTree = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- to ensure that memory isn't claimed on the client for server terrain (this must be last)
	{
		ClientTerrainSorter::remove ();
		ClientChunk::remove ();
		ShaderSet::remove ();
		Cache::remove ();
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::addToWorld()
{
	ProceduralTerrainAppearance::addToWorld();
}

void ClientProceduralTerrainAppearance::removeFromWorld()
{
	ProceduralTerrainAppearance::removeFromWorld();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setDirectionToLight (const Vector& newDirectionToLight)
{
	ms_directionToLight = newDirectionToLight;
}

//-------------------------------------------------------------------

const Vector& ClientProceduralTerrainAppearance::getDirectionToLight ()
{
	return ms_directionToLight;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getDot3Terrain ()
{
	return GraphicsOptionTags::get (TAG_DOT3) && Graphics::getShaderCapability () >= ShaderCapability(1,1) && MemoryManager::getLimit () >= 375;
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getSpecularTerrainCapable()
{
	return ClientProceduralTerrainAppearance::getDot3Terrain() && Graphics::getShaderCapability () >= ShaderCapability(2,0) && Graphics::getVideoMemoryInMegabytes() >= 100 && MemoryManager::getLimit () >= 375;
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getSpecularTerrainEnabled()
{
	return getSpecularTerrainCapable() && ms_specularTerrainEnabled;
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getDeferredSpecularTerrainEnabled()
{
	return getSpecularTerrainCapable() && ms_deferredSpecularTerrainEnabled;
}

//----------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setDeferredSpecularTerrainEnabled(bool b)
{
	ms_deferredSpecularTerrainEnabled = b;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setRenderRunTimeRules (bool renderRunTimeRules)
{
	ms_renderRunTimeRules = renderRunTimeRules;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getRenderRunTimeRules ()
{
	return ms_renderRunTimeRules;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setRenderOverlappingRunTimeRules (bool renderOverlappingRunTimeRules)
{
	ms_renderOverlappingRunTimeRules = renderOverlappingRunTimeRules;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getRenderOverlappingRunTimeRules ()
{
	return ms_renderOverlappingRunTimeRules;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setShowChunkExtents (const bool showChunkExtents)
{
	ms_showChunkExtents = showChunkExtents;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getShowChunkExtents()
{
	return ms_showChunkExtents;
}

//----------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setShowPassable(bool b)
{
	ms_showPassable = b;
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::isShowPassable()
{
	return ms_showPassable;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getUseMultiThreadedTerrainGeneration()
{
	return ms_multiThreadedTerrainGeneration;
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::setUseMultiThreadedTerrainGeneration(bool b)
{
	ms_multiThreadedTerrainGeneration = b;
}

//-----------------------------------------------------------------

int ClientProceduralTerrainAppearance::getNumberOfChunksRendered () const
{
	return m_numberOfChunksRendered;
}

//-----------------------------------------------------------------

ClientProceduralTerrainAppearance::LevelOfDetail& ClientProceduralTerrainAppearance::getLevelOfDetail ()
{
	NOT_NULL (m_levelOfDetail);
	return *m_levelOfDetail;
}

//-----------------------------------------------------------------

const ClientProceduralTerrainAppearance::LevelOfDetail& ClientProceduralTerrainAppearance::getLevelOfDetail () const
{
	NOT_NULL (m_levelOfDetail);
	return *m_levelOfDetail;
}

//-----------------------------------------------------------------

ClientProceduralTerrainAppearance::ClientChunk *ClientProceduralTerrainAppearance::createClientChunk (const int x, const int z, const int chunkSize, unsigned hasLargerNeighborFlags)
{
	PerformanceTimer timer;

	timer.start ();

	const TerrainGenerator* terrainGenerator      = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();
	const int               numberOfTilesPerChunk = proceduralTerrainAppearanceTemplate->getNumberOfTilesPerChunk ();
	const float             chunkWidthInMeters    = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();
	const float             tileWidthInMeters     = proceduralTerrainAppearanceTemplate->getTileWidthInMeters ();

	//-- chunk does not exist -- it needs to be created. find out what map data i'll need to ask the generator for
	const float  distanceBetweenPoles = tileWidthInMeters * 0.5f * chunkSize;

	const Vector start (
		static_cast<float> (x) * chunkWidthInMeters - static_cast<float>(originOffset) * distanceBetweenPoles,
		0.0f,
		static_cast<float> (z) * chunkWidthInMeters - static_cast<float>(originOffset) * distanceBetweenPoles);

	ClientChunk* chunk = new ClientChunk (*this);
	chunk->setOwner(getOwner());

	//-- setup data needed to create a chunk
	ClientCreateChunkData createChunkData (&createChunkBuffer);

	createChunkData.chunkX                  = x;
	createChunkData.chunkZ                  = z;
	createChunkData.start                   = start;
	createChunkData.numberOfTilesPerChunk   = numberOfTilesPerChunk;
	createChunkData.chunkWidthInMeters      = chunkWidthInMeters * chunkSize;
	createChunkData.shaderCache             = m_shaderCache;
	createChunkData.shaderGroup             = &terrainGenerator->getShaderGroup ();
	createChunkData.floraGroup              = &terrainGenerator->getFloraGroup ();
	createChunkData.radialGroup             = &terrainGenerator->getRadialGroup ();
	createChunkData.environmentGroup        = &terrainGenerator->getEnvironmentGroup ();
	createChunkData.fractalGroup            = &terrainGenerator->getFractalGroup ();
	createChunkData.bitmapGroup             = &terrainGenerator->getBitmapGroup ();
	createChunkData.originOffset            = originOffset;
	createChunkData.numberOfPoles           = numberOfPoles;
	createChunkData.isLeaf                  = chunkSize == 1;

	if (hasLargerNeighborFlags)
	{
		createChunkData.hasLargerNeighborFlags = static_cast<uint8>(hasLargerNeighborFlags);
	}

	//-- ask the generator to fill out this area
	TerrainGenerator::GeneratorChunkData generatorChunkData(proceduralTerrainAppearanceTemplate->getLegacyMode());

	generatorChunkData.shaderGroup          = createChunkData.shaderGroup;
	generatorChunkData.floraGroup           = createChunkData.floraGroup;
	generatorChunkData.radialGroup          = createChunkData.radialGroup;
	generatorChunkData.environmentGroup     = createChunkData.environmentGroup;
	generatorChunkData.fractalGroup         = createChunkData.fractalGroup;
	generatorChunkData.bitmapGroup          = createChunkData.bitmapGroup;
	generatorChunkData.heightMap            = &createChunkData.createChunkBuffer->heightMap;
	generatorChunkData.colorMap             = &createChunkData.createChunkBuffer->colorMap;
	generatorChunkData.shaderMap            = &createChunkData.createChunkBuffer->shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &createChunkData.createChunkBuffer->floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &createChunkData.createChunkBuffer->floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap  = &createChunkData.createChunkBuffer->floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap   = &createChunkData.createChunkBuffer->floraDynamicFarMap;
	generatorChunkData.environmentMap       = &createChunkData.createChunkBuffer->environmentMap;
	generatorChunkData.vertexPositionMap    = &createChunkData.createChunkBuffer->vertexPositionMap;
	generatorChunkData.vertexNormalMap      = &createChunkData.createChunkBuffer->vertexNormalMap;
	generatorChunkData.excludeMap           = &createChunkData.createChunkBuffer->excludeMap;
	generatorChunkData.passableMap          = &createChunkData.createChunkBuffer->passableMap;
	generatorChunkData.start                = start;
	generatorChunkData.originOffset         = originOffset;
	generatorChunkData.numberOfPoles        = numberOfPoles;
	generatorChunkData.upperPad             = upperPad;
	generatorChunkData.distanceBetweenPoles = distanceBetweenPoles;

	terrainGenerator->generateChunk (generatorChunkData);

	timer.stop ();

	m_totalChunkGenerationTime += timer.getElapsedTime ();

#ifdef _DEBUG
	const float generationTime = timer.getElapsedTime ();
#endif

	timer.start ();
	//-- create the chunk using the data the generator created
	chunk->create (createChunkData);

	timer.stop ();
	++m_totalNumberOfChunksCreated;
	m_totalChunkCreationTime += timer.getElapsedTime ();

#ifdef _DEBUG
	const float creationTime = timer.getElapsedTime ();

	DEBUG_REPORT_PRINT (ms_reportCreationTime, ("g=%1.3f  c=%1.3f\n", generationTime, creationTime));
#endif

	return chunk;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::createChunk (const int x, const int z, const int chunkSize, unsigned hasLargerNeighborFlags)
{
	//-- make sure indices are valid
	if (!areValidChunkIndices (x, z))
		return;

	//-- see if the chunk already exists
	if (findChunk (x, z, chunkSize))
		return;

	// build the chunk immediately
	ClientChunk* chunk = createClientChunk(x, z, chunkSize, hasLargerNeighborFlags);
	createFlora (chunk);

	// add it to the terrain
	IGNORE_RETURN (getChunkTree ()->addChunk (chunk, chunkSize));
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::alter (const float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE ("ClientProceduralTerrainAppearance::alter");

	//-- Chain up
	ProceduralTerrainAppearance::alter (elapsedTime);

	//-- Verify that we have only one reference object
	DEBUG_FATAL (getNumberOfReferenceObjects () > 1, ("ClientProceduralTerrainAppearance::alter - there should really be only one reference object\n"));

	if (getRenderedThisFrame ())
		calculateLod ();

	//-- Update terrain rebuild requests
	clearInvalidRegionList ();

	//-- Service the terrain generation thread
	retrieveCompletedChunkCreationRequests ();

	//-- Alter the shaders (probably not needed because no terrain shaders should animate)
	NOT_NULL (m_shaderCache);
	m_shaderCache->alter (elapsedTime);

	//-- Alter all of the flora lists
	{
		PROFILER_AUTO_BLOCK_DEFINE ("terrain update flora");
		std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), VoidBindSecond (VoidMemberFunction (&ClientRadialFloraManager::alter), elapsedTime));
	}

	//-- Update the water system
	{
		PROFILER_AUTO_BLOCK_DEFINE ("terrain update water");
		std::for_each (m_waterManagerList->begin (), m_waterManagerList->end (), VoidBindSecond (VoidMemberFunction (&WaterManager::alter), elapsedTime));
	}

	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::calculateLod () const
{
	//-- we at least need a camera
	if (!ms_referenceCamera)
	{
		NOT_NULL (ms_referenceCamera);
		return;
	}

	//-- client level of detail is based on the first reference object, which is generally the player
	const Object* const referenceObject = getNumberOfReferenceObjects () == 0 ? ms_referenceCamera : getReferenceObject (0);
	if (!referenceObject)
	{
		NOT_NULL (referenceObject);
		return;
	}

	PROFILER_AUTO_BLOCK_DEFINE ("ClientProceduralTerrainAppearance::calculateLod");

	//-- base level of detail selection on whether or not the camera has moved
	const Transform& transform = ms_referenceCamera->getTransform_o2w ();

	//-- frustum volume in world space
	m_worldFrustum.transform (ms_referenceCamera->getFrustumVolume (), ms_referenceCamera->getTransform_o2w ());

	if (!m_lockTerrainLevelOfDetail)
	{
		//-- fill out the terrain in the frustum
		NOT_NULL (m_levelOfDetail);

		//m_levelOfDetailFillComplete=false; // DEBUG

		Vector refPosition;

		refPosition.x = static_cast<float>(floor(transform.getPosition_p().x) + .5f);
		refPosition.y = static_cast<float>(floor(transform.getPosition_p().y) + .5f);
		refPosition.z = static_cast<float>(floor(transform.getPosition_p().z) + .5f);

		if (m_levelOfDetail->isDirty () || !m_levelOfDetailFillComplete || refPosition != m_lastRefPosition_w)
		{
			m_lastRefPosition_w = refPosition;

			//-- m_levelOfDetailFillComplete indicates that all potentially viewable terrain, from the camera position, has been filled in
			m_levelOfDetailFillComplete = const_cast<ClientProceduralTerrainAppearance*> (this)->selectActualLevelOfDetail (ms_referenceCamera, referenceObject, &m_worldFrustum);
		}
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::preRender (const Camera* camera) const
{
	PROFILER_AUTO_BLOCK_DEFINE ("ClientProceduralTerrainAppearance::preRender");

	m_numberOfChunksRendered = 0;

	bool traverseTree = false;

	if (ms_showChunkExtents)
	{
		traverseTree=true;
	}
#ifdef _DEBUG
	//traverseTree=true;
	if (ms_badLocationShader && (ms_showBadWaterLocations || ms_showBadSlopeLocations))
	{
		traverseTree=true;
	}
#endif

	if (traverseTree)
	{
		//-- frustum volume in world space (used for culling)
		m_worldFrustum.transform (ms_referenceCamera->getFrustumVolume (), ms_referenceCamera->getTransform_o2w ());

 		TerrainQuadTree::ConstIterator node_iter (getChunkTree ()->getTopNode ());

		const TerrainQuadTree::Node * snode = 0;

		//-- iterate through the quadtree
		while ((snode = node_iter.getCurNode ()) != 0)
		{
			//-- test this heirarchical node of the quadtree for intersection with
			//-- the frustum.  if it does not intersect, then don't process this
			//-- node or its children.

			if (snode->isOutsideBuildRange () || !m_worldFrustum.intersects (snode->getBoxExtent ().getSphere ()))
			{
				IGNORE_RETURN (node_iter.advance ());
				continue;
			}

			//-- push non-leaf nodes onto the stack and continue the iteration
			if (!snode->isSelectedForRender ())
			{
				IGNORE_RETURN (node_iter.descend ());
				continue;
			}

			if (ms_showChunkExtents && snode->getHasAnyLargerNeighbors ())
			{
				Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
				Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
				Vector mv = snode->getBoxExtent ().getMax ();
				mv.y += 32;
				Graphics::drawBox (snode->getBoxExtent ().getMin (), mv, VectorArgb::solidBlack);
			}

			//-- this should not happen, unless the terrain generation frustum test was wack.
			if (snode->getChunk() == 0)
			{
	//			DEBUG_FATAL (true, ("hole in terrain\n"));
				IGNORE_RETURN (node_iter.advance ());
				continue;
			}

			//--
			//-- the node has a chunk and it should now be rendered.
			//--
		#ifdef _DEBUG
			if (ms_enableRender && !ms_noRenderTerrain)
			{
				ClientProceduralTerrainAppearance::ClientChunk* const clientChunk = const_cast<ClientProceduralTerrainAppearance::ClientChunk*> (safe_cast<const ClientProceduralTerrainAppearance::ClientChunk*> (snode->getChunk ()));
				DEBUG_FATAL(!clientChunk->isObjectInWorld(), ("Chunk is visible but not in world."));

				if (ms_showBadWaterLocations && ms_badLocationShader && snode->getSize () == 1 && getWater (clientChunk->getChunkX (), clientChunk->getChunkZ ()))
					clientChunk->setLotShader (ms_badLocationShader);

				if (ms_showBadSlopeLocations && ms_badLocationShader && snode->getSize () == 1 && getSlope (clientChunk->getChunkX (), clientChunk->getChunkZ ()))
					clientChunk->setLotShader (ms_badLocationShader);

				if (ms_showChunkExtents)
				{
					Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
					Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
					Graphics::drawBox (snode->getBoxExtent ().getMin (), snode->getBoxExtent ().getMax (),
						snode->getSize () == 1 ?
						VectorArgb::solidRed :
					snode->getSize () == 2 ?
						VectorArgb::solidMagenta :
					snode->getSize () == 4 ?
						VectorArgb::solidBlue :
					snode->getSize () == 8 ?
						VectorArgb::solidCyan :
					snode->getSize () == 16 ?
						VectorArgb::solidGreen :
					VectorArgb::solidYellow);

				}
			}
		#endif

			IGNORE_RETURN (node_iter.advance ());
		}
	}

	{
		PROFILER_AUTO_BLOCK_DEFINE ("terrain preRender flora");
		std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), VoidBindSecond (VoidMemberFunction (&ClientRadialFloraManager::preRender), camera));
	}
}

//-------------------------------------------------------------------

DPVS::Object* ClientProceduralTerrainAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::render () const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE ("ClientProceduralTerrainAppearance::render");

	//-- render the environment
	GroundEnvironment::getInstance().draw();

	//-- render flora
	{
		PROFILER_AUTO_BLOCK_DEFINE ("terrain draw flora");
		std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), VoidMemberFunction (&ClientRadialFloraManager::draw));
	}

	//-- render the water
	if (m_localWaterManager)
	{
		PROFILER_AUTO_BLOCK_DEFINE ("terrain draw local water");
		std::for_each (m_waterManagerList->begin (), m_waterManagerList->end (), VoidMemberFunction (&WaterManager::draw));
	}

#ifdef _DEBUG
	const Camera *camera = &ShaderPrimitiveSorter::getCurrentCamera();

	//-- render the clear flora entries
	if (ms_renderClearFloraMap)
	{
		const ClearCollidableFloraMap* const clearCollidableFloraMap = getClearCollidableFloraMap ();
		if (clearCollidableFloraMap && !clearCollidableFloraMap->empty ())
		{
			ClearCollidableFloraMap::const_iterator iter = clearCollidableFloraMap->begin ();
			for (; iter != clearCollidableFloraMap->end (); ++iter)
			{
				Vector center (iter->second.position);
				if (getHeight (center, center.y))
				{
					CircleDebugPrimitive* const debugPrimitive = new CircleDebugPrimitive (CircleDebugPrimitive::S_none, Transform::identity, center, iter->second.radius, 20);
					debugPrimitive->setColor (VectorArgb::solidGreen);
					camera->addDebugPrimitive (debugPrimitive);
				}
			}
		}
	}

	//-- render run-time rules
	if (ms_renderRunTimeRules)
	{
		NOT_NULL (m_runTimeRuleList);

		uint i;
		for (i = 0; i < m_runTimeRuleList->size (); ++i)
		{
			const TerrainGenerator::Layer* const layer = (*m_runTimeRuleList) [i];
			const Rectangle2d& layerExtent = layer->getExtent ();

			Vector center (layerExtent.getCenter ().x, 0.f, layerExtent.getCenter ().y);
			if (getHeight (center, center.y))
			{
				CircleDebugPrimitive* const debugPrimitive = new CircleDebugPrimitive (CircleDebugPrimitive::S_none, Transform::identity, center, std::max (layerExtent.y1 - layerExtent.getCenter ().y, layerExtent.x1 - layerExtent.getCenter ().x), 20);
				debugPrimitive->setColor (VectorArgb::solidWhite);
				camera->addDebugPrimitive (debugPrimitive);
			}
		}
	}

	//-- render overlapping run time rules
	if (ms_renderOverlappingRunTimeRules && !m_runTimeRuleList->empty ())
	{
		NOT_NULL (m_runTimeRuleList);

		uint i;
		uint j;
		for (i = 0; i < m_runTimeRuleList->size () - 1; ++i)
			for (j = 1; j < m_runTimeRuleList->size (); ++j)
			{
				const TerrainGenerator::Layer* const layer1 = (*m_runTimeRuleList) [i];
				const TerrainGenerator::Layer* const layer2 = (*m_runTimeRuleList) [j];

				if (layer1->getExtent ().intersects (layer2->getExtent ()))
				{
					{
						const Rectangle2d& e = layer1->getExtent ();

						float height;
						if (getHeight (Vector (e.getCenter ().x, 0.f, e.getCenter ().y), height))
						{
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x0, height, e.y0), Vector (e.x1, height, e.y0), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x1, height, e.y0), Vector (e.x1, height, e.y1), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x1, height, e.y1), Vector (e.x0, height, e.y1), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x0, height, e.y1), Vector (e.x0, height, e.y0), VectorArgb::solidRed));
						}
					}

					{
						const Rectangle2d& e = layer2->getExtent ();

						float height;
						if (getHeight (Vector (e.getCenter ().x, 0.f, e.getCenter ().y), height))
						{
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x0, height, e.y0), Vector (e.x1, height, e.y0), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x1, height, e.y0), Vector (e.x1, height, e.y1), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x1, height, e.y1), Vector (e.x0, height, e.y1), VectorArgb::solidRed));
							camera->addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, Vector (e.x0, height, e.y1), Vector (e.x0, height, e.y0), VectorArgb::solidRed));
						}
					}
				}
			}
	}
#endif // _DEBUG

#ifdef RIBBON_DEBUG_FEELERS
	/*
	Vector lastBottomPoint;
	Vector lastTopPoint;
	for (int ribbonIndex = 0; ribbonIndex < m_debugRibbonAffectorList.size (); ++ribbonIndex)
	{
		const AffectorRibbon* affector = m_debugRibbonAffectorList[ribbonIndex];
		const ArrayList<Vector2d>& pointList = affector->getPointList();

		TerrainObject* terrain = TerrainObject::getInstance();
		for(int pointIndex = 0; pointIndex < pointList.size(); ++pointIndex)
		{
			const Vector2d point = pointList[pointIndex];
			float terrainHeight;
			terrain->getHeight(Vector(point.x,0.0f,point.y),terrainHeight);
			
			Vector bottom =  Vector(point.x,terrainHeight,point.y);
			Vector top = Vector(point.x,terrainHeight + affector->getDepth(),point.y);
			camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,bottom,top,VectorArgb::solidRed));
			if(pointIndex != 0)
			{
				
				camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,bottom,lastBottomPoint,VectorArgb::solidRed));
				camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,top,lastTopPoint,VectorArgb::solidRed));
			}
			lastBottomPoint = bottom;
			lastTopPoint = top;
			
		
		}// for(int pointInd...
	}//for (int ribbonIndex = 0; ri...
*/
	for(int panelVertIndex = 0; panelVertIndex < m_debugRibbonPanelVerts.size (); panelVertIndex += 4)
	{
		/*
				Line3dDebugPrimitive::S_none,
				Line3dDebugPrimitive::S_alpha,
				Line3dDebugPrimitive::S_z,
				Line3dDebugPrimitive::S_litZ,
		*/
		const Vector b0 = m_debugRibbonPanelVerts[panelVertIndex];
		const Vector b1 = m_debugRibbonPanelVerts[panelVertIndex + 1];
		const Vector b2 = m_debugRibbonPanelVerts[panelVertIndex + 2];
		const Vector b3 = m_debugRibbonPanelVerts[panelVertIndex + 3];
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b0,b1,VectorArgb::solidGreen));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b1,b2,VectorArgb::solidGreen));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b2,b3,VectorArgb::solidGreen));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b3,b0,VectorArgb::solidGreen));

		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b0,b2,VectorArgb::solidWhite));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,b1,b3,VectorArgb::solidWhite));
		

		Vector t0 = b0;
		Vector t1 = b1;
		Vector t2 = b2;
		Vector t3 = b3;
		t0.y += 5.0f;
		t1.y += 5.0f;
		t2.y += 5.0f;
		t3.y += 5.0f;
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t0,t1,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t1,t2,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t2,t3,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t3,t0,VectorArgb::solidBlue));

		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t0,b0,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t1,b1,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t2,b2,VectorArgb::solidBlue));
		camera->addDebugPrimitive(new Line3dDebugPrimitive (Line3dDebugPrimitive::S_z, Transform::identity,t3,b3,VectorArgb::solidBlue));

	} //for(int panelVertInde...
#endif // RIBBON_DEBUG_FEELERS
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::postRender () const
{
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::collideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	return m_staticNonCollidableFloraManager && m_staticNonCollidableFloraManager->collide (start_w, end_w, result);
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::approximateCollideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	return m_staticNonCollidableFloraManager && m_staticNonCollidableFloraManager->approximateCollide (start_w, end_w, result);
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::findStaticNonCollidableFlora (float positionX, float positionZ, ClientProceduralTerrainAppearance::StaticFloraData& data, bool& floraAllowed) const
{
	const ClientChunk* chunk = safe_cast<const ClientChunk*> (ProceduralTerrainAppearance::findFirstRenderableChunk2D(positionX, positionZ));

	return chunk && chunk->findStaticNonCollidableFlora (positionX, positionZ, data, floraAllowed);
}  //lint !e1763  // function marked as const modifies class

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::findDynamicNearFlora (float positionX, float positionZ, ClientProceduralTerrainAppearance::DynamicFloraData& data, bool& floraAllowed) const
{
	const ClientChunk* chunk = safe_cast<const ClientChunk*> (ProceduralTerrainAppearance::findFirstRenderableChunk2D(positionX, positionZ));

	return chunk && chunk->findDynamicNearFlora (positionX, positionZ, data, floraAllowed);
}  //lint !e1763  // function marked as const modifies class

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::findDynamicFarFlora (float positionX, float positionZ, ClientProceduralTerrainAppearance::DynamicFloraData& data, bool& floraAllowed) const
{
	const ClientChunk* chunk = safe_cast<const ClientChunk*> (ProceduralTerrainAppearance::findFirstRenderableChunk2D(positionX, positionZ));

	return chunk && chunk->findDynamicFarFlora (positionX, positionZ, data, floraAllowed);
}  //lint !e1763  // function marked as const modifies class

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::findEnvironment (const Vector& position, ClientProceduralTerrainAppearance::EnvironmentData& data) const
{
	const ClientChunk* chunk = safe_cast<const ClientChunk*> (ProceduralTerrainAppearance::findFirstRenderableChunk (position));

	return chunk && chunk->findEnvironment (position, data);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::buildLocalWaterTable (const TerrainGenerator::Layer* layer)
{
	if (!m_waterManagerList)
		return;

	if (!m_localWaterManager)
		return;

	//-- handle placed local water table
	{
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		{
			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_polygon)
			{
				const BoundaryPolygon* boundary = safe_cast<const BoundaryPolygon*> (layer->getBoundary (i));
				NOT_NULL (boundary);

				if (boundary->isLocalWaterTable ())
				{
					NOT_NULL (m_localWaterManager);

					std::vector<Vector2d> vertices;
					vertices.reserve (static_cast<uint> (boundary->getPointList ().getNumberOfElements ()));

					int j;
					for (j = 0; j < boundary->getPointList ().getNumberOfElements (); ++j)
						vertices.push_back (boundary->getPointList () [j]);

					m_localWaterManager->addWater (boundary->getName (), FileName (FileName::P_shader, boundary->getLocalWaterTableShaderTemplateName ()), boundary->getLocalWaterTableShaderSize (), vertices, boundary->getLocalWaterTableHeight (), Vector2d (0.f, 0.f), 0.f);
				}
			}

			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_rectangle)
			{
				const BoundaryRectangle* boundary = safe_cast<const BoundaryRectangle*> (layer->getBoundary (i));
				NOT_NULL (boundary);

				if (boundary->isLocalWaterTable ())
				{
					Rectangle2d const & rectangle2d = boundary->getRectangle ();

					const char *boundaryName = boundary->getName();

					if (boundary->isLocalGlobalWaterTable ())
					{
						NOT_NULL (m_waterManagerList);
						m_waterManagerList->push_back (new ClientGlobalWaterManager2 (*this, proceduralTerrainAppearanceTemplate->getMapWidthInMeters (), FileName (FileName::P_shader, boundary->getLocalWaterTableShaderTemplateName ()), boundary->getLocalWaterTableShaderSize (), boundary->getLocalWaterTableHeight (), &boundary->getRectangle ()));
					}
					else
					{
						NOT_NULL (m_localWaterManager);

						std::vector<Vector2d> vertices;
						vertices.reserve (4);
						vertices.push_back (Vector2d (rectangle2d.x0, rectangle2d.y0));
						vertices.push_back (Vector2d (rectangle2d.x1, rectangle2d.y0));
						vertices.push_back (Vector2d (rectangle2d.x1, rectangle2d.y1));
						vertices.push_back (Vector2d (rectangle2d.x0, rectangle2d.y1));

						m_localWaterManager->addWater (boundaryName, FileName (FileName::P_shader, boundary->getLocalWaterTableShaderTemplateName ()), boundary->getLocalWaterTableShaderSize (), vertices, boundary->getLocalWaterTableHeight (), Vector2d (0.f, 0.f), 0.f);
					}
				}
			}
		}
	}

	//-- handle river which will create local water tables
	{
		std::vector<Vector> vertices;
		std::vector<Vector> texCoords; // used for ribbons

		int i;
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->isActive () && layer->getAffector (i)->getType () == TGAT_river)
			{
				const AffectorRiver* affector = safe_cast<const AffectorRiver*> (layer->getAffector (i));
				NOT_NULL (affector);

				if (affector->getHasLocalWaterTable ())
				{
					ArrayList<AffectorRiver::WaterTable> riverWaterTableList;
					affector->createWaterTables (riverWaterTableList);

					if (riverWaterTableList.getNumberOfElements () != 0)
					{
						int j;
						for (j = 0; j < riverWaterTableList.getNumberOfElements (); ++j)
						{
							vertices.clear ();

							int k;
							for (k = 0; k < 4; ++k)
								vertices.push_back (riverWaterTableList [j].points [k]);

							if (affector->getLocalWaterTableShaderTemplateName ())
							{
								Vector2d direction (riverWaterTableList [j].direction);
								IGNORE_RETURN (direction.normalize ());
								direction.y = -direction.y;

								NOT_NULL (m_localWaterManager);
								m_localWaterManager->addWater (affector->getName (), FileName (FileName::P_shader, affector->getLocalWaterTableShaderTemplateName ()), affector->getLocalWaterTableShaderSize (), vertices, direction, affector->getVelocity ());
							}
						}
					}
				}
			}

			// Start Ribbon
			else if (layer->getAffector (i)->isActive () && layer->getAffector (i)->getType () == TGAT_ribbon)
			{
				const AffectorRibbon* affector = safe_cast<const AffectorRibbon*> (layer->getAffector (i));
				NOT_NULL (affector);

				ArrayList<AffectorRibbon::Quad> ribbonQuadList;
				affector->createQuadList (ribbonQuadList);

				Vector2d direction(0,1);
 
				if (ribbonQuadList.getNumberOfElements() != 0)
				{
					direction.x = ribbonQuadList[0].points[3].x - ribbonQuadList[0].points[0].x;
					direction.y = ribbonQuadList[0].points[3].z - ribbonQuadList[0].points[0].z;

					vertices.clear ();
					texCoords.clear();

					vertices.push_back(ribbonQuadList[0].points[0]);
					vertices.push_back(ribbonQuadList[0].points[1]);

					Vector temp(0,0,0), temp2(0,0,0), tc(0,0,0);
					temp = ribbonQuadList[0].points[1] - ribbonQuadList[0].points[0];
					float dx = temp.magnitude() * 0.5f;

					tc.x = dx;
					texCoords.push_back(tc);

					tc.x = -dx;
					texCoords.push_back(tc);

					int j;
					for (j = 0; j < ribbonQuadList.getNumberOfElements (); ++j)
					{
						vertices.push_back(ribbonQuadList[j].points[3]);
						vertices.push_back(ribbonQuadList[j].points[2]);

						temp = ribbonQuadList[j].points[3] - ribbonQuadList[j].points[0];
						temp2 = ribbonQuadList[j].points[2] - ribbonQuadList[j].points[1];

						float dz = (temp.magnitude() + temp2.magnitude()) * 0.5f;

						tc.z += dz;
						tc.y += .07f * dz;

						temp = ribbonQuadList[j].points[3] - ribbonQuadList[j].points[2];
						dx = temp.magnitude() * 0.5f;

						tc.x = dx;
						texCoords.push_back(tc);

						tc.x = -dx;
						texCoords.push_back(tc);
					}

					m_localWaterManager->addRibbonStrip (
						affector->getName (), 
						FileName (FileName::P_shader, affector->getRibbonWaterShaderTemplateName ()), 
						affector->getWaterShaderSize(), 
						vertices, 
						texCoords,
						affector->getVelocity()
					);
				}

				
				// endcap
				if(affector->getCapWidth() > 0)
				{
					std::vector<Vector2d> vertices;
					vertices.reserve (static_cast<uint> (affector->getEndCapPointList ().getNumberOfElements ()));
					int j;
					for(j = 0; j < affector->getEndCapPointList().getNumberOfElements (); ++j)
					{
						vertices.push_back(affector->getEndCapPointList()[j]);
					}



					m_localWaterManager->addRibbonEndCap(
						affector->getName(),
						FileName (FileName::P_shader, affector->getRibbonWaterShaderTemplateName ()),
						affector->getWaterShaderSize(),
						vertices,
						affector->getHeightList ()[0], 
						direction, 
						0.f,
						(ribbonQuadList[0].points[0] + ribbonQuadList[0].points[1])/2.0f

					);
				}
			}
			// End Ribbon
		}
	}

	{
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
			if (layer->getLayer (i)->isActive ())
				buildLocalWaterTable (layer->getLayer (i));
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::buildLocalWaterTables ()
{
	const TerrainGenerator* generator = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();

	int i;
	for (i = 0; i < generator->getNumberOfLayers (); i++)
		if (generator->getLayer (i)->isActive ())
			buildLocalWaterTable (generator->getLayer (i));
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::createRadarShader (const Vector& center, const float range, const int maxSize, RadarShaderInfo& radarShaderInfo, const bool clip, const VectorArgb& clearColor, const bool drawTerrain) const
{
	NOT_NULL (m_radar);
	m_radar->createShader (center, range, maxSize, radarShaderInfo, clip, clearColor, drawTerrain);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::purgeChunks ()
{
	m_requestCriticalSection.enter();

	REPORT_LOG_PRINT (m_totalNumberOfChunksCreated, ("average chunk create: %1.3f generate, %1.3f client\n", m_totalChunkGenerationTime / m_totalNumberOfChunksCreated, m_totalChunkCreationTime / m_totalNumberOfChunksCreated));

	IGNORE_RETURN (getChunkTree ()->getTopNode ()->removeSubNodes (true));
	if (getChunkTree ()->getTopNode ()->getChunk ())
		getChunkTree ()->getTopNode ()->removeChunk (getChunkTree ()->getTopNode ()->getChunk (), true);

	m_totalChunkCreationTime     = 0;
	m_totalChunkGenerationTime   = 0;
	m_totalNumberOfChunksCreated = 0;

	NOT_NULL (m_levelOfDetail);

	m_levelOfDetail->setDirty (true);

	m_requestCriticalSection.leave();
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getPauseEnvironment () const
{
	return GroundEnvironment::getInstance().getPaused();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setPauseEnvironment (bool const paused)
{
	GroundEnvironment::getInstance().setPaused(paused);
}

//-------------------------------------------------------------------

const PackedRgb ClientProceduralTerrainAppearance::getClearColor () const
{
	return GroundEnvironment::getInstance().getClearColor();
}

//-------------------------------------------------------------------

const PackedRgb ClientProceduralTerrainAppearance::getFogColor () const
{
	return GroundEnvironment::getInstance().getFogColor();
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getFogDensity () const
{
	return GroundEnvironment::getInstance().getFogDensity();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::getTime (int& hour, int& minute) const
{
	hour = GroundEnvironment::getInstance().getHour();
	minute = GroundEnvironment::getInstance().getMinute();
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getTime () const
{
	return GroundEnvironment::getInstance().getTime();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setTime (float time, bool force)
{
	GroundEnvironment::getInstance().setTime(time, force);
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::isDay () const
{
	return GroundEnvironment::getInstance().isDay();
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::isTimeLocked() const
{
	return GroundEnvironment::getInstance().isTimeLocked();
}

//-------------------------------------------------------------------

const EnvironmentBlock* ClientProceduralTerrainAppearance::getCurrentEnvironmentBlock () const
{
	return GroundEnvironment::getInstance().getCurrentEnvironmentBlock();
}

//-------------------------------------------------------------------

const InteriorEnvironmentBlock* ClientProceduralTerrainAppearance::getCurrentInteriorEnvironmentBlock () const
{
	return GroundEnvironment::getInstance().getCurrentInteriorEnvironmentBlock();
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getHighLevelOfDetailThreshold () const
{
	return LevelOfDetail::getForceHighThreshold ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setHighLevelOfDetailThreshold (const float highLevelOfDetailThreshold)
{
	//-- tell the flora system the chunk has changed
	if (m_dynamicNearFloraManager)
	{
		const float mapWidthInMeters = proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();
		const Rectangle2d region (-mapWidthInMeters, -mapWidthInMeters, mapWidthInMeters, mapWidthInMeters);

		m_dynamicNearFloraManager->markRegionDirty (region);
	}

	NOT_NULL (m_levelOfDetail);
	if (m_levelOfDetail)
		m_levelOfDetail->setForceHighThreshold (highLevelOfDetailThreshold);
}

//-------------------------------------------------------------------

float ClientProceduralTerrainAppearance::getLevelOfDetailThreshold () const
{
	return LevelOfDetail::getThreshold();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setLevelOfDetailThreshold (const float levelOfDetailThreshold)
{
	//-- tell the flora system the chunk has changed
	if (m_dynamicNearFloraManager)
	{
		const float mapWidthInMeters = proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();
		const Rectangle2d region (-mapWidthInMeters, -mapWidthInMeters, mapWidthInMeters, mapWidthInMeters);

		m_dynamicNearFloraManager->markRegionDirty (region);
	}

	NOT_NULL (m_levelOfDetail);
	if (m_levelOfDetail)
		m_levelOfDetail->setThreshold (levelOfDetailThreshold);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::addClearNonCollidableFloraObject (const Object* const object, const ClearFloraEntryList& clearFloraEntryList)
{
	ClientRadialFloraManager::addClearFloraObject (object, clearFloraEntryList);

	Vector const & position_w = object->getPosition_w ();
	Rectangle2d rectangle;
	rectangle.x0 = position_w.x - 128.f;
	rectangle.y0 = position_w.z - 128.f;
	rectangle.x1 = position_w.x + 128.f;
	rectangle.y1 = position_w.z + 128.f;

	if (m_dynamicFarFloraManager)
		m_dynamicFarFloraManager->markRegionDirty (rectangle);

	if (m_staticNonCollidableFloraManager)
		m_staticNonCollidableFloraManager->markRegionDirty (rectangle);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::removeClearNonCollidableFloraObject (const Object* const object)
{
	ClientRadialFloraManager::removeClearFloraObject (object);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::setChunkLotShader (const int chunkX, const int chunkZ, const Shader* lotShader) const
{
	ClientChunk* chunk = const_cast<ClientChunk*> (safe_cast<const ClientChunk*> (findChunk (chunkX, chunkZ, 1)));

	if (chunk)
		chunk->setLotShader (lotShader);
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::clearLotShaders ()
{
	TerrainQuadTree::Iterator iter = getChunkTree ()->getIterator ();
	TerrainQuadTree::Node* node = 0;

	while ((node = iter.getCurNode ()) != 0)
	{
		ClientChunk* const chunk = safe_cast<ClientChunk*> (node->getChunk ());

		if (chunk)
			chunk->setLotShader (0);

		IGNORE_RETURN (iter.descend ());
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::removeUnnecessaryChunk ()
{
	const Vector& position = getReferenceObject (0)->getPosition_w ();

	float maxDistanceSquared = -1.0f;

	TerrainQuadTree::Node * chunkNode = 0;

	TerrainQuadTree::Iterator iter = getChunkTree ()->getIterator ();

	TerrainQuadTree::Node * node = 0;

	while ((node = iter.getCurNode ()) != 0)
	{
		if (node->getSize () > 1 )
		{
			IGNORE_RETURN (iter.descend ());
			continue;
		}

		float distanceSquared = position.magnitudeBetweenSquared (node->getChunk()->getSphere().getCenter ());

		if (distanceSquared > maxDistanceSquared)
		{
			maxDistanceSquared	= distanceSquared;
			chunkNode			= node;
		}

		IGNORE_RETURN (iter.advance ());
	}

	if (chunkNode)
	{
		chunkNode->getParent()->destroyNode (chunkNode);
		chunkNode = 0;
	}

	getChunkTree ()->pruneTree ();
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::getPolygonSoup (const Rectangle2d& extent2d, IndexedTriangleList& indexedTriangleList) const
{
	TerrainQuadTree::Iterator iter = getChunkTree ()->getIterator ();
	TerrainQuadTree::Node* node = 0;

	while ((node = iter.getCurNode ()) != 0)
	{
		if (node->getSize () > 1 )
		{
			IGNORE_RETURN (iter.descend ());
			continue;
		}

		const ClientChunk* const clientChunk = safe_cast<const ClientChunk*> (node->getChunk ());
		if (clientChunk)
		{
			const BoxExtent& boxExtent = clientChunk->getBoxExtent ();
			const Rectangle2d chunkExtent2d (boxExtent.getMin ().x, boxExtent.getMin ().z,  boxExtent.getMax ().x, boxExtent.getMax ().z);
			if (extent2d.intersects (chunkExtent2d))
				clientChunk->getPolygonSoup (extent2d, indexedTriangleList);
		}

		IGNORE_RETURN (iter.advance ());
	}
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::debugDump () const
{
#ifdef _DEBUG
	ProceduralTerrainAppearance::debugDump ();

	DEBUG_REPORT_PRINT (true, ("-- ClientProceduralTerrainAppearance\n"));
	DEBUG_REPORT_PRINT (true, ("             lightScaling = %s\n", ConfigClientTerrain::getEnableLightScaling () ? "on" : "off"));
	DEBUG_REPORT_PRINT (true, ("   numberOfChunksRendered = %i\n", getNumberOfChunksRendered ()));
	DEBUG_REPORT_PRINT (true, ("                threshold = %1.2f\n", m_levelOfDetail->getThreshold ()));
	DEBUG_REPORT_PRINT (true, ("            highThreshold = %1.2f\n", getHighLevelOfDetailThreshold ()));
	DEBUG_REPORT_PRINT (true, ("              dot3Terrain = %s\n", getDot3Terrain() ? "yes" : "no"));
	DEBUG_REPORT_PRINT (true, ("         directionToLight = <%1.2f, %1.2f, %1.2f>\n", ms_directionToLight.x, ms_directionToLight.y, ms_directionToLight.z));
	DEBUG_REPORT_PRINT (true, (" numberOfInvalidatedNodes = %i\n", ms_maximumNumberOfInvalidatedNodes));
	DEBUG_REPORT_PRINT (true, ("            multiThreaded = %s\n", ms_multiThreadedTerrainGeneration ? "yes" : "no"));
	DEBUG_REPORT_PRINT (true, ("        requestThreadMode = %i\n", static_cast<int> (m_requestThreadMode)));
	DEBUG_REPORT_PRINT (true, ("  numberOfPendingRequests = %i\n", static_cast<int> (m_pendingChunkRequestInfoMap->size ())));
	DEBUG_REPORT_PRINT (true, ("numberOfInvalidateRegions = %i\n", static_cast<int> (m_invalidateRegionList->size ())));
#endif
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ClientProceduralTerrainAppearance::findChunk (const int x, const int z, const int chunkSize) const
{
	return getChunkTree ()->findChunk (x, z, chunkSize);
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ClientProceduralTerrainAppearance::findFirstRenderableChunk (const int x, const int z) const
{
	const TerrainQuadTree::Node* const node = getChunkTree ()->getTopNode ()->findFirstRenderableNode (x, z);

	return node ? node->getChunk () : 0;
}

//-------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::collide (const Vector& start_o, const Vector& end_o, CollideParameters const & /*collideParameters*/, CollisionInfo& result) const
{
	result.setPoint (end_o);

	bool collided = false;

	//-- fire ray through chunks
	TerrainQuadTree::Iterator iter = getChunkTree ()->getIterator ();

	TerrainQuadTree::Node * node = 0;

	while ((node = iter.getCurNode ()) != 0 )
	{
		// skip nodes without chunks, or chunks that are not meant to be rendered
		// non-rendered chunks don't floatly 'exist' in the world -- they are simply being cached
		if (node->getChunk () == 0 || !node->isSelectedForRender ())
		{
			if (node->getBoxExtent ().testSphereOnly (start_o, end_o))
				IGNORE_RETURN (iter.descend ());
			else
				IGNORE_RETURN (iter.advance ());

			continue;
		}

		NOT_NULL (node->getChunk ());

		if (node->getChunk ()->collide (start_o, end_o, CollideParameters::cms_default, result))
			collided = true;

		IGNORE_RETURN (iter.advance ());
	}

	return collided;
}

//-------------------------------------------------------------------

void ClientProceduralTerrainAppearance::addChunk (Chunk* const chunk, int /*chunkSize*/)
{
	verifyChunk (chunk);

	const float mapWidthInMeters = proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();

	IGNORE_RETURN (getChunkTree ()->addChunk (chunk, static_cast<int> (mapWidthInMeters / chunk->getChunkWidthInMeters ())));
}

//-----------------------------------------------------------------

int ClientProceduralTerrainAppearance::getNumberOfChunks () const
{
	return getChunkTree ()->getNumberOfChunks ();
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::hasHighLevelOfDetailTerrain (const Vector&  position_o ) const
{
	const ClientChunk* const chunk = safe_cast<const ClientChunk*> (ProceduralTerrainAppearance::findFirstRenderableChunk (position_o));
	return chunk && chunk->getChunkWidthInMeters () == getChunkWidthInMeters ();
}

bool ClientProceduralTerrainAppearance::terrainGenerationStabilized () const
{
	const_cast<ClientProceduralTerrainAppearance*>(this)->m_requestCriticalSection.enter ();
	bool const done = m_pendingChunkRequestInfoMap->empty ();
	const_cast<ClientProceduralTerrainAppearance*>(this)->m_requestCriticalSection.leave ();

	return done;
}

//-----------------------------------------------------------------

const TerrainQuadTree* ClientProceduralTerrainAppearance::getChunkTree () const
{
	return m_chunkTree;
}

//-----------------------------------------------------------------

TerrainQuadTree* ClientProceduralTerrainAppearance::getChunkTree ()
{
	return m_chunkTree;
}

//----------------------------------------------------------------------

/**
* this function gets called from places like CollisionWorld when objects move.
* on the client, the terrain generation is handled in a seperate thread.
* it is invalid for force chunk creation from the main thread on the client.
*/

bool ClientProceduralTerrainAppearance::isPassableForceChunkCreation(const Vector& position) const
{
	if (!hasPassableAffectors())
		return true;

	Chunk const * const chunk = ProceduralTerrainAppearance::findFirstRenderableChunk(position);
	return !chunk || chunk->isPassable(position);
}

//----------------------------------------------------------------------

const BoxExtent* ClientProceduralTerrainAppearance::getChunkExtentForceChunkCreation (const Vector& position_o) const
{
	Chunk const * const chunk = ProceduralTerrainAppearance::findFirstRenderableChunk(position_o);
	return chunk ? &chunk->getBoxExtent() : NULL;
}

//----------------------------------------------------------------------

bool ClientProceduralTerrainAppearance::getHeightForceChunkCreation (const Vector& position_o, float& height) const
{
	Chunk const * const chunk = ProceduralTerrainAppearance::findFirstRenderableChunk(position_o);
	return chunk && chunk->getHeightAt(position_o, &height);
}

//===================================================================

