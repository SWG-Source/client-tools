//
// MayaHierarchy.h
// asommers 2001-01-31
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef MAYAHIERARCHY_H
#define MAYAHIERARCHY_H

//-------------------------------------------------------------------

class Messenger;

#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <string>

class IndexedTriangleList;
class MSelectionList;
class MFnSet;
class MFnDagNode;
class MString;
class SimplePathGraph;

namespace pobExportUtils
{
	struct Portal;
	struct PortalConnectivity;
	struct CellLight;
	struct Cell;

	typedef stdvector<IndexedTriangleList const *>::fwd PortalGeometryList;
	typedef stdvector<Portal>::fwd  PortalList;
	
	typedef stdvector<CellLight>::fwd    LightList;
	typedef stdvector<MString>::fwd      FloorList;
	typedef stdvector<Cell>::fwd         CellList;
}

using namespace pobExportUtils;
#include "maya/MDagPath.h"
#include "maya/MObject.h"
#include "maya/MObjectArray.h"
#include "maya/MString.h"

#include <vector>

// ----------------------------------------------------------------------

class MayaHierarchy
{
//the ExporterLog class needs access to the Node data to build it's log file
friend class ExporterLog;

public:

	enum Type
	{
		T_uninitialized,
		T_cmp,              // Component node
		T_lod,              // LOD node. May contain a test and write shape
		T_msh,              // Mesh node
		T_ext,              // External reference
		T_prt,              // Portal
		T_pls,              // Portal list
		T_cel,              // Cell
		T_pob,              // Portalled object
		T_collision,        // Collision group;
		T_floor,            // Floor node
		T_lightList,        // List of lights in a cell
		T_light,            // Light in a cell
		T_extent,           // Collision extent
		T_cylinder,         // Cylinder collision extent
		T_sphere,           // Sphere collision extent
		T_box,              // Box collision extent
		T_cmesh,            // Collision mesh

		T_COUNT
	};

public:

	struct Instance
	{
	public:

		MObject mayaObject;
		MString engineName;

	private:

		Instance (void);

	public:

		static Instance* create (void);
		~Instance (void);
		Instance& operator= (const Instance& rhs);
	};

public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Node
	{
	private:

		typedef stdvector<Node*>::fwd NodeList;

	private:

		const MayaHierarchy* m_hierarchy;
		std::string          m_hierarchyName;

		Type                 m_type;

		MDagPath             m_mayaDagPath;

		std::string          m_name;

		Node*                m_parent;

		NodeList             m_childList;

		mutable Sphere       m_sphere;

		int                  m_desiredOrder;
		int                  m_mayaChildIndex;	// The child index of this node in the Maya hierarchy.
		                                        // After sortNodes(), the node's child index and its mayaChildIndex will be different.

		bool                 m_instanced;
		bool                 m_canSeeWorldCell;

		Transform            m_transform;

		std::string          m_externalReferenceName;

		mutable Vector       m_minVector;
		mutable Vector       m_maxVector;

		int                  m_buildingCellIndex;    // If this node is a cell node, this is the index of which cell in the building it is.

		int                  m_buildingPortalIndex;  // If this node is a portal node, this is the index of which portal in the building it is.
		int                  m_cellPortalIndex;      // If this node is a portal node, this is the index of which portal in the cell it is.

		mutable Vector       m_cellPathNodePosition; // Where in each cell the path node for the building graph goes.

		IndexedTriangleList * m_portalGeometry;
		bool m_clockwise;

		PortalGeometryList m_portalGeometryList;

	private:

		void grow (void);

		bool writeCmp (bool publish) const;
		bool writeLod (bool publish) const;
		bool writeMsh (bool publish) const;
		bool writePob (bool publish) const;

		void computeMinMaxBoxVector () const;
		void computeSphere (void) const;
		bool getLodThresholds (real & outNear, real & outFar) const;
		bool shouldCollectHardpoints() const;

	private:

		Node (void);
		Node (const Node&);
		Node& operator= (const Node&);

	public:

		explicit Node (const MayaHierarchy* newHierarchy);
		virtual ~Node (void);

		// ----------

		Type             getType (void) const;
		void             setType (Type newType);

		bool             isAppearance ( void ) const;
		bool             isCollision  ( void ) const;

		// ----------

		void             setCanSeeWorldCell ();
		bool             getCanSeeWorldCell () const;

		MDagPath         getMayaDagPath (void) const;
		void             setMayaDagPath (MDagPath newMayaDagPath);

		bool             getMayaDagNodeFn ( MFnDagNode & outFn ) const;

		const char*      getName (void) const;
		bool             hasName (void) const;
		void             setName (const char* newName);

		MayaHierarchy const *   getHierarchy ( void ) const;

		const char*      getHierarchyName (void) const;
		void             setHierarchyName (const char* newHierarchyName);

		MString          getFilename (void) const;
		MString          getReferenceName (void) const;
		MString          getAppearanceReferenceName (void) const;
		MString          getPartFilename (void) const;

		const char*      getExternalReferenceName (void) const;
		void             setExternalReferenceName (const char* newExternalReferenceName);
		bool             hasExternalReferenceName (void) const;

		MString          getServerAppearanceTemplateReferenceName (void) const;
		MString          getServerAppearanceTemplateFilename (void) const;

		MString          getAptReferenceName (void) const;
		MString          getAptFilename (void) const;

		MString          getServerAptReferenceName ( void ) const;
		MString          getServerAptFilename ( void ) const;

		Node*            getParent (void);
		const Node*      getParent (void) const;
		void             setParent (Node* newParent);

		int              getNumberOfChildren (void) const;
		Node*            getChild (int index);
		void             setChild (int index, Node * child);
		const Node*      getChild (int index) const;
		void             addChild (Node* child);
		void             removeChild (const Node* child);
		void             removeAllChildren (void);
		void             swapChildren (int index1, int index2);
		void             gatherChildren(MSelectionList &sels) const;

		int              getDesiredOrder (void) const;
		void             setDesiredOrder (int newDesiredOrder);

		int              getMayaChildIndex (void) const;
		void             setMayaChildIndex (int newIndex);

		int              getBuildingCellIndex (void) const;
		void             setBuildingCellIndex (int newIndex);

		int              getBuildingPortalIndex (void) const;
		void             setBuildingPortalIndex (int newIndex);

		int              getCellPortalIndex (void) const;
		void             setCellPortalIndex (int newIndex);

		Vector const &   getCellPathNodePosition (void) const;
		void             setCellPathNodePosition (Vector const &position) const;

		void             setInstanced (bool newInstanced);
		bool             getInstanced (void) const;

		const Sphere&    getSphere (void) const;

		void             setTransform (const Transform& newTransform);
		const Transform& getTransform (void) const;

		virtual bool     write (bool publish) const;

		void             absorb(const Node * o);

		bool             findChildByType (Type type, Node const * & outNode) const;
		bool             findParentByType (Type type, Node const * & outNode) const;

		bool             validate (void) const;

		const Vector    &getMinBoxVector () const;
		const Vector    &getMaxBoxVector () const;

		//-- Used for portal nodes
		IndexedTriangleList const * getPortalGeometry() const;
		void setPortalGeometry(IndexedTriangleList * portalGeometry);
		bool getClockwise() const;
		void setClockwise(bool clockwise);

		//-- Used for pob nodes
		PortalGeometryList const & getPortalGeometryList() const;
		void setPortalGeometryList(PortalGeometryList const & portalGeometryList);
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct NodeInfo
	{
		NodeInfo() : m_type(T_uninitialized), m_index(-1), m_isShapeNode(false)
		{
		}

		Type	m_type;
		int		m_index;
		bool	m_isShapeNode;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	Node*           m_tree;
	MObject m_rootMayaObject;

//	char*           m_appearanceWriteDir;

	std::string     m_baseWriteDir;
	std::string     m_appearanceWriteDir;
	std::string     m_serverAppearanceWriteDir;
	std::string     m_collisionWriteDir;

	std::string     m_shaderTemplateWriteDir;
	std::string     m_shaderTemplateReferenceDir;
	std::string     m_effectReferenceDir;
	std::string     m_textureReferenceDir;
	std::string     m_textureWriteDir;

	std::string     m_cmpWriteDir;
	std::string     m_lodWriteDir;
	std::string     m_mshWriteDir;

	bool            m_hasPortal;

	stdvector<Instance*>::fwd m_instanceList;

	std::string     m_pobFileName;

private:

	// ----------
	// Functions for determining a node's type given it's name

	static bool    determineNodeType           ( Node * pNode, NodeInfo & nodeInfo );
	static bool    decodeNodeName              ( const char* name, NodeInfo& nodeInfo );
	static bool    decodeNameString            ( const char* name, NodeInfo& nodeInfo );

	// ----------
	
	bool    setupNode      ( Node * root, Node * parent );
	bool    createChildren ( Node * node );
	bool    setNodeTypeFromChildren ( Node * root );
	void    setChildOrderIndices ( Node * root );

	bool    populateTree (const MSelectionList &transformList);
	bool    sortNode (Node* root);
	bool    fixupNames (Node* root);
	Node *  promoteNode (Node* root);

	void    dump (const Node* root, const int indentLevel=0) const;
	bool    write (const Node* root, bool publish) const;

	void    addInstance (MObject mayaObject);

private:

	MayaHierarchy (void);
	MayaHierarchy (const MayaHierarchy&);
	MayaHierarchy& operator= (const MayaHierarchy&);

public:

	explicit MayaHierarchy (Messenger* newMessenger);
	~MayaHierarchy (void);

	void        setAppearanceWriteDir (const char* newAppearanceWriteDir);
	void        setShaderTemplateWriteDir (const char* newShaderTemplateWriteDir);
	void        setShaderTemplateReferenceDir (const char* newShaderTemplateReferenceDir);
	void        setEffectReferenceDir (const char* newEffectReferenceDir);
	void        setTextureReferenceDir (const char* newTextureReferenceDir);
	void        setTextureWriteDir (const char* newTextureWriteDir);

	const char* getAppearanceWriteDir (void) const;
	const char* getServerAppearanceWriteDir (void) const;
	const char* getAppearanceCmpWriteDir (void) const;
	const char* getAppearanceLodWriteDir (void) const;
	const char* getAppearanceMshWriteDir (void) const;
	const char* getAppearancePobWriteDir (void) const;
	const char* getShaderTemplateWriteDir (void) const;
	const char* getShaderTemplateReferenceDir (void) const;
	const char* getEffectReferenceDir (void) const;
	const char* getTextureReferenceDir (void) const;
	const char* getTextureWriteDir (void) const;
	const char* getCollisionWriteDir ( void ) const;

	void setPobFileName (const std::string &pobFileName);
	const std::string &getPobFileName (void) const;

	const char* getBaseName (void) const;
	const char* getBaseType (void) const;

	MObject const & getRootMayaObject() const;

	bool        build (const MSelectionList &transformList);

	void        dump (void) const;
	bool        write (bool publish) const;
	bool        writeClientApt ( Node const * node ) const;
	bool        writeServerApt ( Node const * node ) const;

	bool        writeServerAppearanceTemplates ( void ) const;
	bool        writeServerAppearanceTemplate ( Node const * node ) const;

	//-- public for private classes of MayaHierarchy
	bool        instanceWritten (MObject mayaObject) const;
	MString     getInstanceName (MObject mayaObject, const MString& engineName) const;

public:

	static bool        MayaTransform_to_Transform	( MObject & object, Transform & out );

	static bool        extractFloorName ( Node const * root, std::string & floorName );

	static const char* getTypeName (Type type);
	static const char* getSubdirectoryName (Type type);
	static const char* getExtension (Type type);

	static bool        testMatchingPolygons(std::vector<Vector> const & polygon1, std::vector<Vector> const & polygon2, float epsilon);
	static bool        testMatchingPortals(IndexedTriangleList const & portal1, IndexedTriangleList const & portal2, float epsilon);
	static int         findMatchingPortal(PortalGeometryList const & portalList, IndexedTriangleList const & portal);
	static IndexedTriangleList * extractPortalGeometry(Node const * portalNode);

	static bool        preprocessPobNode(Node * pobNode);
	static bool        shouldAllowNonPlanarPortal(Node const * node);
	static bool        isPortalNodeDisabled(Node const * node);
	static bool        isPortalNodePassable        ( Node const * node );
	static MString     extractDoorStyle            ( Node const * node );
	static SimplePathGraph * createBuildingPathGraph(CellList & cellList, PortalGeometryList const & portalList);

	static bool        convertCellPortalIndexToBuildingPortalIndex ( Node const * startNode, int cellPortalIndex, int & buildingPortalIndex );
};

//-------------------------------------------------------------------

#endif
