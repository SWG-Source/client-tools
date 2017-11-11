//======================================================================
//
// CuiChatRoomManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatRoomManager_H
#define INCLUDED_CuiChatRoomManager_H

class ChatFailedEnterRoom;
class ChatOnAddModeratorToRoom;
class ChatOnBanAvatarFromRoom;
class ChatOnCreateRoom;
class ChatOnDestroyRoom;
class ChatOnEnteredRoom;
class ChatOnInviteGroupToRoom;
class ChatOnInviteToRoom;
class ChatOnKickAvatarFromRoom;
class ChatOnLeaveRoom;
class ChatOnReceiveRoomInvitation;
class ChatOnRemoveModeratorFromRoom;
class ChatOnUnbanAvatarFromRoom;
class ChatQueryRoomResults;
class ChatRoomMessage;
class CuiChatAvatarId;
class CuiChatRoomDataNode;
class CuiChatRoomManagerStatusMessage;
class CuiChatRoomMessage;
class CuiPrelocalizedChatMessage;
struct ChatAvatarId;
struct ChatRoomData;

#include "StringId.h"

//======================================================================

class CuiChatRoomManager
{
public:

	typedef stdvector<ChatAvatarId>::fwd    AvatarVector;
	typedef stdvector<ChatRoomData>::fwd    RoomDataVector;
	typedef stdvector<uint32>::fwd          ChatRoomsEnteredVector;
	typedef CuiChatRoomManagerStatusMessage StatusMessage;
	typedef stdvector<std::string>::fwd     StringVector;

	struct Messages
	{
		typedef std::pair <const CuiChatRoomDataNode *, const CuiChatAvatarId *> RoomAvatarPair;

		typedef std::pair <const CuiChatAvatarId *, const CuiChatAvatarId *> AvatarPair;

		typedef std::pair <const CuiChatRoomDataNode *, AvatarPair>      RoomAvatarPairPair;
		typedef std::pair <const CuiChatRoomDataNode *, Unicode::String> RoomStringPair;

		struct OtherEntered
		{
			typedef RoomAvatarPair Payload;
		};

		struct SelfEntered
		{
			typedef CuiChatRoomDataNode Payload;
		};
 
		struct JoinFailed
		{
			typedef Unicode::String Payload;
		};

		struct OtherLeft
		{
			typedef RoomAvatarPair Payload;
		};

		struct SelfLeft
		{
			typedef CuiChatRoomDataNode Payload;
		};

		struct Destroyed
		{
			typedef RoomAvatarPair Payload;
		};

		struct DestroyFailed
		{
			typedef RoomStringPair Payload;
		};

		struct Created
		{
			typedef CuiChatRoomDataNode Payload;
		};

		struct CreateFailed
		{
			typedef Unicode::String Payload;
		};

		struct Modified
		{
			typedef uint32 Payload;
		};

		struct MessageReceived
		{
			typedef CuiChatRoomMessage Payload;
		};

		struct RoomMembersChanged
		{
			typedef CuiChatRoomDataNode Payload;
		};

		struct StatusMessage
		{
			typedef CuiChatRoomManagerStatusMessage Payload;
		};

		struct ModeratorAdded
		{
			typedef RoomAvatarPair Payload;
		};

		struct ModeratorRemoved
		{
			typedef RoomAvatarPair Payload;
		};

		struct PlanetRoomIdChanged
		{
			typedef uint32 Payload;
		};

		struct GroupRoomIdChanged
		{
			typedef uint32 Payload;
		};

		struct GuildRoomIdChanged
		{
			typedef uint32 Payload;
		};

		struct CityRoomIdChanged
		{
			typedef uint32 Payload;
		};

		struct NamedRoomIdChanged
		{
			typedef uint32 Payload;
		};

		struct InviteeAdded
		{
			typedef RoomAvatarPairPair Payload;
		};

		struct InviteeRemoved
		{
			typedef RoomAvatarPairPair Payload;
		};

		struct Banned
		{
			typedef RoomAvatarPairPair Payload;
		};

		struct Unbanned
		{
			typedef RoomAvatarPairPair Payload;
		};

		struct Prelocalized
		{
			typedef CuiPrelocalizedChatMessage Payload;
		};

	};

	static void                        install             ();
	static void                        remove              ();
	static void                        requestRoomsRefresh ();
	static void                        reset               ();

	static bool                        enterRoom     (uint32 id, const CuiChatRoomDataNode * room, Unicode::String & result);
	static bool                        enterRoom     (const std::string & name, Unicode::String & result);
	static void                        sendToRoom    (uint32 id, const Unicode::String & message, const Unicode::String & outOfBand);
	static void                        createRoom    (const std::string & name, bool isModerated, bool isPublic, const Unicode::String & title);

	static bool                        destroyRoom   (const std::string & name,                                                     Unicode::String & result);
	static bool                        destroyRoom   (uint32 id,                                                                    Unicode::String & result);
	static bool                        leaveRoom     (const std::string & name,                                                     Unicode::String & result);
	static bool                        leaveRoom     (uint32 id,                                                                    Unicode::String & result);
	static bool                        queryRoom     (const std::string & path,                                                     Unicode::String & result);
	static bool                        queryRoom     (uint32 id,                                                                    Unicode::String & result);
	static bool                        setInvited    (uint32, const std::string & avatarName, bool invited,                         Unicode::String & result);
	static bool                        setInvited    (const std::string & roomName, const std::string & avatarName, bool invited,   Unicode::String & result);
	static bool                        inviteGroup   (uint32, const std::string & avatarName,                                       Unicode::String & result);
	static bool                        inviteGroup   (const std::string & roomName, const std::string & avatarName,                 Unicode::String & result);
	static bool                        setBanned     (uint32, const std::string & avatarName, bool invited,                         Unicode::String & result);
	static bool                        setBanned     (const std::string & roomName, const std::string & avatarName, bool invited,   Unicode::String & result);
	static bool                        setModerator  (uint32, const std::string & avatarName, bool moderator,                       Unicode::String & result);
	static bool                        setModerator  (const std::string & roomName, const std::string & avatarName, bool moderator, Unicode::String & result);
	static bool                        kick          (uint32, const std::string & avatarName,                                       Unicode::String & result);
	static bool                        kick          (const std::string & roomName, const std::string & avatarName,                 Unicode::String & result);
	static bool                        hasNamedRoom  (uint32);

	static void	                       sendPrelocalizedChat(const Unicode::String & value);

	static const CuiChatRoomDataNode & getTree                 ();

	static void                        receiveRoomList                       (const RoomDataVector & v);
	static void                        receiveRoomQueryResults               (const ChatQueryRoomResults & msg);
	static void                        receiveOnDestroyRoom                  (const ChatAvatarId & destroyer, uint32 result, uint32 roomId, uint32 sequence);
	static void                        receiveOnEnteredRoom                  (const ChatOnEnteredRoom & msg);
	static void                        receiveOnCreateRoom                   (const uint32 sequence, const uint32 result, const ChatRoomData & roomData);
	static void                        receiveFailedEnterRoom                (const uint32 sequence, const uint32 result);
	static void                        receiveOnLeaveRoom                    (const ChatOnLeaveRoom & msg);
	static void                        receiveOnKickAvatarFromRoom           (const ChatOnKickAvatarFromRoom & msg);
	static void                        receiveChatOnReceiveRoomInvitation    (const ChatOnReceiveRoomInvitation & msg);
	static void                        receiveChatRoomMessage                (const ChatRoomMessage & msg);
	static void                        receiveChatOnSendRoomMessage          (const uint32 result, const uint32 sequence);
	static void                        receiveOnInviteToRoom                 (const ChatOnInviteToRoom & msg);
	static void                        receiveOnInviteGroupToRoom            (const ChatOnInviteGroupToRoom & msg);
	static void                        receiveOnUninviteFromRoom             (const uint32 result, const uint32 sequence);
	static void                        receiveOnAddModeratorToRoom           (const ChatOnAddModeratorToRoom & msg);
	static void                        receiveOnRemoveModeratorFromRoom      (const ChatOnRemoveModeratorFromRoom & msg);
	static void                        receiveOnBanAvatarFromRoom            (const ChatOnBanAvatarFromRoom & msg);
	static void                        receiveOnUnbanAvatarFromRoom          (const ChatOnUnbanAvatarFromRoom & msg);

	static const CuiChatRoomDataNode * findRoomNode            (const std::string & fullPath);
	static const CuiChatRoomDataNode * findRoomNode            (uint32 id);
	static const CuiChatRoomDataNode * findEnteredRoomNode     (const std::string & path);

	static int                         parse                   (const Unicode::String & str, Unicode::String & result, bool anyRoom = false);

	static void                        getEnteredRooms         (ChatRoomsEnteredVector & rv);

	static void                        getRoomShortPath        (const CuiChatRoomDataNode & roomNode, std::string & shortPath);

	static bool                        isRoomEntered           (uint32 id);

	static void                        addFriend               (const ChatAvatarId & avatarId);
	static uint32                      getPlanetRoomId         ();
	static uint32                      getGroupRoomId          ();
	static uint32                      getGuildRoomId          ();
	static uint32                      getCityRoomId           ();
	
	static const Unicode::String       ms_cmdGroup;

private:

	static CuiChatRoomDataNode *       findRoomNodeInternal     (uint32 id);
	static CuiChatRoomDataNode *       findRoomNodeInternal     (const std::string & fullpath);
	static void                        sendGenericStatusMessage (uint32 sequenceId, const StringId & prefixStringId, CuiChatRoomDataNode * roomNode, const CuiChatAvatarId * avatarId);
	static void                        internalDestroyRoom      (CuiChatRoomDataNode *& roomNode);
	static void                        setPlanetRoomId          (uint32 roomId);
	static void                        setGroupRoomId           (uint32 roomId);
	static void                        setGuildRoomId           (uint32 roomId);
	static void                        setCityRoomId            (uint32 roomId);
	static void                        setNamedRoomId           (uint32 roomId);

	static void                        processAvatarLeavingRoom (uint32 roomId, const ChatAvatarId & avatarId);

	static void                        getChatRoomSearchPathsLower   (StringVector & sv, const std::string & path);
};

//======================================================================

#endif
