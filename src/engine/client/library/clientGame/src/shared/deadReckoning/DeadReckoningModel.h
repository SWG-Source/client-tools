//===================================================================
//
// DeadReckoningModel.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DeadReckoningModel_H
#define INCLUDED_DeadReckoningModel_H

//===================================================================

#include "sharedMath/Transform.h"

//===================================================================

class DeadReckoningModel
{
public:

	DeadReckoningModel ();
	virtual ~DeadReckoningModel ();

	void         set (const Transform& startTransform_w, const Vector& startVelocity_w, const Transform& endTransform_w, const Vector& endVelocity_w);

	virtual void computeTransform (float t, Transform& transform_w) const;
	float        computeLength () const;
	void         computeVelocity (float t, Vector& velocity_w) const;

protected:

	Transform    m_startTransform;
	Vector       m_startVelocity;
	Transform    m_endTransform;
	Vector       m_endVelocity;

protected:

	const Vector computePosition (const Vector* vList, float t) const;
};

//===================================================================

#endif
