// ======================================================================
//
// ServerObjectData.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerObjectData_H
#define INCLUDED_ServerObjectData_H

// ======================================================================

#include "Singleton/Singleton.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

//----------------------------------------------------------------------

class ServerObjectData : public Singleton<ServerObjectData>, public MessageDispatch::Receiver, public MessageDispatch::Emitter
{
public:

	//----------------------------------------------------------------------
	struct ObjectInfo
	{
		NetworkId         networkId;
		std::string       serverTemplateName;
		std::vector<std::string> scriptList;
		std::vector<std::string> objvarList;
		std::vector<std::string> skillList;
	};
	typedef stdmap<NetworkId, ObjectInfo>::fwd ObjectInfoMap;
	
	//----------------------------------------------------------------------

	struct Messages
	{
		static const char* const OBJECT_INFO_CHANGED;
	};

	//----------------------------------------------------------------------

	class ObjectInfoChangedMessage : public MessageDispatch::MessageBase
	{
		public:
			explicit ObjectInfoChangedMessage(const NetworkId& networkId) : MessageBase(Messages::OBJECT_INFO_CHANGED), m_networkId(networkId) {}

			const NetworkId& getNetworkId() const
			{
				return m_networkId;
			};

		private:
			//disabled
			ObjectInfoChangedMessage();
			ObjectInfoChangedMessage& operator=(const ObjectInfoChangedMessage& rhs);
			ObjectInfoChangedMessage(const ObjectInfoChangedMessage& rhs);

		private:
			NetworkId m_networkId;
	};

	//----------------------------------------------------------------------

	enum MessageType
	{
		MT_serverTemplate,
		MT_scriptList,
		MT_objvarList,
		MT_triggerList
	};

	ServerObjectData();
	~ServerObjectData();
	const ObjectInfoMap& getObjectInfoMap() const;
	const ObjectInfo*    getObjectInfo(const NetworkId& networkId, bool requestIfEmpty);
	void                 requestObjectInfo(const NetworkId& networkId, bool getAllData);
	void                 receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);
	void                 setObjectInfo(NetworkId const &networkId, std::string const &serverTemplateName, std::vector<std::string> const &scripts, std::vector<std::string> const &objvars);

private:
	//disabled
	ServerObjectData& operator=(const ServerObjectData& rhs);
	ServerObjectData(const ServerObjectData& rhs);

private:
	ObjectInfoMap*    m_objectInfoMap;
	typedef stdmap<unsigned int, MessageType>::fwd RequestIdMap;
	RequestIdMap*     m_outstandingRequests;
};

//----------------------------------------------------------------------

inline const ServerObjectData::ObjectInfoMap& ServerObjectData::getObjectInfoMap() const
{
	NOT_NULL(m_objectInfoMap);
	return *m_objectInfoMap;
}


// ======================================================================

#endif
