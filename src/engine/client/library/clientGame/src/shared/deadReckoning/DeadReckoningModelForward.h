//===================================================================
//
// DeadReckoningModelForward.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DeadReckoningModelForward_H
#define INCLUDED_DeadReckoningModelForward_H

//===================================================================

#include "clientGame/DeadReckoningModel.h"

//===================================================================

class DeadReckoningModelForward : public DeadReckoningModel
{
public:

	DeadReckoningModelForward ();
	virtual ~DeadReckoningModelForward ();

	virtual void computeTransform (float t, Transform& transform_w) const;
};

//===================================================================

#endif
