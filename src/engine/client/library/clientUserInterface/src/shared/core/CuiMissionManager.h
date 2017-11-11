//======================================================================
//
// CuiMissionManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMissionManager_H
#define INCLUDED_CuiMissionManager_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

#include "clientGame/ClientMissionObject.h"

//======================================================================

class PopulateMissionBrowserMessage;
class MessageQueueMissionDetailsResponse;
class MessageQueueMissionGenericResponse;
class NetworkId;
class MessageQueueMissionCreateRequest;
class MessageQueueNetworkId;
class MessageQueueMissionCreateRequestData;
class SwgCuiMissionDetails;

//----------------------------------------------------------------------

class CuiMissionManager
{
public:
	
	struct Messages
	{		
		struct ReceiveList
		{
			typedef PopulateMissionBrowserMessage      Payload;
		};
		
		struct ReceiveDetails
		{
			typedef MessageQueueMissionDetailsResponse Payload;
		};
		
		struct ResponseAccept
		{
			typedef MessageQueueMissionGenericResponse Payload;
		};
		
		struct ResponseRemove
		{
			typedef MessageQueueMissionGenericResponse Payload;
		};

		struct ResponseAbort
		{
			typedef MessageQueueNetworkId Payload;
		};

		struct ResponseCreate
		{
			typedef MessageQueueMissionGenericResponse Payload;
		};
	};


	static void   install ();
	static void   remove  ();

	static void receiveList           (const Messages::ReceiveList::Payload &);
	static void receiveDetails        (const Messages::ReceiveDetails::Payload &);
	static void receiveResponseAccept (const Messages::ResponseAccept::Payload &);
	static void receiveResponseRemove (const Messages::ResponseRemove::Payload &);
	static void receiveResponseAbort  (const Messages::ResponseAbort::Payload  &);

	static void requestList    (bool mine);
	static void requestDetails (const NetworkId & mission);

	static void setDetails     (const ClientMissionObject* missionObject);
	static bool getDetailsType ();

	static void acceptMission (const NetworkId & mission, bool confirmed);
	static void removeMission (const NetworkId & mission, bool confirmed);

	static void addCurrentDestroyMission(const ClientMissionObject*);
	static void removeDestroyMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentDestroyMissions();
	static void addCurrentDeliverMission(const ClientMissionObject*);
	static void removeDeliverMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentDeliverMissions();
	static void addCurrentBountyMission(const ClientMissionObject*);
	static void removeBountyMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentBountyMissions();

	static void addCurrentDancerMission(const ClientMissionObject*);
	static void removeDancerMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentDancerMissions();
	static void addCurrentMusicianMission(const ClientMissionObject*);
	static void removeMusicianMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentMusicianMissions();
	static void addCurrentCraftingMission(const ClientMissionObject*);
	static void removeCraftingMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentCraftingMissions();
	static void addCurrentSurveyMission(const ClientMissionObject*);
	static void removeSurveyMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentSurveyMissions();
	static void addCurrentHuntingMission(const ClientMissionObject*);
	static void removeHuntingMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentHuntingMissions();
	static void addCurrentAssassinMission(const ClientMissionObject*);
	static void removeAssassinMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentAssassinMissions();
	static void addCurrentReconMission(const ClientMissionObject*);
	static void removeReconMission(const ClientMissionObject*);
	static const stdvector<ConstWatcher<ClientMissionObject> >::fwd & getCurrentReconMissions();

	static stdvector<NetworkId>::fwd const & getActiveMissions();
	static int getNumberOfActiveMissions();

	static ClientMissionObject const * getMission();

	static uint8 getSequenceRequestList ();
	static uint8 getSequenceRequestDetails ();

	static void              setTerminalId (const NetworkId & terminalNetworkId);
	static const NetworkId & getTerminalId ();

	static void update (float deltaTimeSeconds);

	static void formatLocationString (Unicode::String & locationStr, const Location & location);
	static void formatTime(Unicode::String & str, const struct tm & localTime);
	static void formatTimeSpan(Unicode::String & str, int hours);
	static void formatMoney (Unicode::String & str, int amount);
	static void formatProfessionRank (Unicode::String & str, int rank, const std::string & professionName);
	static void formatSuccessRate (Unicode::String & str, int rate);
	static void formatDifficulty (Unicode::String & str, int difficulty);
};

//======================================================================

#endif
