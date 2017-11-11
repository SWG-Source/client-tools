// ======================================================================
//
// MeshBuilder.cpp
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MeshBuilder.h"

#include "ExporterLog.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "FloorBuilder.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/ComponentExtent.h"
#include "sharedCollision/CylinderExtent.h"
#include "sharedCollision/DetailExtent.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/MeshExtent.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Vector2d.h"
#include "sharedObject/Hardpoint.h"

#include "maya/MDagPath.h"
#include "maya/MGlobal.h"
#include "maya/MMatrix.h"
#include "maya/MObject.h"
#include "maya/MSelectionList.h"
#include "maya/MFnDagNode.h"
#include "maya/MFnTransform.h"
#include "maya/MMatrix.h"
#include "maya/MFnMesh.h"

#include <algorithm>

static const Tag TAG_HPTS = TAG (H,P,T,S);
static const Tag TAG_HPNT = TAG (H,P,N,T);
static const Tag TAG_APPR = TAG (A,P,P,R);
static const Tag TAG_NULL = TAG (N,U,L,L);
static const Tag TAG_FLOR = TAG (F,L,O,R);

#define MESSENGER_REJECT_NORETURN(a,b)                  \
	if (a)                                       \
	{                                            \
		messenger->enableIndent(false);            \
		messenger->enableFileAndLineDisplay(true); \
		messenger->setFileName(__FILE__);          \
		messenger->setLineNumber(__LINE__);        \
		messenger->logErrorMessage b;              \
	}

extern Messenger * messenger;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(MeshBuilder::Face, false, 128, 0, 0);

// ======================================================================

void MeshBuilder::install(Messenger *newMessenger)
{
	messenger = newMessenger;

	MeshBuilder::Face::install();
}

// ----------------------------------------------------------------------

void MeshBuilder::remove(void)
{
	messenger = 0;
}

// ----------------------------------------------------------------------

bool MeshBuilder::collectHardpoints(MObject const * const rootMayaObject, HardpointVec & outArray, char const * const prefix, bool const spam)
{
	MESSENGER_INDENT;

	// find locators starting with the given prefix, extract transform and add to ouptut array

	MSelectionList  mayaHardpoints;
	MStatus         status;
	
	std::string selectionName = std::string(prefix) + "*";

	status = MGlobal::getSelectionListByName(selectionName.c_str(), mayaHardpoints);
	if (!status)
	{
		// no hardpoints
		return true;
	}

	MDagPath    hardpointDag;
	MObject     hardpointObject;
	MMatrix     hardpointMatrix;
	bool        result = true;
	Quaternion  orientation;
	Vector      position;

	char        hardpointName[128];
	Transform   hardpointTransform;

	const unsigned hardpointCount = mayaHardpoints.length();
	for (unsigned index = 0; index < hardpointCount; ++index)
	{
		status = mayaHardpoints.getDagPath(index, hardpointDag);
		if (!status)
		{
			// probably not a dag (in which case it can't be a hardpoint locator).  ignore it.
			continue;
		}

		hardpointObject = hardpointDag.node(&status);
		MESSENGER_REJECT(!status, ("failed to get dag hardpoint,\"%s\"\n", status.errorString().asChar()));
		if (hardpointObject.apiType() != MFn::kLocator)
		{
			// not a locator, ignore (good spot for a warning)
			continue;
		}

		MObject mayaTransform = hardpointDag.transform (&status);
		if (status == MS::kFailure) return false;

		IGNORE_RETURN( MayaHierarchy::MayaTransform_to_Transform(mayaTransform,hardpointTransform) );

		MFnDependencyNode hardpointDep(hardpointObject,&status);
		if(!status) return NULL;

		//-- Determine if the hardpoint should be added
		{
			//-- See if the hardpoint is a global hardpoint
			MFnDagNode hardpointDagNode(hardpointDag, &status);
			if (!status)
				return false;

			MObject hardpointDagRoot = MayaUtility::getRootMayaObject(hardpointObject, &status);
			if (!status)
				return false;

			MObject hardpointObjectParent = hardpointDagNode.parent(0, &status);
			if (!status)
				return false;

			//-- if the root object is also the hardpoint object, we don't have a root
			if (hardpointDagRoot != hardpointObjectParent)
			{
				//-- If the root dag is not the root we were constructed with, 
				//   we're a hardpoint from another hierarchy
				if (rootMayaObject && hardpointDagRoot != *rootMayaObject)
					continue;
			}
		}

		Vector localPos;

		result = result & MayaUtility::getNodeFloatValue(hardpointDep,"localPositionX",&localPos.x);
		result = result & MayaUtility::getNodeFloatValue(hardpointDep,"localPositionY",&localPos.y);
		result = result & MayaUtility::getNodeFloatValue(hardpointDep,"localPositionZ",&localPos.z);

		MESSENGER_REJECT(!result, ("failed to extract local position from locator\n"));

		localPos.x = -localPos.x;

		Vector worldPos = hardpointTransform.rotateTranslate_l2p(localPos);

		hardpointTransform.setPosition_p(worldPos);

		// get the name
		result = MayaUtility::getNodeName(hardpointObject, hardpointName, sizeof(hardpointName));
		MESSENGER_REJECT(!result, ("failed to get name of matrix\n"));

		//remove prefix from the hardpoint name if found
		std::string stdHpName = hardpointName;
		std::string::size_type pos = stdHpName.find(prefix);
		if(pos != std::string::npos)
		{
			stdHpName = stdHpName.substr(pos + 3);
		}

		//remove "Shape" from the hardpoint name if found
		pos = stdHpName.find("Shape");
		if(pos != std::string::npos)
		{
			stdHpName.erase( stdHpName.begin() + pos, stdHpName.begin() + pos + 5 );
		}

		Hardpoint *const hp = new Hardpoint(stdHpName.c_str(), hardpointTransform);
		outArray.push_back(hp);

		// hack - don't spam the found hardpoint message for path node and door hardpoints

		char const * const stdHpName_cstr = stdHpName.c_str();

		if(spam && (strncmp(stdHpName_cstr,"path",4) != 0) && (strncmp(stdHpName_cstr,"door",4) != 0))
		{
			MESSENGER_LOG(("found hardpoint \"%s\"\n", stdHpName.c_str()));
		}
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Compute the dot3 transform appropriate for this triangle face.
 *
 * @param face                  the face for which the dot3 transform will be computed.
 *                              Dot3 values are returned in face.
 */

void MeshBuilder::computeDot3Transform(Face &face, int normalMapTextureCoordinateIndex)
{
	face.dot3TransformMatrixValid = true;
	face.du = DoubleVector::zero;
	face.dv = DoubleVector::zero;

	double lengths[3];

	lengths[0] = face.positionArray[0].magnitudeBetweenSquared(face.positionArray[1]);
	lengths[1] = face.positionArray[1].magnitudeBetweenSquared(face.positionArray[2]);
	lengths[2] = face.positionArray[2].magnitudeBetweenSquared(face.positionArray[0]);

	double vlengths[3];

	vlengths[0] = lengths[0] + lengths[2];
	vlengths[1] = lengths[0] + lengths[1];
	vlengths[2] = lengths[1] + lengths[2];

	int triIndeces[3] = { 0, 1, 2 };
	if (vlengths[0] > vlengths[1])
	{
		if (vlengths[0] > vlengths[2])
		{
			// max is 0
			triIndeces[0] = 1;
			triIndeces[1] = 2;
			triIndeces[2] = 0;
		}
		else
		{
			// max is 2
			triIndeces[0] = 0;
			triIndeces[1] = 1;
			triIndeces[2] = 2;
		}
	}
	else if (vlengths[1] > vlengths[2])
	{
		// max is 1
		triIndeces[0] = 2;
		triIndeces[1] = 0;
		triIndeces[2] = 1;
	}
	else
	{
		// max is 2
		triIndeces[0] = 0;
		triIndeces[1] = 1;
		triIndeces[2] = 2;
	}

	// get the directional vector in 3d space
	const Vector p0 = face.positionArray[triIndeces[0]];
	const Vector p1 = face.positionArray[triIndeces[1]];
	const Vector p2 = face.positionArray[triIndeces[2]];
	const double  u0 = face.uArray[normalMapTextureCoordinateIndex][triIndeces[0]];
	const double  v0 = face.vArray[normalMapTextureCoordinateIndex][triIndeces[0]];
	const double  u1 = face.uArray[normalMapTextureCoordinateIndex][triIndeces[1]];
	const double  v1 = face.vArray[normalMapTextureCoordinateIndex][triIndeces[1]];
	const double  u2 = face.uArray[normalMapTextureCoordinateIndex][triIndeces[2]];
	const double  v2 = face.vArray[normalMapTextureCoordinateIndex][triIndeces[2]];

	//////////////////////////////////////////////////////////////////////////
	double tu1, tv1, tu2, tv2;

	tu1 = double(u1) - double(u2);
	tv1 = double(v1) - double(v2);

	tu2 = double(u0) - double(u2);
	tv2 = double(v0) - double(v2);

   double c = tv1*tu2 - tu1*tv2;
   if (c==0) c=.000001f;

   // solve for coefficients that give us the u==v line.
	//const double k1 = (tu2 - tv2) / c;
	//const double k2 = (tv1 - tu1) / c;

   // solve for coefficients that give us the u==0 and v==0 lines
	double coeffs[4];
   coeffs[0] = -tv2 / c;
   coeffs[1] =  tv1 / c;
   coeffs[2] =  tu2 / c;
   coeffs[3] = -tu1 / c;

	//////////////////////////////////////////////////////////////////////////
	double e1[3], e2[3];

	e1[0] = double(p1.x) - double(p2.x);
	e1[1] = double(p1.y) - double(p2.y);
	e1[2] = double(p1.z) - double(p2.z);

	e2[0] = double(p0.x) - double(p2.x);
	e2[1] = double(p0.y) - double(p2.y);
	e2[2] = double(p0.z) - double(p2.z);


   // apply coefficients to vertex positions to get tangent space vectors corresponding
   // to the u==v texture line
	//Vector ueqv = float(k1)*e1 + float(k2)*e2;
	//ueqv.normalize();

   // apply coefficients to vertex positions to get tangent space vectors corresponding
   // to the primary texture axes.
	double du[3], dv[3];

	du[0] = e1[0]*coeffs[0] + e2[0]*coeffs[1];
	du[1] = e1[1]*coeffs[0] + e2[1]*coeffs[1];
	du[2] = e1[2]*coeffs[0] + e2[2]*coeffs[1];

	dv[0] = e1[0]*coeffs[2] + e2[0]*coeffs[3];
	dv[1] = e1[1]*coeffs[2] + e2[1]*coeffs[3];
	dv[2] = e1[2]*coeffs[2] + e2[2]*coeffs[3];

	double mag;
	mag = sqrt(du[0]*du[0] + du[1]*du[1] + du[2]*du[2]);
	if (mag!=0)
	{
		du[0]/=mag;
		du[1]/=mag;
		du[2]/=mag;
	}
	mag = sqrt(dv[0]*dv[0] + dv[1]*dv[1] + dv[2]*dv[2]);
	if (mag!=0)
	{
		dv[0]/=mag;
		dv[1]/=mag;
		dv[2]/=mag;
	}

	//Vector du, dv;
   //du = e1*float(coeffs[0]) + e2*float(coeffs[1]);
   //dv = e1*float(coeffs[2]) + e2*float(coeffs[3]);

	/*
	Vector test;

	test = du+dv;
	test.normalize();

	test = du.cross(dv);
	test.normalize();
	*/

	//du.normalize();
	//dv.normalize();

	face.du.x = du[0];
	face.du.y = du[1];
	face.du.z = du[2];

	face.dv.x = dv[0];
	face.dv.y = dv[1];
	face.dv.z = dv[2];
	
	//face.du=du;
	//face.dv=dv;
	// k = i.cross(j)
	face.dot3TransformMatrixFlipped = face.du.cross(face.dv).dot(face.faceNormal) < 0;
}
// ======================================================================
// construct a MeshBuilder
// ======================================================================

MeshBuilder::MeshBuilder(MObject const & rootMayaObject)
: m_dot3(false),
  m_normalMapTextureCoordinateIndex(0),
  m_hardpoints(),
  m_extent(NULL),
  m_collisionExtent(NULL),
  m_rootMayaObject(rootMayaObject)
{
}

// ----------------------------------------------------------------------
/**
 * destroy a MeshBuilder.
 */

MeshBuilder::~MeshBuilder(void)
{
	int nHardpoints = m_hardpoints.size();

	for(int i = 0; i < nHardpoints; i++)
	{
		delete m_hardpoints.at(i);
		m_hardpoints.at(i) = NULL;
	}

	delete m_extent;
	delete m_collisionExtent;
}

// ----------------------------------------------------------------------

bool MeshBuilder::hasFrames ( void ) const
{
	return false;
}

// ----------------------------------------------------------------------
/**
 * set the current frame frame for which mesh data is being captured.
 * 
 * This function must be called before setShaderTemplate is called
 * for any data during the frame specified.  The caller should only
 * step forward in time, not backward.
 * 
 * Derived classes should not call this definition.  It is only
 * provided for the convenience of derived classes that do not do
 * anything when this function is called.
 */

bool MeshBuilder::setFrame(int animationFrameNumber, int firstAnimationFrameNumber, int lastAnimationFrameNumber, real framesPerSecond)
{
	UNREF(animationFrameNumber);
	UNREF(firstAnimationFrameNumber);
	UNREF(lastAnimationFrameNumber);
	UNREF(framesPerSecond);

	return true;
}

// ----------------------------------------------------------------------
/**
 * set the shader template to be used by all new added geometry.
 * 
 * This function should be called to set the current ShaderTemplate
 * information before adding geometry using the ShaderTemplate.  All
 * geometry added is assumed to use the currently-set ShaderTemplate.
 * 
 * Each call to this function assumes the shaderTemplateName being
 * specified is new.  e.g. if shader template name A is set, some
 * faces are added, shader template name B is set, some faces are
 * added, then shader template name A is set again, with more faces
 * added.  The last set of faces added for shader template name A
 * will not be aggregated with the faces listed for the first occurrence
 * of shader template name A if such is allowed by the underlying
 * object being created.  Derived builder classes should follow this
 * policy if the object being built can support it.
 */

bool MeshBuilder::setShaderTemplate(const char *shaderTemplateName, int *shaderTemplateIndex, bool dot3, int normalMapTextureCoordinateIndex)
{
	UNREF(shaderTemplateName);
	UNREF(shaderTemplateIndex);

	m_dot3 = dot3;
	if (m_dot3)
		m_normalMapTextureCoordinateIndex = normalMapTextureCoordinateIndex;

	return true;
}

// ----------------------------------------------------------------------

void MeshBuilder::setShaderNormalsInfo(
	const char *, 
	CompressType,
	bool,
	bool,
	float
)
{
}

// ----------------------------------------------------------------------

bool MeshBuilder::validateFace(Face &face) const
{
	// detect 0 area polygons
	if (face.positionArray[0] == face.positionArray[1])
	{
		MESSENGER_LOG_WARNING(("Zero area polygon detected at %f %f %f\n", face.positionArray[0].x, face.positionArray[0].y, face.positionArray[0].z));
		return false;
	}
	if (face.positionArray[0] == face.positionArray[2])
	{
		MESSENGER_LOG_WARNING(("Zero area polygon detected at %f %f %f\n", face.positionArray[0].x, face.positionArray[0].y, face.positionArray[0].z));
		return false;
	}
	if (face.positionArray[1] == face.positionArray[2])
	{
		MESSENGER_LOG_WARNING(("Zero area polygon detected at %f %f %f\n", face.positionArray[1].x, face.positionArray[1].y, face.positionArray[1].z));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Compute the dot3 transform appropriate for this triangle face.
 *
 * @param face                  the face for which the dot3 transform will be computed.
 *                              Dot3 values are returned in face.
 */

void MeshBuilder::processDot3Transform(Face &face) const
{
	if (m_dot3) 
		computeDot3Transform(face, m_normalMapTextureCoordinateIndex);
	else
		face.dot3TransformMatrixValid = false;
}

// ----------------------------------------------------------------------
/**
 * add a face to the object being built.
 * 
 * The added mesh is assumed to use the currently set shader.
 * 
 * Mesh welding policy is up to the derived builder class.
 */

bool MeshBuilder::addTriangleFace(const Face &face)
{
	UNREF(face);
	return true;
}

// ----------------------------------------------------------------------
/**
 * specify no more data is forthcoming, mesh building is complete.
 * 
 * The client should call this when it will not be handing any more
 * data to the builder.
 */

bool MeshBuilder::specifyNoMoreData(void)
{
	return true;
}

// ----------------------------------------------------------------------

void MeshBuilder::getDot3TextureCoordinateInfo(const char *shaderTemplateName, bool &hasDot3Coordinate, int &textureCoordinateIndex) const
{
	UNREF(shaderTemplateName);
	UNREF(hasDot3Coordinate);
	UNREF(textureCoordinateIndex);

	//-- If this gets called, it must be overridden.  Many of the builders do not actually require
	//   this since they don't directly generate shader templates; otherwise I would have made this
	//   function pure virtual.
	FATAL(true, ("getDot3TextureCoordinateInfo(): MeshBuilder-derived class must define getDot3TextureCoordinateInfo() to make use of this function."));
}

// ----------------------------------------------------------------------

void MeshBuilder::attachExtent ( Extent * pNewExtent )
{
	if(m_extent != pNewExtent)
	{
		delete m_extent;
		m_extent = pNewExtent;
	}
}

void MeshBuilder::attachCollisionExtent ( Extent * pNewExtent )
{
	if(m_collisionExtent != pNewExtent)
	{
		delete m_collisionExtent;
		m_collisionExtent = pNewExtent;
	}
}

// ----------

Extent const * MeshBuilder::getExtent ( void ) const
{
	return m_extent;
}

Extent const * MeshBuilder::getCollisionExtent ( void ) const
{
	return m_collisionExtent;
}

// ----------------------------------------------------------------------

void MeshBuilder::addFloorName (const char* name)
{
	m_floors.push_back(name);
}

// ----------------------------------------------------------------------

bool MeshBuilder::writeHardpoints(Iff & iff) const
{
	iff.insertForm(TAG_HPTS);

		int nHardpoints = m_hardpoints.size();

		for(int i = 0; i < nHardpoints; i++)
		{
			Hardpoint const * hp = m_hardpoints[i];

			iff.insertChunk(TAG_HPNT);
				iff.insertChunkFloatTransform(hp->getTransform());
				iff.insertChunkString(hp->getName().getString());
			iff.exitChunk(TAG_HPNT);
		}

	iff.exitForm(TAG_HPTS);

	return true;
}

// ----------------------------------------------------------------------

bool MeshBuilder::writeExtents (Iff & iff ) const
{
	if(getExtent() != NULL)
	{
		getExtent()->write(iff);
	}
	else
	{
		iff.insertForm(TAG_NULL);
		iff.exitForm(TAG_NULL);
	}

	// ----------

	if(getCollisionExtent() != NULL)
	{
		getCollisionExtent()->write(iff);
	}
	else
	{
		iff.insertForm(TAG_NULL);
		iff.exitForm(TAG_NULL);
	}

	return true;
}

// ----------

bool MeshBuilder::writeFloors(Iff & iff) const
{
	iff.insertForm(TAG_FLOR);

		iff.insertChunk(TAG_DATA);
		
			bool bHasFloors = !m_floors.empty();

			iff.insertChunkData(bHasFloors);

			if(bHasFloors)
			{
				iff.insertChunkString( m_floors[0].c_str() );
			}

		iff.exitChunk(TAG_DATA);

	iff.exitForm(TAG_FLOR);

	return true;
}

// ----------------------------------------------------------------------

bool MeshBuilder::write (Iff & iff ) const
{
	iff.insertForm(TAG_APPR);

		iff.insertForm(TAG_0003);
		
			writeExtents(iff);

			writeHardpoints(iff);

			writeFloors(iff);
			
		iff.exitForm(TAG_0003);

	iff.exitForm(TAG_APPR);

	return true;
}

// ----------------------------------------------------------------------

bool MeshBuilder::collectHardpoints(void)
{
	bool const collectOK = collectHardpoints(&m_rootMayaObject, m_hardpoints, "hp_", true);

	// Prune special-purpose hardpoints from the list

	for(uint i = 0; i < m_hardpoints.size(); i++)
	{
		Hardpoint * h = m_hardpoints[i];

		if( (strncmp(h->getName().getString(),"path",4) == 0) || (strncmp(h->getName().getString(),"door",4) == 0) )
		{
			delete h;

			m_hardpoints[i] = NULL;
		}
	}

	HardpointVec::iterator it = std::remove(m_hardpoints.begin(),m_hardpoints.end(), static_cast<Hardpoint*>(0) );

	m_hardpoints.erase(it,m_hardpoints.end());	

	return collectOK;
}

// ----------------------------------------------------------------------

bool MeshBuilder::addNode ( MayaHierarchy::Node const * node )
{
	UNREF(node);
	return true;
}

// ----------------------------------------------------------------------

bool isTranslateScale ( MMatrix & matrix )
{
	if(matrix[0][1] != 0.0f) return false;
	if(matrix[0][2] != 0.0f) return false;
	if(matrix[0][3] != 0.0f) return false;

	if(matrix[1][0] != 0.0f) return false;
	if(matrix[1][2] != 0.0f) return false;
	if(matrix[1][3] != 0.0f) return false;

	if(matrix[2][0] != 0.0f) return false;
	if(matrix[2][1] != 0.0f) return false;
	if(matrix[2][3] != 0.0f) return false;

	return true;
}

// ----------------------------------------------------------------------

bool extractTranslateScale( MObject mayaTransform, Vector & outTranslate, Vector & outScale )
{
	MStatus status;

	MFnTransform fnTransform(mayaTransform, &status);
	if(!status) return false;

	MMatrix matrix = fnTransform.transformation(&status).asMatrix();
	if(!status) return false;

	// Extents can't have rotation yet...

	if(!isTranslateScale(matrix)) 
	{
		MESSENGER_REJECT(true,("Collision shape has rotation - not supported yet\n"));
		return false;
	}

	float transX = static_cast<float>( matrix[3][0] );
	float transY = static_cast<float>( matrix[3][1] );
	float transZ = static_cast<float>( matrix[3][2] );

	float scaleX = static_cast<float>( matrix[0][0] );
	float scaleY = static_cast<float>( matrix[1][1] );
	float scaleZ = static_cast<float>( matrix[2][2] );

	//-- fixup rh to lh
	transX = -transX;

	outTranslate = Vector(transX,transY,transZ);
	outScale = Vector(scaleX,scaleY,scaleZ);

	return true;
}

// ----------------------------------------------------------------------

Extent * MeshBuilder::createBoxExtent( real width, real height, real depth, Vector translate, Vector scale )
{
	Vector delta( width * scale.x / 2.0f, height * scale.y / 2.0f, depth * scale.z / 2.0f );

	Vector minCorner = translate - delta;
	Vector maxCorner = translate + delta;

	real radius = delta.magnitude();

	Sphere sphere(translate,radius);

	BoxExtent * extent = new BoxExtent(minCorner,maxCorner,sphere);

	return extent;
}

Extent * MeshBuilder::createCylinderExtent ( real radius, real height, Vector translate, Vector scale )
{
    if(scale.x != scale.z)
    {
        MESSENGER_REJECT(true,("Collision cylinder has non-uniform X-Z scale - not supported\n"));
    }

	radius *= scale.x;
	height *= scale.y;

	Vector base = Vector(translate.x, translate.y - (height / 2.0f), translate.z);

	Cylinder cylinder(base,radius,height);

	CylinderExtent * extent = new CylinderExtent( cylinder );

	return extent;
}

Extent * MeshBuilder::createSphereExtent( real radius, Vector translate, Vector scale )
{
    if((scale.x != scale.y) || (scale.y != scale.z))
    {
        MESSENGER_REJECT(true,("Collision sphere has non-uniform scale - not supported\n"));
    }

	radius *= scale.x;

	Sphere sphere(translate,radius);

	Extent * extent = new Extent(sphere);

	return extent;
}

// ----------------------------------------------------------------------

void MeshBuilder::createChildExtents( MayaHierarchy::Node const * node, ExtentVec2 & extents, bool onlyAppearances )
{
	if(node == NULL) return;

	extents.clear();

	for (int i = 0; i < node->getNumberOfChildren(); i++)
	{
		MayaHierarchy::Node const * child = node->getChild(i);

		if(onlyAppearances)
		{
			if(!child->isAppearance())
			{
				continue;
			}
		}

		Extent * childExtent = createExtent( child );

		if(childExtent)
		{
			extents.push_back(childExtent);
		}
	}
}

// ----------------------------------------------------------------------

Extent * MeshBuilder::createComponentExtent( ExtentVec2 const & extents )
{
	if(extents.size() == 0)
	{
		return NULL;
	}
	else if(extents.size() == 1)
	{
		return extents[0];
	}

	ComponentExtent * extent = new ComponentExtent();

	for(uint i = 0; i < extents.size(); i++)
	{
		extent->attachExtent(extents[i]);
	}

	return extent;
}

// ----------

Extent * MeshBuilder::createDetailExtent( ExtentVec2 const & extents )
{
	if(extents.size() == 0)
	{
		return NULL;
	}
	else if(extents.size() == 1)
	{
		return extents[0];
	}

	DetailExtent * extent = new DetailExtent();

	for(uint i = 0; i < extents.size(); i++)
	{
		extent->attachExtent(extents[i]);
	}

	return extent;
}

// ----------------------------------------------------------------------
// Create an extent for a node of type T_extent

Extent * MeshBuilder::createExtent_Extent( MayaHierarchy::Node const * node )
{
	ExtentVec2 childExtents;

	createChildExtents(node,childExtents,false);

	return createComponentExtent(childExtents);
}

// ----------------------------------------------------------------------
// Create an extent for a node of type T_collision

Extent * MeshBuilder::createExtent_Collision( MayaHierarchy::Node const * node )
{
	ExtentVec2 childExtents;

	createChildExtents(node,childExtents,false);

	return createDetailExtent(childExtents);
}

// ----------------------------------------------------------------------
// Create an extent for a node of type T_cylinder, T_box, or T_sphere

Extent * MeshBuilder::createExtent_Primitive ( MayaHierarchy::Node const * node )
{
	if(node == NULL) return NULL;

	MStatus status;

	bool result = true;

	MDagPath path = node->getMayaDagPath();

	// ----------
	// Extract the translate and scale for this node

	Vector trans;
	Vector scale;

	result = extractTranslateScale( path.transform(&status), trans, scale );

	MESSENGER_REJECT(!result,("Couldn't extract translation & scale for collision node\n"));

	// ----------
	// Find the source mesh for this shape node

	MObject obj = path.node(&status);

	MFnDependencyNode fnDep(obj,&status);

	MObjectArray array;

	result &= MayaUtility::findSourceObjects (fnDep,"inMesh",&array);
	if(!result) return NULL;

	// ----------
	// The source polymesh should always be the first element in the array

	uint nObjects = array.length();

	MESSENGER_REJECT(nObjects == 0,("Collision extent node %s has no polyShape input node - someone either copied it or deleted the history for it\n",node->getName()));
	if(nObjects == 0) return NULL;

	MObject & polyObject = array[0];

	MFnDependencyNode fnPolyDep(polyObject,&status);
	MESSENGER_REJECT(!status,("Couldn't create MFnDependencyNode for node %s\n",node->getName()));
	if(!status) return NULL;

	// ----------
				
	switch(polyObject.apiType())
	{
		case MFn::kPolySphere:
			{
				real radius;

				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"radius",&radius);
				MESSENGER_REJECT(!result,("Couldn't get radius for collision sphere node\n"));
				if(!result) return NULL;

				return createSphereExtent(radius,trans,scale);
			}
		case MFn::kPolyCylinder:
			{
				real radius;
				real height;

				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"radius",&radius);
				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"height",&height);
				MESSENGER_REJECT(!result,("Couldn't get radius or height for collision cylinder node\n"));
				if(!result) return NULL;

				return createCylinderExtent(radius,height,trans,scale);
			}
		case MFn::kPolyCube:
			{
				real width;
				real height;
				real depth;

				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"width",&width);
				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"height",&height);
				result &= MayaUtility::getNodeFloatValue(fnPolyDep,"depth",&depth);
				MESSENGER_REJECT(!result,("Couldn't get dimensions for collision box node\n"));
				if(!result) return NULL;

				return createBoxExtent(width,height,depth,trans,scale);
			}
		default:
			return NULL;

	}
}

// ----------------------------------------------------------------------
// Create an extent for a node of type T_msh or T_cmesh

Extent * MeshBuilder::createExtent_CMesh( MayaHierarchy::Node const * node )
{
	MStatus status;

	MDagPath path = node->getMayaDagPath();

	MObject obj = path.node(&status);

	MFnMesh fnMesh(obj,&status);
	if (!status) return false;

	// ----------

	VectorVector vertices;
	IntVector indices;

	// using utility functions defined in FloorBuilder... these should be factored out...

	bool extractVerts = FloorBuilder::extractPoints(fnMesh,vertices);
	bool extractIndices = FloorBuilder::extractIndices(fnMesh,indices);

	MESSENGER_REJECT_NORETURN( !extractVerts, ("MeshBuilder::createExtent_CMesh - Couldn't extract vertices for node %s\n",node->getName()));
	MESSENGER_REJECT_NORETURN( !extractIndices, ("MeshBuilder::createExtent_CMesh - Node %s isn't tesselated into triangles\n",node->getName()));

	if(extractVerts && extractIndices)
	{
		IndexedTriangleList * mesh = new IndexedTriangleList();

		mesh->addIndexedTriangleList(&vertices[0],vertices.size(),&indices[0],indices.size());

		MeshExtent * extent = new MeshExtent();

		extent->attachMesh(mesh);

		return extent;
	}
	else
	{
		return NULL;
	}
}

// ----------------------------------------------------------------------

Extent * MeshBuilder::createExtent_Lod( MayaHierarchy::Node const * node )
{
	return createExtent(findCollisionGroup(node));
}

// ----------------------------------------------------------------------

Extent * MeshBuilder::createExtent_Mesh( MayaHierarchy::Node const * node )
{
	return createExtent(findCollisionGroup(node));
}

// ----------------------------------------------------------------------
// Component nodes are a little tricky - The extents for thei

Extent * MeshBuilder::createExtent_Component( MayaHierarchy::Node const * node )
{
	Extent * partHalf = createExtent_ComponentParts(node);
	Extent * collisionHalf = createExtent(findCollisionGroup(node));

	if(partHalf && collisionHalf)
	{
		DetailExtent * pExtent = new DetailExtent();

		pExtent->attachExtent(collisionHalf);
		pExtent->attachExtent(partHalf);

		return pExtent;
	}
	else if(partHalf)
	{
		return partHalf;
	}
	else if(collisionHalf)
	{
		return collisionHalf;
	}
	else
	{
		return NULL;
	}
}

// ----------
// The parts of a component node 

Extent * MeshBuilder::createExtent_ComponentParts( MayaHierarchy::Node const * node )
{
	ExtentVec2 childExtents;

	createChildExtents(node,childExtents,true);

	return createComponentExtent( childExtents );
}

// ----------------------------------------------------------------------

Extent * MeshBuilder::createExtent ( MayaHierarchy::Node const * node )
{
	if(node == NULL) return NULL;

	switch(node->getType())
	{
	// ----------
	// Colliison node types

	case MayaHierarchy::T_collision:	return createExtent_Collision(node);

	case MayaHierarchy::T_extent:		return createExtent_Extent(node);
		
	case MayaHierarchy::T_cylinder:		return createExtent_Primitive(node);
	case MayaHierarchy::T_sphere:		return createExtent_Primitive(node);
	case MayaHierarchy::T_box:			return createExtent_Primitive(node);

	case MayaHierarchy::T_cmesh:		return createExtent_CMesh(node);

	// ----------
	// Appearance node types

	case MayaHierarchy::T_lod:			return createExtent_Lod(node);

	case MayaHierarchy::T_cmp:			return createExtent_Component(node);

	case MayaHierarchy::T_msh:			return createExtent_Mesh(node);

	// ----------

	default:							
		return NULL;
	}
}

// ----------------------------------------------------------------------

bool MeshBuilder::extractFloors ( MayaHierarchy::Node const * node )
{
	if(node->getType() == MayaHierarchy::T_floor)
	{
		addFloorName(node->getAppearanceReferenceName().asChar());
	}

	for (int i = 0; i < node->getNumberOfChildren(); i++)
	{
		extractFloors(node->getChild(i));
	}

	return true;
}

// ----------------------------------------------------------------------
// returns true if any collision data was extracted from the node

bool MeshBuilder::addCollisionNode( MayaHierarchy::Node const * node )
{
	Extent * extent = createExtent(node);

	if(extent)
	{
		attachCollisionExtent(extent);
	}

	extractFloors(node);

	return true;
}

// ----------------------------------------------------------------------

/*
bool MeshBuilder::isCollisionNode ( MayaHierarchy::Node const * node )
{
	if(node == NULL) return false;

	switch(node->getType())
	{
	case T_collision:   return true;
	case T_floor:       return true;
	case T_extent:      return true;
	case T_cylinder:    return true;
	case T_sphere:      return true;
	case T_box:         return true;
	case T_cmesh:       return true;
	
	default: return false;
	}
}
*/

// ----------------------------------------------------------------------

/*
bool MeshBuilder::hasCollision( MayaHierarchy::Node const * node )
{
	if(isCollisionNode(node)) return true;

	for(int i = 0; i < node->getNumberOfChildren(); i++)
	{
		if(hasCollision(node->getChild(i))) return true;
	}

	return false;
}
*/

// ----------------------------------------------------------------------

MayaHierarchy::Node const * MeshBuilder::findCollisionGroup( MayaHierarchy::Node const * node )
{
	for(int i = 0; i < node->getNumberOfChildren(); i++)
	{
		MayaHierarchy::Node const * child = node->getChild(i);

		if(child->getType() == MayaHierarchy::T_collision)
		{
			return child;
		}
	}

	return NULL;
}

// ======================================================================
