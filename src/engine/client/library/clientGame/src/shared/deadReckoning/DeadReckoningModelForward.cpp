//===================================================================
//
// DeadReckoningModelForward.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DeadReckoningModelForward.h"

//===================================================================

DeadReckoningModelForward::DeadReckoningModelForward () :
	DeadReckoningModel ()
{
}

//-------------------------------------------------------------------

DeadReckoningModelForward::~DeadReckoningModelForward ()
{
}

//-------------------------------------------------------------------

void DeadReckoningModelForward::computeTransform (const float t, Transform& transform_w) const
{
	//-- if our distance to move is too small, just assume the end transform
	const float minimumLength = m_startTransform.getPosition_p ().magnitudeBetween (m_endTransform.getPosition_p ());
	if (minimumLength < Vector::NORMALIZE_THRESHOLD)
	{
		transform_w = m_endTransform;
		return;
	}

	//-- the four points should be start, start + startVelocity, end - endVelocity, end
	const Vector vList [4] =
	{
		m_startTransform.getPosition_p (),
		m_startTransform.getPosition_p () + (m_startTransform.getLocalFrameK_p () * 0.1f),
		m_endTransform.getPosition_p () - (m_endTransform.getLocalFrameK_p () * 0.1f),
		m_endTransform.getPosition_p ()
	};

	//-- the transform will orient along the cubic spline by sampling two points and computing
	//	 the direction between them. since I can only move forward, orient me along the spline
	const float step = 0.02f;

	Vector start;
	Vector end;
	Vector position;

	//-- if t is too close to the end, use t - step and t, otherwise use t and t + step
	if (t >= 1.f - step)
	{
		start    = computePosition (vList, t - step);
		end      = computePosition (vList, t);
		position = end;
	}
	else
	{
		start    = computePosition (vList, t);
		end      = computePosition (vList, t + step);
		position = start;
	}

	end -= start;

	//-- if the transform has not moved, just assume the end transform's direction. otherwise, compute new transform
	if (end.magnitudeSquared () <= Vector::NORMALIZE_THRESHOLD)
	{
		transform_w = m_endTransform;
	}
	else
	{
		IGNORE_RETURN (end.normalize ());

		transform_w.setLocalFrameKJ_p (end, Vector::unitY);
		transform_w.reorthonormalize ();
		transform_w.setPosition_p (position);
	}
}

//===================================================================
