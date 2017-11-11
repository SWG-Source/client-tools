//======================================================================
//
// CuiChatRoomManagerListener.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiChatRoomManagerListener.h"

#include "clientGame/CommunityManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIdsChatRoom.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedNetworkMessages/ChatAddFriend.h"
#include "sharedNetworkMessages/ChatAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatCreateRoom.h"
#include "sharedNetworkMessages/ChatEnterRoom.h"
#include "sharedNetworkMessages/ChatEnterRoomById.h"
#include "sharedNetworkMessages/ChatInviteAvatarToRoom.h"
#include "sharedNetworkMessages/ChatOnAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatFriendsListUpdate.h"
#include "sharedNetworkMessages/ChatOnBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnConnectAvatar.h"
#include "sharedNetworkMessages/ChatOnCreateRoom.h"
#include "sharedNetworkMessages/ChatOnDestroyRoom.h"
#include "sharedNetworkMessages/ChatOnEnteredRoom.h"
#include "sharedNetworkMessages/ChatOnInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatOnInviteToRoom.h"
#include "sharedNetworkMessages/ChatOnKickAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnLeaveRoom.h"
#include "sharedNetworkMessages/ChatOnReceiveRoomInvitation.h"
#include "sharedNetworkMessages/ChatOnRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatOnSendRoomMessage.h"
#include "sharedNetworkMessages/ChatOnUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatOnUninviteFromRoom.h"
#include "sharedNetworkMessages/ChatQueryRoom.h"
#include "sharedNetworkMessages/ChatQueryRoomResults.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatRemoveModeratorFromRoom.h"
#include "sharedNetworkMessages/ChatRequestRoomList.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"
#include "sharedNetworkMessages/ChatRoomList.h"
#include "sharedNetworkMessages/ChatRoomMessage.h"
#include "sharedNetworkMessages/ChatSendToRoom.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/VerifyPlayerNameResponseMessage.h"

//======================================================================
namespace
{
	//----------------------------------------------------------------------

	namespace MessageNames
	{
		const char * const ChatOnAddModeratorToRoom      = "ChatOnAddModeratorToRoom";
		const char * const ChatOnBanAvatarFromRoom       = "ChatOnBanAvatarFromRoom";
		const char * const ChatOnUnbanAvatarFromRoom     = "ChatOnUnbanAvatarFromRoom";
		const char * const ChatOnCreateRoom              = "ChatOnCreateRoom";
		const char * const ChatOnDestroyRoom             = "ChatOnDestroyRoom";
		const char * const ChatOnEnteredRoom             = "ChatOnEnteredRoom";
		const char * const ChatOnInviteToRoom            = "ChatOnInviteToRoom";
		const char * const ChatOnInviteGroupToRoom       = "ChatOnInviteGroupToRoom";
		const char * const ChatOnKickAvatarFromRoom      = "ChatOnKickAvatarFromRoom";
		const char * const ChatOnLeaveRoom               = "ChatOnLeaveRoom";
		const char * const ChatOnReceiveRoomInvitation   = "ChatOnReceiveRoomInvitation";
		const char * const ChatOnRemoveModeratorFromRoom = "ChatOnRemoveModeratorFromRoom";
		const char * const ChatOnSendRoomMessage         = "ChatOnSendRoomMessage";
		const char * const ChatOnUninviteFromRoom        = "ChatOnUninviteFromRoom";
		const char * const ChatQueryRoomResults          = "ChatQueryRoomResults";
		const char * const ChatRoomList                  = "ChatRoomList";
		const char * const ChatRoomMessage               = "ChatRoomMessage";
		const char * const CmdStartScene                 = "CmdStartScene";
		const char * const ChatFriendsListUpdate         = "ChatFriendsListUpdate";
		const char * const ChatOnConnectAvatar           = "ChatOnConnectAvatar";
		const char * const VerifyPlayerNameResponseMessage = "VerifyPlayerNameResponseMessage";
		const char * const ChatSpamLimited               = "ChatSpamLimited";
	}

	//----------------------------------------------------------------------

	class Listener : 
	public MessageDispatch::Receiver
	{
	public:
		
		//----------------------------------------------------------------------
		
		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage (MessageNames::ChatOnAddModeratorToRoom);
			connectToMessage (MessageNames::ChatOnCreateRoom);
			connectToMessage (MessageNames::ChatOnDestroyRoom);
			connectToMessage (MessageNames::ChatOnEnteredRoom);
			connectToMessage (MessageNames::ChatOnInviteToRoom);
			connectToMessage (MessageNames::ChatOnInviteGroupToRoom);
			connectToMessage (MessageNames::ChatOnKickAvatarFromRoom);
			connectToMessage (MessageNames::ChatOnLeaveRoom);
			connectToMessage (MessageNames::ChatOnReceiveRoomInvitation);
			connectToMessage (MessageNames::ChatOnRemoveModeratorFromRoom);
			connectToMessage (MessageNames::ChatOnSendRoomMessage);
			connectToMessage (MessageNames::ChatOnUninviteFromRoom);
			connectToMessage (MessageNames::ChatQueryRoomResults);
			connectToMessage (MessageNames::ChatRoomList);
			connectToMessage (MessageNames::ChatRoomMessage);
			connectToMessage (MessageNames::CmdStartScene);
			connectToMessage (MessageNames::ChatFriendsListUpdate);
			connectToMessage (MessageNames::ChatOnConnectAvatar);
			connectToMessage (MessageNames::ChatOnBanAvatarFromRoom);
			connectToMessage (MessageNames::ChatOnUnbanAvatarFromRoom);
			connectToMessage (MessageNames::VerifyPlayerNameResponseMessage);
			connectToMessage (MessageNames::ChatSpamLimited);
		}
		
		//----------------------------------------------------------------------
		
		void receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			//----------------------------------------------------------------------
			
			if(message.isType(MessageNames::CmdStartScene))
			{
				CuiChatRoomManager::requestRoomsRefresh ();
			}

			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnConnectAvatar))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatOnConnectAvatar msg (ri);
				UNREF(msg);

				CommunityManager::chatSystemConnected();
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatRoomList))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatRoomList msg (ri);
				CuiChatRoomManager::receiveRoomList (msg.getRoomData ());
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatQueryRoomResults))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatQueryRoomResults msg (ri);
				CuiChatRoomManager::receiveRoomQueryResults (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnEnteredRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatOnEnteredRoom msg (ri);
				
				if(msg.getResult())
				{
					// only sequence and result are guaranteed to be valid when there's a failure
					CuiChatRoomManager::receiveFailedEnterRoom (msg.getSequence (), msg.getResult ());
				}
				else
				{
					CuiChatRoomManager::receiveOnEnteredRoom (msg);
				}
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnKickAvatarFromRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatOnKickAvatarFromRoom msg (ri);
				CuiChatRoomManager::receiveOnKickAvatarFromRoom (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnLeaveRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatOnLeaveRoom msg (ri);
				CuiChatRoomManager::receiveOnLeaveRoom (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatRoomMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();				  
				const ChatRoomMessage msg (ri);				  
				CuiChatRoomManager::receiveChatRoomMessage (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnCreateRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				
				const ChatOnCreateRoom msg (ri);
				CuiChatRoomManager::receiveOnCreateRoom (msg.getSequence (), msg.getResultCode (), msg.getRoomData ());
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnDestroyRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnDestroyRoom msg (ri);
				CuiChatRoomManager::receiveOnDestroyRoom (msg.getDestroyer (), msg.getResultCode (), msg.getRoomId (), msg.getSequence ());
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnAddModeratorToRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnAddModeratorToRoom msg (ri);
				CuiChatRoomManager::receiveOnAddModeratorToRoom (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnRemoveModeratorFromRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnRemoveModeratorFromRoom msg (ri);
				CuiChatRoomManager::receiveOnRemoveModeratorFromRoom (msg);
			}
			
			//----------------------------------------------------------------------
			
			else if (message.isType (MessageNames::ChatOnReceiveRoomInvitation))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnReceiveRoomInvitation msg (ri);
				CuiChatRoomManager::receiveChatOnReceiveRoomInvitation (msg);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatOnSendRoomMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnSendRoomMessage msg (ri);
				CuiChatRoomManager::receiveChatOnSendRoomMessage (msg.getResult (), msg.getSequence ());
			}			

			//----------------------------------------------------------------------
			else if (message.isType (MessageNames::ChatOnInviteToRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnInviteToRoom msg (ri);
				CuiChatRoomManager::receiveOnInviteToRoom (msg);
			}

			//----------------------------------------------------------------------
			else if (message.isType (MessageNames::ChatOnInviteGroupToRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnInviteGroupToRoom msg (ri);
				CuiChatRoomManager::receiveOnInviteGroupToRoom (msg);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatOnUninviteFromRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnUninviteFromRoom msg (ri);
				CuiChatRoomManager::receiveOnUninviteFromRoom (msg.getResult (), msg.getSequence ());
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatFriendsListUpdate))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatFriendsListUpdate msg (ri);

				//DEBUG_REPORT_LOG(true, ("ChatFriendsListUpdate - <friend> %s <online> %s\n", msg.getCharacterName().name.c_str(), msg.getIsOnline() ? "yes" : "no"));

				CommunityManager::setFriendOnlineStatus(Unicode::narrowToWide(msg.getCharacterName().getNameWithNecessaryPrefix(std::string("SWG"), GameNetwork::getCentralServerName())), msg.getIsOnline());
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatOnBanAvatarFromRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnBanAvatarFromRoom msg (ri);
				CuiChatRoomManager::receiveOnBanAvatarFromRoom (msg);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatOnUnbanAvatarFromRoom))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const ChatOnUnbanAvatarFromRoom msg (ri);
				CuiChatRoomManager::receiveOnUnbanAvatarFromRoom (msg);
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::VerifyPlayerNameResponseMessage))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
				const VerifyPlayerNameResponseMessage msg (ri);
				CommunityManager::requestVerifyPlayerNameResponse(msg.isValid(), msg.getPlayerName());
			}

			//----------------------------------------------------------------------

			else if (message.isType (MessageNames::ChatSpamLimited))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();

				const GenericValueTypeMessage<int> csl(ri);
				if (csl.getValue() > 0)
				{
					Unicode::String errorString;
					CuiStringVariablesManager::process(CuiStringIdsChatRoom::chat_spam_limited, Unicode::emptyString, Unicode::narrowToWide(CalendarTime::convertSecondsToMS(csl.getValue())), Unicode::emptyString, errorString);
					CuiSystemMessageManager::sendFakeSystemMessage(errorString);
				}
			}
			else
			{
				DEBUG_REPORT_LOG(true, ("CuiChatRoomManagerListener: Unhandled message\n"));
			}
		}
	};

	Listener * s_listener     = 0;
}

//----------------------------------------------------------------------

void CuiChatRoomManagerListener::install ()
{
	s_listener  = new Listener;
}

//----------------------------------------------------------------------

void CuiChatRoomManagerListener::remove ()
{
	NOT_NULL (s_listener);
	delete s_listener;
	s_listener = 0;
}

//======================================================================
