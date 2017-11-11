//===================================================================
//
// DeadReckoningModelOmniDirectional.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/DeadReckoningModelOmniDirectional.h"

#include "sharedMath/Quaternion.h"

//===================================================================

DeadReckoningModelOmniDirectional::DeadReckoningModelOmniDirectional () :
	DeadReckoningModel ()
{
}

//-------------------------------------------------------------------

DeadReckoningModelOmniDirectional::~DeadReckoningModelOmniDirectional ()
{
}

//-------------------------------------------------------------------

void DeadReckoningModelOmniDirectional::computeTransform (const float t, Transform& transform_w) const
{
	//-- if our distance to move is too small, just assume the end transform
	const float minimumLength = m_startTransform.getPosition_p ().magnitudeBetween (m_endTransform.getPosition_p ());
	if (minimumLength < Vector::NORMALIZE_THRESHOLD)
	{
		transform_w = m_endTransform;
		return;
	}

	//-- the four points should be start, start + startVelocity, end - endVelocity, end
	Vector startOffset = m_startVelocity.magnitudeSquared () < Vector::NORMALIZE_THRESHOLD ? m_startTransform.getLocalFrameK_p () : m_startVelocity;
	startOffset.normalize ();
	startOffset *= 0.1f;

	Vector endOffset = m_endVelocity.magnitudeSquared () < Vector::NORMALIZE_THRESHOLD ? m_endTransform.getLocalFrameK_p () : m_endVelocity;
	endOffset.normalize ();
	endOffset *= 0.1f;

	const Vector vList [4]   =
	{
		m_startTransform.getPosition_p (),
		m_startTransform.getPosition_p () + startOffset,
		m_endTransform.getPosition_p () - endOffset,
		m_endTransform.getPosition_p ()
	};

	//-- slerp orientation
	const Quaternion startOrientation (m_startTransform);
	const Quaternion endOrientation (m_endTransform);
	const Quaternion orientation (startOrientation.slerp (endOrientation, t));

	orientation.getTransform (&transform_w);
	transform_w.reorthonormalize ();

	//-- compute position
	transform_w.setPosition_p (computePosition (vList, t));
}

//===================================================================
