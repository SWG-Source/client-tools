// ============================================================================
// 
// MatchMakingManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_MatchMakingManager_H
#define INCLUDED_MatchMakingManager_H

#include "clientGame/PlayerCreatureController.h"
#include "sharedGame/MatchMakingId.h"

class Object;
class MatchMakingResult;

//-----------------------------------------------------------------------------
class MatchMakingManager
{
public:

	typedef std::string                                 CategoryName;
	typedef std::string                                 TypeName;
	typedef int                                         BitIndex;
	typedef stdmap<TypeName, BitIndex>::fwd             TypeNameToBitMap;
	typedef stdmap<CategoryName, TypeNameToBitMap>::fwd Categories;
	typedef stdvector<std::string>::fwd                 StringVector;
	typedef stdvector<Unicode::String>::fwd             UnicodeStringVector;

	struct Messages
	{
		struct MatchFoundResult
		{
			typedef MatchMakingResult MatchMakingResult;
		};

		struct MatchFoundResultString
		{
			typedef Unicode::String MatchMakingResultString;
		};

		struct WhoIdChanged
		{
			typedef MatchMakingId Id;
		};

		struct QuickMatchFoundResultString
		{
			typedef Unicode::String QuickMatchResultString;
		};
	};

	static void  install();
	static void  loadUserSettings();
	static void  alter(float const deltaTime);
	static void  findMatches(Object *object);
	static void  findQuickMatch(Unicode::String const &quickMatchString);
	static void  setUpdateTime(const float seconds);
	static float getUpdateTime();
	static float getSensitivity();
	static void  setSensitivity(const float percent);

	static void                 setPreferenceId(MatchMakingId const &matchMakingId);
	static MatchMakingId const &getPreferenceId();

	static bool  isSearchableByCtsSourceGalaxy();
	static bool  isDisplayLocationInSearchResults();
	static bool  isAnonymous();
	static bool  isRolePlay();
	static bool  isHelper();
	static bool  isLookingForGroup();
	static bool  isLookingForWork();
	static bool  isAwayFromKeyBoard();
	static bool  isDisplayingFactionRank();

private:

	static void remove();
	static bool isWhoBitSet(MatchMakingId::Bit const bit);

	// Disable

	MatchMakingManager();
	~MatchMakingManager();
	MatchMakingManager(MatchMakingManager const &);
	MatchMakingManager &operator =(MatchMakingManager const &);
};

// ============================================================================

#endif // INCLUDED_MatchMakingManager_H
