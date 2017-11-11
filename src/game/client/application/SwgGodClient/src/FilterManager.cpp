// ======================================================================
//
// FilterManager.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FilterManager.h"

#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedObject/Object.h"

#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"

// ======================================================================

FilterManager::NetworkIdFilter  FilterManager::ms_networkIdFilter;
FilterManager::RadiusFilter     FilterManager::ms_radiusFilter;
bool                            FilterManager::ms_installed;

// ======================================================================

/**
 * Validate that an object is between our radius thresholds.  Either, none, or both of the thresholds may be activated.
 */
bool FilterManager::RadiusFilter::validate(const Object& obj)
{
	if(!m_isOn)
		return true;

	GroundScene* gs         = NON_NULL(dynamic_cast<GroundScene*>(Game::getScene()));
	const Object* player    = gs->getPlayer();
	const Vector& playerLoc = player->getPosition_w();
	const Vector& objLoc    = obj.getPosition_w();

	real distance = (playerLoc - objLoc).magnitude();

	if(m_minThresholdUsed)
		if (distance < m_minThreshold)
			return false;

	if(m_maxThresholdUsed)
		if (distance > m_maxThreshold)
			return false;

	return true;
}

// ======================================================================

/**
 * Validate that the object is within the ObjectId thresholds.  Either, none, or both of the thresholds may be activated.
 */
bool FilterManager::NetworkIdFilter::validate(const Object& obj)
{
	if(!m_isOn)
		return true;

	const NetworkId& objNetworkId = obj.getNetworkId();

	if(m_minNetworkIdUsed)
		if (objNetworkId.getValue() < m_minNetworkId)
			return false;

	if(m_maxNetworkIdUsed)
		if (objNetworkId.getValue() > m_maxNetworkId)
			return false;

	return true;
}

// ======================================================================

/**
 * Validate that an object passes all active filters.
 */
bool FilterManager::validate(const Object& obj)
{
	DEBUG_FATAL(!ms_installed, ("FilterManager not installed"));
	//never filter out the avatar, check for that case
	GroundScene* gs         = NON_NULL(dynamic_cast<GroundScene*>(Game::getScene()));
	const Object* player    = gs->getPlayer();
	if(obj.getNetworkId() == player->getNetworkId())
		return true;

	if(!ms_radiusFilter.validate(obj))
		return false;

	if(!ms_networkIdFilter.validate(obj))
		return false;

	return true;
}

// ======================================================================

