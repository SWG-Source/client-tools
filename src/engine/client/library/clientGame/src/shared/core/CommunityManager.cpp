// ============================================================================
//
// CommunityManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/CommunityManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CommunityManager_FriendData.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/VerifyPlayerNameMessage.h"
#include "sharedUtility/CurrentUserOptionManager.h"

#include <map>
#include <set>
#include <vector>

//-----------------------------------------------------------------------------
namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CommunityManager::Messages::FriendListChanged::Status &, CommunityManager::Messages::FriendListChanged>
			friendListChanged;

		MessageDispatch::Transceiver<const CommunityManager::Messages::ShowFriendListMessage::Status &, CommunityManager::Messages::ShowFriendListMessage>
			showFriendListMessage;

		MessageDispatch::Transceiver<const CommunityManager::Messages::FriendOnlineStatusChanged::Name &, CommunityManager::Messages::FriendOnlineStatusChanged>
			friendOnlineStatusChanged;

		MessageDispatch::Transceiver<const CommunityManager::Messages::IgnoreListChanged::Status &, CommunityManager::Messages::IgnoreListChanged>
			ignoreListChanged;

		MessageDispatch::Transceiver<const CommunityManager::Messages::ShowIgnoreListMessage::Status &, CommunityManager::Messages::ShowIgnoreListMessage>
			showIgnoreListMessage;

		MessageDispatch::Transceiver<const CommunityManager::Messages::RequestVerifyPlayerNameResponse::Response &, CommunityManager::Messages::RequestVerifyPlayerNameResponse>
			requestVerifyPlayerNameResponse;
	}
}

// ============================================================================
//
// CommunityManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace CommunityManagerNamespace
{
	bool      s_installed = false;
	bool      s_chatSystemConnected = false;
	Tag const s_tag = TAG(F,R,N,D);
	bool      s_loaded = false;
	bool      s_gameStart = false;
	bool      s_saveRequested = false;
	bool      s_hideOfflineFriends = false;

	CommunityManager::FriendList s_friendList;
	CommunityManager::FriendList s_pendingFriendList;
	CommunityManager::IgnoreList s_ignoreList;
	CommunityManager::FriendList s_queuedFriendOnlineStatus;

	void requestFriendList();
	void requestIgnoreList();
	void requestRemoveFriend(Unicode::String const &name);
	void requestAddFriend(Unicode::String const &name);
	void requestRemoveIgnore(Unicode::String const &name);
	void requestAddIgnore(Unicode::String const &name);
	void load();
	void save();
	void processOnlineStatusMessages();
	std::string getFileNameOld();
	std::string getFileName();
	bool isPlayerInfoValid();
}

using namespace CommunityManagerNamespace;

// ----------------------------------------------------------------------------
bool CommunityManagerNamespace::isPlayerInfoValid()
{
	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;

	return Game::getPlayerPath(loginId, clusterName, playerName, id);
}

// ----------------------------------------------------------------------------
std::string CommunityManagerNamespace::getFileNameOld()
{
	std::string result;
	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;

	if (Game::getPlayerPath(loginId, clusterName, playerName, id))
	{
		result = "profiles/" + loginId + "/" + clusterName + "/friend_data.iff";
	}

	return result;
}

// ----------------------------------------------------------------------------
std::string CommunityManagerNamespace::getFileName()
{
	std::string result;
	std::string loginId;
	std::string clusterName;
	Unicode::String playerName;
	NetworkId id;

	if (Game::getPlayerPath(loginId, clusterName, playerName, id))
	{
		result = "profiles/" + loginId + "/" + clusterName + "/" + id.getValueString().c_str() + "_friend_data.iff";
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestFriendList()
{
	DEBUG_REPORT_LOG(true, ("CommunityManagerNamespace::requestFriendList()\n"));

	uint32 const hash = Crc::normalizeAndCalculate("getFriendList");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestIgnoreList()
{
	DEBUG_REPORT_LOG(true, ("CommunityManagerNamespace::requestIgnoreList()\n"));

	uint32 const hash = Crc::normalizeAndCalculate("getIgnoreList");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestRemoveFriend(Unicode::String const &name)
{
	uint32 const hash = Crc::normalizeAndCalculate("removeFriend");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, name));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestAddFriend(Unicode::String const &name)
{
	uint32 const hash = Crc::normalizeAndCalculate("addFriend");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, name));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestRemoveIgnore(Unicode::String const &name)
{
	uint32 const hash = Crc::normalizeAndCalculate("removeIgnore");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, name));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::requestAddIgnore(Unicode::String const &name)
{
	uint32 const hash = Crc::normalizeAndCalculate("addIgnore");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, name));
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::load()
{
	s_loaded = true;

	std::string fileName(getFileName());

	if (!FileNameUtils::isReadable(fileName))
	{
		fileName = getFileNameOld();
	}

	DEBUG_REPORT_LOG(true, ("CommunityManagerNamespace::load() Loading local friend group/comments for: %s\n", fileName.c_str()));

	s_friendList.clear();

	// Check for file corruption

	if (Iff::isValid(fileName.c_str()))
	{
		Iff iff(2);

		if (iff.open(fileName.c_str(), true))
		{
			iff.enterForm(s_tag);
			{
				iff.enterChunk(TAG_0000);
				{
					int const count = iff.read_int32();

					for (int i = 0; i < count; ++i)
					{
						Unicode::String name(iff.read_unicodeString());
						Unicode::String group(iff.read_unicodeString());
						Unicode::String comment(iff.read_unicodeString());
						bool notifyOnlineStatus = (iff.read_int8() != 0);

						// Possibly add the new name to the list

						CommunityManager::FriendData &friendData = s_friendList[Unicode::toLower(name)];

						// Update the data, this prevents clobbering online/offline status

						friendData.setName(name);
						friendData.setGroup(group);
						friendData.setComment(comment);
						friendData.setNotifyOnlineStatus(notifyOnlineStatus);
					}
				}
				iff.exitChunk(TAG_0000);
			}
			iff.exitForm(s_tag);
		}
	}
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::save()
{
	// Save all the words

	Iff iff(2);

	iff.insertForm(s_tag);
	{
		iff.insertChunk(TAG_0000);
		{
			// Game text

			iff.insertChunkData(static_cast<int>(s_friendList.size()));

			CommunityManager::FriendList::const_iterator iterFriendList = s_friendList.begin();

			for (; iterFriendList != s_friendList.end(); ++iterFriendList)
			{
				CommunityManager::FriendData const &friendData = iterFriendList->second;

				iff.insertChunkString(friendData.getName());
				iff.insertChunkString(friendData.getGroup());
				iff.insertChunkString(friendData.getComment());
				iff.insertChunkData(static_cast<int8>(friendData.isNotifyOnlineStatus()));
			}
		}
		iff.exitChunk(TAG_0000);
	}
	iff.exitForm(s_tag);

	std::string fileName(getFileName());

	if (!fileName.empty())
	{
		iff.write(fileName.c_str());

		DEBUG_REPORT_LOG(true, ("CommunityManagerNamespace::save() %s\n", fileName.c_str()));
	}
}

//-----------------------------------------------------------------------------
void CommunityManagerNamespace::processOnlineStatusMessages()
{
	if (!s_queuedFriendOnlineStatus.empty())
	{
		CommunityManager::FriendList::const_iterator iterQueuedFriendOnlineStatus = s_queuedFriendOnlineStatus.begin();

		for (; iterQueuedFriendOnlineStatus != s_queuedFriendOnlineStatus.end(); ++iterQueuedFriendOnlineStatus)
		{
			Unicode::String const &name = iterQueuedFriendOnlineStatus->second.getName();
			bool const onlineStatus = iterQueuedFriendOnlineStatus->second.isOnline();
			Unicode::String const &lowerName = iterQueuedFriendOnlineStatus->first;

			CommunityManager::FriendList::iterator iterFriendList = s_friendList.find(lowerName);

			if (iterFriendList != s_friendList.end())
			{
				// Friend already exists, change the status

				iterFriendList->second.setOnline(onlineStatus);
			}
			else
			{
				// If the name is in the pending list, change the online status

				CommunityManager::FriendList::iterator iterPendingFriendList = s_pendingFriendList.find(lowerName);

				if (iterPendingFriendList != s_pendingFriendList.end())
				{
					iterPendingFriendList->second.setOnline(onlineStatus);
				}
				else
				{
					// This person must already be a friend but they are not in our
					// friend list yet since the server takes time to sync it, go ahead
					// and shove them into the list

					CommunityManager::FriendData &friendData = s_friendList[lowerName];
					friendData.setName(name);
					friendData.setOnline(onlineStatus);
				}
			}

			// Possibly notify the player of the person's online status

			if (   !CommunityManager::isIgnored(name)
			    && CommunityManager::isNotifyOnlineStatus(name))
			{
				Transceivers::friendOnlineStatusChanged.emitMessage(name);
			}
		}

		s_queuedFriendOnlineStatus.clear();
	}
}

// ============================================================================
//
// CommunityManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void CommunityManager::install()
{
	InstallTimer const installTimer("CommunityManager::install");

	DEBUG_FATAL(s_installed, ("Already installed."));

	s_loaded = false;

	CurrentUserOptionManager::registerOption(s_hideOfflineFriends, "ClientGame", "HideOfflineFriends");

	ExitChain::add(CommunityManager::remove, "CommunityManager::remove", 0, false);
	s_installed = true;
}

//-----------------------------------------------------------------------------
void CommunityManager::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed."));

	s_friendList.clear();
	s_ignoreList.clear();

	s_installed = false;
}

//-----------------------------------------------------------------------------
void CommunityManager::chatSystemConnected()
{
	s_chatSystemConnected = true;
}

//-----------------------------------------------------------------------------
void CommunityManager::alter(float const deltaTime)
{
	// We could potentially do timeouts from friend/ignore list requests

	UNREF(deltaTime);

	if (   isPlayerInfoValid()
	    && s_gameStart
	    && (!Game::playerIsLastPlayer()))
	{
		s_gameStart = false;
		load();
	}

	if (s_loaded)
	{
		if (s_chatSystemConnected)
		{
			s_chatSystemConnected = false;

			requestFriendList();
			requestIgnoreList();
		}

		// Handle any queued online status messages

		processOnlineStatusMessages();
	}

	if (s_saveRequested)
	{
		s_saveRequested = false;

		// This prevents multiple saves in a single frame

		save();
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::setFriendList(UnicodeStringSet const &friendList)
{
	{
		// Add all the new friends to the list

		UnicodeStringSet::const_iterator iterFriendList = friendList.begin();

		for (; iterFriendList != friendList.end(); ++iterFriendList)
		{
			Unicode::String const &name = (*iterFriendList);
			Unicode::String lowerName(Unicode::toLower(name));

			if (s_friendList.find(lowerName) == s_friendList.end())
			{
				// The name is not already in the list, so add it

				Unicode::String const &name = (*iterFriendList);

				FriendData friendData;
				friendData.setName(name);

				// See if we have any new group or comment information for this name

				FriendList::iterator iterPendingFriendList = s_pendingFriendList.find(lowerName);

				if (iterPendingFriendList != s_pendingFriendList.end())
				{
					friendData.setGroup(iterPendingFriendList->second.getGroup());
					friendData.setComment(iterPendingFriendList->second.getComment());
					friendData.setOnline(iterPendingFriendList->second.isOnline());
					friendData.setNotifyOnlineStatus(iterPendingFriendList->second.isNotifyOnlineStatus());

					s_pendingFriendList.erase(iterPendingFriendList);
				}

				s_friendList[lowerName] = friendData;
			}
		}

		// This throws out any pending people that were never added successfully
		// to the friend list

		if (s_pendingFriendList.size() > 1000)
		{
			s_pendingFriendList.clear();
		}
	}

	{
		typedef stdset<Unicode::String>::fwd RemoveList;
		RemoveList removeList;

		// Build a list of friends that are in the existing list but not in the new
		// list

		FriendList::iterator iterExistingFriendList = s_friendList.begin();

		for (; iterExistingFriendList != s_friendList.end(); ++iterExistingFriendList)
		{
			Unicode::String const &existingName = iterExistingFriendList->first;

			UnicodeStringSet::const_iterator iterNewFriendList = friendList.find(existingName);

			if (iterNewFriendList == friendList.end())
			{
				removeList.insert(existingName);
			}
		}

		// Remove anyone who is not in the new list

		RemoveList::const_iterator iterRemoveList = removeList.begin();

		for (; iterRemoveList != removeList.end(); ++iterRemoveList)
		{
			Unicode::String const &name = (*iterRemoveList);

			iterExistingFriendList = s_friendList.find(name);

			if (iterExistingFriendList != s_friendList.end())
			{
				s_friendList.erase(iterExistingFriendList);
			}
			else
			{
				DEBUG_WARNING(true, ("Friend name should be removed but is not found: %s", Unicode::wideToNarrow(name).c_str()));
			}
		}
	}

	Transceivers::friendListChanged.emitMessage(true);

	s_saveRequested = true;
}

//-----------------------------------------------------------------------------
void CommunityManager::addFriend(Unicode::String const &name, Unicode::String const &group, Unicode::String const &comment, bool const notifyOnlineStatus)
{
	Unicode::String trimName(Unicode::getTrim(name));

	if (!trimName.empty())
	{
		// See if the friend is already in the friend list

		FriendList::iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

		if (iterFriendList != s_friendList.end())
		{
			s_saveRequested = true;

			// They are already a friend, update the group and comment

			iterFriendList->second.setGroup(Unicode::getTrim(group));
			iterFriendList->second.setComment(Unicode::getTrim(comment));
			iterFriendList->second.setNotifyOnlineStatus(notifyOnlineStatus);

			// Let any UI know the group and comment changed

			Transceivers::friendListChanged.emitMessage(true);
		}
		else
		{
			// They are not in the friend list, so add the friend to the pending list,
			// while we wait for verification

			FriendData friendData;
			friendData.setName(trimName);
			friendData.setGroup(Unicode::getTrim(group));
			friendData.setComment(Unicode::getTrim(comment));
			friendData.setNotifyOnlineStatus(notifyOnlineStatus);
			s_pendingFriendList[Unicode::toLower(trimName)] = friendData;

			requestAddFriend(trimName);
		}
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::removeFriend(Unicode::String const &name)
{
	if (!name.empty())
	{
		requestRemoveFriend(name);
	}
}

//-----------------------------------------------------------------------------
bool CommunityManager::isFriend(Unicode::String const &name)
{
	bool result = false;
	FriendList::const_iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

	if (iterFriendList != s_friendList.end())
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManager::showFriendList()
{
	Transceivers::showFriendListMessage.emitMessage(true);
}

//-----------------------------------------------------------------------------
CommunityManager::FriendList const &CommunityManager::getFriendList()
{
	return s_friendList;
}

//-----------------------------------------------------------------------------
void CommunityManager::setFriendOnlineStatus(Unicode::String const &name, bool const online)
{
	//DEBUG_REPORT_LOG(true, ("CommunityManager::setFriendOnlineStatus() %s (%s)\n", Unicode::wideToNarrow(name).c_str(), online ? "online" : "offline"));

	FriendData &friendData = s_queuedFriendOnlineStatus[Unicode::toLower(name)];
	friendData.setName(name);
	friendData.setOnline(online);
}

//-----------------------------------------------------------------------------
bool CommunityManager::isFriendOnline(Unicode::String const &name)
{
	bool result = false;
	FriendList::const_iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

	if (iterFriendList != s_friendList.end())
	{
		result = iterFriendList->second.isOnline();
	}
	else
	{
		DEBUG_WARNING(true, ("Requesting the online status for a non-friend: %s", Unicode::wideToNarrow(name).c_str()));
	}

	return result;
}

//-----------------------------------------------------------------------------
bool CommunityManager::getFriendComment(Unicode::String const &name, Unicode::String &comment)
{
	bool result = false;
	FriendList::iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

	if (iterFriendList != s_friendList.end())
	{
		result = true;
		comment = iterFriendList->second.getComment();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool CommunityManager::getFriendGroup(Unicode::String const &name, Unicode::String &group)
{
	bool result = false;
	FriendList::iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

	if (iterFriendList != s_friendList.end())
	{
		result = true;
		group = iterFriendList->second.getGroup();
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManager::setIgnoreList(UnicodeStringSet const &ignoreList)
{
	s_ignoreList.clear();
	UnicodeStringSet::const_iterator iterIgnoreList = ignoreList.begin();

	for (; iterIgnoreList != ignoreList.end(); ++iterIgnoreList)
	{
		Unicode::String const &name = (*iterIgnoreList);

		s_ignoreList[Unicode::toLower(name)] = name;
	}

	Transceivers::ignoreListChanged.emitMessage(true);
}

//-----------------------------------------------------------------------------
void CommunityManager::addIgnore(Unicode::String const &name)
{
	if (!name.empty())
	{
		requestAddIgnore(name);
	}
}

//-----------------------------------------------------------------------------
void CommunityManager::removeIgnore(Unicode::String const &name)
{
	if (!name.empty())
	{
		requestRemoveIgnore(name);
	}
}

//-----------------------------------------------------------------------------
bool CommunityManager::isIgnored(Unicode::String const &name)
{
	bool result = false;

	IgnoreList::const_iterator iterIgnoreList = s_ignoreList.find(Unicode::toLower(name));

	if (iterIgnoreList != s_ignoreList.end())
	{
		result = true;
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManager::showIgnoreList()
{
	Transceivers::showIgnoreListMessage.emitMessage(true);
}

//-----------------------------------------------------------------------------
CommunityManager::IgnoreList const &CommunityManager::getIgnoreList()
{
	return s_ignoreList;
}

//-----------------------------------------------------------------------------
bool CommunityManager::isNotifyOnlineStatus(Unicode::String const &name)
{
	bool result = false;
	FriendList::const_iterator iterFriendList = s_friendList.find(Unicode::toLower(name));

	if (iterFriendList != s_friendList.end())
	{
		result = iterFriendList->second.isNotifyOnlineStatus();
	}
	else
	{
		DEBUG_WARNING(true, ("Requesting whether to notify online status for a non-friend: %s", Unicode::wideToNarrow(name).c_str()));
	}

	return result;
}

//-----------------------------------------------------------------------------
void CommunityManager::setHideOfflineFriends(bool const hide)
{
	s_hideOfflineFriends = hide;
}

//-----------------------------------------------------------------------------
bool CommunityManager::isHideOfflineFriends()
{
	return s_hideOfflineFriends;
}

//-----------------------------------------------------------------------------
void CommunityManager::gameStart()
{
	s_gameStart = true;
	s_loaded = Game::playerIsLastPlayer();
}

//-----------------------------------------------------------------
void CommunityManager::requestVerifyPlayerName(Unicode::String const &playerName)
{
	Unicode::String trimmedName(Unicode::getTrim(playerName));

	if (trimmedName.empty())
	{
		requestVerifyPlayerNameResponse(false, playerName);
	}
	else
	{
		Object *object = Game::getPlayer();

		if (object != NULL)
		{
			GameNetwork::send(VerifyPlayerNameMessage(playerName, object->getNetworkId()), true);
		}
		else
		{
			DEBUG_REPORT_LOG(true, ("CommunityManager::requestVerifyPlayerName() Unable to get the player's network id because the object is NULL!\n"));
		}
	}
}

//-----------------------------------------------------------------
void CommunityManager::requestVerifyPlayerNameResponse(bool const success, Unicode::String const &playerName)
{
	//DEBUG_REPORT_LOG(true, ("CommunityManager::requestVerifyPlayerNameResponse() success(%s) playerName(%s)\n", success ? "yes" : "no", Unicode::wideToNarrow(playerName).c_str()));

	Transceivers::requestVerifyPlayerNameResponse.emitMessage(std::make_pair(success, playerName));
}

// ============================================================================
