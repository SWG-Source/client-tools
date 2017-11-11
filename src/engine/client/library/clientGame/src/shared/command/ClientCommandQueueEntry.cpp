//======================================================================
//
// ClientCommandQueueEntry.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientCommandQueueEntry.h"

#include "sharedGame/CommandTable.h"

//======================================================================

ClientCommandQueueEntry::ClientCommandQueueEntry() :
	m_command  (&CommandTable::getNullCommand ()),
	m_targetId (NetworkId::cms_invalid),
	m_params   ()
{
}

// ----------------------------------------------------------------------

ClientCommandQueueEntry::ClientCommandQueueEntry(Command const &command, NetworkId const &targetId, Unicode::String const &params) :
	m_command  (&command),
	m_targetId (targetId),
	m_params   (params)
{
}

// ----------------------------------------------------------------------

ClientCommandQueueEntry::ClientCommandQueueEntry(ClientCommandQueueEntry const &rhs) :
	m_command  (rhs.m_command),
	m_targetId (rhs.m_targetId),
	m_params   (rhs.m_params)
{
}

// ----------------------------------------------------------------------

ClientCommandQueueEntry &ClientCommandQueueEntry::operator=(ClientCommandQueueEntry const &rhs)
{
	if (&rhs != this)
	{
		m_command = rhs.m_command;
		m_targetId = rhs.m_targetId;
		m_params = rhs.m_params;
	}
	return *this;
}

//======================================================================
