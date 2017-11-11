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
 * Description:		Model base class code
 *
 * $Archive: /dpvs/implementation/sources/dpvsImpModel.cpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 23.10.01 15:31 $
 * $Date: 2003/06/19 $
 * 
 ******************************************************************************/

#include "dpvsImpModel.hpp"
#include "dpvsBounds.hpp"
#include "dpvsClipPolygon.hpp"
#include "dpvsRandom.hpp"
#include "dpvsDebug.hpp"
#include "dpvsInstanceCount.hpp"
#include "dpvsStatistics.hpp"
#include "dpvsRectangle.hpp"
#include "dpvsImpObject.hpp"
#include "dpvsVQData.hpp"

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		ImpModel::ImpModel()
 *
 * Description:		Constructor
 *
 *****************************************************************************/

ImpModel::ImpModel (void) :
	m_OBB				(),
	m_sphere			(),
	m_backFaceCullable	(false),
	m_occlusionWritable	(false),
	m_exactOBB			(false),
	m_solid				(false)
{
	InstanceCount::incInstanceCount(InstanceCount::MODEL);
}

/*****************************************************************************
 *
 * Function:		ImpModel::~ImpModel()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

ImpModel::~ImpModel	(void)
{
	InstanceCount::decInstanceCount(InstanceCount::MODEL);
}

/*****************************************************************************
 *
 * Function:		ImpModel::estimateGeometryCost()
 *
 * Description:		Estimates geometry cost of an object in cycles.
 *
 * Parameters:		
 *
 *****************************************************************************/

float ImpModel::estimateGeometryCost(int vnum,int tnum)
{
	return ImpObject::calculateRenderCost(vnum,tnum,1.f);
}

/*****************************************************************************
 *
 * Function:		ImpModel::getOBBRectangle()
 *
 * Description:		Calculates raster-space rectangle from the OBB of the model
 *
 * Parameters:		r		= reference to output rectangle structure 
 *				
 * Returns:			true if rectangle is valid, false otherwise
 *
 *****************************************************************************/

bool ImpModel::getOBBRectangle (FloatRectangle& r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	DPVS_PROFILE(Statistics::incStatistic(Library::STAT_MODELRECTANGLESQUERIED,1));
	
	bool valid = SilhouetteMath::getRectangle_FLOW(r);

	if(valid)
	{
		DPVS_ASSERT(VQData::get().testProperties(VQData::DEPTH_RANGE_VALID));
		if(Debug::getLineDrawFlags() & Library::LINE_RECTANGLES)		// Debug output?
		{
			Library::LineType type = Library::LINE_RECTANGLES;
			Debug::drawLine(type, Vector2(r.x0, r.y0), Vector2(r.x1, r.y0), Vector4(1.f, 1.f, 1.f, 0.3f));
			Debug::drawLine(type, Vector2(r.x1, r.y0), Vector2(r.x1, r.y1), Vector4(1.f, 1.f, 1.f, 0.3f));
			Debug::drawLine(type, Vector2(r.x1, r.y1), Vector2(r.x0, r.y1), Vector4(1.f, 1.f, 1.f, 0.3f));
			Debug::drawLine(type, Vector2(r.x0, r.y1), Vector2(r.x0, r.y0), Vector4(1.f, 1.f, 1.f, 0.3f));
		}
	}

	return valid;
}

/*****************************************************************************
 *
 * Function:		ImpModel::getExactRectangle()
 *
 * Description:		Default implementation falls back to getRectangle()
 *
 *****************************************************************************/

bool ImpModel::getExactRectangle(FloatRectangle& r)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	return getOBBRectangle(r);
}

bool ImpModel::backFaceCull (const Vector4&) const		
{ 
	DPVS_ASSERT(0); 
	return true; 
}

void ImpModel::getTestMesh (VertexArray& v)	const
{ 
	v.m_vertexPositions = null;
	v.m_vertexCount		= 0; 
}

const Mesh* ImpModel::getTestMesh	(void) const						
{ 
	return null;	
}

float ImpModel::getGeometryCost (void) const				
{ 
	return 1.f; 
}

bool ImpModel::getTestSilhouette (EdgeSilhouette& s)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	return SilhouetteMath::computeOBBTestSilhouette_FLOW(s);
}

bool ImpModel::getWriteSilhouette (EdgeSilhouette &, Range<float>& , const Matrix4x3&, ImpCamera*)		
{ 
	return false; 
}

const OBB& ImpModel::getOBB (void)
{
	if (!m_exactOBB && !(g_random.getI()&127))
		computeExactOBB();										// compute exact OBB?

	DPVS_ASSERT(Math::isInvertible(m_OBB.getMatrix()));		// make sure exact OBB is invertible..

	return m_OBB;
}

const OBB& ImpModel::getExactOBB(void)
{
	if(!m_exactOBB)
		computeExactOBB();
	DPVS_ASSERT(Math::isInvertible(m_OBB.getMatrix()));		// make sure exact OBB is invertible..
	return m_OBB;
}

// routine for enhancing current OBB
void ImpModel::computeExactOBB (void)
{
	// nada
}

//------------------------------------------------------------------------
