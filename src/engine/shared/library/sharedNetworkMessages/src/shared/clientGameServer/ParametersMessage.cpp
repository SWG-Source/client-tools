// ======================================================================
//
// ParametersMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ParametersMessage.h"

// ======================================================================

ParametersMessage::ParametersMessage(int weatherUpdateInterval, int entertainerCaptchaPercent) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval(weatherUpdateInterval),
		m_entertainerCaptchaPercent(entertainerCaptchaPercent)
{
	addVariable(m_weatherUpdateInterval);
	addVariable(m_entertainerCaptchaPercent);
}

//-----------------------------------------------------------------------

ParametersMessage::ParametersMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ParametersMessage"),
		m_weatherUpdateInterval(),
		m_entertainerCaptchaPercent(40)
{
	addVariable(m_weatherUpdateInterval);
	addVariable(m_entertainerCaptchaPercent);

	unsigned short packedSize = 0;
	Archive::get(source, packedSize);

	unsigned short unpackedSize = 0;
	std::vector<Archive::AutoVariableBase *>::iterator i;
	for(i = members.begin(); i != members.end() && unpackedSize < packedSize; ++i, ++unpackedSize)
	{
		(*i)->unpack(source);
	}
}

// ----------------------------------------------------------------------

ParametersMessage::~ParametersMessage()
{
}


// ======================================================================

