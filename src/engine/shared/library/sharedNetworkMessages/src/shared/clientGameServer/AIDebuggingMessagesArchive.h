// ======================================================================
//
// AIDebuggingMessagesArchive.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AIDebuggingMessagesArchive_H
#define INCLUDED_AIDebuggingMessagesArchive_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"

// ======================================================================

struct AIPathInfo_NodeInfo;

namespace Archive
{
	void put(ByteStream & target, AIPathInfo_NodeInfo const &n);
	void get(ReadIterator & source, AIPathInfo_NodeInfo &n);
};

// ======================================================================

#endif
