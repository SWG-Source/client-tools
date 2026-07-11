// ======================================================================
//
// AIDebuggingMessagesArchive.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/AIDebuggingMessagesArchive.h"
#include "AIDebuggingMessagesArchiveTypes.h"

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"

// ======================================================================

namespace Archive
{
	void put(ByteStream& target, AIPathInfo_NodeInfo const& n)
	{
		put(target, n.node);
		unsigned char state = static_cast<unsigned char>(n.state);
		put(target, state);
	}

	void get(ReadIterator& source, AIPathInfo_NodeInfo& n)
	{
		get(source, n.node);
		unsigned char state;
		get(source, state);
		n.state = static_cast<AIPathInfo_NodeInfo::eState>(state);
	}
};

// ======================================================================
