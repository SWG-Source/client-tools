//
// ListObject.h
// asommers 6-11-2000
//
// copyright 2000, verant interactive, inc.
//

//-------------------------------------------------------------------

#ifndef INCLUDED_ListObject_H
#define INCLUDED_ListObject_H

//-------------------------------------------------------------------

class RandomGenerator;

//-------------------------------------------------------------------

#include "sharedObject/Object.h"

//-------------------------------------------------------------------

class ListObject : public Object
{
private:

	real yawRotation;
	real pitchRotation;
	real rollRotation;

private:

	ListObject (const ListObject&);
	ListObject& operator= (const ListObject&);

public:

	ListObject (real newYawRotation=CONST_REAL (0), real newPitchRotation=CONST_REAL (0), real newRollRotation=CONST_REAL (0));
	virtual ~ListObject (void);

	void          assignRandomRotations (RandomGenerator* randomGenerator);

	virtual float alter (float time);
};

//-------------------------------------------------------------------

#endif
