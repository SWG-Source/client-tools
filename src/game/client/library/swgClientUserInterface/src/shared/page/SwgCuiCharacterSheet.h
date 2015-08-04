// ======================================================================
//
// SwgCuiCharacterSheet.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef SwgCuiCharacterSheet_H
#define SwgCuiCharacterSheet_H

//-----------------------------------------------------------------

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"

#include "clientGame/PlayerCreatureController.h" //needed for inner class reference

#include <vector>
#include <map>

//-----------------------------------------------------------------

class CachedNetworkId;
class CreatureObject;
class CuiCreatureAttributeTable;
class CuiLoginManagerAvatarInfo;
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class SwgCuiFaction;
class UIButton;
class UIDataSource;
class UIImage;
class UIList;
class UIPage;
class UITabbedPane;
class UITable;
class UIText;
class PlayerObject;
class SwgCuiInventoryEquipment;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

template <typename T> class Watcher;

//-----------------------------------------------------------------

class SwgCuiCharacterSheet :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	struct Messages
	{
		struct ShowStatMigration
		{
			typedef NetworkId Payload;
		};
	};

	static SwgCuiCharacterSheet*  createInto         (UIPage & parent);

public:
	explicit                   SwgCuiCharacterSheet  (UIPage & page);
	void                       OnButtonPressed(UIWidget *context);
	void                       receiveMessage        (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
	void                       setExamineMode        (CreatureObject *playerToExamine);
	bool                       isExaminingSelf       () const;
	CreatureObject*            getCreatureToExamine  () const;

protected:
	void                       updateGuild           (const std::string &guildName, const std::string &memberTitle);
	void                       update                (float deltaTimeSecs);
	void                       requestServerInfo     () const;
	void                       updateFactionTable    ();
	void                       updateSkillModsTable  ();
	void                       performActivate       ();
	void                       performDeactivate     ();

private:
	                          ~SwgCuiCharacterSheet  ();
	                           SwgCuiCharacterSheet  (const SwgCuiCharacterSheet &);
	SwgCuiCharacterSheet &    operator=              (const SwgCuiCharacterSheet &);
	void                       refreshBadgeWindow    ();
	void                       checkCybernetics      ();
	void                       updateAttributes      ();
	void					   updateChroniclesInfo  ();
	int getSkillModValue(CreatureObject const & creature, std::string const & skillModName);

	void                       onSkillModsChanged    (const CreatureObject & creature);

	void                       updateGcwPage         ();
	void                       formatHighRankText    (uint32 factionCrc, int maxRank, int format);

private:
	UITabbedPane*              m_tabbedPane;
	long                       m_lastActiveTab;

	//data for the title bar
	UIText*                    m_rank;

	//data for status page
	UIText*                    m_characterName;
	UIText*                    m_pvpStatus;

	//data for personal page
	UIText*                    m_title;
	UIText*                    m_born;
	UIText*                    m_species;
	UIText*                    m_played;
	UIText*                    m_home;
	UIText*                    m_married;
	UIText*                    m_bindLocation;
	UIText*                    m_lotsAvailable;
	UIText*                    m_guild;
	UIText*                    m_guildAbbreviation;
	UIText*                    m_guildTitle;
	UIText*                    m_badgeWindow;
	UIText*                    m_rebelFaction;
	UIText*                    m_imperialFaction;
	UIButton*                  m_cyberneticsButton;
	UIButton*                  m_buttonCollections;
	UIPage*                    m_personalPageTop;

	UITable*                   m_tableFactions;
	std::map<std::string, int> m_factions;


	UIImage* m_healthBar;
	UIImage* m_healthBarBack;
	UIImage* m_actionBar;
	UIImage* m_actionBarBack;
	
	UIText* m_levelValue;
	UIText* m_attrHealthValue;
	UIText* m_attrActionValue;
	UIText* m_attrStrengthValue;
	UIText* m_attrConstitutionValue;
	UIText* m_attrPrecisionValue;
	UIText* m_attrAgilityValue;
	UIText* m_attrStaminaValue;
	UIText* m_attrLuckValue;
	UIText* m_attrKineticValue;
	UIText* m_attrEnergyValue ;
	UIText* m_attrHeatValue;
	UIText* m_attrColdValue;
	UIText* m_attrAcidValue;
	UIText* m_attrElectricityValue;

	UITable* m_skillModsTable;


	MessageDispatch::Callback* m_callBack;

	SwgCuiInventoryEquipment * m_eqMediator;

	// GCW page
	UIText* m_gcwRank;
	UIText* m_gcwPoints;
	UIText* m_gcwPvPKills;
	UIText* m_gcwLifetime;
	UIText* m_gcwPvPLifetime;
	UIText* m_gcwHighRankReb;
	UIText* m_gcwHighRankImp;
	UIText* m_gcwHighRankRebLabel;
	UIText* m_gcwHighRankImpLabel;
	UIText* m_gcwPercent;
	UIText* m_gcwPercentSign;
	UIText* m_gcwProgressText;
	UIText* m_gcwTimerLabel;
	UIText* m_gcwTimer;

	UIPage* m_gcwImperialProgBar;
	UIPage* m_gcwRebelProgBar;
	UIPage* m_gcwProgressBarBack;

	UIImage* m_gcwBackgroundRebel;
	UIImage* m_gcwBackgroundImperial;

	UIComposite* m_characterAttributesComp;

	typedef Watcher<PlayerObject> PlayerObjectWatcher;
	PlayerObjectWatcher *m_playerObjectWatcher;

	typedef Watcher<CreatureObject> CreatureObjectWatcher;
	CreatureObjectWatcher *m_creatureObjectWatcher;

	UIText * m_chroniclesRating;
	UIText * m_chroniclesUpdate;
};

//-----------------------------------------------------------------

#endif

