/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2004 Hybrid Graphics, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irreparable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description:     OBB implementation
 *
 * $Archive: /dpvs/implementation/sources/dpvsOBB.cpp $
 * $Author: wili $
 * $Revision: #2 $
 * $Modtime: 4/24/02 16:28 $
 * $Date: 2004/02/05 $
 *
 ******************************************************************************/

#include "dpvsOBB.hpp"
#include "dpvsAABB.hpp"
#include "dpvsBounds.hpp"
#include "dpvsSphere.hpp"
#include "dpvsMath.hpp"
#include "dpvsTempArray.hpp"

using namespace DPVS;

namespace DPVS
{

/*****************************************************************************
 *
 * Function:        DPVS::getOBB()
 *
 * Description:     Internal routine for calculating the OBB from a matrix
 *                  and a set of vertices
 *
 * Parameters:      obb      = destination OBB
 *                  m        = source transformation matrix
 *                  v        = array of vertex positions
 *                  N        = number of vertices
 *
 *****************************************************************************/

static inline void getOBB (
    OBB&                obb, 
    const Matrix4x3&    m, 
    const Vector3*      v, 
    int                 N)
{
    //--------------------------------------------------------------------
    // Find extents (note: transposed transformation)
    //--------------------------------------------------------------------

    float minX = +FLT_MAX, minY = +FLT_MAX, minZ = +FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

    for (int i = 0; i < N; i++)
    {
        float x = m[0][0] * v[i][0] + m[1][0] * v[i][1] + m[2][0] * v[i][2];
        float y = m[0][1] * v[i][0] + m[1][1] * v[i][1] + m[2][1] * v[i][2];
        float z = m[0][2] * v[i][0] + m[1][2] * v[i][1] + m[2][2] * v[i][2];

        minX = Math::min(minX, x);
        maxX = Math::max(maxX, x);
        minY = Math::min(minY, y);
        maxY = Math::max(maxY, y);
        minZ = Math::min(minZ, z);
        maxZ = Math::max(maxZ, z);
    }

    Vector3 mn(minX, minY, minZ);
    Vector3 mx(maxX, maxY, maxZ);

    //--------------------------------------------------------------------
    // Construct 4x3 matrix for the OBB and set extents.
    //--------------------------------------------------------------------

    Vector3 center = (mn+mx)*0.5;
    Vector3 scale  = mx - center;

    DPVS_ASSERT(scale.x >= 0.0f && scale.y >= 0.0f && scale.z >= 0.0f);

    //--------------------------------------------------------------------
    // We force-clamp the scale values to EPSILON so that the matrix
    // always stays invertible even if the input data was completely planar ....
    //--------------------------------------------------------------------

    float maxScale = scale.x;
    maxScale = Math::max(maxScale, scale.y);
    maxScale = Math::max(maxScale, scale.z);

    float EPSILON = 1e-5f * maxScale;
    EPSILON = Math::max(EPSILON, 1e-20f);

    scale.x = Math::max(scale.x, EPSILON);
    scale.y = Math::max(scale.y, EPSILON);
    scale.z = Math::max(scale.z, EPSILON);

    Matrix4x3 d;
    float d0x = m[0][0]*scale.x;
    float d0y = m[0][1]*scale.y;
    float d0z = m[0][2]*scale.z;
    float d0w = m[0][0]*center.x + m[0][1]*center.y + m[0][2]*center.z;
    float d1x = m[1][0]*scale.x;
    float d1y = m[1][1]*scale.y;
    float d1z = m[1][2]*scale.z;
    float d1w = m[1][0]*center.x + m[1][1]*center.y + m[1][2]*center.z;
    float d2x = m[2][0]*scale.x;
    float d2y = m[2][1]*scale.y;
    float d2z = m[2][2]*scale.z;
    float d2w = m[2][0]*center.x + m[2][1]*center.y + m[2][2]*center.z;

    d[0].x = d0x; d[0].y = d0y; d[0].z = d0z; d[0].w = d0w;
    d[1].x = d1x; d[1].y = d1y; d[1].z = d1z; d[1].w = d1w;
    d[2].x = d2x; d[2].y = d2y; d[2].z = d2z; d[2].w = d2w;

    obb.setMatrix (d);
}

/*****************************************************************************
 *
 * Function:        DPVS::calculateOBB()
 *
 * Description:     Generates oriented bounding box bounding the vertices
 *
 * Parameters:      obb      = destination OBB
 *                  vertices = array of vertex positions
 *                  N        = number of vertices
 *
 * Notes:           Uses an O(N) algorithm which is sub-optimal in quality
 *                  but yields good results in practice.
 *
 *                  It is recommended that the convex hull vertices of the
 *                  object are passed in rather than the actual vertices
 *                  of the mesh (yields better results).
 *
 *****************************************************************************/

void calculateOBB (
    OBB&            obb, 
    const Vector3*  vertices, 
    int             N)
{
    //--------------------------------------------------------------------
    // Validate input data
    //--------------------------------------------------------------------

    DPVS_ASSERT (vertices && N>0);

#if defined (DPVS_DEBUG)
    for (int i = 0; i < N; i++)
        DPVS_API_ASSERT(Math::isFinite(vertices[i]) && "DPVS::calculateOBB() - input vertices contain invalid floats");
#endif

    //--------------------------------------------------------------------
    // New brute-force sampling code with a couple of twists for
    // generating better OBBs...
    //--------------------------------------------------------------------

    static const float HALF_PI = Math::PI*0.5f;

    //--------------------------------------------------------------------
    // Copy input vertex array so that we can swap the vertex order later.
    //--------------------------------------------------------------------

    TempArray<Vector3,false> v(N);
    for (int i = 0; i < N; i++)
        v[i] = vertices[i];

    const int   PASSES      = 10;                                       // # of passes
    float       bestArea    = FLT_MAX;                                  // best OBB surface area
    int         cnt         = 0;
    int         ccnt        = 0;
    int         indices[6]  = {0,0,0,0,0,0};                            // six "border" vertices
    Vector3     bestAngles(0.0f,0.0f,0.0f);                             // best rotation angles so far
    Matrix4x3   mtx;
    Matrix4x3   bestMatrix;                                             // best 4x3 matrix
    Vector3     start(0.0f,0.0f,0.0f);
    Vector3     end(HALF_PI,HALF_PI,HALF_PI);
    Vector3     stepper(HALF_PI/4,HALF_PI/4,HALF_PI/4);

    for (int p = 0; p < PASSES; p++)                                    // PASSES
    {
        for (float x = start.x; x < end.x; x += stepper.x)
        {
            const float fCos0 = Math::cos(x);
            const float fSin0 = Math::sin(x);

            for (float y = start.y; y < end.y; y += stepper.y)
            {
                const float     fCos1 = Math::cos(y);
                const float     fSin1 = Math::sin(y);
                const Vector3   axis(fCos0*fSin1,fSin0*fSin1,fCos1);

                for (float z = start.z; z < end.z; z += stepper.z)
                {
                    //------------------------------------------------------------
                    // Generate rotation matrix
                    //------------------------------------------------------------

                    const float c   = Math::cos(z);
                    const float s   = Math::sin(z);

                    mtx[0][0] = ( Math::sqr(axis[0]) + c*(1-Math::sqr(axis[0])));
                    mtx[0][1] = ( axis[0]*axis[1]*(1-c) + axis[2]*s);
                    mtx[0][2] = ( axis[2]*axis[0]*(1-c) - axis[1]*s);
                    mtx[1][0] = ( axis[0]*axis[1]*(1-c) - axis[2]*s);
                    mtx[1][1] = ( Math::sqr(axis[1]) + c*(1-Math::sqr(axis[1])));
                    mtx[1][2] = ( axis[1]*axis[2]*(1-c) + axis[0]*s);
                    mtx[2][0] = ( axis[2]*axis[0]*(1-c) + axis[1]*s);
                    mtx[2][1] = ( axis[1]*axis[2]*(1-c) - axis[0]*s);
                    mtx[2][2] = ( Math::sqr(axis[2]) + c*(1-Math::sqr(axis[2])));

                    //------------------------------------------------------------
                    // Transform vertices and scan new bounding box. Early-exit
                    // as soon as the surface area of the bounding box is
                    // greater than the best match found so far.
                    //------------------------------------------------------------

                    Vector3 mn(FLT_MAX,FLT_MAX,FLT_MAX);
                    Vector3 mx = -mn;

                    for (int i = 0; i < N; i++)
                    {
                        ccnt++;
                        bool mod = false;

                        float x = mtx[0].x * v[i].x + mtx[0].y * v[i].y + mtx[0].z * v[i].z;
                        float y = mtx[1].x * v[i].x + mtx[1].y * v[i].y + mtx[1].z * v[i].z;
                        float z = mtx[2].x * v[i].x + mtx[2].y * v[i].y + mtx[2].z * v[i].z;

                        if (x < mn.x) { mn.x = x; mod = true; indices[0] = i; }
                        if (x > mx.x) { mx.x = x; mod = true; indices[1] = i; }
                        if (y < mn.y) { mn.y = y; mod = true; indices[2] = i; }
                        if (y > mx.y) { mx.y = y; mod = true; indices[3] = i; }
                        if (z < mn.z) { mn.z = z; mod = true; indices[4] = i; }
                        if (z > mx.z) { mx.z = z; mod = true; indices[5] = i; }

                        if (mod)
                        {
                            Vector3 d(mx-mn);
                            float area = 2.0f*(d.x*d.y + d.x*d.z + d.y*d.z);
                            if ( area >= bestArea)
                                break;
                        }
                    }

                    //------------------------------------------------------------
                    // Optimize vertex order for next test, i.e., move the 
                    // six vertices at the bounding planes to the beginning
                    // of the vertex list.
                    //------------------------------------------------------------

                    if (N >= 6)
                    {
                        swap(v[0],v[indices[0]]);
                        swap(v[1],v[indices[1]]);
                        swap(v[2],v[indices[2]]);
                        swap(v[3],v[indices[3]]);
                        swap(v[4],v[indices[4]]);
                        swap(v[5],v[indices[5]]);
                    }

                    //------------------------------------------------------------
                    // Is this the best OBB so far?
                    //------------------------------------------------------------

                    cnt++;

                    Vector3 d(mx-mn);
                    float v = 2.0f*(d.x*d.y + d.x*d.z + d.y*d.z);

                    if (v < bestArea)
                    {
                        bestAngles.make(x,y,z);
                        bestMatrix = mtx;
                        bestArea = v;
                    }
                } // z loop
            } // y loop
        } // x loop

        //------------------------------------------------------------
        // End of pass, advance start/end points and stepper. The
        // scale values chosen are totally ad hoc.
        //------------------------------------------------------------

        start   = bestAngles - stepper*0.779232f;
        end     = bestAngles + stepper*0.745641f;
        stepper *= 0.51233984f;     
    }

    //------------------------------------------------------------
    // Transpose matrix
    //------------------------------------------------------------

    swap(bestMatrix[0][1],bestMatrix[1][0]);
    swap(bestMatrix[0][2],bestMatrix[2][0]);
    swap(bestMatrix[1][2],bestMatrix[2][1]);

    getOBB (obb,bestMatrix,vertices,N);

//  printf ("ops = %d, N = %d avg = %.3f\n",ccnt, N,(double)(ccnt)/(cnt));
//  printf ("cnt = %d, best area = %.8f\n",cnt, bestArea);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateOBB()
 *
 * Description:		Generates oriented bounding box from an AABB
 *
 * Parameters:		obb		 = destination OBB
 *					aabb	 = source AABB
 *
 * Notes:			The OBB will have an identity rotation matrix with
 *					scales and translation from the AABB.
 *
 *****************************************************************************/

void calculateOBB (OBB& obb, const AABB& aabb)
{
	Matrix4x3 m;
	m.ident();
	m.setTranslation(aabb.getCenter());

	//--------------------------------------------------------------------
	// Find out scale factors. Note that if we have zero scaling terms,
	// we clamp the value to EPSILON. Otherwise the resulting matrix
	// would be non-invertible. The value EPSILON has been found by
	// empirical testing.
	//--------------------------------------------------------------------

	float xscale = aabb.getAxisLength(0)*0.5f;
	float yscale = aabb.getAxisLength(1)*0.5f;
	float zscale = aabb.getAxisLength(2)*0.5f;

	float maxScale = xscale;
	maxScale = Math::max(maxScale, yscale);
	maxScale = Math::max(maxScale, zscale);

	float EPSILON = 1e-7f * maxScale;
	EPSILON = Math::max(EPSILON, 1e-12f);

	xscale = Math::max(xscale, EPSILON);
	yscale = Math::max(yscale, EPSILON);
	zscale = Math::max(zscale, EPSILON);

	Vector4 ext(xscale,yscale,zscale,1.0f);

	m[0] *= ext;
	m[1] *= ext;
	m[2] *= ext;

	obb.setMatrix (m);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateSphere()
 *
 * Description:		Generates sphere from OBB
 *
 * Parameters:
 *
 *****************************************************************************/

void calculateSphere	(Sphere& d, const OBB& s)
{
	float rx = s.getHalfExtentLength(0);
	float ry = s.getHalfExtentLength(1);
	float rz = s.getHalfExtentLength(2);

	d.setCenter( s.getMatrix().getTranslation() );
	d.setRadius( Math::sqrt(rx*rx + ry*ry + rz*rz) );	// DEBUG DEBUG is this correct?
}

/*****************************************************************************
 *
 * Function:		DPVS::OBB::getAABBExtentLength()
 *
 * Description:		Calculates axis-aligned lengths of OBB
 *
 * Parameters:
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void OBB::getAABBExtentLength(Vector3& l) const
{
	for (int i = 0; i < 3; i++)
		l[i] = Math::fabs(m_matrix[i][0]) + Math::fabs(m_matrix[i][1]) + Math::fabs(m_matrix[i][2]);
}

/*****************************************************************************
 *
 * Function:		DPVS::calculateAABB()
 *
 * Description:		Generates AABB from OBB
 *
 * Parameters:
 *
 *****************************************************************************/

void calculateAABB		(AABB& d, const OBB& obb)
{
	Vector3 center = obb.getMatrix().getTranslation();
	Vector3 size;
	obb.getAABBExtentLength(size);

	d.setMin(center-size);
	d.setMax(center+size);
}

} //DPVS

//------------------------------------------------------------------------
