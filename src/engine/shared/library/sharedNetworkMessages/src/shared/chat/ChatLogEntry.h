//======================================================================
//
// ChatLogEntry.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatLogEntry_H
#define INCLUDED_ChatLogEntry_H

//======================================================================

#include "Unicode.h"

//-----------------------------------------------------------------------

struct ChatLogEntry
{
	ChatLogEntry();

    ChatLogEntry(Unicode::String const &from, Unicode::String const &to, Unicode::String const &channel, Unicode::String const &message, time_t time);

    Unicode::String m_from;
    Unicode::String m_to;
    Unicode::String m_channel;
    Unicode::String m_message;
    time_t m_time;
};

//======================================================================
#endif // INCLUDED_ChatLogEntry_H