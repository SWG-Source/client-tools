//===================================================================
//
// WorldSnapshotReaderWriter.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotReaderWriter_H
#define INCLUDED_WorldSnapshotReaderWriter_H

//===================================================================

#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include <string>

class CrcString;
class Iff;
class MemoryBlockManager;
class SpatialSubdivisionHandle;

//===================================================================

class WorldSnapshotReaderWriter
{
public:

	class Node
	{
		friend class WorldSnapshotReaderWriter;

		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	public:

		static void      install ();

		static float     getDetailLevelBias ();
		static void      setDetailLevelBias (float detailLevelBias);

		typedef void (*DetailLevelChangedFunction) ();
		static void setDetailLevelChangedFunction (DetailLevelChangedFunction detailLevelChangedFunction);

	public:

		Node ();
		~Node ();

		void             setDeleted ();
		void             setNetworkIdInt (int64 networkIdInt);
		void             setContainedByNetworkIdInt (int64 containedByNetworkIdInt);
		void             setObjectTemplateNameIndex (int objectTemplateNameIndex);
		void             setCellIndex (int cellIndex);
		void             setTransform_p (const Transform& transform_p); 
		void             setRadius (float radius);
		void             setPortalLayoutCrc (uint32 portalLayoutCrc);
		void             setEventName(const std::string & eventName);

		bool             isDeleted () const;
		int64            getNetworkIdInt () const;
		int64            getContainedByNetworkIdInt () const;
		int              getObjectTemplateNameIndex () const;
		int              getCellIndex () const;
		const Transform& getTransform_p () const;
		float            getRadius () const;
		uint32           getPortalLayoutCrc () const;
		const std::string& getEventName() const; 

		int              getNumberOfNodes () const;
		const Node*      getNode (int nodeIndex) const;
		const Node*      getParent () const;

		int              getTotalNumberOfNodes () const;

		const Sphere     getSphere () const;
		SpatialSubdivisionHandle* getSpatialSubdivisionHandle () const;
		void             setSpatialSubdivisionHandle (SpatialSubdivisionHandle* spatialSubdivisionHandle) const;

		//-- used during run-time operation
		void             computeDistanceSquaredTo (const Vector& position_p) const;
		float            getDistanceSquaredTo () const;

		bool             isInWorld () const;
		void             addToWorld () const;
		void             removeFromWorld () const;

	private:

		static void      remove ();

	private:

		void             setParent (Node* node);
		void             addNode (Node* node);

		void             load (Iff& iff);
		void             save (Iff& iff) const;
		// Goal B Wave 3 (2026-07-18): save skipping DELETED children recursively
		// (tombstones from removeNode would otherwise serialize with zeroed ids)
		void             saveFiltered (Iff& iff) const;

		void             load_0000 (Iff& iff);

	private:

		Node (const Node&);
		Node& operator= (const Node&);

	private:

		bool             m_deleted;
		int64            m_networkIdInt;
		int64            m_containedByNetworkIdInt;
		int              m_objectTemplateNameIndex;
		int              m_cellIndex;
		Transform        m_transform_p;
		float            m_radius;
		uint32           m_portalLayoutCrc;
		Node*            m_parent;
		std::string      m_eventName;

		typedef stdvector<Node*>::fwd NodeList;
		NodeList*        m_nodeList;

		mutable SpatialSubdivisionHandle* m_spatialSubdivisionHandle;
		mutable float m_distanceSquaredTo;

		mutable bool m_inWorld;
	};

public:

	WorldSnapshotReaderWriter ();
	~WorldSnapshotReaderWriter ();

	bool             load (const char* filename);
	void             load (Iff& iff);
	bool             save (const char* filename) const;
	void             save (Iff& iff) const;

	// Goal B Wave 3 (2026-07-18): filtered save -- the editor's Save writes
	// authored content only. Skips DELETED (tombstoned) nodes recursively and
	// any TOP-LEVEL node for which includeTopLevelNode returns false (the
	// caller's provenance filter, e.g. buildout-row exclusion). The OTNL
	// template-name table is written WHOLE (excluded nodes may leave unused
	// names; surviving nodes' indices stay valid without a remap). Additive
	// API -- no layout change; Node::save/saveFiltered remain the sole format
	// writers.
	// The callback receives the NODE POINTER, not the id -- provenance must be
	// identity-keyed (a buildout row and an authored node can share an id; the
	// 2026-07-18 self-test caught the id-keyed filter refusing every
	// buildout-planet save).
	typedef bool (*IncludeTopLevelNodeFunction) (const Node* node, void* context);
	bool             saveFiltered (const char* filename, IncludeTopLevelNodeFunction includeTopLevelNode, void* context) const;
	void             saveFiltered (Iff& iff, IncludeTopLevelNodeFunction includeTopLevelNode, void* context) const;

	// CONSULT-60: incremental load. beginIncrementalLoad enters the WSNP/0001/
	// NODS forms (returns false if there is nothing to parse); each
	// stepIncrementalLoad call parses whole top-level node subtrees under
	// budgetMs (<=0 = run to completion), inserting each subtree into the
	// networkId map as it lands, and returns true once the OTNL table is read
	// and all forms are exited. The caller owns the Iff and must keep it alive
	// between calls. The synchronous load(filename) path is unchanged.
	bool             beginIncrementalLoad (Iff& iff);
	bool             stepIncrementalLoad (Iff& iff, float budgetMs);

	//-- creation interface
	void             clear ();
	const Node*      addObject (int64 networkIdInt, int64 containedByNetworkIdInt, CrcString const &objectTemplateName, int cellIndex, const Transform& transform_p, float radius, uint32 portalLayoutCrc, const std::string & eventName = "");
	int              internObjectTemplateName (CrcString const &objectTemplateName);   // find-or-append in the OTNL table; returns the index (the addObject intern path, exposed for in-place node re-pointing -- v23 wsSetNodeTemplateName)

	//-- query interface
	int              getNumberOfNodes () const;
	const Node*      getNode (int nodeIndex) const;
	int              getNumberOfObjectTemplateNames () const;
	const char*      getObjectTemplateName (int objectTemplateNameIndex) const;

	int              getTotalNumberOfNodes () const;

	Node*            find (int64 networkIdInt);
	const Node*      find (int64 networkIdInt) const;
	void             removeNode (int64 networkIdInt);

	void             removeFromWorld ();

private:

	void             load_0001 (Iff& iff);
	void             insertSubtreeIntoNetworkIdMap (Node* root);

	void             preSave () const;

private:

	WorldSnapshotReaderWriter (const WorldSnapshotReaderWriter&);
	WorldSnapshotReaderWriter& operator= (const WorldSnapshotReaderWriter&);

private:

	typedef stdvector<Node*>::fwd NodeList;
	NodeList* const m_nodeList;

	typedef stdvector<char*>::fwd ObjectTemplateNameList;
	ObjectTemplateNameList* const m_objectTemplateNameList;

	typedef stdhash_map<uint32, uint> ObjectTemplateCrcMap;
	ObjectTemplateCrcMap* const m_objectTemplateCrcMap; // map from crc to index in name list

	typedef stdmap<int64, Node*>::fwd NetworkIdNodeMap;
	NetworkIdNodeMap* const m_networkIdNodeMap;
};

//===================================================================

#endif
