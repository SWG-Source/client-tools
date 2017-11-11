// ======================================================================
//
// ClientProceduralTerrainAppearance_LevelOfDetail.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_LevelOfDetail.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/Camera.h"
#include "clientTerrain/ClientProceduralTerrainAppearance_ClientChunk.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include "sharedFoundation/Os.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace ClientProceduralTerrainAppearanceNamespace
{	
	const float ms_sqrt_2 = sqrt (2.f);

	typedef ClientProceduralTerrainAppearance::ClientChunk ClientChunk;

	//-------------------------------------------------------------------

	void selectChunkForRender(TerrainQuadTree::Node* node)
	{
		if (node->getChunk ())
		{
			ClientChunk *chunk = safe_cast<ClientChunk*>(node->getChunk());
			chunk->addObjectToWorld();
		}
		node->setSelectedForRender(true);
	}

	//-------------------------------------------------------------------

	void deselectChunkForRender(TerrainQuadTree::Node* node)
	{
		if (node->getChunk ())
		{
			ClientChunk *chunk = safe_cast<ClientChunk*>(node->getChunk());
			chunk->removeObjectFromWorld ();
		}
		node->setSelectedForRender(false);
	}

	//-------------------------------------------------------------------
	
	void deselectChildChunksForRender(TerrainQuadTree::Node* node)
	{
		for (int i = 0; i < 4; ++i)
		{
			TerrainQuadTree::Node *child = node->getSubNode(i);
			if (child)
			{
				deselectChunkForRender(child);
				deselectChildChunksForRender(child);
			}
		}
	}

	//-------------------------------------------------------------------

	void cleanupSubNodeChunks(TerrainQuadTree::Node& node)
	{
		//-- delete some subnodes in a cleanup pass
		if (node.getSize() > 2 && node.getNumberOfChunks() > 1)
		{
			for (int i = 0; i < 4; ++i)
			{
				TerrainQuadTree::Node *child = node.getSubNode(i);
				if (child)
				{
					IGNORE_RETURN (child->removeSubNodes(true));
				}
			}
		}
	}

	//-------------------------------------------------------------------

	bool s_installed = false;
}

using namespace ClientProceduralTerrainAppearanceNamespace;

//----------------------------------------------------------------------

bool      ClientProceduralTerrainAppearance::LevelOfDetail::ms_useHeightBias;
float     ClientProceduralTerrainAppearance::LevelOfDetail::ms_forceHighThreshold;
float     ClientProceduralTerrainAppearance::LevelOfDetail::ms_threshold;
float     ClientProceduralTerrainAppearance::LevelOfDetail::ms_heightBiasFactor;
int       ClientProceduralTerrainAppearance::LevelOfDetail::ms_heightBiasMax;

// ======================================================================

ClientProceduralTerrainAppearance::LevelOfDetail::LevelOfDetail (ClientProceduralTerrainAppearance& cpta) :
	m_cpta (cpta),
	m_chunkRequestInfoList (new ChunkRequestInfoList),
	m_referenceObject (0),
	m_frustum (0),
	m_dirty (true),
	m_buildRadiusSquared (0.f),
	m_thresholdInternal (0.0f),
	m_heightBiasThresholdInternal (0),
	m_forceHighRange (0.0f),
	m_forceHighRangeSquared (0.0f)
{
	if (!s_installed)
	{
		ms_useHeightBias     = !ConfigClientTerrain::getHeightBiasDisabled ();
		setThreshold          (ConfigClientTerrain::getThreshold ());
		setHeightBiasMax      (ConfigClientTerrain::getHeightBiasMax ());
		setHeightBiasFactor   (ConfigClientTerrain::getHeightBiasFactor ());
		setForceHighThreshold (ConfigClientTerrain::getHighLevelOfDetailThreshold ());
		
		const char * const section = "ClientProceduralTerrainAppearance_LevelOfDetail";
		LocalMachineOptionManager::registerOption (ms_useHeightBias,       section, "useHeightBias");
		LocalMachineOptionManager::registerOption (ms_forceHighThreshold,  section, "forceHighThreshold");
		LocalMachineOptionManager::registerOption (ms_threshold,           section, "threshold");
		LocalMachineOptionManager::registerOption (ms_heightBiasFactor,    section, "heightBiasFactor");
		LocalMachineOptionManager::registerOption (ms_heightBiasMax,       section, "heightBiasMax");
		
		s_installed = true;
	}
	
	//-- set these to reset the internal precalculations
	setThreshold          (ms_threshold);
	setHeightBiasMax      (ms_heightBiasMax);
	setHeightBiasFactor   (ms_heightBiasFactor);
	setForceHighThreshold (ms_forceHighThreshold);
}

//-------------------------------------------------------------------

ClientProceduralTerrainAppearance::LevelOfDetail::~LevelOfDetail ()
{
	delete m_chunkRequestInfoList;
	m_chunkRequestInfoList = 0;
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::removeAllObjectsFromWorld (TerrainQuadTree::Node* const node) const
{
	TerrainQuadTree::Iterator clean_node_iter (node);
	TerrainQuadTree::Node* childNode = 0;

	//-- iterate through the quadtree
	while ((childNode = clean_node_iter.getCurNode ()) != 0)
	{
		if (childNode->getChunk ())
			safe_cast<ClientProceduralTerrainAppearance::ClientChunk*> (childNode->getChunk ())->removeObjectFromWorld ();

		IGNORE_RETURN (clean_node_iter.descend ());
	}
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setThreshold (const float threshold)
{
	ms_threshold = threshold;
	m_thresholdInternal = 1 / (ms_threshold * m_cpta.getChunkWidthInMeters ());

	setDirty (true);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setHeightBiasMax (const int heightBiasMax)
{
	ms_heightBiasMax = heightBiasMax;
	setDirty (true);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setHeightBiasFactor (const float heightBiasFactor)
{
	ms_heightBiasFactor = heightBiasFactor;
	m_heightBiasThresholdInternal = static_cast<int>(ms_heightBiasFactor * m_cpta.getChunkWidthInMeters ());
	setDirty (true);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setForceHighThreshold (const float forceHighThreshold)
{
	ms_forceHighThreshold = forceHighThreshold;

	// the literal float const added to this is a a hack to prevent artifacts in tools like the
	// viewer where you can rotate around a chunk corner at a precise distance.
	
	m_forceHighRange = (ms_forceHighThreshold * m_cpta.getChunkWidthInMeters ()) + 0.012345f;
	m_forceHighRangeSquared = sqr (m_forceHighRange);

	setDirty (true);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setUseHeightBias (const bool useHeightBias)
{
	ms_useHeightBias = useHeightBias;
	setDirty (true);
}

//-----------------------------------------------------------------

const ClientProceduralTerrainAppearance::ChunkRequestInfoList& ClientProceduralTerrainAppearance::LevelOfDetail::getChunkRequestInfoList () const
{
	return *m_chunkRequestInfoList;
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::LevelOfDetail::setDirty (const bool dirty)
{
	m_dirty = dirty;
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::LevelOfDetail::isDirty () const
{
	return m_dirty;
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::LevelOfDetail::getUseHeightBias () const
{
	return ms_useHeightBias;
}

//-----------------------------------------------------------------

float ClientProceduralTerrainAppearance::LevelOfDetail::getThreshold ()
{
	return ms_threshold;
}

//-----------------------------------------------------------------

int ClientProceduralTerrainAppearance::LevelOfDetail::getHeightBiasMax () const
{
	return ms_heightBiasMax;
}

//-----------------------------------------------------------------

float ClientProceduralTerrainAppearance::LevelOfDetail::getHeightBiasFactor () const
{
	return ms_heightBiasFactor;
}

//-----------------------------------------------------------------

float ClientProceduralTerrainAppearance::LevelOfDetail::getForceHighThreshold ()
{
	return ms_forceHighThreshold;
}

//-------------------------------------------------------------------

int ClientProceduralTerrainAppearance::LevelOfDetail::computeDesiredLevelOfDetail (TerrainQuadTree::Node & snode, float & closeness) const
{
	const float distanceSquaredToNode = snode.getDistanceSquared (m_referenceObject->getPosition_w ());
	
	//-- get minimum distance to node (closest corner)
	closeness = m_buildRadiusSquared - distanceSquaredToNode;

	//-- see if this node is inside our build radius
	if (closeness < 0)
	{
		snode.setOutsideBuildRange (true);
		return snode.getSize ();
	}
	
	// if the distance is within the high level of detail range, then just treat the distance as zero
	// otherwise, treat the distance as relative to the high-detail range.
	const float dist = (distanceSquaredToNode <= m_forceHighRangeSquared) ? 0 : (sqrt (distanceSquaredToNode) - m_forceHighRange);
	
	int levelOfDetailBias = 0;
	
	// this bias factor should be determined on a per-map or per-region basis, not
	// be arbitrary
	
	if (ms_useHeightBias && m_heightBiasThresholdInternal && snode.isBoxExtentInitialized ())
	{					
		const int delta_y = static_cast <int> (snode.getBoxExtent().getMax ().y - snode.getBoxExtent ().getMin ().y);
		
		levelOfDetailBias = delta_y / m_heightBiasThresholdInternal;
		levelOfDetailBias = std::min (levelOfDetailBias, static_cast<int>(snode.getSize () * ms_heightBiasMax / 100));		
		levelOfDetailBias *= snode.getSize ();					
	}

	int result = static_cast<int>(dist * m_thresholdInternal) + 1 - levelOfDetailBias;
	if (result<1)
	{
		result=1;
	}

	return result;
}

//-----------------------------------------------------------------

int ClientProceduralTerrainAppearance::LevelOfDetail::requestSubNodeChunks(const TerrainQuadTree::Node& node, const int priority)
{
	const int half = node.getSize () / 2;
	
	int count = 0;
	for (int i = 0; i < 4; ++i)
	{
		const TerrainQuadTree::Node *child = node.getSubNode(i);

		if (child == 0 || child->getChunk() == 0)
		{			
			const int x = (i == 2 || i == 1) ? node.getX () : node.getX () + half;
			const int z = (i == 2 || i == 3) ? node.getZ () : node.getZ () + half;
			
			const ChunkRequestInfo ri (x, z, half, Graphics::getFrameNumber () + priority);
			m_chunkRequestInfoList->push_back (ri);

			++count;
		}
	}

	return count;
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::LevelOfDetail::attemptSplit (TerrainQuadTree::Node * node)
{
	//-- hit the bottom of the tree, only one thing to do!
	if (node->getSize () == 1)
	{
		selectChunkForRender(node);
		// No children to worry about.
		return true;
	}

	float closeness = 0.f;

	//-- Get desired level of detail for this node.
	//-- Detail level is a measure of the node's desired
	//-- size in chunk-space.
	const int desiredSize = computeDesiredLevelOfDetail (*node, closeness);

	//-- double the priority of chunks inside the frustum
	const bool frustum_intersects = !node->isBoxExtentInitialized () || m_frustum->intersects (node->getBoxExtent ().getSphere ());
	if (frustum_intersects)
	{
		closeness *= 2;
	}

	//-- if the node was marked as outstide the build range
	//-- free up its sub-tree, starting with grandchildren.
	if (node->isOutsideBuildRange ())
	{
		cleanupSubNodeChunks(*node);
		deselectChunkForRender(node);
		deselectChildChunksForRender(node); // make sure there are no children selected for render
		return true;
	}

	//-- if the desired size of this node is less than the actual
	//-- size, then split it.
	if (desiredSize < node->getSize())
	{
		const bool wasFilled = 0 == requestSubNodeChunks(*node, static_cast<int>(closeness));
		
		if (wasFilled)
		{
			// This is the only non-terminal exit case.
			// A successful split.
			deselectChunkForRender(node);
			return true;
		}
		else
		{
			selectChunkForRender(node);
			deselectChildChunksForRender(node); // make sure there are no children selected for render
			//-- this indicates that we had to stop splitting nodes prematurely, thus the perfect level of detail fill is incomplete
			return false;
		}
	}
	else //-- desired size of this node is GE than the actual size.
	{
		//-- free up node's sub-tree, starting with grandchildren.
		cleanupSubNodeChunks(*node);
		NOT_NULL (node->getChunk());
		selectChunkForRender(node);
		deselectChildChunksForRender(node); // make sure there are no children selected for render

		return true;
	}
} 

//-----------------------------------------------------------------

struct Wombat
{
	int outer [2]; // neighbors outside the parent quad
};

static const Wombat s_wombats[4] =
{
	{
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::EAST}
	},
	{
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::WEST}
	},
	{
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::WEST}
	},
	{
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::EAST}
	}
};

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::LevelOfDetail::processSplitList()
{
	
	bool complete = true;

	//-- attempt splits on all nodes.
	{
		for (SplitList::iterator iter = m_currentSplitList.begin(); iter != m_currentSplitList.end(); ++iter)
		{
			complete = attemptSplit (*iter) && complete;
		}
	}
	
	
	{
		for (SplitList::iterator iter = m_currentSplitList.begin (); iter != m_currentSplitList.end (); ++iter)
		{
			
			TerrainQuadTree::Node * node = (*iter);
			
			NOT_NULL (node);
			
			if (node->isOutsideBuildRange())
			{
				continue;
			}

			// if a node is selected for render after 'attemptSplit' this means:
			//    a) The node is of size 1.
			// OR b) requestSubNodeChunks returned non-zero (requests still pending).
			// OR c) the node was already high detail enough.
			//
			// if a node is NOT selected for render after 'attemptSplit' this means:
			//    a) The node was outside the build range. (case handled above).
			//    b) The node was split and no requests are pending (complete children).

			if (!node->isSelectedForRender()) // If the node was split and no requests are pending (complete children).
			{
			#ifdef _DEBUG
				ClientChunk *_chunk = safe_cast<ClientChunk*>(node->getChunk());
				DEBUG_FATAL(_chunk && _chunk->isObjectInWorld(), ("chunk should not be in world."));
			#endif

				bool larger [4] = { false, false, false, false };
				int i;
				
				//-- check the edges for larger neighbors
				for (i = 0; i < 4; ++i)
				{
					const TerrainQuadTree::Node * const neighbor = node->getNeighbor (i);

					if (neighbor && neighbor->isSelectedForRender())
					{
						larger[i] = true;
					}
				}

				//-- handle the corners
				for (i = 0; i < 4; ++i)
				{
					const Wombat & wb = s_wombats [i];

					TerrainQuadTree::Node * const snode = node->getSubNode (i);

					//-- cannot descend
					if (larger [wb.outer [0]] || larger [wb.outer [1]])
					{
						snode->setHasLargerNeighbor (wb.outer [0], larger [wb.outer [0]]);
						snode->setHasLargerNeighbor (wb.outer [1], larger [wb.outer [1]]);
						
						// if our chunks larger-neighbor flags don't match the node's larger-neighbor flags
						// reset the chunk's flags.
						NOT_NULL (snode->getChunk ());
						if (snode->getHasLargerNeighbor(0) != snode->getChunk()->getHasLargerNeighbor(0) ||
							 snode->getHasLargerNeighbor(1) != snode->getChunk()->getHasLargerNeighbor(1) ||
							 snode->getHasLargerNeighbor(2) != snode->getChunk()->getHasLargerNeighbor(2) ||
							 snode->getHasLargerNeighbor(3) != snode->getChunk()->getHasLargerNeighbor(3)
							 )
						{
							safe_cast <ClientChunk *>(snode->getChunk ())->resetIndices (snode->getHasLargerNeighbors());
						}

						selectChunkForRender(snode);

						// make sure there are no children selected for render
						// Not sure if this is needed....
						deselectChildChunksForRender(snode); 
					}
					else //-- can descend
					{
						m_nextSplitList.push_back (snode);						
					}
				}
			}
			else // is selected for render
			{		
				//-----------------------------------------------------------------
				//-- fixup cracks in unsplit nodes
				//-----------------------------------------------------------------

				const TerrainQuadTree::Node * const p = node->getParent ();

				if (p)
				{
					//-- check the edges for larger neighbors
					for (int i = 0; i < 2; ++i)
					{
						const int outer = s_wombats [node->getChildIndex ()].outer [i];
						const TerrainQuadTree::Node * const neighbor = p->getNeighbor (outer);

						if (neighbor && neighbor->isSelectedForRender ())
						{
							node->setHasLargerNeighbor (outer, true);
						}
					}
				}
				
				// if our chunks larger-neighbor flags don't match the node's larger-neighbor flags
				// reset the chunk's flags.
				NOT_NULL (node->getChunk ());
				if (node->getHasLargerNeighbor(0) != node->getChunk()->getHasLargerNeighbor(0) ||
					 node->getHasLargerNeighbor(1) != node->getChunk()->getHasLargerNeighbor(1) ||
					 node->getHasLargerNeighbor(2) != node->getChunk()->getHasLargerNeighbor(2) ||
					 node->getHasLargerNeighbor(3) != node->getChunk()->getHasLargerNeighbor(3)
					 )
				{
					safe_cast <ClientChunk *>(node->getChunk ())->resetIndices (node->getHasLargerNeighbors());
				}
				
			}
		}
	}
	
	m_currentSplitList.clear ();
	m_currentSplitList.swap(m_nextSplitList);
	return complete;
}


//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::LevelOfDetail::selectActualLevelOfDetail(const Camera* /*const camera*/, 
																											const Object* const referenceObject, 
																											const Volume* const frustum, 
																											TerrainQuadTree::Node* const node
																											)
{
	TerrainQuadTree::Node *snode = 0;
	TerrainQuadTree::Iterator clean_node_iter (node);

	//--------------------------------------------------------------------------
	//-- iterate through the quadtree and remove all the chunks from the world.
	//-- also, prepare each node for LOD calculation by clearing relevant flags.
	while ((snode = clean_node_iter.getCurNode()) != 0)
	{
		snode->setOutsideBuildRange(false);
		snode->setSelectedForRender(false);
		snode->clearHasLargerNeighbors();

		IGNORE_RETURN(clean_node_iter.descend());
	}
	//--------------------------------------------------------------------------
 	
	m_referenceObject = referenceObject;
	m_frustum = frustum;

	// sqrt of 2 is the correct (liberal approximation) value for a 90 degree view frustum
	m_buildRadiusSquared = sqr(4096.f * ms_sqrt_2);

	//--------------------------------------------------------------------------
	m_chunkRequestInfoList->clear ();

	bool complete = true;
	m_nextSplitList.clear();
	m_currentSplitList.clear();
	m_currentSplitList.push_back(node);
	while (!m_currentSplitList.empty())
	{
		complete = processSplitList() && complete;
	}
	//--------------------------------------------------------------------------

#ifdef _DEBUG
	//--------------------------------------------------------------------------
	//-- iterate through the quadtree and remove all the chunks from the world.
	//-- also, prepare each node for LOD calculation by clearing relevant flags.
	{
		TerrainQuadTree::Iterator test_node_iter (node);
		while ((snode = test_node_iter.getCurNode()) != 0)
		{
			ClientChunk *chunk = safe_cast<ClientChunk*>(snode->getChunk());
			if (snode->isSelectedForRender())
			{
				DEBUG_FATAL(!chunk->isObjectInWorld(), ("Chunk is selected for render but not in the world!"));
			}
			else
			{
				if (chunk)
				{
					DEBUG_FATAL(chunk->isObjectInWorld(), ("Chunk is not selected for render but is in the world!"));
				}
			}
			IGNORE_RETURN(test_node_iter.descend());
		}
	}
	//--------------------------------------------------------------------------
#endif

	m_referenceObject = 0;
	m_frustum = 0;

	setDirty (false);

	return complete;
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::initializeLevelOfDetail (const int levels)
{
	DEBUG_FATAL (levels > 5, ("terrain initializeLevelOfDetail levels %d is too deep.\n", levels));

	TerrainQuadTree::Node * snode = 0;
	TerrainQuadTree::Iterator node_iter (getChunkTree ()->getTopNode ());

	const int create_extra_chunk_threshold = getChunkTree ()->getTopNode ()->getSize () / (1 << (levels-1));

	//-- iterate through the quadtree
	while ((snode = node_iter.getCurNode ()) != 0)
	{
		// a chunkless node subtree is worthless for testing distances and/or frustum intersection
		// so the first thing we need to have is some chunks
		
		if (snode->getChunk () == 0 && snode->getSize () >= create_extra_chunk_threshold)
		{
			createChunk (snode->getX (), snode->getZ (), snode->getSize (), snode->getHasLargerNeighbors ());
			
			// place new subnodes if needed
			if (snode->getSize () > 1)
			{
				const int half = snode->getSize () / 2;
				for (int i = 0; i < 4; ++i )
				{
					if (snode->getSubNode (i) == 0)
					{
						snode->addNode ( new TerrainQuadTree::Node(
							(i == 2 || i == 1) ? snode->getX () : snode->getX () + half,
							(i == 2 || i == 3) ? snode->getZ () : snode->getZ () + half,
							half,
							proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ()), i);
					}
				}
			}
			IGNORE_RETURN (node_iter.descend ());
			continue;
		}
		
		IGNORE_RETURN (node_iter.advance ());
		continue;
	}
}

//-----------------------------------------------------------------

bool ClientProceduralTerrainAppearance::selectActualLevelOfDetail (const Camera* camera, const Object* const referenceObject, const Volume* const frustum)
{
	NOT_NULL (referenceObject);
	NOT_NULL (frustum);

	NOT_NULL (getChunkTree ()->getTopNode ());

	TerrainQuadTree::Node * startNode = getChunkTree ()->getTopNode ();
	startNode->clearHasLargerNeighbors ();

	//-- the level of detail generator must have at least the root chunk to function
	if (startNode->getChunk () == 0)
	{		

		DEBUG_FATAL (startNode->getNumberOfChunks (), ("terrain level of detail root node is chunkless but children have chunks.\n"));
		initializeLevelOfDetail (1);
		DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportLogPrint (),(" INIT chunk count %d\n", getChunkTree ()->getNumberOfChunks ()));		
	}

	bool complete = m_levelOfDetail->selectActualLevelOfDetail (camera, referenceObject, frustum, startNode);

#if _DEBUG

	TerrainQuadTree::Iterator node_iter (getChunkTree ()->getTopNode ());

	TerrainQuadTree::Node * snode = 0;
	
	//-- iterate through the quadtree
	while ((snode = node_iter.getCurNode ()) != 0)
	{
		if (snode->isSelectedForRender ())
		{
			const Wombat & wb = s_wombats [snode->getChildIndex ()];

			UNREF (wb);

			DEBUG_FATAL (!snode->getHasLargerNeighbor (wb.outer [0]) &&
				snode->getParent () &&
				snode->getParent ()->getNeighbor (wb.outer [0]) &&
				snode->getParent ()->getNeighbor (wb.outer [0])->isSelectedForRender (), ("crack in terrain\n"));

			DEBUG_FATAL (!snode->getHasLargerNeighbor (wb.outer [1]) &&
				snode->getParent () &&
				snode->getParent ()->getNeighbor (wb.outer [1]) &&
				snode->getParent ()->getNeighbor (wb.outer [1])->isSelectedForRender (), ("crack in terrain\n"));

			IGNORE_RETURN (node_iter.advance ());
			continue;
		}

		IGNORE_RETURN (node_iter.descend ());
	}

#endif

	const ChunkRequestInfoList& requestInfoList = m_levelOfDetail->getChunkRequestInfoList ();

	//-- todo remove this when the level of detail system proactively pre-requests chunks
	DEBUG_FATAL (complete && requestInfoList.size (), ("terrain level of detail claimed to be complete but requested more chunks.\n"));
	
	if (!requestInfoList.empty ())
	{
		if (ms_multiThreadedTerrainGeneration)
			insertChunkCreationRequests (requestInfoList);
		else
		{
			for (ChunkRequestInfoList::const_iterator iter = requestInfoList.begin (); iter != requestInfoList.end (); ++iter)
			{
				const ChunkRequestInfo& requestInfo = (*iter);
				createChunk (requestInfo.m_x, requestInfo.m_z, requestInfo.m_size, 0);
			}
		}
	}

	return complete;
}

// ======================================================================

void ClientProceduralTerrainAppearance::threadRoutine()
{
	for (;;)
	{
		// if the thread is terminated while we are waiting here, then the path of execution will be A
		m_requestGate.wait(); // C1 - fall through

		m_requestCriticalSection.enter (); // A1, C2: wait here until the destructor releases the lock
		for (;;)
		{
			if (m_quitRequestThread) // A2, B3, C3: true
			{
				m_requestCriticalSection.leave(); // A3, B4, C4
				return; // A4, B5, C5 terminate thread
			}

			if (m_pendingChunkRequestInfoMap->empty ())
			{
				break;
			}
			
			TerrainQuadTree::Node * const startNode = getChunkTree ()->getTopNode ();
			if (startNode->getChunk () == 0)		
				WARNING(true, ("ClientProceduralTerrainAppearance_LevelOfDetail processing threadRoutine with no master chunk"));
			
			
			ChunkRequestInfoMap::reverse_iterator riter = m_pendingChunkRequestInfoMap->rbegin ();
			//-- make a copy
			ChunkRequestInfo requestInfo = (*riter).second;
			m_pendingChunkRequestInfoMap->erase ((++riter).base ());

			DEBUG_FATAL (requestInfo.m_chunk, ("pending chunk request had non-null chunk\n"));

			//-- go ahead and insert the chunkless request on the completed set
			m_completedChunkRequestInfoList->push_back (requestInfo);

			//-- we can desynchronize while we do the actual (expensive) work of creating the chunk
			m_requestCriticalSection.leave ();
			// Thread can't terminate until the critical section is released

			requestInfo.m_chunk = createClientChunk (requestInfo.m_x, requestInfo.m_z, requestInfo.m_size, 0);
			// If it terminates while we are creating the chunk, the flow of control will be B

			//-- resync to modify the completed chunk request info
			m_requestCriticalSection.enter (); // B1 - enters here when the destructor releases the lock

			//-- this is valid because inserting/removing elements into a set does not invalidate iterators
			m_completedChunkRequestInfoList->back ().m_chunk = requestInfo.m_chunk; // B2 - useless
		}

		m_requestGate.close();
		m_requestCriticalSection.leave();
		// If the terminate request happens here before we wait on the gate, path C
	}
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::retrieveCompletedChunkCreationRequests ()
{
	if (!ms_multiThreadedTerrainGeneration)
		return;

	m_requestCriticalSection.enter ();
	{
		m_chunksGeneratedThisFrame = m_completedChunkRequestInfoList->size ();

		switch (m_requestThreadMode)
		{
		case RTM0_normal:
			{
				if (!m_completedChunkRequestInfoList->empty ())
				{
					BoxExtent boxExtent;
					boxExtent.setMin (Vector::maxXYZ);
					boxExtent.setMax (Vector::negativeMaxXYZ);

					ChunkRequestInfoList::iterator iter = m_completedChunkRequestInfoList->begin ();
					ChunkRequestInfoList::iterator end = m_completedChunkRequestInfoList->end ();
					for (; iter != end; ++iter)
					{
						const ChunkRequestInfo& requestInfo = *iter;

						//-- a null chunk means that this request is actually still being created
						//-- all the remaining elements of the vector are being created.
						//-- stop here
						if (!requestInfo.m_chunk)
							break;

						// add it to the terrain
						if (getChunkTree ()->findChunk (requestInfo.m_x, requestInfo.m_z, requestInfo.m_size) == 0)
						{
							IGNORE_RETURN (getChunkTree ()->addChunk (requestInfo.m_chunk, requestInfo.m_size));
							createFlora (requestInfo.m_chunk);

							//-- tell the flora system the chunk has changed
							boxExtent.grow (requestInfo.m_chunk->getBoxExtent ());
						}
						else
						{
							static int count = 0;
							DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportLogPrint (), ("Deleted chunk immediately after creating it %d (%d,%d; %d)!\n", ++count, requestInfo.m_x, requestInfo.m_z, requestInfo.m_size));
							delete requestInfo.m_chunk;
							requestInfo.m_chunk = 0;
						}
					}

					IGNORE_RETURN (m_completedChunkRequestInfoList->erase (m_completedChunkRequestInfoList->begin (), iter));

					//-- tell the flora system what regions have changed
					std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), VoidBindSecond (VoidMemberFunction (&ClientRadialFloraManager::markExtentDirty), &boxExtent));

					TerrainObject::terrainChanged (Rectangle2d (boxExtent.getLeft (), boxExtent.getBack (), boxExtent.getRight (), boxExtent.getFront ()));
				}
			}
			break;

		case RTM1_clearingRequests:
			{
				//-- Unfortunately, if we're clearing requests, we need 
				//   throw away chunks we've potentially generated. We 
				//   can't just clear the completed request list because 
				//   we may be in the middle of generating a chunk.
				if (m_completedChunkRequestInfoList->empty ())
				{
					//-- We have no requests to clear, so set our state to 
					//   rebuilding and enqueue the requests to build dirty 
					//   chunks
					insertChunkRebuildRequests (*m_invalidateChunkRequestInfoList);

					m_requestThreadMode = RTM2_rebuilding;
				}
				else
				{
					while (!m_completedChunkRequestInfoList->empty ())
					{
						const ChunkRequestInfo& requestInfo = m_completedChunkRequestInfoList->front ();
						if (!requestInfo.m_chunk)
							break;

						delete requestInfo.m_chunk;

						m_completedChunkRequestInfoList->erase (m_completedChunkRequestInfoList->begin ());
					}
				}
			}
			break;

		case RTM2_rebuilding:
			{
				//-- This state means we have invalidated a portion of 
				//   the tree and are waiting for it to be rebuilt. If 
				//   we don't have any more pending requests and the 
				//   terrain level of detail is locked, unlock it to 
				//   allow new level of detail selection to take place.
				
				//-- Wait for all requests to finish
				bool finished = true;
				if (!m_pendingChunkRequestInfoMap->empty ())
				{
					//-- If we have any pending requests, we're not finished
					finished = false;
				}
				else
				{
					//-- We have no pending requests, but we may be working 
					//   on completed requests.
					ChunkRequestInfoList::const_iterator iter = m_completedChunkRequestInfoList->begin ();
					ChunkRequestInfoList::const_iterator end = m_completedChunkRequestInfoList->end ();
					for (; iter != end; ++iter)
					{
						const ChunkRequestInfo& requestInfo = *iter;
						if (!requestInfo.m_chunk)
						{
							finished = false;
							break;
						}
					}
				}

				if (finished)
				{
					//-- Verify that the number of completed requests is the 
					//   same as the number of chunks we wanted to rebuild
					DEBUG_FATAL (m_completedChunkRequestInfoList->size () != m_invalidateChunkRequestInfoList->size (), ("race condition detected in terrain generation thread: m_completedChunkRequestInfoList->size () != m_invalidateChunkRequestInfoList->size ()"));

					//-- Replace the portions of the tree that were just built
					ChunkRequestInfoList::const_iterator iter = m_completedChunkRequestInfoList->begin ();
					ChunkRequestInfoList::const_iterator end = m_completedChunkRequestInfoList->end ();
					for (; iter != end; ++iter)
					{
						const ChunkRequestInfo& requestInfo = *iter;
						NOT_NULL (requestInfo.m_chunk);

						//-- Find the node in the tree
						TerrainQuadTree::Node* const node = getChunkTree ()->findChunkNode (requestInfo.m_x, requestInfo.m_z, requestInfo.m_size);

						//-- Remove the existing chunk
						node->removeChunk (node->getChunk (), true);

						//-- Add the new chunk
						node->addChunk (requestInfo.m_chunk, requestInfo.m_size);
					}

					//-- Clear lists
					m_completedChunkRequestInfoList->clear ();
					m_invalidateChunkRequestInfoList->clear ();

					//-- We're done!  Set the state to normal and unlock the terrain level of detail creation
					m_requestThreadMode = RTM0_normal;
					m_lockTerrainLevelOfDetail = false;

					//-- Resnap all flora to terrain
					BoxExtent boxExtent;
					boxExtent.setMin (Vector::negativeMaxXYZ);
					boxExtent.setMax (Vector::maxXYZ);
					std::for_each (m_floraManagerList->begin (), m_floraManagerList->end (), VoidBindSecond (VoidMemberFunction (&ClientRadialFloraManager::markExtentDirty), &boxExtent));
				}
			}
			break;
		}
	}
	m_requestCriticalSection.leave ();
}

//-----------------------------------------------------------------

struct MapFinder : public std::unary_function <ClientProceduralTerrainAppearance::ChunkRequestInfoMap::value_type, bool>
{
	const ClientProceduralTerrainAppearance::ChunkRequestInfo * m;

	inline result_type operator () (const argument_type & rhs) const
	{
		return *m == (rhs.second);
	}
};

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::insertChunkCreationRequests (const ChunkRequestInfoList& requestInfoList)
{
	// todo: make non-multithread terrain queue up requests and amortize chunk creation over time in single thread
	DEBUG_FATAL (!ms_multiThreadedTerrainGeneration, ("insertChunkCreationRequests should only be called for multi-threaded terrain.\n"));

	int numberOfRequests = 0;

	m_requestCriticalSection.enter ();
	{
		int lowPriority = 9999999;

		ChunkRequestInfoList::const_iterator iter = requestInfoList.begin ();
		for (; iter != requestInfoList.end (); ++iter)
		{
			const ChunkRequestInfo& requestInfo = (*iter);
			const ChunkRequestInfoList::iterator completed_iter = std::find (m_completedChunkRequestInfoList->begin (), m_completedChunkRequestInfoList->end (), requestInfo);

			//-- if it is already on the completed set, skip it
			if (completed_iter != m_completedChunkRequestInfoList->end ())
			{
				continue;
			}

			//-- chunk is already in the tree, please don't build it again!
			if (getChunkTree ()->findChunk (requestInfo.m_x, requestInfo.m_z, requestInfo.m_size) != 0)
				continue;

			MapFinder mf;
			mf.m = &requestInfo;

			const ChunkRequestInfoMap::iterator piter = std::find_if (m_pendingChunkRequestInfoMap->begin (), m_pendingChunkRequestInfoMap->end (), mf);

			//-- request is already on the pending map.  bump up the priority
			if (piter != m_pendingChunkRequestInfoMap->end ())
			{
				m_pendingChunkRequestInfoMap->erase (piter);

				//-- make a copy
				ChunkRequestInfo newChunkRequestInfo = requestInfo;
				++newChunkRequestInfo.m_priority;
				lowPriority = std::min (lowPriority, newChunkRequestInfo.m_priority);
				IGNORE_RETURN (m_pendingChunkRequestInfoMap->insert (std::make_pair (newChunkRequestInfo.m_priority, newChunkRequestInfo)));
			}

			//-- simply add it to the pending set
			else
			{
				lowPriority = std::min (lowPriority, requestInfo.m_priority);
				IGNORE_RETURN (m_pendingChunkRequestInfoMap->insert (std::make_pair (requestInfo.m_priority, requestInfo)));
				++numberOfRequests;
			}
		}

		//-- prune the low priority requests from the list
		//-- todo: a better scheme for handling this... simply replace all requests with incoming?
		if (numberOfRequests && (static_cast<size_t>(numberOfRequests * 2) < m_pendingChunkRequestInfoMap->size ()))
			m_pendingChunkRequestInfoMap->erase (m_pendingChunkRequestInfoMap->begin (), m_pendingChunkRequestInfoMap->lower_bound (lowPriority));
	}

	// signal the worker thread that some new requests are waiting
	if (numberOfRequests)
		m_requestGate.open();

	m_requestCriticalSection.leave ();
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::insertChunkRebuildRequests (const ChunkRequestInfoList& requestInfoList)
{
	DEBUG_FATAL (!ms_multiThreadedTerrainGeneration, ("insertChunkRebuildRequests should only be called for multi-threaded terrain.\n"));

	m_requestCriticalSection.enter ();

		//-- put all of the requests on the pending list
		DEBUG_FATAL (!m_pendingChunkRequestInfoMap->empty (), ("race condition detected in terrain generation thread: !m_pendingChunkRequestInfoMap->empty ()"));

		ChunkRequestInfoList::const_iterator iter = requestInfoList.begin ();
		ChunkRequestInfoList::const_iterator end = requestInfoList.end ();
		for (; iter != end; ++iter)
		{
			const ChunkRequestInfo& requestInfo = *iter;
			m_pendingChunkRequestInfoMap->insert (std::make_pair (0, requestInfo));
		}

		//-- signal the worker thread that some new requests are waiting
		m_requestGate.open();

	m_requestCriticalSection.leave ();
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::invalidateRegion (const Rectangle2d& extent2d)
{
	m_invalidateRegionList->push_back (extent2d);
}

//-----------------------------------------------------------------

void ClientProceduralTerrainAppearance::clearInvalidRegionList ()
{
	//-- If the list is empty, we have nothing to do
	if (m_invalidateRegionList->empty ())
		return;

	//-- If we're locked, we should wait until we're not locked so we can process the list again
	if (m_lockTerrainLevelOfDetail)
		return;

	const Rectangle2d extent2d = m_invalidateRegionList->back ();
	m_invalidateRegionList->pop_back ();

	//-- 
	m_requestCriticalSection.enter ();

	//-- Lock the terrain because we need to rebuild what is dirty
	m_lockTerrainLevelOfDetail = true;

	//-- Set our mode to clearing existing requests (we need to clear the list before rebuilding what's dirty)
	DEBUG_FATAL (m_requestThreadMode != RTM0_normal, ("race condition detected in terrain generation thread: m_requestThreadMode != RTM0_normal"));
	m_requestThreadMode = RTM1_clearingRequests;

	//-- Clear out the pending request list
	m_pendingChunkRequestInfoMap->clear ();

	//-- Clear the list of nodes to rebuild within our extent
	DEBUG_FATAL (!m_invalidateChunkRequestInfoList->empty (), ("race condition detected in terrain generation thread: !m_invalidateChunkRequestInfoList->empty ()"));
	m_invalidateChunkRequestInfoList->clear ();
	{
		TerrainQuadTree::Iterator iter = getChunkTree ()->getIterator ();
		TerrainQuadTree::Node* node = 0;

		while ((node = iter.getCurNode ()) != 0)
		{
			const Chunk* const chunk = safe_cast<const Chunk*> (node->getChunk ());
			if (chunk)
			{
				const BoxExtent& boxExtent = chunk->getBoxExtent ();
				const Rectangle2d chunkExtent2d (boxExtent.getMin ().x, boxExtent.getMin ().z, boxExtent.getMax ().x, boxExtent.getMax ().z);
				if (extent2d.intersects (chunkExtent2d))
					m_invalidateChunkRequestInfoList->push_back (ChunkRequestInfo (node->getX (), node->getZ (), node->getSize (), 0));
			}

			IGNORE_RETURN (iter.descend ());
		}
	}

	ms_maximumNumberOfInvalidatedNodes = std::max (ms_maximumNumberOfInvalidatedNodes, static_cast<int> (m_invalidateChunkRequestInfoList->size ()));

	m_requestCriticalSection.leave ();

/*
	while (!m_invalidateNodeList->empty ())
	{
		TerrainQuadTree::Node* const node = m_invalidateNodeList->back ();
		m_invalidateNodeList->pop_back ();
		node->removeChunk (node->getChunk (), true);
	}

	//-- tell the terrain system to kick-off its rebuilding
	initializeLevelOfDetail (5);
	m_levelOfDetail->setDirty (true);
*/
}

//-------------------------------------------------------------------

