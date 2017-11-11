// ======================================================================
//
// ClientStructureFootprintObject.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientStructureFootprintObject_H
#define INCLUDED_ClientStructureFootprintObject_H

// ======================================================================

#include "sharedObject/Object.h"

class ObjectList;
class StructureFootprint;

// ======================================================================

class ClientStructureFootprintObject : public Object
{
public:

	static void install ();

public:

	explicit ClientStructureFootprintObject (const StructureFootprint& structureFootprint);
	virtual ~ClientStructureFootprintObject ();

	void resetBoundary ();

private:

	void create (const StructureFootprint& structureFootprint);

private:

	ClientStructureFootprintObject ();
	ClientStructureFootprintObject (const ClientStructureFootprintObject&);
	ClientStructureFootprintObject& operator= (const ClientStructureFootprintObject&);
	
private:

	ObjectList* const m_objectList;
	Object*           m_ribbonObject;
};

// ======================================================================

#endif

