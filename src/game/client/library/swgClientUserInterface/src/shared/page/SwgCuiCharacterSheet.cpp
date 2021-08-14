// ======================================================================
//
// SwgCuiCharacterSheet.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCharacterSheet.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"


#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GuildObject.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientGame/Species.h"
#include "clientGame/WeaponObject.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsCharacterSheet.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/PvpData.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/CharacterSheetResponseMessage.h"
#include "sharedNetworkMessages/FactionRequestMessage.h"
#include "sharedNetworkMessages/FactionResponseMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GuildRequestMessage.h"
#include "sharedNetworkMessages/GuildResponseMessage.h"
#include "sharedNetworkMessages/PlayerMoneyRequest.h"
#include "sharedNetworkMessages/PlayerMoneyResponse.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgClientUserInterface/SwgCuiFaction.h"
#include "swgClientUserInterface/SwgCuiInventoryEquipment.h"
#include "UIComposite.h"

//-----------------------------------------------------------------------

namespace SwgCuiCharacterSheetNamespace
{	
	const Unicode::String s_guildTagPrefix(Unicode::narrowToWide(" <"));
	const Unicode::String s_guildTagSuffix(Unicode::narrowToWide(">"));
	const Unicode::String s_guildMemberTitlePrefix(Unicode::narrowToWide(" ["));
	const Unicode::String s_guildMemberTitleSuffix(Unicode::narrowToWide("]"));

	float ms_updateCheckTimer = 0.0f;
	float UPDATE_CHECK_THRESHOLD = 1.0f;
	std::vector<std::string> cms_cyberneticWearableSlots;
	enum SkillModTypes 
	{
		SMT_strength,
		SMT_constitution,
		SMT_stamina,
		SMT_precision,
		SMT_agility,
		SMT_luck,
		SMT_kinetic,
		SMT_energy,
		SMT_heat,
		SMT_cold,
		SMT_acid,
		SMT_electricity,

		SMT_numSmts
	};
	char const * const ms_statMods[SMT_numSmts] = 
	{
			"strength",
			"constitution",
			"stamina",
			"precision",
			"agility",
			"luck",
			"kinetic",
			"energy",
			"heat",
			"cold",
			"acid",
			"electricity"
	};

	// data that controls how skill mods are laid out on the character sheet
	struct SkillModListingRecord {
		std::string profession;
		std::string category;
		bool display;
		int  rank;
		bool percent;
		int  divisor;
	};
	// Index is the skill or category name
	std::map<std::string, SkillModListingRecord> s_skillModListingRecords;
	void loadSkillModListingDatatable();
	std::string const s_miscCategoryName = "misc";
	std::string const s_allProfessionName = "ALL";
	void getSkillModsForDisplay(UITableModelDefault *model);

	int const s_numGcwRankIcons = 12;

	UIImage* s_gcwRebelRankIcons[s_numGcwRankIcons];
	UIImage* s_gcwImperialRankIcons[s_numGcwRankIcons];

	enum GcwHighRankFormat
	{
		FMT_top,
		FMT_bottom
	};

	enum TabPages
	{
		TAB_status,
		TAB_personal,
		TAB_factions,
		TAB_skillMods,
		TAB_gcw,

		TAB_numTabPages
	};

	UIPoint s_tabLocations[TAB_numTabPages];
}

using namespace SwgCuiCharacterSheetNamespace;

void SwgCuiCharacterSheetNamespace::loadSkillModListingDatatable()
{
	if(!s_skillModListingRecords.empty())
		return;
	DataTable *t = DataTableManager::getTable("datatables/expertise/skill_mod_listing.iff", true);
	for (int row = 0; row < t->getNumRows(); ++row)
	{
		SkillModListingRecord smlr;
		smlr.profession = t->getStringValue("profession", row);
		smlr.category = t->getStringValue("category", row);
		smlr.display = (t->getIntValue("display", row) != 0);
		smlr.rank = t->getIntValue("rank", row);
		smlr.percent = (t->getIntValue("percent", row) != 0);
		smlr.divisor = (t->getIntValue("divisor", row));
		
		IGNORE_RETURN(s_skillModListingRecords.insert(std::make_pair(t->getStringValue("skill_mod", row), smlr)));	

	}
	DataTableManager::close("datatables/expertise/skill_mod_listing.iff");
}

//-----------------------------------------------------------------------

// For more information about how the skill mods are displayed, see
// http://wiki.station.sony.com/twiki/bin/view/SWG/SWGDesignDocumentsSystemsSkillModDisplay

void SwgCuiCharacterSheetNamespace::getSkillModsForDisplay(UITableModelDefault *model)
{
	struct SkillModSubRecord
	{
		std::string name;
		int amount;
		int rank;
		bool percent;
		int  divisor;
	};
	struct CategoryRecord
	{
		std::string categoryName;
		std::vector<SkillModSubRecord> skillModsInThisCategory;
		int rank;
	};

	// A map of categories to records which list, among other things, the mods in that category
	std::map<std::string, CategoryRecord> m_categories;

	// A vector that chains through the above map; we will use it to sort the categories by rank
	// because sorting a map in place is hard.  
	std::vector<CategoryRecord *> m_categoriesInRankOrder;
	
	
	CreatureObject *player = Game::getPlayerCreature();
	CreatureObject::SkillModMap modMap = player->getSkillModMap();
	for(CreatureObject::SkillModMap::iterator i = modMap.begin(); i != modMap.end(); ++i)
	{
		std::pair<std::string, std::pair<int, int> > p = *i;
		std::string skillModName = p.first;
		int skillModAmount = p.second.first;
		int skillModBonus = p.second.second;   // True skill mod is actually first, or base, plus second, or bonus
		//DEBUG_WARNING(true, ("GSMFD: processing skillMod name %s value %d bonus %d", skillModName.c_str(), skillModAmount, skillModBonus));
		std::map<std::string, SkillModListingRecord>::iterator i2 = s_skillModListingRecords.find(skillModName);
		if(i2 == s_skillModListingRecords.end())
		{
			//DEBUG_WARNING(true, ("GSMFD: processing skillMod name %s was not found in datatable", skillModName.c_str()));
			continue;
		}
		SkillModListingRecord & smlr = (*i2).second;
		//DEBUG_WARNING(true, ("GSMFD: name %s profession %s category %s display %d rank %d",
		//	skillModName.c_str(), smlr.profession.c_str(), smlr.category.c_str(), smlr.display, smlr.rank));
		if(!smlr.display)
		{
			//DEBUG_WARNING(true, ("GSMFD: processing skillMod name %s has display set to false", skillModName.c_str()));
			continue;
		}
		std::string const & professionName = smlr.profession;
		std::string & categoryName = smlr.category;
		if( (professionName != CuiSkillManager::getSkillTemplate()) && (professionName != s_allProfessionName) )
		{
			//DEBUG_WARNING(true, ("GSMFD: professionName %s didn't match skill template %s, changing to %s",
			//	professionName.c_str(), CuiSkillManager::getSkillTemplate().c_str(), s_miscCategoryName.c_str()));
			categoryName = s_miscCategoryName;
		}
		
		//See if this category already exists
		std::map<std::string, CategoryRecord>::iterator i3 = m_categories.find(categoryName);
		if(i3 != m_categories.end())
		{
			//DEBUG_WARNING(true, ("GSMFD: category already existed, adding record"));
			CategoryRecord & cr = (*i3).second;		
			SkillModSubRecord smsr;
			smsr.name = skillModName;
			smsr.amount = skillModAmount + skillModBonus;
			smsr.rank = smlr.rank;
			smsr.percent = smlr.percent;
			smsr.divisor = smlr.divisor;
			cr.skillModsInThisCategory.push_back(smsr);
		}
		else
		{
			//DEBUG_WARNING(true, ("GSMFD: category didn't exist, adding category and record"));
			CategoryRecord cr;
			cr.categoryName = categoryName;
			SkillModSubRecord smsr;
			smsr.name = skillModName;
			smsr.amount = skillModAmount + skillModBonus;
			smsr.rank = smlr.rank;
			smsr.percent = smlr.percent;
			smsr.divisor = smlr.divisor;
			cr.skillModsInThisCategory.push_back(smsr);
			
			std::map<std::string, SkillModListingRecord>::iterator i4 = s_skillModListingRecords.find(categoryName);
			if(i4 == s_skillModListingRecords.end())
				cr.rank = -1;
			else
				cr.rank = (*i4).second.rank;
			m_categories.insert(std::make_pair(categoryName, cr));
			i3 = m_categories.find(categoryName);
			DEBUG_FATAL(i3 == m_categories.end(), ("GSMFD: category wasn't found right after add"));
			//Here we're storing addresses but it's OK because all this memory is cleared up through local refs
			//DEBUG_WARNING(true, ("GSMFD: addresses %x %x", &cr, &( (*i3).second) ));
			m_categoriesInRankOrder.push_back(&( (*i3).second ));
		}
	}

	//Sort the categories by their rank
	unsigned int ci;
	for(ci = 0; ci < m_categoriesInRankOrder.size(); ++ci)
	{
		for(unsigned int cj = ci + 1; cj < m_categoriesInRankOrder.size(); ++cj)
		{
			CategoryRecord *cri = m_categoriesInRankOrder[ci];
			CategoryRecord *crj = m_categoriesInRankOrder[cj];
			bool outOfOrder = false;
			if(cri->rank < crj->rank)
				outOfOrder = true;
			else if((cri->rank == crj->rank) && (cri->categoryName > crj->categoryName))
				outOfOrder = true;
			if(outOfOrder)
			{
				m_categoriesInRankOrder[ci] = crj;
				m_categoriesInRankOrder[cj] = cri;
			}
		}
	}

	//Sort the skills in each category
	for(ci = 0; ci < m_categoriesInRankOrder.size(); ++ci)
	{

		CategoryRecord *cri = m_categoriesInRankOrder[ci];
		bool isMiscCategory = (cri->categoryName == s_miscCategoryName);
		for(unsigned int ck = 0; ck < cri->skillModsInThisCategory.size(); ++ck)
		{
			for(unsigned int cm = ck + 1; cm < cri->skillModsInThisCategory.size(); ++cm)
			{
				boolean outOfOrder = false;
				SkillModSubRecord &srk = cri->skillModsInThisCategory[ck];
				SkillModSubRecord &srm = cri->skillModsInThisCategory[cm];
				if(!isMiscCategory && (srk.rank < srm.rank))
					outOfOrder = true;
				else if((isMiscCategory || (srk.rank == srm.rank)) && (srk.name > srm.name))
					outOfOrder = true;
				if(outOfOrder)
				{
					SkillModSubRecord tmp = srk;
					cri->skillModsInThisCategory[ck] = srm;
					cri->skillModsInThisCategory[cm] = tmp;
				}
			}
		}
	}

	// Now we have a vector of categories, each of which has a vector of skill mods.  Use these to
	// fill out the UIDataTableModel.
	int rowCount = 0;
	for(ci = 0; ci < m_categoriesInRankOrder.size(); ++ci)
	{

		CategoryRecord *cri = m_categoriesInRankOrder[ci];
		StringId catNameId("skillmod_category_n", cri->categoryName.c_str());
		Unicode::String catName;
		catName = Unicode::narrowToWide("+ \\#ff0000");
		catName.append(catNameId.localize());		
		UIData * const data0 = model->AppendCell(0, cri->categoryName.c_str(), catName);
		UIData * const data1 = model->AppendCell(1, 0, Unicode::emptyString);
		UNREF(data0);
		UNREF(data1);
		rowCount++;
		for(unsigned int ck = 0; ck < cri->skillModsInThisCategory.size(); ++ck)
		{
			Unicode::String skillModName;
			skillModName = Unicode::narrowToWide("\\#.\\>040");
			Unicode::String localizedName;
			CuiSkillManager::localizeSkillModName(cri->skillModsInThisCategory[ck].name, localizedName);
			skillModName.append(localizedName);
			
			IGNORE_RETURN(model->AppendCell(0, cri->skillModsInThisCategory[ck].name.c_str(), skillModName));
						
			if (cri->skillModsInThisCategory[ck].percent)
				IGNORE_RETURN(model->AppendCell(1, 0, Unicode::narrowToWide(FormattedString<128>().sprintf("%.2f %%",float (cri->skillModsInThisCategory[ck].amount) / float (cri->skillModsInThisCategory[ck].divisor)))));
			else
				IGNORE_RETURN(model->AppendCell(1, 0, Unicode::narrowToWide(FormattedString<128>().sprintf("%d", cri->skillModsInThisCategory[ck].amount))));
								
			UIString extendedSkillModDescription;
			CuiSkillManager::localizeSkillModExtendedDesc(cri->skillModsInThisCategory[ck].name, extendedSkillModDescription);
			model->SetTooltipAt(rowCount, 0, extendedSkillModDescription);
			model->SetTooltipAt(rowCount, 1, Unicode::emptyString);
			rowCount++;

		}
	}
	return;
}

//-----------------------------------------------------------------------

SwgCuiCharacterSheet::SwgCuiCharacterSheet(UIPage & page)
: 
CuiMediator              ("SwgCuiCharacterSheet", page),
UIEventCallback          (),
MessageDispatch::Receiver(),
m_tabbedPane             (0),
m_lastActiveTab          (0),
m_rank                   (0),
m_characterName          (0),
m_pvpStatus              (0),
m_title                  (0),
m_born                   (0),
m_species                (0),
m_played                 (0),
m_home                   (0),
m_married                (0),
m_bindLocation           (0),
m_lotsAvailable          (0),
m_guild                  (0),
m_guildAbbreviation      (0),
m_guildTitle             (0),
m_badgeWindow            (0),
m_rebelFaction           (0),
m_imperialFaction        (0),
m_cyberneticsButton      (0),
m_buttonCollections      (0),
m_personalPageTop        (0),
m_tableFactions          (0),
m_factions(),
m_healthBar              (0),
m_healthBarBack          (0),
m_actionBar              (0),
m_actionBarBack          (0),
m_levelValue             (0),
m_attrHealthValue        (0),
m_attrActionValue        (0),
m_attrStrengthValue      (0),
m_attrConstitutionValue  (0),
m_attrStaminaValue       (0),
m_attrPrecisionValue     (0),
m_attrAgilityValue       (0),
m_attrLuckValue          (0),
m_attrKineticValue       (0),
m_attrEnergyValue        (0),
m_attrHeatValue          (0),
m_attrColdValue          (0),
m_attrAcidValue          (0),
m_attrElectricityValue   (0),
m_skillModsTable         (0),
m_callBack(new MessageDispatch::Callback),
m_eqMediator             (0),
m_gcwRank                (0),
m_gcwPoints              (0),
m_gcwPvPKills            (0),
m_gcwLifetime            (0),
m_gcwPvPLifetime         (0),
m_gcwHighRankReb         (0),
m_gcwHighRankImp         (0),
m_gcwHighRankRebLabel    (0),
m_gcwHighRankImpLabel    (0),
m_gcwPercent             (0),
m_gcwPercentSign         (0),
m_gcwProgressText        (0),
m_gcwTimerLabel          (0),
m_gcwTimer               (0),
m_gcwImperialProgBar     (0),
m_gcwRebelProgBar        (0),
m_gcwProgressBarBack     (0),
m_gcwBackgroundRebel     (0),
m_gcwBackgroundImperial  (0),
m_characterAttributesComp(0),
m_playerObjectWatcher    (new PlayerObjectWatcher),
m_creatureObjectWatcher  (new CreatureObjectWatcher),
m_chroniclesRating		 (0),
m_chroniclesUpdate       (0)
{
	loadSkillModListingDatatable();

	//get codedatas for shared items
	getCodeDataObject(TUIButton,     m_cyberneticsButton,   "buttonCybernetics");
	getCodeDataObject(TUIButton,     m_buttonCollections,   "buttonCollections");
	getCodeDataObject(TUITabbedPane, m_tabbedPane,          "tabs");
	getCodeDataObject(TUITable,      m_tableFactions,       "tableFactions");

	//get codedatas for the status page
	
	UIPage* factionPage       = 0;

	getCodeDataObject(TUIPage,   factionPage   ,       "pageFactions");
	getCodeDataObject(TUIText,   m_characterName,      "textCharacterName");
	getCodeDataObject(TUIText,   m_rank,               "rank");	
	getCodeDataObject(TUIText,   m_pvpStatus,          "textPvPStatus");
	getCodeDataObject(TUIText,   m_rebelFaction,       "factionRebelText");
	getCodeDataObject(TUIText,   m_imperialFaction,    "factionImperialText");

	getCodeDataObject(TUIImage,  m_healthBar,            "attrHealthBar");
	getCodeDataObject(TUIImage,  m_healthBarBack,        "attrHealthBarBack");
	getCodeDataObject(TUIImage,  m_actionBar,            "attrActionBar");
	getCodeDataObject(TUIImage,  m_actionBarBack,        "attrActionBarBack");

	getCodeDataObject(TUIText, m_levelValue, "levelValue");
	getCodeDataObject(TUIText, m_attrHealthValue, "attrHealthValue");
	m_attrHealthValue->SetTextColor(UIColor::white);
	getCodeDataObject(TUIText, m_attrActionValue,"attrActionValue");
	m_attrActionValue->SetTextColor(UIColor::white);
	getCodeDataObject(TUIText, m_attrStrengthValue,"attrStrengthValue");
	getCodeDataObject(TUIText, m_attrConstitutionValue,"attrConstitutionValue");
	getCodeDataObject(TUIText, m_attrStaminaValue,"attrStaminaValue");
	getCodeDataObject(TUIText, m_attrPrecisionValue,"attrPrecisionValue");
	getCodeDataObject(TUIText, m_attrAgilityValue,"attrAgilityValue");
	getCodeDataObject(TUIText, m_attrLuckValue,"attrLuckValue");
	getCodeDataObject(TUIText, m_attrKineticValue,"attrKineticValue");
	getCodeDataObject(TUIText, m_attrEnergyValue,"attrEnergyValue");
	getCodeDataObject(TUIText, m_attrHeatValue,"attrHeatValue");
	getCodeDataObject(TUIText, m_attrColdValue,"attrColdValue");
	getCodeDataObject(TUIText, m_attrAcidValue,"attrAcidValue");
	getCodeDataObject(TUIText, m_attrElectricityValue,"attrElectricityValue");
	
	getCodeDataObject(TUITable, m_skillModsTable, "skillsText");

	//set up data for the status page
	m_characterName->Clear   ();
	m_rank->Clear            ();
	m_pvpStatus->Clear       ();
	m_rebelFaction->Clear    ();
	m_imperialFaction->Clear ();

	//get codedatas for the personal page
	getCodeDataObject(TUIText,   m_title,            "title");
	getCodeDataObject(TUIText,   m_born,             "bornDate");
	getCodeDataObject(TUIText,   m_species,          "species");
	getCodeDataObject(TUIText,   m_home,             "home");
	getCodeDataObject(TUIText,   m_played,           "playedTime");
	getCodeDataObject(TUIText,   m_married,          "married");
	getCodeDataObject(TUIText,   m_badgeWindow,      "badges");
	getCodeDataObject(TUIText,   m_bindLocation,     "bindLocation");
	getCodeDataObject(TUIText,   m_lotsAvailable,    "lotsAvailable");
	getCodeDataObject(TUIText,   m_guild,            "guild");
	getCodeDataObject(TUIText,   m_guildAbbreviation,"guildAbbreviation");
	getCodeDataObject(TUIText,   m_guildTitle,       "title");
	getCodeDataObject(TUIPage,   m_personalPageTop,  "personalTop");

	UIPage * eqPage = 0;
	getCodeDataObject (TUIPage,  eqPage,             "paperDoll");

	m_eqMediator  = new SwgCuiInventoryEquipment (*eqPage);
	m_eqMediator->fetch ();

	//set up data for the personal page
	m_title->Clear             ();
	m_born->Clear              ();
	m_species->Clear           ();
	m_home->Clear              ();
	m_played->Clear            ();
	m_married->Clear           ();
	m_bindLocation->Clear      ();
	m_lotsAvailable->Clear     ();
	m_guild->Clear             ();
	m_guildAbbreviation->Clear ();
	m_guildTitle->Clear        ();
	m_badgeWindow->Clear       ();
	m_badgeWindow->SetPreLocalized (true);
	setState   (MS_closeable);
	setState   (MS_closeDeactivates);
	removeState(MS_iconifiable);

	m_born->Clear   ();
	m_played->Clear ();
	m_bindLocation->SetLocalText  (CuiStringIdsCharacterSheet::unknown.localize());
	m_lotsAvailable->SetLocalText (CuiStringIdsCharacterSheet::unknown.localize());
	m_tabbedPane->SetActiveTab (-1);
	m_tabbedPane->SetActiveTab (0);

	updateFactionTable         ();
	updateSkillModsTable       ();

	cms_cyberneticWearableSlots.clear();
	cms_cyberneticWearableSlots.push_back("bracer_lower_l");
	cms_cyberneticWearableSlots.push_back("bracer_lower_r");
	cms_cyberneticWearableSlots.push_back("pants1");
	cms_cyberneticWearableSlots.push_back("chest2");
	cms_cyberneticWearableSlots.push_back("cybernetic_hand_l");
	cms_cyberneticWearableSlots.push_back("cybernetic_hand_r");


	// get codedatas for gcw page
	getCodeDataObject(TUIText,   m_gcwRank,             "gcwRank");  
	getCodeDataObject(TUIText,   m_gcwPoints,           "gcwPoints");
	getCodeDataObject(TUIText,   m_gcwPvPKills,         "gcwPvPKills");
	getCodeDataObject(TUIText,   m_gcwLifetime,         "gcwLifetime");
	getCodeDataObject(TUIText,   m_gcwPvPLifetime,      "gcwPvPLifetime");
	getCodeDataObject(TUIText,   m_gcwHighRankReb,      "gcwHighRankReb");
	getCodeDataObject(TUIText,   m_gcwHighRankImp,      "gcwHighRankImp");
	getCodeDataObject(TUIText,   m_gcwPercent,          "gcwPercent");
	getCodeDataObject(TUIText,   m_gcwPercentSign,      "gcwPercentSign");
	getCodeDataObject(TUIPage,   m_gcwImperialProgBar,  "gcwImperialProgBar");
	getCodeDataObject(TUIPage,   m_gcwRebelProgBar,     "gcwRebelProgBar");
	getCodeDataObject(TUIImage,  m_gcwBackgroundRebel,  "gcwBackRebel");
	getCodeDataObject(TUIImage, m_gcwBackgroundImperial,"gcwBackImperial");
	getCodeDataObject(TUIText,   m_gcwHighRankRebLabel, "gcwHighRebLabel");
	getCodeDataObject(TUIText,   m_gcwHighRankImpLabel, "gcwHighImpLabel");
	getCodeDataObject(TUIPage,   m_gcwProgressBarBack,  "gcwProgressBarback");
	getCodeDataObject(TUIText,   m_gcwProgressText,     "gcwProgressText");
	getCodeDataObject(TUIPage,   m_gcwProgressBarBack,  "gcwProgressbarBack");
	getCodeDataObject(TUIComposite, m_characterAttributesComp, "characterAttributes");
	getCodeDataObject(TUIText,   m_gcwTimerLabel,       "gcwTimerText");
	getCodeDataObject(TUIText,   m_gcwTimer,             "gcwTimer");

	std::string rebelIcon = "gcwRebelRankIcon";
	std::string imperialIcon = "gcwImperialRankIcon";
	for (int i = 0; i < s_numGcwRankIcons; ++i)
	{
		char buff[8];
		_itoa(i+1, buff, 10);

		std::string s = rebelIcon + buff;
		getCodeDataObject(TUIImage,	s_gcwRebelRankIcons[i],	s.c_str());	

		s.clear();
		s = imperialIcon + buff;
		getCodeDataObject(TUIImage, s_gcwImperialRankIcons[i], s.c_str());
	}
	
	// set up data for gcw page
	m_gcwPoints->Clear		();
	m_gcwPvPKills->Clear	();
	m_gcwLifetime->Clear	();
	m_gcwPvPLifetime->Clear	();
	m_gcwHighRankReb->Clear	();
	m_gcwHighRankImp->Clear	();
	m_gcwPercent->Clear		();
	m_gcwTimer->Clear       ();

	for (int j = 0; j < TAB_numTabPages; ++j)
	{
		UIButton * const button = m_tabbedPane->GetTabButton(j);
		s_tabLocations[j] = button->GetLocation();
	}

	getCodeDataObject(TUIText, m_chroniclesRating, "chronicleRating");
	getCodeDataObject(TUIText, m_chroniclesUpdate, "chronicleUpdate");

	m_chroniclesRating->Clear();
	m_chroniclesUpdate->Clear();
}

//-----------------------------------------------------------------------

SwgCuiCharacterSheet::~SwgCuiCharacterSheet()
{
	if (m_eqMediator)
	{
		m_eqMediator->release ();
		m_eqMediator = 0;
	}

	delete m_callBack;
	m_callBack = NULL;

	delete m_playerObjectWatcher;
	m_playerObjectWatcher = NULL;

	delete m_creatureObjectWatcher;
	m_creatureObjectWatcher = NULL;

}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::performActivate()
{
	CuiManager::requestPointer(true);

	if (m_creatureObjectWatcher->getPointer())
	{
		const FactionRequestMessage factionMsg;
		GameNetwork::send(factionMsg, true);

		m_characterName->SetLocalText(m_creatureObjectWatcher->getPointer()->getLocalizedName());

		const ObjectTemplate* ot = m_creatureObjectWatcher->getPointer()->getObjectTemplate();
		if (ot)
		{
			const SharedCreatureObjectTemplate* scot = safe_cast<const SharedCreatureObjectTemplate*>(ot);
			if (scot)
			{
				const Unicode::String & s = Species::getLocalizedName(scot->getSpecies());
				m_species->SetText(s);
			}
		}

		// Update the faction rank
		if (m_playerObjectWatcher->getPointer())
			m_rank->SetLocalText(CreatureObject::getLocalizedGcwRankString(m_playerObjectWatcher->getPointer()->getCurrentGcwRank(), m_creatureObjectWatcher->getPointer()->getPvpFaction()));
	}
	else
	{
		m_characterName->SetLocalText(Unicode::String());
		m_rank->SetLocalText(Unicode::emptyString);
	}

	
	if (m_playerObjectWatcher->getPointer())
	{
		char buf[256];
		int born = m_playerObjectWatcher->getPointer()->getBornDate();
		//set born info
		_itoa(born, buf, 10);
		m_born->SetLocalText(Unicode::narrowToWide(buf));
		//set played info
		int played = m_playerObjectWatcher->getPointer()->getPlayedTime();
		_itoa(played, buf, 10);
		m_played->SetLocalText(Unicode::narrowToWide(buf));
	}

	m_home->SetText(CuiStringIdsCharacterSheet::homeless.localize());

	m_married->SetText(CuiStringIdsCharacterSheet::unmarried.localize());

	connectToMessage(PlayerMoneyResponse::MessageType);
	connectToMessage(FactionResponseMessage::MessageType);
	connectToMessage(GuildResponseMessage::MessageType);
	connectToMessage(CharacterSheetResponseMessage::cms_name);
	connectToMessage("CharacterSheetResponseResLoc");

	requestServerInfo();
	
	setIsUpdating (true);

	updateFactionTable();
	updateSkillModsTable();
	m_cyberneticsButton->AddCallback(this);
	m_buttonCollections->AddCallback(this);
	ClientObject * const player = dynamic_cast<ClientObject*>(m_creatureObjectWatcher->getPointer());
	if (player)
	{
		m_eqMediator->activate ();
		m_eqMediator->setupCharacterViewer        (player);
	}

	checkCybernetics();
	updateAttributes();
	updateGcwPage();

	if (m_playerObjectWatcher->getPointer())
		refreshBadgeWindow();

	m_callBack->connect (*this, &SwgCuiCharacterSheet::onSkillModsChanged, static_cast<CreatureObject::Messages::SkillModsChanged *>(0));
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::performDeactivate()
{
	setIsUpdating (false);
	m_eqMediator->deactivate();
	
	m_callBack->disconnect (*this, &SwgCuiCharacterSheet::onSkillModsChanged,                  static_cast<CreatureObject::Messages::SkillModsChanged *>(0));

	disconnectFromMessage("CharacterSheetResponseResLoc");
	disconnectFromMessage(CharacterSheetResponseMessage::cms_name);
	disconnectFromMessage(GuildResponseMessage::MessageType);
	disconnectFromMessage(FactionResponseMessage::MessageType);
	disconnectFromMessage(PlayerMoneyResponse::MessageType);

	CuiManager::requestPointer   (false);
	m_cyberneticsButton->RemoveCallback(this);
	m_buttonCollections->RemoveCallback(this);

	if (isExaminingSelf())
		m_lastActiveTab = m_tabbedPane->GetActiveTab();
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if (message.isType(FactionResponseMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const FactionResponseMessage msg(ri);

		int rebel    = msg.getFactionRebel();
		int imperial = msg.getFactionImperial();
		char buf[256];
		_itoa(rebel, buf, 10);
		m_rebelFaction->SetLocalText(Unicode::narrowToWide(buf));
		_itoa(imperial, buf, 10);
		m_imperialFaction->SetLocalText(Unicode::narrowToWide(buf));
	
		const std::vector<std::string> & npcFactionNames  = msg.getNPCFactionNames();
		const std::vector<float>       & npcFactionValues = msg.getNPCFactionValues();
		if(npcFactionNames.size() == npcFactionValues.size())
		{
			m_factions.clear();
			for(size_t i = 0; i < npcFactionNames.size(); ++i)
			{
				m_factions[npcFactionNames[i]] = static_cast<int>(npcFactionValues[i]);
			}
		}

		updateFactionTable();
	}
	else if(message.isType(GuildResponseMessage::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GuildResponseMessage msg(ri);
		if (m_creatureObjectWatcher->getPointer() && m_creatureObjectWatcher->getPointer()->getNetworkId() == msg.getTargetId())
		{
			if (isExaminingSelf())
				updateGuild(msg.getGuildName(), msg.getMemberTitle());
			else
				updateGuild(msg.getGuildName(), "");
		}
	}
	else if(message.isType(PlayerMoneyResponse::MessageType))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const PlayerMoneyResponse msg(ri);
		UNREF(msg);
	}
	else if(message.isType(CharacterSheetResponseMessage::cms_name))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const CharacterSheetResponseMessage msg(ri);

		//pull data from the message
		const Vector& bindLoc             = msg.getBindLoc();
		const std::string& bindPlanet     = msg.getBindPlanet();
		const Vector& resLoc              = msg.getResidenceLoc();
		const std::string& resPlanet      = msg.getResidencePlanet();
		const std::string& citizensOf     = msg.getCitizensOf();
		const Unicode::String& spouseName = msg.getSpouseName();
		int lotsUsed                      = msg.getLotsUsed();

		//set lots used info
		char buf[256];
		_itoa(lotsUsed, buf, 10);
		m_lotsAvailable->SetLocalText(Unicode::narrowToWide(buf));

		//set the bind location info
		if(bindPlanet.empty())
		{
			m_bindLocation->SetLocalText(CuiStringIdsCharacterSheet::unknown.localize());
		}
		else
		{
			Unicode::String bindString;
			CuiUtils::FormatVector(bindString, bindLoc);
			IGNORE_RETURN(bindString.append(Unicode::narrowToWide(", ")));
			StringId sid("planet_n", bindPlanet.c_str());
			IGNORE_RETURN(bindString.append(sid.localize()));
			m_bindLocation->SetLocalText(bindString);
		}

		//set the residence location info
		Unicode::String resString;
		if(resPlanet.empty())
		{
			resString = CuiStringIdsCharacterSheet::homeless.localize();
		}
		else
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "%d,%d,%d, ", static_cast<int>(resLoc.x), static_cast<int>(resLoc.y), static_cast<int>(resLoc.z));
			buffer[sizeof(buffer)-1] = '\0';

			resString = Unicode::narrowToWide(buffer);
			StringId sid("planet_n", resPlanet.c_str());
			IGNORE_RETURN(resString.append(sid.localize()));
		}

		if (!citizensOf.empty())
		{
			IGNORE_RETURN(resString.append(Unicode::narrowToWide(" (")));
			IGNORE_RETURN(resString.append(Unicode::narrowToWide(citizensOf)));
			IGNORE_RETURN(resString.append(Unicode::narrowToWide(")")));
		}

		m_home->SetLocalText(resString);

		if(spouseName == Unicode::emptyString)
			m_married->SetLocalText(CuiStringIdsCharacterSheet::unmarried.localize());
		else
			m_married->SetLocalText(spouseName);
	}
	else if(message.isType("CharacterSheetResponseResLoc"))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GenericValueTypeMessage<std::pair<std::string, std::string> > msg(ri);

		std::string const residenceLoc = msg.getValue().first;
		std::string const citizensOf = msg.getValue().second;

		//set the residence location info
		if (!residenceLoc.empty())
		{
			// Parse the residence location info
			char sceneId[100];
			int x, y, z;
			if (4 == ::sscanf(residenceLoc.c_str(), "%s %d %d %d", sceneId, &x, &y, &z))
			{
				char buffer[128];
				snprintf(buffer, sizeof(buffer)-1, "%d,%d,%d, ", x, y, z);
				buffer[sizeof(buffer)-1] = '\0';

				Unicode::String resString = Unicode::narrowToWide(buffer);
				StringId sid("planet_n", sceneId);
				IGNORE_RETURN(resString.append(sid.localize()));

				if (!citizensOf.empty())
				{
					IGNORE_RETURN(resString.append(Unicode::narrowToWide(" (")));
					IGNORE_RETURN(resString.append(Unicode::narrowToWide(citizensOf)));
					IGNORE_RETURN(resString.append(Unicode::narrowToWide(")")));
				}

				m_home->SetLocalText(resString);
			}
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::setExamineMode(CreatureObject *playerToExamine)
{
	*m_creatureObjectWatcher = playerToExamine;

	// examining another player
	if (m_creatureObjectWatcher->getPointer())
	{
		UIButton * const factionsTab = m_tabbedPane->GetTabButton(TAB_factions);
		factionsTab->SetVisible(false);

		UIButton * const skillModsTab = m_tabbedPane->GetTabButton(TAB_skillMods);
		skillModsTab->SetVisible(false);

		// move next to closest visible tab
		UIButton * const gcwTab = m_tabbedPane->GetTabButton(TAB_gcw);
		gcwTab->SetLocation(s_tabLocations[TAB_factions]);

		m_characterAttributesComp->SetVisible(false);
		m_personalPageTop->SetVisible(false);

		m_tabbedPane->SetActiveTab(TAB_status);
	}
	// examining self
	else
	{
		for (int i = 0; i < TAB_numTabPages; ++i)
		{
			UIButton * const button = m_tabbedPane->GetTabButton(i);
			button->SetVisible(true);

			// set back to original location
			UIButton * const gcwTab = m_tabbedPane->GetTabButton(TAB_gcw);
			gcwTab->SetLocation(s_tabLocations[TAB_gcw]);
		
			m_characterAttributesComp->SetVisible(true);

			m_tabbedPane->SetActiveTab(m_lastActiveTab);
		}

		m_personalPageTop->SetVisible(true);
		*m_creatureObjectWatcher = Game::getPlayerCreature();
	}

	if (m_creatureObjectWatcher->getPointer())
		*m_playerObjectWatcher = m_creatureObjectWatcher->getPointer()->getPlayerObject();
	else
		*m_playerObjectWatcher = NULL;
}

//-----------------------------------------------------------------------

bool SwgCuiCharacterSheet::isExaminingSelf() const
{
	return m_creatureObjectWatcher->getPointer() == Game::getPlayerCreature();
}

//-----------------------------------------------------------------------

CreatureObject* SwgCuiCharacterSheet::getCreatureToExamine() const
{
	return m_creatureObjectWatcher->getPointer();
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::OnButtonPressed(UIWidget *context)
{
	if(context == m_cyberneticsButton)
	{
		CuiActionManager::performAction(CuiActions::cybernetics, Unicode::emptyString);
	}
	else if(context == m_buttonCollections && getCreatureToExamine())
	{
		CuiActionManager::performAction(CuiActions::collections, Unicode::narrowToWide(getCreatureToExamine()->getNetworkId().getValueString()));
	}
}

//-----------------------------------------------------------------------

SwgCuiCharacterSheet* SwgCuiCharacterSheet::createInto(UIPage & parent)
{
	UIPage * const dupe = NON_NULL(UIPage::DuplicateInto(parent, "/PDA.CharacterSheet"));
	return new SwgCuiCharacterSheet(*dupe);
}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::update             (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (!m_playerObjectWatcher->getPointer() || !m_creatureObjectWatcher->getPointer())
	{
		closeNextFrame();
		return;
	}

	CreatureObject const * const creatureObject = m_creatureObjectWatcher->getPointer();
	if (creatureObject && (creatureObject->getNetworkId() != Game::getPlayerNetworkId()) && !PlayerObject::isAdmin() && !creatureObject->getCoverVisibility() && !creatureObject->isPassiveRevealPlayerCharacter(Game::getPlayerNetworkId()))
	{
		closeNextFrame();
		return;
	}

	ms_updateCheckTimer += deltaTimeSecs;
	if(ms_updateCheckTimer > UPDATE_CHECK_THRESHOLD)
	{
		ms_updateCheckTimer = 0.0f;
		checkCybernetics();
		updateAttributes();
		updateGcwPage();
		updateChroniclesInfo();
	}

}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::checkCybernetics()
{
	// do not show cybernetics page when examining other characters
	if (!isExaminingSelf())
	{
		m_cyberneticsButton->SetVisible(false);
		return;
	}

	bool hasACyberneticPieceOn = false;

	if (m_creatureObjectWatcher->getPointer())
	{
		for(std::vector<std::string>::const_iterator i = cms_cyberneticWearableSlots.begin(); i != cms_cyberneticWearableSlots.end(); ++i)
		{
			ClientObject * equippedObject = m_creatureObjectWatcher->getPointer()->getEquippedObject(i->c_str());

			if(equippedObject && GameObjectTypes::isTypeOf (equippedObject->getGameObjectType(), SharedObjectTemplate::GOT_cybernetic))
			{
				hasACyberneticPieceOn = true;
				break;
			}

			equippedObject = m_creatureObjectWatcher->getPointer()->getAppearanceEquippedObject(i->c_str());

			if(equippedObject && GameObjectTypes::isTypeOf (equippedObject->getGameObjectType(), SharedObjectTemplate::GOT_cybernetic))
			{
				hasACyberneticPieceOn = true;
				break;
			}
		}
	}

	m_cyberneticsButton->SetVisible(hasACyberneticPieceOn);
}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::requestServerInfo() const
{
	GameNetwork::send(PlayerMoneyRequest(), true);

	if (m_creatureObjectWatcher->getPointer())
		GameNetwork::send(GuildRequestMessage(m_creatureObjectWatcher->getPointer()->getNetworkId()), true);

	if (isExaminingSelf())
		IGNORE_RETURN(ClientCommandQueue::enqueueCommand("requestCharacterSheetInfo", NetworkId::cms_invalid, Unicode::emptyString));
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::updateGuild(const std::string &guildName, const std::string &memberTitle)
{
	if (m_creatureObjectWatcher->getPointer())
	{
		if (guildName.empty())
			m_characterName->SetLocalText(m_creatureObjectWatcher->getPointer()->getLocalizedName());
		else
		{
			Unicode::String s(m_creatureObjectWatcher->getPointer()->getLocalizedName());
			s += SwgCuiCharacterSheetNamespace::s_guildTagPrefix;
			s += Unicode::narrowToWide(guildName);
			s += SwgCuiCharacterSheetNamespace::s_guildTagSuffix;
			if (!memberTitle.empty())
			{
				s += SwgCuiCharacterSheetNamespace::s_guildMemberTitlePrefix;
				s += Unicode::narrowToWide(memberTitle);
				s += SwgCuiCharacterSheetNamespace::s_guildMemberTitleSuffix;
			}
			m_characterName->SetLocalText(s);		
		}
	}

	m_guild->SetLocalText(Unicode::narrowToWide(guildName));
	m_guildTitle->SetLocalText(Unicode::narrowToWide(memberTitle));
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::updateFactionTable ()
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_tableFactions->GetTableModel ());
	NOT_NULL (model);

	model->Attach (0);
	m_tableFactions->SetTableModel (0);

	model->ClearData ();

	Unicode::String tmp;
	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	for (std::map<std::string, int>::const_iterator it = m_factions.begin (); it != m_factions.end (); ++it)
	{
		const std::string & factionName  = Unicode::toLower((*it).first);
		const int           factionValue = (*it).second;

		tmp.clear ();
		StringId s("faction/faction_names", factionName);
		s.localize(tmp);
		UIData * const data0 = model->AppendCell (0, factionName.c_str (), tmp);

		snprintf (buf, buf_size, "%d", factionValue);
		UIData * const data1 = model->AppendCell (1, 0, Unicode::narrowToWide (buf));
		UNREF(data0);
		UNREF(data1);
	}

	m_tableFactions->SetTableModel (model);
	model->Detach (0);
}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::updateSkillModsTable()
{
	UITableModelDefault * const model = safe_cast<UITableModelDefault *>(m_skillModsTable->GetTableModel ());
	NOT_NULL (model);

	model->Attach (0);
	m_skillModsTable->SetTableModel (0);

	model->ClearData ();

	Unicode::String tmp;
	static char buf [128];
	static const size_t buf_size = sizeof (buf);

	getSkillModsForDisplay(model);

	m_skillModsTable->SetTableModel (model);
	model->Detach (0);

}

//-----------------------------------------------------------------------

void SwgCuiCharacterSheet::refreshBadgeWindow()
{
	m_badgeWindow->Clear();

	if (!m_playerObjectWatcher->getPointer())
		return;
	
	//show the badge *names* to ourselves, but view the desc of others (See uiexamine)
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> earnedBadges;
	IGNORE_RETURN(m_playerObjectWatcher->getPointer()->getCompletedCollectionSlotCountInBook("badge_book", &earnedBadges));

	UIString current;
	current = m_badgeWindow->GetLocalText();
	current += CuiStringIdsCharacterSheet::badges_earned.localize();
	current.push_back('\n');
	m_badgeWindow->SetLocalText(current);
	
	if (earnedBadges.empty())
	{
		current = m_badgeWindow->GetLocalText();
		current += Unicode::narrowToWide("\\>025");
		current += CuiStringIdsCharacterSheet::badges_none.localize();
		current += Unicode::narrowToWide("\\>000");
		current.push_back('\n');
		m_badgeWindow->SetLocalText(current);
	}
	else
	{
		for(std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator i = earnedBadges.begin(); i != earnedBadges.end(); ++i)
		{
			current = m_badgeWindow->GetLocalText();
			current += Unicode::narrowToWide("\\#pcontrast1 x\\>025");
			current += CollectionsDataTable::localizeCollectionName((*i)->name);
			current += Unicode::narrowToWide("\\>000\\#.");
			current.push_back('\n');
			m_badgeWindow->SetLocalText(current);
		}
	}
	
	// if examining another player, do not show badges that have not been won
	if (!isExaminingSelf())
		return;

	// get the list of all OTHER badges
	current = m_badgeWindow->GetLocalText();
	current += CuiStringIdsCharacterSheet::badges_unearned.localize();
	current.push_back('\n');
	m_badgeWindow->SetLocalText(current);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & allBadges = CollectionsDataTable::getSlotsInBook("badge_book");
	if (allBadges.size() == earnedBadges.size())
	{
		current = m_badgeWindow->GetLocalText();
		current += Unicode::narrowToWide("\\>025");
		current += CuiStringIdsCharacterSheet::badges_all.localize();
		current += Unicode::narrowToWide("\\>000");
		current.push_back('\n');
		m_badgeWindow->SetLocalText(current);
	}
	else
	{
		for(std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator i = allBadges.begin(); i != allBadges.end(); ++i)
		{
			if (!m_playerObjectWatcher->getPointer()->hasCompletedCollectionSlot(**i) && (*i)->showIfNotYetEarned != CollectionsDataTable::SE_none)
			{
				current = m_badgeWindow->GetLocalText();
				current += Unicode::narrowToWide("\\>025");
				current += CollectionsDataTable::localizeCollectionDescription((*i)->name);
				current += Unicode::narrowToWide("\\>000");
				current.push_back('\n');
				m_badgeWindow->SetLocalText(current);
			}
		}
	}
}

//----------------------------------------------------------------------

// JU_TODO: debug
#if 0
static void debug_skillModValues(CreatureObject const & creature)
{
	CreatureObject::SkillModMap::const_iterator iterSkillModMap;
	for(iterSkillModMap = creature.getSkillModMap().begin(); iterSkillModMap != creature.getSkillModMap().end(); ++iterSkillModMap)
	{
		DEBUG_WARNING(true,("\n[%s] (%d,%d)",iterSkillModMap->first.c_str(),iterSkillModMap->second.first,iterSkillModMap->second.second));
	}

	DEBUG_WARNING(true,("bp trap"));

}
#endif
// JU_TODO: end debug

void SwgCuiCharacterSheet::updateAttributes()
{
	if (!m_creatureObjectWatcher->getPointer() || !m_playerObjectWatcher->getPointer())
		return;

	static Unicode::String tmpStr1;
	static Unicode::String tmpStr2;
	static const Unicode::String sep = Unicode::narrowToWide(" / ");

	// LEVEL AND ROADMAP
	const int playerLevel = m_creatureObjectWatcher->getPointer()->getLevel();
	const std::string skillTemplateName = m_playerObjectWatcher->getPointer()->getSkillTemplate();

	if (!skillTemplateName.empty())
	{
		const std::string roadmapName = "title_" + RoadmapManager::getRoadmapNameForTemplateName(skillTemplateName);
		const StringId roadmapNameStringId("ui_roadmap", roadmapName.c_str());
		const StringId levelId("ui_charsheet", "level");
		tmpStr1 = levelId.localize() + Unicode::narrowToWide(" ");
		tmpStr2.clear ();
		UIUtils::FormatLong(tmpStr2, playerLevel);
		tmpStr1 += tmpStr2;
		tmpStr1 += Unicode::narrowToWide(" ");
		tmpStr1 += roadmapNameStringId.localize();
		m_levelValue->SetLocalText(tmpStr1);
		m_levelValue->SetVisible(true);
	}
	else
	{
		m_levelValue->SetVisible(false);
	}

	// HEALTH
	const Attributes::Value currentHealth = m_creatureObjectWatcher->getPointer()->getAttribute(Attributes::Health);
	const Attributes::Value currentMaxHealth = m_creatureObjectWatcher->getPointer()->getCurrentMaxAttribute (Attributes::Health) > 0 ? m_creatureObjectWatcher->getPointer()->getCurrentMaxAttribute (Attributes::Health) : 10000;
	tmpStr1.clear ();
	UIUtils::FormatLong (tmpStr1, currentHealth);
	tmpStr1 += sep;
	tmpStr2.clear ();
	UIUtils::FormatLong(tmpStr2, currentMaxHealth);
	tmpStr1 += tmpStr2;
	m_attrHealthValue->SetLocalText(tmpStr1);
	//HEALTH_BAR
	float percentHealthFull = static_cast<float>(currentHealth)/static_cast<float>(currentMaxHealth);
	percentHealthFull = clamp(0.0f,percentHealthFull,1.0f);
	UIScalar healthMaxSize = m_healthBarBack->GetWidth();
	UIScalar newHealthSize = static_cast<UIScalar>(static_cast<float>(healthMaxSize) * percentHealthFull);
	m_healthBar->SetWidth(newHealthSize);

	// ACTION
	const Attributes::Value currentAction = m_creatureObjectWatcher->getPointer()->getAttribute(Attributes::Action);
	const Attributes::Value currentMaxAction = m_creatureObjectWatcher->getPointer()->getCurrentMaxAttribute (Attributes::Action) > 0 ? m_creatureObjectWatcher->getPointer()->getCurrentMaxAttribute (Attributes::Action) : 10000;
	tmpStr1.clear ();
	UIUtils::FormatLong (tmpStr1, currentAction);
	tmpStr1 += sep;
	tmpStr2.clear ();
	UIUtils::FormatLong(tmpStr2, currentMaxAction);
	tmpStr1 += tmpStr2;
	m_attrActionValue->SetLocalText(tmpStr1);
	//ACTION_BAR
	float percentActionFull = static_cast<float>(currentAction)/static_cast<float>(currentMaxAction);
	percentActionFull = clamp(0.0f,percentActionFull,1.0f);
	UIScalar actionMaxSize = m_actionBarBack->GetWidth();
	UIScalar newActionSize = static_cast<UIScalar>(static_cast<float>(actionMaxSize) * percentActionFull);
	m_actionBar->SetWidth(newActionSize);


	UIText* skillMods[SMT_numSmts] = {
		m_attrStrengthValue,
		m_attrConstitutionValue,
		m_attrStaminaValue,
		m_attrPrecisionValue,
		m_attrAgilityValue,
		m_attrLuckValue,
		m_attrKineticValue,
		m_attrEnergyValue,
		m_attrHeatValue,
		m_attrColdValue,
		m_attrAcidValue,
		m_attrElectricityValue
	};

	// SKILL_MODS
	for (int i = 0; i < SMT_numSmts;i++)
	{	
		const std::string modString = std::string(ms_statMods[i]) + "_modified";
		const int baseValue = getSkillModValue(*m_creatureObjectWatcher->getPointer(),ms_statMods[i]);
		const int modValue = getSkillModValue(*m_creatureObjectWatcher->getPointer(),modString);
		const int total = baseValue + modValue;
		tmpStr1.clear ();
		UIUtils::FormatLong (tmpStr1, total);
		skillMods[i]->SetLocalText(tmpStr1);
		if(modValue > 0)
		{
			skillMods[i]->SetTextColor(UIColor::green);
		}
		else
		{
			skillMods[i]->SetTextColor(UIColor::white);
		}
	}
}	

//----------------------------------------------------------------------

int SwgCuiCharacterSheet::getSkillModValue(CreatureObject const & creature, std::string const & skillModName)
{
	CreatureObject::SkillModMap::const_iterator iterSkillModMap = creature.getSkillModMap().find(skillModName);

	if (iterSkillModMap != creature.getSkillModMap().end())
	{
		int const baseValue = iterSkillModMap->second.first;
		int const modValue = iterSkillModMap->second.second;

		return (baseValue + modValue);
	}
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::onSkillModsChanged (const CreatureObject & creature)
{
	if (&creature == static_cast<const Object *>(Game::getPlayer ()))
	{
		updateSkillModsTable();
	}	
}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::updateGcwPage()
{
	if (!m_creatureObjectWatcher->getPointer() || !m_playerObjectWatcher->getPointer())
		return;

	uint32 const currentRank = m_playerObjectWatcher->getPointer()->getCurrentGcwRank();

	static Unicode::String tmp_str;
	tmp_str.clear();

	// clear factional affiliation and rank icons
	m_pvpStatus->Clear();
	for (int i = 0; i < s_numGcwRankIcons; ++i)
	{
		s_gcwRebelRankIcons[i]->SetVisible(false);
		s_gcwImperialRankIcons[i]->SetVisible(false);
	}

	// set up for progress bar and percentage text
	float const percent = m_playerObjectWatcher->getPointer()->getCurrentGcwRankProgress();
	int const width = static_cast<int>(percent * 0.01f * m_gcwProgressBarBack->GetSize().x); 

	m_gcwPercent->SetVisible(true);
	m_gcwPercentSign->SetVisible(true);
	m_gcwProgressText->SetVisible(true);

	tmp_str.clear();
	UIUtils::FormatFloat(tmp_str, percent);
	m_gcwPercent->SetLocalText(tmp_str);
	m_gcwPercent->SetTextColor(UIColor::white);

	m_gcwPercentSign->SetTextColor(UIColor::white);

	// set up for high rank text
	m_gcwHighRankRebLabel->SetVisible(false);
	m_gcwHighRankReb->SetVisible(false);
	m_gcwHighRankImpLabel->SetVisible(false);
	m_gcwHighRankImp->SetVisible(false);

	int highRebelRank       = m_playerObjectWatcher->getPointer()->getMaxGcwRebelRank();
	int highImperialRank    = m_playerObjectWatcher->getPointer()->getMaxGcwImperialRank();

	// set all faction-specific values //
	uint32 const pvpFaction = m_creatureObjectWatcher->getPointer()->getPvpFaction();
	//rebel
	if (PvpData::isRebelFactionId(pvpFaction))
	{
		switch(m_creatureObjectWatcher->getPointer()->getPvpType())
			{
				//on leave
			case 0:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_rebelonleave.localize());
					break;
				}
				//covert
			case 1:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_rebelcovert.localize());
					break;
				}
				//declared
			case 2:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_rebeldeclared.localize());
					break;
				}
			default:
				{
					DEBUG_FATAL(true, ("Unknown factional alignment %d received from server, faction %lu", m_creatureObjectWatcher->getPointer()->getPvpType(), pvpFaction));
					break;
				}
			}

		// set rank icon
		if (currentRank > 0)
			s_gcwRebelRankIcons[currentRank-1]->SetVisible(true);

		// format high rank text
		if (highRebelRank > 0)
			formatHighRankText(PvpData::getRebelFactionId(), highRebelRank, FMT_top);

		if (highImperialRank > 0)
			formatHighRankText(PvpData::getImperialFactionId(), highImperialRank, FMT_bottom);

		// set progress bar and background image
		m_gcwProgressBarBack->SetVisible(true);
		m_gcwImperialProgBar->SetWidth(0);
		m_gcwRebelProgBar->SetWidth(width);
		m_gcwBackgroundImperial->SetVisible(false);
		m_gcwBackgroundRebel->SetVisible(true);
	}

	//imperial
	else if (PvpData::isImperialFactionId(pvpFaction))
	{
		switch(m_creatureObjectWatcher->getPointer()->getPvpType())
			{
				//on leave
			case 0:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_imperialonleave.localize());
					break;
				}
				//covert
			case 1:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_imperialcovert.localize());
					break;
				}
				//declared
			case 2:
				{
					m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_imperialdeclared.localize());
					break;
				}
			default:
				{
					DEBUG_FATAL(true, ("Unknown factional alignment %d received from server, faction %lu", m_creatureObjectWatcher->getPointer()->getPvpType(), pvpFaction));
					break;
				}
			}

		// set rank icon
		if (currentRank > 0)
			s_gcwImperialRankIcons[currentRank-1]->SetVisible(true);

		// format high rank text
		if (highRebelRank > 0)
			formatHighRankText(PvpData::getRebelFactionId(), highRebelRank, FMT_bottom);

		if (highImperialRank > 0)
			formatHighRankText(PvpData::getImperialFactionId(), highImperialRank, FMT_top);

		// set progress bar and background image
		m_gcwProgressBarBack->SetVisible(true);
		m_gcwImperialProgBar->SetWidth(width);
		m_gcwRebelProgBar->SetWidth(0);
		m_gcwBackgroundImperial->SetVisible(true);
		m_gcwBackgroundRebel->SetVisible(false);

	}

	//neutral
	else if (PvpData::isNeutralFactionId(pvpFaction))
	{

		// GU 18.4 - Display Mercenary Status on Character Sheet for Self Only
		if(isExaminingSelf())
		{
			const PlayerObject* po = Game::getPlayerObject();
			if(po)
			{
				unsigned long slotVal = 0;
				const char *slots[4] = {
					"covert_imperial_mercenary",
					"overt_imperial_mercenary",
					"covert_rebel_mercenary",
					"overt_rebel_mercenary"
				};
				for (unsigned i = 0; i < 4; i++)
				{
					po->getCollectionSlotValue(slots[i], slotVal);
					if(slotVal > 0)
					{
						slotVal = i+1;
						break;
					}
				}
				switch(slotVal)
				{
					case 1:
						m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral_imperialhelpercovert.localize());
						break;
					case 2:
						m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral_imperialhelperdeclared.localize());
						break;
					case 3:
						m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral_rebelhelpercovert.localize());
						break;
					case 4:
						m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral_rebelhelperdeclared.localize());
						break;
					default:
						m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral.localize());
						break;
				}
			}
			else
			{
				m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral.localize());
			}
		}
		else
		{
			m_pvpStatus->SetText(CuiStringIdsCharacterSheet::faction_neutral.localize());
		}
	
		// format high rank text: put highest rank at top. if ranks are equal, just put rebel at top
		if (highRebelRank >= highImperialRank)
		{
			if (highRebelRank > 0)
				formatHighRankText(PvpData::getRebelFactionId(), highRebelRank, FMT_top);

			if (highImperialRank > 0)
				formatHighRankText(PvpData::getImperialFactionId(), highImperialRank, FMT_bottom);
		}
		else
		{
			if (highRebelRank > 0)
				formatHighRankText(PvpData::getRebelFactionId(), highRebelRank, FMT_bottom);

			if (highImperialRank > 0)
				formatHighRankText(PvpData::getImperialFactionId(), highImperialRank, FMT_top);
		}

		// set progress bar and background image
		m_gcwProgressBarBack->SetVisible(false);
		m_gcwImperialProgBar->SetWidth(0);
		m_gcwRebelProgBar->SetWidth(0);
		m_gcwBackgroundImperial->SetVisible(false);
		m_gcwBackgroundRebel->SetVisible(false);

		// do not show progress percentage text
		m_gcwPercent->SetVisible(false);
		m_gcwPercentSign->SetVisible(false);
		m_gcwProgressText->SetVisible(false);

	}

	else
		DEBUG_FATAL(true, ("Unknown factional alignment received from server, faction %lu", pvpFaction));


	// stats 
	int32 const gcwPoints = m_playerObjectWatcher->getPointer()->getCurrentGcwPoints();
	UIUtils::FormatLong(tmp_str, gcwPoints);
	m_gcwPoints->SetLocalText(tmp_str);
	m_gcwPoints->SetTextColor(UIColor::white);

	int32 const pvpKills = m_playerObjectWatcher->getPointer()->getCurrentPvpKills();
	tmp_str.clear();
	UIUtils::FormatLong(tmp_str, pvpKills);
	m_gcwPvPKills->SetLocalText(tmp_str);
	m_gcwPvPKills->SetTextColor(UIColor::white);

	int64 const lifetimePoints = m_playerObjectWatcher->getPointer()->getLifetimeGcwPoints();
	char tmp_char[256];
	sprintf(tmp_char, INT64_FORMAT_SPECIFIER, lifetimePoints);
	m_gcwLifetime->SetLocalText(Unicode::narrowToWide(tmp_char));
	m_gcwLifetime->SetTextColor(UIColor::white);

	int32 const lifetimePvP = m_playerObjectWatcher->getPointer()->getLifetimePvpKills();
	tmp_str.clear();
	UIUtils::FormatLong(tmp_str, lifetimePvP);
	m_gcwPvPLifetime->SetLocalText(tmp_str);
	m_gcwPvPLifetime->SetTextColor(UIColor::white);

	// rank text
	m_gcwRank->SetLocalText(m_creatureObjectWatcher->getPointer()->getLocalizedGcwRankString(m_playerObjectWatcher->getPointer()->getCurrentGcwRank(), pvpFaction)); 
	m_gcwRank->SetTextColor(UIColor::white);

	// time until next rank update
	time_t time = m_playerObjectWatcher->getPointer()->getGcwRatingActualCalcTime();

	if (time <= 0)
	{
		m_gcwTimerLabel->SetVisible(false);
		m_gcwTimer->SetVisible(false);
	}
	else
	{
		char buffer[512];
		int timeUntil = static_cast<int>(time - ::time(NULL));

		if (timeUntil >= 0)
		{
			snprintf(buffer, sizeof(buffer)-1, "%s", CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeUntil)).c_str());
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "-%s", CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(-timeUntil)).c_str());
		}

		buffer[sizeof(buffer)-1] = '\0';

		m_gcwTimerLabel->SetVisible(true);
		m_gcwTimer->SetVisible(true);
		m_gcwTimer->SetColor(UIColor::white);
		m_gcwTimer->SetLocalText(Unicode::narrowToWide(buffer));
	}

}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::formatHighRankText(uint32 factionCrc, int maxRank, int format)
{
	UIText* label;
	UIText* value;
	int y;

	if (PvpData::isRebelFactionId(factionCrc))
	{
		label = m_gcwHighRankRebLabel;
		value = m_gcwHighRankReb;
	}
	else if (PvpData::isImperialFactionId(factionCrc))
	{
		label = m_gcwHighRankImpLabel;
		value = m_gcwHighRankImp;
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Neutral faction crc sent to formatHighRankText, crc %lu\n", factionCrc));
		return;
	}

	int spacing = m_gcwPvPLifetime->GetLocation().y - m_gcwLifetime->GetLocation().y;

	if (format == FMT_top)
		y = m_gcwPvPLifetime->GetLocation().y + spacing; 
	else if (format == FMT_bottom)
		y = m_gcwPvPLifetime->GetLocation().y + spacing*2; 
	else 
	{
		DEBUG_REPORT_LOG(true, ("Unknown format type sent to formatHighRankText, format %i\n", format));
		return;
	}

	Unicode::String rankString = CreatureObject::getLocalizedGcwRankString(maxRank, factionCrc);

	label->SetVisible(true);
	label->SetLocation(0, y, false);                              // align with left column
	value->SetVisible(true);
	value->SetLocation(m_gcwPoints->GetLocation().x, y, false);   // align with right column
	value->SetLocalText(rankString);
	value->SetTextColor(UIColor::white);
}

//----------------------------------------------------------------------

void SwgCuiCharacterSheet::updateChroniclesInfo()
{
	PlayerObject const * const player = Game::getPlayerObject();

	if(!player)
		return;

	float ratingTotal = 0.0f;

	if(player->getPgcRatingCount() != 0)
		ratingTotal = static_cast<float>(player->getPgcRatingTotal()) / static_cast<float>(player->getPgcRatingCount());

	char displayBuffer[256];
	memset(displayBuffer, 0, 256);
	sprintf(displayBuffer, "%.1f (from %I64i total ratings)", ratingTotal, player->getPgcRatingCount());

	m_chroniclesRating->SetLocalText(Unicode::narrowToWide(displayBuffer));

	if(player->getPgcLastRatingTime() == 0)
		m_chroniclesUpdate->SetLocalText(Unicode::narrowToWide("Not Available"));
	else
		m_chroniclesUpdate->SetLocalText(Unicode::narrowToWide(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(player->getPgcLastRatingTime())));

}

// ======================================================================
