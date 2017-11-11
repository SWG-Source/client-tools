// ======================================================================
//
// ClientGameAppearanceEvents.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ClientGameAppearanceEvents_H
#define INCLUDED_ClientGameAppearanceEvents_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/LabelHash.h"

// ======================================================================

#define CGAE_LABEL_HASH_DOMAIN  ClientGameAppearanceEvents

// ======================================================================

class ClientGameAppearanceEvents
{
public:

	static void install();

	static LabelHash::Id  getOnEquippedEventId();
	static LabelHash::Id  getOnUnequippedEventId();

	static LabelHash::Id  getOnEquippedEnteredCombatEventId();
	static LabelHash::Id  getOnEquippedExitedCombatEventId();

	static LabelHash::Id  getOnInitializeCopyEquippedInCombatStateEventId();
};

// ======================================================================

#endif
