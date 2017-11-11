//======================================================================
//
// ClientObjectUsabilityManager.h
// copyright (c) Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientObjectUsabilityManager_H
#define INCLUDED_ClientObjectUsabilityManager_H

//======================================================================

class CreatureObject;
class TangibleObject;

//----------------------------------------------------------------------

class ClientObjectUsabilityManager
{
public:
	// Initialize
	static void install();
	static void remove();

	// clear out internal caches
	static void reset();

	static bool isUsable(TangibleObject const * target);

	static bool canUse(CreatureObject const * user, TangibleObject const * target);

	// Can the user wear the target.  This is not a check to see if you can equip something.
	static bool canWear(CreatureObject const * user, TangibleObject const * target);

	// Is the target tangible object a wearable.
	static bool isWearable(TangibleObject const * target);
};

//======================================================================

#endif
