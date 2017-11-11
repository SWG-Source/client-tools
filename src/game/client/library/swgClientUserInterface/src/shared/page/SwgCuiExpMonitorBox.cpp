//======================================================================
//
// SwgCuiExpMonitorBox.cpp
// copyright(c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiExpMonitorBox.h"

#include "clientUserInterface/CuiPreferences.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "swgClientUserInterface/SwgCuiExpMonitor.h"

#include "UIData.h"
#include "UIPage.h"
#include "UIString.h"
#include "UIText.h"

//======================================================================
	
SwgCuiExpMonitorBox::SwgCuiExpMonitorBox(UIPage & page) :
CuiMediator("SwgCuiExpMonitorBox", page),
m_experienceMonitor()
{
	SwgCuiExpMonitor * const mon = new SwgCuiExpMonitor(page);
	mon->fetch();
	m_experienceMonitor = mon;

	setStickyVisible(true);
	setSettingsAutoSizeLocation(true, true);

	setState(MS_closeable);
	setState(MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiExpMonitorBox::~SwgCuiExpMonitorBox()
{
	if (m_experienceMonitor)
	{
		m_experienceMonitor->release();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitorBox::performActivate()
{
	CuiPreferences::setUseExpMonitor(true);

	if (m_experienceMonitor) 
	{
		m_experienceMonitor->activate();
	}
}

//----------------------------------------------------------------------

void SwgCuiExpMonitorBox::performDeactivate()
{
	if (m_experienceMonitor) 
	{
		m_experienceMonitor->deactivate();
	}
}
	

//======================================================================
