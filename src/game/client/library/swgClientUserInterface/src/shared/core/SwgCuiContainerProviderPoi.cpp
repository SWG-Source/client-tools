//======================================================================
//
// SwgCuiContainerProviderPoi.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiContainerProviderPoi.h"

#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/Game.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ClientWaypointObject.h"

#include "UIMessage.h"

#include <algorithm>

//======================================================================

SwgCuiContainerProviderPoi::SwgCuiContainerProviderPoi () :
SwgCuiContainerProvider (),
m_objects               (new ObjectWatcherVector),
m_sceneId()
{
}

//----------------------------------------------------------------------

SwgCuiContainerProviderPoi::~SwgCuiContainerProviderPoi ()
{
	delete m_objects;
	m_objects = 0;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderPoi::getObjectVector        (ObjectWatcherVector & owv)
{
	if (m_sceneId != Game::getSceneId())
	{
		setContentDirty(true);
	}

	if (isContentDirty ())
		updateObjectVector ();

	owv = *m_objects;
}

//----------------------------------------------------------------------

bool SwgCuiContainerProviderPoi::populateFromData ()
{
	bool changed = false;
	const std::vector<ClientWaypointObject*> & pois = CuiPoiManager::getData();

	m_objects->clear();

	const Filter * const filter = getFilter ();

	for (std::vector<ClientWaypointObject*>::const_iterator it = pois.begin (); it != pois.end (); ++it)
	{
		ClientWaypointObject* const wp = *it;
		if(wp)
		{
			if(filter)
			{
				if(!filter->showObject (*wp))
					continue;
			}

			m_objects->push_back (ObjectWatcher (wp));
			changed = true;
		}
	}

	return changed;
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderPoi::updateObjectVector ()
{
	populateFromData ();

	m_sceneId = Game::getSceneId();
	setContentDirty (false);
}

//----------------------------------------------------------------------

void SwgCuiContainerProviderPoi::setObjectSorting (const IntVector & iv)
{
	if (iv.size () != m_objects->size ())
	{
		WARNING (true, ("bad vector sizes"));
		setContentDirty (true);
		return;
	}

	ObjectWatcherVector newObjects;
	const int size = m_objects->size ();
	newObjects.reserve (size);

	int which = 0;

	for (IntVector::const_iterator it = iv.begin (); it != iv.end (); ++it, ++which)
	{
		const int index = *it;

		if (index < 0 || index >= size)
		{
			DEBUG_FATAL (true, ("bad index %d for sort at position %d", index, which));
			return;
		}
		ClientObject * const obj = (*m_objects) [index];
		DEBUG_WARNING (obj == NULL, ("setObjectSorting(): NULL object found, unexpected"));
		newObjects.push_back (ObjectWatcher (obj));
	}

	*m_objects = newObjects;

	setContentDirty (true);
}

//======================================================================
