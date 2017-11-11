//======================================================================
//
// CuiDamageManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDamageManager.h"

#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "clientObject/TimerObject.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Container.h"
#include "sharedObject/SimpleDynamics.h"
#include <algorithm>
#include <cstdio>

//======================================================================

namespace CuiDamageManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiDamageManager::Messages::ShipDamage::Payload&, CuiDamageManager::Messages::ShipDamage> 
			shipDamaged;
	}

	//----------------------------------------------------------------------

	class DeferredDamage
	{
	public:

		DeferredDamage (Object &object, float deferredTime, const int deltaDamage[3]);

		bool  update (float deltaTime);

	private:
	
		// Disabled.
		DeferredDamage ();

	private:

		Watcher<Object>  m_object;
		float            m_timeRemaining;
		int              m_accumulatedDamage[3];

	};

	//----------------------------------------------------------------------

	void displayDamage (Object & obj, int damage, const VectorArgb & color)
	{
		//-- Get the client object.
		ClientObject *const clientObject = safe_cast<ClientObject *> (&obj);
		if (!clientObject)
		{
			DEBUG_WARNING (true, ("CuiDamangeManager::displayDamage: can't display damage because object is not a ClientObject."));
			return;
		}

		//-- Construct the damage string.
		char buffer[32];
		_snprintf (buffer, sizeof (buffer), "-%d", damage);

		//-- Retrieve remaining fly text parameters.
		const float fadeTime     = 3.0f;
		
		const int heavyText = (&obj == Game::getPlayer ()) ? CuiTextManagerTextEnqueueInfo::TW_starwars : CuiTextManagerTextEnqueueInfo::TW_starwars;
		clientObject->addFlyText (Unicode::narrowToWide (buffer), fadeTime, color, 1.0f, heavyText);
	}

	//----------------------------------------------------------------------

	typedef std::vector<DeferredDamage*>  DeferredDamageVector;

	//----------------------------------------------------------------------

	bool                   s_installed = false;

	DeferredDamageVector * s_damageVector;
}

using namespace CuiDamageManagerNamespace;

// =====================================================================

CuiDamageManagerNamespace::DeferredDamage::DeferredDamage (Object &object, float deferredTime, const int deltaDamage[3]) :
	m_object (&object),
	m_timeRemaining (deferredTime)
{
	for (int i = 0; i < 3; ++i)
		m_accumulatedDamage [i] = deltaDamage [i];
}

// ----------------------------------------------------------------------

bool CuiDamageManagerNamespace::DeferredDamage::update (float deltaTime)
{
	//-- Check if deferred waiting period has expired.
	m_timeRemaining    -= deltaTime;
	const bool expired  = (m_timeRemaining <= 0.0f);

	if (expired)
	{
		Object *object = m_object.getPointer ();
		if (!object)
		{
			DEBUG_REPORT_LOG(true, ("CuiDamageManager: ignored deferred damage display because creature was deleted.\n"));
			return false;
		}

		VectorArgb color;
		CuiDamageManager::findHamColor (color, m_accumulatedDamage [0], m_accumulatedDamage [1], m_accumulatedDamage [2]);

		int totalDamage = 0;
		for (int i = 0; i < 3; ++i)
			totalDamage += m_accumulatedDamage [i];
	
		//-- Render the damage.
		displayDamage (*object, totalDamage, color);
	}

	//-- Return true (keep this instance) when time has not yet expired.
	return !expired;
}

//----------------------------------------------------------------------

void CuiDamageManager::findHamColor (VectorArgb & color, int h, int a, int m)
{	
	int theMaxIndex = 0;
	int theMaxValue = 0;

	if (h >= a && h >= m)
	{
		theMaxValue = h;
		theMaxIndex = 0;
		color = VectorArgb::solidRed;
	}
	else if (a >= h && a >= m)
	{
		theMaxValue = a;
		theMaxIndex = 1;
		color = VectorArgb::solidGreen;
	}
	else
	{
		theMaxValue = m;
		theMaxIndex = 2;
		color = VectorArgb::solidBlue;
	}
	
	if (theMaxIndex != 0 && h)
		color.r += static_cast<float>(h) / static_cast<float>(theMaxValue);
	if (theMaxIndex != 1 && a)
		color.g += static_cast<float>(a) / static_cast<float>(theMaxValue);
	if (theMaxIndex != 2 && m)
		color.b += static_cast<float>(m) / static_cast<float>(theMaxValue);
}

// =====================================================================


void CuiDamageManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));

	s_damageVector = new DeferredDamageVector;

	s_installed = true;
}

//----------------------------------------------------------------------

void CuiDamageManager::remove  ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	
	std::for_each (s_damageVector->begin (), s_damageVector->end (), PointerDeleter ());
	delete s_damageVector;

	s_installed = false;
}

//----------------------------------------------------------------------

void CuiDamageManager::update  (float deltaTime)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	NOT_NULL (s_damageVector);

	if (!Game::getPlayer ())
	{
		if (!s_damageVector->empty ())
			s_damageVector->clear ();
		return;
	}

	//-- Loop through all the DeferredDamage instances and update.
	//   This will cause expired items to render.
	for (DeferredDamageVector::iterator it = s_damageVector->begin (); it != s_damageVector->end ();)
	{
		DeferredDamage *const damage = *it;
		NOT_NULL (damage);

		const bool keepIt = damage->update (deltaTime);
		if (!keepIt)
		{
			// Delete this damage item. Erase and increment loop.
			delete damage;
			it = s_damageVector->erase (it);
		}
		else
		{
			// Increment loop.
			++it;
		}
	}
}

//----------------------------------------------------------------------

void CuiDamageManager::handleShipDamage (ShipDamageMessage const & shipDamageMsg)
{
	Transceivers::shipDamaged.emitMessage(shipDamageMsg);
}


//======================================================================
