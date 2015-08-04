//======================================================================
//
// CuiMinigameManager.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMinigameManager_H
#define INCLUDED_CuiMinigameManager_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

class ValueDictionary;

class CuiMinigameManager
{
public:

	static std::string const keyGame;
	static std::string const keyScore;
	static std::string const keyTableOid;
	static std::string const keyPlayerOid;

	static void              install ();
	static void              remove  ();


	static bool startMinigame(ValueDictionary const & data);
	static bool closeMinigame(ValueDictionary const & data);

	static bool submitMinigameResult(ValueDictionary const & data);

	static void registerMediatorForGame(std::string const & gameName, std::string const & mediatorName);
	static bool unregisterMediatorForGame(std::string const & gameName);

	static void addStandardScoreDataElements(ValueDictionary & data, std::string const & gameName, NetworkId const & table, int score);


	static bool getStringFromData(ValueDictionary const & data, std::string const & key, std::string & returnString);
	static bool getNetworkIdFromData(ValueDictionary const & data, std::string const & key, NetworkId & returnId);
	static bool getIntFromData(ValueDictionary const & data, std::string const & key, int & returnInt);

private:

	static bool FindMediatorForGame(std::string const & gameName, std::string & returnMediatorName);

};


class CuiMinigameBase : public CuiMediator
{
public:
	explicit CuiMinigameBase (const char * const mediatorDebugName, UIPage & newPage)
		: CuiMediator(mediatorDebugName, newPage) {};

	virtual bool readData(ValueDictionary const & data);
};

//======================================================================

#endif
