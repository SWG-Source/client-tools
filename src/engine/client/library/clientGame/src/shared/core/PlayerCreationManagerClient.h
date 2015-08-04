//======================================================================
//
// PlayerCreationManagerClient.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerCreationManagerClient_H
#define INCLUDED_PlayerCreationManagerClient_H

#include "sharedGame/PlayerCreationManager.h"

class CreatureObject;

//======================================================================

class PlayerCreationManagerClient : public PlayerCreationManager
{
public:
	static void                   install ();
	static void                   remove ();

	static bool                   setupPlayer(CreatureObject & obj, const std::string & profession);

	static std::string            findDefaultProfession (const std::string & category);

	static void                   getCategoryProfessions (const std::string & categoryName, StringVector & sv);

	static void                   strip (CreatureObject & obj);
};

//======================================================================

#endif
