//========================================================================
//
// PlayerObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerObject.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/CommunityManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/MatchMakingManager.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsAwayFromKeyBoard.h"
#include "clientUserInterface/CuiStringIdsGameLanguage.h"
#include "clientUserInterface/CuiStringIdsWho.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/CityData.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/SharedPlayerObjectTemplate.h"
#include "sharedGame/Waypoint.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedSkillSystem/SkillObject.h"
#include "swgSharedUtility/JediConstants.h"


#if PRODUCTION == 0
bool PlayerObject::s_admin = false;
#endif

namespace PlayerObjectNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const PlayerObject::Messages::WhoStatusMessage::Message &, PlayerObject::Messages::WhoStatusMessage> whoStatusMessage;
		MessageDispatch::Transceiver<const PlayerObject::Messages::WhoStatusChanged::Status &, PlayerObject::Messages::WhoStatusChanged> whoStatusChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::DraftSchematicsChanged::Payload &,  PlayerObject::Messages::DraftSchematicsChanged>  draftSchematicsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::DraftSchematicAdded::Payload &,     PlayerObject::Messages::DraftSchematicAdded>     draftSchematicAdded;
		MessageDispatch::Transceiver<const PlayerObject::Messages::DraftSchematicRemoved::Payload &,   PlayerObject::Messages::DraftSchematicRemoved>   draftSchematicRemoved;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CommandAdded::Payload &,            PlayerObject::Messages::CommandAdded>            commandAdded;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CommandRemoved::Payload &,          PlayerObject::Messages::CommandRemoved>          commandRemoved;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CommandsChanged::Payload &,         PlayerObject::Messages::CommandsChanged>         commandsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::ExperienceChanged::Payload &,       PlayerObject::Messages::ExperienceChanged>       experienceChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::WaypointsChanged::Payload &,        PlayerObject::Messages::WaypointsChanged>        waypointsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::SpokenLangaugeChanged::Language &, PlayerObject::Messages::SpokenLangaugeChanged> spokenLanguageChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::JediStateChanged::Payload &,        PlayerObject::Messages::JediStateChanged>        jediStateChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CompletedQuestsChanged::Payload &,  PlayerObject::Messages::CompletedQuestsChanged>  completedQuestsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::ActiveQuestsChanged::Payload &,     PlayerObject::Messages::ActiveQuestsChanged>     activeQuestsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::QuestsChanged::Payload &,           PlayerObject::Messages::QuestsChanged>           questsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CurrentQuestChanged::Payload &,     PlayerObject::Messages::CurrentQuestChanged>     currentQuestChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CraftingBioLinkChanged::Payload &,  PlayerObject::Messages::CraftingBioLinkChanged>  craftingBioLinkChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::RoleIconChoiceChanged::Payload &,   PlayerObject::Messages::RoleIconChoiceChanged>   roleIconChoiceChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::SkillTemplateChanged::Payload &,    PlayerObject::Messages::SkillTemplateChanged>    skillTemplateChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::WorkingSkillChanged::Payload &,     PlayerObject::Messages::WorkingSkillChanged>     workingSkillChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::PetChanged::Payload &,              PlayerObject::Messages::PetChanged>              petIdChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::PetCommandsChanged::Payload &,      PlayerObject::Messages::PetCommandsChanged>      petCommandsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::PetToggledCommandsChanged::Payload &, PlayerObject::Messages::PetToggledCommandsChanged> petToggledCommandsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::CollectionsChanged::Payload &,      PlayerObject::Messages::CollectionsChanged>      collectionsChanged;
		MessageDispatch::Transceiver<const PlayerObject::Messages::GalacticReserveDepositChanged::Payload &, PlayerObject::Messages::GalacticReserveDepositChanged> galacticReserveDepositChanged;
	}

	//-----------------------------------------------------------------------

	void updateCitizenshipTitle(std::string const & citizenshipCity, int8 citizenshipType, Unicode::String & citizenshipTitle)
	{
		if (citizenshipType == static_cast<int8>(CityDataCitizenType::Mayor))
		{
			citizenshipTitle = StringId("skl_t", "citizenship_mayor").localize();
			citizenshipTitle += Unicode::narrowToWide(citizenshipCity);
		}
		else if (citizenshipType == static_cast<int8>(CityDataCitizenType::Militia))
		{
			citizenshipTitle = StringId("skl_t", "citizenship_militia").localize();
			citizenshipTitle += Unicode::narrowToWide(citizenshipCity);
		}
		else if (citizenshipType == static_cast<int8>(CityDataCitizenType::Citizen))
		{
			citizenshipTitle = StringId("skl_t", "citizenship_citizen").localize();
			citizenshipTitle += Unicode::narrowToWide(citizenshipCity);
		}
		else
		{
			citizenshipTitle = StringId("skl_t", "citizenship_none").localize();
		}
	}

	//-----------------------------------------------------------------------

	void updateGcwRegionDefenderTitle(std::string const & gcwDefenderRegion, bool canUseTitle, Unicode::String & gcwRegionDefenderTitle)
	{
		if (gcwDefenderRegion.empty() || !canUseTitle)
		{
			gcwRegionDefenderTitle.clear();
		}
		else
		{
			gcwRegionDefenderTitle = StringId("skl_t", "gcw_region_defender").localize();
			gcwRegionDefenderTitle += StringId("gcw_regions", gcwDefenderRegion).localize();
		}
	}

	//-----------------------------------------------------------------------

	const std::string s_adminCommand = "admin";
	char const * const ms_debugInfoSectionName = "PlayerObject";

}

using namespace PlayerObjectNamespace;

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::SpokenLanguageChanged::modified(PlayerObject &target, const int &oldValue, const int &newValue, bool /* local */)
{
	UNREF(oldValue);

	if (&target == Game::getPlayerObject())
	{
		Unicode::String localizedName;
		GameLanguageManager::getLocalizedLanguageName(newValue, localizedName);

		CuiStringVariablesData data;
		data.targetName = localizedName;

		Unicode::String currentLanguageString;
		CuiStringVariablesManager::process(CuiStringIdsGameLanguage::language_current, data, currentLanguageString);

		CuiSystemMessageManager::sendFakeSystemMessage(currentLanguageString);

		Transceivers::spokenLanguageChanged.emitMessage(true);
	}
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::JediStateChanged::modified(PlayerObject &target, const int &oldValue, const int &/*newValue*/, bool /* local */)
{
	UNREF(oldValue);

	Transceivers::jediStateChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::PetChanged::modified(PlayerObject & target, const NetworkId &oldValue, const NetworkId &newValue, bool local) const
{
	UNREF(oldValue);
	UNREF(local);
	UNREF(newValue);
	Transceivers::petIdChanged.emitMessage(target);
}

//-----------------------------------------------------------------------

void PlayerObject::onPetCommandsChanged()
{
	Transceivers::petCommandsChanged.emitMessage(*this);
}

//-----------------------------------------------------------------------

void PlayerObject::onPetToggledCommandsChanged()
{
	Transceivers::petToggledCommandsChanged.emitMessage(*this);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::RoleIconChoiceChanged::modified(PlayerObject &target, const int &oldValue, const int &/*newValue*/, bool /* local */)
{
	UNREF(oldValue);

	Transceivers::roleIconChoiceChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::CraftingBioLinkChanged::modified(PlayerObject &target, const NetworkId &oldValue, const NetworkId &newValue, bool /* local */)
{
	UNREF(target);
	UNREF(oldValue);

	CuiCraftManager::setCraftingBioLinkedItem(newValue != NetworkId::cms_invalid);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::MatchMakingCharacterIdChanged::modified(PlayerObject &target, const MatchMakingId &oldValue, const MatchMakingId &newValue, bool /* local */)
{
	// Only emit these signals if this is the player object on this client.

	if ((&target == Game::getPlayerObject()) && !target.isSettingBaselines())
	{
		// Looking for group

		if (oldValue.isBitSet(MatchMakingId::B_lookingForGroup) != newValue.isBitSet(MatchMakingId::B_lookingForGroup))
		{
			if (newValue.isBitSet(MatchMakingId::B_lookingForGroup))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::looking_for_group_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::looking_for_group_false.localize());
			}
		}

		// Roleplayer

		if (oldValue.isBitSet(MatchMakingId::B_rolePlay) != newValue.isBitSet(MatchMakingId::B_rolePlay))
		{
			if (newValue.isBitSet(MatchMakingId::B_rolePlay))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::roleplay_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::roleplay_false.localize());
			}
		}

		// Newbie helper

		if (oldValue.isBitSet(MatchMakingId::B_helper) != newValue.isBitSet(MatchMakingId::B_helper))
		{
			if (newValue.isBitSet(MatchMakingId::B_helper))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::helper_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::helper_false.localize());
			}
		}

		// Anonymous

		if (oldValue.isBitSet(MatchMakingId::B_anonymous) != newValue.isBitSet(MatchMakingId::B_anonymous))
		{
			if (newValue.isBitSet(MatchMakingId::B_anonymous))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::anonymous_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::anonymous_false.localize());
			}
		}

		// Away from keyboard

		if (oldValue.isBitSet(MatchMakingId::B_awayFromKeyBoard) != newValue.isBitSet(MatchMakingId::B_awayFromKeyBoard))
		{
			if (newValue.isBitSet(MatchMakingId::B_awayFromKeyBoard))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_on.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsAwayFromKeyBoard::away_from_keyboard_off.localize());
			}
		}

		// Displaying faction rank

		if (oldValue.isBitSet(MatchMakingId::B_displayingFactionRank) != newValue.isBitSet(MatchMakingId::B_displayingFactionRank))
		{
			if (newValue.isBitSet(MatchMakingId::B_displayingFactionRank))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::displaying_faction_rank_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::displaying_faction_rank_false.localize());
			}
		}

		// Looking For Work
		if (oldValue.isBitSet(MatchMakingId::B_lookingForWork) != newValue.isBitSet(MatchMakingId::B_lookingForWork))
		{
			if (newValue.isBitSet(MatchMakingId::B_lookingForWork))
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::lfw_true.localize());
			}
			else
			{
				Transceivers::whoStatusMessage.emitMessage(CuiStringIdsWho::lfw_false.localize());
			}
		}


		// Let all interested parties know of the current who status

		Transceivers::whoStatusChanged.emitMessage(newValue);
	}
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::TitleChanged::modified(PlayerObject &target, const std::string &, const std::string &newValue, bool /* local */) const
{
	if (newValue.empty())
		target.m_localizedTitle.clear ();
	else if (newValue == "citizenship") // update citizenship title
		updateCitizenshipTitle(target.m_citizenshipCity.get(), target.m_citizenshipType.get(), target.m_localizedTitle);
	else if (newValue == "city_gcw_region_defender") // update city GCW Region Defender title
		updateGcwRegionDefenderTitle(target.m_cityGcwDefenderRegion.get().first, target.m_cityGcwDefenderRegion.get().second.second, target.m_localizedTitle);
	else if (newValue == "guild_gcw_region_defender") // update guild GCW Region Defender title
		updateGcwRegionDefenderTitle(target.m_guildGcwDefenderRegion.get().first, target.m_guildGcwDefenderRegion.get().second.second, target.m_localizedTitle);
	else
		CuiSkillManager::localizeSkillTitle (newValue, target.m_localizedTitle);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::PrivledgedTitleChanged::modified(PlayerObject &target, const int8&, const int8&, bool /* local */) const
{
	std::string const & title = target.getTitle();

	if (title.empty())
		target.m_localizedTitle.clear ();
	else if (title == "citizenship") // update citizenship title
		updateCitizenshipTitle(target.m_citizenshipCity.get(), target.m_citizenshipType.get(), target.m_localizedTitle);
	else if (title == "city_gcw_region_defender") // update city GCW Region Defender title
		updateGcwRegionDefenderTitle(target.m_cityGcwDefenderRegion.get().first, target.m_cityGcwDefenderRegion.get().second.second, target.m_localizedTitle);
	else if (title == "guild_gcw_region_defender") // update guild GCW Region Defender title
		updateGcwRegionDefenderTitle(target.m_guildGcwDefenderRegion.get().first, target.m_guildGcwDefenderRegion.get().second.second, target.m_localizedTitle);
	else
		CuiSkillManager::localizeSkillTitle (title, target.m_localizedTitle);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::SkillTemplateChanged::modified(PlayerObject & target, const std::string & /*oldValue*/, const std::string &newValue, bool /* local */) const
{
	if(&target == Game::getPlayerObject())
		CuiSkillManager::setSkillTemplate(newValue, false);
}

//
void PlayerObject::Callbacks::ShowBackpackChanged::modified(PlayerObject & target, const  bool & /*old*/, const  bool & value, bool /*isLocal*/) const
{
	target.shouldShowBackpack(value);
	if(&target == Game::getPlayerObject())
		CuiPreferences::setShowBackpack(value);
}

void PlayerObject::Callbacks::ShowHelmetChanged::modified(PlayerObject & target, const  bool & /*old*/, const  bool & value, bool /*isLocal*/) const
{
	target.shouldShowHelmet(value);
	if(&target == Game::getPlayerObject())
		CuiPreferences::setShowHelmet(value);
}
//----------------------------------------------------------------------

void PlayerObject::Callbacks::WorkingSkillChanged::modified(PlayerObject & /*target*/, const std::string & /*oldValue*/, const std::string &newValue, bool /* local */) const
{
	CuiSkillManager::setWorkingSkill(newValue, false);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::GcwRatingActualCalcTimeServerEpochChanged::modified(PlayerObject &target, const int32 & /*oldValue*/, const int32 &newValue, bool /* local */) const
{
	if (newValue > 0)
		target.m_gcwRatingActualCalcTimeClientEpoch = static_cast<time_t>(newValue) + GameNetwork::getServerEpochTimeDifference();
	else
		target.m_gcwRatingActualCalcTimeClientEpoch = 0;
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::CitizenshipCityChanged::modified(PlayerObject &target, const std::string &, const std::string &newValue, bool /* local */) const
{
	// update citizenship title
	if (target.getTitle() == "citizenship")
		updateCitizenshipTitle(newValue, target.m_citizenshipType.get(), target.m_localizedTitle);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::CitizenshipTypeChanged::modified(PlayerObject &target, const int8 &, const int8 &newValue, bool /* local */) const
{
	// update citizenship title
	if (target.getTitle() == "citizenship")
		updateCitizenshipTitle(target.m_citizenshipCity.get(), newValue, target.m_localizedTitle);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::CityGcwRegionDefenderChanged::modified(PlayerObject &target, const std::pair<std::string, std::pair<bool, bool> > &, const std::pair<std::string, std::pair<bool, bool> > &newValue, bool /* local */) const
{
	// update city GCW Region Defender title
	if (target.getTitle() == "city_gcw_region_defender")
		updateGcwRegionDefenderTitle(newValue.first, newValue.second.second, target.m_localizedTitle);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::GuildGcwRegionDefenderChanged::modified(PlayerObject &target, const std::pair<std::string, std::pair<bool, bool> > &, const std::pair<std::string, std::pair<bool, bool> > &newValue, bool /* local */) const
{
	// update city GCW Region Defender title
	if (target.getTitle() == "guild_gcw_region_defender")
		updateGcwRegionDefenderTitle(newValue.first, newValue.second.second, target.m_localizedTitle);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::GalacticReserveDepositChanged::modified(PlayerObject &target, const int8 &, const int8 &, bool /* local */) const
{
	Transceivers::galacticReserveDepositChanged.emitMessage(target);
}

//----------------------------------------------------------------------

void PlayerObject::Callbacks::SquelchExpireTimeServerEpochChanged::modified(PlayerObject &target, const int32 & /*oldValue*/, const int32 &newValue, bool /* local */) const
{
	if (newValue < 0)
		target.m_squelchExpireTimeClientEpoch = static_cast<time_t>(-1);
	else if (newValue > 0)
		target.m_squelchExpireTimeClientEpoch = static_cast<time_t>(newValue) + GameNetwork::getServerEpochTimeDifference();
	else
		target.m_squelchExpireTimeClientEpoch = static_cast<time_t>(0);
}

//-----------------------------------------------------------------------

PlayerObject::PlayerObject(const SharedPlayerObjectTemplate* newTemplate) :
IntangibleObject                (newTemplate),
m_commands                      (),
m_draftSchematics               (),
m_craftingComponentBioLink      (),
m_experiencePoints              (),
m_expModified                   (0),
m_maxForcePower                 (0),
m_forcePower                    (0),
m_craftingLevel                 (0),
m_experimentPoints              (8),
m_craftingStage                 (Crafting::CS_none),
m_craftingStation               (),
m_biography                     (),
m_requestedBiography            (false),
m_haveBiography                 (false),
m_localizedTitle                (),
m_matchMakingPersonalProfileId  (),
m_matchMakingCharacterProfileId (),
m_friendList                    (),
m_ignoreList                    (),
m_skillTitle                    (),
m_spokenLanguage                (),
m_waypoints                     (new Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>),
m_groupWaypoints                (new Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>),
m_bornDate                      (0),
m_playedTime                    (0),
m_lastPlayedTime                (0),
m_playedTimeAccum               (0),
m_food                          (0),
m_maxFood                       (0),
m_drink                         (0),
m_maxDrink                      (0),
m_meds                          (0),
m_maxMeds                       (0),
m_privledgedTitle               (static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer)),
m_lastSitOnAttemptUniqueId      (0),
m_completedQuests               (),
m_activeQuests                  (),
m_currentQuest                  (0),
m_quests                        (),
m_jediState                     (0),
m_roleIconChoice                (),
m_skillTemplate                 (),
m_workingSkill                  (),
m_currentGcwPoints              (0),
m_currentPvpKills               (0),
m_lifetimeGcwPoints             (0),
m_lifetimePvpKills              (0),
m_currentGcwRank                (0),
m_currentGcwRankProgress        (0.0f),
m_maxGcwImperialRank            (0),
m_maxGcwRebelRank               (0),
m_gcwRatingActualCalcTimeServerEpoch(0),
m_gcwRatingActualCalcTimeClientEpoch(0),
m_playerHateList                (),
m_killMeter                     (0),
m_accountNumLotsOverLimitSpam   (0),
m_petId                         (NetworkId::cms_invalid),
m_petCommandList                (0),
m_collections                   (),
m_collections2                  (),
m_citizenshipCity               (),
m_citizenshipType               (static_cast<int8>(CityDataCitizenType::NotACitizen)),
m_cityGcwDefenderRegion         (std::make_pair(std::string(), std::make_pair(false, false))),
m_guildGcwDefenderRegion        (std::make_pair(std::string(), std::make_pair(false, false))),
m_squelchedById                 (NetworkId::cms_invalid),
m_squelchedByName               (),
m_squelchExpireTimeServerEpoch  (0),
m_squelchExpireTimeClientEpoch  (static_cast<time_t>(0)),
m_showBackpack					(true),
m_showHelmet					(true),
m_environmentFlags              (0),
m_defaultAttackOverride         (),
m_guildRank                     (),
m_citizenRank                   (),
m_galacticReserveDeposit        (0),
m_pgcRatingCount                (0),
m_pgcRatingTotal                (0),
m_pgcLastRatingTime             (0)
{
	m_allCollections[0] = &m_collections;
	m_allCollections[1] = &m_collections2;

	addFirstParentClientServerVariable_np(m_craftingLevel);
	addFirstParentClientServerVariable_np(m_craftingStage);
	addFirstParentClientServerVariable_np(m_craftingStation);
	addFirstParentClientServerVariable_np(m_draftSchematics);
	addFirstParentClientServerVariable_np(m_craftingComponentBioLink);
	addFirstParentClientServerVariable_np(m_experimentPoints);
	addFirstParentClientServerVariable_np(m_expModified);
	addFirstParentClientServerVariable_np(m_friendList);
	addFirstParentClientServerVariable_np(m_ignoreList);
	addFirstParentClientServerVariable_np(m_spokenLanguage);
	addFirstParentClientServerVariable_np(m_food);
	addFirstParentClientServerVariable_np(m_maxFood);
	addFirstParentClientServerVariable_np(m_drink);
	addFirstParentClientServerVariable_np(m_maxDrink);
	addFirstParentClientServerVariable_np(m_meds);
	addFirstParentClientServerVariable_np(m_maxMeds);
	addFirstParentClientServerVariable_np(*m_groupWaypoints);
	addFirstParentClientServerVariable_np(m_playerHateList);
	addFirstParentClientServerVariable_np(m_killMeter);
	addFirstParentClientServerVariable_np(m_accountNumLotsOverLimitSpam);
	addFirstParentClientServerVariable_np(m_petId);
	addFirstParentClientServerVariable_np(m_petCommandList);
	addFirstParentClientServerVariable_np(m_petToggledCommands);
	addFirstParentClientServerVariable_np(m_guildRank);
	addFirstParentClientServerVariable_np(m_citizenRank);
	addFirstParentClientServerVariable_np(m_galacticReserveDeposit);
	addFirstParentClientServerVariable_np(m_pgcRatingCount);
	addFirstParentClientServerVariable_np(m_pgcRatingTotal);
	addFirstParentClientServerVariable_np(m_pgcLastRatingTime);

	// DO NOT ADD ANY PACKAGEDATA.TXT ENTRIES AFTER JEDI STATE!
	addFirstParentClientServerVariable_np(m_jediState);

	addFirstParentClientServerVariable   (m_experiencePoints);
	addFirstParentClientServerVariable   (*m_waypoints);
	addFirstParentClientServerVariable   (m_forcePower);
	addFirstParentClientServerVariable   (m_maxForcePower);
	addFirstParentClientServerVariable   (m_completedQuests);
	addFirstParentClientServerVariable   (m_activeQuests);
	addFirstParentClientServerVariable   (m_currentQuest);
	addFirstParentClientServerVariable   (m_quests);

	addSharedVariable                    (m_matchMakingCharacterProfileId);
	addSharedVariable                    (m_matchMakingPersonalProfileId);
	addSharedVariable                    (m_skillTitle);
	addSharedVariable                    (m_bornDate);
	addSharedVariable                    (m_playedTime);
	addSharedVariable_np                 (m_privledgedTitle);
	addSharedVariable                    (m_roleIconChoice);

	addSharedVariable					 (m_skillTemplate);
	addFirstParentClientServerVariable   (m_workingSkill);

	addSharedVariable                    (m_currentGcwPoints);
	addSharedVariable                    (m_currentPvpKills);
	addSharedVariable                    (m_lifetimeGcwPoints);
	addSharedVariable                    (m_lifetimePvpKills);
	addSharedVariable_np                 (m_currentGcwRank);
	addSharedVariable_np                 (m_currentGcwRankProgress);
	addSharedVariable_np                 (m_maxGcwImperialRank);
	addSharedVariable_np                 (m_maxGcwRebelRank);
	addSharedVariable_np                 (m_gcwRatingActualCalcTimeServerEpoch);

	addSharedVariable                    (m_collections);
	addSharedVariable                    (m_collections2);

	addSharedVariable_np                 (m_citizenshipCity);
	addSharedVariable_np                 (m_citizenshipType);

	addSharedVariable_np                 (m_cityGcwDefenderRegion);
	addSharedVariable_np                 (m_guildGcwDefenderRegion);

	addSharedVariable_np                 (m_squelchedById);
	addSharedVariable_np                 (m_squelchedByName);
	addSharedVariable_np                 (m_squelchExpireTimeServerEpoch);
	addSharedVariable_np                 (m_environmentFlags);
	addSharedVariable_np                 (m_defaultAttackOverride);

	addSharedVariable				(m_showBackpack);
	addSharedVariable				(m_showHelmet);

	m_draftSchematics.setOnInsert                    (this, &PlayerObject::draftSchematicsOnInit);

	m_skillTitle.setSourceObject                     (this);
	m_privledgedTitle.setSourceObject(this);
	m_roleIconChoice.setSourceObject                 (this);

	m_skillTemplate.setSourceObject(this);
	m_workingSkill.setSourceObject(this);

	m_citizenshipCity.setSourceObject(this);
	m_citizenshipType.setSourceObject(this);

	m_cityGcwDefenderRegion.setSourceObject(this);
	m_guildGcwDefenderRegion.setSourceObject(this);

	m_gcwRatingActualCalcTimeServerEpoch.setSourceObject(this);
	m_squelchExpireTimeServerEpoch.setSourceObject(this);
}

//-----------------------------------------------------------------------

PlayerObject::~PlayerObject()
{
	delete m_groupWaypoints;
	m_groupWaypoints = 0;
	delete m_waypoints;
	m_waypoints = 0;
}

//----------------------------------------------------------------------

void PlayerObject::setPlayerSpecificCallbacks ()
{
	// technically, only the authoritative PlayerObject needs to be
	// notified when XP changes, but we won't try to be smart here,
	// and take the simple approach, and just have all PlayerObject
	// be notified when their XP changes; the server will only send
	// XP changes to the authoritative PlayerObject anyway; and the
	// UI component that updates the display on XP changes will only
	// do it if the XP change occurs on the authoritative PlayerObject
	m_experiencePoints.setOnErase                    (this, &PlayerObject::expOnErase);
	m_experiencePoints.setOnInsert                   (this, &PlayerObject::expOnInsert);
	m_experiencePoints.setOnSet                      (this, &PlayerObject::expOnSet);
	m_expModified.setSourceObject                    (this);

	m_showBackpack.setSourceObject					 (this);
	m_showHelmet.setSourceObject					 (this);

	if (this != Game::getPlayerObject ())
		return;

	m_draftSchematics.setOnInsert                    (this, &PlayerObject::draftSchematicsOnInsert);
	m_draftSchematics.setOnErase                     (this, &PlayerObject::draftSchematicsOnErase);
	m_friendList.setOnChanged                        (this, &PlayerObject::onFriendListChanged);
	m_ignoreList.setOnChanged                        (this, &PlayerObject::onIgnoreListChanged);
	m_spokenLanguage.setSourceObject                 (this);
	m_waypoints->setOnSet                            (this, &PlayerObject::waypointsOnSet);
	m_waypoints->setOnInsert                         (this, &PlayerObject::waypointsOnInsert);
	m_waypoints->setOnErase                          (this, &PlayerObject::waypointsOnErase);
	m_groupWaypoints->setOnSet                       (this, &PlayerObject::groupWaypointsOnSet);
	m_groupWaypoints->setOnInsert                    (this, &PlayerObject::groupWaypointsOnInsert);
	m_groupWaypoints->setOnErase                     (this, &PlayerObject::groupWaypointsOnErase);
	m_matchMakingCharacterProfileId.setSourceObject  (this);
	m_jediState.setSourceObject                      (this);
	m_completedQuests.setSourceObject                (this);
	m_activeQuests.setSourceObject                   (this);
	m_craftingComponentBioLink.setSourceObject       (this);
	m_currentQuest.setSourceObject                   (this);
	m_petId.setSourceObject                          (this);
	m_petCommandList.setOnChanged                    (this, &PlayerObject::onPetCommandsChanged);
	m_petToggledCommands.setOnChanged                (this, &PlayerObject::onPetToggledCommandsChanged);
	m_quests.setOnChanged                            (this, &PlayerObject::questsOnChanged);
	m_collections.setSourceObject                    (this);
	m_collections2.setSourceObject                   (this);
	m_galacticReserveDeposit.setSourceObject         (this);

	shouldShowHelmet(m_showHelmet.get());
	shouldShowBackpack(m_showBackpack.get());

	CuiPreferences::setShowBackpack(m_showBackpack.get());
	CuiPreferences::setShowHelmet(m_showHelmet.get());
}

//-----------------------------------------------------------------------

void PlayerObject::waypointsOnSet(const NetworkId & key, const Waypoint & oldValue, const Waypoint & newValue)
{
	UNREF(key);
	ClientWaypointObject * const wp = dynamic_cast<ClientWaypointObject*>(NetworkIdManager::getObjectById(oldValue.getNetworkId()));
	if(wp)
	{
		*wp = newValue;
		Transceivers::waypointsChanged.emitMessage (*this);
	}
	else
	{
		DEBUG_WARNING(true, ("Could not get the ClientWaypointObject in waypointsOnSet"));
	}
}

//-----------------------------------------------------------------------

void PlayerObject::waypointsOnInsert(const NetworkId & key, const Waypoint & newValue)
{
	UNREF(key);

	CreatureObject * const creaturePlayer = Game::getPlayerCreature();
	if (creaturePlayer)
	{
		ClientObject * const datapad = creaturePlayer->getDatapadObject();
		if (datapad)
		{
			VolumeContainer * const vc = ContainerInterface::getVolumeContainer(*datapad);
			if (vc)
			{
				ClientWaypointObject * const newWp = ClientWaypointObject::createClientWaypoint(newValue);
				if (!newWp)
				{
					DEBUG_WARNING(true, ("Could not create ClientWaypointObject in waypointsOnInsert"));
					return;
				}
				int const oldCapacity = vc->debugDoNotUseSetCapacity(-1);
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				const bool res = vc->add(*newWp, tmp, true);
				vc->debugDoNotUseSetCapacity(oldCapacity);
				vc->recalculateVolume();

				if(res)
				{
					Transceivers::waypointsChanged.emitMessage (*this);
					return;
				}
				else
				{
					DEBUG_WARNING(true, ("ClientWaypointObject transfer to Datapad's VolumeContainer failed in waypointsOnInsert"));
					delete newWp;
				}
			}
			else
			{
				DEBUG_WARNING(true, ("Could not get the Datapad's VolumeContainer in waypointsOnInsert"));
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Could not get the Player in waypointsOnInsert"));
	}
}

//-----------------------------------------------------------------------

void PlayerObject::waypointsOnErase(const NetworkId & key, const Waypoint & value)
{
	UNREF(key);
	ClientWaypointObject * wp = dynamic_cast<ClientWaypointObject*>(NetworkIdManager::getObjectById(value.getNetworkId()));
	if(wp)
	{
		CreatureObject* const creaturePlayer = Game::getPlayerCreature ();
		if (creaturePlayer)
		{
			ClientObject* const datapad = creaturePlayer->getDatapadObject();
			if(datapad)
			{
				VolumeContainer* const vc = ContainerInterface::getVolumeContainer(*datapad);
				if(vc)
				{
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					const bool res = vc->remove(*wp, tmp);
					DEBUG_WARNING(!res, ("ClientWaypointObject transfer from Datapad's VolumeContainer failed in waypointsOnErase"));
					UNREF(res); // for release build
					delete wp;
					wp = 0;
					Transceivers::waypointsChanged.emitMessage (*this);
				}
				else
				{
					DEBUG_WARNING(true, ("Could not get the Datapad's VolumeContainer in waypointsOnErase"));
				}
			}
			else
			{
				DEBUG_WARNING(true, ("Could not get the Datapad in waypointsOnErase"));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Could not get the Player's CreatureObject in waypointsOnErase"));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Could not get the ClientWaypointObject in waypointsOnErase"));
	}
}

//-----------------------------------------------------------------------

void PlayerObject::groupWaypointsOnSet(NetworkId const &key, Waypoint const &oldValue, Waypoint const &newValue)
{
	if (m_waypoints->find(key) == m_waypoints->end())
	{
		ClientWaypointObject * const wp = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById(oldValue.getNetworkId()));
		if (wp)
		{
			*wp = newValue;
			Transceivers::waypointsChanged.emitMessage(*this);
		}
	}
}

//-----------------------------------------------------------------------

void PlayerObject::groupWaypointsOnInsert(NetworkId const &key, Waypoint const &value)
{
	if (m_waypoints->find(key) == m_waypoints->end())
	{
		CreatureObject * const creaturePlayer = Game::getPlayerCreature();
		if (creaturePlayer)
		{
			ClientObject * const datapad = creaturePlayer->getDatapadObject();
			if (datapad)
			{
				VolumeContainer * const vc = ContainerInterface::getVolumeContainer(*datapad);
				if (vc)
				{
					ClientWaypointObject * const newWp = ClientWaypointObject::createClientWaypoint(value, true);
					if (newWp)
					{
						int const oldCapacity = vc->debugDoNotUseSetCapacity(-1);
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						if (vc->add(*newWp, tmp))
							Transceivers::waypointsChanged.emitMessage(*this);
						else
							delete newWp;
						vc->debugDoNotUseSetCapacity(oldCapacity);
						vc->recalculateVolume();
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void PlayerObject::groupWaypointsOnErase(NetworkId const &key, Waypoint const &value)
{
	if (m_waypoints->find(key) == m_waypoints->end())
	{
		ClientWaypointObject * wp = dynamic_cast<ClientWaypointObject *>(NetworkIdManager::getObjectById(value.getNetworkId()));
		if (wp)
		{
			CreatureObject * const creaturePlayer = Game::getPlayerCreature ();
			if (creaturePlayer)
			{
				ClientObject * const datapad = creaturePlayer->getDatapadObject();
				if (datapad)
				{
					VolumeContainer * const vc = ContainerInterface::getVolumeContainer(*datapad);
					if (vc)
					{
						Container::ContainerErrorCode tmp = Container::CEC_Success;
						IGNORE_RETURN(vc->remove(*wp, tmp));
						delete wp;
						wp = 0;
						Transceivers::waypointsChanged.emitMessage(*this);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void PlayerObject::commandsOnErase(const std::string & cmd, const int & /*value*/)
{
#if PRODUCTION == 0
	if (boolEqual(s_admin, true) && cmd == s_adminCommand)
		s_admin = false;
#endif
	Transceivers::commandsChanged.emitMessage (*this);

	Transceivers::commandRemoved.emitMessage (Messages::CommandRemoved::Payload (this, cmd));
}

//-----------------------------------------------------------------------

void PlayerObject::commandsOnInsert(const std::string & cmd, const int & /*value*/)
{
#if PRODUCTION == 0
	if (boolEqual(s_admin, false) && cmd == s_adminCommand)
		s_admin = true;
#endif
	Transceivers::commandsChanged.emitMessage (*this);

	Transceivers::commandAdded.emitMessage (Messages::CommandAdded::Payload (this, cmd));
}

//-----------------------------------------------------------------------

void PlayerObject::draftSchematicsOnErase(const std::pair<uint32, uint32> & schematicCrc, const int & /*count*/)
{
	Transceivers::draftSchematicRemoved.emitMessage (Messages::DraftSchematicRemoved::Payload (this, schematicCrc));
}

//-----------------------------------------------------------------------

void PlayerObject::draftSchematicsOnInsert(const std::pair<uint32, uint32> & schematicCrc, const int & /*count*/)
{
	Transceivers::draftSchematicAdded.emitMessage (Messages::DraftSchematicAdded::Payload (this, schematicCrc));
}

//-----------------------------------------------------------------------

void PlayerObject::draftSchematicsOnInit(const std::pair<uint32, uint32> & /*schematicCrc*/, const int & /*count*/)
{
	Transceivers::draftSchematicsChanged.emitMessage (*this);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::ExperienceChanged::modified(PlayerObject & target, const int &, const int & , bool) const
{
	Transceivers::experienceChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::CompletedQuestsChanged::modified(PlayerObject & target, const BitArray &, const BitArray & , bool) const
{
	Transceivers::completedQuestsChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::ActiveQuestsChanged::modified(PlayerObject & target, const BitArray &, const BitArray & , bool) const
{
	Transceivers::activeQuestsChanged.emitMessage (target);
}

//-----------------------------------------------------------------------

void PlayerObject::questsOnChanged()
{
	Transceivers::questsChanged.emitMessage (*this);
}

//-----------------------------------------------------------------------

void PlayerObject::Callbacks::CurrentQuestChanged::modified(PlayerObject & target, const uint32 &, const uint32 &, bool) const
{
	Transceivers::currentQuestChanged.emitMessage(target);
}

//-----------------------------------------------------------------------

void PlayerObject::expOnSet(const std::string & , const int & , const int & )
{
	Transceivers::experienceChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void PlayerObject::expOnInsert(const std::string & , const int & )
{
	Transceivers::experienceChanged.emitMessage (*this);
}

//----------------------------------------------------------------------

void PlayerObject::expOnErase(const std::string & , const int & )
{
	Transceivers::experienceChanged.emitMessage (*this);
}

//-----------------------------------------------------------------------

void PlayerObject::clientGrantSchematic (uint32 serverCrc, uint32 sharedCrc)
{
	std::pair<uint32,uint32> crc = std::make_pair(serverCrc, sharedCrc);
	std::map<std::pair<uint32,uint32>,int>::const_iterator found = m_draftSchematics.find(crc);
	if (found != m_draftSchematics.end())
		m_draftSchematics.set(crc, (*found).second + 1);
	else
		m_draftSchematics.set(crc, 1);
}

//-----------------------------------------------------------------------

void PlayerObject::clientRevokeSchematic(uint32 serverCrc, uint32 sharedCrc)
{
	std::pair<uint32,uint32> crc = std::make_pair(serverCrc, sharedCrc);
	std::map<std::pair<uint32,uint32>,int>::const_iterator found = m_draftSchematics.find(crc);
	if (found != m_draftSchematics.end())
	{
		if ((*found).second > 1)
			m_draftSchematics.set(crc, (*found).second - 1);
		else
			m_draftSchematics.erase(found);
	}
}

//-----------------------------------------------------------------------

void PlayerObject::clientGrantExp(const std::string & exptype, int amount)
{
	m_experiencePoints.set (exptype, amount);
}

//-----------------------------------------------------------------------

bool PlayerObject::getExperience (const std::string & expName, int & result) const
{
	const ExperiencePointMap::const_iterator it = m_experiencePoints.find (expName);

	if (it != m_experiencePoints.end ())
	{
		result = (*it).second;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

void PlayerObject::onFriendListChanged()
{
	if (this != Game::getPlayerObject())
	{
		return;
	}

	CommunityManager::UnicodeStringSet unicodeStringSet;

	StringVector::const_iterator iterFriendList = m_friendList.begin();

	for (; iterFriendList != m_friendList.end(); ++iterFriendList)
	{
		unicodeStringSet.insert(Unicode::narrowToWide(*iterFriendList));
	}

	CommunityManager::setFriendList(unicodeStringSet);
}

// ----------------------------------------------------------------------

void PlayerObject::onIgnoreListChanged()
{
	if (this != Game::getPlayerObject())
	{
		return;
	}

	CommunityManager::UnicodeStringSet unicodeStringList;

	StringVector::const_iterator iterIgnoreList = m_ignoreList.begin();

	for (; iterIgnoreList != m_ignoreList.end(); ++iterIgnoreList)
	{
		unicodeStringList.insert(Unicode::narrowToWide(*iterIgnoreList));
	}

	CommunityManager::setIgnoreList(unicodeStringList);
}

// ----------------------------------------------------------------------

void PlayerObject::requestBiography() const
{
	if (Game::getSinglePlayer())
		return;

	static uint32 const hash = Crc::normalizeAndCalculate("requestBiography");

	//only request our own biography once
	if(this == Game::getPlayerObject())
	{
		if (!m_requestedBiography)
		{
			m_requestedBiography = true;
			const CreatureObject * const containedBy = safe_cast<const CreatureObject* const>(ContainerInterface::getContainedByObject (*this));
			if(containedBy)
			{
				IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, containedBy->getNetworkId(), Unicode::emptyString));
			}
		}
	}
	//always request others' bio
	else
	{
		const CreatureObject * const containedBy = safe_cast<const CreatureObject* const>(ContainerInterface::getContainedByObject (*this));
		if(containedBy)
		{
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, containedBy->getNetworkId(), Unicode::emptyString));
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::setBiography(Unicode::String const &biography, bool serverSync)
{
	if (Game::getSinglePlayer())
		return;

	if (serverSync &&
	    (m_biography != biography))
	{
		// Update the biography on the server
		static uint32 const hash = Crc::normalizeAndCalculate("setBiography");
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, biography));
	}

	m_biography = biography;
}

// ----------------------------------------------------------------------

Unicode::String const &PlayerObject::getBiography() const
{
	return m_biography;
}

// ----------------------------------------------------------------------

bool PlayerObject::haveBiography() const
{
	return m_haveBiography;
}

// ----------------------------------------------------------------------

void PlayerObject::setHaveBiography(bool haveBiography)
{
	m_haveBiography = haveBiography;
}

//----------------------------------------------------------------------

MatchMakingId const &PlayerObject::getMatchMakingPersonalId() const
{
	return m_matchMakingPersonalProfileId.get();
}

//----------------------------------------------------------------------

MatchMakingId const &PlayerObject::getMatchMakingCharacterProfileId() const
{
	return m_matchMakingCharacterProfileId.get();
}

// ----------------------------------------------------------------------

void PlayerObject::toggleSearchableByCtsSourceGalaxy() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("searchableByCtsSourceGalaxy");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

void PlayerObject::toggleDisplayLocationInSearchResults() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("displayLocationInSearchResults");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

void PlayerObject::toggleAnonymous() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("anon");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isSearchableByCtsSourceGalaxy() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_searchableByCtsSourceGalaxy);
}

// ----------------------------------------------------------------------

bool PlayerObject::isDisplayLocationInSearchResults() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_displayLocationInSearchResults);
}

// ----------------------------------------------------------------------

bool PlayerObject::isAnonymous() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_anonymous);
}

// ----------------------------------------------------------------------

void PlayerObject::toggleRolePlay() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("rolePlay");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isRolePlay() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_rolePlay);
}

// ----------------------------------------------------------------------

void PlayerObject::toggleHelper() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("newbiehelper");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isHelper() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_helper);
}

// ----------------------------------------------------------------------

void PlayerObject::toggleLookingForGroup() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("lfg");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isLookingForGroup() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_lookingForGroup);
}

//-----------------------------------------------------------------------------
void PlayerObject::toggleAwayFromKeyBoard () const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject() ||
		!Game::isClient())
	{
		return;
	}

	uint32 const hash = Crc::normalizeAndCalculate("toggleAwayFromKeyBoard");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

//-----------------------------------------------------------------------------
bool PlayerObject::isAwayFromKeyBoard() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_awayFromKeyBoard);
}

//-----------------------------------------------------------------------------
void PlayerObject::toggleDisplayingFactionRank () const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject() ||
	    !Game::isClient())
	{
		return;
	}

	uint32 const hash = Crc::normalizeAndCalculate("toggleDisplayingFactionRank");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

//-----------------------------------------------------------------------------
bool PlayerObject::isDisplayingFactionRank() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_displayingFactionRank);
}

//-----------------------------------------------------------------------------
bool PlayerObject::isLinkDead() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_linkDead);
}

//-----------------------------------------------------------------------------
void PlayerObject::setTitle(std::string const &title)
{
	if (Game::getSinglePlayer())
		return;

	if (getTitle() != title)
	{
		static uint32 const hash = Crc::normalizeAndCalculate("setCurrentSkillTitle");
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::narrowToWide(title)));
	}
}

//-----------------------------------------------------------------------------
std::string const &PlayerObject::getTitle() const
{
	return m_skillTitle.get();
}

//----------------------------------------------------------------------

void PlayerObject::setSpokenLanguage(int const language)
{
	if (Game::getSinglePlayer())
		return;

	if (GameLanguageManager::isLanguageValid(language) &&
	    (getSpokenLanguage() != language))
	{
		static uint32 const hash = Crc::normalizeAndCalculate("setSpokenLanguage");

		char text[256];
		snprintf(text, sizeof(text), "%d", language);
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::narrowToWide(text)));
	}
}

//----------------------------------------------------------------------

int PlayerObject::getSpokenLanguage() const
{
	return m_spokenLanguage.get();
}

//----------------------------------------------------------------------

bool PlayerObject::speaksLanguage(int const languageId) const
{
	bool result = false;

	if (this == Game::getPlayerObject())
	{
		std::string skillModName;
		GameLanguageManager::getLanguageSpeakSkillModName(languageId, skillModName);

		CreatureObject * const creatureObject = Game::getPlayerCreature ();

		if (creatureObject != NULL)
		{
			CreatureObject::SkillModMap::const_iterator iterSkillModMap = creatureObject->getSkillModMap().find(skillModName);

			if (iterSkillModMap != creatureObject->getSkillModMap().end())
			{
				int const baseValue = iterSkillModMap->second.first;
				int const modValue = iterSkillModMap->second.second;

				if ((baseValue + modValue) > 0)
				{
					result = true;
				}
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

bool PlayerObject::comprehendsLanguage(int const languageId) const
{
	bool result = false;

	if (this == Game::getPlayerObject())
	{
		std::string skillModName;
		GameLanguageManager::getLanguageComprehendSkillModName(languageId, skillModName);

		CreatureObject * const creatureObject = Game::getPlayerCreature ();

		if (creatureObject != NULL)
		{
			CreatureObject::SkillModMap::const_iterator iterSkillModMap = creatureObject->getSkillModMap().find(skillModName);

			if (iterSkillModMap == creatureObject->getSkillModMap().end())
			{
				// See if the player has the special modifier which grants all languages

				iterSkillModMap = creatureObject->getSkillModMap().find("language_all_comprehend");
			}

			if (iterSkillModMap != creatureObject->getSkillModMap().end())
			{
				int const baseValue = iterSkillModMap->second.first;
				int const modValue = iterSkillModMap->second.second;

				if ((baseValue + modValue) > 0)
				{
					result = true;
				}
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

void PlayerObject::endBaselines()
{
	IntangibleObject::endBaselines();

	//-- disable the onChanged callback after endBaselines
//	m_draftSchematics.setOnChanged (0, 0);

	setPlayerSpecificCallbacks ();
	
}

//----------------------------------------------------------------------

void PlayerObject::applyDeferredWaypointCreation()
{
	// Make sure that all waypoints have been created
	CreatureObject * const creaturePlayer = Game::getPlayerCreature();
	if (creaturePlayer)
	{
		ClientObject * const datapad = creaturePlayer->getDatapadObject();
		if (datapad)
		{
			VolumeContainer * const vc = ContainerInterface::getVolumeContainer(*datapad);
			if (vc)
			{
				int const oldCapacity = vc->debugDoNotUseSetCapacity(-1);

				// For each waypoint that we have, make sure that it lives in the datapad
				{
					std::map<NetworkId, Waypoint> const &m = m_waypoints->getMap();
					for (std::map<NetworkId, Waypoint>::const_iterator i = m.begin(); i != m.end(); ++i)
					{
						NetworkId wpId = i->first;
						Waypoint  wp   = i->second;
						bool found = false;
						for (ContainerIterator j = vc->begin(); j != vc->end(); ++j)
						{
							CachedNetworkId cnid = *j;
							ClientObject * const o = NetworkIdManager::getObjectById(cnid)->asClientObject();
							if (o && o->getNetworkId() == wpId)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							ClientWaypointObject * const newWp = ClientWaypointObject::createClientWaypoint(wp);
							if (!newWp)
							{
								DEBUG_WARNING(true, ("Could not create ClientWaypointObject in waypointsOnInsert"));
								return;
							}
							Container::ContainerErrorCode tmp = Container::CEC_Success;
							if (vc->add(*newWp, tmp))
								Transceivers::waypointsChanged.emitMessage (*this);
							else
								delete newWp;
						}
					}
				}

				// For each group waypoint that we have, make sure that it lives in the datapad
				{
					std::map<NetworkId, Waypoint> const &m = m_groupWaypoints->getMap();
					for(std::map<NetworkId, Waypoint>::const_iterator i = m.begin(); i != m.end(); ++i)
					{
						NetworkId wpId = i->first;
						Waypoint  wp   = i->second;
						bool found = false;
						for (ContainerIterator j = vc->begin(); j != vc->end(); ++j)
						{
							CachedNetworkId cnid = *j;
							ClientObject * const o = NetworkIdManager::getObjectById(cnid)->asClientObject();
							if (o && o->getNetworkId() == wpId)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							ClientWaypointObject * const newWp = ClientWaypointObject::createClientWaypoint(wp, true);
							if (!newWp)
							{
								DEBUG_WARNING(true, ("Could not create ClientWaypointObject in waypointsOnInsert"));
								return;
							}
							Container::ContainerErrorCode tmp = Container::CEC_Success;
							if (vc->add(*newWp, tmp))
								Transceivers::waypointsChanged.emitMessage(*this);
							else
								delete newWp;
						}
					}
				}
				vc->debugDoNotUseSetCapacity(oldCapacity);
				vc->recalculateVolume();
			}
			else
			{
				DEBUG_WARNING(true, ("Could not get the Datapad's VolumeContainer in waypointsOnInsert"));
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Could not get the Player in waypointsOnInsert"));
	}
}

//----------------------------------------------------------------------

#ifdef _DEBUG
void PlayerObject::showMatchMakingStatistics()
{
	DEBUG_REPORT_PRINT(true, ("** Match Making **\n\n"));

	CreatureObject * const playerCreatureObject = Game::getPlayerCreature ();

	if ((playerCreatureObject != NULL) &&
	    (playerCreatureObject->getPlayerObject() != NULL))
	{
		std::string playerNetworkStringId(playerCreatureObject->getNetworkId().getValueString());
		std::string playerMatchMakingStringId(MatchMakingManager::getPreferenceId().getDebugIntString());

		DEBUG_REPORT_PRINT(true, ("Local Player:\n"));
		DEBUG_REPORT_PRINT(true, ("netId %s - preference %s\n\n", playerNetworkStringId.c_str(), playerMatchMakingStringId.c_str()));

		// Display all the matchmaking ids for the players in the world

		int const objectCount = ClientWorld::getNumberOfObjects(static_cast<int>(WOL_Tangible));

		DEBUG_REPORT_PRINT(true, ("Other Players:\n"));

		for (int i = 0; i < objectCount; ++i)
		{
			Object *object = ClientWorld::getObject(static_cast<int>(WOL_Tangible), i);
			CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(object);

			if ((creatureObject != NULL) &&
			    (creatureObject->getPlayerObject() != NULL))
			{
				int const personalPercent = static_cast<int>(100.0f * MatchMakingManager::getPreferenceId().getMatchPercent(creatureObject->getPlayerObject()->getMatchMakingPersonalId()));

				std::string currentNetworkStringId(creatureObject->getNetworkId().getValueString());
				std::string currentMatchMakingPersonalStringId(creatureObject->getPlayerObject()->getMatchMakingPersonalId().getDebugIntString());
				std::string currentMatchMakingCharacterProfileStringId(creatureObject->getPlayerObject()->getMatchMakingCharacterProfileId().getDebugIntString());

				DEBUG_REPORT_PRINT(true, ("netId %s personal  %3d%% (%s)\n", currentNetworkStringId.c_str(), personalPercent, currentMatchMakingPersonalStringId.c_str()));
				DEBUG_REPORT_PRINT(true, ("netId %s character %3d%% (%s)\n", currentNetworkStringId.c_str(), personalPercent, currentMatchMakingCharacterProfileStringId.c_str()));
			}
		}
	}
}
#endif // _DEBUG

//----------------------------------------------------------------------

void PlayerObject::setLastSitOnAttemptUniqueId (uint32 id)
{
	m_lastSitOnAttemptUniqueId = id;
}

//----------------------------------------------------------------------

int PlayerObject::getBornDate () const
{
	return m_bornDate.get();
}

//----------------------------------------------------------------------

uint32 PlayerObject::getPlayedTime () const
{
	return m_playedTime.get() + static_cast<uint32>(m_playedTimeAccum);
}

//----------------------------------------------------------------------

void PlayerObject::clientSetMaxForcePower  (int value)
{
	m_maxForcePower = value;
}

//----------------------------------------------------------------------

void PlayerObject::clientSetForcePower     (int value)
{
	m_forcePower = value;
}

//----------------------------------------------------------------------

int PlayerObject::getFood() const
{
	return m_food.get();
}

//----------------------------------------------------------------------

int PlayerObject::getMaxFood() const
{
	return m_maxFood.get();
}

//----------------------------------------------------------------------

int PlayerObject::getDrink() const
{
	return m_drink.get();
}

//----------------------------------------------------------------------

int PlayerObject::getMaxDrink() const
{
	return m_maxDrink.get();
}

//----------------------------------------------------------------------

int PlayerObject::getMeds() const
{
	return m_meds.get();
}

//----------------------------------------------------------------------

int PlayerObject::getMaxMeds() const
{
	return m_maxMeds.get();
}

//----------------------------------------------------------------------

bool PlayerObject::isJedi() const
{
	int state = getJediState();
	if(state == JS_jedi || state == JS_forceRankedLight || state == JS_forceRankedDark)
	{
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

bool PlayerObject::isQuestActive(int questId) const
{
	return m_activeQuests.get().testBit(questId);
}

// ----------------------------------------------------------------------

bool PlayerObject::isQuestComplete(int questId) const
{
	return m_completedQuests.get().testBit(questId);
}

// ----------------------------------------------------------------------

/** Get the PlayerQuestData object for the given questId, or NULL if an invalid questId
 */
PlayerQuestData const * PlayerObject::getQuestData(int const questId) const
{
	PlayerQuestDataMap::const_iterator i = m_quests.find(questId);
	if(i != m_quests.end())
	{
		return &i->second;
	}
	else
	{
		return NULL;
	}
}

// ----------------------------------------------------------------------

void PlayerObject::questGetActiveQuests(stdvector<Quest const *>::fwd & results) const
{
	for (PlayerQuestDataMap::const_iterator i = m_quests.begin(); i != m_quests.end(); ++i)
	{
		if (!i->second.isCompleted())
		{
			Quest const * const quest=QuestManager::getQuest(i->first);
			if (quest)
				results.push_back(quest);
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::questGetAllQuests(stdvector<Quest const *>::fwd & results) const
{
	for (PlayerQuestDataMap::const_iterator i = m_quests.begin(); i != m_quests.end(); ++i)
	{
		Quest const * const quest=QuestManager::getQuest(i->first);
		if (quest)
			results.push_back(quest);
	}
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasActiveQuest(uint32 const questId) const
{
	PlayerQuestDataMap::const_iterator i = m_quests.find(questId);
	if (i == m_quests.end())
		return false;
	else
		return !(i->second.isCompleted());
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasCompletedQuest(uint32 const questId) const
{
	PlayerQuestDataMap::const_iterator i = m_quests.find(questId);
	if (i == m_quests.end())
		return false;
	else
		return (i->second.isCompleted());
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasActiveQuestTask(uint32 const questCrc, int const taskId) const
{
	PlayerQuestDataMap::const_iterator questData = m_quests.find(questCrc);
	if (questData == m_quests.end())
		return false;

	return (questData->second.isTaskActive(taskId));

}

// ----------------------------------------------------------------------

bool PlayerObject::questHasCompletedQuestTask(uint32 const questCrc, int const taskId) const
{
	PlayerQuestDataMap::const_iterator questData = m_quests.find(questCrc);
	if (questData == m_quests.end())
		return false;

	return (questData->second.isTaskCompleted(taskId));
}

// ----------------------------------------------------------------------

bool PlayerObject::questHasReceivedQuestReward(uint32 const questCrc) const
{
	PlayerQuestDataMap::const_iterator i = m_quests.find(questCrc);
	if (i == m_quests.end())
		return true;
	else
		return (i->second.hasReceivedReward());
}

//----------------------------------------------------------------------

int PlayerObject::questCount() const
{
	return m_quests.size() + m_activeQuests.get().getNumberOfSetBits();
}

//----------------------------------------------------------------------

void PlayerObject::setCurrentQuest(uint32 questCrc)
{
	CreatureObject * const creaturePlayer = Game::getPlayerCreature();
	if (creaturePlayer != NULL && creaturePlayer->getPlayerObject() == this && creaturePlayer->getController() != NULL)
	{
		MessageQueueGenericValueType<uint32> * const msg = new MessageQueueGenericValueType<uint32>(questCrc);
		creaturePlayer->getController()->appendMessage (
			CM_setCurrentQuest, 
			0.0f, 
			msg, 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
	}	
}

//----------------------------------------------------------------------

void PlayerObject::updatePlayedTimeAccum(float deltaTime)
{
	if (m_lastPlayedTime != m_playedTime.get())
	{
		m_playedTimeAccum = 0.0f;
		m_lastPlayedTime = m_playedTime.get();
	}
	m_playedTimeAccum += deltaTime;
}

//-----------------------------------------------------------------------

void PlayerObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
#if PRODUCTION == 0
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/

//	Archive::AutoDeltaMap<std::pair<uint32, uint32>, int, PlayerObject>         m_draftSchematics;    // draft schematics the player may use
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CraftingComponentBioLink", m_craftingComponentBioLink.get().getValueString());
//	Archive::AutoDeltaMap<std::string, int, PlayerObject>  m_experiencePoints;   // xp name->amount map
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ExpModified", m_expModified.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MaxForcePower", m_maxForcePower.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ForcePower", m_forcePower.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CraftingLevel", m_craftingLevel.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ExperimentPoints", m_experimentPoints.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CraftingStage", m_craftingStage.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "CraftingStation", m_craftingStation.get().getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Biography", m_biography);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "RequestedBiography", m_requestedBiography);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "HaveBiography", m_haveBiography);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LocalizedTitle", m_localizedTitle);
//	Archive::AutoDeltaVariable<MatchMakingId>                                                                  m_matchMakingPersonalProfileId;
//	Archive::AutoDeltaVariableCallback<MatchMakingId, Callbacks::MatchMakingCharacterIdChanged, PlayerObject>  m_matchMakingCharacterProfileId;
//	Archive::AutoDeltaVector<std::string, PlayerObject>                                                        m_friendList;
//	Archive::AutoDeltaVector<std::string, PlayerObject>                                                        m_ignoreList;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SkillTitle", m_skillTitle.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SpokenLanguage", m_spokenLanguage.get());
//	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  * m_waypoints;
//	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  * m_groupWaypoints;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "BornDate", m_bornDate.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PlayedTime", m_playedTime.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LastPlayedTime", m_lastPlayedTime);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PlayedTimeAccum", m_playedTimeAccum);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Food", m_food.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MaxFood", m_maxFood.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Drink", m_drink.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MaxDrink", m_maxDrink.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Meds", m_meds.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "MaxMeds", m_maxMeds.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PrivledgedTitle", m_privledgedTitle.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "LastSitOnAttemptUniqueId", m_lastSitOnAttemptUniqueId);
//	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::CompletedQuestsChanged, PlayerObject>  m_completedQuests;
//	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::ActiveQuestsChanged, PlayerObject>  m_activeQuests;
//	Archive::AutoDeltaPackedMap<uint32, PlayerQuestData, PlayerObject> m_quests;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsAdmin", s_admin);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "JediState", m_jediState.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "RoleIconChoice", m_roleIconChoice.get());

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "SkillTemplate", m_skillTemplate.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "WorkingSkill", m_workingSkill.get());

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PgcRatingCount", NetworkId(m_pgcRatingCount.get()).getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PgcRatingTotal", NetworkId(m_pgcRatingTotal.get()).getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "PgcLastRatingTime", m_pgcLastRatingTime.get());

	char buffer[512];
	if (m_gcwRatingActualCalcTimeServerEpoch.get() > 0)
	{
		int timeUntil = static_cast<int>(static_cast<time_t>(m_gcwRatingActualCalcTimeServerEpoch.get()) - ::time(NULL));
		if (timeUntil >= 0)
		{
			snprintf(buffer, sizeof(buffer)-1, "%ld (%s) (%s) (%s)", m_gcwRatingActualCalcTimeServerEpoch.get(), CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeUntil)).c_str(), CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(m_gcwRatingActualCalcTimeServerEpoch.get())).c_str(), CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(m_gcwRatingActualCalcTimeServerEpoch.get())).c_str());
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "%ld (-%s) (%s) (%s)", m_gcwRatingActualCalcTimeServerEpoch.get(), CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(-timeUntil)).c_str(), CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(m_gcwRatingActualCalcTimeServerEpoch.get())).c_str(), CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(m_gcwRatingActualCalcTimeServerEpoch.get())).c_str());
		}
	}
	else
	{
		snprintf(buffer, sizeof(buffer)-1, "%ld", m_gcwRatingActualCalcTimeServerEpoch.get());
	}

	buffer[sizeof(buffer)-1] = '\0';
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GcwRatingActualCalcTimeServerEpoch", buffer);

	if (m_gcwRatingActualCalcTimeClientEpoch > 0)
	{
		int timeUntil = static_cast<int>(m_gcwRatingActualCalcTimeClientEpoch - ::time(NULL));
		if (timeUntil >= 0)
		{
			snprintf(buffer, sizeof(buffer)-1, "%ld (%s) (%s) (%s)", m_gcwRatingActualCalcTimeClientEpoch, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeUntil)).c_str(), CalendarTime::convertEpochToTimeStringGMT(m_gcwRatingActualCalcTimeClientEpoch).c_str(), CalendarTime::convertEpochToTimeStringLocal(m_gcwRatingActualCalcTimeClientEpoch).c_str());
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "%ld (-%s) (%s) (%s)", m_gcwRatingActualCalcTimeClientEpoch, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(-timeUntil)).c_str(), CalendarTime::convertEpochToTimeStringGMT(m_gcwRatingActualCalcTimeClientEpoch).c_str(), CalendarTime::convertEpochToTimeStringLocal(m_gcwRatingActualCalcTimeClientEpoch).c_str());
		}
	}
	else
	{
		snprintf(buffer, sizeof(buffer)-1, "%ld", m_gcwRatingActualCalcTimeClientEpoch);
	}

	buffer[sizeof(buffer)-1] = '\0';
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GcwRatingActualCalcTimeClientEpoch", buffer);

	// citizenship info
	std::string citizenshipInfo;
	if (m_citizenshipType.get() == static_cast<int8>(CityDataCitizenType::NotACitizen))
	{
		citizenshipInfo = "Wanderer";
	}
	else if (m_citizenshipType.get() == static_cast<int8>(CityDataCitizenType::Mayor))
	{
		citizenshipInfo = "Mayor of (";
		citizenshipInfo += m_citizenshipCity.get();
		citizenshipInfo += ")";
	}
	else if (m_citizenshipType.get() == static_cast<int8>(CityDataCitizenType::Militia))
	{
		citizenshipInfo = "Militia of (";
		citizenshipInfo += m_citizenshipCity.get();
		citizenshipInfo += ")";
	}
	else if (m_citizenshipType.get() == static_cast<int8>(CityDataCitizenType::Citizen))
	{
		citizenshipInfo = "Citizen of (";
		citizenshipInfo += m_citizenshipCity.get();
		citizenshipInfo += ")";
	}
	else
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer)-1, "Citizen type (%d) of (", m_citizenshipType.get());
		buffer[sizeof(buffer)-1] = '\0';

		citizenshipInfo = buffer;
		citizenshipInfo += m_citizenshipCity.get();
		citizenshipInfo += ")";
	}

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Citizenship", citizenshipInfo);

	// GCW Region Defender info
	if (!m_cityGcwDefenderRegion.get().first.empty())
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GCW Region Defender Title - City", FormattedString<512>().sprintf("%s, bonus=%s, title=%s", m_cityGcwDefenderRegion.get().first.c_str(), (m_cityGcwDefenderRegion.get().second.first ? "true" : "false"), (m_cityGcwDefenderRegion.get().second.second ? "true" : "false")));

	if (!m_guildGcwDefenderRegion.get().first.empty())
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "GCW Region Defender Title - Guild", FormattedString<512>().sprintf("%s, bonus=%s, title=%s", m_guildGcwDefenderRegion.get().first.c_str(), (m_guildGcwDefenderRegion.get().second.first ? "true" : "false"), (m_guildGcwDefenderRegion.get().second.second ? "true" : "false")));

	// squelch info
	std::string squelchInfo;
	int const secondsUntilUnsquelched = getSecondsUntilUnsquelched();
	if (secondsUntilUnsquelched == 0)
	{
		squelchInfo = "Not squelched";
	}
	else if (secondsUntilUnsquelched < 0)
	{
		squelchInfo = "Squelched indefinitely by ";
		squelchInfo += m_squelchedByName.get();
		squelchInfo += " (";
		squelchInfo += m_squelchedById.get().getValueString();
		squelchInfo += ")";
	}
	else
	{
		squelchInfo = "Squelched for ";
		squelchInfo += CalendarTime::convertSecondsToHMS(secondsUntilUnsquelched);
		squelchInfo += " by ";
		squelchInfo += m_squelchedByName.get();
		squelchInfo += " (";
		squelchInfo += m_squelchedById.get().getValueString();
		squelchInfo += ")";
	}

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Squelch", squelchInfo);

	IntangibleObject::getObjectInfo(propertyMap);

#else
	UNREF(propertyMap);
#endif
}

//----------------------------------------------------------------------

void PlayerObject::setAdmin(bool admin)
{
#if PRODUCTION == 0
	s_admin = admin;
#else
	UNREF(admin);
#endif
}

//----------------------------------------------------------------------

Quest const * PlayerObject::questGetMostRecentActiveVisibleQuest() const
{
	uint32 lastRelativeAgeIndex = 0;
	Quest const * lastQuest = 0;

	for (PlayerQuestDataMap::const_iterator i = m_quests.begin(); i != m_quests.end(); ++i)
	{
		PlayerQuestData const & questData = i->second;
		uint32 const questCrc = i->first;

		if (!questData.isCompleted() && questData.getRelativeAgeIndex() >= lastRelativeAgeIndex)
		{
			Quest const * const quest = QuestManager::getQuest(questCrc);

			if (quest && quest->isVisible())
			{
				lastQuest = quest;
				lastRelativeAgeIndex = questData.getRelativeAgeIndex();
			}
		}
	}

	return lastQuest;
}

//----------------------------------------------------------------------

bool const PlayerObject::isInPlayerHateList(NetworkId const & id) const
{
	return m_playerHateList.contains(id);
}

//----------------------------------------------------------------------

int32 PlayerObject::getKillMeter() const
{
	return m_killMeter.get();
}

// ----------------------------------------------------------------------

bool PlayerObject::getCollectionSlotValue(std::string const & slotName, unsigned long & value) const
{
	value = 0;

	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if (!slotInfo)
		return false;

	return getCollectionSlotValue(*slotInfo, value);
}

// ----------------------------------------------------------------------

bool PlayerObject::getCollectionSlotValue(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long & value) const
{
	// bit-type slot
	if (!slotInfo.counterTypeSlot)
	{
		if (m_allCollections[slotInfo.slotIdIndex]->get().testBit(slotInfo.beginSlotId))
			value = 1;
		else
			value = 0;
	}
	else
	{
		value = m_allCollections[slotInfo.slotIdIndex]->get().getValue(slotInfo.beginSlotId, slotInfo.endSlotId);
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlot(std::string const & slotName) const
{
	CollectionsDataTable::CollectionInfoSlot const * slotInfo = CollectionsDataTable::getSlotByName(slotName);
	if (!slotInfo)
		return false;

	return hasCompletedCollectionSlot(*slotInfo);
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo) const
{
	// bit-type slot
	if (!slotInfo.counterTypeSlot)
		return m_allCollections[slotInfo.slotIdIndex]->get().testBit(slotInfo.beginSlotId);

	// counter-type slot is only completed if a max slot value is
	// specified and the current slot value equals the max slot value
	return ((slotInfo.maxSlotValue > 0) && (slotInfo.maxSlotValue == m_allCollections[slotInfo.slotIdIndex]->get().getValue(slotInfo.beginSlotId, slotInfo.endSlotId)));
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollection(std::string const & collectionName) const
{
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionName);

	if (slots.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (!hasCompletedCollectionSlot(**iter))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionPage(std::string const & pageName) const
{
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageName);

	if (collections.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (!hasCompletedCollection((*iter)->name))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool PlayerObject::hasCompletedCollectionBook(std::string const & bookName) const
{
	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookName);

	if (pages.empty())
		return false;

	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = pages.begin(); iter != pages.end(); ++iter)
	{
		if (!hasCompletedCollectionPage((*iter)->name))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInCollection(std::string const & collectionName, stdvector<CollectionsDataTable::CollectionInfoSlot const *>::fwd * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage(pageName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionCountInPage(std::string const & pageName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageName);
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (hasCompletedCollection((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionSlotCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoSlot const *> * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		if (hasCompletedCollectionSlot(**iter))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoCollection const *> * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		if (hasCompletedCollection((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionPageCountInBook(std::string const & bookName, std::vector<CollectionsDataTable::CollectionInfoPage const *> * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookName);
	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = pages.begin(); iter != pages.end(); ++iter)
	{
		if (hasCompletedCollectionPage((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------

int PlayerObject::getCompletedCollectionBookCount(std::vector<CollectionsDataTable::CollectionInfoBook const *> * collectionInfo /*= NULL*/) const
{
	if (collectionInfo)
		collectionInfo->clear();

	int count = 0;
	std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
	for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iter = books.begin(); iter != books.end(); ++iter)
	{
		if (hasCompletedCollectionBook((*iter)->name))
		{
			++count;

			if (collectionInfo)
				collectionInfo->push_back(*iter);
		}
	}

	return count;
}

// ----------------------------------------------------------------------
// returns 0 if not currently squelched; returns < 0 if indefinitely squelched
int PlayerObject::getSecondsUntilUnsquelched() const
{
	// not squelched
	if (!m_squelchedById.get().isValid())
		return 0;

	// squelched, but need to check to see if the squelch period has already expired
	if (m_squelchExpireTimeClientEpoch < static_cast<time_t>(0)) // squelched indefinitely
		return -1;

	time_t const timeNow = ::time(NULL);
	if (timeNow < m_squelchExpireTimeClientEpoch) // still in squelch period
		return static_cast<int>(m_squelchExpireTimeClientEpoch - timeNow);

	// squelch has expired
	return 0;
}


void PlayerObject::getActiveCollectionBooks(std::vector<std::string> * const bookNames) const
{
	std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();

	for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator bookIter = books.begin(); bookIter != books.end(); ++bookIter)
	{
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook((*bookIter)->name);

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator slotIter = slots.begin(); slotIter != slots.end(); ++slotIter)
		{
			unsigned long collectionSlotValue;

			if (getCollectionSlotValue(**slotIter, collectionSlotValue) && collectionSlotValue > 0)
			{
				bookNames->push_back((*bookIter)->name);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::getActiveCollectionPages(std::string const & bookName, std::vector<std::string> * const pageNames) const
{
	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookName);

	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator pageIter = pages.begin(); pageIter != pages.end(); ++pageIter)
	{
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage((*pageIter)->name);

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator slotIter = slots.begin(); slotIter != slots.end(); ++slotIter)
		{
			unsigned long collectionSlotValue;

			if (getCollectionSlotValue(**slotIter, collectionSlotValue) && collectionSlotValue > 0)
			{
				pageNames->push_back((*pageIter)->name);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::getActiveCollections(std::string const & pageName, std::vector<std::string> * const  collectionNames) const
{
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageName);

	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator collectionIter = collections.begin(); collectionIter != collections.end(); ++collectionIter)
	{
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*collectionIter)->name);

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator slotIter = slots.begin(); slotIter != slots.end(); ++slotIter)
		{
			unsigned long collectionSlotValue;

			if (getCollectionSlotValue(**slotIter, collectionSlotValue) && collectionSlotValue > 0)
			{
				collectionNames->push_back((*collectionIter)->name);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlayerObject::Callbacks::CollectionsChanged::modified(PlayerObject & target, const BitArray &, const BitArray &, bool) const
{
	Transceivers::collectionsChanged.emitMessage(target);
}

// ----------------------------------------------------------------------

void PlayerObject::shouldShowBackpack(bool show)
{
	Object* obj = ContainerInterface::getContainedByObject(*this);

	SkeletalAppearance2* mesh = NULL;

	if(obj)
		mesh = obj->getAppearance()->asSkeletalAppearance2();
	
	if(mesh)
	{
		for(int i = 0; i < mesh->getWearableCount(); ++i)
		{
			SkeletalAppearance2* wearable = mesh->getWearableAppearance(i);

			if(isObjectABackpack(wearable))
			{
				Object* wearObj = mesh->getWearableObject(i);
				if(wearObj)
				{
					wearObj->setShouldBakeIntoMesh(show);
					mesh->markAsDirty();

					return;
				}
			}

		}
	}

}

// ----------------------------------------------------------------------

void PlayerObject::sendBackpackMsg(bool showBackpack)
{
	GenericValueTypeMessage<std::pair<NetworkId, bool > > const msg("ShowBackpack", std::make_pair(getNetworkId(), showBackpack));
	GameNetwork::send(msg, true);
}

// ----------------------------------------------------------------------

void PlayerObject::shouldShowHelmet(bool show)
{
	Object* obj = ContainerInterface::getContainedByObject(*this);

	SkeletalAppearance2* mesh = NULL;

	if(obj)
		mesh = obj->getAppearance()->asSkeletalAppearance2();

	if(mesh)
	{
		for(int i = 0; i < mesh->getWearableCount(); ++i)
		{
			SkeletalAppearance2* wearable = mesh->getWearableAppearance(i);
	
			if(isObjectAHelmet(wearable))
			{
				Object* wearObj = mesh->getWearableObject(i);

				wearObj->setShouldBakeIntoMesh(show);
				mesh->markAsDirty();
				return;
			}

		}
	}

}

// ----------------------------------------------------------------------

void PlayerObject::sendHelmetMsg(bool showHelmet)
{
	GenericValueTypeMessage<std::pair<NetworkId, bool > > const msg("ShowHelmet", std::make_pair(getNetworkId(), showHelmet));
	GameNetwork::send(msg, true);
}

// ----------------------------------------------------------------------

bool PlayerObject::isObjectABackpack(Appearance* const appearance)
{
	if(!appearance)
		return false;
	if(!appearance->getAppearanceTemplateName())
		return false;
	if(std::string(appearance->getAppearanceTemplateName()).find("backpack") != std::string::npos || 
		std::string(appearance->getAppearanceTemplateName()).find("bandolier") != std::string::npos)
		return true;

	return false;
}

// ----------------------------------------------------------------------

bool PlayerObject::isObjectAHelmet(Appearance* const appearance)
{
	if(!appearance)
		return false;

	if(!appearance->getOwner())
		return false;

	ClientObject* clientOwner = appearance->getOwner()->asClientObject();

	if(!clientOwner)
		return false;

	if(clientOwner->getGameObjectType() == SharedObjectTemplate::GOT_armor_head || clientOwner->getGameObjectType() == SharedObjectTemplate::GOT_clothing_head)
		return true;
	
	return false;
}

// ----------------------------------------------------------------------

void PlayerObject::toggleOutOfCharacter() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("ooc");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isOutOfCharacter() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_outOfCharacter);
}

// ----------------------------------------------------------------------

void PlayerObject::toggleLookingForWork() const
{
	if (Game::getSinglePlayer())
		return;

	if (this != Game::getPlayerObject())
	{
		return;
	}

	static uint32 const hash = Crc::normalizeAndCalculate("lfw");

	IGNORE_RETURN(ClientCommandQueue::enqueueCommand(hash, NetworkId::cms_invalid, Unicode::emptyString));
}

// ----------------------------------------------------------------------

bool PlayerObject::isLookingForWork() const
{
	return m_matchMakingCharacterProfileId.get().isBitSet(MatchMakingId::B_lookingForWork);
}

//----------------------------------------------------------------------

int PlayerObject::getTotalWaypoints() const
{
	return m_waypoints ? m_waypoints->size() : 0;
}

//----------------------------------------------------------------------

// ======================================================================
