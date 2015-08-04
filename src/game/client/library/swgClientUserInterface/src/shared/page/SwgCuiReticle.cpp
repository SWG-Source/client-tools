//======================================================================
//
// SwgCuiReticle.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiReticle.h"

#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/WeaponObject.h"
#include "clientGraphics/Camera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/ContainedByProperty.h"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"
#include <cstdio>

//======================================================================

namespace
{

	enum Mods
	{
		STANDING,
		SNEAKING,
		WALKING,
		RUNNING,
		KNEELING,
		CROUCH_WALKING,
		PRONE,
		CRAWLING,
		FLYING,
		NUM_MODS
	};
	
	// attack roll is to beat a 75 on a 1d100 roll.
	// thus a fully unmodified roll has a 25% chance of success, 
	// a roll modified to -25 has a 0% chance,
	// and a roll modified to +75 has a 100% chance
	
	namespace Ranged
	{
		namespace Attacker
		{
			float mods [NUM_MODS] =
			{
				0.00f,     // standing
				-0.06f,    // sneaking
				-0.20f,    // walking
				-0.30f,    // running
				0.15f,     // kneeling
				0.00f,     // crouch walking
				0.30f,     // prone
				-0.10f,    // crawling
				-0.10f     // flying
			};
		}
		
		namespace Defender
		{
			const float mods [NUM_MODS] = 
			{
				0.10f,     // standing
				-0.05f,    // sneaking
				-0.05f,    // walking
				-0.15f,    // running
				0.05f,     // kneeling
				0.00f,     // crouch walking
				-0.15f,    // prone
				-0.05f,    // crawling
				0.10f      // flying
			};
		}
	}

	namespace Melee
	{
		namespace Attacker
		{
			float mods [NUM_MODS] =
			{
				0.00f,     // standing
				-0.20f,    // sneaking
				-0.5f,     // walking
				-0.15f,    // running
				0.10f,     // kneeling
				0.00f,     // crouch walking
				-0.30f,    // prone
				-0.40f,    // crawling
				-0.10f     // flying
			};
		}
		
		namespace Defender
		{
			const float mods [NUM_MODS] = 
			{
				0.00f,     // standing
				0.05f,     // sneaking
				0.05f,     // walking
				0.10f,     // running
				0.10f,     // kneeling
				0.00f,     // crouch walking
				0.30f,     // prone
				0.45f,     // crawling
				0.40f      // flying
			};
		}
	}
	//----------------------------------------------------------------------

	float getAccuracyForPosture (const CreatureObject & creature, const float * mods, bool isRunning, bool isWalking)
	{
		switch (creature.getVisualPosture ())
		{
		case Postures::Upright:
		case Postures::KnockedDown:
		case Postures::Blocking:
		case Postures::RidingCreature:
		case Postures::Flying:
			
			{
				float flyingmod = 0.0f;
				if (creature.getVisualPosture () == Postures::Flying)
					flyingmod = mods [FLYING];
				
				if (isRunning)
					return mods [RUNNING] + flyingmod;
				else if (isWalking)
					return mods [WALKING] + flyingmod;
				else
					return mods [STANDING] + flyingmod;
			}

		case Postures::Crouched:
		case Postures::Sitting:
		case Postures::DrivingVehicle:
		case Postures::Climbing:
			
			if (isWalking)
				return mods [CROUCH_WALKING];
			else
				return mods [KNEELING];

		case Postures::Prone:
		case Postures::LyingDown:

			if (isWalking)
				return mods [CRAWLING];
			else
				return mods [PRONE];

		case Postures::Sneaking:
			return mods [SNEAKING];

		default:
			break;
		}

		return 0.0f;
	}

	//----------------------------------------------------------------------

	float getAccuracyForWeapon (const WeaponObject & weapon)
	{
		return static_cast<float>(weapon.getAccuracy()) / 100.0f;
	}

	//----------------------------------------------------------------------

	bool computeAccuracy (const CreatureObject & player, const TangibleObject & defender, float range, float & accuracy)
	{
		range = std::max (0.0f, range);

		const CreatureController * const playerController = NON_NULL (safe_cast<const CreatureController *>(player.getController ()));

		if (!playerController)
			return 0.0f;

		accuracy = 0.35f;

		if (player.getState (States::Aiming))
			accuracy += 0.25f;

		bool isRanged = false;

		const WeaponObject * const weapon = player.getCurrentWeapon ();
		if (weapon)
		{
			const int weapon_got = weapon->getGameObjectType ();

			isRanged = (weapon_got == SharedObjectTemplate::GOT_weapon_ranged_misc ||
				weapon_got == SharedObjectTemplate::GOT_weapon_ranged_pistol || 
				weapon_got == SharedObjectTemplate::GOT_weapon_ranged_rifle || 
				weapon_got == SharedObjectTemplate::GOT_weapon_ranged_carbine);

			const float weaponAccuracy = getAccuracyForWeapon (*weapon);

			if (weaponAccuracy <= -10000.0f)
				return false;

			accuracy += weaponAccuracy;
		}

		const float playerCurrentSpeed = playerController->getCurrentSpeed ();

		const bool isPlayerRunning = playerCurrentSpeed > player.getMaximumWalkSpeed ();
		const bool isPlayerWalking = playerCurrentSpeed > 0.0f;

		if (isRanged)
			accuracy += getAccuracyForPosture (player, Ranged::Attacker::mods, isPlayerRunning, isPlayerWalking);
		else
			accuracy += getAccuracyForPosture (player, Melee::Attacker::mods, isPlayerRunning, isPlayerWalking);

		const CreatureObject * const creature = dynamic_cast<const CreatureObject *>(&defender);

		if (creature)
		{
			const CreatureController * const creatureController = safe_cast<const CreatureController *>(creature->getController ());

			if (creatureController)
			{
				const float defenderCurrentSpeed = creatureController->getCurrentSpeed ();

				const bool isDefenderRunning = defenderCurrentSpeed > creature->getMaximumWalkSpeed ();
				const bool isDefenderWalking = defenderCurrentSpeed > 0.0f;

				if (isRanged)
					accuracy += getAccuracyForPosture (*creature, Ranged::Defender::mods, isDefenderRunning, isDefenderWalking);
				else
					accuracy += getAccuracyForPosture (*creature, Melee::Defender::mods, isDefenderRunning, isDefenderWalking);
			}
		}

		accuracy = std::max (0.0f, accuracy);
		accuracy = std::min (1.0f, accuracy);

		return true;
	}
}

//----------------------------------------------------------------------

SwgCuiReticle::SwgCuiReticle  (UIPage & page) :
CuiMediator ("SwgCuiReticle", page),
m_pageOut   (0),
m_pageIn    (0),
m_inDiff    (),
m_pageDiff  (),
m_text      (0)
{
	getCodeDataObject (TUIPage, m_pageOut, "out");
	getCodeDataObject (TUIPage, m_pageIn,  "in");
	getCodeDataObject (TUIText, m_text,    "text");

	m_inDiff = m_pageIn->GetMaximumSize () - m_pageIn->GetMinimumSize ();
	m_pageDiff = page.GetMaximumSize () - page.GetMinimumSize ();
}

//----------------------------------------------------------------------

SwgCuiReticle::~SwgCuiReticle ()
{
	m_pageOut = 0;
	m_pageIn  = 0;
	m_text    = 0;
}

//----------------------------------------------------------------------

void SwgCuiReticle::performActivate   ()
{
}

//----------------------------------------------------------------------

void SwgCuiReticle::performDeactivate ()
{
	m_pageOut->SetEnabled (false);
	m_pageIn->SetEnabled  (false);
}
 
//----------------------------------------------------------------------

float SwgCuiReticle::getRotation () const
{
	if (m_pageOut->IsVisible ())
		return m_pageOut->GetRotation ();
	else
		return m_pageIn->GetRotation ();
}

//----------------------------------------------------------------------

void SwgCuiReticle::setRotation (float f)
{
	if (m_pageOut->IsVisible ())
		m_pageOut->SetRotation (f);
	else
		m_pageIn->SetRotation (f);
}

//----------------------------------------------------------------------

bool SwgCuiReticle::computeTargetAccuracy (const CreatureObject & player, const TangibleObject & defender, float range, float & accuracy)
{
	return computeAccuracy (player, defender, range, accuracy);
}

//======================================================================
