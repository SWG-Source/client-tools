//===================================================================
//
// ClientBattlefieldMarkerOutlineObjectNotification.h
// asommers
//
// copyright 2003, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientBattlefieldMarkerOutlineObjectNotification_H
#define INCLUDED_ClientBattlefieldMarkerOutlineObjectNotification_H

//===================================================================

#include "sharedObject/ObjectNotification.h"

class Rectangle2d;

//===================================================================

class ClientBattlefieldMarkerOutlineObjectNotification : public ObjectNotification
{
public:

	static void install ();
	static ClientBattlefieldMarkerOutlineObjectNotification& getInstance ();
	static void terrainChanged (Rectangle2d const & extent);

public:

	ClientBattlefieldMarkerOutlineObjectNotification ();
	virtual ~ClientBattlefieldMarkerOutlineObjectNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	void resetBoundary (Object & object) const;

private:

	ClientBattlefieldMarkerOutlineObjectNotification (const ClientBattlefieldMarkerOutlineObjectNotification&);
	ClientBattlefieldMarkerOutlineObjectNotification& operator= (const ClientBattlefieldMarkerOutlineObjectNotification&);

private:

	static ClientBattlefieldMarkerOutlineObjectNotification ms_instance;
};

//===================================================================

#endif
