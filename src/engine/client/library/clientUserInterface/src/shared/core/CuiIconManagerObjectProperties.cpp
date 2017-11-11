//======================================================================
//
// CuiIconManagerObjectProperties.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiIconManagerObjectProperties.h"

#include "clientGame/ResourceContainerObject.h"
#include "clientGame/TangibleObject.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedObject/VolumeContainer.h"

//======================================================================

CuiIconManagerObjectProperties::CuiIconManagerObjectProperties () :
m_damageTaken   (-1),
m_maxHitpoints  (-1),
m_condition     (-1),
m_count         (-1),
m_content       (-1),
m_contentMax    (-1)
{
}

//----------------------------------------------------------------------

CuiIconManagerObjectProperties::CuiIconManagerObjectProperties (const ClientObject & obj) :
m_damageTaken   (-1),
m_maxHitpoints  (-1),
m_condition     (-1),
m_count         (-1),
m_content       (-1),
m_contentMax    (-1)
{
	updateFromObject (obj);
}

//----------------------------------------------------------------------

void CuiIconManagerObjectProperties::updateFromObject (const ClientObject & obj)
{
	TangibleObject const * const tangible = obj.asTangibleObject ();
	if (tangible)
	{
		m_damageTaken  = tangible->getDamageTaken  ();
		m_maxHitpoints = tangible->getMaxHitPoints ();
		m_condition    = tangible->getCondition    ();
		m_count        = tangible->getCount        ();

		if (tangible->getObjectType () == SharedResourceContainerObjectTemplate::SharedResourceContainerObjectTemplate_tag)
		{
			ResourceContainerObject const * const rc = NON_NULL (safe_cast<const ResourceContainerObject *>(tangible));
			m_content    = rc->getQuantity    ();	
			m_contentMax = rc->getMaxQuantity ();
		}

		VolumeContainer const * const volumeContainer = tangible->getVolumeContainerProperty();
		if (volumeContainer)
		{
			m_content    = volumeContainer->getCurrentVolume ();
			m_contentMax = volumeContainer->getTotalVolume   ();
		}
	}
}

//----------------------------------------------------------------------


bool CuiIconManagerObjectProperties::updateAndCompareFromObject (const ClientObject & obj)
{
	const CuiIconManagerObjectProperties op (obj);
	bool retval = !(op == *this);
	*this = op;
	return retval;
}

//======================================================================
