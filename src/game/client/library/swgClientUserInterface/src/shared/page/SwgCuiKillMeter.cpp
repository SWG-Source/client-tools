// ======================================================================
//
// SwgCuiKillMeter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiKillMeter.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedSkillSystem/SkillManager.h"

#include "UIText.h"

// ======================================================================

SwgCuiKillMeter::SwgCuiKillMeter(UIPage & page) :
SwgCuiLockableMediator("SwgCuiKillMeter", page),
m_callback(new MessageDispatch::Callback),
m_lastValue(-1),
m_textKills(0),
m_waitingForSkills(true)
{
	getCodeDataObject(TUIText, m_textKills, "kills");
	
	m_textKills->SetPreLocalized(true);

	setStickyVisible(true);
	setShowFocusedGlowRect(false);
	getPage().SetVisible(true);
	registerMediatorObject(getPage(), true);
}

// ----------------------------------------------------------------------

SwgCuiKillMeter::~SwgCuiKillMeter()
{	
	m_textKills = 0;
	delete m_callback;
	m_callback = 0;
}

// ----------------------------------------------------------------------

void SwgCuiKillMeter::performActivate()
{
	setIsUpdating(true);
	getPage().SetVisible(shouldShowKillMeter());
	m_callback->connect(*this, &SwgCuiKillMeter::onSkillTemplateChanged, static_cast<CuiSkillManager::Messages::ProfessionTemplateChanged *>(0));
	m_lastValue = -1;
}

// ----------------------------------------------------------------------

void SwgCuiKillMeter::performDeactivate()
{
	setIsUpdating(false);
	m_callback->disconnect(*this, &SwgCuiKillMeter::onSkillTemplateChanged, static_cast<CuiSkillManager::Messages::ProfessionTemplateChanged *>(0));
}

// ----------------------------------------------------------------------

void  SwgCuiKillMeter::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	const PlayerObject * const player = Game::getPlayerObject();
	if (!player)
		return;

	if (m_waitingForSkills)
		getPage().SetVisible(shouldShowKillMeter());

	if (!getPage().IsVisible())
		return;

	int value = player->getKillMeter();
	if (value == m_lastValue)
		return;

	char buf[64];
	const size_t buf_size = sizeof(buf);
	snprintf(buf, buf_size, "%d", value);

	m_textKills->SetLocalText(Unicode::narrowToWide(buf));

	if (value <= 0)
	{
		m_textKills->SetProperty(UIText::PropertyName::Style, Unicode::narrowToWide("starwars_12"));
		m_textKills->SetTextColor(UIColor::white);
	}
	else if (value <= 5)
	{
		m_textKills->SetProperty(UIText::PropertyName::Style, Unicode::narrowToWide("starwars_12"));
		m_textKills->SetTextColor(UIColor::white);
	}
	else if (value <= 15)
	{
		m_textKills->SetProperty(UIText::PropertyName::Style, Unicode::narrowToWide("starwars_18"));
		m_textKills->SetTextColor(UIColor::yellow);
	}
	else if (value <= 25)
	{
		m_textKills->SetProperty(UIText::PropertyName::Style, Unicode::narrowToWide("starwars_21"));
		m_textKills->SetTextColor(UIColor::lerp(UIColor::yellow, UIColor::red, 0.5f));
	}
	else
	{
		m_textKills->SetProperty(UIText::PropertyName::Style, Unicode::narrowToWide("starwars_26"));
		m_textKills->SetTextColor(UIColor::red);
	}
	m_lastValue = value;
}

// ----------------------------------------------------------------------

bool SwgCuiKillMeter::shouldShowKillMeter()
{

	PlayerObject const * const playerObject =  Game::getPlayerObject();
	if (!playerObject || playerObject->getSkillTemplate().empty())
	{
		m_waitingForSkills = true;
		return false;
	}
	
	m_waitingForSkills = false;
	
	if (playerObject->getSkillTemplate().find("commando_") == 0)
	{
		return true;
	}
		
	return false;
}

// ----------------------------------------------------------------------

void SwgCuiKillMeter::onSkillTemplateChanged(std::string const &)
{
	getPage().SetVisible(shouldShowKillMeter());
}

// ======================================================================
