//===================================================================
//
// ClientNoBuildNotification.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientNoBuildNotification.h"

#include "clientGame/GroundScene.h"
#include "clientGame/Game.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientObject/GameCamera.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"

//===================================================================

namespace ClientNoBuildNotificationNamespace
{
	ObjectList ms_objectList;

#ifdef _DEBUG

	bool ms_renderNoBuildRadii;

	void renderNoBuildRadii ()
	{
		GroundScene const * const groundScene = dynamic_cast<GroundScene const *> (Game::getConstScene ());
		if (groundScene)
		{
			Camera const * const camera = groundScene->getCurrentCamera ();
			if (camera)
			{
				for (int i = 0; i < ms_objectList.getNumberOfObjects (); ++i)
				{
					Object const * const object = ms_objectList.getObject (i);
					if (object)
					{
						const SharedObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedObjectTemplate*> (object->getObjectTemplate ());
						if (!sharedObjectTemplate)
							continue;

						const float noBuildRadius = sharedObjectTemplate->getNoBuildRadius ();

						CircleDebugPrimitive* const debugPrimitive = new CircleDebugPrimitive (CircleDebugPrimitive::S_none, object->getTransform_o2w (), Vector::zero, noBuildRadius, 20);
						debugPrimitive->setColor (VectorArgb::solidCyan);
						camera->addDebugPrimitive (debugPrimitive);
					}
				}
			}
		}
	}
#endif

}

using namespace ClientNoBuildNotificationNamespace;

//===================================================================
// STATIC PUBLIC ClientNoBuildNotification
//===================================================================

ClientNoBuildNotification ClientNoBuildNotification::ms_instance;

//-------------------------------------------------------------------

void ClientNoBuildNotification::install ()
{
	InstallTimer const installTimer("ClientNoBuildNotification::install");

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_renderNoBuildRadii, "ClientGame", "renderNoBuildRadii", renderNoBuildRadii);
#endif
}

//-------------------------------------------------------------------

ClientNoBuildNotification& ClientNoBuildNotification::getInstance ()
{
	return ms_instance;
}

//===================================================================
// PUBLIC ClientNoBuildNotification
//===================================================================

ClientNoBuildNotification::ClientNoBuildNotification () :
	NoBuildNotification ()
{
}

//-------------------------------------------------------------------

ClientNoBuildNotification::~ClientNoBuildNotification ()
{
}

//-------------------------------------------------------------------

void ClientNoBuildNotification::addToWorld (Object& object) const
{
	const SharedObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedObjectTemplate*> (object.getObjectTemplate ());
	if (!sharedObjectTemplate)
	{
		DEBUG_WARNING (true, ("ClientNoBuildNotification::addToWorld - [%s] does not have a shared object template", object.getObjectTemplate ()->getName ()));
		return;
	}

	const float noBuildRadius = sharedObjectTemplate->getNoBuildRadius ();
	NoBuildNotification::addToWorld (object, noBuildRadius);
	ms_objectList.addObject (&object);
}

//-------------------------------------------------------------------

void ClientNoBuildNotification::removeFromWorld (Object& object) const
{
	NoBuildNotification::removeFromWorld (object);
	ms_objectList.removeObject (&object);
}

//===================================================================

