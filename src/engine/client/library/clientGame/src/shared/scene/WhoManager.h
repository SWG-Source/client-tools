// ============================================================================
// 
// WhoManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_WhoManager_H
#define INCLUDED_WhoManager_H

class MatchMakingCharacterPreferenceId;
class MatchMakingId;
class UITableModelDefault;
struct MatchMakingCharacterResult;

//-----------------------------------------------------------------------------
class WhoManager
{
public:
	typedef stdvector<Unicode::String>::fwd UnicodeStringList;

	static void buildResultStringList(MatchMakingCharacterResult const &matchMakingCharacterResultList, UnicodeStringList &unicodeStringList);
	static void buildResultTable(MatchMakingCharacterResult const &matchMakingCharacterResultList, UITableModelDefault * const tableModel);
	static void requestWhoMatch(MatchMakingCharacterPreferenceId const &id);
	static void updateWhoId(MatchMakingId const &matchMakingId);

private:

	// Disable

	WhoManager();
	~WhoManager();
	WhoManager(WhoManager const &);
	WhoManager &operator =(WhoManager const &);
};

// ============================================================================

#endif // INCLUDED_WhoManager_H
