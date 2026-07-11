//======================================================================
//
// ChatLogEntryArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatLogEntryArchive_H
#define INCLUDED_ChatLogEntryArchive_H

//======================================================================

#include "Archive/ByteStream.h"

struct ChatLogEntry;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatLogEntry & target);
	void put (ByteStream & target, const ChatLogEntry & source);
}
//======================================================================

#endif //INCLUDED_ChatLogEntryArchive_H
