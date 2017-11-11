//======================================================================
//
// ShipObjectDestruction.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipObjectDestruction_H
#define INCLUDED_ShipObjectDestruction_H

//======================================================================

#include "clientAudio/SoundId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedObject/Object.h"

#include <vector>

class ClientDataFile;
class ClientEffectTemplate;
class DestructionSequence;
class DynamicDebrisObject;
class ShipObject;

//----------------------------------------------------------------------

class ShipObjectDestruction	: public Object
{
public:

	static void install();

public:

	ShipObjectDestruction(ShipObject & ship, float severity);
	virtual ~ShipObjectDestruction();

	typedef Watcher<ShipObject> ShipObjectWatcher;
	typedef Watcher<Object> ObjectWatcher;
	typedef Watcher<DynamicDebrisObject> DynamicDebrisObjectWatcher;

	typedef stdvector<SoundId>::fwd SoundIdVector;

	virtual float alter(float elapsedTime);

	class ObjectData;

	typedef stdvector<ObjectData *>::fwd ObjectDataVector;
	typedef stdvector<DynamicDebrisObjectWatcher>::fwd DynamicDebrisObjectWatcherVector;

	ClientEffectTemplate const * findRandomExplosion(float debrisSize, float & scaleToUse) const;
	DestructionSequence const * getDestructionSequence() const;

	float getOriginalShipRadius() const;

private:

	ShipObjectDestruction();
	ShipObjectDestruction(ShipObjectDestruction const & rhs);
	ShipObjectDestruction & operator=(ShipObjectDestruction const & rhs);

private:

	void pushObjectsAwayFrom(Object const & objectToSplit);

private:

	ShipObjectWatcher m_ship;
	ObjectDataVector m_objectDataVector;

	float m_objectSizeSplitThreshold;

	SoundIdVector m_soundIdVector;
	DynamicDebrisObjectWatcherVector m_debrisVector;

	float m_timeUntilFinale;
	float m_originalShipRadius;

	Vector m_originalShipCenter;

	float m_destructionRollRate;
	float m_destructionPitchRate;

	ClientDataFile const * m_clientDataFile;
};

//======================================================================

#endif
