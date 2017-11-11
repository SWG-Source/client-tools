//======================================================================
//
// SwgCuiAllTargetsSpace_DamagerData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAllTargetsSpace_DamagerData.h"

#include "UIPage.h"

//----------------------------------------------------------------------

SwgCuiAllTargetsSpace::DamagerData::DamagerData(): 
m_damageTimer(0.0f), 
m_damagePage(NULL), 
m_damagePosition() 
{
}

//----------------------------------------------------------------------

SwgCuiAllTargetsSpace::DamagerData::DamagerData(float decayTime, UIPage * pageObject, Vector const & position) : 
m_damageTimer(decayTime), 
m_damagePage(pageObject), 
m_damagePosition(position) 
{
	if (m_damagePage)
	{
		m_damagePage->Attach(0);
	}
}

//----------------------------------------------------------------------

SwgCuiAllTargetsSpace::DamagerData::~DamagerData()
{
	if (m_damagePage)
	{
		m_damagePage->Detach(0);
		m_damagePage = NULL;
	}
}

//----------------------------------------------------------------------

float SwgCuiAllTargetsSpace::DamagerData::getTimer() const
{
	return m_damageTimer;
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::DamagerData::setTimer(float timer)
{
	m_damageTimer = timer;
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::DamagerData::incrementTimer(float timer)
{
	m_damageTimer += timer;
}

//----------------------------------------------------------------------

UIPage const * const SwgCuiAllTargetsSpace::DamagerData::getPage() const
{
	return m_damagePage;
}

//----------------------------------------------------------------------

UIPage * SwgCuiAllTargetsSpace::DamagerData::getPage()
{
	return m_damagePage;
}

//----------------------------------------------------------------------

Vector const & SwgCuiAllTargetsSpace::DamagerData::getPosition() const
{
	return m_damagePosition;
}

//----------------------------------------------------------------------

void SwgCuiAllTargetsSpace::DamagerData::setPosition(Vector const & postion)
{
	m_damagePosition = postion;
}

//======================================================================
