//======================================================================
//
// ChatLogEntry.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatLogEntry.h"

//-----------------------------------------------------------------------

//======================================================================

ChatLogEntry::ChatLogEntry()
: m_from()
, m_to()
, m_channel()
, m_message()
, m_time(0)
{
}

ChatLogEntry::ChatLogEntry(Unicode::String const &from, Unicode::String const &to, Unicode::String const &channel, Unicode::String const &message, time_t time)
: m_from(from)
, m_to(to)
, m_channel(channel)
, m_message(message)
, m_time(time)
{
}

//======================================================================
