//======================================================================
//
// SwgCuiChatWindow_ChannelId.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiChatWindow_ChannelId.h"

#include "UnicodeUtils.h"
#include "clientUserInterface/CuiChatRoomDataNode.h"
#include "clientUserInterface/CuiChatRoomManager.h"
#include "clientUserInterface/CuiStringIdsChat.h"

//======================================================================

namespace SwgCuiChatWindowChannelIdNamespace
{
	const std::string s_chatTabNamesStringFile = "chat_tab_names";
};

SwgCuiChatWindow::ChannelId::ChannelId () : 
type        (CT_none),
name        (),
lowerName   (),
displayName (),
isPublic    (true)
{
	updateDefaultName ();
}

//----------------------------------------------------------------------

SwgCuiChatWindow::ChannelId::ChannelId  (ChannelType _type) :
type        (_type),
name        (),
lowerName   (),
displayName (),
isPublic    (true)
{
	updateDefaultName ();
}

//----------------------------------------------------------------------

SwgCuiChatWindow::ChannelId::ChannelId  (ChannelType _type, const std::string & _name) :
type        (_type),
name        (_name),
lowerName   (Unicode::toLower (_name)),
displayName (),
isPublic    (true)
{
	updateDisplayName ();
}

//----------------------------------------------------------------------

SwgCuiChatWindow::ChannelId::ChannelId  (ChannelType _type, const std::string & _name, bool _isPublic) :
type        (_type),
name        (_name),
lowerName   (Unicode::toLower (_name)),
displayName (),
isPublic    (_isPublic)
{
	updateDisplayName ();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::ChannelId::setName (const std::string & _name)
{
	name      = _name;
	lowerName = Unicode::toLower (_name);

	updateDisplayName ();
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::ChannelId::updateDefaultName ()
{
	switch (type)
	{
	case CT_none:
		setName ("none");
		break;
	case CT_spatial:
		setName ("spatial");
		break;
	case CT_combat:
		setName ("combat");
		break;
	case CT_gcw:
		setName ("gcw");
		break;
	case CT_systemMessage:
		setName ("system");
		break;		
	case CT_instantMessage:
		setName ("im");
		break;		
	case CT_planet:
		setName ("planet");
		break;		
	case CT_group:
		setName ("group");
		break;		
	case CT_guild:
		setName ("guild");
		break;
	case CT_city:
		setName ("city");
		break;
	case CT_matchMaking:
		setName ("matchMaking");
		break;
	case CT_quest:
		setName ("quest");
		break;
	case CT_named:
	case CT_chatRoom:
		updateDisplayName ();
		break;
	}
}

//----------------------------------------------------------------------

void SwgCuiChatWindow::ChannelId::updateDisplayName ()
{
	switch (type)
	{
	case CT_named:
		{
			if(! name.empty())
			{
				size_t dot = name.rfind('.');
				if(dot > 0 && dot != std::string::npos && dot < name.length() - 1)
				{
					++dot;
					//Try to look up the name in the chat tab names string file
					StringId lookup(SwgCuiChatWindowChannelIdNamespace::s_chatTabNamesStringFile, Unicode::toLower(name.substr(dot)));
					if(!lookup.localize(displayName))
						displayName = Unicode::narrowToWide(name.substr(dot));
				}
			}
		}
		break;
	case CT_chatRoom:
		{
			const CuiChatRoomDataNode * const roomNode = CuiChatRoomManager::findRoomNode (name);
			
			if (roomNode)
			{
				std::string shortPath;
				CuiChatRoomManager::getRoomShortPath (*roomNode, shortPath);
				//Try to look up the name in the chat tab names string file
				StringId lookup(SwgCuiChatWindowChannelIdNamespace::s_chatTabNamesStringFile, Unicode::toLower(shortPath));
				if(!lookup.localize(displayName))
					displayName = Unicode::narrowToWide(shortPath);
			}
		}
		break;
	case CT_none:
		displayName = CuiStringIdsChat::chat_channelid_none.localize ();
		break;
	case CT_spatial:
		displayName = CuiStringIdsChat::chat_channelid_spatial.localize ();
		break;
	case CT_combat:
		displayName = CuiStringIdsChat::chat_channelid_combat.localize ();
		break;
	case CT_gcw:
		displayName = CuiStringIdsChat::chat_channelid_gcw.localize ();
		break;
	case CT_systemMessage:
		displayName = CuiStringIdsChat::chat_channelid_systemmessage.localize ();
		break;		
	case CT_instantMessage:
		displayName = CuiStringIdsChat::chat_channelid_instantmessage.localize ();
		break;		
	case CT_planet:
		displayName = CuiStringIdsChat::chat_channelid_planet.localize ();
		break;		
	case CT_group:
		displayName = CuiStringIdsChat::chat_channelid_group.localize ();
		break;
	case CT_guild:
		displayName = CuiStringIdsChat::chat_channelid_guild.localize ();
		break;
	case CT_city:
		displayName = CuiStringIdsChat::chat_channelid_city.localize ();
		break;
	case CT_matchMaking:
		displayName = CuiStringIdsChat::chat_channelid_match_making.localize ();
		break;		
	case CT_quest:
		displayName = CuiStringIdsChat::chat_channelid_quest.localize ();
		break;		
	}
}

//----------------------------------------------------------------------

bool SwgCuiChatWindow::ChannelId::getIsPublic() const
{
	return isPublic;
}

//======================================================================
