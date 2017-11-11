// ======================================================================
//
// ClientGameAppearanceEvents.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientGameAppearanceEvents.h"

#include "sharedDebug/InstallTimer.h"

// ======================================================================

namespace ClientGameAppearanceEventsNamespace
{
	bool           s_installed;

	LabelHash::Id  s_onEquippedEventId;
	LabelHash::Id  s_onUnequippedEventId;
	LabelHash::Id  s_onEquippedEnteredCombatEventId;
	LabelHash::Id  s_onEquippedExitedCombatEventId;
	LabelHash::Id  s_onInitializeCopyEquippedInCombatStateEventId;
}

using namespace ClientGameAppearanceEventsNamespace;

// ======================================================================

void ClientGameAppearanceEvents::install()
{
	InstallTimer const installTimer("ClientGameAppearanceEvents::install");

	s_onEquippedEventId                            = LABEL_HASH(CGAE_LABEL_HASH_DOMAIN, OnEquipped, 0x6a99519f);
	s_onUnequippedEventId                          = LABEL_HASH(CGAE_LABEL_HASH_DOMAIN, OnUnequipped, 0x4ce8be8f);
	s_onEquippedEnteredCombatEventId               = LABEL_HASH(CGAE_LABEL_HASH_DOMAIN, OnEquippedEnteredCombat, 0xbaf0c56a);
	s_onEquippedExitedCombatEventId                = LABEL_HASH(CGAE_LABEL_HASH_DOMAIN, OnEquippedExitedCombat, 0xea2a5691);
	s_onInitializeCopyEquippedInCombatStateEventId = LABEL_HASH(CGAE_LABEL_HASH_DOMAIN, OnInitializeCopyEquippedInCombatStateEventId, 0x3f1d01cb);

	s_installed = true;
}

// ----------------------------------------------------------------------

LabelHash::Id ClientGameAppearanceEvents::getOnEquippedEventId()
{
	DEBUG_FATAL(!s_installed, ("ClientGameAppearanceEvents not installed."));
	return s_onEquippedEventId;
}

// ----------------------------------------------------------------------

LabelHash::Id ClientGameAppearanceEvents::getOnUnequippedEventId()
{
	DEBUG_FATAL(!s_installed, ("ClientGameAppearanceEvents not installed."));
	return s_onUnequippedEventId;
}

// ----------------------------------------------------------------------

LabelHash::Id ClientGameAppearanceEvents::getOnEquippedEnteredCombatEventId()
{
	DEBUG_FATAL(!s_installed, ("ClientGameAppearanceEvents not installed."));
	return s_onEquippedEnteredCombatEventId;
}

// ----------------------------------------------------------------------

LabelHash::Id ClientGameAppearanceEvents::getOnEquippedExitedCombatEventId()
{
	DEBUG_FATAL(!s_installed, ("ClientGameAppearanceEvents not installed."));
	return s_onEquippedExitedCombatEventId;
}

// ----------------------------------------------------------------------

LabelHash::Id ClientGameAppearanceEvents::getOnInitializeCopyEquippedInCombatStateEventId()
{
	DEBUG_FATAL(!s_installed, ("ClientGameAppearanceEvents not installed."));
	return s_onInitializeCopyEquippedInCombatStateEventId;
}

// ======================================================================
