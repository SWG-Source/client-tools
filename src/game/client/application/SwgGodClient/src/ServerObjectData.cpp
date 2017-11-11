// ======================================================================
//
// ActionsScript.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ServerObjectData.h"

#include "BuildoutAreaSupport.h"
#include "GodClientData.h"
#include "ServerCommander.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/World.h"

#include <map>

//----------------------------------------------------------------------

namespace
{
	typedef MessageDispatch::Message<std::pair<NetworkId, int> > ObjectMessage;
}

const char * const ServerObjectData::Messages::OBJECT_INFO_CHANGED = "ServerObjectData::Messages::OBJECT_INFO_CHANGED";

//----------------------------------------------------------------------

ServerObjectData::ServerObjectData()
: Singleton<ServerObjectData>(),
  MessageDispatch::Receiver(),
  MessageDispatch::Emitter(),
  m_objectInfoMap(0),
  m_outstandingRequests(0)
{
	m_objectInfoMap       = new ObjectInfoMap;
	m_outstandingRequests = new RequestIdMap;
	connectToMessage("ConGenericMessage");
	connectToMessage(World::Messages::OBJECT_ADDED);
}

//----------------------------------------------------------------------

ServerObjectData::~ServerObjectData()
{
	delete m_outstandingRequests;
	m_outstandingRequests = 0;
	delete m_objectInfoMap;
	m_objectInfoMap = 0;
}

//----------------------------------------------------------------------

const ServerObjectData::ObjectInfo * ServerObjectData::getObjectInfo(const NetworkId& networkId, bool requestIfEmpty)
{
	NOT_NULL(m_objectInfoMap);
	const ObjectInfoMap::const_iterator it = m_objectInfoMap->find(networkId);
	if(it == m_objectInfoMap->end())
	{
		if(requestIfEmpty)
			requestObjectInfo(networkId, false);
		return 0;
	}
	return &(*it).second;
}

//----------------------------------------------------------------------

void ServerObjectData::requestObjectInfo(const NetworkId& networkId, bool getAllData)
{
	if (Game::getSinglePlayer())
	{
		if (networkId < NetworkId::cms_invalid)
			BuildoutAreaSupport::populateServerObjectData(networkId);
		return;
	}

	NOT_NULL(m_objectInfoMap);
	if(getAllData)
	{
		const unsigned int reqnum1 = ServerCommander::getInstance().scriptRequestList(networkId);
		if(reqnum1)
		{
			NOT_NULL(m_outstandingRequests);
			IGNORE_RETURN((*m_outstandingRequests)[reqnum1] = MT_scriptList);
		}

		const unsigned int reqnum2 = ServerCommander::getInstance().objvarRequestList(networkId);
		if(reqnum2)
		{
			NOT_NULL(m_outstandingRequests);
			IGNORE_RETURN((*m_outstandingRequests)[reqnum2] = MT_objvarList);
		}
	}

	const unsigned int reqnum3 = ServerCommander::getInstance().serverTemplateRequest(networkId);
	if(reqnum3)
	{
		NOT_NULL(m_outstandingRequests);
		IGNORE_RETURN((*m_outstandingRequests)[reqnum3] = MT_serverTemplate);
	}
	const unsigned int reqnum4 = ServerCommander::getInstance().getTriggerVolumes(networkId);
	if(reqnum4)
	{
		NOT_NULL(m_outstandingRequests);
		IGNORE_RETURN((*m_outstandingRequests)[reqnum4] = MT_triggerList);
	}
}

//----------------------------------------------------------------------

void ServerObjectData::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	//get all the data for an object immediately, so that all the GUI is updated appropriately
	if(message.isType(World::Messages::OBJECT_ADDED))
	{
		const ObjectMessage * const om = dynamic_cast<const ObjectMessage *>(&message);
		if (om)
		{
			NetworkId nid = om->getValue().first;
			if (nid.isValid())
			{
				ClientObject * obj = dynamic_cast<ClientObject*>(NetworkIdManager::getObjectById (nid));
				if (obj)
				{
					requestObjectInfo(obj->getNetworkId(), false);
					if(GodClientData::getInstance().getObjectCreationPending())
					{
						GodClientData::getInstance().setObjectCreationPending(false);
						GodClientData::getInstance().setSelection(obj);
					}
				}
			}
		}
	}

	//handle all messages that come in as regular text (yuck)
	if(message.isType("ConGenericMessage"))
	{
		NOT_NULL(m_outstandingRequests);

		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		ConGenericMessage cg(ri);
		const RequestIdMap::iterator it = m_outstandingRequests->find(cg.getMsgId());
		if(it == m_outstandingRequests->end())
			return;

		MessageType type = it->second;
		m_outstandingRequests->erase(it);

		const std::string msg = cg.getMsg();
		std::vector<std::string> lines;
		std::string line;
		size_t startpos = 0;
		size_t endpos   = 0;

		//make sure the line is well formed
		if(!Unicode::getFirstToken(msg, 0, endpos, line, "\n"))
			return;

		//build a vector of the lines
		startpos = 0;
		endpos   = 0;
		while(Unicode::getFirstToken(msg, startpos, endpos, line, "\n"))
		{
			lines.push_back(line);
			if(endpos == std::string::npos) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
				break;
			else
				startpos = endpos + 1;
		}

		char oid[256];
		switch(type)
		{
			case MT_scriptList:
			{
				size_t       num_scripts = 0;
				int result = sscanf(line.c_str(), "script list object %s, num %d\n", oid, &num_scripts);
				UNREF(num_scripts);
				if(result > 0)
				{
					NOT_NULL(m_objectInfoMap);
					ObjectInfoMap::iterator oit = m_objectInfoMap->find(NetworkId(oid));
					if(oit == m_objectInfoMap->end())
					{
						oit = m_objectInfoMap->insert(std::make_pair(NetworkId(oid), ObjectInfo())).first;
					}
					ObjectInfo & oi =(*oit).second;
					oi.scriptList.clear();
					
					if(num_scripts)
					{
						while(endpos != msg.npos && Unicode::getFirstToken(msg, ++endpos, endpos, line, "\n")) //lint !e737 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
						{
							oi.scriptList.push_back(line);
						}
					}
				}
				break;
			}

			case MT_serverTemplate:
			{
				if(!Unicode::getFirstToken(msg, 0, endpos, line, "\n"))
					return;
				//the name comes back with an .iff, strip it
				size_t pos = line.find(".iff");
				if(pos == std::string::npos) //lint !e737 !e650 implicit promotion, bug in STL with size_t and std::string::npos being of different signage
					return;
				char serverTemplateName[1024];
				int result = sscanf(line.c_str(), "%s %s\n", oid, serverTemplateName);
				if(result == 0)
					return;
				NOT_NULL(m_objectInfoMap);
				ObjectInfoMap::iterator oit = m_objectInfoMap->find(NetworkId(oid));
				if(oit == m_objectInfoMap->end())
				{
					oit = m_objectInfoMap->insert(std::make_pair(NetworkId(oid), ObjectInfo())).first;
				}
				oit->second.serverTemplateName = serverTemplateName;
				break;
			}

			case MT_objvarList:
			{
				break;
			}

			case MT_triggerList:
			{
				unsigned int numTriggers;
				int result = sscanf(lines[0].c_str(), "Listing %d trigger volumes for object %s", &numTriggers, oid);

				//the first line must list the number of triggers following, and the number of additional lines we read should be this big
				//subtract 2, one for the header line and one for the footer line
				if((result != 2) || (numTriggers != lines.size()-2))
					return;

				char name[1024];
				real radius;
				for(unsigned int i = 1; i <= numTriggers; ++i)
				{
					result = sscanf(lines[i].c_str(), "trigger volume for object %s %s %f\n", oid, name, &radius);
					IGNORE_RETURN(std::make_pair(std::string(name), radius));

					//TODO send this to the object editor
				}

				break;
			}

			default:
				DEBUG_FATAL(true, ("unknown message type"));
		}

		emitMessage(ObjectInfoChangedMessage(NetworkId(oid)));
	}
}

//----------------------------------------------------------------------

void ServerObjectData::setObjectInfo(NetworkId const &networkId, std::string const &serverTemplateName, std::vector<std::string> const &scripts, std::vector<std::string> const &objvars)
{
	ObjectInfo &objectInfo = (*m_objectInfoMap)[networkId];
	objectInfo.networkId = networkId;
	objectInfo.serverTemplateName = serverTemplateName;
	objectInfo.scriptList = scripts;
	objectInfo.objvarList = objvars;
}

//----------------------------------------------------------------------
