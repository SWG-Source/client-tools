// ======================================================================
//
// ResourceListForSurveyMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceListForSurveyMessage_H
#define INCLUDED_ResourceListForSurveyMessage_H

//-----------------------------------------------------------------------

#include "archive/ByteStream.h"
#include "archive/Archive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StlForwardDeclaration.h"

struct ResourceList_DataItem
{
	std::string     resourceName;
	NetworkId       resourceId;
	std::string     parentClassName;
};

namespace Archive
{
	void put(ByteStream& target, const ResourceList_DataItem& data);
	void get(ReadIterator& source, ResourceList_DataItem& data);
}

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ResourceListForSurveyMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef ResourceList_DataItem DataItem;

public:
	ResourceListForSurveyMessage                        (const stdvector<DataItem>::fwd &data, const std::string& type, const NetworkId& surveyToolId);
	explicit ResourceListForSurveyMessage               (Archive::ReadIterator & source);
	virtual  ~ResourceListForSurveyMessage              ();
	const std::vector<DataItem> &getData                () const;
	const std::string& getType                           () const;
	const NetworkId &getSurveyToolId                    () const;
	
private:
	Archive::AutoArray<DataItem>        m_data;
	Archive::AutoVariable<std::string>  m_surveyType;
	Archive::AutoVariable<NetworkId>    m_surveyToolId;
	
private:
	ResourceListForSurveyMessage            (const ResourceListForSurveyMessage&);
	ResourceListForSurveyMessage& operator= (const ResourceListForSurveyMessage&);
};

// ======================================================================



// ======================================================================

#endif
