//
// CuiPlayerQuestManager.h
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiPlayerQuestManager_H
#define INCLUDED_CuiPlayerQuestManager_H

//======================================================================

#include "clientGame/ClientPlayerQuestObject.h"

//======================================================================

class CuiPlayerQuestManager
{
public:
	static void install();
	static void remove();

	static std::vector<NetworkId> const & getActivePlayerQuests();
};

#endif