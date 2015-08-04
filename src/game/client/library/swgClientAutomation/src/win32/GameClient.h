// ============================================================================
//
// GameClient.h
// copyright (c) 2005 Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_GameClient_H
#define INCLUDED_GameClient_H

#include "Automation/IGameClient.h"

class GameClient : public Automation::IGameClient
{
public:
	virtual const wchar_t *getPlayerName() const;
};

#endif 