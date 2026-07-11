// ======================================================================
//
// SurveyMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SurveyMessage_H
#define INCLUDED_SurveyMessage_H

//-----------------------------------------------------------------------

#include "SurveyMessageTypes.h"
#include "SurveyMessageArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class SurveyMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	using DataItem = Survey_DataItem;
	
public:
	SurveyMessage           (const stdvector<DataItem>::fwd &data);
	explicit SurveyMessage  (Archive::ReadIterator & source);
	virtual  ~SurveyMessage ();
	const stdvector<DataItem>::fwd& getData() const;
	
private:
	
	Archive::AutoArray<DataItem> m_data;
	
private:
	SurveyMessage            (const SurveyMessage&);
	SurveyMessage& operator= (const SurveyMessage&);
};

// ======================================================================


// ======================================================================

#endif
