//======================================================================
//
// ClientCommandQueueEntry.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientCommandQueueEntry_H
#define INCLUDED_ClientCommandQueueEntry_H

//======================================================================

#include "Unicode.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class Command;

//----------------------------------------------------------------------

class ClientCommandQueueEntry
{
public:
	ClientCommandQueueEntry ();
	ClientCommandQueueEntry (Command const &command, NetworkId const &targetId, Unicode::String const &params);
	ClientCommandQueueEntry (ClientCommandQueueEntry const &rhs);
	ClientCommandQueueEntry &operator=(ClientCommandQueueEntry const &rhs);
public:
	Command const * m_command;
	NetworkId       m_targetId;
	Unicode::String m_params;
};

//======================================================================

#endif
