//===================================================================
//
// ClientObjectTerrainModificationNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientObjectTerrainModificationNotification_H
#define INCLUDED_ClientObjectTerrainModificationNotification_H

//===================================================================

class ProceduralTerrainAppearance;

#include "sharedObject/ObjectNotification.h"

//===================================================================

class ClientObjectTerrainModificationNotification : public ObjectNotification
{
public:

	static void install ();
	static void setTerrainAppearance (ProceduralTerrainAppearance* const terrainAppearance);
	static ClientObjectTerrainModificationNotification& getInstance ();

public:

	ClientObjectTerrainModificationNotification ();
	virtual ~ClientObjectTerrainModificationNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

	virtual bool positionChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;
	virtual void rotationChanged (Object& object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& oldPosition) const;

private:

	void updateTerrain (Object& object, bool updateOldExtent, bool updateNewExtent) const;

private:

	ClientObjectTerrainModificationNotification (const ClientObjectTerrainModificationNotification&);
	ClientObjectTerrainModificationNotification& operator= (const ClientObjectTerrainModificationNotification&);

private:

	static ProceduralTerrainAppearance* ms_terrainAppearance;
	static ClientObjectTerrainModificationNotification ms_instance;
};

//===================================================================

#endif
