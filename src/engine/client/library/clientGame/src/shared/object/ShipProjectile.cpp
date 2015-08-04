// ======================================================================
//
// ShipProjectile.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ShipProjectile.h"

#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientGraphics/Camera.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedGame/ShipComponentFlags.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Volume.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"

// ======================================================================

//lint -e655
//lint -e641

namespace ShipProjectileNamespace
{
	float const ms_range = 128.f;
	
	char const * const cms_boltFlyBySound = "sound/cbt_bolt_flyby.snd";
	
	MemoryBlockManager * ms_memoryBlockManager;
	
	float const cms_flybySoundRange = 50.0f;
	float const cms_flybySoundRangeSquared = cms_flybySoundRange * cms_flybySoundRange;
	
	int ms_numberOfInstances;
		
	//----------------------------------------------------------------------
}

using namespace ShipProjectileNamespace;

// ======================================================================

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ShipProjectile, true, 0, 0, 0);

// ======================================================================

void ShipProjectile::install()
{
	InstallTimer const installTimer("ShipProjectile::install");
	
	installMemoryBlockManager();
		
	ShipProjectileMissHitData::install();

	ExitChain::add(ShipProjectile::remove, "ShipProjectileNamespace::remove");
}

// ----------------------------------------------------------------------

int ShipProjectile::getNumberOfInstances()
{
	return ms_numberOfInstances;
}

// ----------------------------------------------------------------------

void ShipProjectile::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("ShipProjectile is not installed"));
	
	removeMemoryBlockManager();
}

// ======================================================================

ShipProjectile::ShipProjectile(AppearanceTemplate const * const appearanceTemplate, MissHitData const & missHitData) : 
Projectile(appearanceTemplate, 0, 0),
m_missHitData(missHitData),
m_travelDistance(ms_range),
m_ownerShip(),
m_canPlayFlyby(true),
m_weaponIndex(-1)
{
	
	m_missHitData.fetch();
	
	++ms_numberOfInstances;
	
	if (getAppearance())
		IGNORE_RETURN(getAppearance()->alter(0.1f));
}

// ----------------------------------------------------------------------

ShipProjectile::~ShipProjectile()
{
	--ms_numberOfInstances;
	
	m_missHitData.release();
}

// ----------------------------------------------------------------------

float ShipProjectile::alter(float const elapsedTime)
{
	//-- Update the projectile and see if it died on its own
	if (Projectile::alter(elapsedTime) == AlterResult::cms_kill) //lint !e777
		return AlterResult::cms_kill;
	
	float const distanceThisFrame = getPosition_w().magnitudeBetween(m_lastPosition_w);
	Vector const endPosition_w = m_lastPosition_w + getObjectFrameK_w() * distanceThisFrame;
	
	Vector resultEndPos_w;
	if (m_missHitData.handleCollision(*this, m_ownerShip, m_weaponIndex, m_lastCellProperty, m_lastPosition_w, endPosition_w, resultEndPos_w, true))
		AlterResult::cms_kill;
	
	//if a flyby sound *could* be played for this bolt
	if(m_canPlayFlyby)
	{
		ShipObject const * const playerShip = Game::getPlayerContainingShip();
		if (playerShip)
		{
			// if the bolt is close enough to us, and we didn't fire it, play a flyby
			if (   playerShip != m_ownerShip.getPointer()
				&& getPosition_p().magnitudeBetweenSquared(playerShip->getPosition_p()) < cms_flybySoundRangeSquared)
			{
				IGNORE_RETURN(Audio::attachSound(cms_boltFlyBySound, this));
				m_canPlayFlyby = false;
			}
		}
	}
	
	//-- See if we've travelled our travel distance
	m_travelDistance -= distanceThisFrame;
	if (m_travelDistance <= 0.f)
		return AlterResult::cms_kill;
	
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void ShipProjectile::setOwnerShip(ShipObject *ownerShip)
{
	m_ownerShip = ownerShip;
}

// ----------------------------------------------------------------------

void ShipProjectile::setWeaponIndex(int weaponIndex)
{
	m_weaponIndex = weaponIndex;
}

// ----------------------------------------------------------------------

void ShipProjectile::setTravelDistance(float const travelDistance)
{
	m_travelDistance = travelDistance;
}

// ======================================================================

