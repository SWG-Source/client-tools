// ======================================================================
//
// ClientCollisionProperty.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCollisionProperty.h"

#include "clientGame/CreatureObject.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCollisionProperty, true, 0, 0, 0);

// ======================================================================

ClientCollisionProperty::ClientCollisionProperty( ClientObject & owner )
: CollisionProperty(owner)
{
}

ClientCollisionProperty::ClientCollisionProperty( ClientObject & owner, SharedObjectTemplate const * objTemplate )
: CollisionProperty(owner,objTemplate)
{
}

// ----------------------------------------------------------------------

bool ClientCollisionProperty::canCollideWith( CollisionProperty const * otherCollision ) const
{
	if(otherCollision == NULL) return false;

	if(isMobile() && otherCollision->isMobile())
	{
		ClientObject const * const ownerClientObject = getOwner().asClientObject();
		ClientObject const * const otherOwnerClientObject = otherCollision->getOwner().asClientObject();

		CreatureObject const * const ownerA = (ownerClientObject != 0) ? ownerClientObject->asCreatureObject() : 0;
		CreatureObject const * const ownerB = (otherOwnerClientObject != 0) ? otherOwnerClientObject->asCreatureObject() : 0;

		if ((ownerA) && (ownerA->isDead() || ownerA->isIncapacitated())) 
			return false;

		if ((ownerB) && (ownerB->isDead() || ownerB->isIncapacitated())) 
			return false;
	}

	return CollisionProperty::canCollideWith(otherCollision);
}
