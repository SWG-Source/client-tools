//========================================================================
//
// PlayerObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_PlayerObject_H
#define INCLUDED_PlayerObject_H

#include "archive/AutoDeltaMap.h"
#include "archive/AutoDeltaPackedMap.h"
#include "archive/AutoDeltaSet.h"
#include "archive/AutoDeltaVector.h"
#include "clientGame/IntangibleObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/BitArray.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/CraftingDataArchive.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedGame/PlayerData.h"
#include "sharedGame/PlayerQuestData.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CachedNetworkIdArchive.h"

class ClientWaypointObject;
class Quest;
class SharedPlayerObjectTemplate;
class Waypoint;

//----------------------------------------------------------------------

/**
  * A PlayerObject represents player-specific data.
  */
typedef int32 EnvironmentInfo;

class PlayerObject : public IntangibleObject
{
public:

	typedef stdvector<std::string>::fwd                StringVector;
	typedef stdvector<std::pair<uint32, uint32> >::fwd SchematicVector;
	typedef stdmap<std::string, int>::fwd              ExperiencePointMap;
	typedef Archive::AutoDeltaPackedMap<uint32, PlayerQuestData> PlayerQuestDataMap;

	struct Messages
	{
		struct WhoStatusMessage  // Signals a message about a who status change
		{
			typedef Unicode::String Message;
		};

		struct WhoStatusChanged  // Signals the who status changed
		{
			typedef MatchMakingId Status;
		};

		struct CommandsChanged
		{
			typedef PlayerObject Payload;
		};

		struct DraftSchematicsChanged
		{
			typedef PlayerObject Payload;
		};

		struct DraftSchematicAdded
		{
			typedef std::pair<PlayerObject *, std::pair<uint32, uint32> > Payload;
		};

		struct DraftSchematicRemoved
		{
			typedef std::pair<PlayerObject *, std::pair<uint32, uint32> > Payload;
		};

		struct CommandAdded
		{
			typedef std::pair<const PlayerObject *, std::string> Payload;
		};

		struct CommandRemoved
		{
			typedef std::pair<PlayerObject *, std::string> Payload;
		};

		struct ExperienceChanged
		{
			typedef PlayerObject Payload;
		};

		struct WaypointsChanged
		{
			typedef PlayerObject Payload;
		};

		struct JediStateChanged
		{
			typedef PlayerObject Payload;
		};

		struct SpokenLangaugeChanged
		{
			typedef int Language;
		};

		struct CompletedQuestsChanged
		{
			typedef PlayerObject Payload;
		};

		struct RoleIconChoiceChanged
		{
			typedef PlayerObject Payload;
		};

		struct ActiveQuestsChanged
		{
			typedef PlayerObject Payload;
		};

		struct QuestsChanged
		{
			typedef PlayerObject Payload;
		};

		struct CurrentQuestChanged
		{
			typedef PlayerObject Payload;
		};

		struct CraftingBioLinkChanged
		{
			typedef PlayerObject Payload;
		};

		struct SkillTemplateChanged
		{
			typedef PlayerObject Payload;
		};

		struct WorkingSkillChanged
		{
			typedef PlayerObject Payload;
		};

		struct PetChanged
		{
			typedef PlayerObject Payload;
		};

		struct PetCommandsChanged
		{
			typedef PlayerObject Payload;
		};

		struct PetToggledCommandsChanged
		{
			typedef PlayerObject Payload;
		};

		struct CollectionsChanged
		{
			typedef PlayerObject Payload;
		};

		struct GalacticReserveDepositChanged
		{
			typedef PlayerObject Payload;
		};

		struct ShowBackpackChanged
		{
			typedef PlayerObject Payload;
		};

		struct ShowHelmetChanged
		{
			typedef PlayerObject Payload;
		};
	};

public:

	explicit PlayerObject(const SharedPlayerObjectTemplate* newTemplate);
	virtual ~PlayerObject();

	virtual void            endBaselines           ();

	Crafting::CraftingStage getCraftingStage       () const;
	int                     getCraftingLevel       () const;
	int                     getExperimentPoints    () const;
	const CachedNetworkId & getCraftingStation     () const;

	const ExperiencePointMap & getExperiencePointMap   () const;
	const stdmap<std::pair<uint32,uint32>,int>::fwd & getDraftSchematics() const;

	int                        getMaxForcePower        () const;
	int                        getForcePower           () const;

	//@todo It is unfortunate that these jedi-related things are in the supposedly generic PlayerObject
    virtual bool               isJedi                  () const;
	int						   getJediState			   () const;

	void                       clientSetMaxForcePower  (int value);
	void                       clientSetForcePower     (int value);

	void                       clientGrantSchematic    (uint32 schematicCrc, uint32 sharedCrc);
	void                       clientGrantCommand      (const std::string & command);
	void                       clientRevokeSchematic   (uint32 schematicCrc, uint32 sharedCrc);
	void                       clientRevokeCommand     (const std::string & command);
	void                       clientGrantExp          (const std::string & exptype, int amount);
	bool                       getExperience           (const std::string & expName, int & result) const;

	void                    requestBiography       () const;
	bool                    haveBiography          () const;
	void                    setHaveBiography       (bool haveBiography);
	void                    setBiography           (Unicode::String const &biography, bool serverSync);
	Unicode::String const & getBiography           () const;

	//void                    setMatchMakingSensitivity        (const float percent);
	//float                   getMatchMakingSensitivity        () const;

	MatchMakingId const &   getMatchMakingPersonalId         () const;
	MatchMakingId const &   getMatchMakingCharacterProfileId () const;

	void                    toggleSearchableByCtsSourceGalaxy() const;
	bool                    isSearchableByCtsSourceGalaxy() const;

	void                    toggleDisplayLocationInSearchResults() const;
	bool                    isDisplayLocationInSearchResults() const;

	void                    toggleAnonymous               () const;
	bool                    isAnonymous                   () const;

	void                    toggleRolePlay                () const;
	bool                    isRolePlay                    () const;

	void                    toggleOutOfCharacter          () const;
	bool                    isOutOfCharacter              () const;

	void                    toggleLookingForWork          () const;
	bool                    isLookingForWork              () const;

	void                    toggleHelper                  () const;
	bool                    isHelper                      () const;

	void                    toggleLookingForGroup         () const;
	bool                    isLookingForGroup             () const;

	void                    toggleAwayFromKeyBoard        () const;
	bool                    isAwayFromKeyBoard            () const;

	void                    toggleDisplayingFactionRank   () const;
	bool                    isDisplayingFactionRank       () const;

	bool                    isLinkDead                    () const;

	void                    setTitle                      (std::string const &title);
	std::string const &     getTitle                      () const;
	const Unicode::String & getLocalizedTitle             () const;

	int                     getBornDate                   () const;
	uint32                  getPlayedTime                 () const;

	void                    setSpokenLanguage             (int const languageId);
	int                     getSpokenLanguage             () const;
	bool                    speaksLanguage                (int const languageId) const;
	bool                    comprehendsLanguage           (int const languageId) const;

	uint32                  getLastSitOnAttemptUniqueId   () const;
	void                    setLastSitOnAttemptUniqueId   (uint32 id);

	void                    applyDeferredWaypointCreation ();

	void                    setPlayerSpecificCallbacks    ();

	int                     getFood() const;
	int                     getMaxFood() const;
	int                     getDrink() const;
	int                     getMaxDrink() const;
	int                     getMeds() const;
	int                     getMaxMeds() const;

	bool                    isNormalPlayer() const;
	bool                    isCustomerServiceRepresentative() const;
	bool                    isDeveloper() const;
	bool					isQualityAssurance() const;
	bool                    isWarden() const;
	bool                    isQuestComplete(int questId) const;
	bool                    isQuestActive(int questId) const;

	PlayerQuestData const * getQuestData(int questId) const;

	static bool isAdmin();
	static void setAdmin(bool admin);

#ifdef _DEBUG
	static void             showMatchMakingStatistics     ();
#endif // _DEBUG

	// New quest system
	void                    questGetActiveQuests(stdvector<Quest const *>::fwd & results) const;
	void                    questGetAllQuests(stdvector<Quest const *>::fwd & results) const;
	bool                    questHasActiveQuest(uint32 questId) const;
	bool                    questHasCompletedQuest(uint32 questId) const;
	bool                    questHasActiveQuestTask(uint32 questCrc, int taskId) const;
	bool                    questHasCompletedQuestTask(uint32 questCrc, int taskId) const;
	Quest const *           questGetMostRecentActiveVisibleQuest() const;
	bool                    questHasReceivedQuestReward(uint32 questCrc) const;

	int                     questCount() const;
	uint32                  getCurrentQuest() const;
	void                    setCurrentQuest(uint32 questCrc);

	void                    updatePlayedTimeAccum(float deltaTime);

	int                     getRoleIconChoice() const;
	std::string const &     getWorkingSkill() const;
	std::string const &     getSkillTemplate() const;

	virtual void            getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

	int32 getCurrentGcwPoints() const;
	int32 getCurrentPvpKills() const;
	int64 getLifetimeGcwPoints() const;
	int32 getLifetimePvpKills() const;
	int getCurrentGcwRank() const;
	float getCurrentGcwRankProgress() const;
	int getMaxGcwImperialRank() const;
	int getMaxGcwRebelRank() const;
	time_t getGcwRatingActualCalcTime() const;

	bool const isInPlayerHateList(NetworkId const & id) const;

	int32 getKillMeter() const;

	int getAccountNumLotsOverLimitSpam() const;

	NetworkId const & getPetId() const;
	std::vector<std::string> const & getPetCommands() const;
	std::vector<std::string> const & getPetToggledCommands() const;

	bool getCollectionSlotValue(std::string const & slotName, unsigned long & value) const;
	bool getCollectionSlotValue(CollectionsDataTable::CollectionInfoSlot const & slotInfo, unsigned long & value) const;

	bool hasCompletedCollectionSlot(std::string const & slotName) const;
	bool hasCompletedCollectionSlot(CollectionsDataTable::CollectionInfoSlot const & slotInfo) const;

	bool hasCompletedCollection(std::string const & collectionName) const;

	bool hasCompletedCollectionPage(std::string const & pageName) const;

	bool hasCompletedCollectionBook(std::string const & bookName) const;

	int getCompletedCollectionSlotCountInCollection(std::string const & collectionName, stdvector<CollectionsDataTable::CollectionInfoSlot const *>::fwd * collectionInfo = NULL) const;

	int getCompletedCollectionSlotCountInPage(std::string const & pageName, stdvector<CollectionsDataTable::CollectionInfoSlot const *>::fwd * collectionInfo = NULL) const;
	int getCompletedCollectionCountInPage(std::string const & pageName, stdvector<CollectionsDataTable::CollectionInfoCollection const *>::fwd * collectionInfo = NULL) const;

	int getCompletedCollectionSlotCountInBook(std::string const & bookName, stdvector<CollectionsDataTable::CollectionInfoSlot const *>::fwd * collectionInfo = NULL) const;
	int getCompletedCollectionCountInBook(std::string const & bookName, stdvector<CollectionsDataTable::CollectionInfoCollection const *>::fwd * collectionInfo = NULL) const;
	int getCompletedCollectionPageCountInBook(std::string const & bookName, stdvector<CollectionsDataTable::CollectionInfoPage const *>::fwd * collectionInfo = NULL) const;
	int getCompletedCollectionBookCount(stdvector<CollectionsDataTable::CollectionInfoBook const *>::fwd * collectionInfo = NULL) const;

	void getActiveCollectionBooks(stdvector<std::string>::fwd * const bookNames) const;
	void getActiveCollectionPages(std::string const & bookName, stdvector<std::string>::fwd * const pageNames) const;
	void getActiveCollections(std::string const & pageName, stdvector<std::string>::fwd * const  collectionNames) const;

	// GCW Region Defender info
	bool cityGcwDefenderRegionTitleAvailable() const;
	bool guildGcwDefenderRegionTitleAvailable() const;

	// squelch info
	NetworkId const & getSquelchedById() const;
	std::string const & getSquelchedByName() const;
	int getSecondsUntilUnsquelched() const; // returns 0 if not currently squelched; returns < 0 if indefinitely squelched

	bool getShowBackpack() const;
	void sendBackpackMsg(bool showBackpack);

	bool getShowHelmet() const;
	void sendHelmetMsg(bool showHelmet);

	static bool isObjectABackpack(Appearance* const appearance);
	static bool isObjectAHelmet(Appearance* const appearance);

	BitArray const & getGuildRank() const;

	BitArray const & getCitizenRank() const;

	EnvironmentInfo const & getEnvironmentFlags() const;
	std::string const & getDefaultAttackOverride() const;

	int8 getGalacticReserveDeposit() const;

	int64 getPgcRatingCount() const;
	int64 getPgcRatingTotal() const;
	int getPgcLastRatingTime() const;

	int getTotalWaypoints() const;

protected:

private:

	PlayerObject();
	PlayerObject(const PlayerObject& rhs);
	PlayerObject&	operator=(const PlayerObject& rhs);

private:
	
	static bool s_admin;

	void shouldShowBackpack(bool show);
	void shouldShowHelmet(bool show);

	void waypointsOnSet    (const NetworkId & key, const Waypoint & oldValue, const Waypoint & newValue);
	void waypointsOnInsert (const NetworkId & key, const Waypoint & value);
	void waypointsOnErase  (const NetworkId & key, const Waypoint & value);
	void groupWaypointsOnSet    (NetworkId const &key, Waypoint const &oldValue, Waypoint const &newValue);
	void groupWaypointsOnInsert (NetworkId const &key, Waypoint const &value);
	void groupWaypointsOnErase  (NetworkId const &key, Waypoint const &value);

	struct Callbacks
	{
		template <typename T, typename U> struct DefaultCallback
		{
			void modified (PlayerObject & target, const U & old, const U & value, bool isLocal) const;
		};

		struct DummyMessagesTitle;
		struct DummyMessagesPrivledgedTitle;
		struct DummyMessagesSkillTemplate;
		struct DummyMessagesWorkingSkill;
		struct DummyMessagesCitizenshipCity;
		struct DummyMessagesCitizenshipType;
		struct DummyMessagesCityGcwRegionDefender;
		struct DummyMessagesGuildGcwRegionDefender;
		struct DummyMessagesShowBackpack;
		struct DummyMessagesShowHelmet;

		struct MatchMakingCharacterIdChanged
		{
			void modified(PlayerObject &target, const MatchMakingId &oldValue, const MatchMakingId &newValue, bool local);
		};

		struct SpokenLanguageChanged
		{
			void modified(PlayerObject &target, const int &oldValue, const int &newValue, bool local);
		};

		struct JediStateChanged
		{
			void modified(PlayerObject &target, const int &oldValue, const int &newValue, bool local);
		};

		struct RoleIconChoiceChanged
		{
			void modified(PlayerObject &target, const int &oldValue, const int &newValue, bool local);
		};

		struct CraftingBioLinkChanged
		{
			void modified(PlayerObject &target, const NetworkId &oldValue, const NetworkId &newValue, bool local);
		};

		
		typedef DefaultCallback<DummyMessagesShowBackpack, bool> ShowBackpackChanged;
		typedef DefaultCallback<DummyMessagesShowHelmet, bool> ShowHelmetChanged;
		typedef DefaultCallback<DummyMessagesTitle, std::string> TitleChanged;
		typedef DefaultCallback<DummyMessagesPrivledgedTitle, int8> PrivledgedTitleChanged;
		typedef DefaultCallback<Messages::ExperienceChanged, int> ExperienceChanged;
		typedef DefaultCallback<Messages::CompletedQuestsChanged, BitArray> CompletedQuestsChanged;
		typedef DefaultCallback<Messages::ActiveQuestsChanged, BitArray> ActiveQuestsChanged;
		typedef DefaultCallback<Messages::CurrentQuestChanged, uint32> CurrentQuestChanged;
		typedef DefaultCallback<DummyMessagesSkillTemplate, std::string> SkillTemplateChanged;
		typedef DefaultCallback<DummyMessagesWorkingSkill, std::string> WorkingSkillChanged;
		typedef DefaultCallback<Messages::PetChanged, NetworkId> PetChanged;
		typedef DefaultCallback<DummyMessagesCitizenshipCity, std::string> CitizenshipCityChanged;
		typedef DefaultCallback<DummyMessagesCitizenshipType, int8> CitizenshipTypeChanged;
		typedef DefaultCallback<DummyMessagesCityGcwRegionDefender, std::pair<std::string, std::pair<bool, bool> > > CityGcwRegionDefenderChanged;
		typedef DefaultCallback<DummyMessagesGuildGcwRegionDefender, std::pair<std::string, std::pair<bool, bool> > > GuildGcwRegionDefenderChanged;
		typedef DefaultCallback<Messages::CollectionsChanged, BitArray> CollectionsChanged;
		typedef DefaultCallback<Messages::GalacticReserveDepositChanged, int8> GalacticReserveDepositChanged;

		struct GcwRatingActualCalcTimeServerEpochChanged
		{
			void modified(PlayerObject &target, const int32 &oldValue, const int32 &newValue, bool local) const;
		};

		struct SquelchExpireTimeServerEpochChanged
		{
			void modified(PlayerObject &target, const int32 &oldValue, const int32 &newValue, bool local) const;
		};
	};

	void onPetCommandsChanged();
	void onPetToggledCommandsChanged();

	friend Callbacks::TitleChanged;
	friend Callbacks::PrivledgedTitleChanged;
	friend Callbacks::GcwRatingActualCalcTimeServerEpochChanged;
	friend Callbacks::SquelchExpireTimeServerEpochChanged;

	void onFriendListChanged();
	void onIgnoreListChanged();

	void draftSchematicsOnErase   (const std::pair<uint32, uint32> & schematicCrc, const int & count);
	void draftSchematicsOnInsert  (const std::pair<uint32, uint32> & schematicCrc, const int & count);
	void draftSchematicsOnInit    (const std::pair<uint32, uint32> & schematicCrc, const int & count);

	void commandsOnErase          (const std::string & cmd, const int & value);
	void commandsOnInsert         (const std::string & cmd, const int & value);

	void expOnSet                 (const std::string & key, const int & oldValue, const int & value);
	void expOnInsert              (const std::string & key, const int & value);
	void expOnErase               (const std::string & key, const int & value);

	void questsOnChanged();

	// player abilities
	Archive::AutoDeltaMap<std::string, int, PlayerObject>    m_commands;           // game commands player may execute
	Archive::AutoDeltaMap<std::pair<uint32, uint32>, int, PlayerObject>         m_draftSchematics;    // draft schematics the player may use
	Archive::AutoDeltaVariableCallback<NetworkId, Callbacks::CraftingBioLinkChanged, PlayerObject> m_craftingComponentBioLink;        // the bio link id from a bio-linked component during crafting
	Archive::AutoDeltaMap<std::string, int, PlayerObject>  m_experiencePoints;   // xp name->amount map
	Archive::AutoDeltaVariableCallback<int, Callbacks::ExperienceChanged, PlayerObject> m_expModified;        // just a flag that we increment any time the creature is granted new xp
	Archive::AutoDeltaVariable<int>                      m_maxForcePower;      ///< Maximum force power the player can have
	Archive::AutoDeltaVariable<int>                      m_forcePower;         ///< Current force power the player has

	// crafting system data
	Archive::AutoDeltaVariable<int>                        m_craftingLevel;     // crafting level of the current crafting session
	Archive::AutoDeltaVariable<int>                        m_experimentPoints;  // experiment points available to the player
	Archive::AutoDeltaVariable<Crafting::CraftingStage>    m_craftingStage;     // what stage in the crafting process a player is in
	Archive::AutoDeltaVariable<CachedNetworkId>            m_craftingStation;         // station a player is crafting with
	Unicode::String                                        m_biography;

	mutable bool                                           m_requestedBiography : 1;
	bool                                                   m_haveBiography : 1;
	Unicode::String                                        m_localizedTitle;

	Archive::AutoDeltaVariable<MatchMakingId>                                                                  m_matchMakingPersonalProfileId;
	Archive::AutoDeltaVariableCallback<MatchMakingId, Callbacks::MatchMakingCharacterIdChanged, PlayerObject>  m_matchMakingCharacterProfileId;

	// these 2 lists ***MUST BE LOWERCASE AND MUST BE KEPT SORTED FOR QUICK LOOKUP***
	Archive::AutoDeltaVector<std::string, PlayerObject>                                                        m_friendList;
	Archive::AutoDeltaVector<std::string, PlayerObject>                                                        m_ignoreList;

	Archive::AutoDeltaVariableCallback<std::string, Callbacks::TitleChanged, PlayerObject>                     m_skillTitle;
	Archive::AutoDeltaVariableCallback<int, Callbacks::SpokenLanguageChanged, PlayerObject>                    m_spokenLanguage;
	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  * m_waypoints;
	Archive::AutoDeltaMap<NetworkId, Waypoint, PlayerObject>  * m_groupWaypoints;
	Archive::AutoDeltaVariable<int>                             m_bornDate;
	Archive::AutoDeltaVariable<uint32>                          m_playedTime;
	uint32                                                      m_lastPlayedTime;
	float                                                       m_playedTimeAccum;
	Archive::AutoDeltaVariable<int>                             m_food;
	Archive::AutoDeltaVariable<int>                             m_maxFood;
	Archive::AutoDeltaVariable<int>                             m_drink;
	Archive::AutoDeltaVariable<int>                             m_maxDrink;
	Archive::AutoDeltaVariable<int>                             m_meds;
	Archive::AutoDeltaVariable<int>                             m_maxMeds;
	Archive::AutoDeltaVariableCallback<int8, Callbacks::PrivledgedTitleChanged, PlayerObject>      m_privledgedTitle;

	uint32                                                      m_lastSitOnAttemptUniqueId;

	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::CompletedQuestsChanged, PlayerObject> m_completedQuests;
	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::ActiveQuestsChanged, PlayerObject> m_activeQuests;
	Archive::AutoDeltaVariableCallback<uint32, Callbacks::CurrentQuestChanged, PlayerObject> m_currentQuest;
	Archive::AutoDeltaVariableCallback<bool, Callbacks::ShowBackpackChanged, PlayerObject> m_showBackpack;
	Archive::AutoDeltaVariableCallback<bool, Callbacks::ShowHelmetChanged, PlayerObject> m_showHelmet;

	Archive::AutoDeltaPackedMap<uint32, PlayerQuestData, PlayerObject> m_quests;

	// from SwgPlayerObject
	// @todo: do we want to create a client SwgPlayerObject?
	Archive::AutoDeltaVariableCallback<int, Callbacks::JediStateChanged, PlayerObject>                         m_jediState;

	Archive::AutoDeltaVariableCallback<int, Callbacks::RoleIconChoiceChanged, PlayerObject>  m_roleIconChoice;

	Archive::AutoDeltaVariableCallback<std::string, Callbacks::SkillTemplateChanged, PlayerObject> m_skillTemplate;
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::WorkingSkillChanged, PlayerObject> m_workingSkill;

	Archive::AutoDeltaVariable<int32> m_currentGcwPoints;
	Archive::AutoDeltaVariable<int32> m_currentPvpKills;
	Archive::AutoDeltaVariable<int64> m_lifetimeGcwPoints;
	Archive::AutoDeltaVariable<int32> m_lifetimePvpKills;
	Archive::AutoDeltaVariable<int> m_currentGcwRank;
	Archive::AutoDeltaVariable<float> m_currentGcwRankProgress;
	Archive::AutoDeltaVariable<int> m_maxGcwImperialRank;
	Archive::AutoDeltaVariable<int> m_maxGcwRebelRank;

	// the server Epoch when the next messageTo to cause GCW rating recalculation will occur
	Archive::AutoDeltaVariableCallback<int32, Callbacks::GcwRatingActualCalcTimeServerEpochChanged, PlayerObject> m_gcwRatingActualCalcTimeServerEpoch;

	// m_gcwRatingActualCalcTimeClientEpoch is recalculated whenever 
	// m_gcwRatingActualCalcTimeServerEpoch changes
	time_t m_gcwRatingActualCalcTimeClientEpoch;

	Archive::AutoDeltaSet<NetworkId> m_playerHateList;

	Archive::AutoDeltaVariable<int32> m_killMeter;

	Archive::AutoDeltaVariable<int> m_accountNumLotsOverLimitSpam; /// controls whether the player should be spammed for exceeding the lot limit

	Archive::AutoDeltaVariableCallback<NetworkId, Callbacks::PetChanged, PlayerObject> m_petId;
	Archive::AutoDeltaVector<std::string, PlayerObject> m_petCommandList;
	Archive::AutoDeltaVector<std::string, PlayerObject> m_petToggledCommands;

	// both m_collections and m_collections2 form a "contiguous" collections bit array
	// we only break them up into multiple members because of DB storage restrictions
	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::CollectionsChanged, PlayerObject> m_collections;
	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::CollectionsChanged, PlayerObject> m_collections2;
	Archive::AutoDeltaVariableCallback<BitArray, Callbacks::CollectionsChanged, PlayerObject> * m_allCollections[2];

	// citizenship info
	Archive::AutoDeltaVariableCallback<std::string, Callbacks::CitizenshipCityChanged, PlayerObject> m_citizenshipCity;
	Archive::AutoDeltaVariableCallback<int8, Callbacks::CitizenshipTypeChanged, PlayerObject> m_citizenshipType; // CityDataCitizenType

	// GCW Region Defender info
	// std::pair<region, std::pair<qualify for region bonus, qualify to use region defender title> >
	Archive::AutoDeltaVariableCallback<std::pair<std::string, std::pair<bool, bool> >, Callbacks::CityGcwRegionDefenderChanged, PlayerObject> m_cityGcwDefenderRegion;
	Archive::AutoDeltaVariableCallback<std::pair<std::string, std::pair<bool, bool> >, Callbacks::GuildGcwRegionDefenderChanged, PlayerObject> m_guildGcwDefenderRegion;

	// squelch info
	Archive::AutoDeltaVariable<NetworkId> m_squelchedById; // id of the toon who squelched this toon; is NetworkId::cms_invalid if this toon is not squelched
	Archive::AutoDeltaVariable<std::string> m_squelchedByName; // name of the toon who squelched this toon
	Archive::AutoDeltaVariableCallback<int32, Callbacks::SquelchExpireTimeServerEpochChanged, PlayerObject> m_squelchExpireTimeServerEpoch; // the server Epoch time when the toon will be unsquelched; is < 0 for indefinite squelch

	// m_squelchExpireTimeClientEpoch is recalculated whenever 
	// m_squelchExpireTimeServerEpoch changes
	// is < 0 for indefinite squelch
	time_t m_squelchExpireTimeClientEpoch;

	Archive::AutoDeltaVariable<EnvironmentInfo> m_environmentFlags;
	Archive::AutoDeltaVariable<std::string> m_defaultAttackOverride;

	Archive::AutoDeltaVariable<BitArray> m_guildRank;
	Archive::AutoDeltaVariable<BitArray> m_citizenRank;

	Archive::AutoDeltaVariableCallback<int8, Callbacks::GalacticReserveDepositChanged, PlayerObject> m_galacticReserveDeposit;

	Archive::AutoDeltaVariable<int64> m_pgcRatingCount;
	Archive::AutoDeltaVariable<int64> m_pgcRatingTotal;
	Archive::AutoDeltaVariable<int> m_pgcLastRatingTime;
};

//----------------------------------------------------------------------

inline const PlayerObject::ExperiencePointMap & PlayerObject::getExperiencePointMap () const
{
	return m_experiencePoints.getMap ();
}

//----------------------------------------------------------------------

inline const stdmap<std::pair<uint32,uint32>,int>::fwd & PlayerObject::getDraftSchematics () const
{
	return m_draftSchematics.getMap();
}

//----------------------------------------------------------------------

inline Crafting::CraftingStage PlayerObject::getCraftingStage() const
{
	return m_craftingStage.get();
}

//----------------------------------------------------------------------

inline int PlayerObject::getCraftingLevel() const
{
	return m_craftingLevel.get();
}

//----------------------------------------------------------------------

inline int PlayerObject::getExperimentPoints() const
{
	return m_experimentPoints.get ();
}

//----------------------------------------------------------------------

inline const CachedNetworkId & PlayerObject::getCraftingStation() const
{
	return m_craftingStation.get();
}

//----------------------------------------------------------------------

inline const Unicode::String & PlayerObject::getLocalizedTitle () const
{
	return m_localizedTitle;
}

//----------------------------------------------------------------------

inline uint32 PlayerObject::getLastSitOnAttemptUniqueId () const
{
	return m_lastSitOnAttemptUniqueId;
}

//----------------------------------------------------------------------

inline int PlayerObject::getMaxForcePower() const
{
	return m_maxForcePower.get();
}

//------------------------------------------------------------------------

inline int PlayerObject::getForcePower() const
{
	return m_forcePower.get();
}

//----------------------------------------------------------------------

inline int PlayerObject::getJediState() const
{
	return m_jediState.get();
}

//----------------------------------------------------------------------

inline bool PlayerObject::isNormalPlayer() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isCustomerServiceRepresentative() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::CustomerServiceRepresentative);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isDeveloper() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::Developer);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isWarden() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::Warden);
}

//----------------------------------------------------------------------

inline bool PlayerObject::isQualityAssurance() const
{
	return m_privledgedTitle.get() == static_cast<int8>(PlayerDataPriviledgedTitle::QualityAssurance);
}

//----------------------------------------------------------------------

inline int PlayerObject::getRoleIconChoice() const
{
	return m_roleIconChoice.get();
}

//----------------------------------------------------------------------

inline std::string const & PlayerObject::getWorkingSkill() const
{
	return m_workingSkill.get();
}

//----------------------------------------------------------------------

inline std::string const & PlayerObject::getSkillTemplate() const
{
	return m_skillTemplate.get();
}

//----------------------------------------------------------------------

inline uint32 PlayerObject::getCurrentQuest() const
{
	return m_currentQuest.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getCurrentGcwPoints() const
{
	return m_currentGcwPoints.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getCurrentPvpKills() const
{
	return m_currentPvpKills.get();
}

// ----------------------------------------------------------------------

inline int64 PlayerObject::getLifetimeGcwPoints() const
{
	return m_lifetimeGcwPoints.get();
}

// ----------------------------------------------------------------------

inline int32 PlayerObject::getLifetimePvpKills() const
{
	return m_lifetimePvpKills.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getCurrentGcwRank() const
{
	return m_currentGcwRank.get();
}

// ----------------------------------------------------------------------

inline float PlayerObject::getCurrentGcwRankProgress() const
{
	return m_currentGcwRankProgress.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getMaxGcwImperialRank() const
{
	return m_maxGcwImperialRank.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getMaxGcwRebelRank() const
{
	return m_maxGcwRebelRank.get();
}

// ----------------------------------------------------------------------

inline time_t PlayerObject::getGcwRatingActualCalcTime() const
{
	return m_gcwRatingActualCalcTimeClientEpoch;
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getPetId() const
{
	return m_petId.get();
}

// ----------------------------------------------------------------------

inline std::vector<std::string> const & PlayerObject::getPetCommands() const
{
	return m_petCommandList.get();
}

// ----------------------------------------------------------------------

inline std::vector<std::string> const & PlayerObject::getPetToggledCommands() const
{
	return m_petToggledCommands.get();
}

// ----------------------------------------------------------------------

inline bool PlayerObject::cityGcwDefenderRegionTitleAvailable() const
{
	return (!m_cityGcwDefenderRegion.get().first.empty() && m_cityGcwDefenderRegion.get().second.second);
}

// ----------------------------------------------------------------------

inline bool PlayerObject::guildGcwDefenderRegionTitleAvailable() const
{
	return (!m_guildGcwDefenderRegion.get().first.empty() && m_guildGcwDefenderRegion.get().second.second);
}

// ----------------------------------------------------------------------

inline NetworkId const & PlayerObject::getSquelchedById() const
{
	return m_squelchedById.get();
}

// ----------------------------------------------------------------------

inline std::string const & PlayerObject::getSquelchedByName() const
{
	return m_squelchedByName.get();
}

//----------------------------------------------------------------------

inline bool PlayerObject::getShowBackpack() const
{
	return m_showBackpack.get();
}

//----------------------------------------------------------------------

inline bool PlayerObject::getShowHelmet() const
{
	return m_showHelmet.get();
}

inline EnvironmentInfo const & PlayerObject::getEnvironmentFlags() const
{
	return m_environmentFlags.get();
}

//----------------------------------------------------------------------

inline std::string const & PlayerObject::getDefaultAttackOverride() const
{
	return m_defaultAttackOverride.get();
}

//----------------------------------------------------------------------

inline bool PlayerObject::isAdmin()
{
	return s_admin;
}

// ----------------------------------------------------------------------

inline BitArray const & PlayerObject::getGuildRank() const
{
	return m_guildRank.get();
}

// ----------------------------------------------------------------------

inline BitArray const & PlayerObject::getCitizenRank() const
{
	return m_citizenRank.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getAccountNumLotsOverLimitSpam() const
{
	return m_accountNumLotsOverLimitSpam.get();
}

// ----------------------------------------------------------------------

inline int8 PlayerObject::getGalacticReserveDeposit() const
{
	return m_galacticReserveDeposit.get();
}

// ----------------------------------------------------------------------

inline int64 PlayerObject::getPgcRatingCount() const
{
	return m_pgcRatingCount.get();
}

// ----------------------------------------------------------------------

inline int64 PlayerObject::getPgcRatingTotal() const
{
	return m_pgcRatingTotal.get();
}

// ----------------------------------------------------------------------

inline int PlayerObject::getPgcLastRatingTime() const
{
	return m_pgcLastRatingTime.get();
}

//----------------------------------------------------------------------

#endif	// INCLUDED_PlayerObject_H
