// ======================================================================
//
// SequenceObject.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SequenceObject_H
#define INCLUDED_SequenceObject_H

// ======================================================================

#include "sharedObject/Object.h"

// ======================================================================
/**
 * Provide an object that sequences a collection of other objects.
 *
 * This instance maintains a collection of Object instances partitioned
 * into phases.  One phase is active at any given time.  All objects in the
 * active phase are updated and altered like normal objects.  When all
 * objects in the active phase have died (i.e. every object in the phase
 * has returned false from alter()), the object moves to the next phase.
 * When no more objects exist in any phase, the SequenceObject instance
 * returns false from its alter.
 */

class SequenceObject: public Object
{
public:

	typedef stdvector<Object*>::fwd  ObjectVector;
	typedef stdvector<int>::fwd      IntVector;

public:

	SequenceObject(const ObjectVector &objects, const IntVector &phases);
	virtual ~SequenceObject();

	virtual float  alter(float time);

private:

	void  findPhaseRange(int phaseBeginIndex, int &phaseEndIndex);

	void  queuePhaseObjectsInWorld();
	bool  allPhaseObjectsDead();

	// disabled
	SequenceObject();
	SequenceObject(const SequenceObject&);
	SequenceObject &operator =(const SequenceObject&);

private:

	class ObjectData;

	typedef stdvector<ObjectData*>::fwd  ObjectDataVector;

private:

	ObjectDataVector *m_objectDataVector;

	int               m_phaseBeginIndex;
	int               m_phaseEndIndex;
};

// ======================================================================

#endif
