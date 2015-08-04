/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description:		MeshModel code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpMeshModel.cpp $
 * $Author: wili $ 
 * $Revision: #3 $
 * $Modtime: 12.02.02 15:02 $
 * $Date: 2003/06/19 $
 * 

 ******************************************************************************/

#include "dpvsImpMeshModel.hpp"
#include "dpvsBitMath.hpp"
#include "dpvsSilhouette.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsBounds.hpp"
#include "dpvsIntersect.hpp"
#include "dpvsClipPolygon.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsInstanceCount.hpp"		
#include "dpvsDebug.hpp"
#include "dpvsRange.hpp"
#include "dpvsVQData.hpp"
#include "dpvsTempArray.hpp"

using namespace DPVS;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//#define TIMO_BITBOYS_TEST
#ifdef TIMO_BITBOYS_TEST

class Cache
{
protected:
	enum
	{
		BANKS = 4
	};

	struct Bank
	{
		bool	hasVertex	(int index)		{ return (index>=firstVertex && index<=lastVertex); }

		int	timeStamp;
		int	firstVertex;		// inclusive
		int	lastVertex;			// inclusive
	};


	Bank& findLRUBank		(void)
	{
		int	bi = 0;
		for(int i=1;i<BANKS;i++)
		{
			if(m_banks[i].timeStamp < m_banks[bi].timeStamp)
				bi = i;
		}

		return m_banks[bi];
	}

	int		findBank		(int index, int timeStamp)
	{
		for(int i=0;i<BANKS;i++)
		if(m_banks[i].hasVertex(index))
		{
			m_banks[i].timeStamp = timeStamp;
			return i;
		}

		return -1;
	}

	Bank	m_banks[BANKS];
	int		m_verticesPerFetch;
	int		m_timeStamp;

	int		m_hits;
	int		m_misses;
	int		m_overlaps;
	int		m_runs;
	float	m_ratio;

public:
	Cache	(int verticesPerFetch)
	{
		m_verticesPerFetch = verticesPerFetch;
		reset();
		resetBanks();
	}

	void	reset()
	{
		m_hits		= 0;
		m_misses	= 0;
		m_overlaps	= 0;
		m_runs		= 0;
		m_timeStamp	= 0;
		m_ratio		= 0.f;
	}

	void	resetBanks()
	{
		for(int i=0;i<BANKS;i++)
		{
			m_banks[i].firstVertex	= -1;
			m_banks[i].lastVertex	= -1;
			m_banks[i].timeStamp	= -1;
		}
	}

	virtual void fetch(int index) = 0;
};

//-------------------------------------------------------------------

class Cache1 : public Cache
{
public:
	Cache1	(int v) : Cache(v)	{}
	void fetch(int index);
};


void Cache1::fetch(int index)
{
	if(findBank(index,m_timeStamp)!=-1)
	{
		m_hits++;
	}
	else
	{
		m_misses++;
		Bank& bank = findLRUBank();
		bank.firstVertex	= index;
		bank.lastVertex		= bank.firstVertex + m_verticesPerFetch - 1;
		bank.timeStamp		= m_timeStamp;
	}

	m_timeStamp++;
	m_runs++;
	m_ratio = 100.f*m_hits/m_runs;
}

//-------------------------------------------------------------------

class CacheCenter : public Cache
{
public:
	CacheCenter	(int v) : Cache(v)	{}
	void fetch(int index);
};


void CacheCenter::fetch(int index)
{
	if(findBank(index,m_timeStamp)!=-1)
	{
		m_hits++;
	}
	else
	{
		m_misses++;
		Bank& bank = findLRUBank();
		bank.firstVertex	= index - 1;
		bank.lastVertex		= bank.firstVertex + m_verticesPerFetch - 1;
		bank.timeStamp		= m_timeStamp;
	}

	m_timeStamp++;
	m_runs++;
	m_ratio = 100.f*m_hits/m_runs;
}

//-------------------------------------------------------------------

class CachePetri : public Cache
{
public:
	CachePetri	(int v) : Cache(v)	{}
	void fetch(int index);
};


void CachePetri::fetch(int index)
{
	if(findBank(index,m_timeStamp)!=-1)
	{
		m_hits++;
	}
	else
	{
		m_misses++;
		Bank& bank = findLRUBank();
		bank.firstVertex	= (index / m_verticesPerFetch) * m_verticesPerFetch;
		bank.lastVertex		= bank.firstVertex + m_verticesPerFetch - 1;
		bank.timeStamp		= m_timeStamp;
	}

	m_timeStamp++;
	m_runs++;
	m_ratio = 100.f*m_hits/m_runs;
}

//-------------------------------------------------------------------

Cache1		cb_4(4);
Cache1		cb_8(8);
CachePetri	cp_4(4);
CachePetri	cp_8(8);
CacheCenter	cc_4(4);
CacheCenter	cc_8(8);

#endif //TIMO_BITBOYS_TEST

/*****************************************************************************
 *
 * Function:		ImpMeshModel::getCameraInverseTranslation()
 *
 * Description:		Returns valid homogenous inverse translation vector
 *
 * Returns:			four-component homogenous vector
 *
 *****************************************************************************/

Vector4 ImpMeshModel::getCameraInverseTranslation (const Matrix4x3& objectToCamera)
{
	//--------------------------------------------------------------------
	// The "camera in object space" is expressed as a homogenous coordinate. 
	// The W value is 1.0 if we're using a perspective projection, 0.0 if
	// using ortho projection.
	//--------------------------------------------------------------------

	if (VQData::get().isOrthoProjection())
	{
		Matrix4x3 cameraToObject(NO_CONSTRUCTOR);
		Math::invertMatrix(cameraToObject, objectToCamera);
		return Vector4(-cameraToObject.getDof(),0.0f);
	}

	return Vector4(Math::getInverseTranslation (objectToCamera),1.0f);
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::getTestMesh()
 *
 * Description:		Routine for querying test mesh
 *
 * Returns:			pointer to test mesh
 *
 *****************************************************************************/

const Mesh* ImpMeshModel::getTestMesh (void) const
{
	return getMesh();
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::getConvexHull()
 *
 * Description:		Returns vertex data
 *
 * Parameters:		v = reference to VertexArray structure
 *
 *****************************************************************************/

void ImpMeshModel::getTestMesh (VertexArray& v) const
{
	v.m_vertexPositions = getVertices();
	v.m_vertexCount     = getVertexCount();
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::ImpMeshModel()
 *
 * Description:		MeshModel Constructor
 *
 * Parameters:		loc			= vertex positions in object-space
 *					ind			= triangle vertex indices
 *					vn			= number of vertices
 *					tn			= number of triangles
 *					clockwise	= boolean indicating whether vertex indices are in clockwise order..
 *
 * Notes:			The ctor is now designed to perform a minimal amount of
 *					work -- this should allow very fast creation of new 
 *					objects/models.
 *
 *****************************************************************************/

ImpMeshModel::ImpMeshModel(const Vector3* loc,const Vector3i* ind,int vn,int tn,bool clockwise)
{
#ifdef TIMO_BITBOYS_TEST

	cb_4.resetBanks();
	cb_8.resetBanks();
	cc_4.resetBanks();
	cc_8.resetBanks();
	cp_4.resetBanks();
	cp_8.resetBanks();

//	for(int i=0;i<tn;i++)
	for(int i=tn-1;i>=0;i--)
	{
		cb_4.fetch(ind[i].i);
		cb_4.fetch(ind[i].j);
		cb_4.fetch(ind[i].k);
		cb_8.fetch(ind[i].i);
		cb_8.fetch(ind[i].j);
		cb_8.fetch(ind[i].k);

		cc_4.fetch(ind[i].i);
		cc_4.fetch(ind[i].j);
		cc_4.fetch(ind[i].k);
		cc_8.fetch(ind[i].i);
		cc_8.fetch(ind[i].j);
		cc_8.fetch(ind[i].k);

		cp_4.fetch(ind[i].i);
		cp_4.fetch(ind[i].j);
		cp_4.fetch(ind[i].k);
		cp_8.fetch(ind[i].i);
		cp_8.fetch(ind[i].j);
		cp_8.fetch(ind[i].k);
	}

	static int counter = 0;
	counter++;
	if(counter%100 == 0)
	{
		int tmp = 0;
	}

#endif //TIMO_BITBOYS_TEST

	//--------------------------------------------------------------------
	// Update some statistics
	//--------------------------------------------------------------------

	InstanceCount::incInstanceCount(InstanceCount::MESHMODEL);

	//--------------------------------------------------------------------
	// Check input parameters in debug build. In release build the code
	// doesn't do anything
	//--------------------------------------------------------------------

#if defined (DPVS_DEBUG)
	DPVS_API_ASSERT( loc && ind && vn >= 3 && tn >= 1 && "MeshModel::create() - invalid input parameters");
	
	for (int i = 0; i < tn; i++)
	{
		DPVS_API_ASSERT(ind[i].i >= 0 && ind[i].i < vn && "MeshModel::create() - invalid vertex index");
		DPVS_API_ASSERT(ind[i].j >= 0 && ind[i].j < vn && "MeshModel::create() - invalid vertex index");
		DPVS_API_ASSERT(ind[i].k >= 0 && ind[i].k < vn && "MeshModel::create() - invalid vertex index");
	}

	// check for NaN etc.
	for (int i = 0; i < vn; i++)
	{
		DPVS_API_ASSERT(Math::isFinite(loc[i]) && "MeshModel::create() - invalid vertex data");
	}

#endif

	//--------------------------------------------------------------------
	// Submit model data to the SilhouetteCache::Client
	// part. This will in turn weld the mesh data and remove
	// degenerate triangles. If the data was not specified
	// in clock-wise order, perform the flipping here (so that all internal 
	// routines can operate with clock-wise data).
	//--------------------------------------------------------------------

	if(clockwise)
		setMesh(loc,vn,ind,tn);
	else
	{
		TempArray<Vector3i,false> cw_ind(tn);			// alloc temporary array
		for(int i=0;i<tn;i++)
		{
			cw_ind[i].i = ind[i].j;						// flip each triangle by flipping the first and the second vertex index
			cw_ind[i].j = ind[i].i;
			cw_ind[i].k = ind[i].k;
		}

		setMesh(loc,vn,&cw_ind[0],tn);
	}

	//--------------------------------------------------------------------
	// Calculate bounding sphere of the model and an
	// approximate oriented bounding box. The real 
	// bounding box is computed later... Initially we just
	// approximate it with the AABB of the model.
	//--------------------------------------------------------------------

	OBB		tmpOBB;										// temporary variables..
	AABB	tmpAABB;
	Sphere	tmpSphere;

	calculateSphereAndAABB		(tmpSphere,	tmpAABB, getVertices(), getVertexCount());
	calculateOBB				(tmpOBB, tmpAABB);		// initially the OBB is the same as AABB...
	
	setSphere					(tmpSphere);
	setOBB						(tmpOBB, false);		// "false" means that the OBB is not considered exact and should be updated later..

	setBackFaceCullable			(isPlanar());			// if model is planar, set backface culling flag on
	setOcclusionWritable		(true);					// the model has a write silhouette by default..
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::computeExactOBB()
 *
 * Description:		Function for computing the exact OBB of a MeshModel
 *
 * Notes:			Initially only an estimated OBB is used. More exact OBBs
 *					are computed lazily (by a random chance) later, because
 *					the OBB computation time is quite large.
 *
 *****************************************************************************/

void ImpMeshModel::computeExactOBB (void)
{
	OBB tmpOBB;
	calculateOBB	(tmpOBB, getVertices(), getVertexCount());
	setOBB			(tmpOBB, true);
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::~ImpMeshModel()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpMeshModel::~ImpMeshModel(void)
{
	//--------------------------------------------------------------------
	// The SilhouetteCache::Client destructor will release all the mesh 
	// data automatically, so we don't have to worry about it here...
	//--------------------------------------------------------------------
	InstanceCount::decInstanceCount(InstanceCount::MESHMODEL);
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpMeshModel::backfaceCull()
 *
 * Description:		Performs back-face culling
 *
 * Parameters:		tlat = camera position in object-space
 *
 * Returns:			true if visible, false if completely bf-culled
 *
 * Notes:			The function causes the computation of plane equations,
 *					if that has not been done yet..
 *
 *					The function performs only "object-level" back-face
 *					culling, i.e. it reports of the entire model is
 *					back-facing.
 *
 *****************************************************************************/

bool ImpMeshModel::backFaceCull (const Vector4& tlat) const
{
	// TODO: take in a tolerance value (based on object's size) and
	// compute the maximum arithmetic error that comes from the
	// tlat and p terms. Then make EPSILON a dynamic value based
	// on the max of these two terms.

	//--------------------------------------------------------------------
	// If the model does not have a "derived" structure for some reason,
	// skip the test (and tell that the model is visible). Note that we
	// must ensure that _portals_ have always derived structures..
	//--------------------------------------------------------------------

	const SilhouetteCache::Client::Derived*	derived	= const_cast<ImpMeshModel*>(this)->getDerived();

	if (!derived)
		return true;

	//--------------------------------------------------------------------
	// Perform back-face culling
	//--------------------------------------------------------------------

	int				cnt		= isPlanar() ? 1 : derived->m_numPlanes;		// planar models only have a single pleq
	const Vector4*	plEq	= derived->m_plEq;								// get plane equations...

	DPVS_ASSERT (plEq);

	for(int i=0;i<cnt;i++)
	{
		const float    EPSILON	= 0.0f;										// assign negative values to give some leeway to the calculation..
		const Vector4& p		= plEq[i];									// access to plEq
		float          dist		= tlat.x*p.x + tlat.y*p.y + tlat.z*p.z + tlat.w * p.w;

		if (dist >= EPSILON)		// front-facing -> early-exit
			return true;
	}
	
	return false;															// completely back-face culled
}

/*****************************************************************************
 *
 * Function:		DPVS::ImpMeshModel::getExactRectangle()
 *
 * Description:		Computes exact axis-aligned rectangle
 *
 * Parameters:		r				= reference to rectangle (output)
 *
 * Returns:			true if visible, false if outside the view frustum
 *
 * Notes:			This function is mainly used by portals, as they
 *					gain considerably from calculating the exact 
 *					rectangle. The routine first approximates the rectangle
 *					and depth values from the OBB of the model, then attempts
 *					to refine it using vertex data.
 *
 *****************************************************************************/

bool ImpMeshModel::getExactRectangle(FloatRectangle& r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELEXACTRECTANGLESQUERIED,1));

	ImpObject* o = VQData::get().getObject();
	ImpCamera* c = VQData::get().getCamera();
	DPVS_ASSERT(o);
	DPVS_ASSERT(c);

	const Matrix4x3& objectToCamera = o->getObjectToCameraMatrix(c);

	//--------------------------------------------------------------------
	// Start by performing a clipped query for the OBB. This should give
	// a relatively good rectangle.
	//--------------------------------------------------------------------

	if (!SilhouetteMath::getClippedRectangle_FLOW(r))
		return false;
	
	//--------------------------------------------------------------------
	// Get test mesh vertices
	//--------------------------------------------------------------------

	VertexArray hv;
	getTestMesh(hv);
	DPVS_ASSERT(hv.m_vertexCount);

	//--------------------------------------------------------------------
	// Transform all vertices of the object and find their screen-space
	// minmax values.
	//--------------------------------------------------------------------


	Matrix4x4 objectToScreen(NO_CONSTRUCTOR);
	Math::productFromLeftProjection (objectToScreen, objectToCamera, c->getCameraToScreen());

	Vector3	topLeft,bottomRight;
	bool frontClipping = Math::minMaxTransformDivByW(topLeft,bottomRight,hv.m_vertexPositions,objectToScreen,hv.m_vertexCount);

	//--------------------------------------------------------------------
	// If mesh wasn't front-clipping, see if we can make the original
	// rectangle smaller.
	//--------------------------------------------------------------------

	DPVS_ASSERT(VQData::get().testProperties(VQData::DEPTH_RANGE_VALID));
	Range<float>& depthRange = VQData::get().accessDepthRange();

	if (!frontClipping)
	{
		const FloatRectangle& v = VQData::get().getRasterViewport();

		float mnz = topLeft.z		* 0.5f + 0.5f;
		float mxz = bottomRight.z	* 0.5f + 0.5f;

		if (mxz < 0.0f)
			return false;							// hidden

		if (mnz > depthRange.getMin())	depthRange.setMin(mnz);
		if (mxz < depthRange.getMax())	depthRange.setMax(mxz);

		if (depthRange.getMin() >= depthRange.getMax())
			depthRange.setMax(depthRange.getMin());			// humm? DEBUG DEBUG SHOULD WE PERHAPS RETURN FALSE?

		float xscale	=  0.5f * v.width();
		float xofs		=  0.5f * v.width()  + v.x0;
		float yscale	= -0.5f * v.height();
		float yofs		=  0.5f * v.height() + v.y0;

		FloatRectangle t;

		t.x0 = topLeft.x    *xscale+xofs;		
		t.x1 = bottomRight.x*xscale+xofs+1.0f;	
		t.y0 = bottomRight.y*yscale+yofs;		// here flip because yscale is negative
		t.y1 = topLeft.y	*yscale+yofs+1.0f;

		t.setSpace(SPACE_RASTER);

		if (!r.intersect(t))					// intersection becomes non-existent
			return false;
	}

	//-------------------------------------------------------------------
	// If debug visualization is on, draw the rectangle
	//-------------------------------------------------------------------

	if(Debug::getLineDrawFlags() & Library::LINE_RECTANGLES)
	{
		Library::LineType type = Library::LINE_RECTANGLES;
		Debug::drawLine(type, Vector2(r.x0, r.y0), Vector2(r.x1, r.y0), Vector4(1.f, 1.f, 0.1f, 0.4f));
		Debug::drawLine(type, Vector2(r.x1, r.y0), Vector2(r.x1, r.y1), Vector4(1.f, 1.f, 0.1f, 0.4f));
		Debug::drawLine(type, Vector2(r.x1, r.y1), Vector2(r.x0, r.y1), Vector4(1.f, 1.f, 0.1f, 0.4f));
		Debug::drawLine(type, Vector2(r.x0, r.y1), Vector2(r.x0, r.y0), Vector4(1.f, 1.f, 0.1f, 0.4f));
	}

	DPVS_ASSERT(r.x0<=r.x1 && r.y0<=r.y1);
	DPVS_ASSERT(depthRange.getMin() <= depthRange.getMax());

	return true;
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::getGeometryCost()
 *
 * Description:		
 *
 *****************************************************************************/

float ImpMeshModel::getGeometryCost (void) const				
{ 
	return estimateGeometryCost(getVertexCount(), getTriangleCount()); 
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::splatWriteSilhouette()
 *
 * Description:		Splats an extracted write silhouette into the screen
 *
 * Parameters:		s				= destination silhouette structure
 *					objectToCamera	= object->camera matrix
 *					inf				= source silhouette info
 *					cam				= pointer to camera
 *			
 *****************************************************************************/

void ImpMeshModel::splatWriteSilhouette (EdgeSilhouette& s, const Matrix4x3& objectToCamera, const SilhouetteCache::EdgeSilhouetteInfo& inf, ImpCamera *cam)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEFRONTCLIPPINGOCCLUDERSUSED,1));

	//--------------------------------------------------------------------
	// Calculate object-to-screen matrix.
	//--------------------------------------------------------------------

	Matrix4x4 objectToScreen(NO_CONSTRUCTOR);
	Math::productFromLeftProjection(objectToScreen, objectToCamera, cam->getCameraToScreen());

	//--------------------------------------------------------------------
	// This naive clipper implementation results in
	// (clipped vertex <= ]2,3] * edge count).
	// OPTIMIZE: extra vertices could be avoided (not worth it)
	//--------------------------------------------------------------------

	int	required = (inf.m_numVertices>3*inf.m_numEdges) ? (inf.m_numVertices) : (3*inf.m_numEdges);

	TempArray<Vector4,false>	clipspace(required*2);
	Vector4*					clipped	= &clipspace[0] + required;

	//--------------------------------------------------------------------
	// Transform vertices into clip-space
	//--------------------------------------------------------------------

	Math::transform (&clipspace[0], inf.m_loc, inf.m_numVertices, objectToScreen);
	/*
	int	i;
	for(i=0;i<inf.m_numVertices;i++)
	{
		clipspace[i] = objectToScreen.transform( inf.m_loc[i] );
	}
	*/

	const FloatRectangle&	viewPort	= cam->getRasterViewport();
	float					xscale		=  0.5f * viewPort.width();
	float					xofs		=  0.5f * viewPort.width()  + viewPort.x0;
	float					yscale		= -0.5f * viewPort.height();
	float					yofs		=  0.5f * viewPort.height() + viewPort.y0;

	const Frustum&			frustum		= cam->getFrustum();
	float					zNear		= frustum.zNear;
	int						nEdges		= inf.m_numEdges;
	const	Vector2i*		edges		= inf.m_edges;
	int						j			= 0;

	//--------------------------------------------------------------------
	// Perform clipping
	//--------------------------------------------------------------------

	for(int i=0;i<nEdges;i++)
	{
		int indexA = edges[i].i;
		int indexB = edges[i].j;

		// DEBUG DEBUG CHECK IF THIS IS ORDER-DEPENDENT

		const Vector4& A = clipspace[indexA];
		const Vector4& B = clipspace[indexB];

		float cva = -A.z - A.w;		// front clip if >0
		float cvb = -B.z - B.w;		// front clip if >0

		int	pj = j;

		//---------------------------------------------
		// start point
		//---------------------------------------------

		if(cva <= 0)						// inside
		{
			clipped[j] = A;
			j++;
		}
		else
		{
//				float w = float(fabs(A.w));		// projection
			clipped[j].x = A.x;
			clipped[j].y = A.y;
			clipped[j].z =-zNear;
			clipped[j].w = zNear;
			j++;
		}

		//---------------------------------------------
		// mid point
		//---------------------------------------------

		if(cva*cvb < 0)
		{
			float dv = cva/(cva-cvb);
			clipped[j].x = A.x + (B.x - A.x) * dv;
			clipped[j].y = A.y + (B.y - A.y) * dv;
			clipped[j].w = A.w + (B.w - A.w) * dv;

			clipped[j].z = -clipped[j].w;
			*(UINT32*)&clipped[j].z ^= 0x00000001;		//EEEEEIIIHHH

//				clipped[j].z = A.z + (B.z - A.z) * dv;
//				clipped[j].w = A.w + (B.w - A.w) * dv;
			j++;
		}

		//---------------------------------------------
		// end point
		//---------------------------------------------

		if(cvb <= 0)						// inside
		{
			clipped[j] = B;
			j++;
		}
		else
		{
			clipped[j].x = B.x;
			clipped[j].y = B.y;
			clipped[j].z =-zNear;
			clipped[j].w = zNear;
			j++;
		}

		//---------------------------------------------
		// div by w
		//---------------------------------------------

		for(int k=pj;k<j;k++)
		{
			float oow = Math::reciprocal(clipped[k].w);
			clipped[k].x = (clipped[k].x * oow * xscale + xofs);
			clipped[k].y = (clipped[k].y * oow * yscale + yofs);

			if(clipped[k].z == -zNear)	clipped[k].z = 0.f;
			else						clipped[k].z = (clipped[k].z * oow * 0.5f + 0.5f);
		}

		//---------------------------------------------
		// append edges to silhouette
		//---------------------------------------------

		for(int k=pj;k<j-1;k++)
		{
			s.addEdge(clipped[k].y < clipped[k+1].y ? 1 : 0, k,k+1);
		}
	}

	s.setVertices(clipped,j);
	SilhouetteMath::computeOBBWritePlanes (s, objectToCamera, cam, getOBB());
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::frontClipWriteSilhouette()
 *
 * Description:		Calculates correct silhouette for a front-clipping model
 *
 * Parameters:		s				= destination silhouette structure
 *					objectToCamera	= object->camera matrix
 *					inf				= input silhouette info structure
 *					cam				= pointer to camera
 *			
 * Returns:			boolean value indicating whether silhouette is valid or not		
 *
 * Notes:			This routine is considerably slower than the normal
 *					silhouette extraction. Therefore the clipping status
 *					should been checked in advance.
 *
 *					The function operates by taking an input silhouette and
 *					adding all non-clipping edges from that. Then it tests
 *					the clip status of all triangles in the original mesh and
 *					inserts all clipping edges from that.
 *
 *****************************************************************************/

bool ImpMeshModel::frontClipWriteSilhouette (EdgeSilhouette& s, const Matrix4x3& objectToCamera, const SilhouetteCache::EdgeSilhouetteInfo& inf, ImpCamera *cam)
{
	//--------------------------------------------------------------------
	// If we cannot access the derived data, return 'false' at this
	// point..
	//--------------------------------------------------------------------
	
	const SilhouetteCache::Client::Derived*	derived	= getDerived();
	if (!derived)
		return false;

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEFRONTCLIPPINGOCCLUDERSUSED,1));


	int	numVertices		= getVertexCount();
	
	DPVS_ASSERT(numVertices >= inf.m_numVertices);							// HHHHUUUH?
	DPVS_ASSERT(derived);

	//--------------------------------------------------------------------
	// Initialize vertex clip bit array to zero.
	//--------------------------------------------------------------------
	
	BitVector vertexClipBits(numVertices);
	vertexClipBits.clearAll();

	//--------------------------------------------------------------------
	// Transform vertices into screen-space (non-div-by-W) and determine
	// for each vertex if it's in front of the near-clipping plane. Mark
	// such vertices into the 'vertexClipBits' array.
	//--------------------------------------------------------------------
	
	Vector4					camInObSpace	(getCameraInverseTranslation (objectToCamera));
	Matrix4x4				objectToScreen(NO_CONSTRUCTOR);
	
	Math::productFromLeftProjection (objectToScreen, objectToCamera, cam->getCameraToScreen());

	TempArray<Vector4,false> vertices(numVertices);							// allocate temporary memory
	Math::transform (&vertices[0],getVertices(),numVertices, objectToScreen);	// transform vertices

	for (int i = 0; i < numVertices; i++)
	if (vertices[i].z <= -vertices[i].w)									// vertex is front-clipping
		vertexClipBits.set(i);

	//--------------------------------------------------------------------
	// Find all partially clipping, front-facing triangles and collect 
	// them to the 'clippingTriangles' array. If the triangle is 
	// completely in front of the front-clipping plane, we can discard it.
	//--------------------------------------------------------------------

	const UINT16*			trianglePlanes	= derived->m_trianglePlanes;		// triangle plane indices
	const Vector4*			plEq			= derived->m_plEq;

	int						numTriangles	= getTriangleCount();
	const Vector3i*			triVertices		= getTriangleVertices();
	TempArray<int,false>	clippingTriangles(numTriangles);					// DEBUG DEBUG
	int						numClippingTriangles = 0;

	for (int i = 0; i < numTriangles; i++)
	{
		const Vector4& p = plEq[trianglePlanes[i]];	// fetch plane equation for the triangle
		if ((p.x*camInObSpace.x + p.y*camInObSpace.y + p.z*camInObSpace.z + p.w*camInObSpace.w)>=0.0f)		// if front-facing..
		{
			const Vector3i& v = triVertices[i];
			if (vertexClipBits.test(v.i) || vertexClipBits.test(v.j) || vertexClipBits.test(v.k))			// triangle is clipping...
			{
				if (!(vertexClipBits.test(v.i) && vertexClipBits.test(v.j) && vertexClipBits.test(v.k)))	// if there's at least one non-clipping vertex.
					clippingTriangles[numClippingTriangles++] = i;											// ..tag triangle as clipping
			} 
		}
	}

	//--------------------------------------------------------------------
	// Nothing was visible??
	//--------------------------------------------------------------------

	if (!inf.m_numVertices && !numClippingTriangles)
		return false;

	//--------------------------------------------------------------------
	// Allocate space for output vertices + vertices potentially produced
	// by clipping (each clipping triangle may produce up to 4 new vertices).
	//--------------------------------------------------------------------

	TempArray<Vector2,false>	proj(inf.m_numVertices + numClippingTriangles*4);
	int							numClippedVertices	= 0;

	//--------------------------------------------------------------------
	// Process clipping triangles (if any)
	//--------------------------------------------------------------------

	if (numClippingTriangles)
	{
		Vector4 src[3];
		Vector4 dst[4];

		for (int i = 0; i < numClippingTriangles; i++)
		{
			int				index = clippingTriangles[i];					// triangle index
			const Vector3i&	ind	  = triVertices[index];						// get vertices

			src[0] = vertices[ind.i];										// setup source (homogenous coordinates)
			src[1] = vertices[ind.j];
			src[2] = vertices[ind.k];

			int N = clipHomogenousFrontPlane (dst,src);						// perform clipping (returns # of vertices)

			if (!N)															// triangle was completely removed
				continue;

			//--------------------------------------------------------------------
			// Transform output vertices into raster-space and then append them
			// into the vertex list. Then add corresponding edges to the output
			// silhouette.
			//--------------------------------------------------------------------

			DPVS_ASSERT(N==3 || N==4);

			int offset = numClippedVertices;
			Math::screenToRasterDivByW	(&proj[offset],dst,N,cam->getRasterViewport());	// screen->raster

			for (int e = 0; e < N; e++)												// add edges
			{
				int A = e;
				int B = A + 1;
				if (B == N)
					B = 0;

				if (dst[A].z != -dst[A].w &&										// non-clipping edge.. discard..
					dst[B].z != -dst[B].w)				
					continue;

				A+=offset;
				B+=offset;
				s.addEdge(proj[A].y < proj[B].y ? 0 : 1,A,B);						// edge direction
			}

			numClippedVertices+=N;													// advance clipped vertex counter
		}
	}

	//--------------------------------------------------------------------
	// Now take the input silhouette and add all non-clipping edges. Also
	// project the vertices in the input silhouette.
	//--------------------------------------------------------------------

	if (inf.m_numVertices)
	{
		Math::transform (&vertices[0],inf.m_loc,inf.m_numVertices, objectToScreen);		// transform input silhouette vertices

		vertexClipBits.clearAll();												// reuse the vertex clip bit array...

		for (int i = 0; i < inf.m_numVertices; i++)
		if (vertices[i].z <= -vertices[i].w)
		{
			vertexClipBits.set(i);
			vertices[i].w = 1.0f;												// DEBUG DEBUG HACK HACK (this is done to avoid floating point exceptions in the divbyw below)
		}

		Math::screenToRasterDivByW	(&proj[numClippedVertices],&vertices[0],inf.m_numVertices,cam->getRasterViewport());	// screen->raster

		for (int i = 0; i < inf.m_numEdges; i++)
		{
			int A = inf.m_edges[i].i;
			int B = inf.m_edges[i].j;

			if (vertexClipBits.test(A) || vertexClipBits.test(B))				// reject edge as it's clipping..
				continue;

			A += numClippedVertices;
			B += numClippedVertices;

			s.addEdge(proj[A].y < proj[B].y ? 0 : 1,A,B);						// edge direction
		}
	}

	//--------------------------------------------------------------------
	// Add all vertices into the output silhouette
	//--------------------------------------------------------------------

	s.setVertices (&proj[0],numClippedVertices+inf.m_numVertices);

	//--------------------------------------------------------------------
	// Compute OBB write planes
	//--------------------------------------------------------------------

	SilhouetteMath::computeOBBWritePlanes (s, objectToCamera, cam, getOBB()); 	// Setup Z-rasterization
	return true;																// Indicate success
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::testShaftIntersection()
 *
 * Description:		Tests whether object intersects the back-projection shaft
 *
 * Parameters:		objectToCamera	= object->camera matrix
 *					cam				= pointer to camera
 *			
 * Returns:			boolean value indicating whether model intersects the back-projection shaft of the camera
 *
 * Notes:			Only front-clipping objects can intersect the shaft,
 *					so please perform a range test first...
 *
 *****************************************************************************/

bool ImpMeshModel::testShaftIntersection(const Matrix4x3& objectToCamera, ImpCamera* cam) 
{
	//--------------------------------------------------------------------
	// Calculate back projection shaft. Then perform object mesh vs.
	// shaft intersection test to see if the object intersects the
	// shaft -- such objects need to be clipped (triangle by triangle)..
	//--------------------------------------------------------------------

	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_WRITEQUEUEFRONTCLIPPINGOCCLUDERSTESTED,1));

	AABB	aabb;
	cam->getBackProjectionShaft(aabb);
	aabb.setMin(2, -2.f*getSphere().getRadius() * objectToCamera.getMaxScale());

	//--------------------------------------------------------------------
	// The idea here is the following: if we have a sheared frustum
	// (for example scissoring generates such a frustum), the "back-
	// projection shaft" must be sheared also. Because we want to use an
	// AABB for the intersection testing, we shear the _object_ geometry
	// instead. This is done by calculating an inverse shearing matrix
	// and applying it to the object->camera matrix. The end result is
	// sheared object geometry that can be tested against an AABB.
	//--------------------------------------------------------------------

	Vector2 shear( (aabb.getMin().x+aabb.getMax().x)*0.5f, (aabb.getMin().y+aabb.getMax().y)*0.5f);

	if(shear.x!=0.f || shear.y!=0.f)
	{
		Matrix4x3 shearMatrix;		// will init matrix to identity

		shearMatrix[0][2] = shear.x / aabb.getMax().z;	
		shearMatrix[1][2] = shear.y / aabb.getMax().z;
		shearMatrix[0][3] = -shear.x;
		shearMatrix[1][3] = -shear.y;

		// object->"inverse sheared camera" = o->cell * cell->camera * inverse shear
		
		// NOTE THAT MATRIX INVERSION HERE IS EQUIVALENT TO JUST NEGATING SHEAR.X AND SHEAR.Y ORIGINALLY...

		Math::invertMatrix(shearMatrix);

		Matrix4x3 shearedObjectToCamera(NO_CONSTRUCTOR);
		Math::productFromLeft(shearedObjectToCamera,objectToCamera,shearMatrix);

		if (intersectOBBAABB(getOBB(),shearedObjectToCamera,aabb))
			return true;

	} else
	{
		if (intersectOBBAABB(getOBB(),objectToCamera,aabb))
			return true;
	}
	return false;					// does not intersect shaft
}

/*****************************************************************************
 *
 * Function:		ImpMeshModel::getSilhouette()
 *
 * Description:		Calculates write silhouette of the mesh model
 *
 * Parameters:		s				= destination silhouette structure
 *					range			= reference to range for holding the depth range of the silhouette
 *					objectToCamera	= object->camera matrix
 *					cam				= pointer to camera
 *			
 * Returns:			boolean value indicating whether silhouette is valid or not		
 *
 *****************************************************************************/

bool ImpMeshModel::getWriteSilhouette (EdgeSilhouette& s, Range<float>& range, const Matrix4x3 &objectToCamera, ImpCamera *cam)
{
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELWRITESILHOUETTESQUERIED,1));

	//--------------------------------------------------------------------
	// Query min & max depth values from the OBB. We cannot use depth
	// values of the silhouette. Check if model is front-clipping.
	// Then clamp the values to [0,1] range.
	//--------------------------------------------------------------------

	SilhouetteMath::computeOBBDepthRange (range,objectToCamera,cam,getOBB());		// compute OBB depth range

	if(range.getMax()<=0.f || range.getMin()>=1.0f)									// entirely in front of near plane or behind back plane
		return false;

	bool frontClipping = (range.getMin() <= 0.0f);									// set the front-clipping flag?

	range.clampToUnit();															// clamp the values to [0,1] range.

	//--------------------------------------------------------------------
	// If we don't support front-clipping occluders (debug testing only!!),
	// then return now if the object is front-clipping.
	//--------------------------------------------------------------------

#if !defined (DPVS_FRONT_CLIPPING_OCCLUDERS)
	if (frontClipping)
		return false;
#endif
	//--------------------------------------------------------------------
	// Get the silhouette from the silhouette cache.
	//--------------------------------------------------------------------

	SilhouetteCache::EdgeSilhouetteInfo inf;
	SilhouetteCache::get()->getSilhouette(this,inf,getCameraInverseTranslation (objectToCamera));

	//--------------------------------------------------------------------
	// If the model is front-clipping, we must determine what we should do
	// with it - this is because in some cases front-clipping silhouettes
	// are not valid. There are basically two options: if the object does
	// not intersect the "back projection shaft", we can perform a faster
	// operation called "splatting". Otherwise, we need to perform triangle-
	// level "clipping".
	//
	// Note that splatting is currently supported only for perspective
	// projection. If ortho projection is used, we always perform front-
	// clipping.
	//--------------------------------------------------------------------

	if(frontClipping)
	{

#if defined (DPVS_PS2)
		// we don't perform shaft-intersection testing or splatting on
		// the PS2
//PS2-BEGIN
		bool performClipping = true;
//PS2-END
#else

		bool performClipping = false;
		if (VQData::get().isOrthoProjection() || testShaftIntersection(objectToCamera,cam))					
			performClipping = true;
#endif

		if (performClipping) // .. perform fromt-clipping
			return frontClipWriteSilhouette(s,objectToCamera,inf,cam);	

		//--------------------------------------------------------------------
		// If the silhouette is front-clipping (but it did not intersect the
		// back-projection shaft) we must perform some extra work in order to 
		// use the silhouette. This is handled by the splatting routine.
		//--------------------------------------------------------------------

		if(!inf.m_numVertices)											// silhouette does not contain any edges (completely back-face culled?)
			return false;

		splatWriteSilhouette(s,objectToCamera,inf,cam);					// perform splatting
		return true;													// our work here is done..
	}

	//--------------------------------------------------------------------
	// If silhouette does not contain any edges/vertices, we can return
	// at this point (object is completely back-facing). Note that we
	// could not perform this test before the 'frontClipping' test because
	// there are cases where a completely front-facing object can generate
	// edges due to front-plane clipping..
	//--------------------------------------------------------------------

	if(!inf.m_numVertices)												// silhouette does not contain any edges (completely back-face culled?)
		return false;

	//--------------------------------------------------------------------
	// Calculate object-to-screen transformation matrix.
	//--------------------------------------------------------------------

	Matrix4x4 objectToScreen(NO_CONSTRUCTOR);
	Math::productFromLeftProjection(objectToScreen, objectToCamera, cam->getCameraToScreen());

	//--------------------------------------------------------------------
	// Transform and project the silhouette vertices. At this point we 
	// know that none of the edges are front-clipping, so we can safely
	// map them into screen-space without further checks.
	//--------------------------------------------------------------------

	TempArray<Vector2,false> proj(inf.m_numVertices);
	Math::objectToRaster (&proj[0], inf.m_loc, inf.m_numVertices, objectToScreen, cam->getRasterViewport());
	s.setVertices(&proj[0],inf.m_numVertices);								// copy data into silhouette

	//--------------------------------------------------------------------
	// Copy edges to the silhouette. The left/right selection is based
	// on the 'direction' of the edge (compare edge vertex projected
	// Y values).
	//--------------------------------------------------------------------

	int					nEdges	= inf.m_numEdges;
	const Vector2i*		edges	= inf.m_edges;

	for (int i = 0; i < nEdges; i++)
	{
		int A = edges[i].i, B = edges[i].j;
		s.addEdge(proj[A].y < proj[B].y ? 1 : 0,A,B);
	}

	//--------------------------------------------------------------------
	// Setup Z-rasterization. This is done by scanning the OBB back
	// faces and configuring the silhouette's Z-interpolators 
	// correspondingly.
	//--------------------------------------------------------------------

	SilhouetteMath::computeOBBWritePlanes (s, objectToCamera, cam, getOBB());

	//--------------------------------------------------------------------
	// Indicate that the resulting silhouette is valid and can be used
	// as an occluder.
	//--------------------------------------------------------------------

	return true;
}

//------------------------------------------------------------------------
