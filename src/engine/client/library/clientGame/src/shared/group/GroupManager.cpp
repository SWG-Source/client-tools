// ======================================================================
//
// GroupManager.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GroupManager.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientUserInterface/CuiConsentManager.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ProsePackage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"

#include <map>
#include <set>

// ======================================================================


namespace GroupManagerNamespace
{
	std::map<NetworkId, GroupManager::CreatureSet> s_groupMembers;
	
	NetworkId ms_consentGroupInviter;
	NetworkId ms_spaceLaunchConsentGroupInviter;
	NetworkId ms_currentSpaceLaunchConsentGroupInviter;
	const int ms_noConsentId = -1;
	int ms_consentId = ms_noConsentId;
	int ms_spaceLaunchConsentId = ms_noConsentId;
	int ms_numberOfLaunchIntoSpaceUIOpen = 0;

	/** ConsentManger callback when a user selects an option off the consent widget to accept a group invite.
	Since we can only have one invite pending, we don't need to track the consent id
	*/
	void inviteResponseHandler(int consentId, bool response)
	{
		if (consentId != ms_consentId)
			return;
		
		ms_consentId = ms_noConsentId;
		ms_consentGroupInviter = NetworkId::cms_invalid;

		if(response)
		{
			static const uint32 hash_group_join = Crc::normalizeAndCalculate("join");
			ClientCommandQueue::enqueueCommand (hash_group_join, ms_consentGroupInviter, Unicode::emptyString);
		}
		else
		{
			static const uint32 hash_group_decline = Crc::normalizeAndCalculate("decline");
			ClientCommandQueue::enqueueCommand (hash_group_decline, ms_consentGroupInviter, Unicode::emptyString);
		}
	}

	void sendInvitationToLaunchIntoSpaceResponse(NetworkId const inviter, bool response)
	{
		Object * const player = Game::getPlayer();
		Controller * const playerController = (player != 0) ? player->getController() : 0;

		if (playerController != 0)
		{
			typedef std::pair<NetworkId, bool> Payload;
			Payload payload;
			payload.first = inviter;
			payload.second = response;

			MessageQueueGenericValueType<Payload> * data = new MessageQueueGenericValueType<Payload>(payload);

			playerController->appendMessage(CM_groupMemberInvitationToLaunchIntoSpaceResponse,
												0.0f,
												data,
												GameControllerMessageFlags::SEND |
												GameControllerMessageFlags::RELIABLE |
												GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}

	void inviteToLaunchIntoSpaceResponseHandler(int consentId, bool response)
	{
		if (consentId == ms_spaceLaunchConsentId)
		{
			if (response)
			{
				if (ms_currentSpaceLaunchConsentGroupInviter.isValid())
				{
					sendInvitationToLaunchIntoSpaceResponse(ms_currentSpaceLaunchConsentGroupInviter, false);
				}

				ms_currentSpaceLaunchConsentGroupInviter = ms_spaceLaunchConsentGroupInviter;
			}

			if (ms_numberOfLaunchIntoSpaceUIOpen == 0)
			{
				sendInvitationToLaunchIntoSpaceResponse(ms_spaceLaunchConsentGroupInviter, response);
			}
			else
			{
				CuiSystemMessageManager::sendFakeSystemMessage (ClientStringIds::group_space_launch_cannot_be_passenger.localize());
			}

			ms_spaceLaunchConsentGroupInviter = NetworkId::cms_invalid;
			ms_spaceLaunchConsentId = ms_noConsentId;
		}
	}
}

using namespace GroupManagerNamespace;

// ======================================================================

void GroupManager::handleGroupChange(CreatureObject & creature, NetworkId const &oldGroup, NetworkId const &newGroup)
{
	// remove from old group if any
	if (oldGroup != NetworkId::cms_invalid)
	{
		std::map<NetworkId, CreatureSet>::iterator i = s_groupMembers.find(oldGroup);
		(*i).second.erase(&creature);
		if ((*i).second.empty())
			s_groupMembers.erase(i);
	}
	// add to new group if any
	if (newGroup != NetworkId::cms_invalid)
	{
		s_groupMembers[newGroup].insert(&creature);
	}
}

// ----------------------------------------------------------------------

void GroupManager::getGroupMembers(CreatureObject & creature, CreatureSet &members)
{
	members.clear();

	NetworkId const &group = creature.getGroup();
	if (group != NetworkId::cms_invalid)
	{
		members = s_groupMembers[group];
		return;
	}
	members.insert(&creature);
}

//----------------------------------------------------------------------

void GroupManager::handleGroupInviterChange (CreatureObject & creature)
{
	if (&creature == Game::getPlayer ())
	{
		NetworkId const & id = creature.getAppropriateGroupInviterBasedOffSceneType();

		if (id.isValid ())
		{
			std::string const & groupInviterName = creature.getGroupInviter().first.second;
			const ClientObject * const inviter = safe_cast<ClientObject *>(NetworkIdManager::getObjectById (id));
	
			//auto reject?
			if (CuiPreferences::getAutoInviteReject ())
			{
				if (inviter || !groupInviterName.empty())
				{
					Unicode::String result;
					CuiStringVariablesManager::process (ClientStringIds::group_auto_invite_reject_prose, Unicode::emptyString, (inviter ? inviter->getLocalizedName () : Unicode::narrowToWide(groupInviterName)), Unicode::emptyString, result);
					CuiSystemMessageManager::sendFakeSystemMessage (result);
				}
				else
					CuiSystemMessageManager::sendFakeSystemMessage (ClientStringIds::group_auto_invite_reject.localize ());

				static const uint32 hash_group_decline = Crc::normalizeAndCalculate("decline");
				ClientCommandQueue::enqueueCommand (hash_group_decline, NetworkId::cms_invalid, Unicode::emptyString);
			}
			else
			{
				if (ms_consentId != ms_noConsentId)
					CuiConsentManager::dismissLocalConsentRequest (ms_consentId);

				ms_consentGroupInviter = id;

				ProsePackage p;
				p.stringId = ClientStringIds::group_invite;
				p.target.id = id;
				if(inviter)
					p.target.str = inviter->getObjectName();
				else if (!groupInviterName.empty())
					p.target.str = Unicode::narrowToWide(groupInviterName);
				else
					p.target.str = Unicode::emptyString;
				Unicode::String result;
				ProsePackageManagerClient::appendTranslation(p, result);
				ms_consentId = CuiConsentManager::askLocalConsent(result, &GroupManagerNamespace::inviteResponseHandler);
			}
		}
	}
}

//----------------------------------------------------------------------

/** This function handles a user's response to an invite request, only one can be pending.
    At this point a consent window should be open, this entering this function meant that a
		user used some other method to accept the invite (type /join, via radial menu, etc.)
*/
void GroupManager::handleInviteResponse (bool )
{
	//force the consent page closed
	if (ms_consentId != ms_noConsentId)
	{
		CuiConsentManager::dismissLocalConsentRequest (ms_consentId);
		ms_consentId = ms_noConsentId;
		ms_consentGroupInviter = NetworkId::cms_invalid;
	}
}

//----------------------------------------------------------------------

void GroupManager::handleInviteToLaunchIntoSpace(NetworkId const & inviterId)
{
	if (ms_spaceLaunchConsentId != ms_noConsentId)
	{
		CuiConsentManager::dismissLocalConsentRequest(ms_spaceLaunchConsentId);
	}

	if (ms_numberOfLaunchIntoSpaceUIOpen == 0)
	{
		ms_spaceLaunchConsentGroupInviter = inviterId;

		ClientObject const * const inviter = safe_cast<ClientObject *>(NetworkIdManager::getObjectById(inviterId));

		if (inviter != 0)
		{
			ProsePackage p;
			p.stringId = ClientStringIds::group_space_launch_invite;
			p.target.id = inviterId;
			p.target.str = inviter->getObjectName();

			Unicode::String result;
			ProsePackageManagerClient::appendTranslation(p, result);
			ms_spaceLaunchConsentId = CuiConsentManager::askLocalConsent(result, &GroupManagerNamespace::inviteToLaunchIntoSpaceResponseHandler);
		}
	}
	else
	{
		// send a cancel and then clear the data
		if (ms_currentSpaceLaunchConsentGroupInviter.isValid())
		{
			sendInvitationToLaunchIntoSpaceResponse(ms_currentSpaceLaunchConsentGroupInviter, false);
		}

		sendInvitationToLaunchIntoSpaceResponse(inviterId, false);

		CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::group_space_launch_cannot_be_passenger.localize());
	}
}

void GroupManager::openedLaunchIntoSpaceUI()
{
	++ms_numberOfLaunchIntoSpaceUIOpen;

	bool sendFakeSystemMessage = false;

	// send a cancel and then clear the data for previous results
	if (ms_currentSpaceLaunchConsentGroupInviter.isValid())
	{
		sendInvitationToLaunchIntoSpaceResponse(ms_currentSpaceLaunchConsentGroupInviter, false);
		sendFakeSystemMessage = true;
	}

	// consent dialog is opened but has not been answered yet
	if (ms_spaceLaunchConsentGroupInviter.isValid())
	{
		// send a cancel
		sendInvitationToLaunchIntoSpaceResponse(ms_spaceLaunchConsentGroupInviter, false);

		// close the dialog
		if (ms_spaceLaunchConsentId != ms_noConsentId)
		{
			CuiConsentManager::dismissLocalConsentRequest(ms_spaceLaunchConsentId);
		}

		sendFakeSystemMessage = true;
	}

	if (sendFakeSystemMessage)
	{
		CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::group_space_launch_cannot_be_passenger.localize());
	}

	ms_currentSpaceLaunchConsentGroupInviter = NetworkId::cms_invalid;
	ms_spaceLaunchConsentGroupInviter = NetworkId::cms_invalid;
	ms_spaceLaunchConsentId = ms_noConsentId;
}

void GroupManager::closedLaunchIntoSpaceUI()
{
	--ms_numberOfLaunchIntoSpaceUIOpen;
}


// ======================================================================

