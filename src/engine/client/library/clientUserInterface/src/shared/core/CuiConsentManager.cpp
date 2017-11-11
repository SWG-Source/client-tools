//======================================================================

//
// CuiConsentManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiConsentManager.h"

#include "clientUserInterface/CuiConsent.h"
#include "clientUserInterface/CuiWorkspace.h"

#include "UIPage.h"

#include <map>

//======================================================================

std::map<int, CuiConsentManager::CallbackFunction> CuiConsentManager::m_consentMap;
std::map<int, CuiConsent*> CuiConsentManager::m_widgetMap;
std::map<int, CuiConsent*> CuiConsentManager::m_placementMap;
int CuiConsentManager::m_nextId = 0;

//======================================================================

void CuiConsentManager::install ()
{
}

//----------------------------------------------------------------------

void CuiConsentManager::remove ()
{
}

//----------------------------------------------------------------------

int CuiConsentManager::askLocalConsent(const Unicode::String& question, CallbackFunction callback)
{
	int newId = m_nextId++;

	m_consentMap.insert(std::make_pair(newId, callback));

	showConsentWindow(question, newId, true);
	return newId;
}

//----------------------------------------------------------------------

void CuiConsentManager::showConsentWindow(const Unicode::String& question, int id, bool clientOnly)
{
	CuiWorkspace * const workspace = CuiWorkspace::getGameWorkspace();
	if (workspace)
	{
		CuiConsent * const consent = CuiConsent::createInto (workspace->getPage ());
		NOT_NULL (consent);
		consent->setQuestion      (question);
		consent->setId            (id);
		consent->setClientOnly    (clientOnly);
		consent->activate         ();
		consent->setStickyVisible (true);
		consent->setEnabled       (true);
		workspace->addMediator    (*consent);
		m_widgetMap.insert(std::make_pair(id, consent));

		int placement = 0;
		while(m_placementMap.find(placement) != m_placementMap.end())
		{
			++placement;
		}
		//we now have the lowest open placement value
		m_placementMap.insert(std::make_pair(placement, consent));
		setWindowAtPlacement(consent, placement);
	}
}

//----------------------------------------------------------------------

void CuiConsentManager::setWindowAtPlacement(CuiConsent* consent, int placement)
{
	const int startingPlacementX = 100;
	const int startingPlacementY = 100;
	const int deltaX = 20;
	const int deltaY = 20;
	
	UIPoint finalPlacement;

	finalPlacement.x = startingPlacementX + (deltaX * (placement%10));
	finalPlacement.y = startingPlacementY + (deltaY * (placement%10));

	UIPage & page = consent->getPage ();

	if (consent->getContainingWorkspace ())
		consent->getContainingWorkspace ()->autoPositionMovingRect (&page, UIRect (finalPlacement, page.GetSize ()), false);
	else
		page.SetLocation (finalPlacement);
}

//----------------------------------------------------------------------

void CuiConsentManager::handleResponse(int id, bool response)
{
	std::map<int, CallbackFunction>::iterator i = m_consentMap.find(id);
	if(i != m_consentMap.end())
	{
		CallbackFunction func = i->second;
		m_consentMap.erase(i);

		std::map<int, CuiConsent*>::iterator i2 = m_widgetMap.find(id);
		CuiConsent* c = i2->second;
		m_widgetMap.erase(i2);

		for(std::map<int, CuiConsent*>::iterator i3 = m_placementMap.begin(); i3 != m_placementMap.end(); ++i3)
		{
			if(i3->second == c)
			{
				m_placementMap.erase(i3);
				break;
			}
		}

		if(func)
			func(id, response);
	}
}

//----------------------------------------------------------------------

void CuiConsentManager::dismissLocalConsentRequest(int id)
{
	std::map<int, CuiConsent*>::iterator i = m_widgetMap.find(id);
	if(i != m_widgetMap.end())
	{
		CuiConsent* c = i->second;
		m_widgetMap.erase(i);
		std::map<int, CallbackFunction>::iterator i2 = m_consentMap.find(id);
		if(i2 != m_consentMap.end())
			m_consentMap.erase(i2);

		c->deactivate();
	}
}

//======================================================================
