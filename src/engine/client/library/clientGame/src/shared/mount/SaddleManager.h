// ======================================================================
//
// SaddleManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SaddleManager_H
#define INCLUDED_SaddleManager_H

#include "sharedGame/SharedSaddleManager.h"

// ======================================================================

class CrcString;
class CreatureObject;
class HardpointObject;
class Object;

// ======================================================================

class SaddleManager : public SharedSaddleManager
{
public:

	static void install(char const *logicalSaddleNameMapFile, char const *saddleAppearanceMapFile, char const *riderPoseMapFile);

	static CrcString const *getSaddleAppearanceNameForMount(CreatureObject const &mount, CrcString & clientDataFilename);
	static CrcString const *getRiderPoseNameForMountSeatIndex(CreatureObject const &mount, int seatIndex);

	static void    addDressingToMount(CreatureObject &mount);

	static Object *addRiderSaddleToMount(CreatureObject &mount);

	static Object *getSaddleObjectFromMount(Object &mount);
	static const Object *getSaddleObjectFromMount(const Object &mount);

	static bool isThereRoomForPassenger(Object const & mount);
	static int getSaddleSeatingCapacity(Object const & mount);
	static int countTotalNumberOfPassengers(Object const & mount);
	static int getRiderSeatIndex(Object const & mount, Object const & rider);
	static int findFirstOpenSeat(Object const & mount);

	static HardpointObject * createRiderHardpointObjectAndAttachToSaddle(Object & mount);

private:
	SaddleManager();
	SaddleManager(SaddleManager const & copy);
	SaddleManager & operator=(SaddleManager const & copy);
};

// ======================================================================

#endif
