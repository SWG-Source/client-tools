//======================================================================
//
// ChatRoomDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "ChatLogEntryArchive.h"
#include "ChatLogEntry.h"

//----------------------------------------------------------------------
namespace Archive
{
	void get(ReadIterator& source, ChatLogEntry& target)
	{
		get(source, target.m_from);
		get(source, target.m_to);
		get(source, target.m_channel);
		get(source, target.m_message);
		get(source, target.m_time);
	}

	void put(ByteStream& target, const ChatLogEntry& source)
	{
		put(target, source.m_from);
		put(target, source.m_to);
		put(target, source.m_channel);
		put(target, source.m_message);
		put(target, source.m_time);
	}
}

//======================================================================
