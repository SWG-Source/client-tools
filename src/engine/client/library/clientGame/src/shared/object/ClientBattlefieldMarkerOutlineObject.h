// ======================================================================
//
// ClientBattlefieldMarkerOutlineObject.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientBattlefieldMarkerOutlineObject_H
#define INCLUDED_ClientBattlefieldMarkerOutlineObject_H

// ======================================================================

#include "sharedObject/Object.h"

class ObjectList;

// ======================================================================

class ClientBattlefieldMarkerOutlineObject : public Object
{
public:

	static void install ();

public:

	ClientBattlefieldMarkerOutlineObject (int numberOfPoles, float radius);
	virtual ~ClientBattlefieldMarkerOutlineObject ();

	void resetBoundary ();

private:

	void create (int numberOfPoles, float radius);

private:

	ClientBattlefieldMarkerOutlineObject ();
	ClientBattlefieldMarkerOutlineObject (const ClientBattlefieldMarkerOutlineObject&);
	ClientBattlefieldMarkerOutlineObject& operator= (const ClientBattlefieldMarkerOutlineObject&);
	
private:

	ObjectList* const m_objectList;
	Object*           m_ribbonObject;
};

// ======================================================================

#endif

