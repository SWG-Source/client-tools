// ClientCentralMessagesArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "ClientCentralMessagesTypes.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "Archive/Archive.h"

namespace Archive
{
	void get(ReadIterator& source, EnumerateCharacterId_Chardata& c)
	{
		get(source, c.m_name);
		get(source, c.m_objectTemplateId);
		get(source, c.m_networkId);
		get(source, c.m_clusterId);
		get(source, c.m_characterType);
	}

	void put(ByteStream& target, EnumerateCharacterId_Chardata const& c)
	{
		put(target, c.m_name);
		put(target, c.m_objectTemplateId);
		put(target, c.m_networkId);
		put(target, c.m_clusterId);
		put(target, c.m_characterType);
	}
}