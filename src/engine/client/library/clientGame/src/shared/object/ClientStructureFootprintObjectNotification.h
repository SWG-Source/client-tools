//===================================================================
//
// ClientStructureFootprintObjectNotification.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientStructureFootprintObjectNotification_H
#define INCLUDED_ClientStructureFootprintObjectNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

class Rectangle2d;

//===================================================================

class ClientStructureFootprintObjectNotification : public ObjectNotification
{
public:

	static void install ();
	static ClientStructureFootprintObjectNotification& getInstance ();
	static void terrainChanged (Rectangle2d const & extent);

public:

	ClientStructureFootprintObjectNotification ();
	virtual ~ClientStructureFootprintObjectNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	void resetBoundary (Object & object) const;

private:

	ClientStructureFootprintObjectNotification (const ClientStructureFootprintObjectNotification&);
	ClientStructureFootprintObjectNotification& operator= (const ClientStructureFootprintObjectNotification&);

private:

	static ClientStructureFootprintObjectNotification ms_instance;
};

//===================================================================

#endif
