// ChatOnRequestLog.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "ChatLogEntryArchive.h"
#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnRequestLog.h"

//-----------------------------------------------------------------------

ChatOnRequestLog::ChatOnRequestLog(const unsigned int s, const std::vector<ChatLogEntry> & entries) :
GameNetworkMessage("ChatOnRequestLog"),
logEntries(),
sequence(s)
{
	logEntries.set(entries);
	addVariable(logEntries);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnRequestLog::ChatOnRequestLog(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnRequestLog"),
logEntries(),
sequence()
{
	addVariable(logEntries);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnRequestLog::~ChatOnRequestLog()
{
}

//======================================================================
