// ClientCentralMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_ClientCentralMessagesTypes_H
#define	_INCLUDED_ClientCentralMessagesTypes_H

#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

//-----------------------------------------------------------------------

struct EnumerateCharacterId_Chardata
{
	enum CharacterType { CT_normal = 1, CT_jedi = 2, CT_spectral = 3 };

	Unicode::String m_name;
	int m_objectTemplateId;
	NetworkId m_networkId;
	uint32 m_clusterId;
	int m_characterType;

	EnumerateCharacterId_Chardata();
	EnumerateCharacterId_Chardata(const Unicode::String& name, int objectTemplateId, const NetworkId& networkId, uint32 clusterId, CharacterType characterType);
};

#endif	// _INCLUDED_ClientCentralMessagesTypes_H
