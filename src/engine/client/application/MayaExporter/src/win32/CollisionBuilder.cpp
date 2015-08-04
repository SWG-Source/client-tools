//===================================================================
//
// CollisionBuilder.cpp
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstMayaExporter.h"
#include "CollisionBuilder.h"

#include "FloorBuilder.h"
#include "maya/MFnDagNode.h"

#include "Messenger.h"

#include "sharedFile/Iff.h"

extern Messenger * messenger;


CollisionBuilder::CollisionBuilder( MayaHierarchy::Node const * pNode )
{
	m_pNode = pNode;
}

bool CollisionBuilder::writeCollision ( bool bPublish )
{
	if(m_pNode->getType() == MayaHierarchy::T_collision)
	{
		// This is a collision group node, export a collision file for it

		// For now, export separate floor files for all floor children
		// in the group

		int n = m_pNode->getNumberOfChildren();

		for(int i = 0; i < n; i++)
		{
			MayaHierarchy::Node const * child = m_pNode->getChild(i);

			if(child->getType() == MayaHierarchy::T_floor)
			{
				FloorBuilder B(child);

				if(!B.writeFloor(bPublish))
				{
					return false;
				}
			}
		}

		return true;
	}
	else if(m_pNode->getType() == MayaHierarchy::T_floor)
	{
		MayaHierarchy::Node const * parent = m_pNode->getParent();

		if(parent && parent->getType() == MayaHierarchy::T_collision)
		{
			// This is a floor node, but it's part of a collision group.
			// Don't export this node here, it will be handled by the group.

			return true;
		}
		else
		{
			// This is a bare floor node, no containing collision group.
			// Export the floor by itself

			FloorBuilder B(m_pNode);
            if(!B.writeFloor(bPublish))
			{
				return false;
			}

			return true;
		}
	}
	else
	{
		return false;
	}
}

