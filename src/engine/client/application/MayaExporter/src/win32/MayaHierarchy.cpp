//
// MayaHierarchy.cpp
// asommers 2001-01-31
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstMayaExporter.h"
#include "MayaHierarchy.h"

#include "ExporterLog.h"
#include "MayaIndexedTriangleList.h"
#include "MayaMeshReader.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "StaticMeshBuilder.h"

#include "CollisionBuilder.h"
#include "ComponentAppearanceBuilder.h"
#include "DetailAppearanceBuilder.h"

#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Quaternion.h"

#include "sharedObject/Hardpoint.h"

#include "sharedPathfinding/PathEdge.h"
#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/SimplePathGraph.h"

#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnMesh.h"
#include "maya/MFnAmbientLight.h"
#include "maya/MFnDirectionalLight.h"
#include "maya/MFnPointLight.h"
#include "maya/MFnTransform.h"
#include "maya/MMatrix.h"
#include "maya/MPlug.h"

#include "maya/MSelectionList.h"

#include <algorithm>
#include <stdlib.h>

//===================================================================

class PortalPropertyTemplate
{
public:
	static bool extractPortalLayoutCrc(const char *fileName, uint32 &portalLayoutCrc);
};

//===================================================================

const Tag TAG_CRC  = TAG3(C,R,C);

const Tag TAG_CELL = TAG(C,E,L,L);
const Tag TAG_CELS = TAG(C,E,L,S);
const Tag TAG_LGHT = TAG(L,G,H,T);
const Tag TAG_PRTL = TAG(P,R,T,L);
const Tag TAG_PRTO = TAG(P,R,T,O);
const Tag TAG_PRTS = TAG(P,R,T,S);
const Tag TAG_QUAD = TAG(Q,U,A,D);
const Tag TAG_VRTX = TAG(V,R,T,X);
const Tag TAG_APT  = TAG3(A,P,T);
const Tag TAG_NULL = TAG(N,U,L,L);

//===================================================================
//
//
Messenger* messenger;

//===================================================================
//
//
static bool isDigit (const char ch)
{
	return ch >= '0' && ch <= '9';
}

static bool isLetter ( const char ch )
{
	if( (ch >= 'a') && (ch <= 'z') ) return true;
	if( (ch >= 'A') && (ch <= 'Z') ) return true;

	return false;
}

//-------------------------------------------------------------------

static void convertToLowerCase (char* name)
{
	if (!name)
		return;

	const int length = istrlen (name);
	int i;

	for (i = 0; i < length; ++i)
		name [i] = static_cast<char> (tolower (name [i]));
}

// ----------------------------------------------------------------------

bool	stringEndsWith	( std::string const & string, std::string const & ending )
{
	if(string.length() < ending.length()) return false;

	return string.compare( string.length() - ending.length(), ending.length(), ending ) == 0;
}

void removeEndNumbers ( std::string & string )
{
	uint length = string.length();

	const char * chars = string.c_str();

	uint i;
	for(i = length - 1; i >= 0; i--)
	{
		if(!isDigit(chars[i])) break;
	}

	string.resize(i+1);
}

//-------------------------------------------------------------------
// Returns true if the name ends in "radar"

static bool isRadarShape (const std::string& name)
{
	std::string temp (name);
	removeEndNumbers (temp);

	return stringEndsWith (temp, "radar");
}

//-------------------------------------------------------------------
// Returns true if the name ends in "test"

static bool isTestShape ( std::string const & name )
{
	std::string temp = name;

	removeEndNumbers(temp);

	return stringEndsWith(temp,"test");
}

//-------------------------------------------------------------------
// Returns true if the name ends in "write"

static bool isWriteShape ( std::string const & name )
{
	std::string temp = name;

	removeEndNumbers(temp);

	return stringEndsWith(temp,"write");
}

//-------------------------------------------------------------------
// Write out a tiny redirector file

static void writeAptFile (MString const & aptFilename, MString const & redirectFilename)
{
	Iff iff(128);
	iff.insertForm(TAG_APT);
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_NAME);
				iff.insertChunkString(redirectFilename.asChar());
			iff.exitChunk(TAG_NAME);
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_APT);

	iff.write(aptFilename.asChar());
}

//-------------------------------------------------------------------
// Tests indexedTriangleList for any non-planar geometry and warns accordingly

static void validateCoplanar(IndexedTriangleList const & indexedTriangleList, char const * const nodeName)
{
	std::vector<Vector> const & vertexList = indexedTriangleList.getVertices();
	uint const numberOfVertices = vertexList.size();

	if (numberOfVertices < 3)
		MESSENGER_LOG_WARNING(("Portal %s with fewer than 3 vertices\n", nodeName));

	Plane p(vertexList[0], vertexList[1], vertexList[2]);
	for (uint i = 3; i < numberOfVertices; ++i)
	{
		Vector const position = vertexList[i];
		float const distance = p.computeDistanceTo(position);

		if (abs(distance) > 0.001f)
			MESSENGER_LOG_WARNING(("Portal %s is not planar, vertex %d (%1.2f, %1.2f, %1.2f) is %8.6f from the plane\n", nodeName, static_cast<int>(i), position.x, position.y, position.z, distance));
	}
}

//===================================================================
// Exporter node typeinfo table

struct SNodeTypeInfo
{
	MayaHierarchy::Type     m_typeID;
	const char *            m_name;
	const char *            m_subdirectory;
	const char *            m_extension;
};

SNodeTypeInfo	gNodeTypeInfoTable [MayaHierarchy::T_COUNT] = 
{
	{ MayaHierarchy::T_uninitialized,   "uninitialized",        "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_cmp,             "ComponentAppearance",  "component/",      ".cmp"       },
	{ MayaHierarchy::T_lod,             "DetailAppearance",     "lod/",            ".lod"       },
	{ MayaHierarchy::T_msh,             "MeshAppearance",       "mesh/",           ".msh"       },
	{ MayaHierarchy::T_ext,             "ExternalReference",    "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_prt,             "Portal",               "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_pls,             "PortalList",           "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_cel,             "Cell",                 "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_pob,             "PortalObject",         "portal/",         ".pob"       },
	{ MayaHierarchy::T_collision,       "Collision",            "collision/",      "DONOTUSE"   },
	{ MayaHierarchy::T_floor,           "Floor",                "collision/",      ".flr"       },
	{ MayaHierarchy::T_lightList,       "LightList",            "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_light,           "Light",                "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_extent,          "Extent",               "collision/",      ".ext"       },
	{ MayaHierarchy::T_cylinder,        "Cylinder",             "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_sphere,          "Sphere",               "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_box,             "Box",                  "DONOTUSE",         "DONOTUSE"  },
	{ MayaHierarchy::T_cmesh,           "CMesh",                "DONOTUSE",         "DONOTUSE"  }
};

// ----------------------------------------------------------------------
// Table for decoding node names of the form <string> or <string><number>

struct SDecodeEntry
{
	const char *        m_name;
	MayaHierarchy::Type m_type;
	int                 m_index;
};

const int nDecodeTable_Size = 21;

SDecodeEntry	gDecodeTable[ nDecodeTable_Size ] = 
{
	// Nodes with hardcoded names

	{ "collision",  MayaHierarchy::T_collision,     0 },
	{ "portals",    MayaHierarchy::T_pls,           0 },
	{ "mesh",       MayaHierarchy::T_msh,           1 },
	{ "radar",      MayaHierarchy::T_lod,           1000 },
	{ "test",       MayaHierarchy::T_lod,           1000 },
	{ "write",      MayaHierarchy::T_lod,           1000 },
	{ "lights",     MayaHierarchy::T_lightList,     2 },

	// Nodes in <string><number> format

	{ "l",          MayaHierarchy::T_lod,           -1 },
	{ "c",          MayaHierarchy::T_cmp,           -1 },
	{ "p",          MayaHierarchy::T_prt,           -1 },
	{ "r",          MayaHierarchy::T_cel,           -1 },
	{ "floor",      MayaHierarchy::T_floor,          0 },
	{ "extent",     MayaHierarchy::T_extent,         0 },
	{ "cylinder",   MayaHierarchy::T_cylinder,       0 },
	{ "sphere",     MayaHierarchy::T_sphere,         0 },
	{ "cube",       MayaHierarchy::T_box,            0 },
	{ "pcylinder",  MayaHierarchy::T_cylinder,       0 },   // cylinder
	{ "psphere",    MayaHierarchy::T_sphere,         0 },   // sphere
	{ "pcube",      MayaHierarchy::T_box,            0 },   // box
	{ "pmesh",      MayaHierarchy::T_cmesh,          0 },
	{ "cmesh",      MayaHierarchy::T_cmesh,          0 },
};

//===================================================================
//
//
MayaHierarchy::Node::Node (const MayaHierarchy* newHierarchy) :
	m_hierarchy (newHierarchy),
	m_hierarchyName (),
	m_type (T_uninitialized),
	m_mayaDagPath (),
	m_name (),
	m_parent (0),
	m_childList (),
	m_desiredOrder (0),
	m_mayaChildIndex(-1),
	m_instanced (false),
	m_canSeeWorldCell (false),
	m_transform (),
	m_externalReferenceName (),
	m_minVector (Vector::maxXYZ),
	m_maxVector (Vector::negativeMaxXYZ),
	m_buildingCellIndex (0),
	m_buildingPortalIndex (0),
	m_cellPortalIndex (0),
	m_cellPathNodePosition (),
	m_portalGeometry(0),
	m_clockwise(true),
	m_portalGeometryList()
{
}

//-------------------------------------------------------------------
	
MayaHierarchy::Node::~Node ()
{
	for (uint i = 0; i < m_childList.size(); ++i)
		delete m_childList[i];

	if (m_portalGeometry)
		delete m_portalGeometry;
}

//-------------------------------------------------------------------

MayaHierarchy::Type MayaHierarchy::Node::getType () const
{
	return m_type;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setType (MayaHierarchy::Type newType)
{
	m_type = newType;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::isAppearance() const
{
	if(getType() == T_cmp) return true;
	if(getType() == T_lod) return true;
	if(getType() == T_msh) return true;

	return false;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::isCollision() const
{
	if(getType() == T_extent) return true;
	if(getType() == T_collision) return true;
	if(getType() == T_floor) return true;

	return false;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setCanSeeWorldCell ()
{
	m_canSeeWorldCell = true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::getCanSeeWorldCell () const
{
	return m_canSeeWorldCell;
}

//-------------------------------------------------------------------

MDagPath MayaHierarchy::Node::getMayaDagPath () const
{
	return m_mayaDagPath;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::Node::getMayaDagNodeFn ( MFnDagNode & outFn ) const
{
	MStatus status;

	MDagPath dagPath = getMayaDagPath ();

	MObject object = dagPath.node (&status);
	
	if (!status)
		return false;

	status = outFn.setObject(object);

	if (!status)
		return false;

	return true;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setMayaDagPath (MDagPath newMayaDagPath)
{
	m_mayaDagPath = newMayaDagPath;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::Node::getName () const
{
	return m_name.c_str();
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::hasName () const
{
	return !m_name.empty();
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setName (const char* newName)
{
	NOT_NULL(newName);

	m_name = newName;

	convertToLowerCase(&(*m_name.begin()));
}

//-------------------------------------------------------------------

const char* MayaHierarchy::Node::getExternalReferenceName () const
{
	return m_externalReferenceName.c_str();
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::hasExternalReferenceName () const
{
	return !m_externalReferenceName.empty();
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setExternalReferenceName (const char* newExternalReferenceName)
{
	NOT_NULL(newExternalReferenceName);

	m_externalReferenceName = newExternalReferenceName;

	convertToLowerCase(&(*m_externalReferenceName.begin()));
}

//-------------------------------------------------------------------

MayaHierarchy const *   MayaHierarchy::Node::getHierarchy () const
{
	return m_hierarchy;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::Node::getHierarchyName () const
{
	return m_hierarchyName.c_str();
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setHierarchyName (const char* newHierarchyName)
{
	NOT_NULL(newHierarchyName);

	m_hierarchyName = newHierarchyName;

	convertToLowerCase(&(*m_hierarchyName.begin()));
}

// ----------------------------------------------------------------------

MString MayaHierarchy::Node::getReferenceName () const
{
	MString filename;

	if(getParent() == NULL)
	{
		// Root nodes only go in a subdirectory if they're not pobs
		// and we're using APTs

		if (getType() != T_pob)
		{
			filename += getHierarchy()->getSubdirectoryName (getType());
		}
	}
	else
	{
		// Child nodes always go in a subdirectory
		filename += getHierarchy()->getSubdirectoryName (getType());
	}

	filename += getName();
	filename += getHierarchy()->getExtension (getType());

	return filename;
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getAppearanceReferenceName () const
{
	return MString (SetDirectoryCommand::getDirectoryString (APPEARANCE_REFERENCE_DIR_INDEX)) + getReferenceName ();
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getFilename () const
{
	return MString(getHierarchy()->getAppearanceWriteDir()) + getReferenceName();
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getServerAppearanceTemplateReferenceName () const
{
	return MString (SetDirectoryCommand::getDirectoryString (APPEARANCE_REFERENCE_DIR_INDEX)) + MString ("ssa/") + getName() + MString(".ssa");
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getServerAppearanceTemplateFilename () const
{
	return MString(getHierarchy()->getServerAppearanceWriteDir()) + MString ("ssa\\") + getName() + MString(".ssa");
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getAptReferenceName () const
{
	return MString (SetDirectoryCommand::getDirectoryString (APPEARANCE_REFERENCE_DIR_INDEX)) + MString(getName()) + MString(".apt");
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getAptFilename () const
{
	return MString(getHierarchy()->getAppearanceWriteDir()) + MString(getName()) + MString(".apt");
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getServerAptReferenceName () const
{
	return getAptReferenceName();
}

//-------------------------------------------------------------------

MString MayaHierarchy::Node::getServerAptFilename () const
{
	return MString(getHierarchy()->getServerAppearanceWriteDir()) + MString(getName()) + MString(".apt");
}

// ----------------------------------------------------------------------
// Get the filename that this node should be referred to by if it's part
// of a component appearance

MString MayaHierarchy::Node::getPartFilename () const
{
	MString name = getReferenceName();

	//-- is the child an instance?
	if (getInstanced ())
	{
		MStatus status;

		//-- get the node from the path
		MDagPath dagPath = getMayaDagPath ();

		//-- get the object from the node
		MObject mayaObject = dagPath.node (&status);
		MESSENGER_REJECT(!status,("Could not get component part filename\n"));

		name = getHierarchy()->getInstanceName (mayaObject, name);
	}

	//-- is the child an external reference?
	if (hasExternalReferenceName ())
	{
		name = getExternalReferenceName ();
	}

	return name;
}

//-------------------------------------------------------------------

int MayaHierarchy::Node::getNumberOfChildren () const
{
	return m_childList.size();
}

//-------------------------------------------------------------------

MayaHierarchy::Node* MayaHierarchy::Node::getChild (int index)
{
	return m_childList [index];
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setChild (int index, Node * child)
{
	m_childList [index] = child; 
	child->m_parent = this;
}

//-------------------------------------------------------------------

const MayaHierarchy::Node* MayaHierarchy::Node::getChild (int index) const
{
	return m_childList [index];
}

//-------------------------------------------------------------------

MayaHierarchy::Node* MayaHierarchy::Node::getParent ()
{
	return m_parent;
}

//-------------------------------------------------------------------

const MayaHierarchy::Node* MayaHierarchy::Node::getParent () const
{
	return m_parent;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setParent (Node* newParent)
{
	m_parent = newParent;
}

//-------------------------------------------------------------------

int MayaHierarchy::Node::getDesiredOrder () const
{
	return m_desiredOrder;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setDesiredOrder (int newDesiredOrder)
{
	m_desiredOrder = newDesiredOrder;
}

// ----------------------------------------------------------------------

int MayaHierarchy::Node::getMayaChildIndex () const
{
	return m_mayaChildIndex;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setMayaChildIndex (int newIndex)
{
	m_mayaChildIndex = newIndex;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::addChild (Node* child)
{
	child->setParent (this);

	m_childList.push_back(child);
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::removeChild (const Node* child)
{
	NodeList::iterator it = std::remove(m_childList.begin(), m_childList.end(), child);

	m_childList.erase(it,m_childList.end());
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::removeAllChildren ()
{
	m_childList.clear();
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::swapChildren (int index1, int index2)
{
	uint a = static_cast<uint>(index1);
	uint b = static_cast<uint>(index2);

	DEBUG_FATAL (a < 0 || a >= m_childList.size(), ("index1 out of range"));
	DEBUG_FATAL (b < 0 || b >= m_childList.size(), ("index2 out of range"));

	std::swap (m_childList[a], m_childList[b]);
}

// ----------------------------------------------------------------------

int MayaHierarchy::Node::getBuildingCellIndex (void) const
{
	return m_buildingCellIndex;
}

void MayaHierarchy::Node::setBuildingCellIndex (int newIndex)
{
	m_buildingCellIndex = newIndex;
}

int MayaHierarchy::Node::getBuildingPortalIndex (void) const
{
	return m_buildingPortalIndex;
}

void MayaHierarchy::Node::setBuildingPortalIndex (int newIndex)
{
	m_buildingPortalIndex = newIndex;
}

int MayaHierarchy::Node::getCellPortalIndex (void) const
{
	return m_cellPortalIndex;
}

void MayaHierarchy::Node::setCellPortalIndex (int newIndex)
{
	m_cellPortalIndex = newIndex;
}

Vector const & MayaHierarchy::Node::getCellPathNodePosition ( void ) const
{
	return m_cellPathNodePosition;
}

void MayaHierarchy::Node::setCellPathNodePosition ( Vector const & position ) const
{
	m_cellPathNodePosition = position;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setInstanced (bool newInstanced)
{
	m_instanced = newInstanced;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::getInstanced () const
{
	return m_instanced;
}

//-------------------------------------------------------------------

const Sphere& MayaHierarchy::Node::getSphere () const
{
	return m_sphere;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setTransform (const Transform& newTransform)
{
	m_transform = newTransform;
}

//-------------------------------------------------------------------

const Transform& MayaHierarchy::Node::getTransform () const
{
	return m_transform;
}

//-------------------------------------------------------------------

IndexedTriangleList const * MayaHierarchy::Node::getPortalGeometry() const
{
	return m_portalGeometry;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setPortalGeometry(IndexedTriangleList * const portalGeometry)
{
	m_portalGeometry = portalGeometry;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::getClockwise() const
{
	return m_clockwise;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setClockwise(bool const clockwise)
{
	m_clockwise = clockwise;
}

//-------------------------------------------------------------------

PortalGeometryList const & MayaHierarchy::Node::getPortalGeometryList() const
{
	return m_portalGeometryList;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::setPortalGeometryList(PortalGeometryList const & portalGeometryList)
{
	m_portalGeometryList = portalGeometryList;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::writeCmp ( bool publish ) const
{
	if (getInstanced ())
	{
		MStatus status;

		//-- get the node from the path
		MDagPath dagPath = getMayaDagPath ();

		//-- get the object from the node
		MObject mayaObject = dagPath.node (&status);
		if (!status)
			return false;
	}

	//-- Combine all children min max vectors to generate this item's min max box vectors.
	computeMinMaxBoxVector ();

	if(publish)
	{
		//-- create the iff

		ComponentAppearanceBuilder builder(getHierarchy()->getRootMayaObject(), m_minVector, m_maxVector);

		IndexedTriangleList* radarShape = 0;

		for(int i = 0; i < getNumberOfChildren (); i++)
		{
			Node const * child = getChild(i);

			//-- is the child a radar shape?
			if (child->getType () == T_msh && isRadarShape (child->getName ()))
			{
				MayaIndexedTriangleList mayaIndexedTriangleList (child->getMayaDagPath ());

				if (mayaIndexedTriangleList.getScale () != Vector::xyz111)
					MESSENGER_LOG_WARNING (("Non-unit scale found for radar shape %s\n", child->getHierarchyName ()));

				MESSENGER_REJECT (radarShape, ("found more than one radar shape for node %s\n", getName ()));
				radarShape = mayaIndexedTriangleList.createIndexedTriangleList ();

				continue;
			}

			if( child->isCollision() )
			{
				builder.addCollisionNode(child);
				continue;
			}
			else if( child->isAppearance() )
			{
				builder.addPart( child->getPartFilename().asChar(), child->getTransform() );
			}
			else
			{
				MESSENGER_REJECT(true,("Node found in component group that's not an appearance or a collision group\n"));
				return false;
			}
		}

		if (radarShape)
		{
			builder.addRadarShape (radarShape);

			delete radarShape;
			radarShape = 0;
		}

		//-- Hardpoints should only be added to the root appearance template
		if (shouldCollectHardpoints())
			builder.collectHardpoints();

		Iff iff(1024);
		builder.write(iff);

		MString writeName = getFilename();
		ExporterLog::addClientDestinationFile(writeName.asChar());
		return iff.write (writeName.asChar());
	}
	return true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::Node::getLodThresholds ( real & outNear, real & outFar ) const
{
	//-- get the node from the path

	MDagPath dagPath = getParent()->getMayaDagPath ();

	//-- get the object from the node
	MStatus status;
	MObject mayaObject = dagPath.node (&status);
	MESSENGER_REJECT_STATUS (!status, ("could not get maya object from child dag path\n"));
	MESSENGER_REJECT (mayaObject.apiType () != MFn::kLodGroup, ("found lod node %s that is not a maya lod group\n", mayaObject.apiTypeStr ()));

	//-- see if we have a threshold attribute
	MFnDependencyNode fnDepNode (mayaObject, &status);
	MESSENGER_REJECT_STATUS (!status, ("failed to assign maya lod group object to fnDepNode,\"%s\"\n", status.errorString ().asChar()));

	MPlug threshold = fnDepNode.findPlug ("threshold", &status);
	MESSENGER_REJECT_STATUS (!status, ("could not get threshold multi-attribute from maya lod group\n"));
	MESSENGER_REJECT (!threshold.isArray (), ("threshold is not a multi-attribute"));
	
	real nearDistance = 0.0f;
	real farDistance = 1000.0f;

	int farIndex = getMayaChildIndex();
	int nearIndex = farIndex - 1;

	if(nearIndex >= 0)
	{
		MPlug element = threshold.elementByPhysicalIndex (nearIndex, &status);

		if(status)
		{
			status = element.getValue (nearDistance);
			MESSENGER_REJECT_STATUS (!status, ("could not get threshold element value %i\n", farIndex));
		}
	}

	if(farIndex >= 0)
	{
		MPlug element = threshold.elementByPhysicalIndex (farIndex, &status);

		if(status)
		{
			status = element.getValue (farDistance);
			MESSENGER_REJECT_STATUS (!status, ("could not get threshold element value %i\n", farIndex));
		}
		else
			farDistance = std::max(nearDistance * 2.f, 1000.f);
	}

	outNear = nearDistance;
	outFar = farDistance;

	return true;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::Node::writeLod (bool publish) const
{
	if (getInstanced ())
	{
		MStatus status;

		//-- get the node from the path
		MDagPath dagPath = getMayaDagPath ();

		//-- get the object from the node
		MObject mayaObject = dagPath.node (&status);
		if (!status)
			return false;
	}

	//-- see if our pivot point needs to be 
	bool usePivotPoint = false;
	bool disableLodCrossFade = false;
	{
		MStatus status;

		MDagPath dagPath = getMayaDagPath ();

		MObject mayaObject = dagPath.node (&status);
		if (status)
		{
			MFnDagNode dagNode (dagPath, &status);
			if (status)
			{
				dagNode.attribute ("usePivotPoint", &status);
				if (status == MS::kSuccess)
				{
					usePivotPoint = true;
				}

				dagNode.attribute ("disableLodCrossFade", &status);
				if (status == MS::kSuccess)
				{
					disableLodCrossFade = true;
				}
			}
		}
	}

	//-- Combine all children min max vectors to generate this item's min max box vectors.
	computeMinMaxBoxVector ();

	DetailAppearanceBuilder builder(getHierarchy()->getRootMayaObject(), usePivotPoint, disableLodCrossFade, m_minVector, m_maxVector);

	IndexedTriangleList* radarShape = 0;
	IndexedTriangleList* testShape  = 0;
	IndexedTriangleList* writeShape = 0;

	int i;
	for (i = 0; i < getNumberOfChildren (); ++i)
	{
		//-- get the name of the child
		const Node* child = getChild (i);
		MString childName = child->getReferenceName();

		//-- is the child an instance?
		if (child->getInstanced ())
		{
			MStatus status;

			//-- get the node from the path
			MDagPath dagPath = child->getMayaDagPath ();

			//-- get the object from the node
			MObject mayaObject = dagPath.node (&status);
			if (!status)
				return false;

			childName = m_hierarchy->getInstanceName (mayaObject, childName);
		}

		//-- is the child an external reference?
		if (child->hasExternalReferenceName ())
			childName = child->getExternalReferenceName ();

		//-- is the child a radar shape?
		if (child->getType () == T_msh && isRadarShape (child->getName ()))
		{
			MayaIndexedTriangleList mayaIndexedTriangleList (child->getMayaDagPath ());

			if (mayaIndexedTriangleList.getScale () != Vector::xyz111)
				MESSENGER_LOG_WARNING (("Non-unit scale found for radar shape %s\n", child->getHierarchyName ()));

			MESSENGER_REJECT (radarShape, ("found more than one radar shape for node %s\n", getName ()));
			radarShape = mayaIndexedTriangleList.createIndexedTriangleList ();

			continue;
		}

		//-- is the child a test shape?
		if (child->getType () == T_msh && isTestShape (child->getName ()))
		{
			MayaIndexedTriangleList mayaIndexedTriangleList (child->getMayaDagPath ());

			if (mayaIndexedTriangleList.getScale () != Vector::xyz111)
				MESSENGER_LOG_WARNING (("Non-unit scale found for test shape %s\n", child->getHierarchyName ()));

			MESSENGER_REJECT (testShape, ("found more than one test shape for node %s\n", getName ()));
			testShape = mayaIndexedTriangleList.createIndexedTriangleList ();

			continue;
		}

		//-- is the child a write shape?
		if (child->getType () == T_msh && isWriteShape (child->getName ()))
		{
			MayaIndexedTriangleList mayaIndexedTriangleList (child->getMayaDagPath ());

			if (mayaIndexedTriangleList.getScale () != Vector::xyz111)
				MESSENGER_LOG_WARNING (("Non-unit scale found for write shape %s\n", child->getHierarchyName ()));

			MESSENGER_REJECT (writeShape, ("found more than one write shape for node %s\n", getName ()));
			writeShape = mayaIndexedTriangleList.createIndexedTriangleList ();

			continue;
		}

		if( child->isCollision() )
		{
			builder.addCollisionNode(child);
			continue;
		}

		real nearDistance;
		real farDistance;

		child->getLodThresholds(nearDistance,farDistance);

		builder.addAppearance (i, childName.asChar(), nearDistance, farDistance);
	}

	if (radarShape)
	{
		builder.addRadarShape (radarShape);

		delete radarShape;
		radarShape = 0;
	}

	if (testShape)
	{
		builder.addTestShape (testShape);

		delete testShape;
		testShape = 0;
	}

	if (writeShape)
	{
		builder.addWriteShape (writeShape);

		delete writeShape;
		writeShape = 0;
	}

	//-- Hardpoints should only be added to the root appearance template
	if (shouldCollectHardpoints())
		builder.collectHardpoints();

	MString writeName = getFilename();
	ExporterLog::addClientDestinationFile(writeName.asChar());

	bool sortOK = builder.sort();
	MESSENGER_REJECT (!sortOK, ("MayaHierarchy::Node::writeLOD - Couldn't sort children for node %s\n",getName()));

	if (publish)
		return builder.write(writeName.asChar());
	return true;
}

//-------------------------------------------------------------------
/**
 * Builds a msh file, it's shaders and textures, and writes them to disk
 * It MESSENGER_REJECTS on fatal errors
 *
 * @return true on export success, false on non-fatal error (i.e. a tga file couldn't be exported to a dds, but export continued)
 *
 */
bool MayaHierarchy::Node::writeMsh (bool publish) const
{
	MStatus status;

	//--------------------------------------------------------------------
	//-- if it's a radar, test, or write shape, ignore it
	if (  isRadarShape(getName()) 
		|| isTestShape(getName()) 
		|| isWriteShape(getName())
		)
		return true;
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//-- find out if the mesh has alpha
	bool meshHasAlpha;
	{
		MDagPath dagPath = getMayaDagPath ();

		//-- get the object from the node
		MObject object = dagPath.node (&status);
		if (!status)
		{
			return false;
		}

		MFnMesh fnMesh(object,&status);
		if (!status) 
		{
			return false;
		}

		meshHasAlpha = MayaUtility::meshHasAlpha(fnMesh);
	}
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//-- is the mesh an instance
	if (getInstanced())
	{
		//-- get the node from the path
		MDagPath dagPath = getMayaDagPath();

		//-- get the object from the node
		MObject mayaObject = dagPath.node(&status);
		if (!status)
		{
			return false;
		}
	}
	//--------------------------------------------------------------------

	MSelectionList sels;
	gatherChildren(sels);

	MayaMeshReader     reader (sels);
	MESSENGER_REJECT_STATUS(!reader.isValid(), ("failed to read mesh\n"));
	
	MString writeName = getFilename();
	MESSENGER_LOG(("preparing to export mesh %s\n",writeName.asChar()));

	StaticMeshBuilder builder(getHierarchy()->getRootMayaObject());
	builder.setHasAlpha(meshHasAlpha);

	//--------------------------------------------------------------------
	// Search for collision data under this node
	for (int i = 0; i < getNumberOfChildren (); i++)
	{
		builder.addCollisionNode(getChild(i));
	}
	//--------------------------------------------------------------------

	bool result;

	//--------------------------------------------------------------------
	result = reader.buildMesh(&builder, m_hierarchy->getShaderTemplateReferenceDir(), m_hierarchy->getTextureReferenceDir());
	MESSENGER_REJECT_STATUS(!result, ("failed to build mesh\n"));
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	// tell the builder we're done
	result = builder.specifyNoMoreData();
	MESSENGER_REJECT_STATUS(!result, ("builder failed on specifyNoMoreData()\n"));
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//-- Hardpoints should only be added to the root appearance template
	if (shouldCollectHardpoints())
	{
		builder.collectHardpoints();
	}
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	if (publish)
	{
		ExporterLog::addClientDestinationFile(writeName.asChar());
		MESSENGER_LOG(("writing data file image for \"%s\"\n", writeName.asChar()));

		//--------------------------------------------------------------------
		Iff iff(1024);
		result = builder.write(iff, m_minVector, m_maxVector);
		if (!result)
		{		
			MESSENGER_LOG_ERROR(("failed to write static mesh data image for \"%s\"\n", writeName.asChar()));
			return false;
		}
		//--------------------------------------------------------------------

		//--------------------------------------------------------------------
		result = iff.write(writeName.asChar(), true);
		if (!result)
		{		
			MESSENGER_LOG_ERROR(("failed to write static mesh data image for \"%s\"\n", writeName.asChar()));
			return false;
		}
		//--------------------------------------------------------------------

		MESSENGER_LOG(("successfully wrote static mesh data to \"%s\"\n", writeName.asChar()));
	}
	//--------------------------------------------------------------------

	bool writeSuccess = true;

	//--------------------------------------------------------------------

	result = reader.generateShaderTemplateData(
		builder, 
		m_hierarchy->getTextureReferenceDir(), 
		m_hierarchy->getEffectReferenceDir(), 
		m_hierarchy->getShaderTemplateReferenceDir(), 
		m_hierarchy->getShaderTemplateWriteDir(), 
		m_hierarchy->getTextureWriteDir()
	);

	if (!result)
	{
		writeSuccess = false;
		MESSENGER_LOG_ERROR(("non-fatal error occured generating shader template data for mesh %s\n", m_name.c_str()));
	}
	else
		MESSENGER_LOG(("successfully generated shader templates for mesh %s\n", m_name.c_str()));
	//--------------------------------------------------------------------

	return writeSuccess;
}

//-------------------------------------------------------------------

namespace pobExportUtils
{
	// C++ sucks.  I'd really like all these to be nested within writePob, but you can't use local types as template arguments (in this case, Cell).
	struct Portal
	{
		int  m_index;
		bool m_disabled;
		bool m_passable;
		bool m_clockwise;
		MString m_doorStyle;
		bool m_hasDoorHardpoint;
		Transform m_doorHardpoint;
		std::string m_name;
	};

	struct PortalConnectivity
	{
		int m_clockwiseCell;
		int m_counterclockwiseCell;
		PortalConnectivity();
		std::string m_portalName;
	};	

	struct CellLight
	{
		int        m_type;
		VectorArgb m_diffuse;
		VectorArgb m_specular;
		Transform  m_transform;
		float      m_attenuation0;
		float      m_attenuation1;
		float      m_attenuation2;
	};

	struct Cell
	{
		bool            m_canSeeWorldCell;

		bool            m_hasPortalList;
		PortalList      m_portalList;

		const char     *m_name;

		bool            m_hasAppearance;
		MString         m_appearanceName;

		FloorList       m_floorList;

		LightList       m_lightList;

		int             m_pathNodeIndex;
		Vector          m_pathNodePosition;

		bool            m_hasCollision;
		Extent *        m_collisionExtent;
	};
}

using namespace pobExportUtils;

PortalConnectivity::PortalConnectivity()
:
	m_clockwiseCell(-1),
	m_counterclockwiseCell(-1)
{
}

// ----------------------------------------------------------------------
// HACK - Fix cell 0 not getting a reference to its floor because the
// floor is part of the appearance

bool MayaHierarchy::Node::findChildByType ( MayaHierarchy::Type type, MayaHierarchy::Node const * & outNode ) const
{
	if(getType() == type)
	{
        outNode = this;
		return true;
	}

	for (int i = 0; i < getNumberOfChildren(); i++)
	{
		if(getChild(i)->findChildByType(type,outNode))
		{
			return true;
		}
	}

	return false;
}

bool MayaHierarchy::Node::findParentByType ( MayaHierarchy::Type type, MayaHierarchy::Node const * & outNode ) const
{
	if(getType() == type)
	{
        outNode = this;
		return true;
	}

    if(getParent())
    {
        return getParent()->findParentByType(type,outNode);
    }
    else
    {
        return false;
    }
}

bool MayaHierarchy::extractFloorName ( MayaHierarchy::Node const * node, std::string & floorName )
{
    Node const * floorNode = NULL;

    if(node->findChildByType(MayaHierarchy::T_floor,floorNode))
    {
        floorName = floorNode->getAppearanceReferenceName().asChar();
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------

PathNode createBuildingCellPathNode ( Vector const & position, int cellIndex, int pathNodeIndex )
{
	PathNode node;

	node.setPosition_p( position );
	node.setType( PNT_BuildingCell );
	node.setKey( cellIndex );
	node.setIndex( pathNodeIndex );

	return node;
}

PathNode createBuildingPortalPathNode ( Vector const & position, int buildingPortalIndex, int pathNodeIndex )
{
	PathNode node;

	node.setPosition_p( position );
	node.setType( PNT_BuildingPortal );
	node.setKey( buildingPortalIndex );
	node.setIndex( pathNodeIndex );

	return node;
}

PathNode createBuildingEntrancePathNode ( Vector const & position, int buildingPortalIndex, int pathNodeIndex )
{
	PathNode node;

	node.setPosition_p( position );
	node.setType( PNT_BuildingEntrance );
	node.setKey( buildingPortalIndex );
	node.setIndex( pathNodeIndex );

	return node;
}

SimplePathGraph * MayaHierarchy::createBuildingPathGraph (CellList & cellList, PortalGeometryList const & portalGeometryList)
{
	int cellCount = cellList.size();
	int portalCount = portalGeometryList.size();

	SimplePathGraph::NodeList * nodes = new SimplePathGraph::NodeList();
	SimplePathGraph::EdgeList * edges = new SimplePathGraph::EdgeList();

	nodes->reserve(cellCount + portalCount);

	int i;

	//@todo - It'd be nice if these nodes had positions, for debugging purposes.

	// How would I find the center of a cell? The center of the bounding box
	// of the appearance, with a Y coordinate the average of the portal node
	// Y coordinates?

	int nodeCounter = 0;

	for(i = 0; i < cellCount; i++)
	{
		nodes->push_back( createBuildingCellPathNode(cellList[i].m_pathNodePosition,i,nodeCounter) );

		cellList[i].m_pathNodeIndex = nodeCounter;

		nodeCounter++;
	}

	int cellNodeCount = nodeCounter;

	for(i = 0; i < portalCount; i++)
	{
		IndexedTriangleList const * const indexedTriangleList = portalGeometryList[i];
		std::vector<Vector> const & vertices = indexedTriangleList->getVertices();

		// Find the center of the portal

		Vector center = Vector::zero;

		for (uint k = 0; k < vertices.size(); k++) 
			center += vertices[k];

		center /= float(vertices.size());

		// Flag portals used by cell 0 (the world cell) as entrances to the building

		Cell const & cell = cellList[0];

		bool entrance = false;

		int cellPortalCount = cell.m_portalList.size();

		for(int j = 0; j < cellPortalCount; j++)
		{
			Portal const & pi = cell.m_portalList[j];

			if(pi.m_index == i)
			{
				entrance = true;
				break;
			}
		}

		if(entrance)
		{
			nodes->push_back( createBuildingEntrancePathNode(center,i,nodeCounter) );
		}
		else
		{
			nodes->push_back( createBuildingPortalPathNode(center,i,nodeCounter) );
		}

		nodeCounter++;
	}

	for(i = 0; i < cellCount; i++)
	{
		Cell const & cell = cellList[i];

		int cellPortalCount = cell.m_portalList.size();

		for(int j = 0; j < cellPortalCount; j++)
		{
			Portal const & pi = cell.m_portalList[j];

			int cellPathNodeIndex = cell.m_pathNodeIndex;
			int portalPathNodeIndex = cellNodeCount + pi.m_index;

			edges->push_back( PathEdge(cellPathNodeIndex, portalPathNodeIndex) );
			edges->push_back( PathEdge(portalPathNodeIndex, cellPathNodeIndex) );
		}
	}

	SimplePathGraph * newGraph = new SimplePathGraph( nodes, edges );

	return newGraph;
}

// ----------------------------------------------------------------------
// Two polygons "match" if all their vertices match up and they have 
// opposite windings

bool MayaHierarchy::testMatchingPolygons( std::vector<Vector> const & polyA, std::vector<Vector> const & polyB, float epsilon )
{
	if (polyA.size() != polyB.size()) 
		return false;

	int vertexCount = polyA.size();

	for(int offset = 0; offset < vertexCount; offset++)
	{
		bool match = true;

		for(int i = 0; i < vertexCount; i++)
		{
			int indexA = vertexCount - i - 1;
			int indexB = (offset + i) % vertexCount;

			Vector const & vertA = polyA[indexA];
			Vector const & vertB = polyB[indexB];

			if(!vertA.withinEpsilon(vertB,epsilon)) 
			{
				match = false;
				break;
			}
		}

		if(match)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

int MayaHierarchy::findMatchingPortal(PortalGeometryList const & portalGeometryList, IndexedTriangleList const & indexedTriangleList)
{
	int const portalCount = portalGeometryList.size();
	for (int i = 0; i < portalCount; ++i)
	{
		IndexedTriangleList const * const testPortal = portalGeometryList[i];
		if (testMatchingPortals(indexedTriangleList, *testPortal, 0.05f))
			return i;
	}

	return -1;
}

// ----------------------------------------------------------------------

IndexedTriangleList * MayaHierarchy::extractPortalGeometry(Node const * const node)
{
	//-- Make sure we have the correct node type
	MESSENGER_REJECT(node->getType() != T_prt, ("Portal list children must be portals at %s\n", node->getHierarchyName()));

	//-- Extract the indexed triangle list for the portal geometry from the node geometry
	MayaIndexedTriangleList mayaIndexedTriangleList(node->getMayaDagPath(), true);
	MESSENGER_REJECT(mayaIndexedTriangleList.getScale() != Vector::xyz111, ("Non-unit scale found for portal shape %s\n", node->getHierarchyName()));
	IndexedTriangleList * const indexedTriangleList = mayaIndexedTriangleList.createIndexedTriangleList();

	if (indexedTriangleList->getIndices().empty())
	{
		delete indexedTriangleList;
		return 0;
	}

	return indexedTriangleList;
}

// ----------------------------------------------------------------------
// Mark each cell and portal node with the index they'll have in the pob
// once it's written. The indices are used by nodes elsewhere in the
// hierarchy to refer to a particular cell or portal in the pob.

bool MayaHierarchy::preprocessPobNode(Node * const pobNode)
{
	PortalGeometryList portalGeometryList;

	int buildingPortalCounter = 0;

	// Iterate over all cells in the pob
	int cellCounter = 0;
	for (int i = 0; i < pobNode->getNumberOfChildren (); ++i)
	{
		Node * const cellNode = pobNode->getChild(i);

		if (cellNode->getType() != T_cel) 
			continue;

		cellNode->setBuildingCellIndex(cellCounter);

		cellCounter++;

		// Iterate over all portal lists in the cell
		int cellPortalCounter = 0;
		for (int j = 0; j < cellNode->getNumberOfChildren(); ++j)
		{
			Node * const portalListNode = cellNode->getChild(j);

			if (portalListNode->getType() != T_pls) 
				continue;

			// Iterate over all portals in the portal list
			for (int k = 0; k < portalListNode->getNumberOfChildren(); ++k)
			{
				Node * const portalNode = portalListNode->getChild(k);

				if (portalNode->getType() != T_prt)
					continue;

				portalNode->setCellPortalIndex(cellPortalCounter);

				cellPortalCounter++;

				//-- Extract the portal geometry from the portal node
				IndexedTriangleList * const portalGeometry = extractPortalGeometry(portalNode);
				MESSENGER_REJECT(!portalGeometry, ("MayaHierarchy::assignBuildingIndices - Couldn't extract portal geometry for node %s.  Is the portal triangulated?\n", portalNode->getName()));

				//-- Verify that the geometry is coplanar
				if (!shouldAllowNonPlanarPortal(portalNode))
					validateCoplanar(*portalGeometry, portalNode->getHierarchyName());

				portalNode->setPortalGeometry(portalGeometry);

				// See if we have a matching portal in our list
				int const matchingPortal = findMatchingPortal(portalGeometryList, *portalGeometry);
				if (matchingPortal != -1)
				{
					MESSENGER_LOG(("portalNode %s matches %i (%p)\n", portalNode->getHierarchyName(), matchingPortal, portalGeometry));

					// If so, the portal node's index is the matching portal's index
					portalNode->setBuildingPortalIndex(matchingPortal);
					portalNode->setClockwise(false);
				}
				else
				{
					MESSENGER_LOG(("portalNode %s added as %i (%p)\n", portalNode->getHierarchyName(), buildingPortalCounter, portalGeometry));

					// Otherwise the portal node gets a new index and we add the portal to our portal list
					portalNode->setBuildingPortalIndex(buildingPortalCounter++);
					portalNode->setClockwise(true);

					portalGeometryList.push_back(portalGeometry);
				}
			}
		}
	}

	//-- Only compute the portal geometry list once
	pobNode->setPortalGeometryList(portalGeometryList);

	return true;
}

// ----------------------------------------------------------------------
// Given a cellPortalIndex and a start node, find the portal in startNode's
// cell with the cellPortalId, and return its buildingPortalId.

bool MayaHierarchy::convertCellPortalIndexToBuildingPortalIndex ( Node const * startNode, int cellPortalIndex, int & buildingPortalIndex )
{
	if(startNode == NULL)
	{
		return false;
	}

	Node const * cellNode = NULL;

    if(!startNode->findParentByType(MayaHierarchy::T_cel, cellNode))
	{
		return false;
	}

	Node const * portalListNode = NULL;

    if(!cellNode->findChildByType(MayaHierarchy::T_pls, portalListNode))
	{
        return false;
	}

	// Search through the portals until we find the one with the given cellPortalIndex

	for (int i = 0; i < portalListNode->getNumberOfChildren(); i++)
	{
		Node const * portalNode = portalListNode->getChild(i);

		if(portalNode->getType() != T_prt) continue;

		if(portalNode->getCellPortalIndex() == cellPortalIndex)
		{
			buildingPortalIndex = portalNode->getBuildingPortalIndex();
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::shouldAllowNonPlanarPortal(Node const * const node)
{
	MStatus status;

	// get access to the portal's dag node
	MFnDagNode dagNode;
	if (!node->getMayaDagNodeFn(dagNode))
		MESSENGER_LOG_WARNING(("MayaHierarchy::shouldAllowNonPlanarPortal - Could not get dag node for node %s\n", node->getName()));

	// check allownNonPlanar on the portal
	dagNode.attribute("allowNonPlanar", &status);

	// getting the attribute means the node is allows non planar portals
	return (status == MS::kSuccess);
}

// ----------------------------------------------------------------------

bool MayaHierarchy::isPortalNodeDisabled(Node const * const node)
{
	MStatus status;

	// get access to the portal's dag node
	MFnDagNode dagNode;
	if (!node->getMayaDagNodeFn(dagNode))
		MESSENGER_LOG_WARNING(("MayaHierarchy::isPortalNodeDisabled - Could not get dag node for node %s\n", node->getName()));

	// check disabled on the portal
	dagNode.attribute("disabled", &status);

	// getting the disabled attribute means the node is disabled
	return (status == MS::kSuccess);
}

// ----------------------------------------------------------------------

bool MayaHierarchy::isPortalNodePassable( Node const * node )
{
	MStatus status;

	// get access to the portal's dag node
	MFnDagNode dagNode;
	if (!node->getMayaDagNodeFn(dagNode))
		MESSENGER_LOG_WARNING(("MayaHierarchy::isPortalNodePassable - Could not get dag node for node %s\n",node->getName()));

	// check passability on the portal
	dagNode.attribute ("impassable", &status);

	// failure to get the impassible attribute means the node is passable
	return (status != MS::kSuccess);
}

// ----------------------------------------------------------------------

MString MayaHierarchy::extractDoorStyle( Node const * node )
{
	MStatus status;

	// Get the dag node from the hierarchy node

	MFnDagNode dagNode;
	if (!node->getMayaDagNodeFn(dagNode))
	{
		MESSENGER_LOG_WARNING(("MayaHierarchy::extractDoorStyle - Cant' get dag node for node %s\n",node->getName()));
		return MString();
	}

	// Get the door style attribute from the dag node. The attribute can 
	// be named door or doorStyle

	MObject attributeObject = dagNode.attribute("door", &status);

	if(!status)
	{
		attributeObject = dagNode.attribute("doorStyle", &status);
	}

	if(!status)
	{
//		MESSENGER_LOG_WARNING(("MayaHierarchy::extractDoorStyle - Node %s has no door style attribute\n",node->getName()));
		return MString();
	}

	// Get the plug for the door style attribute

	MPlug attributePlug = dagNode.findPlug(attributeObject, &status);

	if(!status)
	{
		MESSENGER_LOG_WARNING(("MayaHierarchy::extractDoorStyle - Can't get attribute plug for door style on node %s\n",node->getName()));
		return MString();
	}

	// Get the value (a string) from the plug

	MString doorStyle;

	status = attributePlug.getValue(doorStyle);

	if(!status)
	{
		MESSENGER_LOG_WARNING(("MayaHierarchy::extractDoorStyle - Can't get value for door style plug on node %s\n",node->getName()));
		return MString();
	}

	// Done

	return doorStyle;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::Node::writePob (bool /*publish*/) const
{
	PortalGeometryList const & portalGeometryList = getPortalGeometryList();
	CellList cellList;

	// Before we begin, build a list of all door hardpoints in the scene.
	HardpointVec doorHardpoints;
	std::vector<int> doorHardpointUseCounts;
	IGNORE_RETURN(MeshBuilder::collectHardpoints(0, doorHardpoints, "hp_door", false));
	doorHardpointUseCounts.resize(doorHardpoints.size(), 0);

	// gather necessary data from our children
	{
		for (int i = 0; i < getNumberOfChildren (); ++i)
		{
			const Node *cellNode = getChild(i);
			MESSENGER_REJECT(cellNode->getType() != T_cel, ("Portal object children must be cells at %s\n", cellNode->getHierarchyName()));

			Cell cell;
			cell.m_canSeeWorldCell = cellNode->getCanSeeWorldCell();
			cell.m_hasPortalList = false;
			cell.m_hasAppearance = false;
			cell.m_name          = cellNode->getHierarchyName();
			cell.m_pathNodePosition = cellNode->getCellPathNodePosition();
			cell.m_hasCollision = false;
			cell.m_collisionExtent = NULL;
			{
				const char *slash = strrchr(cell.m_name, '/');
				MESSENGER_REJECT(!slash || !slash[1], ("node name appears to be invalid\n"));

				cell.m_name = slash+1;
				const char *underscore = strchr(cell.m_name, '_');
				if (underscore && underscore[1])
					cell.m_name = underscore+1;
			}

			// look for the portal list in the cell
			for (int j = 0; j < cellNode->getNumberOfChildren(); ++j)
			{
				const Node *childNode = cellNode->getChild(j);

				if (childNode->getType() == T_pls)
				{
					const Node *portalListNode = childNode;
					MESSENGER_REJECT(cell.m_hasPortalList, ("Portal object must have exactly one portal list child at %s\n", portalListNode->getHierarchyName()));
					cell.m_hasPortalList = true;

					// process all the portals in the portal list
					for (int k = 0; k < portalListNode->getNumberOfChildren(); ++k)
					{
						const Node *portalNode = portalListNode->getChild(k);
						MESSENGER_REJECT(portalNode->getType() != T_prt, ("Portal list children must be portals at %s\n", portalNode->getHierarchyName()));

						// search for the portal already existing
						Portal pi;
						pi.m_index = portalNode->getBuildingPortalIndex();
						pi.m_disabled = isPortalNodeDisabled(portalNode);
						pi.m_passable = isPortalNodePassable(portalNode);
						pi.m_clockwise = portalNode->getClockwise();
						pi.m_doorStyle = extractDoorStyle(portalNode);
						pi.m_hasDoorHardpoint = false;
						pi.m_doorHardpoint = Transform::identity;
						pi.m_name = portalNode->getName();

						// Search for a door hardpoint to associate with this portal. Door hardpoints are only
						// matched to portals if they're within half a meter of the portal

						{
							IndexedTriangleList const * const portalGeometry = portalNode->getPortalGeometry();

							VertexList vertexList;
							vertexList.reserve(3);

							int doorHardpointIndex = -1;
							float minimumDistance = REAL_MAX;

							for (uint i = 0; i < doorHardpoints.size(); i++)
							{
								Hardpoint * hardpoint = doorHardpoints[i];

								std::vector<Vector> const & vertices = portalGeometry->getVertices();
								std::vector<int> const & indices = portalGeometry->getIndices();
								uint const numberOfFaces = portalGeometry->getIndices().size() / 3;
								for (uint faceIndex = 0, index = 0; faceIndex < numberOfFaces; ++faceIndex)
								{
									vertexList.clear();
									vertexList.push_back(vertices[indices[index++]]);
									vertexList.push_back(vertices[indices[index++]]);
									vertexList.push_back(vertices[indices[index++]]);
									float const distance = Distance3d::DistancePointPoly(hardpoint->getTransform().getPosition_p(), vertexList);

									if (distance < minimumDistance)
									{
										minimumDistance = distance;
										doorHardpointIndex = i;
									}
								}
							}

							if ((doorHardpointIndex != -1) && (minimumDistance < 0.5f))
							{
								pi.m_hasDoorHardpoint = true;
								pi.m_doorHardpoint = doorHardpoints[doorHardpointIndex]->getTransform();

								doorHardpointUseCounts[doorHardpointIndex]++;
							}
						}

						// set the cell portal index
//						MESSENGER_LOG(("writePob: portalNode %s added %i\n", portalNode->getHierarchyName(), pi.m_index));
						cell.m_portalList.push_back(pi);
					}
				}
				else if (childNode->getType() == T_collision)
				{
					MESSENGER_REJECT(cell.m_hasCollision,("Cells %s must have only one collision group\n",childNode->getHierarchyName()));
					cell.m_hasCollision = true;

					// Found the collision data for this cell

					const Node * collisionNode = childNode;

					// ----------
					// Build the collision extent for this cell

					MeshBuilder builder(getHierarchy()->getRootMayaObject());

					cell.m_collisionExtent = builder.createExtent(collisionNode);

					// ----------
					
					for (int i = 0; i < collisionNode->getNumberOfChildren(); ++i)
					{
						const Node *pNode = collisionNode->getChild(i);

						if(pNode->getType() == T_floor)
						{
							// Found a floor for this cell

							MESSENGER_REJECT(!cell.m_floorList.empty(),("Cells %s must have only one floor\n",childNode->getHierarchyName()));

							cell.m_floorList.push_back( pNode->getAppearanceReferenceName().asChar() );
						}
					}			
				}
				else if (childNode->getType() == T_lightList)
				{
					// process all the lights in the light list
					for (int k = 0; k < childNode->getNumberOfChildren(); ++k)
					{
						const Node *lightNode = childNode->getChild(k);
						MESSENGER_REJECT(lightNode->getType() != T_light, ("LightList must have only lights as children at %s\n", childNode->getHierarchyName()));

						MStatus status;
						MDagPath dagPath = lightNode->getMayaDagPath ();
						MObject object = dagPath.node (&status);
						MFnLight light(dagPath, &status);
						MESSENGER_REJECT(!status, ("Light is not a light at %s\n", childNode->getHierarchyName()));

						CellLight cellLight;

						MObject mayaTransform = dagPath.transform (&status);
						MESSENGER_REJECT(!status, ("Could not get transform for light\n"));
						if (!MayaTransform_to_Transform(mayaTransform, cellLight.m_transform))
						{
							MESSENGER_REJECT(!status, ("Could not convert light transform\n"));
						}

						const float intensity = light.intensity(&status);
						const MColor color = light.color(&status);

						const bool diffuse = light.lightDiffuse(&status);
						if (diffuse)
						{
							cellLight.m_diffuse.a = color.a * intensity;
							cellLight.m_diffuse.r = color.r * intensity;
							cellLight.m_diffuse.g = color.g * intensity;
							cellLight.m_diffuse.b = color.b * intensity;
						}

						const bool specular = light.lightSpecular(&status);
						if (specular)
						{
							cellLight.m_specular.a = color.a * intensity;
							cellLight.m_specular.r = color.r * intensity;
							cellLight.m_specular.g = color.g * intensity;
							cellLight.m_specular.b = color.b * intensity;
						}

						UNREF(specular);

						cellLight.m_attenuation0 = 1;
						cellLight.m_attenuation1 = 0;
						cellLight.m_attenuation2 = 0;

						MFn::Type type = object.apiType();
						switch (type)
						{
							case MFn::kAmbientLight:
								cellLight.m_type = 0;
								cellLight.m_diffuse.a = color.a * intensity;
								cellLight.m_diffuse.r = color.r * intensity;
								cellLight.m_diffuse.g = color.g * intensity;
								cellLight.m_diffuse.b = color.b * intensity;
								break;

							case MFn::kDirectionalLight:
								cellLight.m_type = 1;
								break;

							case MFn::kPointLight:
								{
									cellLight.m_type = 2;
									MFnPointLight pointLight(lightNode->getMayaDagPath(), &status);
									MESSENGER_REJECT(!status, ("Light is not a light\n"));

									short decay = pointLight.decayRate(&status);
			
									switch (decay)
									{
										case 0:
											cellLight.m_attenuation0 = 1;
											cellLight.m_attenuation1 = 0;
											cellLight.m_attenuation2 = 0;
											break;

										case 1:
											cellLight.m_attenuation0 = 0;
											cellLight.m_attenuation1 = 1;
											cellLight.m_attenuation2 = 0;
											break;

										case 2:
											cellLight.m_attenuation0 = 0;
											cellLight.m_attenuation1 = 0;
											cellLight.m_attenuation2 = 1;
											break;

										default:
											MESSENGER_REJECT(true, ("Cubic and above fall-off is not supported\n"));
											break;
									}
								}
								break;

							default:
								MESSENGER_REJECT(true, ("Unknown light type\n"));
								break;
						}

#if 1
						MESSENGER_LOG(("%d=cell %d=light %1d=type (%9.4f,%9.4f,%9.4f,%9.4f)=dif (%9.4f,%9.4f,%9.4f,%9.4f)=spec (%9.4f,%9.4f,%9.4f)=pos (%9.4f,%9.4f,%9.4f)=atten\n", 
							i,
							k,
							cellLight.m_type,
							cellLight.m_diffuse.a, cellLight.m_diffuse.r, cellLight.m_diffuse.g, cellLight.m_diffuse.b, 
							cellLight.m_specular.a, cellLight.m_specular.r, cellLight.m_specular.g, cellLight.m_specular.b, 
							cellLight.m_transform.getPosition_p().x, cellLight.m_transform.getPosition_p().y, cellLight.m_transform.getPosition_p().z, 
							cellLight.m_attenuation0, cellLight.m_attenuation1, cellLight.m_attenuation2));
#endif

						cell.m_lightList.push_back(cellLight);
					}
				}
				else if(childNode->isAppearance())
				{
					// Found the appearance node for this cell.

					MESSENGER_REJECT(cell.m_hasAppearance, ("Each cell object must have exactly one appearance at %s\n", childNode->getHierarchyName()));
					cell.m_hasAppearance = true;

					for (int k = 0; k < childNode->getNumberOfChildren(); ++k)
					{
						const Node * node = childNode->getChild(k);

						if(node->getType() == T_collision)
						{
							MESSENGER_REJECT(cell.m_hasCollision,("Cells %s must have only one collision group\n",childNode->getHierarchyName()));
							cell.m_hasCollision = true;

							// ----------
							// Build the collision extent for this cell

							MeshBuilder builder(getHierarchy()->getRootMayaObject());

							cell.m_collisionExtent = builder.createExtent(node);
						}
					}

					//-- The first cell should write an apt for its external appearance
					if (cellNode->getBuildingCellIndex() == 0)
					{
						cell.m_appearanceName = cellNode->getAptReferenceName();

						writeAptFile(cellNode->getAptFilename(), childNode->getAppearanceReferenceName());
						ExporterLog::addClientDestinationFile(cellNode->getAptFilename().asChar());

						writeAptFile(cellNode->getServerAptFilename(), childNode->getServerAppearanceTemplateReferenceName());
						ExporterLog::addServerDestinationFile(cellNode->getServerAptFilename().asChar());

						m_hierarchy->writeServerAppearanceTemplate(childNode);
					}
					else
					{
						cell.m_appearanceName = childNode->getAppearanceReferenceName();
					}
				}
			}

			MESSENGER_REJECT(!cell.m_hasPortalList, ("Portal object must have exactly one portal list child at %s\n", cellNode->getHierarchyName()));
			MESSENGER_REJECT(!cell.m_hasAppearance, ("Each cell object must have exactly one appearance at %s\n", cellNode->getHierarchyName()));

			//@todo - HACK - Fix cell 0 not getting a reference to its floor because the
			// floor is part of the appearance

			if((i == 0) && cell.m_floorList.empty())
			{
				std::string floorName;
				
				if(m_hierarchy->extractFloorName(cellNode,floorName))
				{
					cell.m_floorList.push_back(floorName.c_str());
				}
			}

			// add the cell to the cell list
			cellList.push_back(cell);
		}
	}

	// ----------------------------------------------------------------------

	const int numberOfPortals = portalGeometryList.size();
	const int numberOfCells   = cellList.size();

	// figure out cell/portal connectivity
	std::vector<PortalConnectivity> portalConnectivity;

	portalConnectivity.resize(numberOfPortals);
	{
		for (int j = 0; j < numberOfCells; ++j)
		{
			Cell const & cell = cellList[j];

			const int numberOfPortals = cell.m_portalList.size();
			for (int k = 0; k < numberOfPortals; ++k)
			{
				Portal const & portal = cell.m_portalList[k];
				int const portalIndex = portal.m_index;
				
				if (portal.m_clockwise)
				{
					MESSENGER_REJECT(portalConnectivity[portalIndex].m_clockwiseCell != -1, ("portal with two cells on the clockwise side (cell %s)\n", cell.m_name));
					portalConnectivity[portalIndex].m_clockwiseCell = j;
				}
				else
				{
					MESSENGER_REJECT(portalConnectivity[portalIndex].m_counterclockwiseCell != -1, ("portal with two cells on the counter-clockwise side (cell %s)\n", cell.m_name));
					portalConnectivity[portalIndex].m_counterclockwiseCell = j;
				}

				portalConnectivity[portalIndex].m_portalName = portal.m_name;
			}
		}

		// force all cells with portals to cell 0 to have their canSeeWorldCell flag on
		for (int m = 0; m < numberOfPortals; ++m)
		{
			MESSENGER_REJECT(portalConnectivity[m].m_clockwiseCell == -1, ("portal [%s] has no cell on the clockwise side\n", portalConnectivity[m].m_portalName.c_str()));
			MESSENGER_REJECT(portalConnectivity[m].m_counterclockwiseCell == -1, ("portal [%s] has no cell on the counter-clockwise side\n", portalConnectivity[m].m_portalName.c_str()));

			if (portalConnectivity[m].m_clockwiseCell == 0)
				cellList[portalConnectivity[m].m_counterclockwiseCell].m_canSeeWorldCell = true;
			if (portalConnectivity[m].m_counterclockwiseCell == 0)
				cellList[portalConnectivity[m].m_clockwiseCell].m_canSeeWorldCell = true;
		}	
	}
	
	Iff iff(1024);
	iff.insertForm(TAG_PRTO);

		iff.insertForm(TAG_0004);

			iff.insertChunk(TAG_DATA);
				iff.insertChunkData(static_cast<int32>(numberOfPortals));
				iff.insertChunkData(static_cast<int32>(numberOfCells));
			iff.exitChunk(TAG_DATA);

			// save the portal geometry
			iff.insertForm(TAG_PRTS);
				for (int i = 0; i < numberOfPortals; ++i)
					portalGeometryList[i]->write(iff);
			iff.exitForm(TAG_PRTS);

			// save all the cell data
			iff.insertForm(TAG_CELS);
				for (int j = 0; j < numberOfCells; ++j)
				{
					Cell &cell = cellList[j];

					iff.insertForm(TAG_CELL);
						iff.insertForm(TAG_0005);

							const int32 numberOfPortals = cell.m_portalList.size();

							iff.insertChunk(TAG_DATA);

								iff.insertChunkData(numberOfPortals);
								iff.insertChunkData(cell.m_canSeeWorldCell);
								iff.insertChunkString(cell.m_name);
								iff.insertChunkString(cell.m_appearanceName.asChar());

								bool hasFloor = !cell.m_floorList.empty();

								iff.insertChunkData(hasFloor);
								if(hasFloor) 
								{
									iff.insertChunkString(cell.m_floorList[0].asChar());
								}

							iff.exitChunk(TAG_DATA);

							if(cell.m_collisionExtent)
							{
								cell.m_collisionExtent->write(iff);

								delete cell.m_collisionExtent;
							}
							else
							{
								iff.insertForm(TAG_NULL);
								iff.exitForm(TAG_NULL);
							}
							
							for (int k = 0; k < numberOfPortals; ++k)
							{
								Portal const & pi = cell.m_portalList[k];

								char const * doorStyle = pi.m_doorStyle.asChar();

								UNREF(doorStyle);

								iff.insertForm(TAG_PRTL);
									iff.insertChunk(TAG_0005);

										const int portalIndex = pi.m_index;
										const bool clockwise = pi.m_clockwise;
										const bool disabled = pi.m_disabled;
										const bool passable = pi.m_passable;

										iff.insertChunkData(static_cast<int8>(disabled ? 1 : 0));
										iff.insertChunkData(static_cast<int8>(passable ? 1 : 0));
										iff.insertChunkData(static_cast<int32>(portalIndex));

										// set the direction and target of the portal
										if (clockwise)
										{
											iff.insertChunkData(static_cast<int8>(1));
											iff.insertChunkData(static_cast<int32>(portalConnectivity[portalIndex].m_counterclockwiseCell));
										}
										else
										{
											iff.insertChunkData(static_cast<int8>(0));
											iff.insertChunkData(static_cast<int32>(portalConnectivity[portalIndex].m_clockwiseCell));
										}

										iff.insertChunkString(pi.m_doorStyle.asChar());
										iff.insertChunkData(pi.m_hasDoorHardpoint);
										iff.insertChunkFloatTransform(pi.m_doorHardpoint);

									iff.exitChunk(TAG_0005);
								iff.exitForm(TAG_PRTL);
							}

							iff.insertChunk(TAG_LGHT);

								const int  numberOfLights = static_cast<int>(cell.m_lightList.size());
								iff.insertChunkData(static_cast<int32>(numberOfLights));

								for (int w = 0; w < numberOfLights; ++w)
								{
									const CellLight &l = cell.m_lightList[w];

									iff.insertChunkData(static_cast<int8>(l.m_type));
									iff.insertChunkFloatVectorArgb(l.m_diffuse);
									iff.insertChunkFloatVectorArgb(l.m_specular);
									iff.insertChunkFloatTransform(l.m_transform);
									iff.insertChunkData(static_cast<float>(l.m_attenuation0));
									iff.insertChunkData(static_cast<float>(l.m_attenuation1));
									iff.insertChunkData(static_cast<float>(l.m_attenuation2));
								}

							iff.exitChunk(TAG_LGHT);

						iff.exitForm(TAG_0005);
					iff.exitForm(TAG_CELL);
				}
			iff.exitForm(TAG_CELS);

			// write out the building path graph
			{
				SimplePathGraph * const buildingPathGraph = createBuildingPathGraph(cellList, portalGeometryList);

				if (buildingPathGraph)
					buildingPathGraph->write(iff);

				delete buildingPathGraph;
			}

			uint32 crc = iff.calculateCrc();

			if (getHierarchy() && !getHierarchy()->getPobFileName().empty())
			{
				uint32 oldCrc = 0;

				bool const result = PortalPropertyTemplate::extractPortalLayoutCrc(getHierarchy()->getPobFileName().c_str(), oldCrc);

				MESSENGER_REJECT(!result, ("MayaHierarchy::writePob - Failed to extract POB CRC from existing asset (%s)."
					" The asset may not exist in perforce."
					" Turn off Fix POB CRC or see programming for help.\n", getHierarchy()->getPobFileName().c_str()));

				MESSENGER_REJECT(crc == oldCrc, ("MayaHierarchy::writePob - Fix POB CRC was checked but the new CRC matches the one in the pob file on disk (%s)."
					" Try reverting the pob file or check the perforce history to see if a wrong version was submitted and needs to be rolled back."
					" Turn off Fix POB CRC or see programming for help.\n", getHierarchy()->getPobFileName().c_str()));

				crc = oldCrc;
			}

			iff.insertChunk(TAG_CRC);
				iff.insertChunkData(crc);
			iff.exitChunk(TAG_CRC);

		iff.exitForm(TAG_0004);

	iff.exitForm(TAG_PRTO);


	MString writeName = getFilename();
	ExporterLog::addSharedDestinationFile(writeName.asChar());

	// Check to see if we were unable to match a door hardpoint to a portal,
	// or if any door hardpoints were matched to more than 2 portals

	{
		for(uint i = 0; i < doorHardpointUseCounts.size(); i++)
		{
			if(doorHardpointUseCounts[i] < 2)
			{
				MESSENGER_REJECT(true,("MayaHierarchy::writePob - One of the door hardpoints in the scene was unused\n"));
			}

			if(doorHardpointUseCounts[i] > 2)
			{
				MESSENGER_REJECT(true,("MayaHierrchy::writePob - One of the door hardpoints in the scene was matched to multiple doors\n"));
			}
		}
	}

	bool writeOk = iff.write(writeName.asChar(), true);

	// Everything done, cleanup.

	{
		for(uint i = 0; i < doorHardpoints.size(); i++)
		{
			delete doorHardpoints[i];
			doorHardpoints[i] = NULL;
		}
	}

	return writeOk;
}

// ----------------------------------------------------------------------

void MayaHierarchy::Node::computeMinMaxBoxVector () const
{
	switch (m_type)
	{
	case T_uninitialized:
	case T_cel:
	case T_pob:
	case T_msh:
		break;

	case T_cmp:
	case T_lod:
		{
			//-- Build this node's min/max box vector from its children.
			DEBUG_FATAL (getNumberOfChildren () == 0, ("node that needs children has no children"));

			//-- Initialize values with those from first child.
			Node const *const firstChild = getChild (0);
			NOT_NULL(firstChild);

			m_minVector = firstChild->getMinBoxVector ();
			m_maxVector = firstChild->getMaxBoxVector ();

			//-- Incorporate values from all other children.
			for (int i = 1; i < getNumberOfChildren (); ++i)
			{
				Node const *const child = getChild (i);
				NOT_NULL(child);

				switch (child->getType ())
				{
					case T_msh:
					case T_cmp:
					case T_lod:
						// Incorporate this child into the appearance size.  No other child type affects this.
						{
							Vector const &childMinVector = child->getMinBoxVector ();
							Vector const &childMaxVector = child->getMaxBoxVector ();

							m_minVector.x = std::min(m_minVector.x, childMinVector.x);
							m_minVector.y = std::min(m_minVector.y, childMinVector.y);
							m_minVector.z = std::min(m_minVector.z, childMinVector.z);

							m_maxVector.x = std::max(m_maxVector.x, childMaxVector.x);
							m_maxVector.y = std::max(m_maxVector.y, childMaxVector.y);
							m_maxVector.z = std::max(m_maxVector.z, childMaxVector.z);
						}
						break;

					default:
						break;
				}
			}
		}
		break;
	}
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::computeSphere () const
{
	switch (m_type)
	{
	case T_uninitialized:
		break;

	case T_cmp:
	case T_lod:
	case T_cel:
	case T_pob:
		{
			DEBUG_FATAL (getNumberOfChildren () == 0, ("node that needs children has no children"));

			m_sphere = getChild (0)->getSphere ();

			int i;
			for (i = 1; i < getNumberOfChildren (); ++i)
				m_sphere = Containment3d::EncloseSphere(m_sphere, getChild (i)->getSphere ());
		}
		break;

	case T_msh:
		{
			m_sphere.setCenter (ExporterLog::getCurrentSphereCenter());
			m_sphere.setRadius (ExporterLog::getCurrentSphereRadius());
		}
		break;
	}
}

//-------------------------------------------------------------------
/**
 * Build a nodes files, and write them to disk
 * 
 * @return true on successful build and write, false on non-fatal error (we MESSENGER_REJECT on fatal errors)
 * 
 */
bool MayaHierarchy::Node::write (bool publish) const
{
	MESSENGER_LOG(("Node::write for type %s\n",MayaHierarchy::getTypeName(getType())));

	bool result = false;

	if (getInstanced ())
	{
		MStatus status;

		MObject mayaObject = m_mayaDagPath.node (&status);
		if (!status)
			return false;

		if (m_hierarchy->instanceWritten (mayaObject))
			return true;
	}

	switch (m_type)
	{
	case T_cmp:				
		//-- Compute the sphere now.  This node itself doesn't contribute to the size,
		//   so this is okay.  Also, the write order guarantees that this node's children
		//   are written first, which makes this valid.
		computeSphere();	

		result = writeCmp (publish);  
		break;

	case T_lod:				
		//-- Compute the sphere now.  This node itself doesn't contribute to the size,
		//   so this is okay.  Also, the write order guarantees that this node's children
		//   are written first, which makes this valid.
		computeSphere();	

		result = writeLod (publish);  
		break;

	case T_msh:				result = writeMsh (publish);  break;
	case T_pob:				result = writePob (publish);  break;

	case T_collision:  
		{
			CollisionBuilder B(this);
			result = B.writeCollision(publish); 
			break;
		}
	case T_floor: 
		{
			// This has to be here to handle exporting of
			// older art assets that don't have collision in a
			// collision group.

			CollisionBuilder B(this);
			result = B.writeCollision(publish); 
			break;
		}

	// Trying to export a node with an uninitialized type is an error.

	case T_uninitialized:
		{	
			result = false; 
			MESSENGER_LOG_ERROR(("Node %s is of type U_uninitialized, should this node be here?", m_hierarchyName.c_str()));
			break;
		}
	// All other types aren't exportable directly.

	default:
		{
			result = true; 
			break;
		}
	}
	
	switch (m_type)
	{
		case T_cmp:
		case T_lod:
			// Do nothing, sphere already computed.
			break;

		default:
			// Compute sphere after write for everything else.
			computeSphere ();
			break;
	}

	return result;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::absorb(const MayaHierarchy::Node * child)
{
	setName(child->getName());
	setDesiredOrder(child->getDesiredOrder());
	setMayaChildIndex(child->getMayaChildIndex());
}

// ----------------------------------------------------------------------

bool childIndexLess( MayaHierarchy::Node const * const A, MayaHierarchy::Node const * const B )
{
	return A->getDesiredOrder() < B->getDesiredOrder();
}

bool MayaHierarchy::Node::validate (void) const
{
	bool result = true;

	// ----------
	// Pobs can't have cells with duplicate indices

	if(getType() == T_pob)
	{
		static std::vector<MayaHierarchy::Node const *> children;

		children.clear();

		for(int i = 0; i < getNumberOfChildren (); i++)
		{
			children.push_back( getChild(i) );
		}

		std::sort(children.begin(),children.end(),childIndexLess);

		{
			int nChildren = children.size();

			for(int i = 0; i < nChildren - 1; i++)
			{
				Node const * A = children[i];
				Node const * B = children[i+1];

				if(A->getDesiredOrder() == -1) continue;
				if(B->getDesiredOrder() == -1) continue;

				if(A->getDesiredOrder() == B->getDesiredOrder())
				{
					MESSENGER_LOG_WARNING(("Children with duplicate indices, %s and %s\n",A->getName(),B->getName()));
					result = false;
				}
			}
		}
	}

	// ----------
	// Collision groups can only have floors and extents in them

	if(getType() == T_collision)
	{
		for (int i = 0; i < getNumberOfChildren (); i++)
		{
			Node const * child = getChild(i);

			Type childType = child->getType();

			if((childType == T_floor) || (childType == T_extent))
			{
				continue;
			}
			else
			{
				MESSENGER_LOG_WARNING(("Collision group %s has child %s that isn't a floor or extent - check shape node name\n",getName(),child->getName()));
				return false;
			}
		}
	}

	// ----------

	for (int i = 0; i < getNumberOfChildren (); i++)
	{
		result &= getChild(i)->validate();
	}

	return result;
}

// ----------------------------------------------------------------------

const Vector &MayaHierarchy::Node::getMinBoxVector () const
{
	return m_minVector;
}

// ----------------------------------------------------------------------

const Vector &MayaHierarchy::Node::getMaxBoxVector () const
{
	return m_maxVector;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::Node::shouldCollectHardpoints() const
{
	Node const * const parent = getParent();

	//-- If we don't have a parent, we should collect hardpoints
	if (!parent)
		return true;

	//-- If our parent is a cell and it's the first cell in the building, we should collect the hardpoints
	if (parent->getType() == T_cel && parent->getBuildingCellIndex() == 0)
		return true;

	//-- We shouldn't collect hardpoints
	return false;
}

//===================================================================
//
//
MayaHierarchy::Instance* MayaHierarchy::Instance::create ()
{
	return new Instance ();
}

//-------------------------------------------------------------------

MayaHierarchy::Instance::Instance () :
	mayaObject (),
	engineName ()
{
}

//-------------------------------------------------------------------
	
MayaHierarchy::Instance::~Instance ()
{
}

//-------------------------------------------------------------------
	
MayaHierarchy::Instance& MayaHierarchy::Instance::operator= (const MayaHierarchy::Instance& rhs)
{
	if (this != &rhs)
	{
		mayaObject = rhs.mayaObject;
		engineName = rhs.engineName;
	}

	return *this;
}

//===================================================================
//
//
MayaHierarchy::MayaHierarchy (Messenger* newMessenger) :
	m_tree (0),
	m_rootMayaObject(),
	m_baseWriteDir(),
	m_appearanceWriteDir (),
	m_serverAppearanceWriteDir(),
	m_collisionWriteDir(),
	m_shaderTemplateWriteDir (),
	m_shaderTemplateReferenceDir (),
	m_effectReferenceDir (),
	m_textureReferenceDir (),
	m_textureWriteDir (),
	m_cmpWriteDir (),
	m_lodWriteDir (),
	m_mshWriteDir ()
{
	NOT_NULL (newMessenger);
	messenger = newMessenger;
}

//-------------------------------------------------------------------
	
MayaHierarchy::~MayaHierarchy ()
{
	if (m_tree)
	{
		delete m_tree;
		m_tree = 0;
	}
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getBaseName () const
{
	return m_tree ? m_tree->getName () : 0;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getBaseType () const
{
	return m_tree ? getTypeName(m_tree->getType ()) : 0;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getSubdirectoryName (Type type) 
{
	DEBUG_FATAL (type < 0 || type >= T_COUNT, ("invalid type"));
	return gNodeTypeInfoTable[type].m_subdirectory;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getExtension (Type type) 
{
	DEBUG_FATAL (type < 0 || type >= T_COUNT, ("invalid type"));
	return gNodeTypeInfoTable[type].m_extension;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getTypeName (Type type)
{
	DEBUG_FATAL (type < 0 || type >= T_COUNT, ("invalid type"));
	return gNodeTypeInfoTable[type].m_name;
}

//-------------------------------------------------------------------

static void createDirectory ( std::string const & directory )
{
	std::vector<char> buffer( directory.length() + 1, char(0) );

	char const * cursor1 = &directory[0];
	char * cursor2 = &buffer[0];

	//-- walk the name until / or \ is found, in which case create the directory
	while (*cursor1)
	{
		if (*cursor1 == '/' || *cursor1 == '\\')
		{
			::CreateDirectory (&buffer[0], 0);
		}

		*cursor2++ = *cursor1++;
	}

}

//-------------------------------------------------------------------

void MayaHierarchy::setAppearanceWriteDir (const char* newAppearanceWriteDir)
{
	FATAL(newAppearanceWriteDir == NULL,("MayaHierarchy::setAppearanceWriteDir - string is null\n"));

	// ----------
	// extract the base write directory from the given appearance write directory

	{
		std::string temp = newAppearanceWriteDir;

		std::string tempAppearance("appearance");

		int end = temp.find_last_of(tempAppearance);

		m_baseWriteDir = std::string(temp.begin(), temp.begin() + end - tempAppearance.length() + 1);
	}

	// ----------

	m_appearanceWriteDir.clear();

	m_appearanceWriteDir		= newAppearanceWriteDir;
	m_serverAppearanceWriteDir	= m_baseWriteDir + "server\\" + "appearance\\";
	
	m_collisionWriteDir	= m_appearanceWriteDir + getSubdirectoryName(T_collision);
	m_cmpWriteDir		= m_appearanceWriteDir + getSubdirectoryName(T_cmp);
	m_lodWriteDir		= m_appearanceWriteDir + getSubdirectoryName(T_lod);
	m_mshWriteDir		= m_appearanceWriteDir + getSubdirectoryName(T_msh);

	createDirectory (getAppearanceWriteDir());
	createDirectory (getServerAppearanceWriteDir());
	createDirectory (m_serverAppearanceWriteDir + "ssa\\");
	createDirectory (getCollisionWriteDir());
	createDirectory (getAppearanceCmpWriteDir());
	createDirectory (getAppearanceLodWriteDir());
	createDirectory (getAppearanceMshWriteDir());

	createDirectory (m_baseWriteDir + "log\\");
}

//-------------------------------------------------------------------

void MayaHierarchy::setShaderTemplateWriteDir (const char* newShaderTemplateWriteDir)
{
	m_shaderTemplateWriteDir.clear();

	if(newShaderTemplateWriteDir)
	{
		m_shaderTemplateWriteDir = newShaderTemplateWriteDir;
		createDirectory(m_shaderTemplateWriteDir);
	}
}

//-------------------------------------------------------------------

void MayaHierarchy::setShaderTemplateReferenceDir (const char* newShaderTemplateReferenceDir)
{
	m_shaderTemplateReferenceDir.clear();

	if (newShaderTemplateReferenceDir)
		m_shaderTemplateReferenceDir = newShaderTemplateReferenceDir;
}

//-------------------------------------------------------------------

void MayaHierarchy::setEffectReferenceDir (const char* newEffectReferenceDir)
{
	m_effectReferenceDir.clear();

	if (newEffectReferenceDir)
		m_effectReferenceDir = newEffectReferenceDir;
}

//-------------------------------------------------------------------

void MayaHierarchy::setTextureReferenceDir (const char* newTextureReferenceDir)
{
	m_textureReferenceDir.clear();

	if (newTextureReferenceDir)
		m_textureReferenceDir = newTextureReferenceDir;
}

//-------------------------------------------------------------------

void MayaHierarchy::setTextureWriteDir (const char* newTextureWriteDir)
{
	m_textureWriteDir.clear();

	if (newTextureWriteDir)
	{
		m_textureWriteDir = newTextureWriteDir;
		createDirectory (m_textureWriteDir);
	}
}

//-------------------------------------------------------------------

void MayaHierarchy::setPobFileName (const std::string &pobFileName)
{
	m_pobFileName = pobFileName;
}

//-------------------------------------------------------------------

const std::string &MayaHierarchy::getPobFileName () const
{
	return m_pobFileName;
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getAppearanceWriteDir () const
{
	return m_appearanceWriteDir.c_str();
}

// ----------------------------------------------------------------------

const char * MayaHierarchy::getServerAppearanceWriteDir () const
{
	return m_serverAppearanceWriteDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getAppearanceCmpWriteDir () const
{
	return m_cmpWriteDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getAppearanceLodWriteDir () const
{
	return m_lodWriteDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getAppearanceMshWriteDir () const
{
	return m_mshWriteDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getShaderTemplateWriteDir () const
{
	return m_shaderTemplateWriteDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getShaderTemplateReferenceDir () const
{
	return m_shaderTemplateReferenceDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getEffectReferenceDir () const
{
	return m_effectReferenceDir.c_str();
}

//-------------------------------------------------------------------

const char* MayaHierarchy::getTextureReferenceDir () const
{
	return m_textureReferenceDir.c_str();
}

// ----------------------------------------------------------------------

const char* MayaHierarchy::getTextureWriteDir () const
{
	return m_textureWriteDir.c_str();
}

// ----------------------------------------------------------------------

const char* MayaHierarchy::getCollisionWriteDir	 () const
{
	return m_collisionWriteDir.c_str();
}

//-------------------------------------------------------------------

void MayaHierarchy::addInstance (MObject mayaObject)
{
	int i;
	for (i = 0; i < static_cast<int>(m_instanceList.size()); ++i)
		if (m_instanceList [i]->mayaObject == mayaObject)
			return;

	Instance* const instance = Instance::create ();
	instance->mayaObject = mayaObject;
	m_instanceList.push_back (instance);
}

//-------------------------------------------------------------------

MString MayaHierarchy::getInstanceName (MObject mayaObject, const MString& engineName) const
{
	//-- search the instance list
	int i;
	for (i = 0; i < static_cast<int>(m_instanceList.size()); ++i)
		if (m_instanceList [i]->mayaObject == mayaObject)
			break;

	DEBUG_FATAL (i == static_cast<int>(m_instanceList.size()), ("node marked as an instance, but instance entry was not found"));

	//-- the first instance found is NOT an instance. 
	//	this is so we can get the appearance written
	if (m_instanceList [i]->engineName.length () == 0)
		m_instanceList [i]->engineName = engineName;

	//-- subsequent instances are true instances and should 
	//	just return the name of the previously written appearance
	return m_instanceList [i]->engineName;
}

//-------------------------------------------------------------------

bool MayaHierarchy::instanceWritten (MObject mayaObject) const
{
	//-- search the instance list
	int i;
	for (i = 0; i < static_cast<int>(m_instanceList.size()); ++i)
		if (m_instanceList [i]->mayaObject == mayaObject)
			break;

	DEBUG_FATAL (i == static_cast<int>(m_instanceList.size()), ("node marked as an instance, but instance entry was not found"));

	return m_instanceList [i]->engineName.length () != 0;
}

// ----------------------------------------------------------------------

bool	MayaHierarchy::decodeNameString		( const char * inName, NodeInfo & nodeInfo )
{
	NOT_NULL (inName);

	char name[200];
	strcpy(name,inName);

	convertToLowerCase(name);

	// ----------
	// The name has to be made of a bunch of letters followed by a bunch of digits,
	// so count the letters and digits to see if we have enough of each

	const char * cursor = name;

	int nLetters1 = 0;
	int nDigits = 0;
	int nLetters2 = 0;

	while(isLetter(*cursor))	{ nLetters1++; cursor++; }
	while(isDigit (*cursor))	{ nDigits++;   cursor++; }
	while(isLetter(*cursor))	{ nLetters2++; cursor++; }

	// If the name didn't start with letters, it's an invalid name

	if(nLetters1 == 0) return false;

	// If the name didn't end with a NULL or an underscore, 
	// it's an invalid name.

	if((*cursor != 0) && (*cursor != '_')) return false;

	if(*cursor == '_')
	{
		nLetters1++;
		nLetters1--;
	}

	// ----------
	// Name is in valid <string><number><string> format. Extract the strings and number.

	char stringBuffer1[200];
	char numberBuffer[200];
	char stringBuffer2[200];

	memcpy( stringBuffer1, name, nLetters1 );
	memcpy( numberBuffer,  name+nLetters1, nDigits );
	memcpy( stringBuffer2, name+nLetters1+nDigits, nLetters2 );

	stringBuffer1[nLetters1] = 0;
	numberBuffer[nDigits] = 0;
	stringBuffer2[nLetters2] = 0;

	// ----------
	// HACK - Sometimes Maya names a shape node "floorShape0" instead of "floor0Shape".
	// Detect this and fix it.

	if(stringEndsWith(stringBuffer1,"shape"))
	{
		nLetters1 -= 5;
		stringBuffer1[nLetters1] = 0;

		memcpy(stringBuffer2,"shape",5);
		nLetters2 = 5;
		stringBuffer2[nLetters2] = 0;
	}
	else if((nLetters2 == 0) && stringEndsWith(inName,"shape"))
	{
		// we also need to handle the case where the node's name is blah0_fooShape

		memcpy(stringBuffer2,"shape",5);
		nLetters2 = 5;
		stringBuffer2[nLetters2] = 0;
	}

	// ----------
	// If there's a second set of letters, it must spell "shape", or it's an invalid name

	bool bIsShape = (strcmp(stringBuffer2,"shape") == 0);

	if((nLetters2 != 0) && !bIsShape) return false;
	
	// ----------
	// Done parsing the name, so search for the string in our decode table

	for(int i = 0; i < nDecodeTable_Size; i++)
	{
		if( strcmp(stringBuffer1,gDecodeTable[i].m_name) == 0 ) 
		{
			// Found the string, write the output and we're done

			nodeInfo.m_type = gDecodeTable[i].m_type;
			nodeInfo.m_isShapeNode = bIsShape;

			if(nDigits == 0)
			{
				// The node name didn't contain any digits, so use the default index
				// from the decode table.

				MESSENGER_REJECT(gDecodeTable[i].m_index == -1, 
								("MayaHierarchy::decodeNodeName_StringNumber - Node %s requires an index (blah0,blah1,etc.), didn't get one",inName));

				nodeInfo.m_index = gDecodeTable[i].m_index;
			}
			else
			{
				nodeInfo.m_index = atoi(numberBuffer);
			}

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------
// Determine the type and index for a node given the node's name

bool MayaHierarchy::decodeNodeName		( const char* name, NodeInfo& nodeInfo )
{
	if( decodeNameString(name,nodeInfo) ) return true;

	nodeInfo.m_type = T_uninitialized;
	nodeInfo.m_index = -1;

	return false;
}

// ----------------------------------------------------------------------

bool MayaHierarchy::determineNodeType	( Node * pNode, NodeInfo & nodeInfo )
{
	nodeInfo.m_type = T_uninitialized;
	nodeInfo.m_index = -1;

	// ----------
	// Try to determine the node's type by parsing its name. If its name
	// matches one of our hardcoded types, use that type for the node.

	NodeInfo nameInfo;
	
	IGNORE_RETURN( decodeNodeName( pNode->getName(), nameInfo ) );

	MayaHierarchy::Type type = nameInfo.m_type;

	if(nameInfo.m_isShapeNode)
	{
		// Shape nodes whose types are completely determined by their names

		if( (type == T_floor) || (type == T_cylinder) || (type == T_box) || (type == T_cmesh) || (type == T_sphere) )
		{
			nodeInfo = nameInfo;
			return true;
		}
	}
	else
	{
		// Non-shape nodes whose types are completely determined by their names

		if( (type == T_collision) || (type == T_extent) || (type == T_lightList) )
		{
			nodeInfo = nameInfo;
			return true;
		}
	}
	
	// ----------
	// Determining the node's type from its name failed. Look at the Maya
	// types and the attributes on the nodes to determine the node type.

	MStatus status;

	MDagPath dagPath = pNode->getMayaDagPath ();

	MObject object = dagPath.node (&status);
	if (!status) return false;

	MFnDagNode dagNode (dagPath, &status);
	if (!status) return false;

	// Mesh node types

	if (object.apiType () == MFn::kMesh)
	{
		// ----------
		// If it has a "portal" attribute, it's a portal

		MObject portal = dagNode.attribute ("portal", &status);

		if (status == MS::kSuccess)
		{
			nodeInfo.m_type = T_prt;
			return true;
		}

		// ----------
		// If it has a "sprite" attribute, ignore it

		MObject sprite = dagNode.attribute ("sprite", &status);

		if (status == MS::kSuccess)
		{
			nodeInfo.m_type = T_uninitialized;
			return false;
		}

		// ----------
		// Otherwise it's not a special mesh, just a 
		// plain ol' boring mesh.

		nodeInfo.m_type = T_msh;
		return true;
	}
	else
	{
		// ----------
		// Non-mesh types
		
		if (object.apiType () == MFn::kAmbientLight || object.apiType () == MFn::kDirectionalLight || object.apiType () == MFn::kPointLight)
		{
			nodeInfo.m_type = T_light;
			return true;
		}

		// ----------
		// If it has an external reference plug and a value, it's a
		// external reference

		MPlug externalPlug = dagNode.findPlug ("external_reference", &status);

		if (status == MS::kSuccess)
		{
			// It does, so see if that plug has a value

			MString externalReferenceName;
			status = externalPlug.getValue (externalReferenceName);

			if (status == MS::kSuccess)
			{
				// Got the value. The node has to be an external reference.

				nodeInfo.m_type = T_ext;
				return true;
			}
		}
	}

	// ----------
	// Couldn't determine the node's type

	nodeInfo.m_type = T_uninitialized;
	return false;
}

// ----------------------------------------------------------------------

bool	MayaHierarchy::MayaTransform_to_Transform	( MObject & object, Transform & out )
{
	MStatus status;

	MFnTransform mayaTransform (object, &status);
	if (!status)
		return false;

	MMatrix matrix = mayaTransform.transformation (&status).asMatrix ();
	if (!status)
		return false;

	Quaternion orientation;
	Vector     position;
	if (!MayaUtility::decomposeMatrix (&matrix, &orientation, &position))
		return false;

	orientation.getTransform (&out);
	out.setPosition_p (position);

	return true;
}

// ----------------------------------------------------------------------
// Create child nodes for the given node

// This method is mutually recursive with setupNode

bool MayaHierarchy::createChildren ( Node * node )
{
	MStatus status;

	MDagPath dagPath = node->getMayaDagPath ();

	MESSENGER_INDENT;
	const int n = dagPath.childCount (&status);
	if (!status)
		return false;

	for(int i = 0; i < n; i++)
	{
		//-- find out what type of node this is
		MObject childObject = dagPath.child (i, &status);
		if (!status) return false;

		//-- get dag path for transform
		MDagPath childDagPath = dagPath;
		status = childDagPath.push (childObject);
		if (!status) return false;

		{
			//-- get the dag node to get the node name
			MFnDagNode childDagNode (childDagPath, &status);
			if (!status) return false;

			//-- is this an intermediate mesh?
			const bool isIntermediateMesh = childDagNode.isIntermediateObject (&status);
			if (!status) return false;

			if (isIntermediateMesh)
				continue;

			//-- is this a hardpoint node?
			MString childDagNodeName = childDagNode.name(&status);
			if (!status) 
				return false;

			char const * const name = childDagNodeName.asChar();
			if (name[0] == 'h' && name[1] == 'p')
				continue;
		}

		//-- create the child node
		Node* childNode = new Node (this);
		childNode->setMayaDagPath (childDagPath);
		childNode->setMayaChildIndex(i);

		// ----------

		if (!setupNode (childNode,node))
		{
			delete childNode;
			return false;
		}

		node->addChild (childNode);
	}

	return true;
}

// ----------------------------------------------------------------------

void MayaHierarchy::setChildOrderIndices( Node * root )
{
	for (int i = 0; i < root->getNumberOfChildren (); i++)
	{
		Node* const child = root->getChild (i);

		NodeInfo childInfo;
		decodeNodeName (child->getName (), childInfo);
		child->setDesiredOrder (childInfo.m_index);
	}
}

// ----------------------------------------------------------------------
// The types of some nodes are determined by the children they contain -
// for example, if a group node contains portal nodes as children, then
// the group node must be a portal list. 

bool MayaHierarchy::setNodeTypeFromChildren ( Node * root )
{
	// These node types aren't determined by their child types

	MayaHierarchy::Type type = root->getType();

	if(type == T_collision) return true;
	if(type == T_extent) return true;
	if(type == T_floor) return true;

	// ----------

	for (int i = 0; i < root->getNumberOfChildren (); i++)
	{
		Node* const child = root->getChild (i);

		if (child->getNumberOfChildren () == 0)
		{
			continue;
		}

		NodeInfo childInfo;
		decodeNodeName (child->getName (), childInfo);

		// ----------
		// These node types don't propagate up to their parents

		if(childInfo.m_type == T_floor)		{ continue; }
		if(childInfo.m_type == T_collision)	{ continue; }
		if(childInfo.m_type == T_lightList) { continue; }

		// ----------
		// These node types are sticky - once a node is set to one of these types by
		// its children, it doesn't change.

		if(root->getType() == T_cel)       { continue; }
		if(root->getType() == T_lightList) { continue; }

		// ----------
		
		switch (childInfo.m_type)
		{
		default:
			root->setType (childInfo.m_type);
			break;

		case T_prt:
			root->setType (T_pls);
			break;

		case T_pls:
			root->setType (T_cel);
			break;

		case T_cel:
			root->setType (T_pob);
			break;
		}
	}

	return true;
}



//-------------------------------------------------------------------
// This method recursively creates a MayaHierarchy::Node 
// for each Maya node in the hypergraph, but its main job is to determine export 
// types for each of those nodes and to do some node-specific processing for them.

bool MayaHierarchy::setupNode (Node * root, Node * parent)
{
//	MESSENGER_LOG(("Setting up node %s\n",root->getName()));

	MStatus status;

	MDagPath dagPath = root->getMayaDagPath ();

	MObject object = dagPath.node (&status);
	if (!status)
		return false;

	//-- get the dag node to get the node name
	MFnDagNode dagNode (dagPath, &status);
	if (!status)
		return false;

	//-- is this node an instance?
	{
		const bool isInstanced = dagNode.isInstanced (true, &status);
		if (!status)
			return false;

		//-- tag this node as instanced
		if (isInstanced)
		{
			root->setInstanced (true);

			addInstance (object);
		}
	}

	//-- get the node name
	MString dagNodeName = dagNode.name(&status);
	if (!status) return false;

	root->setName(dagNodeName.asChar());

	//-- create hierarchy name

	MString hierarchyName;
	
	if(parent)
	{
		hierarchyName += parent->getHierarchyName();
		hierarchyName += "/";
	}

	hierarchyName += root->getName();
	root->setHierarchyName( hierarchyName.asChar() );

	// ----------
	// extract transform

	MObject mayaTransform = dagPath.transform (&status);
	if (status == MS::kFailure)
		return false;

	Transform  transform;

	if( !MayaTransform_to_Transform(mayaTransform,transform) )
		return false;

	root->setTransform(transform);

	// ----------

	NodeInfo nodeInfo;

	if( determineNodeType(root,nodeInfo) )
	{
		root->setType(nodeInfo.m_type);
	}

	// ----------
	
	if(root->getType() == T_ext)
	{
		MPlug externalPlug = dagNode.findPlug ("external_reference", &status);
		MESSENGER_REJECT(status.error(), ("Could not get plug for external reference node"));
		MString externalReferenceName;
		status = externalPlug.getValue (externalReferenceName);
		MESSENGER_REJECT(status.error(), ("Could not get name of external reference"));
		root->setExternalReferenceName (externalReferenceName.asChar());
	}

	//-- create children

//	MESSENGER_LOG(("Node %s has type %s, setting up children\n",root->getName(), getTypeName(root->getType()) ));

	bool childCreateOK = createChildren(root);

	if(!childCreateOK)
	{
		return false;
	}

	// Use the types decoded from the names of this node's children to determine
	// this node's type

	bool nodeCheckOK = setNodeTypeFromChildren(root);

	if(!nodeCheckOK)
	{
		return false;
	}

	// ----------
	// The node's type should be completely determined. Do postprocessing now.

	setChildOrderIndices(root);

	if(root->getType() == T_cel)
	{
		MStatus status;

		MFnDagNode dagNode;

		if(!root->getMayaDagNodeFn(dagNode)) return false;

		IGNORE_RETURN( dagNode.attribute ("canSeeWorld", &status) );

		if (status == MS::kSuccess)
		{
			root->setCanSeeWorldCell();
		}
	}

//	MESSENGER_LOG(("Node %s set up with type %s\n",root->getName(), getTypeName(root->getType()) ));

	return true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::populateTree (const MSelectionList &transformList)
{
	MStatus status;

	const unsigned int transformCount = transformList.length(&status);

	if (transformCount != 1)
		return false;

	//-- get dag path for transform
	MDagPath transformDagPath;
	MObject component;
	status = transformList.getDagPath (0, transformDagPath, component);
	MESSENGER_REJECT(status.error(), ("Could not get DAG Path: %s", status.errorString().asChar()));

	//-- get the dag node to get the node name
	MFnDagNode transformDagNode (transformDagPath, &status);
	MESSENGER_REJECT(status.error(), ("Could not get DAG Node: %s", status.errorString().asChar()));

	messenger->logMessage("Node type: %s\n", transformDagNode.typeName().asChar());

	//-- get the node name
	MString transformDagNodeName = transformDagNode.name (&status);
	MESSENGER_REJECT(status.error(), ("Could not get DAG Node Name: %s", status.errorString().asChar()));

	messenger->logMessage("Node named: %s\n", transformDagNodeName.asChar());

	m_tree = new Node (this);
	m_tree->setName (transformDagNodeName.asChar());
	m_tree->setHierarchyName (transformDagNodeName.asChar());
	m_tree->setMayaDagPath (transformDagPath);

	//-- get the object from the node
	MObject rootMayaObject = transformDagPath.node(&status);
	MESSENGER_REJECT(status.error(), ("Could not get root maya object: %s", status.errorString().asChar()));

	m_rootMayaObject = MayaUtility::getRootMayaObject(rootMayaObject, &status);
	MESSENGER_REJECT(status.error(), ("Could not get root maya object: %s", status.errorString().asChar()));

	MESSENGER_REJECT(!setupNode (m_tree,NULL), ("Couldn't create tree node\n"));

	return true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::sortNode (Node* root)
{
	int n = root->getNumberOfChildren ();

	Type type = root->getType();

	int i;
	for (i=0; i<n; ++i)
	{
		sortNode(root->getChild(i));
	}

	//-- reorder nodes in terms of listed order
	if (type == T_lod || type == T_cmp || type == T_prt || type == T_cel || type == T_pob || type == T_collision)
	{
		int i;
		int j;
		for (i = 0; i < n - 1; i++)
		{
			for (j = i + 1; j < n; j++)
			{
				const int o1 = root->getChild (i)->getDesiredOrder ();
				const int o2 = root->getChild (j)->getDesiredOrder ();
				
				if ((type == T_lod) || (type == T_collision))
				{
					//-- lod nodes and collision nodes are sorted highest number to lowest
					if (o1 < o2)
						root->swapChildren (i, j);
				}
				else
				{
					//-- cmp, prt, cel, and pob nodes are sorted lowest to highest
					if (o1 > o2)
						root->swapChildren (i, j);
				}
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------

MayaHierarchy::Node * MayaHierarchy::promoteNode (Node* root)
{
	Node * result = 0;
	int i;

	for (i=0; i<root->getNumberOfChildren(); ++i)
	{
		Node* const child = root->getChild (i);

		Node * replace = promoteNode(child);
		if (replace)
		{
			replace->absorb(child);
			delete child;
			root->setChild(i, replace);
		}
	}

	while (root->getType() == T_uninitialized)
	{
		if (root->getNumberOfChildren())
		{
			// replace root with first child, reparent all siblings to underneath root
			result = root->getChild(0);
			for (i=1; i < root->getNumberOfChildren(); ++i)
			{
				result->addChild(root->getChild(i));
			}
			root->removeAllChildren();
			root = result;
		}
		else
		{
			break;
		}
	}

	return result;
}

//-------------------------------------------------------------------

bool MayaHierarchy::fixupNames (Node* root)
{
	int i;
	for (i = 0; i < root->getNumberOfChildren (); i++)
	{
		Node* const child = root->getChild (i);

		if (!child->hasName())
			return false;

		MString childName;
		childName  = root->getName ();
		childName += "_";
		childName += child->getName ();

		child->setName (childName.asChar());

		fixupNames (child);
	}

	return true;
}

//-------------------------------------------------------------------

MObject const & MayaHierarchy::getRootMayaObject() const
{
	return m_rootMayaObject;
}

//-------------------------------------------------------------------

bool MayaHierarchy::build (const MSelectionList &transformList)
{
	//-- extract the data from maya
	if (!populateTree (transformList))
		return false;

	dump (m_tree);

	//-- delete excess nodes
	Node * result = promoteNode (m_tree);
	if (result)
	{
		result->absorb(m_tree);
		delete m_tree;
		m_tree = result;
		m_tree->setParent(0);
	}

	sortNode(m_tree);

	//-- fixup the names so that the object files can be created
	if (!fixupNames (m_tree))
		return false;

	bool validateOK = m_tree->validate();

	MESSENGER_REJECT(!validateOK,("Node tree did not validate OK - check warnings\n"));

	ExporterLog::buildStaticMesh(m_tree);

	dump (m_tree);

	return true;
}

//-------------------------------------------------------------------

void MayaHierarchy::dump (const Node* root, const int indentLevel) const
{
	const char* const name = root->getName ();

	char buffer [1000];

	int i;
	for (i = 0; i < indentLevel * 2; i++)
		buffer [i] = ' ';
	buffer [i] = 0;

	strcat (buffer, name);
	strcat (buffer, " - ");
	strcat (buffer, getTypeName(root->getType ())),
	strcat (buffer, root->getInstanced () ? " i " : " ");

	MESSENGER_LOG (("%s\n", buffer));

	for (i = 0; i < root->getNumberOfChildren (); i++)
	{
		DEBUG_FATAL(root->getChild(i)->getParent() != root, ("Bad parent node"));
		dump (root->getChild (i), indentLevel + 1);
	}
}

//-------------------------------------------------------------------

void MayaHierarchy::dump () const
{
	dump (m_tree);
}

//-------------------------------------------------------------------
/**
 * Given a root node, build and write the corresponding files to disk
 *
 * @return true on success, false on non-fatal error
 */
bool MayaHierarchy::write (const Node* root, bool publish) const
{
	ExporterLog::pushCurrentObject(root->getName());

	bool writeSuccess = true;
	bool writeChildren = true;

	if (root->getType() == T_collision)
		writeChildren = false;

	// Portal indices need to be assigned for pobs before any of the children are written.
	if (root->getType() == T_pob)
	{
		bool result = preprocessPobNode(const_cast<Node *>(root));

		if (!result)
			return false;
	}

	//-- children get written first because extents need to be calculated
	if (writeChildren)
	{
		if (root->getType() != T_pls)
		{
			for (int i = 0; i < root->getNumberOfChildren (); i++)
			{
				bool result = write (root->getChild (i), publish);
				if (!result)
					writeSuccess = false;
			}
		}
	}

	//-- write the parent last
	bool result = root->write (publish);
	if (!result)
		writeSuccess = false;

	ExporterLog::popCurrentObject();

	return writeSuccess;
}

//-------------------------------------------------------------------

bool MayaHierarchy::writeClientApt ( Node const * node ) const
{
	writeAptFile( node->getAptFilename(), node->getAppearanceReferenceName() );

	ExporterLog::addClientDestinationFile(node->getAptFilename().asChar());

	return true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::writeServerApt ( Node const * node ) const
{
	writeAptFile( node->getServerAptFilename(), node->getServerAppearanceTemplateReferenceName() );

	ExporterLog::addServerDestinationFile(node->getServerAptFilename().asChar());

	return true;
}

//-------------------------------------------------------------------

bool MayaHierarchy::writeServerAppearanceTemplate ( Node const * node ) const
{
	MeshBuilder builder(getRootMayaObject());

	AxialBox box = Containment3d::EncloseABox(node->getSphere());

	builder.attachExtent( new BoxExtent(box) ) ;

	builder.addCollisionNode(node);

	builder.collectHardpoints();

	Iff iff(1024);
	builder.write(iff);

	// ----------

	MString writeName = node->getServerAppearanceTemplateFilename();
	ExporterLog::addServerDestinationFile(writeName.asChar());
	return iff.write (writeName.asChar());
}

//-------------------------------------------------------------------

bool MayaHierarchy::writeServerAppearanceTemplates () const
{
	if(m_tree->isAppearance())
	{
		writeServerAppearanceTemplate(m_tree);
		writeServerApt(m_tree);
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Build and write the entire hierarchy to disk
 *
 * @return true on success, false on non-fatal error
 */
bool MayaHierarchy::write(bool const publish) const
{
	bool const result = write(m_tree, publish);

	if (publish && result)
	{
		if (m_tree->isAppearance())
			writeClientApt(m_tree);

		writeServerAppearanceTemplates();
	}

	return result;
}

//-------------------------------------------------------------------

void MayaHierarchy::Node::gatherChildren(MSelectionList &sels) const
{
	sels.add(getMayaDagPath());
	int x;
	for (x=0; x<getNumberOfChildren(); ++x)
	{
		getChild(x)->gatherChildren(sels);
	}
}

// ----------------------------------------------------------------------

bool MayaHierarchy::testMatchingPortals(IndexedTriangleList const & portal1, IndexedTriangleList const & portal2, float const epsilon)
{
	std::vector<Vector> const & portal1Vertices = portal1.getVertices();
	std::vector<int> const & portal1Indices = portal1.getIndices();
	uint const numberOfPortal1Faces = portal1Indices.size() / 3;
	std::vector<Vector> const & portal2Vertices = portal2.getVertices();
	std::vector<int> const & portal2Indices = portal2.getIndices();
	uint const numberOfPortal2Faces = portal2Indices.size() / 3;

	//-- Make sure they have the same number of vertices
	if (portal1.getVertices().size() != portal2.getVertices().size())
		return false;

	//-- Make sure they have the same number of faces
	if (numberOfPortal1Faces != numberOfPortal2Faces)
		return false;

	//-- Make sure the face count is non-zero
	if (numberOfPortal1Faces == 0)
		return false;

	//-- Save off face indices for polygon 1
	std::set<uint> polygon1FaceIndexSet;
	{
		for (uint i = 0; i < numberOfPortal1Faces; ++i)
			polygon1FaceIndexSet.insert(i);
	}

	//-- Run through all of polygon2 faces comparing them to polygon1 faces.  If a match is found, remove that face index from the face index set.
	{
		std::vector<Vector> polygon1;
		std::vector<Vector> polygon2;

		for (uint portal2FaceIndex = 0, portal2Index = 0; portal2FaceIndex < numberOfPortal2Faces; ++portal2FaceIndex)
		{
			bool found = false;

			polygon2.clear();
			polygon2.push_back(portal2Vertices[portal2Indices[portal2Index++]]);
			polygon2.push_back(portal2Vertices[portal2Indices[portal2Index++]]);
			polygon2.push_back(portal2Vertices[portal2Indices[portal2Index++]]);

			for (uint portal1FaceIndex = 0, portal1Index = 0; portal1FaceIndex < numberOfPortal1Faces; ++portal1FaceIndex)
			{
				polygon1.clear();
				polygon1.push_back(portal1Vertices[portal1Indices[portal1Index++]]);
				polygon1.push_back(portal1Vertices[portal1Indices[portal1Index++]]);
				polygon1.push_back(portal1Vertices[portal1Indices[portal1Index++]]);

				if (testMatchingPolygons(polygon1, polygon2, epsilon))
				{
					found = true;
					break;
				}
			}

			if (found)
				polygon1FaceIndexSet.erase(portal2FaceIndex);
		}
	}

	//-- An empty face index set means we've matched all of the faces
	return polygon1FaceIndexSet.empty();
}

//===================================================================
