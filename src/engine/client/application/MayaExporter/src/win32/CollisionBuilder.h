//===================================================================
//
// CollisionBuilder.h
// aappleby
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_CollisionBuilder_H
#define INCLUDED_CollisionBuilder_H

#include "MayaHierarchy.h"

#include <string>

//===================================================================

class CollisionBuilder
{
public:

	CollisionBuilder( MayaHierarchy::Node const * pNode );

	bool writeCollision ( bool bPublish );

	bool findFloors ( void );

	// ----------

	MayaHierarchy::Node const * m_pNode;
};

//===================================================================

#endif

