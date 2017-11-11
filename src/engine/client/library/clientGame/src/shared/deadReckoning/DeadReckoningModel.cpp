//===================================================================
//
// DeadReckoningModel.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DeadReckoningModel.h"

//===================================================================

namespace
{
	inline const float computeCubic (const float p0, const float p1, const float p2, const float p3, const float t)
	{
		//-- standard cubic basis equation
		const float a = p3 - (3 * p2) + (3 * p1) - p0;    //lint !e834  //-- - followed by + is confusing
		const float b = (3 * p2) - (6 * p1) + (3 * p0);   //lint !e834  //-- - followed by + is confusing
		const float c = (3 * p1) - (3 * p0);
		const float d = p0;
	
		return (a * t * t * t) + (b * t * t) + (c * t) + d;
	}
}

//===================================================================
// PUBLIC DeadReckoningModel
//===================================================================

DeadReckoningModel::DeadReckoningModel () :
	m_startTransform (),
	m_startVelocity (),
	m_endTransform (),
	m_endVelocity ()
{
}

//-------------------------------------------------------------------

DeadReckoningModel::~DeadReckoningModel ()
{
}

//-------------------------------------------------------------------

void DeadReckoningModel::set (const Transform& startTransform_w, const Vector& startVelocity_w, const Transform& endTransform_w, const Vector& endVelocity_w)
{
	m_startTransform = startTransform_w;
	m_startVelocity  = startVelocity_w;
	m_endTransform   = endTransform_w;
	m_endVelocity    = endVelocity_w;
}

//-------------------------------------------------------------------

float DeadReckoningModel::computeLength () const
{
	//-- compute the length by walking the spline and summing segments
	Vector    lastPosition = m_startTransform.getPosition_p ();
	float     length       = 0.f;
	Transform s (Transform::IF_none);
	const int steps        = 50;

	int i;
	for (i = 0; i < steps; ++i)
	{
		computeTransform (static_cast<float> (i) / (steps - 1), s);

		length       += lastPosition.magnitudeBetween (s.getPosition_p ());
		lastPosition  = s.getPosition_p ();
	}

#ifdef _DEBUG
	//-- sanity check by making sure the path length was at least as long as the magnitude between the points
	const float debugLength = m_startTransform.getPosition_p ().magnitudeBetween (m_endTransform.getPosition_p ());
	DEBUG_WARNING (length < debugLength, ("length (%1.2f) < debugLength (%1.2f)", length, debugLength));
#endif

	return length;
}

//-------------------------------------------------------------------

void DeadReckoningModel::computeTransform (const float /*t*/, Transform& /*transform_w*/) const
{
	DEBUG_FATAL (true, ("this should not be called"));
}

//-------------------------------------------------------------------

void DeadReckoningModel::computeVelocity (const float t, Vector& velocity) const
{
	//-- the cubic is invalid outside the range of 0 to 1
	const float clampedT = clamp (0.f, t, 1.f);

	//-- lerp velocity
	velocity = Vector::linearInterpolate (m_startVelocity, m_endVelocity, clampedT);
}

//===================================================================
// PRIVATE DeadReckoningModel
//===================================================================

const Vector DeadReckoningModel::computePosition (const Vector* const vList, const float t) const
{
	//-- the cubic is invalid outside the range of 0 to 1
	const float clampedT = clamp (0.f, t, 1.f);

	//-- computing the cubic in 3d is computing the cubic on each component
	Vector position;
	position.x = computeCubic (vList [0].x, vList [1].x, vList [2].x, vList [3].x, clampedT);
	position.y = computeCubic (vList [0].y, vList [1].y, vList [2].y, vList [3].y, clampedT);
	position.z = computeCubic (vList [0].z, vList [1].z, vList [2].z, vList [3].z, clampedT);

	return position;
}

//===================================================================
