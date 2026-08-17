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
#include <set>

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
	// CONSULT-61: was 1.f -- a full SECOND of synchronous template preloading per
	// loading-screen frame. The audio mixer's queue cannot ride out back-to-back
	// 1s frames; 50ms slices do the same total work across more frames and keep
	// music fed.
	float const cms_callbackTime = 0.05f;

	CellProperty const * ms_lastCellProperty;
	Vector ms_lastPosition_w (0.f, -9999.f, 0.f);
	float ms_updateDistanceSquared;
	int ms_maximumNumberOfCreatesPerFrame = 1000;
	int ms_maximumNumberOfDeletesPerFrame = 1000;

	// Per-frame wall-time budget (ms) for the update() create/delete drain
	// (0 = unlimited). The count caps above bound ITEMS, but a create's cost
	// varies ~1000x (a building pulls templates + appearances + collision
	// buckets + portal/pathfinding from cold disk) -- the zone-in burst ran
	// 300ms in one frame. Un-drained nodes stay pending and re-enter next
	// frame's diff (the update() early-out already yields to non-empty pending
	// lists), nearest-first. Checked BETWEEN items, so one cold item can still
	// overshoot -- this bounds the accumulation, not the worst single create.
	int ms_createTimeBudgetMs = 6;

	//-- KILL SWITCH for the restored delete drain ([ClientGame/WorldSnapshot]
	//   streamOutSnapshotObjects, default true). The drain below had never
	//   actually deleted anything -- its guard read a distance key nothing
	//   recomputed (see the note in update()) -- so restoring it turns on a
	//   streaming path with no field history. Set false to get the
	//   never-stream-out behaviour back without a rebuild.
	bool ms_streamOutSnapshotObjects = true;

	//-- CONSULT-60: phased load state. WorldSnapshot::load() used to parse the
	//   whole .ws + all buildout tables synchronously inside the GroundScene
	//   constructor (a multi-second frame, BEFORE the loading screen was even
	//   enabled). The heavy work now runs in budgeted loadStep() calls; these
	//   cursors persist between frames. ms_parsePending gates donePreloading()/
	//   getLoadingPercent() (the preload counters are 0 mid-parse and would
	//   otherwise read as done).
	enum ParsePhase
	{
		PP_wsNodes,
		PP_buildout,
		PP_sphereTree,
		PP_done
	};
	ParsePhase ms_parsePhase = PP_done;
	bool ms_parsePending = false;

	// CONSULT-71 occupancy probe: which unload() call path is running. Set by each
	// of the three call sites (remove/load/wsUnloadSnapshot) because Codex's call-graph
	// pass established there is NO state reachable from inside unload() that reliably
	// distinguishes a real zone change from an in-place editor reload -- GameNetwork
	// connection state is live in both, and ms_sceneName is set BEFORE the call by
	// load() but cleared AFTER it by wsUnloadSnapshot.
	char const * ms_unloadReason = "unset";

	// CONSULT-71: how many non-client-cached (server-owned) roots the last unload()
	// REFUSED to delete. Surfaced by wsUnloadSnapshot so the toolkit can tell the user
	// "N occupied buildings kept -- their edits show after a zone change or relog".
	int ms_lastUnloadSkippedRoots = 0;
	Iff* ms_parseIff = 0;
	int  ms_buildoutAreaIndex = 0;
	int  ms_sphereNodeIndex = 0;
	std::set<int64> ms_buildoutObjects;

	//-- Goal B Wave 3 (2026-07-18 self-test finding): buildout provenance by
	//   NODE IDENTITY, not id. SWGSource v2 buildout tables (TOC-indexed for the
	//   regular planets) legitimately carry POSITIVE objids, so an id can no
	//   longer discriminate buildout from authored (and on an id collision the
	//   id-keyed filter would drop the AUTHORED node from save). This set holds
	//   the top-level reader nodes INSERTED by loadOneBuildoutArea -- exact by
	//   construction; cleared with ms_buildoutObjects on load/unload.
	std::set<const WorldSnapshotReaderWriter::Node*> ms_buildoutTopLevelNodes;

	//-- Goal B Wave 1 (hookpoints v17): snapshot generation for the editor read
	//   shims (engine_wsGetGeneration, end of file). Bumps on unload (which every
	//   load routes through) so the consumer invalidates cached rows + undo
	//   targets across snapshot generations. A pure counter -- never gates logic.
	int ms_wsEditGeneration = 0;

	//-- Goal B Wave 3 gate aid: when set, the phased parse's completion runs one
	//   engine_wsSaveSnapshot and logs the typed result -- a REAL save exercised
	//   by a normal world entry (the Wave-2 lesson: gates that never exercise
	//   the path ship the bug). Default off; harmless to leave in-tree.
	bool ms_wsSelfTestSaveOnLoad = false;

	void loadOneBuildoutArea (const BuildoutArea& buildoutArea);
	void finishLoadNow ();

	//-- refusal-reason diagnostics (consumer request 2026-07-18): every fail-closed
	//   branch of the mutation shims logs ONE line naming the branch + the offending
	//   value. On-demand editor actions -- no spam risk; PERMANENT by design
	//   ("silently did nothing" is the failure mode the whole consult exists to
	//   prevent; Wave 3's save shims inherit the same discipline).
	//
	//   Lives HERE, not with the editor shims (moved 2026-08-07, when those still sat
	//   inside a Win32-only advertise guard and defining it down there left x64 with a
	//   declaration and no definition, LNK2019). The guard is gone since the 2026-08-15
	//   x64 port, but update() uses this too, so up here stays the right home.
	const char* const cs_wsCreateErrorCodeNames[] = { "objectAlreadyExists", "orphanedAtOrigin", "mismatchedPobCrc", "tooCloseToOrigin" };

	inline const char* wsCreateErrorCodeName (const CreateErrorCode result)
	{
		const int index = static_cast<int> (result);
		if (index >= 0 && index < static_cast<int> (sizeof (cs_wsCreateErrorCodeNames) / sizeof (cs_wsCreateErrorCodeNames [0])))
			return cs_wsCreateErrorCodeNames [index];

		return "unknown";
	}

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
		//-- ObjectTemplateList::fetch resolves ANY template class, and safe_cast is a
		//   bare static_cast in Release (SafeCast.h) -- so a node naming a class outside
		//   the SharedObjectTemplate hierarchy yielded a non-null WRONGLY-TYPED pointer
		//   that instantiateObject then dereferenced (getPortalLayoutFilename, then
		//   createObject). Narrow with the virtual asSharedObjectTemplate (0 in the
		//   base) and fail closed; the caller strips the node from the sphere tree, so
		//   this cannot storm.
		SharedObjectTemplate const * const sharedObjectTemplate = ot->asSharedObjectTemplate ();
		if (!sharedObjectTemplate)
		{
			WARNING(true, ("WorldSnapshot WRONG CLASS template [%s] -- not a SharedObjectTemplate; node skipped", objectTemplateName));
			ot->releaseReference ();
			return 0;
		}

		return sharedObjectTemplate;
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
			if (PortalPropertyTemplate::extractPortalLayoutCrc(objectTemplate->getPortalLayoutFilename().c_str(), portalLayoutCrc))
			{
				if (portalLayoutCrc != static_cast<uint32>(node->getPortalLayoutCrc()))
				{
					// POB CRC mismatch: the buildout node recorded a different .pob
					// CRC than the .pob actually loaded from the TREs (common with a
					// reborn client + post-NGE / upstream content). Previously this
					// returned 0 -> the portalized BUILDING was never instantiated ->
					// its server-sent cells couldn't bind -> black "phantom" interiors.
					// Now: log it and PROCEED with the loaded .pob, unless the strict
					// check is explicitly forced (worldSnapshotIgnorePobChanges=false).
					bool const ignorePob = ConfigClientGame::getWorldSnapshotIgnorePobChanges();
					WARNING(true, ("WorldSnapshot createObject [%s] pob crc changed from [%d] to [%d] -- %s",
								   objectTemplate->getName(), static_cast<int>(node->getPortalLayoutCrc()), static_cast<int>(portalLayoutCrc),
								   ignorePob ? "ignoring, proceeding with loaded .pob" : "STRICT, skipping object"));

					if (!ignorePob)
					{
						objectTemplate->releaseReference ();
						objectTemplate=0;

						result = CEC_mismatchedPobCrc;
						return 0;
					}
				}
			}
		}

		//-- instantiate the object
		//-- a template class that does not override createObject() gets the BASE
		//   new Object(this, cms_invalid) (ObjectTemplate.cpp:155-158) -- a plain
		//   Object, not a ClientObject -- which the Release safe_cast would not catch.
		//   Narrow with the virtual asClientObject (0 in the base) and delete the
		//   wrongly-classed object rather than hand it to the caller's setClientCached/
		//   addToWorld chain; it was never added to the world, so plain delete is safe.
		Object *const created = objectTemplate->createObject();
		ClientObject *const object = created ? created->asClientObject() : 0;
		if (created && !object)
		{
			WARNING(true, ("WorldSnapshot WRONG CLASS object from template [%s] -- not a ClientObject; node skipped", objectTemplate->getName()));
			delete created;
		}

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

//-- Editor/diagnostic log line. REPORT_LOG, not DEBUG_REPORT_LOG: every one of these
//   exists precisely because the failure it reports is only ever seen in a Release
//   build. Moved up from the editor-shim block (2026-08-07) so the engine-side probes
//   in update() and suppressObject can use the same sink and prefix as the shims.
#define WS_EDITOR_LOG(printfArgs) REPORT_LOG (true, printfArgs)

//-- Goal B Wave 3: file-scope forward declaration for the loadStep self-test
//   hook (the shim is defined at the end of this TU; a linkage specification
//   is not allowed inside a function body)
extern "C" int __cdecl engine_wsSaveSnapshot (void);

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
	ms_createTimeBudgetMs = ConfigFile::getKeyInt("ClientGame/WorldSnapshot", "createTimeBudgetMs", ms_createTimeBudgetMs);
	ms_streamOutSnapshotObjects = ConfigFile::getKeyBool("ClientGame/WorldSnapshot", "streamOutSnapshotObjects", ms_streamOutSnapshotObjects);
	ms_wsSelfTestSaveOnLoad = ConfigFile::getKeyBool("ClientGame/WorldSnapshot", "wsSelfTestSaveOnLoad", ms_wsSelfTestSaveOnLoad);

	ExitChain::add (remove, "WorldSnapshot::remove");
}

//-------------------------------------------------------------------

void WorldSnapshot::remove ()
{
	ms_unloadReason = "exitchain";  // CONSULT-71 probe tag: process teardown (asserted to still see a live world)
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
	//-- Goal B Wave 1: new snapshot generation (editor cache/undo invalidation)
	++ms_wsEditGeneration;

	ms_lastUnloadSkippedRoots = 0;   // CONSULT-71 guard tally, per unload

	//-- CONSULT-60: cancel any in-flight phased parse (quit during loading /
	//   startScene->startScene). Partial reader state is torn down by the
	//   existing body below; nodes not yet in the sphere tree have a null
	//   spatial handle and are skipped by the removal loop.
	if (ms_parseIff)
	{
		delete ms_parseIff;
		ms_parseIff = 0;
	}
	ms_parsePending = false;
	ms_parsePhase = PP_done;
	ms_buildoutAreaIndex = 0;
	ms_sphereNodeIndex = 0;
	ms_buildoutObjects.clear ();
	ms_buildoutTopLevelNodes.clear ();

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

			// ------------------------------------------------------------------
			// CONSULT-71 occupancy probe ([ClientGame/WorldSnapshot]
			// logUnloadOccupancy, default 0 = OFF). Measures, per node, what this
			// unguarded delete is actually about to destroy.
			//
			// This delete has NO isClientCachedOnly guard, unlike update()'s drain
			// (:~1299). Deleting a POB cascades through TWO Container hops --
			// PortalProperty(Container) deletes the cells, then each
			// CellProperty(Container) deletes its occupants -- and server-streamed
			// NPCs ARE in cell m_contents (ClientObject::depersistContainedBy ->
			// Container::insertNewItem). A client-side delete is invisible to the
			// server, so they never come back: field-confirmed as permanent loss of
			// every NPC inside a POB after one in-place editor reload.
			//
			// The probe answers the question the fix design hinges on: WHICH call
			// paths actually see live objects here. Predictions to falsify --
			//   reason=load on a real zone change -> live=0 for everything, because
			//     ~GroundScene/World::remove already deleted the world before the
			//     new scene's postload reaches us (if this shows live objects, that
			//     ordering claim is wrong);
			//   reason=wsUnload -> live POBs with serverOwned > 0 (the bug);
			//   reason=exitchain -> asserted to still see a fully live world purely
			//     from ExitChain LIFO registration order. UNMEASURED, and the only
			//     load-bearing claim in the analysis not yet observed.
			// serverOwned is counted with the SAME predicate a guard would use, so
			// the numbers also size the refuse-vs-skip decision.
			// ------------------------------------------------------------------
			static int const s_logUnloadOccupancy = ConfigFile::getKeyInt("ClientGame/WorldSnapshot", "logUnloadOccupancy", 0);
			if (s_logUnloadOccupancy)
			{
				int cells = 0;
				int contents = 0;
				int serverOwned = 0;

				if (object)
				{
					PortalProperty const * const portalProperty = object->getPortalProperty();
					if (portalProperty)
					{
						cells = portalProperty->getNumberOfCells();
						for (int cellIndex = 0; cellIndex < cells; ++cellIndex)
						{
							CellProperty const * const cellProperty = portalProperty->getCell(cellIndex);
							if (!cellProperty)
								continue;

							contents += cellProperty->getNumberOfItems();

							// getContents(int) is PROTECTED; the public read path is the
							// const iterator, which yields the CachedNetworkId directly.
							for (ContainerConstIterator it = cellProperty->begin(); it != cellProperty->end(); ++it)
							{
								Object * const occupant = (*it).getObject();
								if (!occupant)
									continue;

								ClientObject * const clientOccupant = dynamic_cast<ClientObject *>(occupant);
								if (clientOccupant && !ContainerInterface::isClientCachedOnly(*clientOccupant))
									++serverOwned;
							}
						}
					}
				}

				// getNetworkIdInt() is int64 -- it MUST NOT be printed with %d. Doing so
				// consumes only 4 of its 8 bytes and shifts every following vararg one
				// slot left, which silently produced impossible readings (live=-55312385)
				// and dropped serverOwned off the end entirely. Cast + %lld.
				if (object || contents)
					REPORT_LOG(true, ("[ws.unload] reason=%s node=%lld live=%d cells=%d contents=%d serverOwned=%d\n",
						ms_unloadReason, static_cast<long long>(node->getNetworkIdInt()), object ? 1 : 0, cells, contents, serverOwned));
			}

			// ------------------------------------------------------------------
			// CONSULT-71 GUARD. This delete used to be unconditional, and it was
			// the ONE snapshot path that violated the invariant update()'s drain
			// already enforces (:~1299): snapshot code never deletes a
			// non-client-cached object. Deleting a POB root cascades through two
			// Container hops -- PortalProperty(Container) takes the cells, then
			// each CellProperty(Container) takes its occupants -- and server
			// NPCs live in cell m_contents (ClientObject::depersistContainedBy ->
			// Container::insertNewItem). A client-side delete is invisible to the
			// server, so an in-place editor reload permanently emptied every POB
			// the player had entered, for the rest of the session.
			//
			// The guard is a NO-OP on every non-editor caller BY CONSTRUCTION,
			// which is why it is safe to apply unconditionally here:
			//   reason=exitchain -- ExitChain is LIFO and IoWinManager::remove is
			//     registered AFTER SetupClientGame (ClientMain.cpp:417 vs :409),
			//     so the IoWin stack is killed first -> ~GroundScene ->
			//     ClientWorld::remove -> World::remove has already deleted the
			//     world and unregistered every id, so getObjectById misses here.
			//   reason=load (real zone change) -- ~GroundScene ran before the new
			//     scene's postload for the same reason; nothing is live either.
			// Only the editor reload reaches this with a live world, which is
			// exactly the path that was losing NPCs. (The armed logUnloadOccupancy
			// probe measures both claims -- two consultants disagreed about the
			// exitchain one, so it is verified rather than assumed.)
			//
			// NOTE we do NOT touch Container::~Container. Ownership cascade
			// semantics are depended on elsewhere (World::remove skips contained
			// objects precisely because "their container will delete them",
			// World.cpp:224-226). We only stop unload() from INITIATING the
			// delete of a non-client-cached root.
			//
			// Residual, accepted deliberately: a surviving POB collides with the
			// re-parsed node on the next load (createObject -> CEC_objectAlready-
			// Exists -> the new node's sphere handle is stripped, :~1210), so the
			// building shows its pre-edit on-disk state until a zone change or
			// relog. That is bounded, visible staleness instead of irreversible,
			// server-invisible data loss.
			// ------------------------------------------------------------------
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

	//-- CONSULT-71: report what the guard preserved (rare and important -- not gated
	//   behind the probe key, which only controls the per-node detail lines).
	if (ms_lastUnloadSkippedRoots > 0)
		REPORT_LOG (true, ("[ws.unload] reason=%s KEPT %d server-owned root(s) -- not deleted (would have cascade-destroyed their cell occupants)\n", ms_unloadReason, ms_lastUnloadSkippedRoots));

	//-- clear out the snapshot
	ms_reader.clear ();
}

//-------------------------------------------------------------------

void WorldSnapshot::load (char const *sceneName)
{
	NOT_NULL(sceneName);

	//-- clear the current snapshot
	//
	//   DO NOT "FIX" THIS BY MOVING THE ALREADY-LOADED TEST ABOVE IT (2026-08-04 --
	//   I tried, and it is strictly worse). This prologue running BEFORE the early
	//   return is the deliberate SAME-SCENE RE-STREAM path: emptying ms_loadedList
	//   makes update() treat every in-range node as needing creation again, so
	//   re-entering a scene rebuilds its objects from the ALREADY-PARSED ms_reader
	//   without paying for the .ws parse a second time. Note ms_sceneName is sticky
	//   across ~GroundScene (only engine_wsUnloadSnapshot clears it, :2947), so
	//   returning early here without clearing the loaded set would leave a
	//   re-entered scene EMPTY.
	//
	//   The invariant it depends on: the CALLER must already have destroyed the
	//   outgoing scene's objects (~GroundScene -> ClientWorld::remove ->
	//   World::remove). If they are still alive, this becomes the failure SWG-Toolkit
	//   reported as "game::loadScene leaves the snapshot incompletely populated until
	//   a manual reload" -- loaded set emptied, so every node is re-queued for
	//   creation, but every one of those creates then fails CEC_objectAlreadyExists
	//   against the surviving NetworkId, and nothing repopulates. That was a defect in
	//   the game::loadScene SHIM (it skipped the close+delete that all three engine
	//   scene installers do), not here. See engine_advertise.cpp engine_gameLoadScene.
	ms_loadedList.clear ();
	ms_reader.removeFromWorld ();
	ms_lastCellProperty = 0;
	ms_lastPosition_w.set (0.f, -9999.f, 0.f);

	//-- see if we're already loaded
	if (_stricmp (sceneName, ms_sceneName.c_str ()) == 0)
	{
		DEBUG_REPORT_LOG (true, ("WorldSnapshot::load - %s is currently loaded\n", sceneName));

		// ====================================================================
		// CONSULT-73 (2026-08-07): RE-ARM the proximity index before returning.
		//
		// The prologue above restores ms_loadedList so update() will re-create
		// everything -- but ONLY for nodes that are still in ms_sphereTree. Two
		// separate mechanisms leave a node OUT of that index with no way back,
		// because the early return we are standing in skips the re-parse that
		// would rebuild it (only engine_wsUnloadSnapshot clears ms_sceneName):
		//
		//   1. STRIPPED. suppressObject (:1614) drops the handle when the server
		//      streams a POB the snapshot already spawned -- correct for that
		//      session, the server copy supersedes ours. A failed create
		//      (:1373-1375, e.g. CEC_objectAlreadyExists) and the event paths
		//      (:1770, :1821) strip it too.
		//   2. NEVER INDEXED. The PP_sphereTree gate (:1034-1040) skips buildout
		//      POB roots entirely when NOT single-player, because the server was
		//      going to stream them.
		//
		// Both are right while connected and both are WRONG the moment we
		// re-enter the same scene offline as an editor scene: no server will
		// stream any of it, so those buildings simply never exist. Measured
		// 2026-08-07: log in, then load an editor scene, and Mos Eisley is a
		// near-empty hole (tangible sphere tree held 1 object -- the player);
		// on a fresh process that never logged in, the same load is perfect.
		// Class 2 is what empties the CITY, class 1 is what removes individual
		// authored buildings -- a fix covering only class 1 would leave the
		// city broken, so this re-evaluates the gate rather than undoing strips.
		//
		// Re-evaluating the gate under the CURRENT mode reproduces exactly what
		// a fresh-process parse of this scene would have indexed: in an editor
		// scene Game::getSinglePlayer() is true (set by engine_gameLoadScene
		// before setScene), the first disjunct short-circuits, and every live
		// root is armed. While connected it is a near no-op -- suppressed nodes
		// that legitimately failed the gate stay suppressed.
		//
		// !isDeleted() IS LOAD-BEARING, not hygiene: removeNode tombstones a
		// node IN PLACE (WorldSnapshotReaderWriter.cpp:134-139 zeroes the handle
		// AND the network id, sets m_deleted) and leaves it in the node list, so
		// it still enumerates here. Arming those would inject id-0 phantoms into
		// the spawn set.
		//
		// Skipped entirely while a parse is in flight: no strip can have
		// happened yet (every strip path forces finishLoadNow first) and
		// PP_sphereTree does NOT test handle==0 before addObject (:1039), so
		// arming ahead of it would double-insert and leak the first entry.
		// ====================================================================
		if (!ms_parsePending)
		{
			//-- match unload()'s reset (:632) so a re-entered scene re-defers event
			//   objects from scratch instead of accumulating duplicate map entries
			ms_eventObjectMap.clear ();

			int reArmedStripped = 0;
			int reArmedBuildout = 0;

			int const numberOfNodes = ms_reader.getNumberOfNodes ();
			for (int i = 0; i < numberOfNodes; ++i)
			{
				const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
				if (!node || node->isDeleted () || node->getSpatialSubdivisionHandle ())
					continue;

				//-- the PP_sphereTree gate (:1034-1040), re-evaluated under the CURRENT mode
				bool const isBuildoutExcluded =
					isInSet (ms_buildoutObjects, node->getNetworkIdInt ())
					&& !(node->getPortalLayoutCrc () == 0 && node->getContainedByNetworkIdInt () == 0);

				if (Game::getSinglePlayer () || !isBuildoutExcluded)
				{
					node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));

					if (isBuildoutExcluded)
						++reArmedBuildout;   // class 2: gate never indexed it (connected parse)
					else
						++reArmedStripped;   // class 1: it was indexed once and stripped
				}
			}

			//-- NOT behind a probe key. This is rare (once per same-scene re-entry) and it is
			//   the discriminator for the failure both reviewers feared most: "fix lands, bug
			//   persists" when a surviving NetworkId makes every re-armed create fail and get
			//   re-stripped at :1373-1375. Silence here after an editor load means the re-arm
			//   never ran; big numbers followed by an empty world means the creates are failing.
			if (reArmedStripped > 0 || reArmedBuildout > 0)
				REPORT_LOG (true, ("[ws.load] same-scene re-arm: %d stripped + %d buildout node(s) re-indexed (singlePlayer=%d, scene=%s)\n",
					reArmedStripped, reArmedBuildout, Game::getSinglePlayer () ? 1 : 0, sceneName));
		}

		return;
	}

	ms_sceneName = sceneName;
	ms_unloadReason = "load";       // CONSULT-71 probe tag: zone change OR in-place reload's second unload
	unload ();

	//-- CONSULT-60: cheap prologue only. The node parse, per-area buildout
	//   tables, and sphere-tree build all run in budgeted loadStep() calls
	//   pumped from GroundScene's loading update -- the old synchronous body
	//   froze the main loop for seconds inside the GroundScene constructor,
	//   BEFORE the loading screen was even enabled.

#if PRODUCTION
	//always do block below [if ( true )]
#else
	if ( ConfigClientGame::getLoadBuildoutOnly() == false )
#endif
	{
		char filename[256];
		IGNORE_RETURN(snprintf(filename, sizeof(filename)-1, "snapshot/%s.ws", sceneName));
		filename[sizeof(filename)-1] = '\0';

		ms_parseIff = new Iff;
		if (ms_parseIff->open (filename, true))
		{
			if (!ms_reader.beginIncrementalLoad (*ms_parseIff))
			{
				delete ms_parseIff;
				ms_parseIff = 0;
			}
		}
		else
		{
			delete ms_parseIff;
			ms_parseIff = 0;

			//-- only warn if we don't have a buildout and are not in a space scene
			if (strncmp(sceneName, "space_", 6))
				DEBUG_WARNING (!SharedBuildoutAreaManager::isBuildoutScene(sceneName), ("WorldSnapshot::load - could not load %s", sceneName));
		}
	}

	//-- the buildout-area LIST loads synchronously -- GroundScene::init reads
	//   it (getBuildoutNameForPosition) right after postload; only the per-area
	//   object tables are deferred
	SharedBuildoutAreaManager::load(sceneName);

	ms_buildoutObjects.clear ();
	ms_buildoutTopLevelNodes.clear ();
	ms_buildoutAreaIndex = 0;
	ms_sphereNodeIndex = 0;
	ms_parsePhase = ms_parseIff ? PP_wsNodes : PP_buildout;
	ms_parsePending = true;

	//-- budget <= 0 restores the old fully-synchronous load
	if (ConfigClientGame::getWorldSnapshotParseBudgetMs () <= 0)
		finishLoadNow ();
}

//-------------------------------------------------------------------

void WorldSnapshotNamespace::loadOneBuildoutArea (const BuildoutArea& buildoutArea)
{
	const int cs_sharedCellObjectTemplate_tag = 0x0c5401ee;
	const char* const sceneName = ms_sceneName.c_str ();
	std::set< int64 > &buildoutObjects = ms_buildoutObjects;

	{
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
						WorldSnapshotReaderWriter::Node const * const buildoutNode = ms_reader.addObject(
							objId,
							containerId,
							ObjectTemplateList::lookUp(sharedTemplateCrc),
							cellIndex,
							xform,
							radius,
							portalLayoutCrc,
							requiredEvent);

						//-- Goal B: identity-keyed provenance for the editor
						//   shims + save filter (positive v2 objids make the id
						//   set ambiguous vs authored ids)
						if (buildoutNode && containerId == 0)
							IGNORE_RETURN(ms_buildoutTopLevelNodes.insert(buildoutNode));
					}

					buildoutObjects.insert( objId );

				}
			}
		}
	}

}

//-------------------------------------------------------------------

void WorldSnapshotNamespace::finishLoadNow ()
{
	//-- CONSULT-60 exactness valve: run the remaining parse to completion
	//   synchronously. Callers use this when they need complete snapshot data
	//   mid-parse; worst case degrades to the old synchronous-load cost.
	while (ms_parsePending)
		WorldSnapshot::loadStep ();
}

//-------------------------------------------------------------------

void WorldSnapshot::loadStep ()
{
	if (!ms_parsePending)
		return;

	float const budgetMs = static_cast<float> (ConfigClientGame::getWorldSnapshotParseBudgetMs ());

	PerformanceTimer timer;
	timer.start ();

	for (;;)
	{
		switch (ms_parsePhase)
		{
		case PP_wsNodes:
			{
				NOT_NULL (ms_parseIff);

				float stepBudgetMs = 0.f;
				if (budgetMs > 0.f)
				{
					stepBudgetMs = budgetMs - timer.getSplitTime () * 1000.f;
					if (stepBudgetMs <= 0.f)
						return;
				}

				if (ms_reader.stepIncrementalLoad (*ms_parseIff, stepBudgetMs))
				{
					//-- the Iff holds the whole .ws in RAM -- release it the
					//   moment the parse is done (the later phases don't need it)
					delete ms_parseIff;
					ms_parseIff = 0;
					ms_parsePhase = PP_buildout;
				}
			}
			break;

		case PP_buildout:
			{
				const std::vector<BuildoutArea> &buildoutAreas = SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene();
				if (ms_buildoutAreaIndex < static_cast<int> (buildoutAreas.size ()))
				{
					loadOneBuildoutArea (buildoutAreas [static_cast<size_t> (ms_buildoutAreaIndex)]);
					++ms_buildoutAreaIndex;
				}
				else
					ms_parsePhase = PP_sphereTree;
			}
			break;

		case PP_sphereTree:
			{
				//-- add all objects to the sphere tree (chunked by node index)
				int const numberOfNodes = ms_reader.getNumberOfNodes ();
				int const batchEnd = std::min (ms_sphereNodeIndex + 4096, numberOfNodes);
				for (; ms_sphereNodeIndex < batchEnd; ++ms_sphereNodeIndex)
				{
					const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (ms_sphereNodeIndex);

					//
					// if the object is not a buildout object
					// or the object is a top level object
					// then add it to the sphere tree
					//
					const bool isSinglePlayer = Game::getSinglePlayer();
					if ( isSinglePlayer
						|| isInSet( ms_buildoutObjects, node->getNetworkIdInt() ) == false
						|| ( node->getPortalLayoutCrc() == 0 && node->getContainedByNetworkIdInt() == 0 ) )
					{
						node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));
					}
				}

				if (ms_sphereNodeIndex >= numberOfNodes)
				{
					if (ms_reader.getNumberOfNodes ())
						DEBUG_REPORT_LOG (true, ("WorldSnapshot [%s]: %i object templates, %i root objects, %i total objects\n", ms_sceneName.c_str (), ms_reader.getNumberOfObjectTemplateNames (), ms_reader.getNumberOfNodes (), ms_reader.getTotalNumberOfNodes ()));

					ms_parsePhase = PP_done;
				}
			}
			break;

		case PP_done:
			{
				ms_buildoutObjects.clear ();

				//-- setup to preload all the object templates
				ms_numberOfObjectTemplates = ms_reader.getNumberOfObjectTemplateNames ();
				ms_preloadObjectTemplate = 0;
				ms_parsePending = false;
				preloadSomeAssets ();

				//-- Wave-3 gate aid (default off): one REAL save at parse
				//   completion, typed result logged (engine_wsSaveSnapshot is
				//   declared at file scope above -- C2598 forbids a linkage
				//   specification inside a function)
				if (ms_wsSelfTestSaveOnLoad)
				{
					const int selfTestResult = engine_wsSaveSnapshot ();
					REPORT_LOG (true, ("[editor.ws] SELF-TEST save-on-load: result=%d\n", selfTestResult));
				}
			}
			return;
		}

		if (budgetMs > 0.f && timer.getSplitTime () * 1000.f >= budgetMs)
			return;
	}
}

//-------------------------------------------------------------------

void WorldSnapshot::setExcludeArea(char const *areaName)
{
	ms_excludeArea = areaName;
}

//-------------------------------------------------------------------

void WorldSnapshot::preloadSomeAssets ()
{
	//-- CONSULT-60: template names are not complete until the parse finishes
	if (ms_parsePending)
		return;

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
	//-- CONSULT-60: mid-parse the preload counters are 0 and would read as 100
	if (ms_parsePending)
		return 0;

	if (! ConfigClientGame::getPreloadWorldSnapshot () || !ms_numberOfObjectTemplates)
		return 100;

	return (ms_preloadObjectTemplate * 100) / ms_numberOfObjectTemplates;
}

//-------------------------------------------------------------------

bool WorldSnapshot::donePreloading ()
{
	//-- CONSULT-60: hold the loading screen until the phased parse completes
	//   (mid-parse the counters are 0/0 and would otherwise read as done)
	if (ms_parsePending)
		return false;

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

	//-- CONSULT-60: no snapshot object creation until the phased parse is done
	//   (the sphere tree is only populated in the final parse phase)
	if (ms_parsePending)
		return;

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

		//-- the incremental merge walk above replaced the two-binary_search form
		//   kept in the #else below, and DROPPED its computeDistanceSquaredTo
		//   calls. Nothing else ever writes m_distanceSquaredTo (initialised to
		//   0.f, WorldSnapshotReaderWriter.cpp:110), so every node reported
		//   distance 0 forever: the delete guard further down read
		//   0.f < sqr(radius) + 128.f -- ALWAYS true -- so the drain deleted
		//   NOTHING and snapshot objects were never streamed out (memory grew
		//   monotonically with everywhere the player had been), and both sort
		//   keys were constant, making the "nearest first" ordering a no-op.
		//   Compute exactly the set the #else computes: the nodes that go into
		//   the two lists.
		{
			size_t i;
			for (i = 0; i < ms_pendingCreateList.size (); ++i)
				ms_pendingCreateList [i]->computeDistanceSquaredTo (position_w);

			for (i = 0; i < ms_pendingDeleteList.size (); ++i)
				ms_pendingDeleteList [i]->computeDistanceSquaredTo (position_w);
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

		//-- wall-time budget for the create/delete drain below (shared timer:
		//   creates spend first, deletes get whatever remains but always make
		//   >=1 item of progress). Checked at the TOP of each iteration so the
		//   item that exceeds the budget still completes -- never a torn create.
		PerformanceTimer drainTimer;
		if (ms_createTimeBudgetMs > 0)
			drainTimer.start ();

		//-- create all pending creates
		{
			std::sort (ms_pendingCreateList.begin (), ms_pendingCreateList.end (), compareNodesForCreate);

			size_t const n = std::min(ms_pendingCreateList.size(), static_cast<size_t>(ms_maximumNumberOfCreatesPerFrame));
			for (size_t i = 0; i < n; ++i)
			{
				if (i > 0 && ms_createTimeBudgetMs > 0 && drainTimer.getSplitTime () * 1000.f >= static_cast<float> (ms_createTimeBudgetMs))
					break;

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

					//-- 2026-08-07 (CONSULT-73): this strip is PERMANENT for the process -- the
					//   node leaves the proximity index and cannot be created again until a
					//   re-parse or the same-scene re-arm in load(). Every diagnostic below it
					//   is a DEBUG_WARNING, i.e. nothing at all in the build anyone runs, so a
					//   node could vanish from the world for a stated reason that reached no
					//   human. This is also the predicted failure mode of the re-arm itself:
					//   if a NetworkId survives the outgoing-scene teardown, every re-armed
					//   node fails CEC_objectAlreadyExists here and is stripped straight back
					//   out -- an empty world that looks identical to the bug being unfixed.
					//   Bounded: a node can only fail once per arming (the strip stops retries).
					WS_EDITOR_LOG (("[editor.ws] createObject FAILED: id=%I64d [%s] reason=%s -- node dropped from sphere tree (permanent until re-parse/re-arm)\n",
						node->getNetworkIdInt (),
						ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ()),
						wsCreateErrorCodeName (result)));

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
		if (ms_streamOutSnapshotObjects)
		{
			std::sort (ms_pendingDeleteList.begin (), ms_pendingDeleteList.end (), compareNodesForDelete);

			//-- standing probe for the restored drain. This path had never
			//   executed, so it has no field history at all -- report what it
			//   actually does, in the build everyone runs. Reading rule: deleted
			//   should track how far the player has moved; refused is
			//   server-superseded POBs and is expected to be small and steady.
			//   A createObject failure with CEC_objectAlreadyExists appearing
			//   AFTER a line here is the "stream-out broke re-entry" signature.
			int deleted = 0;
			int refused = 0;

			size_t const n = std::min(ms_pendingDeleteList.size(), static_cast<size_t>(ms_maximumNumberOfDeletesPerFrame));
			for (size_t i = 0; i < n; ++i)
			{
				if (i > 0 && ms_createTimeBudgetMs > 0 && drainTimer.getSplitTime () * 1000.f >= static_cast<float> (ms_createTimeBudgetMs))
					break;

				const WorldSnapshotReaderWriter::Node* const node = ms_pendingDeleteList [i];

				//-- Enlarge the radius at which an object will be deleted by 128 meters of it
				if (node->getDistanceSquaredTo() < sqr(node->getRadius()) + 128.f)
					continue;

				//-- find the object
				Object* const object = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (node->getNetworkIdInt ())));

				//-- resolve and REFUSE before any teardown. Until the distance key
				//   was restored (see the note above) this loop always hit the guard,
				//   so its ordering had never executed: it used to call
				//   removeFromWorld() and drop the node from ms_loadedList even when
				//   the delete was then refused, leaving the object ALIVE still
				//   holding its NetworkId. The next approach re-creates, hits
				//   CEC_objectAlreadyExists, and strips the node from the sphere tree
				//   PERMANENTLY. A refused node stays loaded and in the world, and is
				//   simply retried on a later update. asClientObject() rather than
				//   safe_cast because safe_cast is a bare static_cast in Release.
				ClientObject* const clientObject = object ? object->asClientObject () : 0;
				if (object && (!clientObject || !ContainerInterface::isClientCachedOnly (*clientObject)))
				{
					++refused;
					continue;
				}

				node->removeFromWorld ();

				//-- destroy the object
				if (object)
				{
					if (object->isInWorld ())
						object->removeFromWorld ();
					else
						DEBUG_WARNING (true, ("WorldSnapshot::update - deleting client cached object %i which is not in the world\n", node->getNetworkIdInt ()));

					delete object;
				}
				else
					DEBUG_WARNING (true, ("WorldSnapshot::update - attempted to delete client cached object %i which does not exist\n", node->getNetworkIdInt ()));

				//-- the object is now deleted
				NodeList::iterator iter = std::find (ms_loadedList.begin (), ms_loadedList.end (), node);
				if (iter != ms_loadedList.end ())
					IGNORE_RETURN (ms_loadedList.erase (iter));

				++deleted;
			}

			//-- only on an actual state change: a refused node is retried every
			//   update (server-superseded POBs never become deletable), so logging
			//   those too would be steady per-frame noise.
			if (deleted)
				REPORT_LOG (true, ("WorldSnapshot: stream-out deleted=%d refused=%d pending=%d loaded=%d\n", deleted, refused, static_cast<int> (ms_pendingDeleteList.size ()), static_cast<int> (ms_loadedList.size ())));
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
	//-- CONSULT-60 exactness valve: a miss against the partial map could be a
	//   not-yet-parsed node -- finish the parse before answering negatively
	if (ms_parsePending && !ms_reader.find (networkIdInt))
		finishLoadNow ();

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
	//-- CONSULT-60: editor/external mutators need the complete snapshot
	if (ms_parsePending)
		finishLoadNow ();

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
	//-- CONSULT-60: the target node may not be parsed yet
	if (ms_parsePending)
		finishLoadNow ();

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
	//-- CONSULT-60: a mid-parse miss here would let the node parse in later
	//   WITHOUT the delete (duplicate object) -- finish the parse first
	if (ms_parsePending)
		finishLoadNow ();

	const WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (node && !node->isDeleted () && node->getSpatialSubdivisionHandle ())
	{
		ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
		node->setSpatialSubdivisionHandle (0);
	}

	ms_reader.removeNode (networkIdInt);
}

//-------------------------------------------------------------------
// Prevent a snapshot node from ever spawning again this session WITHOUT
// destroying its authored data. The spawn set is the sphere tree, so
// dropping the handle is the whole re-create prevention; removeObject's
// additional removeNode tombstones+erases the AUTHORED row, which (a)
// makes wsSetNodeTemplateName/find miss a building the server replaced,
// (b) silently drops that authored row from every later wsSaveSnapshot
// (tombstone-skip), and (c) makes the id allocator's map-miss free-test
// see a still-authored id as free. Used by the SceneCreateObject
// client-cached-replacement path (GroundScene), where the object is
// merely superseded by the server-streamed copy -- the .ws data is not
// being edited and must survive.

void WorldSnapshot::suppressObject (const int64 networkIdInt)
{
	//-- same discipline as removeObject: a mid-parse miss would let the
	//   node parse in later WITH its sphere handle (duplicate object)
	if (ms_parsePending)
		finishLoadNow ();

	const WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (node && node->getSpatialSubdivisionHandle ())
	{
		ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
		node->setSpatialSubdivisionHandle (0);

		//-- 2026-08-07: this used to strip in TOTAL SILENCE, in every build. It is a
		//   permanent, process-lifetime edit to the proximity index -- the node can
		//   never be created again until a re-parse or the CONSULT-73 re-arm -- and
		//   that invisibility cost a full day of investigation: the effect (buildings
		//   absent from an editor scene entered after a login) was measurable while
		//   the cause left no trace anywhere, in any log, in a shipping build.
		//   REPORT_LOG, not DEBUG_*: Release is the only build this is ever seen in.
		WS_EDITOR_LOG (("[editor.ws] suppressObject: id=%I64d handle dropped (server copy supersedes; node kept)\n", networkIdInt));
	}
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
	//-- CONSULT-60: sphere-tree rebuild needs the complete node set
	if (ms_parsePending)
		finishLoadNow ();

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
			//-- was ms_reader.getNode(i): a READER index walked over the saveList
			//   RANGE -- two different index spaces. It re-added the reader's first
			//   saveList.size() nodes (children included) instead of the nodes that
			//   were actually removed above. Latent because only the dev-console
			//   detail-level command reaches this function.
			const WorldSnapshotReaderWriter::Node* const node = saveList [i];
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
		//-- CONSULT-60 exactness valve: baselines/containment can arrive while
		//   the phased parse is still running (player logging in inside a
		//   client-cached POB). A miss against the partial map must not become
		//   a silent no-op -- finish the parse first, then answer exactly.
		//   (The int-max guard above already excludes most live server ids.)
		if (ms_parsePending && !ms_reader.find (networkId.getValue ()))
			finishLoadNow ();

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
	//-- CONSULT-60: the sphere tree is incomplete mid-parse
	if (ms_parsePending)
		finishLoadNow ();

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
// Goal B Wave 1 (hookpoints v16 -> v17; rev-3 freeze 2026-07-15): the Utinni
// snapshot-editor READ shims. extern "C" __cdecl, advertised by
// engine_advertise.cpp (constant &fn rows; declared in
// engine_worldSnapshot_forward.h). They live HERE because ms_reader and its
// bookkeeping are file-scope in WorldSnapshotNamespace -- the
// sysmsg/lookAtTarget shim pattern at TU scale.
//
// FROZEN contracts (the rev-3 Wave-1 row table -- do not change semantics
// without a version wave):
//  - enumeration is live, AUTHORED-ONLY: tombstones (removeNode's setDeleted)
//    and buildout-provenance rows (the retained ms_buildoutObjects set) are
//    never enumerated, and id-keyed reads answer MISS for them.
//  - node reads force-finish the CONSULT-60 incremental parse (the same
//    finishLoadNow discipline as the mutators above).
//  - wsGetGeneration is a PURE counter read: no parse force (pollable during
//    a loading screen without re-synchronizing the phased load).
//  - game-thread-only (consumer marshals); graceful degradation (a missing
//    row leaves the affordance dark, never a crash).
//
// Both platforms (x64 port 2026-08-15; was 32-bit only). Every shim below is
// extern "C" with a primitives/pointers-only boundary and __int64 ids, so the
// port is guard removal: the frozen-ABI static_asserts just underneath prove
// EngineWsNodeInfo keeps the identical 80-byte layout under either pointer
// size (max member alignment is 8 on both).
//===================================================================

#include <cstddef>  // offsetof (frozen-ABI asserts)
#include <cstring>  // memcpy/memset (POD-out fill)
#include "../../../../../../../game/client/application/SwgClient/src/shared/engine_hookpoints.h" // EngineWsNodeInfo (the shared contract POD; header pulls in no engine headers by design)

//-- pin the rev-3 frozen ABI: any drift here is a contract break, not a build tweak
static_assert (sizeof (EngineWsNodeInfo) == 80,                          "EngineWsNodeInfo: rev-3 froze sizeof == 80");
static_assert (offsetof (EngineWsNodeInfo, size)            ==  0,      "EngineWsNodeInfo: frozen layout drift (size)");
static_assert (offsetof (EngineWsNodeInfo, flags)           ==  4,      "EngineWsNodeInfo: frozen layout drift (flags)");
static_assert (offsetof (EngineWsNodeInfo, containedById)   ==  8,      "EngineWsNodeInfo: frozen layout drift (containedById)");
static_assert (offsetof (EngineWsNodeInfo, cellIndex)       == 16,      "EngineWsNodeInfo: frozen layout drift (cellIndex)");
static_assert (offsetof (EngineWsNodeInfo, portalLayoutCrc) == 20,      "EngineWsNodeInfo: frozen layout drift (portalLayoutCrc)");
static_assert (offsetof (EngineWsNodeInfo, radius)          == 24,      "EngineWsNodeInfo: frozen layout drift (radius)");
static_assert (offsetof (EngineWsNodeInfo, transform)       == 28,      "EngineWsNodeInfo: frozen layout drift (transform)");
static_assert (offsetof (EngineWsNodeInfo, childCount)      == 76,      "EngineWsNodeInfo: frozen layout drift (childCount)");

namespace WorldSnapshotNamespace
{
	//-- IDENTITY-keyed buildout test (2026-07-18 self-test finding): walk to the
	//   subtree root and test membership in the buildout top-level node set.
	//   Ids can no longer discriminate -- SWGSource v2 buildout tables carry
	//   positive objids that can collide with authored ids; on a collision the
	//   reader map keeps the AUTHORED node (parse inserts first), so an
	//   identity test stays exact where an id test would drop authored content.
	bool wsIsBuildoutNode (const WorldSnapshotReaderWriter::Node* node)
	{
		while (node->getParent ())
			node = node->getParent ();

		return ms_buildoutTopLevelNodes.find (node) != ms_buildoutTopLevelNodes.end ();
	}

	//-- the authored-only enumeration filter: live (non-tombstone) and NOT a
	//   buildout-provenance node (identity-keyed).
	inline bool wsIsEnumerable (const WorldSnapshotReaderWriter::Node* const node)
	{
		return node
			&& !node->isDeleted ()
			&& !wsIsBuildoutNode (node);
	}

	//-- id-keyed lookup under the frozen miss contract: parse force-finished
	//   (mutator discipline), tombstones missed by find() itself (erased from
	//   the map + id zeroed), buildout nodes missed by identity provenance.
	const WorldSnapshotReaderWriter::Node* wsFindAuthoredLive (const int64 networkIdInt)
	{
		if (ms_parsePending)
			finishLoadNow ();

		const WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
		if (!node || wsIsBuildoutNode (node))
			return 0;

		return node;
	}

	int wsEnumerableChildCount (const WorldSnapshotReaderWriter::Node* const node)
	{
		int count = 0;
		for (int i = 0; i < node->getNumberOfNodes (); ++i)
			if (wsIsEnumerable (node->getNode (i)))
				++count;

		return count;
	}
}

//-------------------------------------------------------------------

// v28: PURE, NON-FORCING parse-completion read. Deliberately the ONLY ws* row
// with no finishLoadNow() prologue (wsGetGeneration is the other pure read, but
// it is a load/unload counter and says nothing about the parse).
//
// Exists so a consumer can WAIT instead of FORCE. Without it the only way to
// observe completion was to call a forcing row purely for its side effect --
// which pays the whole remaining synchronous parse (~3.1s worst case) to avoid
// a race, exactly the freeze CONSULT-60 removed. getLoadingPercent() is NOT a
// substitute: it returns 0 while ms_parsePending and then reports template-
// preload percent (:983), so a reader cannot tell "still parsing" from
// "parsed, preload at 0%".
//
// 1 = a phased parse is in flight (world still rebuilding), 0 = idle/complete.
// Poll it from a per-frame detour; safe at any time, no side effects.
extern "C" int __cdecl engine_wsIsParsePending (void)
{
	return ms_parsePending ? 1 : 0;
}

//-------------------------------------------------------------------

// v32: FORGET a node -- drop it from the snapshot WITHOUT despawning the live Object.
//
// The consumer's placement gesture mints a temporary preview node via wsAddObject so the
// modder can see and gizmo the thing, then on Persist writes the real row into the .ilf.
// The preview must leave the .ws (a copy there would be a second, world-space instance of
// the same decoration -- they measured exactly that: two 84-byte runtime children of
// building 1082874 carrying world coords where the .ilf rows are cell-relative, and the
// engine CANNOT dedupe them because .ilf-created objects never get a NetworkId, so
// createObject's CEC_objectAlreadyExists guard can never fire).
//
// But wsRemoveNode is a TEARDOWN primitive -- its subtree sweep removeFromWorld()s and
// deletes -- so using it made the object the modder just placed VANISH at the moment they
// saved it. This is the missing half: the DATA leaves the snapshot, the OBJECT stays put
// for the rest of the session, and the reload path picks it up from the .ilf where it now
// lives.
//
// WorldSnapshot::removeObject already IS this operation (drop the sphere handle so nothing
// re-spawns, then removeNode -> setDeleted + map erase, so every later saveFiltered
// tombstone-skips the row). Nothing touches the Object. This shim only adds the typed
// found/not-found result the raw void static cannot give.
//
// NO occupancy guard, deliberately: wsRemoveNode needs one because a Container dtor
// cascade-deletes cell contents, and nothing is deleted here.
//
// ALLOCATOR: forgetting does NOT free the id for re-mint. wsAllocateIdRange's collision
// test consults NetworkIdManager (:~2198), not just ms_reader, and the forgotten node's
// Object is still alive and still registered -- so the id stays taken. The consumer asked;
// this is the answer, and it holds regardless of whether they ever re-add at an explicit id.
//
// 1 = a live node was found and forgotten · 0 = the id did not resolve (or is a tombstone).
extern "C" int __cdecl engine_wsForgetNode (__int64 networkIdInt)
{
	//-- CONSULT-60: a mid-parse miss would let the node parse in later WITH its sphere
	//   handle, i.e. the forget would silently not stick.
	if (ms_parsePending)
		finishLoadNow ();

	const WorldSnapshotReaderWriter::Node * const node = ms_reader.find (networkIdInt);
	if (!node || node->isDeleted ())
	{
		REPORT_LOG (true, ("[editor.ws] wsForgetNode MISS id=%I64d (no live node)\n", networkIdInt));
		return 0;
	}

	WorldSnapshot::removeObject (networkIdInt);

	//-- 2026-08-07 (queued item 6.2): forgetting a node deliberately does NOT un-intern
	//   its template name, so a placement whose template was NOVEL to this snapshot
	//   leaves the .ws larger by exactly strlen(templatePath)+1 (measured:
	//   shared_endor_roba.iff, 44 chars -> 1,400,272 -> 1,400,317) with no node written.
	//   DECIDED, not overlooked:
	//     - Nodes reference the OTNL BY INDEX (m_objectTemplateNameIndex), and the table
	//       is a flat vector<char*> (WorldSnapshotReaderWriter.h:206). Removing an entry
	//       shifts every later index, so un-interning means reindexing every node in the
	//       snapshot and rebuilding the crc map -- the exact index-space hazard that the
	//       detailLevelChanged blind walk in this same file turned out to be.
	//     - The intern is shared and nothing refcounts it, so a safe removal needs a full
	//       scan to prove no other node uses the name.
	//     - The cost is bounded and one-shot: only a template novel to this snapshot pays,
	//       and re-placing it later is free (the crc map hits).
	//   The reclaim belongs at WRITE time instead -- the OTNL garbage-collect in
	//   saveFiltered already recorded as optional polish in the 2026-07-31 .ws size-drift
	//   close-out. That touches no live index and also subsumes the 325 buildout names.
	//   It changes serialized bytes, so it is a COORDINATED change: the toolkit holds
	//   recorded byte baselines and a byte-identical invariant.
	REPORT_LOG (true, ("[editor.ws] wsForgetNode OK id=%I64d (row dropped; live Object untouched; template name stays interned -- see comment)\n", networkIdInt));
	return 1;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetNodeCount (void)
{
	if (ms_parsePending)
		finishLoadNow ();

	int count = 0;
	for (int i = 0; i < ms_reader.getNumberOfNodes (); ++i)
		if (wsIsEnumerable (ms_reader.getNode (i)))
			++count;

	return count;
}

//-------------------------------------------------------------------

extern "C" __int64 __cdecl engine_wsGetTopNodeIdAt (int index)
{
	if (ms_parsePending)
		finishLoadNow ();

	if (index < 0)
		return 0;

	for (int i = 0; i < ms_reader.getNumberOfNodes (); ++i)
	{
		const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
		if (!wsIsEnumerable (node))
			continue;

		if (index-- == 0)
			return node->getNetworkIdInt ();
	}

	return 0;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetChildCount (__int64 networkIdInt)
{
	const WorldSnapshotReaderWriter::Node* const node = wsFindAuthoredLive (networkIdInt);
	if (!node)
		return 0;

	return wsEnumerableChildCount (node);
}

//-------------------------------------------------------------------

extern "C" __int64 __cdecl engine_wsGetChildIdAt (__int64 networkIdInt, int index)
{
	const WorldSnapshotReaderWriter::Node* const node = wsFindAuthoredLive (networkIdInt);
	if (!node || index < 0)
		return 0;

	for (int i = 0; i < node->getNumberOfNodes (); ++i)
	{
		const WorldSnapshotReaderWriter::Node* const child = node->getNode (i);
		if (!wsIsEnumerable (child))
			continue;

		if (index-- == 0)
			return child->getNetworkIdInt ();
	}

	return 0;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetNodeInfo (__int64 networkIdInt, EngineWsNodeInfo* out)
{
	//-- size-first protocol: the caller declares its compiled-against size FIRST
	if (!out || out->size < sizeof (unsigned int))
		return 0;

	const WorldSnapshotReaderWriter::Node* const node = wsFindAuthoredLive (networkIdInt);
	if (!node)
		return 0;

	EngineWsNodeInfo info;
	memset (&info, 0, sizeof (info));
	info.size            = sizeof (EngineWsNodeInfo);
	info.flags           = 0;   // bit0 deleted: never set here (tombstones answer miss); bit1 buildout: RESERVED, 0 in v1
	info.containedById   = node->getContainedByNetworkIdInt ();
	info.cellIndex       = node->getCellIndex ();
	info.portalLayoutCrc = node->getPortalLayoutCrc ();
	info.radius          = node->getRadius ();

	//-- row-major 3x4, position = column 3 (frozen rev-3 layout). Composed from
	//   the Transform column accessors -- layout-defined, independent of
	//   Transform's internal representation.
	{
		const Transform& t = node->getTransform_p ();
		const Vector i = t.getLocalFrameI_p ();
		const Vector j = t.getLocalFrameJ_p ();
		const Vector k = t.getLocalFrameK_p ();
		const Vector p = t.getPosition_p ();
		info.transform [ 0] = i.x;  info.transform [ 1] = j.x;  info.transform [ 2] = k.x;  info.transform [ 3] = p.x;
		info.transform [ 4] = i.y;  info.transform [ 5] = j.y;  info.transform [ 6] = k.y;  info.transform [ 7] = p.y;
		info.transform [ 8] = i.z;  info.transform [ 9] = j.z;  info.transform [10] = k.z;  info.transform [11] = p.z;
	}

	info.childCount = wsEnumerableChildCount (node);

	//-- write min(callerSize, providerSize); never touch caller space beyond
	//   what this provider understands
	const unsigned int copyBytes = out->size < sizeof (info) ? out->size : static_cast<unsigned int> (sizeof (info));
	memcpy (out, &info, copyBytes);

	return 1;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetNodeTemplateName (__int64 networkIdInt, char* buf, int cap)
{
	const WorldSnapshotReaderWriter::Node* const node = wsFindAuthoredLive (networkIdInt);
	if (!node)
		return 0;

	const char* const name = ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ());
	if (!name)
		return 0;

	//-- copy-out: min(cap, needed) bytes, NUL-terminated when it fits; returns
	//   the needed length INCLUDING the NUL (buf==0/cap<=0 is a pure size query)
	const int needed = static_cast<int> (strlen (name)) + 1;
	if (buf && cap > 0)
		memcpy (buf, name, static_cast<size_t> (cap < needed ? cap : needed));

	return needed;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetGeneration (void)
{
	//-- PURE counter read by contract: no finishLoadNow (pollable during a
	//   loading screen without forcing the phased parse synchronous)
	return ms_wsEditGeneration;
}

//===================================================================
// Goal B Wave 2 (hookpoints v17 -> v18; frozen 2026-07-18): LIVE-ONLY
// mutation shims. Explicitly non-persistent -- nothing here touches disk;
// persistence is Wave 3. Semantics per the accepted ANSWERS §5.2/5.3/5.5 +
// the §4 Wave-2 deltas (occupancy-guarded remove, redesigned wsAddNodeAt
// spawn, allocator NetworkIdManager check). Same TU / same guard / same
// game-thread-only + graceful-degradation contracts as Wave 1 above.
//===================================================================

namespace WorldSnapshotNamespace
{
	//-- Wave-2 id-allocator band (engine_wsConfigureIdAllocator). floor 0 =
	//   derive the seed (max positive authored id + 1); the ceiling default is
	//   the consumer's server-id convention (no engine constant exists -- the
	//   ANSWERS 5.1c finding), always <= INT32_MAX (the on-disk id width).
	int64 ms_wsIdFloor   = 0;
	int64 ms_wsIdCeiling = 0x1000000;

	//-- interactive-add default update radius: the god-client fallback value
	//   (BuildoutAreaSupport). The consumer tunes per-node via wsSetNodeRadius.
	const float cs_wsDefaultAddRadius = 512.f;

	//-- row-major 3x4, position = column 3 (the frozen wsGetNodeInfo layout,
	//   inverted): floats -> engine Transform via the column setters.
	void wsTransformFromFloats (const float* const m, Transform& out)
	{
		out.setLocalFrameIJK_p (Vector (m[0], m[4], m[8]), Vector (m[1], m[5], m[9]), Vector (m[2], m[6], m[10]));
		out.setPosition_p (Vector (m[3], m[7], m[11]));
	}

	//-- subtree walk, ROOT FIRST (the remove teardown deletes the root object
	//   before looking at descendants -- the container cascade has already
	//   despawned them by then)
	void wsCollectSubtree (const WorldSnapshotReaderWriter::Node* const root, NodeList& nodes, std::vector<int64>& ids)
	{
		nodes.push_back (root);
		ids.push_back (root->getNetworkIdInt ());

		for (int i = 0; i < root->getNumberOfNodes (); ++i)
			wsCollectSubtree (root->getNode (i), nodes, ids);
	}

	//-- contiguous free-range first-fit (ANSWERS 5.2): seed = max positive
	//   IN-BAND authored id + 1 raised by the consumer floor; every id in
	//   id..id+count verified free against the reader map, the buildout-
	//   provenance set, AND NetworkIdManager (a live server-streamed id in the
	//   band would refuse the spawn with CEC_objectAlreadyExists and leave a
	//   half-added node); fail-closed 0 when the band is exhausted.
	//
	//   2026-07-18 hardening + discriminator (id-mint refusal, consumer paired
	//   logs): (1) ids at/above the ceiling never contribute to the seed -- an
	//   authored id past the band would otherwise drag the seed out of it and
	//   refuse every add before one iteration; (2) the walk and the loop log
	//   their state on ANY refusal, and the first collisions log which
	//   predicate fired -- one click names the mechanism.
	int64 wsAllocateIdRange (const int cellCount)
	{
		int64 seed = 1;
		int64 maxOutOfBand = 0;
		int   walkedNodes  = 0;
		int   inSetSkips   = 0;
		{
			NodeList stack;
			for (int i = 0; i < ms_reader.getNumberOfNodes (); ++i)
				stack.push_back (ms_reader.getNode (i));

			while (!stack.empty ())
			{
				const WorldSnapshotReaderWriter::Node* const node = stack.back ();
				stack.pop_back ();
				++walkedNodes;

				const int64 id = node->getNetworkIdInt ();
				if (ms_buildoutObjects.find (id) != ms_buildoutObjects.end ())
					++inSetSkips;
				else if (id >= ms_wsIdCeiling)
				{
					if (id > maxOutOfBand)
						maxOutOfBand = id;
				}
				else if (id >= seed)
					seed = id + 1;

				for (int i = 0; i < node->getNumberOfNodes (); ++i)
					stack.push_back (node->getNode (i));
			}
		}

		if (ms_wsIdFloor > seed)
			seed = ms_wsIdFloor;

		if (maxOutOfBand)
			REPORT_LOG (true, ("[editor.ws] wsAllocateIdRange: authored ids at/above the ceiling exist (max=%I64d >= %I64d) -- excluded from seeding\n", maxOutOfBand, ms_wsIdCeiling));

		//-- discriminator: remember the first few collisions + which predicate
		const int cs_maxLoggedCollisions = 3;
		int64 loggedCollisionId [cs_maxLoggedCollisions];
		char  loggedCollisionWhy [cs_maxLoggedCollisions];
		int   loggedCollisions = 0;
		int64 totalCollisions = 0;

		for (int64 id = seed; id + cellCount < ms_wsIdCeiling; )
		{
			int64 collided = 0;
			char  why = 0;
			for (int64 k = id; k <= id + cellCount; ++k)
			{
				if (ms_reader.find (k))
					why = 'r';
				else if (ms_buildoutObjects.find (k) != ms_buildoutObjects.end ())
					why = 'b';
				else if (NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (k))) != 0)
					why = 'n';

				if (why)
				{
					collided = k;
					break;
				}
			}

			if (!collided)
				return id;

			++totalCollisions;
			if (loggedCollisions < cs_maxLoggedCollisions)
			{
				loggedCollisionId [loggedCollisions] = collided;
				loggedCollisionWhy [loggedCollisions] = why;
				++loggedCollisions;
			}

			id = collided + 1;
		}

		//-- refusal: dump the full discriminator state. seed >= ceiling with
		//   zero collisions = the seed itself is out of band (walk problem);
		//   millions of collisions = a predicate is false-positive at scale.
		REPORT_LOG (true, ("[editor.ws] wsAllocateIdRange REFUSED: seed=%I64d cells=%d band=[%I64d..%I64d) walked=%d inSetSkips=%d maxOutOfBand=%I64d collisions=%I64d",
			seed, cellCount, ms_wsIdFloor, ms_wsIdCeiling, walkedNodes, inSetSkips, maxOutOfBand, totalCollisions));
		for (int i = 0; i < loggedCollisions; ++i)
			REPORT_LOG (true, (" first[%d]=%I64d(%c)", i, loggedCollisionId [i], loggedCollisionWhy [i]));
		REPORT_LOG (true, ("\n"));

		return 0;
	}
}

//-------------------------------------------------------------------

//-- (wsCreateErrorCodeName + its name table moved to the WorldSnapshotNamespace block at the
//   top of this file on 2026-08-07 -- it is now used by WorldSnapshot::update, which builds on
//   x64 too, and this location is inside the Win32-only advertise guard.)

extern "C" __int64 __cdecl engine_wsAddObject (const char* sharedTemplateFilename, const float* transform12, __int64 containedById)
{
	if (!sharedTemplateFilename || !*sharedTemplateFilename || !transform12)
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (args): template=%s transform12=%s\n", sharedTemplateFilename ? sharedTemplateFilename : "(null)", transform12 ? "ok" : "(null)"));
		return 0;
	}

	if (ms_parsePending)
		finishLoadNow ();

	Transform transform_p;
	wsTransformFromFloats (transform12, transform_p);

	//-- PRE-VALIDATE EVERYTHING before minting or mutating (frozen contract):
	//   a failed add never leaves a half-added node.

	//-- container: must be an authored live node with a spawned, in-world live
	//   object ("spawns immediately" is only honorable against a live parent)
	int cellIndex = 0;
	if (containedById != 0)
	{
		const WorldSnapshotReaderWriter::Node* const containerNode = wsFindAuthoredLive (containedById);
		if (!containerNode)
		{
			WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (container-not-found): id=%I64d (no authored live node)\n", containedById));
			return 0;
		}

		Object* const containerObject = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (containedById)));
		if (!containerObject || !containerObject->isInWorld ())
		{
			WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (container-not-live): id=%I64d object=%s\n", containedById, containerObject ? "exists-but-not-in-world" : "none"));
			return 0;
		}

		//-- buildout v1 convention: a contained row carries its containing
		//   cell's index (unused by non-cell creates; serialized by Wave 3)
		cellIndex = containerNode->getCellIndex ();
	}
	else
	{
		//-- mirror createObject's corrupt-data guards (an add they would refuse
		//   becomes a permanently unspawnable zombie node -- fail here instead)
		const Vector position = transform_p.getPosition_p ();
		if (position == Vector::zero || position.magnitudeSquared () < sqr (ms_closeToOriginDistance))
		{
			WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (origin): pos=(%g %g %g) magSq=%g < %g\n", position.x, position.y, position.z, position.magnitudeSquared (), sqr (ms_closeToOriginDistance)));
			return 0;
		}
	}

	//-- template must resolve; derive pobCrc + cellCount from it (the
	//   god-client recipe: crc from the portal layout file, cell count = the
	//   .pob root's second int32 minus the exterior cell)
	uint32 portalLayoutCrc = 0;
	int cellCount = 0;
	{
		const ObjectTemplate* const fetched = ObjectTemplateList::fetch (sharedTemplateFilename);
		if (!fetched)
		{
			WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (template-fetch): %s (ObjectTemplateList::fetch returned null)\n", sharedTemplateFilename));
			return 0;
		}

		//-- 2026-08-07 (queued item 6.1): fetch resolves ANY template class and safe_cast
		//   is a bare static_cast in Release -- a consumer template path naming e.g.
		//   object/draft_schematic/* reached getPortalLayoutFilename through a wrongly
		//   typed pointer and died on an indirect call read out of string data
		//   (0xC0000005 DEP at 0x736E6172 = ASCII "rans"). The ten instrumented return-0
		//   branches around this one never covered it. Narrow FIRST so a bad path is an
		//   ordinary REFUSED line, and refuse before the id mint so nothing is mutated.
		const SharedObjectTemplate* const sharedTemplate = fetched->asSharedObjectTemplate ();
		if (!sharedTemplate)
		{
			WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (template-wrong-class): %s (resolved, but not a SharedObjectTemplate)\n", sharedTemplateFilename));
			fetched->releaseReference ();
			return 0;
		}

		const std::string& pobName = sharedTemplate->getPortalLayoutFilename ();
		if (!pobName.empty ())
		{
			bool pobCrcOk = PortalPropertyTemplate::extractPortalLayoutCrc (pobName.c_str (), portalLayoutCrc);
			bool pobOpenOk = true;

			if (pobCrcOk)
			{
				Iff iff;
				if (iff.open (pobName.c_str (), true))
				{
					iff.enterForm ();
					iff.enterForm ();
					iff.enterChunk ();
					IGNORE_RETURN (iff.read_int32 ());
					cellCount = iff.read_int32 () - 1;
					if (cellCount < 0)
						cellCount = 0;
				}
				else
					pobOpenOk = false;
			}

			//-- a POB can never go into a container (the buildout loader FATALs
			//   on exactly this shape -- "Tried to add a pob to a cell")
			if (!pobCrcOk || !pobOpenOk || containedById != 0)
			{
				if (!pobCrcOk)
					WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (pob-crc-extract): %s\n", pobName.c_str ()));
				else if (!pobOpenOk)
					WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (pob-open): %s\n", pobName.c_str ()));
				else
					WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (pob-into-container): %s pob=%s containedById=%I64d\n", sharedTemplateFilename, pobName.c_str (), containedById));

				sharedTemplate->releaseReference ();
				return 0;
			}
		}

		sharedTemplate->releaseReference ();
	}

	//-- mint the contiguous range (reader + buildout set + NetworkIdManager + band)
	const int64 networkIdInt = wsAllocateIdRange (cellCount);
	if (!networkIdInt)
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (id-mint): band [floor=%I64d ceiling=%I64d) exhausted/invalid for cellCount=%d\n", ms_wsIdFloor, ms_wsIdCeiling, cellCount));
		return 0;
	}

	//-- MUTATE: node + atomic POB cell expansion, then the FULL streamed-create
	//   bookkeeping (sphere handle for top-level; createObject + addObjectToWorld
	//   exactly like the update() streamed path -- the WorldSnapshot::addObject
	//   gap this shim exists to close)
	IGNORE_RETURN (ms_reader.addObject (networkIdInt, containedById, ConstCharCrcString (sharedTemplateFilename), cellIndex, transform_p, cs_wsDefaultAddRadius, portalLayoutCrc, std::string ()));
	for (int i = 0; i < cellCount; ++i)
		IGNORE_RETURN (ms_reader.addObject (networkIdInt + i + 1, networkIdInt, ConstCharCrcString ("object/cell/shared_cell.iff"), i + 1, Transform::identity, 0.f, 0));

	WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	NOT_NULL (node);

	if (containedById == 0)
		node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));

	//-- init out-of-range: instantiateObject's template-failure path returns null
	//   WITHOUT setting result, so an untouched value must log as [unknown]
	//   (= template instantiate failure), never as a misleading CEC name
	CreateErrorCode result = static_cast<CreateErrorCode> (-1);
	Object* const object = createObject (ms_reader, node, result);
	if (!object)
	{
		//-- pre-validated, so exceptional (template createObject returned null
		//   or a CEC refusal): roll back to nothing-live -- unhook the sphere
		//   handle and tombstone the whole minted range (no live objects exist;
		//   the ids free again)
		WS_EDITOR_LOG (("[editor.ws] wsAddObject REFUSED (createObject): CEC=%d [%s; -1=template-instantiate] template=%s id=%I64d -> rolled back\n", static_cast<int> (result), wsCreateErrorCodeName (result), sharedTemplateFilename, networkIdInt));

		if (node->getSpatialSubdivisionHandle ())
		{
			ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
			node->setSpatialSubdivisionHandle (0);
		}

		for (int i = cellCount; i >= 0; --i)
			ms_reader.removeNode (networkIdInt + i);

		return 0;
	}

	addObjectToWorld (object, node);

	WS_EDITOR_LOG (("[editor.ws] wsAddObject OK: id=%I64d cells=%d template=%s containedById=%I64d\n", networkIdInt, cellCount, sharedTemplateFilename, containedById));

	return networkIdInt;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsAddNodeAt (__int64 explicitId, __int64 containedById, const char* templateFilename, int cellIndex, const float* transform12, float radius, unsigned int portalLayoutCrc)
{
	if (!templateFilename || !*templateFilename || !transform12 || radius < 0.f)
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt REFUSED (args): id=%I64d template=%s transform12=%s radius=%g\n", explicitId, (templateFilename && *templateFilename) ? templateFilename : "(null/empty)", transform12 ? "ok" : "(null)", radius));
		return 0;
	}

	if (ms_parsePending)
		finishLoadNow ();

	//-- the frozen fail-closed set: id band (on-disk int32, positive), reader
	//   collision, LIVE object holding the id (would refuse the spawn later),
	//   buildout-provenance id, missing container (the engine FATAL, finding #2)
	if (explicitId <= 0 || explicitId > static_cast<__int64> (std::numeric_limits<int>::max ()))
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt REFUSED (id-band): id=%I64d (must be positive int32)\n", explicitId));
		return 0;
	}
	if (ms_reader.find (explicitId))
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt REFUSED (id-present): id=%I64d already in the reader\n", explicitId));
		return 0;
	}
	//-- v19 SEMANTIC REFINEMENT (flagged in the Wave-3 handback): the frozen
	//   Wave-2 "id in the buildout set" refusal is RETIRED. Positive v2 buildout
	//   objids are normal SWGSource data and can collide with genuine authored
	//   ids -- refusing on set membership would break undo-replay of any removed
	//   authored node whose id happens to collide. Presence in the READER (the
	//   check above) is the operative collision guard.
	if (NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (explicitId))) != 0)
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt REFUSED (live-object): id=%I64d held by a live object\n", explicitId));
		return 0;
	}
	if (containedById != 0 && !ms_reader.find (containedById))
	{
		WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt REFUSED (container-missing): containedById=%I64d not in the reader\n", containedById));
		return 0;
	}

	Transform transform_p;
	wsTransformFromFloats (transform12, transform_p);

	IGNORE_RETURN (ms_reader.addObject (explicitId, containedById, ConstCharCrcString (templateFilename), cellIndex, transform_p, radius, portalLayoutCrc, std::string ()));

	WorldSnapshotReaderWriter::Node* const node = ms_reader.find (explicitId);
	NOT_NULL (node);

	if (containedById == 0)
	{
		//-- top-level replay: sphere handle + DIRTY the update-diff sentinels --
		//   update() early-outs for a stationary player (the review-caught
		//   starvation), so force the next pass to run the full diff. The spawn
		//   itself stays streaming's job (a distant undo must not force-spawn;
		//   the whole one-batch-replayed subtree is visible to the createObject
		//   recursion when the pass fires).
		node->setSpatialSubdivisionHandle (ms_sphereTree.addObject (node));
		ms_lastCellProperty = 0;
		ms_lastPosition_w.set (0.f, -9999.f, 0.f);
	}
	else
	{
		//-- child under a spawned, in-world parent: immediate spawn -- NO engine
		//   path would ever spawn it (children hold no sphere handles and the
		//   cell-fill only fires on a cell's not-in-world transition, the
		//   review-caught never-spawns case). Parent not spawned -> data-only;
		//   the POB's own streaming create recurses all children present then.
		Object* const containerObject = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (containedById)));
		if (containerObject && containerObject->isInWorld ())
		{
			CreateErrorCode result = static_cast<CreateErrorCode> (-1);
			Object* const object = createObject (ms_reader, node, result);
			if (object)
				addObjectToWorld (object, node);
			else
				//-- a spawn refusal (e.g. template no longer loadable) leaves the
				//   DATA replay in place -- still a successful re-add; logged so a
				//   visually-absent replay is attributable
				WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt: DATA re-added but child spawn refused: CEC=%d [%s; -1=template-instantiate] id=%I64d\n", static_cast<int> (result), wsCreateErrorCodeName (result), explicitId));
		}
	}

	WS_EDITOR_LOG (("[editor.ws] wsAddNodeAt OK: id=%I64d containedById=%I64d template=%s\n", explicitId, containedById, templateFilename));

	return 1;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsRemoveNode (__int64 networkIdInt)
{
	if (ms_parsePending)
		finishLoadNow ();

	WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (!node || wsIsBuildoutNode (node))
	{
		WS_EDITOR_LOG (("[editor.ws] wsRemoveNode MISS: id=%I64d (%s)\n", networkIdInt, node ? "buildout-provenance node" : "no live node"));
		return 0;
	}

	//-- (1) capture the subtree FIRST (tombstoning zeroes node ids)
	NodeList subtreeNodes;
	std::vector<int64> subtreeIds;
	wsCollectSubtree (node, subtreeNodes, subtreeIds);

	//-- (2) OCCUPANCY GUARD (load-bearing, ANSWERS 5.5), BIDIRECTIONAL since the
	//   2026-07-18 cantina flag. Container::~Container cascade-deletes every
	//   contained object -- deleting a POB with the player inside would delete
	//   THEM -- but on server sessions the client does not necessarily link
	//   occupants into the cell's Container CONTENTS (containment is server-
	//   authoritative; the client tracks the occupant's render/physics cell via
	//   Object::getParentCell). The contents walk and the delete cascade share
	//   that blind spot symmetrically (which is why the flagged delete despawned
	//   cleanly), but the occupant's parentCell would dangle into the deleted
	//   building. So: (i) DOWNWARD -- the recursive contents walk (linked
	//   containment; what the cascade would actually delete); (ii) UPWARD -- any
	//   live non-client-cached object PARKED IN a subtree cell by parentCell,
	//   regardless of contents linkage.
	for (size_t i = 0; i < subtreeIds.size (); ++i)
	{
		Object* const object = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (subtreeIds [i])));
		if (object)
		{
			ClientObject* const clientObject = dynamic_cast<ClientObject*> (object);
			if (!clientObject || !ContainerInterface::isClientCachedOnly (*clientObject))
			{
				WS_EDITOR_LOG (("[editor.ws] wsRemoveNode OCCUPIED (contents): root=%I64d subtree-id=%I64d holds a non-client-cached occupant (%s)\n", networkIdInt, subtreeIds [i], clientObject ? "in contents" : "not a ClientObject"));
				return -1;   // "occupied" -- editor tells the user to step out first
			}
		}
	}
	{
		std::set<int64> subtreeIdSet (subtreeIds.begin (), subtreeIds.end ());

		const NetworkIdManager::NetworkIdObjectHashMap& allObjects = NetworkIdManager::getAllObjects ();
		for (NetworkIdManager::NetworkIdObjectHashMap::const_iterator iter = allObjects.begin (); iter != allObjects.end (); ++iter)
		{
			Object* const object = iter->second;
			if (!object)
				continue;

			//-- cheap rejects first: most objects sit in the world cell
			const CellProperty* const parentCell = object->getParentCell ();
			if (!parentCell || parentCell == CellProperty::getWorldCellProperty ())
				continue;

			const int64 cellOwnerIdInt = parentCell->getOwner ().getNetworkId ().getValue ();
			if (subtreeIdSet.find (cellOwnerIdInt) == subtreeIdSet.end ())
				continue;

			//-- standing inside this subtree. Subtree members despawn with the
			//   building (correct); other CLIENT-CACHED objects re-seat via
			//   streaming; a non-client-cached occupant (the player, any
			//   server-streamed NPC/vendor) refuses the delete.
			const int64 occupantIdInt = iter->first.getValue ();
			if (subtreeIdSet.find (occupantIdInt) != subtreeIdSet.end ())
				continue;

			ClientObject* const clientObject = dynamic_cast<ClientObject*> (object);
			if (!clientObject || !ContainerInterface::isClientCachedOnly (*clientObject))
			{
				WS_EDITOR_LOG (("[editor.ws] wsRemoveNode OCCUPIED (parent-cell): root=%I64d occupant=%I64d stands in cell-owner=%I64d (%s)\n", networkIdInt, occupantIdInt, cellOwnerIdInt, clientObject ? "non-client-cached" : "not a ClientObject"));
				return -1;
			}
		}
	}

	//-- (3) sphere handle (root only -- children never hold handles)
	if (node->getSpatialSubdivisionHandle ())
	{
		ms_sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
		node->setSpatialSubdivisionHandle (0);
	}

	//-- (4) recursive in-world unmark
	node->removeFromWorld ();

	//-- (5) live despawn, root first: deleting the root cascades through cells/
	//   contents (the unload() shape); the loop then catches any straggler
	for (size_t i = 0; i < subtreeIds.size (); ++i)
	{
		Object* const object = NetworkIdManager::getObjectById (NetworkId (static_cast<NetworkId::NetworkIdType> (subtreeIds [i])));
		if (object)
		{
			if (object->isInWorld ())
				object->removeFromWorld ();

			delete object;
		}
	}

	//-- (6) ms_loadedList subtree sweep (children reach it via the cell-fill
	//   path -- finding #4). Pending lists need no purge: update() rebuilds
	//   them from the diff before every drain.
	for (size_t i = 0; i < subtreeNodes.size (); ++i)
	{
		NodeList::iterator iter = std::find (ms_loadedList.begin (), ms_loadedList.end (), subtreeNodes [i]);
		if (iter != ms_loadedList.end ())
			IGNORE_RETURN (ms_loadedList.erase (iter));
	}

	//-- (7) tombstone every subtree id -- the map frees them all, keeping the
	//   allocator's map-miss free-test exact
	for (size_t i = 0; i < subtreeIds.size (); ++i)
		ms_reader.removeNode (subtreeIds [i]);

	WS_EDITOR_LOG (("[editor.ws] wsRemoveNode OK: id=%I64d subtree=%u nodes\n", networkIdInt, static_cast<unsigned int> (subtreeIds.size ())));

	return 1;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsSetNodeRadius (__int64 networkIdInt, float radius)
{
	if (radius < 0.f)
		return 0;

	if (ms_parsePending)
		finishLoadNow ();

	WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (!node || wsIsBuildoutNode (node))
		return 0;

	node->setRadius (radius);

	//-- radius changes the node's sphere extent -- re-seat it (the moveObject pattern)
	if (node->getSpatialSubdivisionHandle ())
		ms_sphereTree.move (node->getSpatialSubdivisionHandle ());

	return 1;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsConfigureIdAllocator (__int64 floorId, __int64 ceilingId)
{
	const int64 newFloor   = floorId   != 0 ? floorId   : ms_wsIdFloor;
	const int64 newCeiling = ceilingId != 0 ? ceilingId : ms_wsIdCeiling;

	//-- band sanity: positive, int32-safe ceiling (the on-disk id width), floor
	//   below ceiling. Rejection is VISIBLE (returns 0, nothing changed).
	if (   newFloor < 0
	    || newCeiling <= 0
	    || newCeiling > static_cast<__int64> (std::numeric_limits<int>::max ())
	    || (newFloor != 0 && newFloor >= newCeiling))
		return 0;

	ms_wsIdFloor   = newFloor;
	ms_wsIdCeiling = newCeiling;

	return 1;
}

//===================================================================
// Goal B Wave 3 (hookpoints v18 -> v19; frozen 2026-07-18): PERSISTENCE.
// The disk half -- nothing here mints or spawns. Semantics per ANSWERS 5.1
// (a-d): authored-only + tombstone-skip save, absolute destination in the
// winning loose SearchPath, negative-cache invalidation, post-write shadow
// verification, sticky-scene-name reset on unload.
//
// NOTE on provenance (the Wave-3 request's §3 rider): the feared "runtime
// server nodes in the reader" class DOES NOT EXIST -- the above-ceiling ids
// observed live (609,457,649 etc.) are AUTHORED nodes of the TOC-resolved
// patch_55 .ws copies (NGE collection-system items: hanging lights,
// paintings, the collection fan in cantina cell 1134566) and MUST serialize.
// Nothing inserts server-streamed objects into ms_reader at runtime; the
// authored-only filter here is complete with tombstone-skip + the retained
// buildout set. See 2026-07-18-utinni-goalB-wave2-idmint-CLOSED.md.
//===================================================================

#include <direct.h>  // _mkdir (save destination directory)
#include <cctype>    // tolower (path comparison)
#include "sharedFoundation/Os.h"  // Os::MAX_PATH_LENGTH (resolve buffer)

namespace WorldSnapshotNamespace
{
	//-- engine_wsSaveSnapshot typed result codes. FROZEN once published in the
	//   Wave-3 handback -- append-only from then on.
	enum WsSaveResult
	{
		WSR_ok                  = 0,
		WSR_noSnapshotLoaded    = 1,   // no scene name / nothing to save
		WSR_noLooseSearchPath   = 2,   // no loose SearchPath configured -- nowhere to write
		WSR_destinationShadowed = 3,   // written, but a higher-priority archive still wins the name
		WSR_idInt32Overflow     = 4,   // an authored id/containedById won't round-trip the on-disk int32
		WSR_buildoutSetIntegrity= 5,   // RESERVED (retired same-day, 2026-07-18 self-test): the id-based
		                               // tripwire mis-fired on NORMAL SWGSource data -- positive v2
		                               // buildout objids exist at scale (TOC-indexed per-area tables);
		                               // provenance is IDENTITY-keyed now, so the ambiguity the tripwire
		                               // guarded cannot occur. Code kept so the published enum is stable.
		WSR_writeFailure        = 6    // Iff/Os write failed (disk/permissions)
	};

	bool wsSaveIncludeTopLevelNode (const WorldSnapshotReaderWriter::Node* const node, void*)
	{
		return ms_buildoutTopLevelNodes.find (node) == ms_buildoutTopLevelNodes.end ();
	}

	//-- builds "<top loose SearchPath root>/snapshot/<scene>.ws"; empty = no path
	void wsBuildSaveDestination (std::string& saveRoot, std::string& destination)
	{
		saveRoot.clear ();
		destination.clear ();

		const char* const root = TreeFile::getSearchPath (0);
		if (!root || !*root)
			return;

		saveRoot = root;
		const char last = saveRoot [saveRoot.size () - 1];
		if (last != '/' && last != '\\')
			saveRoot += '/';

		destination = saveRoot;
		destination += "snapshot/";
		destination += ms_sceneName;
		destination += ".ws";
	}

	//-- case/slash-insensitive path compare (Win32 filesystem semantics)
	bool wsPathsEquivalent (const char* a, const char* b)
	{
		if (!a || !b)
			return false;

		while (*a && *b)
		{
			char ca = *a++;
			char cb = *b++;
			if (ca == '\\') ca = '/';
			if (cb == '\\') cb = '/';
			if (tolower (static_cast<unsigned char> (ca)) != tolower (static_cast<unsigned char> (cb)))
				return false;
		}

		return *a == *b;
	}
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsSaveSnapshot (void)
{
	if (ms_parsePending)
		finishLoadNow ();

	if (ms_sceneName.empty ())
	{
		WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot REFUSED (%d no-snapshot): nothing loaded\n", WSR_noSnapshotLoaded));
		return WSR_noSnapshotLoaded;
	}

	//-- (the finding-#5 id tripwire lived here and was RETIRED same-day: the
	//   2026-07-18 self-test proved positive buildout objids are NORMAL data --
	//   provenance is identity-keyed now, WSR_buildoutSetIntegrity reserved)

	//-- id-width fail-closed (ANSWERS 5.1c): every AUTHORED node (the set that
	//   will serialize) must round-trip the on-disk int32
	{
		NodeList stack;
		for (int i = 0; i < ms_reader.getNumberOfNodes (); ++i)
		{
			const WorldSnapshotReaderWriter::Node* const node = ms_reader.getNode (i);
			if (node->isDeleted () || !wsSaveIncludeTopLevelNode (node, 0))
				continue;
			stack.push_back (node);
		}

		const int64 int32Max = static_cast<int64> (std::numeric_limits<int>::max ());
		while (!stack.empty ())
		{
			const WorldSnapshotReaderWriter::Node* const node = stack.back ();
			stack.pop_back ();

			if (   node->getNetworkIdInt () <= 0 || node->getNetworkIdInt () > int32Max
			    || node->getContainedByNetworkIdInt () < 0 || node->getContainedByNetworkIdInt () > int32Max)
			{
				WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot REFUSED (%d id-int32-overflow): id=%I64d containedBy=%I64d\n", WSR_idInt32Overflow, node->getNetworkIdInt (), node->getContainedByNetworkIdInt ()));
				return WSR_idInt32Overflow;
			}

			for (int i = 0; i < node->getNumberOfNodes (); ++i)
				if (!node->getNode (i)->isDeleted ())
					stack.push_back (node->getNode (i));
		}
	}

	//-- destination: the winning loose SearchPath (never CWD-relative)
	std::string saveRoot;
	std::string destination;
	wsBuildSaveDestination (saveRoot, destination);
	if (destination.empty ())
	{
		WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot REFUSED (%d no-loose-search-path)\n", WSR_noLooseSearchPath));
		return WSR_noLooseSearchPath;
	}

	//-- ensure <root>/snapshot exists (EEXIST is fine)
	IGNORE_RETURN (_mkdir ((saveRoot + "snapshot").c_str ()));

	//-- authored-only + tombstone-skip filtered save
	if (!ms_reader.saveFiltered (destination.c_str (), wsSaveIncludeTopLevelNode, 0))
	{
		WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot REFUSED (%d write-failure): %s\n", WSR_writeFailure, destination.c_str ()));
		return WSR_writeFailure;
	}

	//-- the CONSULT-59 negative cache would keep the freshly written file
	//   invisible for the already-probed name -- clear it BEFORE the resolve
	char relativeName [256];
	IGNORE_RETURN (snprintf (relativeName, sizeof (relativeName) - 1, "snapshot/%s.ws", ms_sceneName.c_str ()));
	relativeName [sizeof (relativeName) - 1] = '\0';
	TreeFile::forgetMissingFile (relativeName);

	//-- post-write shadow verification: the engine must resolve the name to
	//   the file we just wrote, or the save is a silent no-op for reload
	char resolved [Os::MAX_PATH_LENGTH];
	resolved [0] = '\0';
	if (!TreeFile::getPathName (relativeName, resolved, sizeof (resolved)) || !wsPathsEquivalent (resolved, destination.c_str ()))
	{
		WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot REFUSED (%d destination-shadowed): wrote %s but the name resolves to %s\n", WSR_destinationShadowed, destination.c_str (), resolved));
		return WSR_destinationShadowed;
	}

	WS_EDITOR_LOG (("[editor.ws] wsSaveSnapshot OK: %s\n", destination.c_str ()));

	return WSR_ok;
}

//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsGetSavePath (char* buf, int cap)
{
	std::string saveRoot;
	std::string destination;
	wsBuildSaveDestination (saveRoot, destination);
	if (saveRoot.empty ())
		return 0;

	const int needed = static_cast<int> (saveRoot.size ()) + 1;
	if (buf && cap > 0)
		memcpy (buf, saveRoot.c_str (), static_cast<size_t> (cap < needed ? cap : needed));

	return needed;
}

//-------------------------------------------------------------------

extern "C" void __cdecl engine_wsUnloadSnapshot (void)
{
	//-- unload() cancels any in-flight phased parse itself and bumps the
	//   generation. The scene-name reset is the ANSWERS §2 delta: load() would
	//   otherwise early-out on the sticky name and reload would return EMPTY.
	ms_unloadReason = "wsUnload";   // CONSULT-71 probe tag: THE in-place editor reload -- the path that destroys interior NPCs
	WorldSnapshot::unload ();
	ms_sceneName.clear ();

	WS_EDITOR_LOG (("[editor.ws] wsUnloadSnapshot OK (scene name reset; generation=%d; keptServerOwnedRoots=%d)\n", ms_wsEditGeneration, ms_lastUnloadSkippedRoots));
}

//-------------------------------------------------------------------
// wsSetNodeTemplateName -- in-place .ws node template re-point (v22 -> v23,
// 2026-07-19 toolkit change request; the CONSULT-70 lossless-rebind fix for
// per-instance interior editing, model D). Re-points an EXISTING authored
// node at a new object-template NAME -- interns the name in the snapshot's
// OTNL table (WorldSnapshotReaderWriter::internObjectTemplateName, the
// addObject intern path exposed; append-only, no other node disturbed) and
// swaps the node's index. Does NOT touch cells, children, id, transform,
// radius, or portalLayoutCrc; the LIVE spawned object is untouched (the
// swap is data-only -- reload spawns from the new template; the consumer's
// derived template inherits the .pob so the spawn-time crc check still
// passes -- THEIR stated guarantee, and a mismatch fails loudly at spawn,
// never silently). Fail-closed: authored non-buildout nodes only, and the
// new template must RESOLVE NOW via TreeFile (forgetMissingFile first --
// the CONSULT-59 negative cache may hold a stale miss for a derived .iff
// the consumer wrote seconds ago). Caller follows with wsSaveSnapshot.
// Returns 1 ok / 0 miss (no such authored node) / -1 refused (empty name,
// buildout-provenance node, or unresolvable template).
//-------------------------------------------------------------------

extern "C" int __cdecl engine_wsSetNodeTemplateName (__int64 networkIdInt, const char* name)
{
	if (!name || !*name)
	{
		WS_EDITOR_LOG (("[editor.ws] wsSetNodeTemplateName REFUSED (args): id=%I64d name=%s\n", networkIdInt, name ? "(empty)" : "(null)"));
		return -1;
	}

	if (ms_parsePending)
		finishLoadNow ();

	WorldSnapshotReaderWriter::Node* const node = ms_reader.find (networkIdInt);
	if (!node)
	{
		WS_EDITOR_LOG (("[editor.ws] wsSetNodeTemplateName MISS: id=%I64d (not in authored map -- unknown id or editor-removed)\n", networkIdInt));
		return 0;
	}
	if (wsIsBuildoutNode (node))
	{
		WS_EDITOR_LOG (("[editor.ws] wsSetNodeTemplateName REFUSED (buildout): id=%I64d\n", networkIdInt));
		return -1;
	}

	TreeFile::forgetMissingFile (name);
	if (!TreeFile::exists (name))
	{
		WS_EDITOR_LOG (("[editor.ws] wsSetNodeTemplateName REFUSED (template-missing): id=%I64d name=%s (stage the derived .iff BEFORE the swap)\n", networkIdInt, name));
		return -1;
	}

	const char* const oldName = ms_reader.getObjectTemplateName (node->getObjectTemplateNameIndex ());
	node->setObjectTemplateNameIndex (ms_reader.internObjectTemplateName (ConstCharCrcString (name)));

	WS_EDITOR_LOG (("[editor.ws] wsSetNodeTemplateName OK: id=%I64d %s -> %s\n", networkIdInt, oldName ? oldName : "(?)", name));
	return 1;
}

//===================================================================
