// ChatOnRequestLog.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnRequestLog_H
#define	_INCLUDED_ChatOnRequestLog_H

//-----------------------------------------------------------------------

#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"
#include "ChatLogEntryArchive.h"
#include "ChatLogEntry.h"

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnRequestLog : public GameNetworkMessage
{
public:
	ChatOnRequestLog(unsigned int sequence, const std::vector<ChatLogEntry> &entries);
	ChatOnRequestLog(Archive::ReadIterator &);
	~ChatOnRequestLog();

	const std::vector<ChatLogEntry> &  getLogEntries  () const;
	const unsigned int                 getSequence    () const;

private:
	ChatOnRequestLog & operator = (const ChatOnRequestLog & rhs);
	ChatOnRequestLog(const ChatOnRequestLog & source);

	Archive::AutoArray<ChatLogEntry>     logEntries;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const std::vector<ChatLogEntry> & ChatOnRequestLog::getLogEntries() const
{
	return logEntries.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnRequestLog::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnRequestLog_H
