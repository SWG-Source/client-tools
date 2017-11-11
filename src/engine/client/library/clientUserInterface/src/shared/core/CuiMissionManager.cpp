//======================================================================
//
// CuiMissionManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMissionManager.h"

#include "UnicodeUtils.h"
#include "UIUtils.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientRegionManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsMission.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListRequest.h"
#include "sharedNetworkMessages/PopulateMissionBrowserMessage.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Controller.h"
#include "sharedObject/VolumeContainer.h"

//======================================================================

namespace CuiMissionManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiMissionManager::Messages::ReceiveList::Payload &,    CuiMissionManager::Messages::ReceiveList > 
			receiveList;
		MessageDispatch::Transceiver<const CuiMissionManager::Messages::ReceiveDetails::Payload &, CuiMissionManager::Messages::ReceiveDetails > 
			receiveDetails;
		MessageDispatch::Transceiver<const CuiMissionManager::Messages::ResponseAccept::Payload &, CuiMissionManager::Messages::ResponseAccept > 
			responseAccept;
		MessageDispatch::Transceiver<const CuiMissionManager::Messages::ResponseRemove::Payload &, CuiMissionManager::Messages::ResponseRemove > 
			responseRemove;
		MessageDispatch::Transceiver<const CuiMissionManager::Messages::ResponseAbort::Payload  &, CuiMissionManager::Messages::ResponseAbort  > 
			responseAbort;
	}
	
	CuiMessageBox * s_messageBoxWaitingList    = 0;
	CuiMessageBox * s_messageBoxWaitingDetails = 0;
	CuiMessageBox * s_messageBoxWaitingAccept  = 0;
	CuiMessageBox * s_messageBoxWaitingRemove  = 0;
	CuiMessageBox * s_messageBoxConfirmAccept  = 0;
	CuiMessageBox * s_messageBoxConfirmRemove  = 0;

	struct MissionId
	{
		NetworkId missionData;
		NetworkId terminal;
	};

	MissionId s_acceptMission;
	MissionId s_removeMission;


	uint8      s_sequenceRequestList;
	uint8      s_sequenceWaitingAccept;
	uint8      s_sequenceWaitingRemove;
	uint8      s_sequenceRequestDetails;

	enum WaitingType
	{
		REQUEST_LIST,
		REQUEST_DETAILS,
		ACCEPT_MISSION,
		REMOVE_MISSION,
		NONE
	} s_waitingType = NONE;

	float       s_timeOut    = 1.0f;
	const float s_maxTimeOut = 1.0f;

	void cancelRequestList ()
	{
		if (s_messageBoxWaitingList)
			s_messageBoxWaitingList->closeMessageBox ();
		++s_sequenceRequestList;
	}

	void cancelRequestDetails ()
	{
		if (s_messageBoxWaitingDetails)
			s_messageBoxWaitingDetails->closeMessageBox ();
		++s_sequenceRequestDetails;
	}

	void cancelWaitingAccept ()
	{
		if (s_messageBoxWaitingAccept)
			s_messageBoxWaitingAccept->closeMessageBox ();

		++s_sequenceWaitingAccept;
	}
	
	void cancelWaitingRemove ()
	{
		if (s_messageBoxWaitingRemove)
			s_messageBoxWaitingRemove->closeMessageBox ();
		++s_sequenceWaitingRemove;
	}

	void cancelConfirmAccept ()
	{
		if (s_messageBoxConfirmAccept)
			s_messageBoxConfirmAccept->closeMessageBox ();
	}

	void cancelConfirmRemove ()
	{
		if (s_messageBoxConfirmRemove)
			s_messageBoxConfirmRemove->closeMessageBox ();
	}

	struct Listener : public MessageDispatch::Receiver 
	{
		Listener()
		: MessageDispatch::Receiver()
		{
			connectToMessage ("PopulateMissionBrowserMessage");
		}

		void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
		{
			const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
			
			if (abm)
			{
				if (message.isType (CuiMessageBox::Messages::COMPLETED))
				{
					const CuiMessageBox::CompletedMessage * const cmsg = NON_NULL(dynamic_cast<const CuiMessageBox::CompletedMessage *>(&message));
					
					if (cmsg->getButtonType () == CuiMessageBox::GBT_Ok)
					{
						if (abm->getMessageBox () == s_messageBoxConfirmAccept)
							CuiMissionManager::acceptMission (s_acceptMission.missionData, true);
						else if (abm->getMessageBox () == s_messageBoxConfirmRemove)
							CuiMissionManager::removeMission (s_removeMission.missionData, true);
					}
				}
				else if (message.isType (CuiMessageBox::Messages::CLOSED))
				{					
					if (abm->getMessageBox () ==      s_messageBoxWaitingList)    s_messageBoxWaitingList    = 0;
					else if (abm->getMessageBox () == s_messageBoxWaitingDetails) s_messageBoxWaitingDetails = 0;
					else if (abm->getMessageBox () == s_messageBoxWaitingAccept)  s_messageBoxWaitingAccept  = 0;
					else if (abm->getMessageBox () == s_messageBoxWaitingRemove)  s_messageBoxWaitingRemove  = 0;
					else if (abm->getMessageBox () == s_messageBoxConfirmAccept)  s_messageBoxConfirmAccept  = 0;
					else if (abm->getMessageBox () == s_messageBoxConfirmRemove)  s_messageBoxConfirmRemove  = 0;
				}

			}

			if (message.isType("PopulateMissionBrowserMessage"))
			{
				Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();				
				const PopulateMissionBrowserMessage msg (ri);
				CuiMissionManager::receiveList(msg);
			}
		}
	};

	Listener * s_listener  = 0;
	bool       s_installed = false;

	enum CurrentDetails
	{
		MissionObject,
		MissionListEntryObject
	} s_current;

	ConstWatcher<ClientMissionObject>               ms_mission;
	NetworkId                                       s_missionTerminalId;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentDestroyMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentDeliverMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentBountyMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentDancerMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentMusicianMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentCraftingMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentSurveyMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentHuntingMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentAssassinMissions;
	std::vector<ConstWatcher<ClientMissionObject> > ms_currentReconMissions;
	std::vector<NetworkId>                          ms_activeMissions;
}

using namespace CuiMissionManagerNamespace;

//----------------------------------------------------------------------

void CuiMissionManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_listener  = new Listener;
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiMissionManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	delete s_listener;
	s_listener  = 0;
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiMissionManager::receiveList    (const Messages::ReceiveList::Payload & payload)
{
	cancelRequestList ();
	Transceivers::receiveList.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiMissionManager::receiveDetails (const Messages::ReceiveDetails::Payload & payload)
{
	if (payload.getSequenceId () != s_sequenceRequestDetails)
	{
		WARNING (true, ("discarding stale mission details response"));
		return;
	}

	cancelRequestDetails ();
	Transceivers::receiveDetails.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiMissionManager::receiveResponseAccept (const Messages::ResponseAccept::Payload & payload)
{
	if (payload.getSequenceId () == s_sequenceWaitingAccept)
		cancelWaitingAccept ();

	//@todo: do something here to display success/fail?
	Transceivers::responseAccept.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiMissionManager::receiveResponseAbort (const Messages::ResponseAbort::Payload & payload)
{
	cancelWaitingRemove ();

	//@todo: do something here to display success/fail?
	Transceivers::responseAbort.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiMissionManager::receiveResponseRemove (const Messages::ResponseRemove::Payload & payload)
{
	cancelWaitingRemove ();

	//@todo: do something here to display success/fail?
	Transceivers::responseRemove.emitMessage (payload);
}

//----------------------------------------------------------------------

void CuiMissionManager::requestList    (bool mine)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	ClientObject * const player = Game::getClientPlayer ();
	Controller * const controller = player ? NON_NULL (player->getController()) : 0;
	
	if (!controller)
		return;

	cancelRequestList ();


	uint8 flags = 0;
	if (mine)
		flags |= static_cast<int>(MessageQueueMissionListRequest::F_mineOnly);

	MessageQueue::Data * const message =  new MessageQueueMissionListRequest (s_missionTerminalId, flags, ++s_sequenceRequestList);

	//-- enqueue message
	controller->appendMessage (static_cast<int>(CM_missionListRequest), 0.0f, message, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

	s_messageBoxWaitingList = CuiMessageBox::createMessageBox (CuiStringIdsMission::wait_list.localize ());
	s_messageBoxWaitingList->setRunner (true);
	s_waitingType = REQUEST_LIST;
	s_timeOut = 0.0f;
	s_messageBoxWaitingList->connectToMessages (*s_listener);

	ms_currentDestroyMissions.clear();
	ms_currentDeliverMissions.clear();
	ms_currentBountyMissions.clear();
	ms_currentDancerMissions.clear();
	ms_currentMusicianMissions.clear();
	ms_currentCraftingMissions.clear();
	ms_currentSurveyMissions.clear();
	ms_currentHuntingMissions.clear();
	ms_currentAssassinMissions.clear();
	ms_currentReconMissions.clear();
	ms_activeMissions.clear();
}

//----------------------------------------------------------------------

void CuiMissionManager::requestDetails (const NetworkId & mission)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	ClientObject * const player = Game::getClientPlayer ();
	Controller * const controller = player ? NON_NULL (player->getController()) : 0;
	
	if (!controller)
		return;

	cancelRequestDetails ();

	MessageQueue::Data * const message = new MessageQueueMissionDetailsRequest (mission, s_missionTerminalId, ++s_sequenceRequestDetails);

	//-- enqueue message
	controller->appendMessage (static_cast<int>(CM_missionDetailsRequest), 0.0f, message, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

	s_messageBoxWaitingDetails = CuiMessageBox::createMessageBox (CuiStringIdsMission::wait_details.localize ());
	s_messageBoxWaitingDetails->setRunner (true);
	s_waitingType = REQUEST_DETAILS;
	s_timeOut = 0.0f;
	s_messageBoxWaitingDetails->connectToMessages (*s_listener);
}

//-----------------------------------------------------------------

void CuiMissionManager::acceptMission (const NetworkId & mission, bool confirmed)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	cancelConfirmAccept ();
	
	if (confirmed)
	{
		ClientObject * const player = Game::getClientPlayer ();
		Controller * const controller = player ? NON_NULL (player->getController()) : 0;
		
		if (!controller)
			return;
		
		MessageQueueMissionGenericRequest * const message = new MessageQueueMissionGenericRequest(mission, s_missionTerminalId, ++s_sequenceWaitingAccept);
		
		//-- enqueue message
		controller->appendMessage (static_cast<int>(CM_missionAcceptRequest), 0.0f, message, 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

		s_messageBoxWaitingAccept = CuiMessageBox::createMessageBox (CuiStringIdsMission::wait_accept.localize ());
		s_messageBoxWaitingAccept->setRunner (true);
		s_waitingType = ACCEPT_MISSION;
		s_timeOut = 0.0f;
		s_messageBoxWaitingAccept->connectToMessages (*s_listener);
	}
	else
	{
		s_acceptMission.missionData = mission;
		s_acceptMission.terminal = s_missionTerminalId;

		s_messageBoxConfirmAccept = CuiMessageBox::createOkCancelBox (CuiStringIdsMission::confirm_accept.localize ());
		s_messageBoxConfirmAccept->connectToMessages (*s_listener);
	}
}

//-----------------------------------------------------------------

void CuiMissionManager::removeMission (const NetworkId & mission, bool confirmed)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	cancelConfirmRemove ();
	
	if (confirmed)
	{
		ClientObject * const player = Game::getClientPlayer ();
		Controller * const controller = player ? NON_NULL (player->getController()) : 0;
		
		if (!controller)
			return;

		MessageQueueNetworkId* const message = new MessageQueueNetworkId(mission);
		controller->appendMessage(static_cast<int>(CM_missionAbort), 0.0f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

		s_messageBoxWaitingRemove = CuiMessageBox::createMessageBox (CuiStringIdsMission::wait_remove.localize ());
		s_messageBoxWaitingRemove->setRunner (true);
		s_waitingType = REMOVE_MISSION;
		s_timeOut = 0.0f;
		s_messageBoxWaitingRemove->connectToMessages (*s_listener);
	}
	else
	{
		s_removeMission.missionData = mission;
		s_removeMission.terminal = s_missionTerminalId;

		s_messageBoxConfirmRemove = CuiMessageBox::createOkCancelBox (CuiStringIdsMission::confirm_remove.localize ());
		s_messageBoxConfirmRemove->connectToMessages (*s_listener);
	}
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentDestroyMission(const ClientMissionObject* mission)
{
	ms_currentDestroyMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeDestroyMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentDestroyMissions.begin(), ms_currentDestroyMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentDestroyMissions.end())
	{
		ms_currentDestroyMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentDestroyMissions()
{
	return ms_currentDestroyMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentDeliverMission(const ClientMissionObject* mission)
{
	ms_currentDeliverMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeDeliverMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentDeliverMissions.begin(), ms_currentDeliverMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentDeliverMissions.end())
	{
		ms_currentDeliverMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentDeliverMissions()
{
	return ms_currentDeliverMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentBountyMission(const ClientMissionObject* mission)
{
	ms_currentBountyMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeBountyMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentBountyMissions.begin(), ms_currentBountyMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentBountyMissions.end())
	{
		ms_currentBountyMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentBountyMissions()
{
	return ms_currentBountyMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentDancerMission(const ClientMissionObject* mission)
{
	ms_currentDancerMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeDancerMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentDancerMissions.begin(), ms_currentDancerMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentDancerMissions.end())
	{
		ms_currentDancerMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentDancerMissions()
{
	return ms_currentDancerMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentMusicianMission(const ClientMissionObject* mission)
{
	ms_currentMusicianMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeMusicianMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentMusicianMissions.begin(), ms_currentMusicianMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentMusicianMissions.end())
	{
		ms_currentMusicianMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentMusicianMissions()
{
	return ms_currentMusicianMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentCraftingMission(const ClientMissionObject* mission)
{
	ms_currentCraftingMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeCraftingMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentCraftingMissions.begin(), ms_currentCraftingMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentCraftingMissions.end())
	{
		ms_currentCraftingMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentCraftingMissions()
{
	return ms_currentCraftingMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentSurveyMission(const ClientMissionObject* mission)
{
	ms_currentSurveyMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeSurveyMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentSurveyMissions.begin(), ms_currentSurveyMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentSurveyMissions.end())
	{
		ms_currentSurveyMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentSurveyMissions()
{
	return ms_currentSurveyMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentHuntingMission(const ClientMissionObject* mission)
{
	ms_currentHuntingMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeHuntingMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentHuntingMissions.begin(), ms_currentHuntingMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentHuntingMissions.end())
	{
		ms_currentHuntingMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentHuntingMissions()
{
	return ms_currentHuntingMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentAssassinMission(const ClientMissionObject* mission)
{
	ms_currentAssassinMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeAssassinMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentAssassinMissions.begin(), ms_currentAssassinMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentAssassinMissions.end())
	{
		ms_currentAssassinMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentAssassinMissions()
{
	return ms_currentAssassinMissions;
}

//-----------------------------------------------------------------

void CuiMissionManager::addCurrentReconMission(const ClientMissionObject* mission)
{
	ms_currentReconMissions.push_back(ConstWatcher<ClientMissionObject>(mission));
}

//-----------------------------------------------------------------

void CuiMissionManager::removeReconMission(const ClientMissionObject* mission)
{
	std::vector<ConstWatcher<ClientMissionObject> >::iterator i = std::find(ms_currentReconMissions.begin(), ms_currentReconMissions.end(), ConstWatcher<ClientMissionObject>(mission));
	if(i != ms_currentReconMissions.end())
	{
		ms_currentReconMissions.erase(i);
	}
}

//-----------------------------------------------------------------

const std::vector<ConstWatcher<ClientMissionObject> > & CuiMissionManager::getCurrentReconMissions()
{
	return ms_currentReconMissions;
}

//-----------------------------------------------------------------

uint8 CuiMissionManager::getSequenceRequestList ()
{
	return s_sequenceRequestList;
}

//----------------------------------------------------------------------

uint8 CuiMissionManager::getSequenceRequestDetails ()
{
	return s_sequenceRequestDetails;
}

//-----------------------------------------------------------------------

void CuiMissionManager::setTerminalId (const NetworkId & id)
{
	s_missionTerminalId = id;
}

//-----------------------------------------------------------------------

const NetworkId & CuiMissionManager::getTerminalId ()
{
	return s_missionTerminalId;
}

//-----------------------------------------------------------------------

void CuiMissionManager::setDetails (const ClientMissionObject* missionObject)
{
	ms_mission = missionObject;
	s_current = MissionObject;
}

//-----------------------------------------------------------------------

bool CuiMissionManager::getDetailsType()
{
	if(s_current == MissionObject)
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------

void CuiMissionManager::update (float deltaTimeSeconds)
{
	if(s_timeOut < s_maxTimeOut)
	{
		s_timeOut += deltaTimeSeconds;
		if(s_timeOut >= s_maxTimeOut)
		{
			switch(s_waitingType)
			{
			case REQUEST_LIST:
				cancelRequestList();
				break;
			case REQUEST_DETAILS:
				cancelRequestDetails();
				break;
			case ACCEPT_MISSION:
				cancelWaitingAccept();
				break;
			case REMOVE_MISSION:
				cancelWaitingRemove();
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------

ClientMissionObject const * CuiMissionManager::getMission()
{
	return ms_mission.getPointer();
}

//-----------------------------------------------------------------------

std::vector<NetworkId> const & CuiMissionManager::getActiveMissions()
{
	CreatureObject const * const cro = Game::getPlayerCreature();
	ClientObject const * const dp = cro ? cro->getDatapadObject() : NULL;
	ms_activeMissions.clear();

	if (dp)
	{
		VolumeContainer const * const vc = ContainerInterface::getVolumeContainer(*dp);
		if (vc)
		{
			for (ContainerConstIterator j = vc->begin(); j != vc->end(); ++j)
			{
				CachedNetworkId const & cnid = *j;
				Object const * const o = cnid.getObject();
				ClientMissionObject const * const cmo = dynamic_cast<ClientMissionObject const *>(o);
				if(cmo)
					ms_activeMissions.push_back(cmo->getNetworkId());
			}
		}
	}

	return ms_activeMissions;
}

//-----------------------------------------------------------------------

int CuiMissionManager::getNumberOfActiveMissions()
{
	return ms_activeMissions.size();
}

//-----------------------------------------------------------------------

void CuiMissionManager::formatLocationString (Unicode::String & locationStr, const Location & location)
{
	const std::string& currentPlanet = Game::getSceneId();
	const std::string& waypointPlanet = location.getSceneId();
	StringId sid("planet_n", waypointPlanet);

	//if unknown planet name
	if(waypointPlanet.empty())
	{
		locationStr = CuiStringIdsMission::unknown_planet.localize();
	}
	//if a different planet
	else if(currentPlanet != waypointPlanet)
	{
		StringId region = ClientRegionManager::getRegionAtPoint(waypointPlanet, location.getCoordinates());
		locationStr.clear();
		if(region.isValid())
		{
			locationStr = region.localize();
			locationStr += Unicode::narrowToWide(", ");
		}
		locationStr += sid.localize();
	}
	//else on the same planet
	else
	{
		const Vector& playerLoc = Game::getPlayer()->getPosition_w();
		const Vector& waypointLoc = location.getCoordinates();
		float distance = abs(playerLoc.magnitudeBetween(waypointLoc));
		int iDistance = static_cast<int>(distance);
		char buffer[256];
		_itoa(iDistance, buffer, 10);
		locationStr = Unicode::narrowToWide(buffer);

		Vector delta = waypointLoc - playerLoc;
		real theta = delta.theta();

		std::string cardinal;

		if(theta > -PI/8 && theta < PI/8)
			cardinal = "N";
		else if(theta > PI/8 && theta < 3*PI/8)
			cardinal = "NE";
		else if(theta > 3*PI/8 && theta < 5*PI/8)
			cardinal = "E";
		else if(theta > 5*PI/8 && theta < 7*PI/8)
			cardinal = "SE";
		else if(theta < -PI/8 && theta > -3*PI/8)
			cardinal = "NW";
		else if(theta < -3*PI/8 && theta > -5*PI/8)
			cardinal = "W";
		else if(theta < -5*PI/8 && theta > -7*PI/8)
			cardinal = "SW";
		else
			cardinal = "S";
		// @todo : localize this!!!!!
		locationStr += Unicode::narrowToWide("m to the ");
		locationStr += Unicode::narrowToWide(cardinal);
	}
}

//----------------------------------------------------------------------

void CuiMissionManager::formatTime(Unicode::String & str, const struct tm & localTime)
{
	const char * atime = asctime (&localTime);
	
	if (!atime)
		return;
	
	Unicode::NarrowString strtime (atime);
	
	size_t endpos = 0;

	std::string strDay;
	IGNORE_RETURN (Unicode::getFirstToken (strtime, 0, endpos, strDay));
	
	std::string strMonth;
	IGNORE_RETURN (Unicode::getFirstToken  (strtime, endpos, endpos, strMonth));
	
	
	str += Unicode::narrowToWide (strMonth);
	IGNORE_RETURN (str.append (1, ' '));
	
	Unicode::String tmp;
	if (UIUtils::FormatLong(tmp, localTime.tm_mday))
		IGNORE_RETURN (str.append (tmp));
	
	IGNORE_RETURN (str.append (1, ','));
	IGNORE_RETURN (str.append (1, ' '));

	if (UIUtils::FormatLong(tmp, localTime.tm_hour % 12))
		IGNORE_RETURN (str.append (tmp));

	if (localTime.tm_hour > 12)
		IGNORE_RETURN (str.append (Unicode::narrowToWide (" PM")));
	else
		IGNORE_RETURN (str.append (Unicode::narrowToWide (" AM")));
}

//----------------------------------------------------------------------

void CuiMissionManager::formatTimeSpan(Unicode::String & str, int hours)
{
	const int days = hours / 24;
	const int hoursRemaining = hours % 24;

	Unicode::String tmp;

	if (days)
	{
		IGNORE_RETURN (UIUtils::FormatLong (tmp, days));
		str += tmp;
		IGNORE_RETURN (str.append (1, ' '));
		IGNORE_RETURN (CuiStringIds::token_days.localize (tmp));
		str += tmp;

		if (hoursRemaining)
		{
			IGNORE_RETURN (str.append (1, ','));
			IGNORE_RETURN (str.append (1, ' '));
		}
	}

	if (hoursRemaining)
	{
		IGNORE_RETURN (UIUtils::FormatLong (tmp, hoursRemaining));
		str += tmp;
		IGNORE_RETURN (str.append (1, ' '));
		IGNORE_RETURN (CuiStringIds::token_hours.localize (tmp));
		str += tmp;
	}
}

//----------------------------------------------------------------------

void CuiMissionManager::formatMoney (Unicode::String & str, int amount)
{
	Unicode::String tmp;
	IGNORE_RETURN (UIUtils::FormatLong (str, amount));
	IGNORE_RETURN (str.append (1, ' '));

	IGNORE_RETURN (CuiStringIds::token_currency.localize (tmp));
	str += tmp;
}

//-----------------------------------------------------------------------

void CuiMissionManager::formatProfessionRank (Unicode::String & str, int rank, const std::string & professionName)
{
	IGNORE_RETURN (UIUtils::FormatLong (str, rank));

	IGNORE_RETURN (str.append (1, ' '));
	str += Unicode::narrowToWide (professionName);
}

//-----------------------------------------------------------------------

void CuiMissionManager::formatSuccessRate (Unicode::String & str, int rate)
{
	IGNORE_RETURN (UIUtils::FormatLong (str, rate));
	IGNORE_RETURN (str.append (1, '%'));
	IGNORE_RETURN (str.append (1, ' '));

	Unicode::String tmp;
	IGNORE_RETURN (CuiStringIdsMission::success_rate.localize (tmp));
	str += tmp;
}

//-----------------------------------------------------------------------

void CuiMissionManager::formatDifficulty (Unicode::String & str, int difficulty)
{
	Unicode::String tmp;
	IGNORE_RETURN (UIUtils::FormatLong (str, difficulty));
	str += tmp;
}


//=======================================================================
