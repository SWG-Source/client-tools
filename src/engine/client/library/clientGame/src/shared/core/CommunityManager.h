// ============================================================================
// 
// CommunityManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_CommunityManager_H
#define INCLUDED_CommunityManager_H

//-----------------------------------------------------------------------------
class CommunityManager
{
public:

	class FriendData;

	typedef stdmap<Unicode::String, FriendData>::fwd      FriendList;
	typedef stdmap<Unicode::String, Unicode::String>::fwd IgnoreList;
	typedef stdset<Unicode::String>::fwd                  UnicodeStringSet;

	struct Messages
	{
		struct FriendListChanged
		{
			typedef bool Status;
		};

		struct ShowFriendListMessage
		{
			typedef bool Status;
		};

		struct FriendOnlineStatusChanged
		{
			typedef Unicode::String Name;
		};

		struct IgnoreListChanged
		{
			typedef bool Status;
		};

		struct ShowIgnoreListMessage
		{
			typedef bool Status;
		};

		struct RequestVerifyPlayerNameResponse
		{
			typedef std::pair<bool, Unicode::String> Response;
		};
	};

public:

	static void              install();

	static void              gameStart();
	static void              chatSystemConnected();
	static void              alter(float const deltaTime);

	static void              requestVerifyPlayerName(Unicode::String const &playerName);

	// Friend list

	static void              setFriendList(UnicodeStringSet const &friendList);
	static void              addFriend(Unicode::String const &name, Unicode::String const &group, Unicode::String const &comment, bool const notifyOnlineStatus);
	static void              removeFriend(Unicode::String const &name);
	static bool              isFriend(Unicode::String const &name);
	static void              showFriendList();
	static FriendList const &getFriendList();
	static void              setFriendOnlineStatus(Unicode::String const &name, bool const online);
	static bool              isFriendOnline(Unicode::String const &name);
	static bool              isNotifyOnlineStatus(Unicode::String const &name);
	static bool              getFriendComment(Unicode::String const &name, Unicode::String &comment);
	static bool              getFriendGroup(Unicode::String const &name, Unicode::String &group);
	static void              setHideOfflineFriends(bool const hide);
	static bool              isHideOfflineFriends();

	// Ignore list

	static void              setIgnoreList(UnicodeStringSet const &ignoreList);
	static void              addIgnore(Unicode::String const &name);
	static void              removeIgnore(Unicode::String const &name);
	static bool              isIgnored(Unicode::String const &name);
	static void              showIgnoreList();
	static IgnoreList const &getIgnoreList();

	// Do not call the following directly. They are called from CuiChatRoomManagerListener.

	static void              requestVerifyPlayerNameResponse(bool const valid, Unicode::String const &playerName);

private:

	static void remove();

	// Disable

	CommunityManager();
	~CommunityManager();
	CommunityManager(CommunityManager const &);
	CommunityManager &operator =(CommunityManager const &);
};

// ============================================================================

#endif // INCLUDED_CommunityManager_H
