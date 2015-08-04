//======================================================================
//
// SwgCuiIncap.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiIncap.h"

#include "UIText.h"
#include "UIPage.h"
#include "UIData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "swgSharedUtility/States.h"


//======================================================================

SwgCuiIncap::SwgCuiIncap (UIPage & page) :
CuiMediator           ("SwgCuiIncap", page),
m_text                (0),
m_valueBar            (0),
m_lastCounter         (-1),
m_lastCounterPrinted  (1),
m_elapsedTimeSecs     (0.0f),
m_maxRange            (0)
{
	getCodeDataObject (TUIText, m_text,     "text");
	getCodeDataObject (TUIPage, m_valueBar, "valueBar");
	setIsUpdating (true);

	m_text->SetPreLocalized (true);
}

//----------------------------------------------------------------------

SwgCuiIncap::~SwgCuiIncap ()
{
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiIncap::performActivate   ()
{
}

//----------------------------------------------------------------------

void SwgCuiIncap::performDeactivate ()
{
}

//----------------------------------------------------------------------

void SwgCuiIncap::update   (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	const CreatureObject * const player = Game::getPlayerCreature ();
	
	if (!player)
		return;
	
	bool isIncapacitated = player->isIncapacitated();
	bool isPerformingDeathBlow = player->getState(States::PerformingDeathBlow);
	if (!isIncapacitated && !isPerformingDeathBlow)
	{
		if (isActive ())
			deactivate ();
		
		return;
	}
	
	static const float UPDATE_TIME = 0.75f;

	int counter = player->getCount ();

	if (!isActive ())
	{
		m_maxRange = counter;
		m_elapsedTimeSecs = UPDATE_TIME;
		activate ();
	}
	
	m_elapsedTimeSecs += deltaTimeSecs;

	if (m_lastCounter != counter)
	{
		m_elapsedTimeSecs = 0.0f;
		m_lastCounter = counter;
	}
	else
	{
		counter = std::max (0, counter - static_cast<int>(m_elapsedTimeSecs));
	}
	
	if (counter != m_lastCounterPrinted)
	{
		m_maxRange = std::max (m_maxRange, counter);
		m_lastCounterPrinted = counter;

		char buf [128];
		const size_t buf_size = sizeof (buf);
		if (isIncapacitated)
			snprintf (buf, buf_size, "Incapacitation Time: %d", counter);
		else
			snprintf (buf, buf_size, "Death Blow Time: %d", counter);
		m_text->SetLocalText (Unicode::narrowToWide (buf));
		
		const int range_counter = std::min (m_maxRange, counter);
		
		const UIWidget * const parent = NON_NULL (m_valueBar->GetParentWidget ());
		const long usableWidth = parent->GetWidth ();
		const long width       = m_maxRange ? (usableWidth * range_counter / m_maxRange) : 0L;
		m_valueBar->SetWidth (width);
	}
}

//======================================================================
