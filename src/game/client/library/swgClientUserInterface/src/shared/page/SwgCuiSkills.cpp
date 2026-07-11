// ======================================================================
//
// SwgCuiSkills.cpp
// Pre-CU skill window mediator. See SwgCuiSkills.h.
//
// V4 rewrite: V1-V3 widget bindings were all silently failing because
// the actual UI tree uses different types and paths than I'd assumed
// (profession list is a TreeView with a UIDataSourceContainer; tables
// are nested under a Composite with column DataSources). This pass
// binds via correct paths and populates the existing widget data
// sources directly. REPORT_LOG every binding to make further debugging
// possible from the client log.
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSkills.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UIText.h"
#include "UITreeView.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedDebug/Report.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include "SwgCuiSkillsData.h"
#include "SwgCuiSkillBoxData.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace
{
	// Find a ProfessionDef by its novice skill name. Returns 0 if the
	// passed skill name isn't a canonical Pre-CU profession root.
	ProfessionDef const * findProfessionDef(std::string const & noviceName)
	{
		for (int i = 0; i < k_professionDefCount; ++i)
		{
			if (noviceName == k_professionDefs[i].noviceSkill)
				return &k_professionDefs[i];
		}
		return 0;
	}

	// Binary-search the sorted k_skillCosts table for a skill's
	// skillPoints cost. Returns 0 if the skill name is unknown
	// (e.g. NGE expertise leaf, chronicler skill, etc.).
	int findSkillCost(std::string const & skillName)
	{
		int lo = 0;
		int hi = k_skillCostsCount;
		while (lo < hi)
		{
			int const mid = (lo + hi) / 2;
			int const cmp = std::strcmp(k_skillCosts[mid].skillName, skillName.c_str());
			if (cmp == 0)
				return k_skillCosts[mid].cost;
			if (cmp < 0)
				lo = mid + 1;
			else
				hi = mid;
		}
		return 0;
	}

	bool isCanonicalProfession(std::string const & name)
	{
		return findProfessionDef(name) != 0;
	}

	// Walk SkillObject parent chain (via getPrevSkill) until we hit a
	// canonical pre-CU profession root, or run out. Returns 0 if the
	// player's skill isn't part of any pre-CU profession tree.
	SkillObject const * walkToCanonicalProfession(SkillObject const * skill)
	{
		for (int hops = 0; skill && hops < 16; ++hops)
		{
			if (isCanonicalProfession(skill->getSkillName()))
				return skill;
			skill = skill->getPrevSkill();
		}
		return 0;
	}

	// Strip the "_novice" suffix to recover the canonical profession key.
	std::string stripNoviceSuffix(std::string const & skillName)
	{
		std::string::size_type const tail = skillName.rfind("_novice");
		if (tail != std::string::npos && tail + 7 == skillName.size())
			return skillName.substr(0, tail);
		return skillName;
	}

	Unicode::String localizeProfessionDisplay(std::string const & noviceSkillName)
	{
		// First preference: the calculator's canonical display name
		// (e.g. "Brawler", "Teras Kasi Artist").
		ProfessionDef const * const def = findProfessionDef(noviceSkillName);
		if (def && def->displayName && def->displayName[0])
			return Unicode::narrowToWide(def->displayName);

		// Fallbacks: try the profession-family key in skl_t.stf / skl_n.stf,
		// then the novice name, then raw.
		std::string const familyKey = stripNoviceSuffix(noviceSkillName);
		Unicode::String out;
		if (CuiSkillManager::localizeSkillTitle(familyKey, out) && !out.empty())
			return out;
		if (CuiSkillManager::localizeSkillName(familyKey, out) && !out.empty())
			return out;
		if (CuiSkillManager::localizeSkillName(noviceSkillName, out) && !out.empty())
			return out;
		return Unicode::narrowToWide(familyKey);
	}

	void appendTableRow(UIDataSource * nameDs, UIDataSource * pointsDs,
	                    Unicode::String const & nameText, Unicode::String const & pointsText,
	                    int rowIndex)
	{
		if (!nameDs || !pointsDs)
			return;

		char rowKey[16];
		snprintf(rowKey, sizeof(rowKey), "row%d", rowIndex);

		// Table cells render from the "Value" property -- UITableModelDefault::
		// GetValueAtText reads LocalValue||Value, NOT Text/LocalText. Setting
		// Text/LocalText (as before) left every XP / skill-mod row blank.
		UIData * const nameRow = new UIData;
		nameRow->SetName(rowKey);
		nameRow->SetProperty(UILowerString("Value"), nameText);
		nameDs->AddChild(nameRow);

		UIData * const pointsRow = new UIData;
		pointsRow->SetName(rowKey);
		pointsRow->SetProperty(UILowerString("Value"), pointsText);
		pointsDs->AddChild(pointsRow);
	}

	// "private_*" commands and skill mods are internal markers (skill-tree
	// bookkeeping, combat-difficulty flags), not player-facing abilities/stats.
	// Retail hides them from the granted lists; so do we.
	bool isPrivateName(std::string const & name)
	{
		return name.size() >= 8 && name.compare(0, 8, "private_") == 0;
	}

	// Fallback display name when no cmd_n / skill-mod string entry exists:
	// drop a trailing level digit, split camelCase + underscores into words,
	// Title-Case. "suppressionFire1" -> "Suppression Fire";
	// "cert_rifle_dlt20" -> "Cert Rifle Dlt"; "pointBlankArea1" -> "Point Blank Area".
	Unicode::String prettifyKey(std::string s)
	{
		while (!s.empty() && isdigit(static_cast<unsigned char>(s[s.size() - 1])))
			s.erase(s.size() - 1);

		std::string out;
		bool startWord = true;
		for (size_t i = 0; i < s.size(); ++i)
		{
			char const c = s[i];
			if (c == '_')
			{
				if (!out.empty() && out[out.size() - 1] != ' ')
					out += ' ';
				startWord = true;
				continue;
			}
			if (i > 0 && isupper(static_cast<unsigned char>(c)) &&
			    islower(static_cast<unsigned char>(s[i - 1])))
			{
				out += ' ';
				startWord = true;
			}
			if (startWord)
			{
				out += static_cast<char>(toupper(static_cast<unsigned char>(c)));
				startWord = false;
			}
			else
				out += static_cast<char>(tolower(static_cast<unsigned char>(c)));
		}
		return Unicode::narrowToWide(out);
	}
}

//-----------------------------------------------------------------------

SwgCuiSkills::SwgCuiSkills(UIPage & page)
:
CuiMediator         ("SwgCuiSkills", page),
UIEventCallback     (),
m_tabs              (0),
m_pageProfessionList(0),
m_pageMyStats       (0),
m_pageProfession    (0),
m_buttonClose       (0),
m_textProfName      (0),
m_textProfessionBody(0),
m_pageGraphs        (0),
m_pageGraph4x4      (0),
m_pageGraph2x4      (0),
m_pageGraph1x4      (0),
m_pageGraphPyramid  (0),
m_dsProfTree        (0),
m_treeProf          (0),
m_dsExpName         (0),
m_dsExpPoints       (0),
m_dsModsName        (0),
m_dsModsPoints      (0),
m_dsInfoModsName    (0),
m_dsInfoModsPoints  (0),
m_dsInfoCmdsName    (0),
m_dsInfoCmdsIcons   (0),
m_textSkillPoints   (0),
m_buttonSurrender   (0),
m_buttonSkills      (),
m_selectedProfession(),
m_selectedSkill     ()
{
	getCodeDataObject(TUITabbedPane, m_tabs,               "tabs");
	getCodeDataObject(TUIPage,       m_pageProfessionList, "pageProfessionList");
	getCodeDataObject(TUIPage,       m_pageMyStats,        "pageMyStats");
	getCodeDataObject(TUIPage,       m_pageProfession,     "pageProfession");
	getCodeDataObject(TUIButton,     m_buttonClose,        "buttonclose");

	REPORT_LOG(true, ("SwgCuiSkills: ctor bind: tabs=%p profList=%p myStats=%p profession=%p close=%p\n",
		(void *)m_tabs, (void *)m_pageProfessionList, (void *)m_pageMyStats, (void *)m_pageProfession, (void *)m_buttonClose));

	// Profession TreeView + its DataSourceContainer.
	if (m_pageProfessionList)
	{
		UIBaseObject * const treeObj = m_pageProfessionList->GetObjectFromPath("tree", TUITreeView);
		if (treeObj)
			m_treeProf = static_cast<UITreeView *>(treeObj);

		UIBaseObject * const dsObj = m_pageProfessionList->GetObjectFromPath("data", TUIDataSourceContainer);
		if (dsObj)
			m_dsProfTree = static_cast<UIDataSourceContainer *>(dsObj);

		REPORT_LOG(true, ("SwgCuiSkills:   pageProfList children: tree=%p data=%p\n",
			(void *)m_treeProf, (void *)m_dsProfTree));
	}

	// XP and Skill-Mods table column DataSources, nested as
	// myStats.comp.TableExp.containerall.{name,points} and same for TableMods.
	if (m_pageMyStats)
	{
		UIBaseObject * const expNameObj   = m_pageMyStats->GetObjectFromPath("comp.TableExp.containerall.name",    TUIDataSource);
		UIBaseObject * const expPointsObj = m_pageMyStats->GetObjectFromPath("comp.TableExp.containerall.points",  TUIDataSource);
		UIBaseObject * const modsNameObj  = m_pageMyStats->GetObjectFromPath("comp.TableMods.containerall.name",   TUIDataSource);
		UIBaseObject * const modsPtsObj   = m_pageMyStats->GetObjectFromPath("comp.TableMods.containerall.points", TUIDataSource);
		if (expNameObj)
			m_dsExpName = static_cast<UIDataSource *>(expNameObj);
		if (expPointsObj)
			m_dsExpPoints = static_cast<UIDataSource *>(expPointsObj);
		if (modsNameObj)
			m_dsModsName = static_cast<UIDataSource *>(modsNameObj);
		if (modsPtsObj)
			m_dsModsPoints = static_cast<UIDataSource *>(modsPtsObj);

		REPORT_LOG(true, ("SwgCuiSkills:   myStats columns: expName=%p expPoints=%p modsName=%p modsPoints=%p\n",
			(void *)m_dsExpName, (void *)m_dsExpPoints, (void *)m_dsModsName, (void *)m_dsModsPoints));
	}

	// Right-panel header + V2 text-body fallback + V3 graph containers.
	if (m_pageProfession)
	{
		UIBaseObject * const profNameObj = m_pageProfession->GetObjectFromPath("textProfName",      TUIText);
		UIBaseObject * const profBodyObj = m_pageProfession->GetObjectFromPath("all.textSkillName", TUIText);
		if (profNameObj)
			m_textProfName = static_cast<UIText *>(profNameObj);
		if (profBodyObj)
			m_textProfessionBody = static_cast<UIText *>(profBodyObj);

		// Per-selected-skill detail panels (right-panel bottom row).
		UIBaseObject * const infoModsNameObj   = m_pageProfession->GetObjectFromPath("all.info.mods.containerall.name",     TUIDataSource);
		UIBaseObject * const infoModsPointsObj = m_pageProfession->GetObjectFromPath("all.info.mods.containerall.points",   TUIDataSource);
		UIBaseObject * const infoCmdsNameObj   = m_pageProfession->GetObjectFromPath("all.info.commands.containerall.name", TUIDataSource);
		UIBaseObject * const infoCmdsIconsObj  = m_pageProfession->GetObjectFromPath("all.info.commands.containerall.icons", TUIDataSource);
		if (infoModsNameObj)   m_dsInfoModsName   = static_cast<UIDataSource *>(infoModsNameObj);
		if (infoModsPointsObj) m_dsInfoModsPoints = static_cast<UIDataSource *>(infoModsPointsObj);
		if (infoCmdsNameObj)   m_dsInfoCmdsName   = static_cast<UIDataSource *>(infoCmdsNameObj);
		if (infoCmdsIconsObj)  m_dsInfoCmdsIcons  = static_cast<UIDataSource *>(infoCmdsIconsObj);
		REPORT_LOG(true, ("SwgCuiSkills:   info detail: modsName=%p modsPts=%p cmdsName=%p cmdsIcons=%p\n",
			(void *)m_dsInfoModsName, (void *)m_dsInfoModsPoints,
			(void *)m_dsInfoCmdsName, (void *)m_dsInfoCmdsIcons));

		// Skill-points display + surrender button.
		UIBaseObject * const spTextObj = m_pageProfession->GetObjectFromPath("all.skillPoints.textSkillPoints", TUIText);
		UIBaseObject * const spBtnObj  = m_pageProfession->GetObjectFromPath("all.skillPoints.buttonSurrender", TUIButton);
		if (spTextObj) m_textSkillPoints = static_cast<UIText *>(spTextObj);
		if (spBtnObj)  m_buttonSurrender = static_cast<UIButton *>(spBtnObj);
		if (m_buttonSurrender)
			registerMediatorObject(*m_buttonSurrender, true);
		REPORT_LOG(true, ("SwgCuiSkills:   skill points: textSP=%p btnSurrender=%p\n",
			(void *)m_textSkillPoints, (void *)m_buttonSurrender));

		UIBaseObject * const graphsObj    = m_pageProfession->GetObjectFromPath("all.graphs",              TUIPage);
		UIBaseObject * const graph4x4Obj  = m_pageProfession->GetObjectFromPath("all.graphs.graph4x4",     TUIPage);
		UIBaseObject * const graph2x4Obj  = m_pageProfession->GetObjectFromPath("all.graphs.graph2x4",     TUIPage);
		UIBaseObject * const graph1x4Obj  = m_pageProfession->GetObjectFromPath("all.graphs.graph1x4",     TUIPage);
		UIBaseObject * const graphPyrObj  = m_pageProfession->GetObjectFromPath("all.graphs.graphPyramid", TUIPage);
		if (graphsObj)     m_pageGraphs       = static_cast<UIPage *>(graphsObj);
		if (graph4x4Obj)   m_pageGraph4x4     = static_cast<UIPage *>(graph4x4Obj);
		if (graph2x4Obj)   m_pageGraph2x4     = static_cast<UIPage *>(graph2x4Obj);
		if (graph1x4Obj)   m_pageGraph1x4     = static_cast<UIPage *>(graph1x4Obj);
		if (graphPyrObj)   m_pageGraphPyramid = static_cast<UIPage *>(graphPyrObj);

		REPORT_LOG(true, ("SwgCuiSkills:   right-panel: profName=%p profBody=%p graphs=%p g4x4=%p g2x4=%p g1x4=%p gPyr=%p\n",
			(void *)m_textProfName, (void *)m_textProfessionBody,
			(void *)m_pageGraphs, (void *)m_pageGraph4x4, (void *)m_pageGraph2x4,
			(void *)m_pageGraph1x4, (void *)m_pageGraphPyramid));
	}

	if (m_buttonClose)
		registerMediatorObject(*m_buttonClose, true);
	if (m_tabs)
		registerMediatorObject(*m_tabs, true);
	if (m_treeProf)
		registerMediatorObject(*m_treeProf, true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);
}

//-----------------------------------------------------------------------

SwgCuiSkills::~SwgCuiSkills()
{
}

//-----------------------------------------------------------------------

void SwgCuiSkills::performActivate()
{
	CuiMediator::performActivate();

	REPORT_LOG(true, ("SwgCuiSkills: performActivate (activeTab=%ld)\n",
		m_tabs ? static_cast<long>(m_tabs->GetActiveTab()) : -1L));

	populateProfessionList();
	populateExperience();
	populateSkillMods();
	populateSelectedProfession();
	updateSkillPointsDisplay();
}

//-----------------------------------------------------------------------

void SwgCuiSkills::performDeactivate()
{
	CuiMediator::performDeactivate();
}

//-----------------------------------------------------------------------

bool SwgCuiSkills::OnMessage(UIWidget * context, const UIMessage & msg)
{
	// "To: <profession>" links are UIText, which never delivers OnButtonPressed.
	// Catch the left-click here and jump the tree to that profession.
	if (msg.Type == UIMessage::LeftMouseUp)
	{
		std::map<UIWidget *, std::string>::const_iterator const link = m_linkSkills.find(context);
		if (link != m_linkSkills.end())
		{
			m_selectedProfession = link->second;
			REPORT_LOG(true, ("SwgCuiSkills: link click -> profession='%s'\n", m_selectedProfession.c_str()));
			populateSelectedProfession();
			return false;   // consume
		}
	}
	return true;            // default processing for everything else
}

//-----------------------------------------------------------------------

void SwgCuiSkills::OnButtonPressed(UIWidget * context)
{
	if (context == m_buttonClose)
	{
		closeThroughWorkspace();
		return;
	}

	if (context == m_buttonSurrender)
	{
		if (m_selectedSkill.empty())
		{
			REPORT_LOG(true, ("SwgCuiSkills: surrender ignored (no skill selected)\n"));
			return;
		}
		// V14: real surrender via ClientCommandQueue. Sends the
		// "surrenderSkill" command (registered on the server in
		// commands.iff) with the skill name as a parameter; the server
		// validates ownership and revokes via the standard skill path.
		CreatureObject const * const player = Game::getPlayerCreature();
		NetworkId const & playerId = player ? player->getNetworkId() : NetworkId::cms_invalid;
		ClientCommandQueue::enqueueCommand("surrenderSkill", playerId,
			Unicode::narrowToWide(m_selectedSkill));
		REPORT_LOG(true, ("SwgCuiSkills: surrenderSkill enqueued for '%s'\n", m_selectedSkill.c_str()));
		return;
	}

	// "To: <profession>" link click: jump the tree to the linked profession.
	std::map<UIWidget *, std::string>::const_iterator const link = m_linkSkills.find(context);
	if (link != m_linkSkills.end())
	{
		m_selectedProfession = link->second;
		REPORT_LOG(true, ("SwgCuiSkills: link click -> profession='%s'\n", m_selectedProfession.c_str()));
		populateSelectedProfession();
		return;
	}

	// Tree-cell button click: look up which skill this button currently
	// represents (populated by tryPopulateGraph4x4) and refresh the
	// per-selected-skill bottom panels.
	std::map<UIWidget *, std::string>::const_iterator const it = m_buttonSkills.find(context);
	if (it != m_buttonSkills.end())
	{
		m_selectedSkill = it->second;
		REPORT_LOG(true, ("SwgCuiSkills: selected skill='%s'\n", m_selectedSkill.c_str()));
		populateSelectedSkill();
	}
}

//-----------------------------------------------------------------------

void SwgCuiSkills::updateSkillPointsDisplay()
{
	if (!m_textSkillPoints)
		return;

	int used = 0;
	CreatureObject const * const player = Game::getPlayerCreature();
	if (player)
	{
		CreatureObject::SkillList const & playerSkills = player->getSkills();
		for (CreatureObject::SkillList::const_iterator it = playerSkills.begin(); it != playerSkills.end(); ++it)
		{
			if (*it)
				used += findSkillCost((*it)->getSkillName());
		}
	}

	char buf[64];
	snprintf(buf, sizeof(buf), "%d / %d", used, k_skillPointCap);
	m_textSkillPoints->SetLocalText(Unicode::narrowToWide(buf));
}

//-----------------------------------------------------------------------

void SwgCuiSkills::OnGenericSelectionChanged(UIWidget * context)
{
	if (context == m_treeProf && m_treeProf)
	{
		long const row = m_treeProf->GetLastSelectedRow();
		if (row >= 0)
		{
			UIDataSourceContainer * const data = m_treeProf->GetDataSourceContainerAtRow(row);
			if (data)
			{
				m_selectedProfession = data->GetName();
				REPORT_LOG(true, ("SwgCuiSkills: selected profession='%s'\n", m_selectedProfession.c_str()));
				populateSelectedProfession();
			}
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiSkills::OnTabbedPaneChanged(UIWidget * context)
{
	REPORT_LOG(true, ("SwgCuiSkills::OnTabbedPaneChanged: context=%p m_tabs=%p activeTab=%ld\n",
		(void *)context, (void *)m_tabs,
		m_tabs ? static_cast<long>(m_tabs->GetActiveTab()) : -1L));
	populateProfessionList();
	populateSelectedProfession();
}

//-----------------------------------------------------------------------

void SwgCuiSkills::populateExperience()
{
	if (m_dsExpName)  m_dsExpName->Clear();
	if (m_dsExpPoints) m_dsExpPoints->Clear();
	if (!m_dsExpName || !m_dsExpPoints)
		return;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return;

	CreatureObject::ExperiencePointMap const & xpMap = player->getExperiencePointMap();
	int rowIdx = 0;
	for (CreatureObject::ExperiencePointMap::const_iterator it = xpMap.begin(); it != xpMap.end(); ++it, ++rowIdx)
	{
		std::string const & xpType = it->first;
		int const xpValue = it->second;

		Unicode::String localized;
		if (!CuiSkillManager::localizeExpName(xpType, localized) || localized.empty())
			localized = Unicode::narrowToWide(xpType);

		appendTableRow(m_dsExpName, m_dsExpPoints, localized,
			Unicode::narrowToWide(FormattedString<32>().sprintf("%d", xpValue)), rowIdx);
	}
}

//-----------------------------------------------------------------------

void SwgCuiSkills::populateSkillMods()
{
	if (m_dsModsName)  m_dsModsName->Clear();
	if (m_dsModsPoints) m_dsModsPoints->Clear();
	if (!m_dsModsName || !m_dsModsPoints)
		return;

	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return;

	CreatureObject::SkillModMap const & modMap = player->getSkillModMap();
	int rowIdx = 0;
	for (CreatureObject::SkillModMap::const_iterator it = modMap.begin(); it != modMap.end(); ++it)
	{
		std::string const & modName = it->first;
		int const modValue = it->second.first + it->second.second;
		if (modValue == 0)
			continue;

		Unicode::String localized;
		if (!CuiSkillManager::localizeSkillModName(modName, localized) || localized.empty())
			localized = Unicode::narrowToWide(modName);

		appendTableRow(m_dsModsName, m_dsModsPoints, localized,
			Unicode::narrowToWide(FormattedString<32>().sprintf("%d", modValue)), rowIdx);
		++rowIdx;
	}
}

//-----------------------------------------------------------------------

void SwgCuiSkills::populateProfessionList()
{
	if (!m_dsProfTree)
	{
		REPORT_LOG(true, ("SwgCuiSkills: populateProfessionList skipped: m_dsProfTree is null\n"));
		return;
	}

	m_dsProfTree->Clear();

	// Determine the set of profession-novice skills to display based on
	// the active tab.
	bool const showAll = (m_tabs && m_tabs->GetActiveTab() == 1);

	std::set<std::string> noviceSet;

	if (showAll)
	{
		// "All Professions": iterate the canonical k_professionDefs table
		// (derived from the SWG Profession Calculator). Include each
		// entry whose novice skill exists in SkillManager.
		SkillManager & skillMgr = SkillManager::getInstance();
		for (int i = 0; i < k_professionDefCount; ++i)
		{
			char const * const nov = k_professionDefs[i].noviceSkill;
			if (nov && skillMgr.getSkill(nov))
				noviceSet.insert(nov);
		}
	}
	else
	{
		// "My Character": canonical professions the player has any skill
		// from. Walk parent chain (getPrevSkill) for each granted skill
		// until we find a pre-CU profession root; ignore NGE expertise /
		// chronicler / pilot trees whose roots aren't pre-CU.
		CreatureObject const * const player = Game::getPlayerCreature();
		if (player)
		{
			CreatureObject::SkillList const & playerSkills = player->getSkills();
			for (CreatureObject::SkillList::const_iterator it = playerSkills.begin(); it != playerSkills.end(); ++it)
			{
				SkillObject const * const skill = *it;
				if (!skill)
					continue;
				SkillObject const * const prof = walkToCanonicalProfession(skill);
				if (prof)
					noviceSet.insert(prof->getSkillName());
			}
		}
	}

	REPORT_LOG(true, ("SwgCuiSkills: populating profession list (%s) with %zu entries\n",
		showAll ? "all" : "mine", noviceSet.size()));

	// Sort by localized profession name and add a UIDataSourceContainer
	// child to the tree for each. The TreeView renders each child's Text.
	typedef std::pair<Unicode::String, std::string> NameRow;
	std::vector<NameRow> rows;
	rows.reserve(noviceSet.size());
	for (std::set<std::string>::const_iterator it = noviceSet.begin(); it != noviceSet.end(); ++it)
		rows.push_back(std::make_pair(localizeProfessionDisplay(*it), *it));
	std::sort(rows.begin(), rows.end());

	for (std::vector<NameRow>::const_iterator it = rows.begin(); it != rows.end(); ++it)
	{
		UIDataSourceContainer * const child = new UIDataSourceContainer;
		child->SetName(it->second);
		child->SetProperty(UILowerString("Text"),      it->first);
		child->SetProperty(UILowerString("LocalText"), it->first);
		m_dsProfTree->AddChild(child);
	}

	// Keep the selection sane.
	if (!m_selectedProfession.empty() && noviceSet.find(m_selectedProfession) == noviceSet.end())
		m_selectedProfession.clear();
	if (m_selectedProfession.empty() && !rows.empty())
		m_selectedProfession = rows.front().second;
}

//-----------------------------------------------------------------------

void SwgCuiSkills::populateSelectedProfession()
{
	if (!m_pageProfession)
		return;

	hideAllGraphs();

	if (m_selectedProfession.empty())
	{
		if (m_textProfName)
			m_textProfName->SetLocalText(Unicode::emptyString);
		if (m_textProfessionBody)
			m_textProfessionBody->SetLocalText(Unicode::emptyString);
		return;
	}

	SkillObject const * const profSkill = SkillManager::getInstance().getSkill(m_selectedProfession);
	if (!profSkill)
		return;

	Unicode::String const localizedProfName = localizeProfessionDisplay(m_selectedProfession);
	if (m_textProfName)
		m_textProfName->SetLocalText(localizedProfName);

	CreatureObject const * const player = Game::getPlayerCreature();
	std::set<std::string> playerSkillNames;
	if (player)
	{
		CreatureObject::SkillList const & playerSkills = player->getSkills();
		for (CreatureObject::SkillList::const_iterator it = playerSkills.begin(); it != playerSkills.end(); ++it)
		{
			if (*it)
				playerSkillNames.insert((*it)->getSkillName());
		}
	}

	bool const handledVisually = tryPopulateGraph4x4(profSkill, playerSkillNames);
	if (handledVisually)
	{
		if (m_textProfessionBody)
			m_textProfessionBody->SetLocalText(Unicode::emptyString);
		return;
	}

	// Non-4x4 fallback: text dump of descendants with [X]/[ ] indicators.
	std::set<SkillObject const *> visited;
	std::vector<SkillObject const *> stack;
	stack.push_back(profSkill);
	std::vector<std::string> descendants;
	while (!stack.empty())
	{
		SkillObject const * const current = stack.back();
		stack.pop_back();
		if (!current || !visited.insert(current).second)
			continue;
		descendants.push_back(current->getSkillName());
		SkillObject::SkillVector const & next = current->getNextSkillBoxes();
		for (SkillObject::SkillVector::const_iterator it = next.begin(); it != next.end(); ++it)
			stack.push_back(*it);
	}
	std::sort(descendants.begin(), descendants.end());

	Unicode::String body;
	for (std::vector<std::string>::const_iterator it = descendants.begin(); it != descendants.end(); ++it)
	{
		bool const has = playerSkillNames.find(*it) != playerSkillNames.end();
		Unicode::String localized;
		if (!CuiSkillManager::localizeSkillName(*it, localized) || localized.empty())
			localized = Unicode::narrowToWide(*it);
		body += Unicode::narrowToWide(has ? "[X] " : "[ ] ");
		body += localized;
		body += Unicode::narrowToWide("\n");
	}
	if (m_textProfessionBody)
		m_textProfessionBody->SetLocalText(body);
}

//-----------------------------------------------------------------------

void SwgCuiSkills::hideAllGraphs()
{
	if (m_pageGraph4x4)     m_pageGraph4x4->SetVisible(false);
	if (m_pageGraph2x4)     m_pageGraph2x4->SetVisible(false);
	if (m_pageGraph1x4)     m_pageGraph1x4->SetVisible(false);
	if (m_pageGraphPyramid) m_pageGraphPyramid->SetVisible(false);
}

//-----------------------------------------------------------------------

void SwgCuiSkills::applyTreeBox(char const * path, std::string const & skillName,
                                std::set<std::string> const & playerSkills, bool nextTrainable)
{
	if (!m_pageGraph4x4 || !path || !path[0])
		return;

	// The Pre-CU ui_skill.inc override wraps each box in a Page (so a child
	// xpbar can render -- UIButton itself does not render children), putting
	// the button at "<path>.b". Fall back to the bare "<path>" button so this
	// also works against the stock un-wrapped layout.
	std::string const btnPath = std::string(path) + ".b";
	UIBaseObject * obj = m_pageGraph4x4->GetObjectFromPath(btnPath.c_str(), TUIButton);
	if (!obj)
		obj = m_pageGraph4x4->GetObjectFromPath(path, TUIButton);
	if (!obj)
		return;

	UIButton * const btn = static_cast<UIButton *>(obj);

	Unicode::String localized;
	if (!CuiSkillManager::localizeSkillName(skillName, localized) || localized.empty())
		localized = Unicode::narrowToWide(skillName);
	// SetText (NOT SetLocalText): UIButton::RenderText early-returns when mText
	// is empty, and SetLocalText only sets mLocalText -- so the box rendered
	// blank. SetText sets BOTH mText (satisfies the guard) and mLocalText (drawn).
	btn->SetText(localized);

	// Green tree_acquired for skills the player has trained, dark tree_default
	// otherwise (both defined in the ui_skill.inc styles section).
	bool const has = playerSkills.find(skillName) != playerSkills.end();
	btn->SetProperty(UILowerString("Style"), Unicode::narrowToWide(
		has ? "/Styles.New.tree_acquired.style" : "/Styles.New.tree_default.style"));

	// Click identifies which skill the user picked for the detail panels.
	m_buttonSkills[btn] = skillName;
	registerMediatorObject(*btn, true);

	applySkillBoxXp(path, btn, skillName, has, nextTrainable);
}

//-----------------------------------------------------------------------

void SwgCuiSkills::applySkillBoxXp(char const * path, UIButton * btn,
                                   std::string const & skillName, bool hasSkill, bool nextTrainable)
{
	if (!btn || !m_pageGraph4x4 || !path)
		return;

	// The xpbar lives at "<path>.xpbar" with a solid 'fill' child whose height
	// + colour we drive (added by the ui_skill.inc override; absent on the
	// stock layout, in which case we just tooltip the box).
	std::string const base(path);
	UIBaseObject * const xpbarObj = m_pageGraph4x4->GetObjectFromPath((base + ".xpbar").c_str(),      TUIPage);
	UIBaseObject * const fillObj  = m_pageGraph4x4->GetObjectFromPath((base + ".xpbar.fill").c_str(), TUIPage);
	UIWidget * const xpbar = xpbarObj ? static_cast<UIWidget *>(xpbarObj) : 0;
	UIWidget * const fill  = fillObj  ? static_cast<UIWidget *>(fillObj)  : 0;

	SkillObject const * const skill = SkillManager::getInstance().getSkill(skillName);
	CreatureObject const * const player = Game::getPlayerCreature();
	SkillObject::ExperiencePair const * const exp = skill ? skill->getPrerequisiteExperience() : 0;

	// Bar shows only on boxes the player can train next (prereq skills met, not
	// yet owned) that carry a real XP requirement.
	bool const showBar = nextTrainable && !hasSkill && exp && exp->second.first > 0 && player;

	if (!showBar)
	{
		if (xpbar)
			xpbar->SetVisible(false);
		Unicode::String name;
		if (CuiSkillManager::localizeSkillName(skillName, name) && !name.empty())
			btn->SetLocalTooltip(name);
		return;
	}

	std::string const & expName = exp->first;
	int const need = exp->second.first;
	int cur = 0;
	if (!player->getExperience(expName, cur))
		cur = 0;

	float ratio = static_cast<float>(cur) / static_cast<float>(need);
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	if (xpbar && fill)
	{
		long const boxH = (xpbar->GetHeight() > 0) ? xpbar->GetHeight() : 46L;
		long fillH = static_cast<long>(static_cast<float>(boxH) * ratio + 0.5f);
		if (fillH < 2 && ratio > 0.0f) fillH = 2;     // keep a sliver visible
		if (fillH > boxH)              fillH = boxH;

		// Height = % of XP earned; bottom-anchored so it grows up from the base.
		// Orange while < 100%, green once the XP requirement is met.
		fill->SetProperty(UILowerString("Size"),
			Unicode::narrowToWide(FormattedString<32>().sprintf("6,%ld", fillH)));
		fill->SetProperty(UILowerString("Location"),
			Unicode::narrowToWide(FormattedString<32>().sprintf("0,%ld", boxH - fillH)));
		fill->SetProperty(UILowerString("BackgroundTint"),
			Unicode::narrowToWide(cur >= need ? "#40FF00" : "#FFAA00"));
		xpbar->SetVisible(true);
	}

	Unicode::String expDisplay;
	if (!CuiSkillManager::localizeExpName(expName, expDisplay) || expDisplay.empty())
		expDisplay = Unicode::narrowToWide(expName);
	Unicode::String skillDisplay;
	if (!CuiSkillManager::localizeSkillName(skillName, skillDisplay) || skillDisplay.empty())
		skillDisplay = Unicode::narrowToWide(skillName);
	Unicode::String tooltip = Unicode::narrowToWide(FormattedString<256>().sprintf(
		"You currently have %d of the %d ", cur, need));
	tooltip += expDisplay;
	tooltip += Unicode::narrowToWide(" experience points required to learn ");
	tooltip += skillDisplay;
	tooltip += Unicode::narrowToWide(".");
	btn->SetLocalTooltip(tooltip);
}

//-----------------------------------------------------------------------

bool SwgCuiSkills::tryPopulateGraph4x4(SkillObject const * novice, std::set<std::string> const & playerSkills)
{
	REPORT_LOG(true, ("SwgCuiSkills::tryPopulateGraph4x4: pageGraph4x4=%p novice=%p (%s)\n",
		(void *)m_pageGraph4x4, (void *)novice,
		novice ? novice->getSkillName().c_str() : "(null)"));

	if (!m_pageGraph4x4 || !novice)
		return false;

	// Use the canonical profession definition table extracted from the
	// SWG Profession Calculator. Each ProfessionDef carries the explicit
	// branch -> skill mapping in canonical column order, so we no longer
	// rely on client-side SkillObject graph linkage (which isn't built)
	// or alphabetical-sort name pattern guessing.
	std::string const noviceName = novice->getSkillName();
	ProfessionDef const * const def = findProfessionDef(noviceName);
	if (!def)
	{
		REPORT_LOG(true, ("SwgCuiSkills::tryPopulateGraph4x4: no ProfessionDef for '%s'\n", noviceName.c_str()));
		return false;
	}

	REPORT_LOG(true, ("SwgCuiSkills::tryPopulateGraph4x4: using ProfessionDef for '%s' (master=%s)\n",
		def->displayName, def->masterSkill ? def->masterSkill : "(none)"));

	// Reset the per-cell skill mapping for the new profession's tree
	// before re-registering buttons + populating labels.
	m_buttonSkills.clear();
	m_linkSkills.clear();

	// Branch "To: X" labels above each column (graph.disciplineNext.<col>.<slot>).
	// Each slot maps to one profession this branch can lead into.
	for (int col = 0; col < 4; ++col)
	{
		for (int slot = 0; slot < 3; ++slot)
		{
			char path[64];
			snprintf(path, sizeof(path), "graph.disciplineNext.%d.%d", col, slot);
			UIBaseObject * const linkObj = m_pageGraph4x4->GetObjectFromPath(path, TUIText);
			if (!linkObj)
				continue;
			UIText * const linkText = static_cast<UIText *>(linkObj);
			char const * const linkRoot = def->branchLinks[col][slot];
			if (!linkRoot || !linkRoot[0])
			{
				linkText->SetLocalText(Unicode::emptyString);
				continue;
			}
			// Resolve link root (e.g. "social_imagedesigner") to a
			// canonical display name by finding the ProfessionDef whose
			// noviceSkill is "<root>_novice". If no match, fall back to
			// localizing the raw key.
			std::string const noviceCandidate = std::string(linkRoot) + "_novice";
			ProfessionDef const * const linkedDef = findProfessionDef(noviceCandidate);
			Unicode::String display;
			if (linkedDef && linkedDef->displayName && linkedDef->displayName[0])
				display = Unicode::narrowToWide(linkedDef->displayName);
			else if (!CuiSkillManager::localizeSkillName(linkRoot, display) || display.empty())
				display = Unicode::narrowToWide(linkRoot);

			Unicode::String label = Unicode::narrowToWide("To: ");
			label += display;
			linkText->SetLocalText(label);

			// Make the link a clickable shortcut to that profession's tree
			// (handled in OnButtonPressed via m_linkSkills).
			if (linkedDef)
			{
				m_linkSkills[linkText] = noviceCandidate;
				registerMediatorObject(*linkText, true);
			}
		}
	}

	for (int col = 0; col < 4; ++col)
	{
		for (int row = 0; row < 4; ++row)
		{
			char const * const skillName = def->branchSkills[col][row];
			if (!skillName || !skillName[0])
				continue;

			// Next-trainable = the prerequisite skill is owned (row 0 needs the
			// novice box; row N needs the box directly below it) and this box is
			// not yet trained. Only such boxes show the XP progress bar.
			char const * const prereq = (row == 0)
				? noviceName.c_str()
				: def->branchSkills[col][row - 1];
			bool const prereqMet = prereq && prereq[0] &&
				playerSkills.find(prereq) != playerSkills.end();
			bool const nextTrainable = prereqMet &&
				(playerSkills.find(skillName) == playerSkills.end());

			char path[64];
			snprintf(path, sizeof(path), "graph.row%d.%d", row, col);
			applyTreeBox(path, skillName, playerSkills, nextTrainable);
		}
	}

	// Master + Novice boxes (graph.master.b / graph.novice.b). Previously this
	// wrote the names into the graph.next.0 / graph.prev.0 "specialist" hint
	// texts, leaving the actual master/novice boxes showing the template's
	// "xxx skill_five_a" placeholder. Now we fill the real boxes, XP-tint them,
	// and register them clickable so their mods/commands populate the panels.
	{
		std::string const masterName = (def->masterSkill && def->masterSkill[0])
			? std::string(def->masterSkill)
			: stripNoviceSuffix(novice->getSkillName()) + "_master";

		// Master is trainable once all four branch tops (row 3) are owned.
		bool allTops = true;
		for (int c = 0; c < 4; ++c)
		{
			char const * const top = def->branchSkills[c][3];
			if (!top || !top[0] || playerSkills.find(top) == playerSkills.end())
			{
				allTops = false;
				break;
			}
		}
		bool const masterNext = allTops && (playerSkills.find(masterName) == playerSkills.end());
		bool const noviceNext = (playerSkills.find(novice->getSkillName()) == playerSkills.end());

		applyTreeBox("graph.master.b", masterName,             playerSkills, masterNext);
		applyTreeBox("graph.novice.b", novice->getSkillName(), playerSkills, noviceNext);
	}

	// Back-links at the bottom (graph.prev.*): the basic profession(s) this
	// elite branches FROM -- e.g. Pistoleer shows "To: Marksman" below Novice.
	// Found by reverse-searching every profession's branchLinks for this
	// profession's root; clickable via m_linkSkills (handled in OnMessage),
	// mirroring the forward "To: X" elite links up top.
	{
		std::string const rootName = stripNoviceSuffix(novice->getSkillName());

		// Clear all four bottom slots first (profession switch may leave stale).
		for (int s = 0; s < 4; ++s)
		{
			char path[64];
			snprintf(path, sizeof(path), "graph.prev.%d", s);
			UIBaseObject * const obj = m_pageGraph4x4->GetObjectFromPath(path, TUIText);
			if (obj)
				static_cast<UIText *>(obj)->SetLocalText(Unicode::emptyString);
		}

		int prevSlot = 0;
		for (int i = 0; i < k_professionDefCount && prevSlot < 4; ++i)
		{
			ProfessionDef const & d = k_professionDefs[i];
			bool linksToUs = false;
			for (int col = 0; col < 4 && !linksToUs; ++col)
				for (int slot = 0; slot < 3; ++slot)
				{
					char const * const lr = d.branchLinks[col][slot];
					if (lr && lr[0] && rootName == lr)
					{
						linksToUs = true;
						break;
					}
				}
			if (!linksToUs)
				continue;

			char path[64];
			snprintf(path, sizeof(path), "graph.prev.%d", prevSlot);
			UIBaseObject * const obj = m_pageGraph4x4->GetObjectFromPath(path, TUIText);
			if (obj && d.displayName && d.noviceSkill)
			{
				UIText * const t = static_cast<UIText *>(obj);
				Unicode::String label = Unicode::narrowToWide("To: ");
				label += Unicode::narrowToWide(d.displayName);
				t->SetLocalText(label);
				m_linkSkills[t] = d.noviceSkill;
				registerMediatorObject(*t, true);
			}
			++prevSlot;
		}
	}

	if (m_pageGraphs)
	{
		m_pageGraphs->SetVisible(true);
		REPORT_LOG(true, ("SwgCuiSkills::tryPopulateGraph4x4:   set m_pageGraphs visible (isVisible=%d)\n",
			m_pageGraphs->IsVisible() ? 1 : 0));
	}
	if (m_pageProfession)
		m_pageProfession->SetVisible(true);
	m_pageGraph4x4->SetVisible(true);
	REPORT_LOG(true, ("SwgCuiSkills::tryPopulateGraph4x4:   set m_pageGraph4x4 visible (isVisible=%d size=%dx%d)\n",
		m_pageGraph4x4->IsVisible() ? 1 : 0,
		static_cast<int>(m_pageGraph4x4->GetSize().x), static_cast<int>(m_pageGraph4x4->GetSize().y)));

	// Default-select the novice skill so the detail panels at the bottom
	// show *something* meaningful as soon as a profession is picked.
	m_selectedSkill = novice->getSkillName();
	populateSelectedSkill();

	return true;
}

//-----------------------------------------------------------------------

void SwgCuiSkills::populateSelectedSkill()
{
	// Clear all four per-skill detail data sources first.
	if (m_dsInfoModsName)   m_dsInfoModsName->Clear();
	if (m_dsInfoModsPoints) m_dsInfoModsPoints->Clear();
	if (m_dsInfoCmdsName)   m_dsInfoCmdsName->Clear();
	if (m_dsInfoCmdsIcons)  m_dsInfoCmdsIcons->Clear();

	// Header text above the bottom panels (e.g. "Strategy III: Volley Fire").
	if (m_textProfessionBody)
	{
		Unicode::String header;
		if (!m_selectedSkill.empty())
		{
			if (!CuiSkillManager::localizeSkillName(m_selectedSkill, header) || header.empty())
				header = Unicode::narrowToWide(m_selectedSkill);
		}
		m_textProfessionBody->SetLocalText(header);
	}

	if (m_selectedSkill.empty())
		return;

	// Pre-CU: render the box's mods + commands from MarcJoyce SKILLS (k_skillBoxData),
	// NOT the client's stock TRE skills.iff (which is NGE -- e.g. it lists "Knockdown
	// Recovery" / "General Ranged" for Novice Marksman). See SwgCuiSkillBoxData.h. We key
	// off m_selectedSkill directly, so boxes absent from the client's skills.iff still populate.

	// Skill Mods (k_skillBoxMods, sorted by skill name).
	if (m_dsInfoModsName && m_dsInfoModsPoints)
	{
		int rowIdx = 0;
		for (int i = 0; i < k_skillBoxModsCount; ++i)
		{
			if (m_selectedSkill != k_skillBoxMods[i].skill)
				continue;

			Unicode::String localizedName;
			if (!CuiSkillManager::localizeSkillModName(k_skillBoxMods[i].mod, localizedName) || localizedName.empty())
				localizedName = prettifyKey(k_skillBoxMods[i].mod);

			appendTableRow(m_dsInfoModsName, m_dsInfoModsPoints, localizedName,
				Unicode::narrowToWide(FormattedString<32>().sprintf("%+d", k_skillBoxMods[i].value)), rowIdx);
			++rowIdx;
		}
	}

	// Commands and Abilities Granted: name-only for V11 (icons V12).
	if (m_dsInfoCmdsName)
	{
		// Two passes so CERTIFICATIONS group together as a visible block first
		// (Pre-CU surfaced certs prominently; they were previously interleaved +
		// scrolled off), abilities second. Both localize to SOE's real Pre-CU
		// names (e.g. cert_rifle_dlt20 -> "DLT20 Rifle Certification").
		int rowIdx = 0;
		for (int pass = 0; pass < 2; ++pass)
		for (int i = 0; i < k_skillBoxCommandsCount; ++i)
		{
			if (m_selectedSkill != k_skillBoxCommands[i].skill)
				continue;
			std::string const cmd = k_skillBoxCommands[i].command;
			bool const isCert = (cmd.compare(0, 5, "cert_") == 0);
			if ((pass == 0) != isCert)   // pass 0: certs only; pass 1: the rest
				continue;

			Unicode::String localizedName;
			// cmd_n STF keys are all lowercase (SOE convention); our command strings
			// are camelCase (e.g. overChargeShot1) -> lowercase for the lookup (mirrors
			// CuiSkillManager.cpp:450) so we show SOE's real Pre-CU command names instead
			// of the prettifyKey fallback. Keep camelCase cmd for the fallback path.
			if (!CuiSkillManager::localizeCmdName(Unicode::toLower(cmd), localizedName) || localizedName.empty())
				localizedName = prettifyKey(cmd);

			char rowKey[16];
			snprintf(rowKey, sizeof(rowKey), "row%d", rowIdx);

			UIData * const nameRow = new UIData;
			nameRow->SetName(rowKey);
			nameRow->SetProperty(UILowerString("Value"), localizedName);
			m_dsInfoCmdsName->AddChild(nameRow);

			if (m_dsInfoCmdsIcons)
			{
				UIData * const iconRow = new UIData;
				iconRow->SetName(rowKey);
				// /styles.icon.command.<lowercasecmd> (blank until Pre-CU icon styles ship).
				std::string iconPath = "/styles.icon.command.";
				for (std::string::const_iterator c = cmd.begin(); c != cmd.end(); ++c)
					iconPath += static_cast<char>(tolower(static_cast<unsigned char>(*c)));
				iconRow->SetProperty(UILowerString("Value"), Unicode::narrowToWide(iconPath));
				m_dsInfoCmdsIcons->AddChild(iconRow);
			}
			++rowIdx;
		}
	}
}
