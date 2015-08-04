//===================================================================
//
// ClientPathObjectNotification.h
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientPathObjectNotification_H
#define INCLUDED_ClientPathObjectNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

class Rectangle2d;

//===================================================================

class ClientPathObjectNotification : public ObjectNotification
{
public:

	static void install ();
	static ClientPathObjectNotification& getInstance ();
	static void terrainChanged (Rectangle2d const & extent);

public:

	ClientPathObjectNotification ();
	virtual ~ClientPathObjectNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	void resetBoundary (Object & object) const;

private:

	ClientPathObjectNotification (const ClientPathObjectNotification&);
	ClientPathObjectNotification& operator= (const ClientPathObjectNotification&);

private:

	static ClientPathObjectNotification ms_instance;
};

//===================================================================

#endif
