//========================================================================
//
// StaticObject.h
//
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
//========================================================================

#ifndef INCLUDED_StaticObject_H
#define INCLUDED_StaticObject_H

// ======================================================================

#include "clientGame/ClientObject.h"

class SharedStaticObjectTemplate;


// ======================================================================
/**
  *  A StaticObject is an object that does not has a physical representation in the world.
  */

class StaticObject : public ClientObject
{
public:

	explicit  StaticObject(const SharedStaticObjectTemplate* newTemplate);
	virtual  ~StaticObject();

	virtual StaticObject *       asStaticObject();
	virtual StaticObject const * asStaticObject() const;

	virtual void endBaselines ();

private:

	// disabled
	StaticObject();
	StaticObject(const StaticObject& rhs);
	StaticObject&	operator=(const StaticObject& rhs);

private:

};

// ======================================================================


#endif	// INCLUDED_StaticObject_H
