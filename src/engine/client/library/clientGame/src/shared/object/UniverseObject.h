// ======================================================================
//
// UniverseObject.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UniverseObject_H
#define INCLUDED_UniverseObject_H

// ======================================================================

#include "clientGame/ClientObject.h"

// ======================================================================

class SharedUniverseObjectTemplate;

// ======================================================================

class UniverseObject: public ClientObject
{
public:
	explicit UniverseObject(SharedUniverseObjectTemplate const *newTemplate);
	virtual ~UniverseObject();

private:
	UniverseObject();
	UniverseObject(UniverseObject const &rhs);
	UniverseObject&	operator=(UniverseObject const &rhs);
};

// ======================================================================

#endif	// INCLUDED_UniverseObject_H

