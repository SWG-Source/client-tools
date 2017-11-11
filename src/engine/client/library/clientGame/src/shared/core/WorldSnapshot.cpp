//===================================================================
//
// WorldSnapshot.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/WorldSnapShot.h"

#include "clientGame/CellObject.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/Game.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/VTune.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedCellObjectTemplate.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/SphereTree.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "sharedUtility/DataTable.h"
#include <algorithm>
#include <limits>

//===================================================================

namespace WorldSnapshotNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum CreateErrorCode
	{
		CEC_objectAlreadyExists,
		CEC_orphanedAtOrigin,
		CEC_mismatchedPobCrc,
		CEC_tooCloseToOrigin
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	WorldSnapshotReaderWriter ms_reader;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NodeSphereExtentAccessor: public BaseSphereTreeAccessor<WorldSnapshotReaderWriter::Node const *, NodeSphereExtentAccessor>
	{
	public:

		static Sphere const getExtent (WorldSnapshotReaderWriter::Node const * const node)
		{
			return node ? node->getSphere () : Sphere::zero;
		}

		static char const *getDebugName (WorldSnapshotReaderWriter::Node const * const node)
		{
			UNREF(node);

			return 0;
		}

	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string ms_sceneName;
	std::string ms_excludeArea;

	SphereTree<const WorldSnapshotReaderWriter::Node*, NodeSphereExtentAccessor> ms_sphereTree;

	typedef std::vector<const WorldSnapshotReaderWriter::Node*> NodeList;
	typedef std::map< std::string, std::list<const WorldSnapshotReaderWriter::Node*> > BuildoutEventMap;

	NodeList ms_loadedList;
	NodeList ms_queryList;
	NodeList ms_pendingCreateList;
	NodeList ms_pendingDeleteList;

	BuildoutEventMap ms_eventObjectMap;

	bool ms_logWorldSnapshotCreates;
	bool ms_reportWorldSnapshotCreates;
	bool ms_vtuneWorldSnapshotCreates;
	bool ms_debugReport;
	bool ms_logUpdate;
	bool ms_preloadSomeAssetsLogs;
	float ms_lastUpdateTime = 0.f;
	bool ms_reportBuildoutVersions = false;

	typedef std::vector<const ObjectTemplate*> PreloadObjectTemplateList;
	PreloadObjectTemplateList ms_preloadObjectTemplateList;
	int  ms_preloadObjectTemplate;
	int  ms_numberOfObjectTemplates;

	const float ms_closeToOriginDistance = 10.f;
	float const cms_callbackTime = 1.f;

	CellProperty const * ms_lastCellProperty;
	Vector ms_lastPosition_w (0.f, -9999.f, 0.f);
	float ms_updateDistanceSquared;
	int ms_maximumNumberOfCreatesPerFrame = 1000;
	int ms_maximumNumberOfDeletesPerFrame = 1000;

	//------------------------------------------------------------------------------------------------------------------

	const SharedObjectTemplate *fetchObjectTemplate(const WorldSnapshotReaderWriter& reader, const WorldSnapshotReaderWriter::Node* const node)
	{
		const char* const objectTemplateName = reader.getObjectTemplateName (node->getObjectTemplateNameIndex ());
		ObjectTemplate const * const ot = ObjectTemplateList::fetch (objectTemplateName);
		if (!ot)
		{
			WARNING(true, ("WorldSnapshot unable to load template [%s]", objectTemplateName));
			return 0;
		}
		return safe_cast<const SharedObjectTemplate*> (ot);	
	}

	//------------------------------------------------------------------------------------------------------------------

	ClientObject *instantiateObject(const WorldSnapshotReaderWriter& reader, const WorldSnapshotReaderWriter::Node* const node, CreateErrorCode& result)
	{
		//-- fetch the object template
		const SharedObjectTemplate * objectTemplate = fetchObjectTemplate(reader, node);
		if (!objectTemplate)
		{
			return 0;
		}

		//-- verify that the portal layout crcs match
		{
			uint32 portalLayoutCrc = 0;
			if (!Game::getSinglePlayer() || !ConfigClientGame::getWorldSnapshotIgnorePobChanges())
			{
				if (PortalPropertyTemplate::extractPortalLayoutCrc (objectTemplate->getPortalLayoutFilename ().c_str (), portalLayoutCrc))
				{
					if (portalLayoutCrc != static_cast<uint32> (node->getPortalLayoutCrc ()))
					{
						WARNING(true, ("WorldSnapshot createObject [%s] pob crc changed from [%d] to [%d]", 
							objectTemplate->getName(), static_cast<int>(node->getPortalLayoutCrc()), static_cast<int>(portalLayoutCrc)));

						objectTemplate->releaseReference ();
						objectTemplate=0;

						result = CEC_mismatchedPobCrc;
						return 0;
					}
				}
			}
		}

		//-- instantiate the object
		ClientObject *const object = safe_cast<ClientObject*> (objectTemplate->createObject());
		objectTemplate->releaseReference();
		objectTemplate=0;

		return object;
	}

	//------------------------------------------------------------------------------------------------------------------

	Object* createObject (const WorldSnapshotReaderWriter& reader, const WorldSnapshotReaderWriter::Node* const node, CreateErrorCode& result)
	{
		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		if (NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (node->getNetworkIdInt ()))) != 0)
		{
			result = CEC_objectAlreadyExists;
			return 0;
		}

		if (node->getContainedByNetworkIdInt () == 0 && node->getTransform_p ().getPosition_p () == Vector::zero)
		{
			result = CEC_orphanedAtOrigin;
			return 0;
		}

		if (node->getContainedByNetworkIdInt () == 0 && node->getTransform_p ().getPosition_p ().magnitudeSquared () < sqr (ms_closeToOriginDistance))
		{
			result = CEC_tooCloseToOrigin;
			return 0;
		}

		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		ClientObject *const object = instantiateObject(reader, node, result);
		if (!object)
		{
			return 0;
		}

		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		object->setClientCached();
		object->setTransform_o2p (node->getTransform_p ());
		object->setNetworkId (NetworkId (static_cast<NetworkId::NetworkIdType> (node->getNetworkIdInt ())));
		object->createDefaultController();

		DEBUG_REPORT_LOG(ms_reportWorldSnapshotCreates, ("start created %i [%s]\n", node->getNetworkIdInt (), reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));

		RenderWorld::addObjectNotifications(*object);
	
		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		//-- begin the baselines
		object->beginBaselines ();

		ContainedByProperty* const containedByProperty = object->getContainedByProperty ();
		NOT_NULL (containedByProperty);

		const NetworkId::NetworkIdType containerId = node->getContainedByNetworkIdInt();
		containedByProperty->setContainedBy (NetworkId (containerId ) );

		bool isCellObject = false;

		PortalProperty* const portalProperty = object->getPortalProperty ();
		if (!portalProperty)
		{
			CellObject* const cellObject = dynamic_cast<CellObject*> (object);
			if (cellObject)
			{
				//-- handle cell objects (set cell index)
				cellObject->setCell (node->getCellIndex ());

				isCellObject = true;
			}
			else
			{
				//-- handle all other objects (just add the notification)
				CellProperty::addPortalCrossingNotification (*object);
			}
		}

		//-- does this object have any child nodes? don't create nodes for cell objects
		if (!isCellObject)
		{
			int i;
			for (i = 0; i < node->getNumberOfNodes (); ++i)
			{
				IGNORE_RETURN (createObject (reader, node->getNode (i), result));

				//-- no need to check result because these objects are inside cells
			}
		}

		//-- end the baselines
		object->endBaselines ();
		//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		DEBUG_REPORT_LOG(ms_reportWorldSnapshotCreates, ("  end created %i [%s]\n", node->getNetworkIdInt (), reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));

		return object;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void debugReport ()
	{
		DEBUG_REPORT_PRINT (true, ("-- WorldSnapshot\n"));
		DEBUG_REPORT_PRINT (true, ("  root objects = %i\n", ms_reader.getNumberOfNodes ()));
		DEBUG_REPORT_PRINT (true, (" total objects = %i\n", ms_reader.getTotalNumberOfNodes ()));
		DEBUG_REPORT_PRINT (true, ("     preloaded = %i\n", ms_preloadObjectTemplateList.size ()));
		DEBUG_REPORT_PRINT (true, ("        loaded = %i\n", ms_loadedList.size ()));
		DEBUG_REPORT_PRINT (true, ("         query = %i\n", ms_queryList.size ()));
		DEBUG_REPORT_PRINT (true, ("  sphere nodes = %i\n", ms_sphereTree.getNodeCount()));
		DEBUG_REPORT_PRINT (true, ("sphere objects = %i\n", ms_sphereTree.getObjectCount()));
		DEBUG_REPORT_PRINT (true, ("pending create = %i\n", ms_pendingCreateList.size ()));
		
		if (ms_pendingCreateList.size () > 1)
			DEBUG_REPORT_PRINT (true, ("%1.2f < %1.2f\n", ms_pendingCreateList [0]->getDistanceSquaredTo (), ms_pendingCreateList [1]->getDistanceSquaredTo ()));

		DEBUG_REPORT_PRINT (true, ("pending delete = %i\n", ms_pendingDeleteList.size ()));
		if (ms_pendingDeleteList.size () > 1)
			DEBUG_REPORT_PRINT (true, ("%1.2f > %1.2f\n", ms_pendingDeleteList [0]->getDistanceSquaredTo (), ms_pendingDeleteList [1]->getDistanceSquaredTo ()));

		DEBUG_REPORT_PRINT (true, ("   update time = %1.2f\n", ms_lastUpdateTime));
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void addObjectToWorld(Object * const object, WorldSnapshotReaderWriter::Node const * const node)
	{
		object->addToWorld ();

		//-- the object is now loaded
		node->addToWorld ();
		ms_loadedList.push_back (node);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// little helper function here
	template< class T, typename U >
	bool isInSet( const T &t, const U &u )
	{
		return t.find( u ) != t.end();
	}

	const int cs_buildingObjIdOffset = 2000;
}

using namespace WorldSnapshotNamespace;

//===================================================================
// STATIC PUBLIC WorldSnapshotReaderWriter
//===================================================================

void WorldSnapshot::install ()
{
	InstallTimer const installTimer("WorldSnapshot::install");

	DEBUG_FATAL (ms_reader.getNumberOfNodes (), ("WorldSnapshot::install - already installed"));
	DEBUG_FATAL (!ms_loadedList.empty (), ("WorldSnapshot::install - already installed"));

	DebugFlags::registerFlag (ms_logWorldSnapshotCreates, "ClientGame/WorldSnapshot", "logWorldSnapshotCreates");
	DebugFlags::registerFlag (ms_reportWorldSnapshotCreates, "ClientGame/WorldSnapshot", "reportWorldSnapshotCreates");
	DebugFlags::registerFlag (ms_vtuneWorldSnapshotCreates, "ClientGame/WorldSnapshot", "vtuneWorldSnapshotCreates");
	DebugFlags::registerFlag (ms_debugReport, "ClientGame/WorldSnapshot", "debugReport", WorldSnapshotNamespace::debugReport);
	DebugFlags::registerFlag (ms_logUpdate, "ClientGame/WorldSnapshot", "logUpdate");
	DebugFlags::registerFlag (ms_preloadSomeAssetsLogs, "ClientGame/WorldSnapshot", "preloadSomeAssetsLogs");

	WorldSnapshotReaderWriter::Node::setDetailLevelChangedFunction (detailLevelChanged);

	ms_updateDistanceSquared = sqr (ConfigFile::getKeyFloat ("ClientGame/WorldSnapshot", "updateDistance", 4.f));
	ms_maximumNumberOfCreatesPerFrame = ConfigFile::getKeyInt("ClientGame/WorldSnapshot", "maximumNumberOfCreatesPerFrame", ms_maximumNumberOfCreatesPerFrame);
	ms_maximumNumberOfDeletesPerFrame = ConfigFile::getKeyInt("ClientGame/WorldSnapshot", "maximumNumberOfDeletesPerFrame", ms_maximumNumberOfDeletesPerFrame);

	ExitChain::add (remove, "WorldSnapshot::remove");
}

//-------------------------------------------------------------------

void WorldSnapshot::remove ()
{
	DebugFlags::unregisterFlag (ms_logWorldSnapshotCreates);
	DebugFlags::unregisterFlag (ms_reportWorldSnapshotCreates);
	DebugFlags::unregisterFlag (ms_vtuneWorldSnapshotCreates);
	DebugFlags::unregisterFlag (ms_debugReport);
	DebugFlags::unregisterFlag (ms_logUpdate);
	DebugFlags::unregisterFlag (ms_preloadSomeAssetsLogs);

	unload ();
}

//-------------------------------------------------------------------

void WorldSnapshot::unload ()
{
	//-- clear out the preloaded object templates
	{
		uint i;
		for (i = 0; i < ms_preloadObjectTemplateList.size (); ++i)
			ms_preloadObjectTemplateList [i]->releaseReference ();

		ms_preloadObjectTemplateList.clear ();
	}

	//-- clear out the sphere tree
	{
		int i;
		for (i = 0; i < ms_reader.getNumberOfNodes (); ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
			if (node->getSpatialSubdivisionHandle ())
			{
				ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
				node->setSpatialSubdivisionHandle (0);
			}

			node->removeFromWorld();
			Object * const object = NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(node->getNetworkIdInt())));
			if (object)
				delete object;
		}
	}

	//-- verify the sphere trees are empty
	if (ms_sphereTree.getObjectCount () > 0)
	{
		DEBUG_FATAL (true, ("WorldSnapshot::unload: sphere tree not empty"));
//		ms_sphereTree.clear ();
	}

	//-- clear out what is loaded
	ms_loadedList.clear ();
	ms_pendingCreateList.clear ();
	ms_pendingDeleteList.clear ();

	// Event object map clean up.
	ms_eventObjectMap.clear();

	//-- clear out the snapshot
	ms_reader.clear ();
}

//-------------------------------------------------------------------

void WorldSnapshot::load (char const *sceneName)
{
	const int cs_sharedCellObjectTemplate_tag = 0x0c5401ee;

	NOT_NULL(sceneName);

	//-- clear the current snapshot
	ms_loadedList.clear ();
	ms_reader.removeFromWorld ();
	ms_lastCellProperty = 0;
	ms_lastPosition_w.set (0.f, -9999.f, 0.f);

	//-- see if we're already loaded
	if (_stricmp (sceneName, ms_sceneName.c_str ()) == 0)
	{
		DEBUG_REPORT_LOG (true, ("WorldSnapshot::load - %s is currently loaded\n", sceneName));
		return;
	}

	ms_sceneName = sceneName;
	unload ();


#if PRODUCTION
	//always do block below [if ( true )]
#else
	if ( ConfigClientGame::getLoadBuildoutOnly() == false )
#endif
	{
		if ( !ms_reader.load (sceneName))
		{
			//-- only warn if we don't have a buildout and are not in a space scene
			if (strncmp(sceneName, "space_", 6))
				DEBUG_WARNING (!SharedBuildoutAreaManager::isBuildoutScene(sceneName), ("WorldSnapshot::load - could not load %s", sceneName));
		}
	}

	std::set< int64 > buildoutObjects;

	//-- load area buildouts
	SharedBuildoutAreaManager::load(sceneName);
	const std::vector<BuildoutArea> &buildoutAreas = SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene();

	for (std::vector<BuildoutArea>::const_iterator buildoutAreaIter = buildoutAreas.begin(); buildoutAreaIter != buildoutAreas.end(); ++buildoutAreaIter)
	{
		const BuildoutArea &buildoutArea = *buildoutAreaIter;

		char areaFilename[256];
		IGNORE_RETURN(snprintf(areaFilename, sizeof(areaFilename)-1, "datatables/buildout/%s/%s.iff", sceneName, buildoutArea.areaName.c_str()));
		areaFilename[sizeof(areaFilename)-1] = '\0';

		Iff iff;
		if (  !buildoutArea.areaName.empty() 
		   && buildoutArea.areaName != ms_excludeArea
		   && iff.open(areaFilename, true)
		   && !iff.atEndOfForm()
		)
		{
			DataTable areaBuildoutTable;
			areaBuildoutTable.load(iff);

			int const buildoutRowCount = areaBuildoutTable.getNumRows();
			if (buildoutRowCount > 0)
			{
				int64 buildingObjId     = buildoutArea.getSharedBaseId();
				int64 objIdBase         = buildingObjId + cs_buildingObjIdOffset;
				int64 currentBuilding   = 0;
				int64 currentCell       = 0;

				std::vector<int> buildingCellIds;

				int const objIdColumn = areaBuildoutTable.findColumnNumber( "objid" );
				int const containerColumn = areaBuildoutTable.findColumnNumber( "container" );
//				int const typeColumn = areaBuildoutTable.findColumnNumber("type");
				int const sharedTemplateCrcColumn = areaBuildoutTable.findColumnNumber("shared_template_crc");
				int const cellIndexColumn = areaBuildoutTable.findColumnNumber("cell_index");
				int const pxColumn = areaBuildoutTable.findColumnNumber("px");
				int const pyColumn = areaBuildoutTable.findColumnNumber("py");
				int const pzColumn = areaBuildoutTable.findColumnNumber("pz");
				int const qwColumn = areaBuildoutTable.findColumnNumber("qw");
				int const qxColumn = areaBuildoutTable.findColumnNumber("qx");
				int const qyColumn = areaBuildoutTable.findColumnNumber("qy");
				int const qzColumn = areaBuildoutTable.findColumnNumber("qz");
				int const radiusColumn = areaBuildoutTable.findColumnNumber("radius");
				int const portalLayoutCrcColumn = areaBuildoutTable.findColumnNumber("portal_layout_crc");
				std::string const requiredEvent = buildoutArea.getRequiredEventName();

				FATAL(sharedTemplateCrcColumn < 0, ("Unable to find column [shared_template_crc] in [%s]", areaFilename));
				FATAL(cellIndexColumn < 0, ("Unable to find column [cell_index] in [%s]", areaFilename));
				FATAL(pxColumn < 0, ("Unable to find column [px] in [%s]", areaFilename));
				FATAL(pyColumn < 0, ("Unable to find column [py] in [%s]", areaFilename));
				FATAL(pzColumn < 0, ("Unable to find column [pz] in [%s]", areaFilename));
				FATAL(qwColumn < 0, ("Unable to find column [qw] in [%s]", areaFilename));
				FATAL(qxColumn < 0, ("Unable to find column [qx] in [%s]", areaFilename));
				FATAL(qyColumn < 0, ("Unable to find column [qy] in [%s]", areaFilename));
				FATAL(qzColumn < 0, ("Unable to find column [qz] in [%s]", areaFilename));
				FATAL(radiusColumn < 0, ("Unable to find column [radius] in [%s]", areaFilename));
				FATAL(portalLayoutCrcColumn < 0, ("Unable to find column [portal_layout_crc] in [%s]", areaFilename));
				
				int buildOutFileVersion = 1;

				if ( objIdColumn != -1 )
				{
					buildOutFileVersion = 2;

					FATAL(containerColumn < 0, ("Unable to find column [container] in [%s]", areaFilename));
				}
				else
				{
					FATAL(containerColumn != -1, ("Missing column [objid] but found column [container] in [%s]", areaFilename));
				}
			
				DEBUG_REPORT_LOG (ms_reportBuildoutVersions, ("WorldSnapshot::load - Buildout table [%s] is version [%d]\n", areaFilename, buildOutFileVersion));


				for (int buildoutRow = 0; buildoutRow < buildoutRowCount; ++buildoutRow)
				{
					int64 objId       = 0;
					int64 containerId = 0;

					unsigned int const cellIndex         = areaBuildoutTable.getIntValue(cellIndexColumn, buildoutRow);
					const uint32       portalLayoutCrc   = static_cast<uint32>(areaBuildoutTable.getIntValue(portalLayoutCrcColumn, buildoutRow));

					uint32 const sharedTemplateCrc = static_cast<uint32>(areaBuildoutTable.getIntValue(sharedTemplateCrcColumn, buildoutRow));
					float const  radius            = areaBuildoutTable.getFloatValue(radiusColumn, buildoutRow);

					// ------------------------------------------------------------------------------------------------------
					/*
					{
						ObjectTemplate const * const ot = ObjectTemplateList::fetch(sharedTemplateCrc);
						SharedObjectTemplate const * const sharedTemplate = safe_cast<SharedObjectTemplate const *>(ot);
						if (!sharedTemplate)
						{
							FATAL(true, ("WorldSnapshot unable to obj from buildout table [%s] row [%d/%d]", areaBuildoutTable.getName().c_str(), buildoutRow, buildoutRowCount));
							continue;
						}
					}
					*/
					// TODO - commented out fetch above does not seem necessary other than to do the error chech.  The following
					// code only validates the CRC.  Is that sufficient?  Was a special error check needed here or was this some
					// temporary debugging code somebody forgot to remove?
					const ConstCharCrcString sharedTemplateName = ObjectTemplateList::lookUp(sharedTemplateCrc);
					FATAL(sharedTemplateName.isEmpty(), ("WorldSnapshot unable to lookup obj from buildout table [%s] row [%d/%d]", areaBuildoutTable.getName().c_str(), buildoutRow, buildoutRowCount));
					// ------------------------------------------------------------------------------------------------------
					
					if ( buildOutFileVersion == 1 )
					{
						if ( portalLayoutCrc ) // if ( is a building )
						{
							objId = buildingObjId++;
							currentBuilding = objId;
						}
						// else if ( is a cell )
						else if ( sharedTemplateCrc == cs_sharedCellObjectTemplate_tag )
						{
							objId = buildingObjId++;
							currentCell = objId;
							containerId = currentBuilding;
						}
						// else if ( an object in a cell )
						else if ( cellIndex > 0 )
						{
							objId = objIdBase++;
							containerId = currentCell;
						}
						// else is an object not in a cell
						else
						{
							objId = objIdBase++;
						}

						FATAL( buildingObjId >= buildoutArea.getSharedBaseId() + cs_buildingObjIdOffset, ( "building object id overflow" ) );
					}
					else
					{
						objId       = areaBuildoutTable.getIntValue(objIdColumn, buildoutRow);
						containerId = areaBuildoutTable.getIntValue(containerColumn, buildoutRow);

						// with new buildout files, the object id is a random 31-bit negative value
						// then we give the area index some bits in the upper part of the number
						// by shifting the area index value left 48 bits.

						const int64 areaIndex = buildoutArea.areaIndex + 1;


						if ( objId < 0 )
						{
							objId ^= areaIndex << 48;
						}


						if ( containerId < 0  )
						{
							containerId ^= areaIndex << 48;
						}

						// this field is not used right now
						//type = static_cast<uint32>(areaBuildoutTable.getIntValue(typeColumn,buildoutRow));
					}

					FATAL( portalLayoutCrc && ( cellIndex != 0 || containerId != 0 ), ( "Tried to add a pob to a cell or other container.  This probably means that your buildout %s is corrupt.", buildoutArea.areaName.c_str() ) );


					Transform xform;

					Quaternion const q(
						areaBuildoutTable.getFloatValue(qwColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(qxColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(qyColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(qzColumn, buildoutRow));

					q.getTransform(&xform);

					if ( cellIndex == 0 )
					{
						xform.setPosition_p(
							buildoutArea.rect.x0+areaBuildoutTable.getFloatValue(pxColumn, buildoutRow),
							areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
							buildoutArea.rect.y0+areaBuildoutTable.getFloatValue(pzColumn, buildoutRow));
					}
					else
					{
						xform.setPosition_p(
							areaBuildoutTable.getFloatValue(pxColumn, buildoutRow),
							areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
							areaBuildoutTable.getFloatValue(pzColumn, buildoutRow));
					}
			
					if ( !containerId || buildoutObjects.find( containerId ) != buildoutObjects.end() )
					{
						ms_reader.addObject(
							objId,
							containerId,
							ObjectTemplateList::lookUp(sharedTemplateCrc),
							cellIndex,
							xform,
							radius,
							portalLayoutCrc,
							requiredEvent);
					}

					buildoutObjects.insert( objId );

				}
			}
		}
	}

	//-- add all objects to the sphere tree
	{
		const int n = ms_reader.getNumberOfNodes ();
		int i;
		for (i = 0; i < n; ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);

			//
			// if the object is not a buildout object
			// or the object is a top level object
			// then add it to the sphere tree
			//
			const bool isSinglePlayer = Game::getSinglePlayer();
			if ( isSinglePlayer
				|| isInSet( buildoutObjects, node->getNetworkIdInt() ) == false 
				|| ( node->getPortalLayoutCrc() == 0 && node->getContainedByNetworkIdInt() == 0 ) )
			{
				node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));
			}
			
		}

		if (ms_reader.getNumberOfNodes ())
			DEBUG_REPORT_LOG (true, ("WorldSnapshot [%s]: %i object templates, %i root objects, %i total objects\n", sceneName, ms_reader.getNumberOfObjectTemplateNames (), ms_reader.getNumberOfNodes (), ms_reader.getTotalNumberOfNodes ()));
	}

	//-- setup to preload all the object templates
	ms_numberOfObjectTemplates = ms_reader.getNumberOfObjectTemplateNames ();
	ms_preloadObjectTemplate = 0;
	preloadSomeAssets ();
}

//-------------------------------------------------------------------

void WorldSnapshot::setExcludeArea(char const *areaName)
{
	ms_excludeArea = areaName;
}

//-------------------------------------------------------------------

void WorldSnapshot::preloadSomeAssets ()
{
	if (ConfigClientGame::getPreloadWorldSnapshot())
	{
		PerformanceTimer preloadTimer;
		preloadTimer.start();
		int objectsLoaded = 0;

		while (ms_preloadObjectTemplate < ms_numberOfObjectTemplates && preloadTimer.getSplitTime () < cms_callbackTime)
		{
#if PRODUCTION == 0
			const int numberOfFilesOpenedTotal = TreeFile::getNumberOfFilesOpenedTotal ();
			const int sizeOfFilesOpenedTotal = TreeFile::getSizeOfFilesOpenedTotal ();
			UNREF (numberOfFilesOpenedTotal);
			UNREF (sizeOfFilesOpenedTotal);

			PerformanceTimer objectTimer;
			objectTimer.start ();
#endif

			const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch (ms_reader.getObjectTemplateName (ms_preloadObjectTemplate));
			if (objectTemplate)
			{
				objectTemplate->preloadAssets ();
				ms_preloadObjectTemplateList.push_back (objectTemplate);
			}

#if PRODUCTION == 0
			objectTimer.stop ();
			REPORT_LOG (ms_logWorldSnapshotCreates, ("WorldSnapshot: %1.2f\t%i\t%i\t%s\n", objectTimer.getElapsedTime (), TreeFile::getNumberOfFilesOpenedTotal () - numberOfFilesOpenedTotal, TreeFile::getSizeOfFilesOpenedTotal () - sizeOfFilesOpenedTotal, objectTemplate->getName ()));
#endif

			++ms_preloadObjectTemplate;
			++objectsLoaded;
		}

#if PRODUCTION == 0
		REPORT_LOG(ms_preloadSomeAssetsLogs && ms_preloadObjectTemplate && ms_preloadObjectTemplate <= ms_numberOfObjectTemplates, ("preloaded %d assets in %f seconds (%d/%d)\n", objectsLoaded, preloadTimer.getSplitTime (), ms_preloadObjectTemplate, ms_numberOfObjectTemplates));
#endif
	}
}

//-------------------------------------------------------------------

int WorldSnapshot::getLoadingPercent ()
{
	if (! ConfigClientGame::getPreloadWorldSnapshot () || !ms_numberOfObjectTemplates)
		return 100;

	return (ms_preloadObjectTemplate * 100) / ms_numberOfObjectTemplates;
}

//-------------------------------------------------------------------

bool WorldSnapshot::donePreloading ()
{
	if (!ConfigClientGame::getPreloadWorldSnapshot () || (ms_preloadObjectTemplate >= ms_numberOfObjectTemplates))
		return true;

	return false;
}

//-------------------------------------------------------------------

static bool compareNodesForCreate (const WorldSnapshotReaderWriter::Node* const a, const WorldSnapshotReaderWriter::Node* const b)
{
	return a->getDistanceSquaredTo () < b->getDistanceSquaredTo ();
}

//-------------------------------------------------------------------

static bool compareNodesForDelete (const WorldSnapshotReaderWriter::Node* const a, const WorldSnapshotReaderWriter::Node* const b)
{
	return b->getDistanceSquaredTo () < a->getDistanceSquaredTo ();
}

//-------------------------------------------------------------------

void WorldSnapshot::update(CellProperty const * const cellProperty, Vector const & position_w)
{
	PROFILER_AUTO_BLOCK_DEFINE("WorldSnapshot::update");

	if (ms_pendingCreateList.empty() && ms_pendingDeleteList.empty() && cellProperty == ms_lastCellProperty && ms_lastPosition_w.magnitudeBetweenSquared (position_w) < ms_updateDistanceSquared)
		return;

	DEBUG_REPORT_LOG (ms_logUpdate, ("WorldSnapshot::update: cell=%s position_w=<%1.2f, %1.2f, %1.2f>\n", cellProperty ? cellProperty->getCellName() : "(null)", position_w.x, position_w.y, position_w.z));
	ms_lastCellProperty = cellProperty;
	ms_lastPosition_w = position_w;

#if PRODUCTION == 0
	PerformanceTimer timer;
	timer.start ();

	int ms_numberOfQueries = 0;
	int ms_numberOfPendingCreates = 0;
	int ms_numberOfPendingDeletes = 0;
#endif

	//-- the first update's pending create query should ask the sphere tree for what should be loaded
	ms_queryList.clear ();

	ms_sphereTree.findInRange (position_w, 1.f, ms_queryList);

	if (!ms_queryList.empty () || !ms_loadedList.empty ())
	{
		if (!ms_queryList.empty ())
			std::sort (ms_queryList.begin (), ms_queryList.end ());

		if (!ms_loadedList.empty ())
			std::sort (ms_loadedList.begin (), ms_loadedList.end ());

#if 1
		ms_pendingCreateList.clear ();
		ms_pendingDeleteList.clear ();

		{
			size_t queryIndex = 0;
			size_t const querySize = ms_queryList.size ();
			size_t loadedIndex = 0;
			size_t const loadedSize = ms_loadedList.size ();

			while (queryIndex < querySize || loadedIndex < loadedSize)
			{
				WorldSnapshotReaderWriter::Node const * const queryNode = queryIndex < querySize ? ms_queryList [queryIndex] : 0;
				WorldSnapshotReaderWriter::Node const * const loadedNode = loadedIndex < loadedSize ? ms_loadedList [loadedIndex] : 0;

				//-- see if we only have creates remaining
				if (queryNode && !loadedNode)
				{
					ms_pendingCreateList.push_back (queryNode);
					++queryIndex;
				}
				else
				{
					//-- see if we only have deletes remaining
					if (!queryNode && loadedNode)
					{
						ms_pendingDeleteList.push_back (loadedNode);
						++loadedIndex;
					}
					else
					{
						//-- if they are the same, we advance both
						if (queryNode == loadedNode)
						{
							++queryIndex;
							++loadedIndex;
						}
						else
						{
							if (queryNode < loadedNode)
							{
								//-- we need to create
								ms_pendingCreateList.push_back (queryNode);
								++queryIndex;
							}
							else
							{
								//-- we need to destroy
								ms_pendingDeleteList.push_back (loadedNode);
								++loadedIndex;
							}
						}
					}
				}
			}
		}
#else
		//-- anything in the query list that is not in the loaded list must be created
		ms_pendingCreateList.clear ();

		{
			uint i;
			for (i = 0; i < ms_queryList.size (); ++i)
				if (!std::binary_search (ms_loadedList.begin (), ms_loadedList.end (), ms_queryList [i]))
				{
					ms_queryList [i]->computeDistanceSquaredTo (position_w);
					ms_pendingCreateList.push_back (ms_queryList [i]);
				}
		}

		//-- anything in the loaded list that is not in the query list must be deleted
		ms_pendingDeleteList.clear ();

		{
			uint i;
			for (i = 0; i < ms_loadedList.size (); ++i)
				if (!std::binary_search (ms_queryList.begin (), ms_queryList.end (), ms_loadedList [i]))
				{
					ms_loadedList [i]->computeDistanceSquaredTo (position_w);
					ms_pendingDeleteList.push_back (ms_loadedList [i]);
				}
		}
#endif

#if PRODUCTION == 0
		ms_numberOfQueries = static_cast<int> (ms_queryList.size ());
		ms_numberOfPendingCreates = static_cast<int> (ms_pendingCreateList.size ());
		ms_numberOfPendingDeletes = static_cast<int> (ms_pendingDeleteList.size ());

		if (ms_vtuneWorldSnapshotCreates)
			VTune::resume();
#endif

		//-- create all pending creates
		{
			std::sort (ms_pendingCreateList.begin (), ms_pendingCreateList.end (), compareNodesForCreate);

			size_t const n = std::min(ms_pendingCreateList.size(), static_cast<size_t>(ms_maximumNumberOfCreatesPerFrame));
			for (size_t i = 0; i < n; ++i)
			{
				const WorldSnapshotReaderWriter::Node* const node = ms_pendingCreateList [i];
				
				// If one of our pending creates is an Event based node - we hold on to it for now.
				// We'll handle loading/deleteing event based objects later.
				if(!node->getEventName().empty())
				{
					BuildoutEventMap::iterator iter = ms_eventObjectMap.find(node->getEventName());
					if(  iter != ms_eventObjectMap.end())
					{
						(*iter).second.push_back(node);
						continue;
					}
					else
					{
						std::pair< BuildoutEventMap::iterator, bool> insertIter;
						insertIter = ms_eventObjectMap.insert(std::pair<std::string, std::list<const WorldSnapshotReaderWriter::Node*> >(node->getEventName(), std::list<const WorldSnapshotReaderWriter::Node*>()));
						
						// This is really ugly syntax wise. Basically we are checking to see if we had a valid map insertion, and then adding the node to the event list.
						if(insertIter.second)
							insertIter.first->second.push_back(node);

						continue;
					}
				}

				//-- create the object
				CreateErrorCode result;
				Object* const object = createObject (ms_reader, node, result);

				if (object)
				{
					addObjectToWorld(object, node);
				}
				else
				{
					//-- remove node from sphere tree
					ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
					node->setSpatialSubdivisionHandle (0);

					switch (result)
					{
					case CEC_objectAlreadyExists:
						DEBUG_WARNING (true, ("WorldSnapshot::update - refused to create object %I64i [%s] because it already exists in the world\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
						break;

					case CEC_orphanedAtOrigin:
						DEBUG_WARNING (true, ("WorldSnapshot::update - refused to create object %I64i [%s] because it is orphaned at the origin\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
						break;

					case CEC_mismatchedPobCrc:
						DEBUG_WARNING (true, ("WorldSnapshot::update - object %I64i [%s] could not be created because the pob crcs do not match\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
						break;

					case CEC_tooCloseToOrigin:
						DEBUG_WARNING (true, ("WorldSnapshot::update - refused to create object %I64i [%s] because it is suspiciously close to the origin of the world\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
						break;

					default:
						break;
					}
				}
			}
		}

		//-- create all pending creates in the same cell
		{
			//-- find all visible cells
			RenderWorld::CellPropertyList const & cellPropertyList = RenderWorld::getVisibleCells();
			for (size_t i = 0; i < cellPropertyList.size(); ++i)
			{
				CellProperty const * const cellProperty = cellPropertyList[i];
				if (!cellProperty || cellProperty == CellProperty::getWorldCellProperty ())
					continue;

				if (!cellProperty->getOwner().isInWorld())
					continue;

				const WorldSnapshotReaderWriter::Node* const cellNode = ms_reader.find ( cellProperty->getOwner ().getNetworkId ().getValue () );
				if (cellNode && !cellNode->isInWorld ())
				{
					cellNode->addToWorld ();

					int j;
					for (j = 0; j < cellNode->getNumberOfNodes (); ++j)
					{
						const WorldSnapshotReaderWriter::Node* const node = cellNode->getNode (j);

						if (NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (node->getNetworkIdInt ()))) == 0)
						{
							CreateErrorCode result;
							Object* const object = createObject (ms_reader, node, result);
							if (object)
								addObjectToWorld(object, node);
						}
					}
				}
			}
		}

		//-- delete all pending deletes
		{
			std::sort (ms_pendingDeleteList.begin (), ms_pendingDeleteList.end (), compareNodesForDelete);

			size_t const n = std::min(ms_pendingDeleteList.size(), static_cast<size_t>(ms_maximumNumberOfDeletesPerFrame));
			for (size_t i = 0; i < n; ++i)
			{
				const WorldSnapshotReaderWriter::Node* const node = ms_pendingDeleteList [i];

				//-- Enlarge the radius at which an object will be deleted by 128 meters of it
				if (node->getDistanceSquaredTo() < sqr(node->getRadius()) + 128.f)
					continue;

				node->removeFromWorld ();

				//-- find the object
				Object* const object = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (node->getNetworkIdInt ())));

				//-- destroy the object
				if (object)
				{
					//-- prevent objects that have non-client cached objects within them from being deleted
					if (ContainerInterface::isClientCachedOnly (*safe_cast<ClientObject*> (object)))
					{
						if (object->isInWorld ())
							object->removeFromWorld ();
						else
							DEBUG_WARNING (true, ("WorldSnapshot::update - deleting client cached object %i which is not in the world\n", node->getNetworkIdInt ()));

						delete object;
					}
				}
				else
					DEBUG_WARNING (true, ("WorldSnapshot::update - attempted to delete client cached object %i which does not exist\n", node->getNetworkIdInt ()));

				//-- the object is now deleted
				NodeList::iterator iter = std::find (ms_loadedList.begin (), ms_loadedList.end (), node);
				IGNORE_RETURN (ms_loadedList.erase (iter));
			}
		}
#if PRODUCTION == 0
		if (ms_vtuneWorldSnapshotCreates)
			VTune::pause();
#endif

	}

#if PRODUCTION == 0
	timer.stop ();
	ms_lastUpdateTime = timer.getElapsedTime ();
	REPORT_LOG (ms_logUpdate, ("query = %i, create = %i, delete = %i, time = %.5f\n", ms_queryList.size (), ms_pendingCreateList.size (), ms_pendingDeleteList.size (), ms_lastUpdateTime));
#endif
}

//-------------------------------------------------------------------

bool WorldSnapshot::isClientCached (const int64 networkIdInt)
{
	return ms_reader.find (networkIdInt) != 0;
}

//-------------------------------------------------------------------

Object* WorldSnapshot::addObject(
	int64            networkIdInt,
	int64            containerIdInt,
	CrcString const &sharedTemplate,
	Transform const &transform_p,
	float            radius,
	uint32           portalLayoutCrc,
	int              cellCount,
	std::string const& requiredEvent)
{
	WorldSnapshotReaderWriter::Node const * const node = ms_reader.addObject(
		networkIdInt,
		containerIdInt,
		sharedTemplate,
		cellCount,
		transform_p,
		radius,
		portalLayoutCrc, 
		requiredEvent);

	// TODO: We probably don't want to load an object if the event required for that object isn't currently active.

	for (int i = 0; i < cellCount; ++i)
	{
		ms_reader.addObject(
			networkIdInt+i+1,
			networkIdInt,
			ConstCharCrcString("object/cell/shared_cell.iff"),
			i+1,
			Transform::identity,
			0,
			0);
	}


	Object *object = 0;

	CreateErrorCode result;
	object = createObject(ms_reader, node, result);
	if (object)
		object->addToWorld();

	return object;
}

//-------------------------------------------------------------------

void WorldSnapshot::moveObject(int64 networkIdInt, Transform const &transform_p)
{
	WorldSnapshotReaderWriter::Node * const node = ms_reader.find(networkIdInt);
	if (node)
	{
		node->setTransform_p(transform_p);
		if (node->getSpatialSubdivisionHandle())
			ms_sphereTree.move(node->getSpatialSubdivisionHandle());
	}
}

//-------------------------------------------------------------------

void WorldSnapshot::removeObject (const int64 networkIdInt)
{
	const WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (node && !node->isDeleted () && node->getSpatialSubdivisionHandle ())
	{
		ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
		node->setSpatialSubdivisionHandle (0);
	}

	ms_reader.removeNode (networkIdInt);
}

//-------------------------------------------------------------------

float WorldSnapshot::getDetailLevelBias ()
{
	return WorldSnapshotReaderWriter::Node::getDetailLevelBias ();
}

//-------------------------------------------------------------------

void WorldSnapshot::setDetailLevelBias (const float detailLevelBias)
{
	WorldSnapshotReaderWriter::Node::setDetailLevelBias (detailLevelBias);
}

//-------------------------------------------------------------------

void WorldSnapshot::detailLevelChanged ()
{
	//-- make sure we have objects
	if (ms_reader.getNumberOfNodes () == 0)
		return;

	//-- save off all objects in the sphere tree
	NodeList saveList;
	saveList.reserve (ms_reader.getNumberOfNodes ());

	//-- clear out the sphere tree
	{
		int i;
		for (i = 0; i < ms_reader.getNumberOfNodes (); ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
			if (node->getSpatialSubdivisionHandle ())
			{
				ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
				node->setSpatialSubdivisionHandle (0);

				saveList.push_back (node);
			}
		}
	}

	//-- recreate the sphere tree
	{
		uint i;
		for (i = 0; i < saveList.size (); ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
			node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));
		}

		saveList.clear ();
	}
}

//-------------------------------------------------------------------

void WorldSnapshot::loadIfClientCached(NetworkId const &networkId)
{
	if (networkId.getValue() <= std::numeric_limits<int>::max()) // <- i have no idea why this is here
	{
		WorldSnapshotReaderWriter::Node const *node = ms_reader.find( networkId.getValue() ) ;
		while (node && node->getParent())
			node = node->getParent();
		if (node)
		{
			CreateErrorCode result;
			Object *object = createObject(ms_reader, node, result);
			if (object)
				addObjectToWorld(object, node);
		}
	}
}

//-------------------------------------------------------------------

NetworkId WorldSnapshot::findClosestCellIdFromWorldPosition(Vector const & position_w)
{
	NetworkId cellid = NetworkId::cms_invalid;
	
	NodeList objects;
	ms_sphereTree.findAtPoint(position_w, objects);
	
	for (NodeList::const_iterator itObj = objects.begin(); itObj != objects.end(); ++itObj) 
	{
		WorldSnapshotReaderWriter::Node const * const node = *itObj;
		if (node != NULL) 
		{
			Object const * const object = NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(node->getNetworkIdInt())));
			if (object)
			{
				PortalProperty const * const portal = object->getPortalProperty();
				if (portal) 
				{
					Vector position_l = object->rotateTranslate_w2o(position_w);

					if (node->getSphere().contains(position_l)) 
					{
						CellProperty const * const cell = const_cast<PortalProperty *>(portal)->findContainingCell(position_l);
						NetworkId const & containingCellId = cell->getOwner().getNetworkId();
						if (containingCellId.isValid()) 
						{
							cellid = containingCellId;
							break;
						}
					}
				}
			}
		}
	}
	
	return cellid;
}

//-------------------------------------------------------------------

void WorldSnapshot::removeEventObjects(const std::string & eventName)
{
	if(ms_eventObjectMap.empty())
		return;

	BuildoutEventMap::iterator iter = ms_eventObjectMap.find(eventName);

	if(iter == ms_eventObjectMap.end())
		return;

	std::list<const WorldSnapshotReaderWriter::Node*> * eventNodeList = &(*iter).second;
	std::list<const WorldSnapshotReaderWriter::Node*>::iterator nodeIter = eventNodeList->begin();

	for(; nodeIter != eventNodeList->end(); ++nodeIter)
	{
		const WorldSnapshotReaderWriter::Node* removeNode = (*nodeIter);
		
		if (removeNode->getSpatialSubdivisionHandle ())
		{
			ms_sphereTree.removeObject (removeNode->getSpatialSubdivisionHandle ());
			removeNode->setSpatialSubdivisionHandle (0);
		}

		removeNode->removeFromWorld();

		Object * const object = NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(removeNode->getNetworkIdInt())));

		if (object)
		{
			if(object->isInWorld())
				object->removeFromWorld();

			delete object;
		}
	}


}

//-------------------------------------------------------------------

void WorldSnapshot::addEventObjects(const std::string & eventName)
{
	if(ms_eventObjectMap.empty())
		return;

	BuildoutEventMap::iterator iter = ms_eventObjectMap.find(eventName);

	if(iter == ms_eventObjectMap.end())
		return;

	std::list<const WorldSnapshotReaderWriter::Node*> * eventNodeList = &(*iter).second;
	std::list<const WorldSnapshotReaderWriter::Node*>::iterator nodeIter = eventNodeList->begin();

	for(; nodeIter != eventNodeList->end(); ++nodeIter)
	{
		const WorldSnapshotReaderWriter::Node* node = (*nodeIter);
		//-- create the object
		CreateErrorCode result;
		Object* const object = createObject (ms_reader, node, result);

		if (object)
		{
			addObjectToWorld(object, node);
		}
		else
		{
			//-- remove node from sphere tree
			ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
			node->setSpatialSubdivisionHandle (0);

			switch (result)
			{
			case CEC_objectAlreadyExists:
				DEBUG_WARNING (true, ("WorldSnapshot::addEventObjects - refused to create object %I64i [%s] because it already exists in the world\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
				break;

			case CEC_orphanedAtOrigin:
				DEBUG_WARNING (true, ("WorldSnapshot::addEventObjects - refused to create object %I64i [%s] because it is orphaned at the origin\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
				break;

			case CEC_mismatchedPobCrc:
				DEBUG_WARNING (true, ("WorldSnapshot::addEventObjects - object %I64i [%s] could not be created because the pob crcs do not match\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
				break;

			case CEC_tooCloseToOrigin:
				DEBUG_WARNING (true, ("WorldSnapshot::addEventObjects - refused to create object %I64i [%s] because it is suspiciously close to the origin of the world\n", node->getNetworkIdInt (), ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ())));
				break;

			default:
				break;
			}
		}
	}
}

//===================================================================
