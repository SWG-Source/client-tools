//===================================================================
//
// DeadReckoningModelOmniDirectional.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DeadReckoningModelOmniDirectional_H
#define INCLUDED_DeadReckoningModelOmniDirectional_H

//===================================================================

#include "clientGame/DeadReckoningModel.h"

//===================================================================

class DeadReckoningModelOmniDirectional : public DeadReckoningModel
{
public:

	DeadReckoningModelOmniDirectional ();
	virtual ~DeadReckoningModelOmniDirectional ();

	virtual void computeTransform (float t, Transform& transform_w) const;
};

//===================================================================

#endif
