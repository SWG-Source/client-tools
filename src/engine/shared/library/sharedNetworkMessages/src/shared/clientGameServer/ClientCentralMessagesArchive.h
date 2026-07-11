// ClientCentralMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_ClientCentralMessagesArchive_H
#define	_INCLUDED_ClientCentralMessagesArchive_H

//-----------------------------------------------------------------------

struct EnumerateCharacterId_Chardata;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator& source, EnumerateCharacterId_Chardata& c);
	void put(ByteStream& target, EnumerateCharacterId_Chardata const& c);
}

#endif	// _INCLUDED_ClientCentralMessagesArchive_H
