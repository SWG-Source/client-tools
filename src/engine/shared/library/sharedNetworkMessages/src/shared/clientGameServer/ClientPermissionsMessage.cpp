// ======================================================================
//
// ClientPermissionsMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientPermissionsMessage.h"

// ======================================================================

ClientPermissionsMessage::ClientPermissionsMessage(bool canLogin, bool canCreateRegularCharacter, bool canCreateJediCharacter, bool canSkipTutorial, bool isAdmin) :
		GameNetworkMessage("ClientPermissionsMessage"),
		m_canLogin(canLogin),
		m_canCreateRegularCharacter(canCreateRegularCharacter),
		m_canCreateJediCharacter(canCreateJediCharacter),
		m_canSkipTutorial(canSkipTutorial),
		m_isAdmin(isAdmin)
{
	addVariable(m_canLogin);
	addVariable(m_canCreateRegularCharacter);
	addVariable(m_canCreateJediCharacter);
	addVariable(m_canSkipTutorial);
	addVariable(m_isAdmin);
}

//-----------------------------------------------------------------------

ClientPermissionsMessage::ClientPermissionsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ClientPermissionsMessage"),
		m_canLogin(),
		m_canCreateRegularCharacter(),
		m_canCreateJediCharacter(),
		m_canSkipTutorial(),
		m_isAdmin()
{
	// m_isAdmin was added in a newer protocol revision than the upstream
	// server we run against. The server packs only 4 bools; if we add
	// m_isAdmin to the auto-unpack list, the unpack runs off the buffer
	// end and throws Archive::ReadException - which our NetworkHandler
	// dispatch catch block silently drops, leaving s_canCreateRegularCharacter
	// at its default `false` and triggering the "Sorry, you are not allowed
	// to create any more characters on this Galaxy." popup. Auto-unpack only
	// the first 4 fields, then optionally pick up m_isAdmin if the wire
	// payload includes it (newer servers).
	addVariable(m_canLogin);
	addVariable(m_canCreateRegularCharacter);
	addVariable(m_canCreateJediCharacter);
	addVariable(m_canSkipTutorial);

	unpack(source);

	if (source.getSize() >= sizeof(bool))
	{
		bool isAdminValue = false;
		Archive::get(source, isAdminValue);
		m_isAdmin.set(isAdminValue);
	}
}

// ----------------------------------------------------------------------

ClientPermissionsMessage::~ClientPermissionsMessage()
{
}

// ======================================================================
