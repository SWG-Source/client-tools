// ======================================================================
//
// SurveyMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "SurveyMessageArchive.h"
#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/SurveyMessage.h"

// ======================================================================

const char * const SurveyMessage::MessageType = "SurveyMessage";

//----------------------------------------------------------------------

SurveyMessage::SurveyMessage (const stdvector<DataItem>::fwd &data) :
		GameNetworkMessage(MessageType),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

SurveyMessage::SurveyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage(MessageType),
		m_data()
{
	addVariable(m_data);
	unpack(source);
}

//----------------------------------------------------------------------

SurveyMessage::~SurveyMessage()
{
}

//----------------------------------------------------------------------

const std::vector<SurveyMessage::DataItem>& SurveyMessage::getData() const
{
	return m_data.get();
}

// ======================================================================
