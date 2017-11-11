// ======================================================================
//
// ClientCollisionProperty.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientCollisionProperty_H
#define INCLUDED_ClientCollisionProperty_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedCollision/CollisionProperty.h"

// ======================================================================

class ClientObject;
class SharedObjectTemplate;

// ======================================================================

class ClientCollisionProperty : public CollisionProperty
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	ClientCollisionProperty( ClientObject & owner );
	ClientCollisionProperty( ClientObject & owner, SharedObjectTemplate const * objTemplate );

	virtual bool canCollideWith ( CollisionProperty const * otherCollision ) const;

private:
	
	ClientCollisionProperty();
	ClientCollisionProperty( ClientCollisionProperty const & copy );
	ClientCollisionProperty & operator = ( ClientCollisionProperty const & copy );
};

// ======================================================================

#endif // INCLUDED_ClientCollisionProperty_H

