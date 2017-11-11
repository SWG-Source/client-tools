//======================================================================
//
// SwgCuiExpMonitor.cpp
// copyright(c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiExpMonitor.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIdsSkill.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedSkillSystem/LevelManager.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"

#include "UIData.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIString.h"
#include "UIText.h"

#include <unordered_map>
#include <map>
#include <vector>

//======================================================================

namespace SwgCuiExpMonitorNamespace
{
	static const std::string s_unobtainable = "unobtainable";

	bool doesCurrentSkillIncreaseLevel();
	Unicode::String formatLevelExpDescriptions(std::string const & skillName, int const expCurrent, int const expCap);

	const int NUM_TICKS = 20;
}

//======================================================================

using namespace SwgCuiExpMonitorNamespace;

//======================================================================

bool SwgCuiExpMonitorNamespace::doesCurrentSkillIncreaseLevel()
{
	SkillObject const * const skill = CuiSkillManager::getWorkingSkillObject();

	SkillObject::ExperiencePair const * const experience = (skill != 0) ? skill->getPrerequisiteExperience() : 0;

	if (experience != 0)
	{
		std::string const & name = experience->first;
		return LevelManager::canSkillIncreaseLevel(name);
	}
	return false;
}

//----------------------------------------------------------------------

Unicode::String SwgCuiExpMonitorNamespace::formatLevelExpDescriptions(std::string const & skillName, int const expCurrent, int const expCap)
{
	Unicode::String result;

	Unicode::String localizedSkillName;
	CuiSkillManager::localizeExpName(skillName, localizedSkillName);

	CuiStringVariablesData csvd;
	csvd.sourceName = localizedSkillName;
	csvd.digit_i = expCap;

	CuiStringVariablesManager::process(CuiStringIdsSkill::acquire_level_prose, csvd, result);

	csvd.digit_i = expCurrent;

	Unicode::String line2;

	CuiStringVariablesManager::process (CuiStringIdsSkill::level_prose_1, csvd, line2);

	csvd.digit_i = expCap - expCurrent;

	Unicode::String line3;

	CuiStringVariablesManager::process (CuiStringIdsSkill::level_prose_2, csvd, line3);

	result.push_back ('\n');
	result.append(line2);
	result.append(line3);

	return result;
}

//======================================================================

SwgCuiExpMonitor::SwgCuiExpMonitor(UIPage & page)
: CuiMediator("SwgCuiExpMonitor", page)
, m_textSkill(0)
, m_pageBar(0)
, m_callback(new MessageDispatch::Callback)
, m_colorComplete(UIColor::green)
, m_colorDefault(UIColor::yellow)
, m_imageTick(0)
{
	getCodeDataObject(TUIText, m_textSkill, "textSkill");
	getCodeDataObject(TUIPage, m_pageBar, "pageBar");
	getCodeDataObject(TUIImage, m_imageTick, "tick", true);

	const UIData * const codeData = getCodeData();
	if (codeData)
	{
		codeData->GetPropertyColorOrPalette(UILowerString("colorComplete"), m_colorComplete);
		codeData->GetPropertyColorOrPalette(UILowerString("colorDefault"), m_colorDefault);
	}

	m_textSkill->SetPreLocalized(true);

	setIsUpdating(true);
}

//----------------------------------------------------------------------

SwgCuiExpMonitor::~SwgCuiExpMonitor()
{
	delete m_callback;
	m_callback = 0;

	m_textSkill = 0;
	m_pageBar = 0;
	m_imageTick = 0;
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::performActivate()
{
	if (m_textSkill)
		m_textSkill->Clear();

	m_callback->connect(*this, &SwgCuiExpMonitor::onPlayerSkillsChanged, static_cast<CreatureObject::Messages::SkillsChanged *>(0));
	m_callback->connect(*this, &SwgCuiExpMonitor::onPlayerExpChanged, static_cast<PlayerObject::Messages::ExperienceChanged *>(0));
	m_callback->connect(*this, &SwgCuiExpMonitor::onPlayerExpChanged2, static_cast<CreatureObject::Messages::TotalLevelXpChanged *>(0));
	m_callback->connect(*this, &SwgCuiExpMonitor::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	m_callback->connect(*this, &SwgCuiExpMonitor::onWorkingSkillChanged, static_cast<CuiSkillManager::Messages::SelectedCurrentWorkingSkillChanged *>(0));

	updateSkillText();
	placeTicks();
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::performDeactivate()
{
	m_callback->disconnect(*this, &SwgCuiExpMonitor::onPlayerSkillsChanged, static_cast<CreatureObject::Messages::SkillsChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiExpMonitor::onPlayerExpChanged, static_cast<PlayerObject::Messages::ExperienceChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiExpMonitor::onPlayerExpChanged2, static_cast<CreatureObject::Messages::TotalLevelXpChanged *>(0));
	m_callback->disconnect(*this, &SwgCuiExpMonitor::onPlayerSetup, static_cast<CreatureObject::Messages::PlayerSetup *>(0));
	m_callback->disconnect(*this, &SwgCuiExpMonitor::onWorkingSkillChanged, static_cast<CuiSkillManager::Messages::SelectedCurrentWorkingSkillChanged *>(0));
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::updateSkillText()
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return;

	PlayerObject const * const owner = player->getPlayerObject();
	if ((!owner) ||(!owner->isInitialized()))
		return;
	
	SkillObject const * const skill = CuiSkillManager::getWorkingSkillObject();
	if (skill) 
	{		
		getPage().SetVisible(true);
		Unicode::String localizedSkillName;
		CuiSkillManager::localizeSkillName(*skill, localizedSkillName);
		m_textSkill->SetLocalText(localizedSkillName);
	}
	else
	{
		getPage().SetVisible(false);
		StringId noSkill("ui", "exp_monitor_no_skill_yet");
		m_textSkill->SetLocalText(noSkill.localize());
	}

	updateBar();
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::updateBar()
{
	UIWidget * const parent = NON_NULL(m_pageBar->GetParentWidget());

	m_pageBar->SetWidth(0L);
	getPage().SetLocalTooltip(Unicode::emptyString);

	CreatureObject const * const player = Game::getPlayerCreature();
	if (!player)
		return;

	SkillObject const * const skill = CuiSkillManager::getWorkingSkillObject();
	if (!skill) 
	{
		return;
	}

	SkillObject::ExperiencePair const * const exp = skill->getPrerequisiteExperience();
	if (!exp)
	{
		return;
	}

	std::string const & expName = exp->first;
	//-- skip unobtainable exp
	if (expName == s_unobtainable)
		return;

	int expCap = exp->second.first;
	int expCurrent = 0;
	int expCapToSubtract = 0;

	bool const skillIncreasesLevel = doesCurrentSkillIncreaseLevel();

	if (skillIncreasesLevel)
	{
		expCurrent = player->getLevelXp();

		LevelManager::LevelData levelData;
		LevelManager::setLevelDataFromXp(levelData, expCurrent);

		int16 const nextLevel = static_cast<int16>(levelData.currentLevel + 1);

		int const currentCap = LevelManager::getRequiredXpToReachLevel(levelData.currentLevel);
		int const nextCap = LevelManager::getRequiredXpToReachLevel(nextLevel);

		expCapToSubtract = currentCap;
		expCap = nextCap;
	}
	else
	{
		if (!player->getExperience(expName, expCurrent))
		{
			expCurrent = 0;
		}
	}

	if (!expCap)
	{
		return;
	}

	long const usableWidth = parent->GetWidth();

	long const denominator = expCap - expCapToSubtract;

	if (denominator > 0)
	{
		long const desiredWidth = static_cast<long>(static_cast<float>(usableWidth) * (expCurrent - expCapToSubtract) / (denominator));
		//DEBUG_REPORT_LOG(true,("expCurrent = %d expCapToSubtract = %d expCap = %d desiredWidth = %d\n",
		//	expCurrent,
		//	expCapToSubtract,
		//	expCap,
		//	desiredWidth)
		//	);
		m_pageBar->SetWidth(desiredWidth);
		m_pageBar->SetColor(m_colorDefault);
	}
	else
	{
		m_pageBar->SetWidth(usableWidth);
		m_pageBar->SetColor(m_colorDefault);
	}

	Unicode::String tmp;

	if (skillIncreasesLevel)
	{
		tmp = formatLevelExpDescriptions(expName, expCurrent, expCap);
	}
	else
	{
		CuiSkillManager::formatSkillExpDescription(*skill, tmp, true, true);
	}

	getPage().SetLocalTooltip(tmp);

	tmp.clear();
	CuiSkillManager::localizeExpName(expName, tmp);
	m_textSkill->SetLocalTooltip(tmp);
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::onPlayerSkillsChanged(const CreatureObject::Messages::SkillsChanged::Payload & creature)
{
	if (&creature == Game::getConstPlayer())
	{
		updateSkillText();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::onPlayerExpChanged(const PlayerObject::Messages::ExperienceChanged::Payload & creature)
{
	if (&creature == Game::getConstPlayerObject())
	{
		updateBar();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::onPlayerExpChanged2(const CreatureObject::Messages::TotalLevelXpChanged::Payload & creature)
{
	if (&creature == Game::getConstPlayer())
	{
		updateBar();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::onPlayerSetup(const CreatureObject::Messages::PlayerSetup::Payload & creature)
{
	if (&creature == Game::getConstPlayer())
	{
		updateSkillText();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::onWorkingSkillChanged(const std::string &)
{
	updateSkillText();	
}

//----------------------------------------------------------------------

bool SwgCuiExpMonitor::close()
{
	CuiPreferences::setUseExpMonitor(false);
	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiExpMonitor::placeTicks()
{
	if(!m_imageTick)
		return;
	UIWidget * const parent = NON_NULL(m_pageBar->GetParentWidget());

	for(int i = 1; i < NUM_TICKS; ++i)
	{
		UIImage *tickDupe = static_cast<UIImage *>(m_imageTick->DuplicateObject());
		getPage ().AddChild (tickDupe);
		getPage().MoveChild (tickDupe, UIBaseObject::Top);		
		tickDupe->SetVisible(true);
		tickDupe->Link();
		const int posX = parent->GetLocation().x + ((parent->GetSize().x * i) / NUM_TICKS) - (m_imageTick->GetSize().x / 2);
		tickDupe->SetLocation(posX, tickDupe->GetLocation().y);
	}
}
//======================================================================
