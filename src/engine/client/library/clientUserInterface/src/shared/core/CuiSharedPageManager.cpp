//======================================================================
//
// CuiSharedPageManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiSharedPageManager.h"

#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiMediatorFactory.h"

#include "UIPage.h"

#include <vector>
#include <map>
#include <list>

//======================================================================

namespace
{
	typedef std::vector<std::string> StringVector;
	typedef std::map<std::string, StringVector> Groups;

	Groups s_groupMap;
}

//----------------------------------------------------------------------

bool CuiSharedPageManager::hasActiveMediators (const std::string & groupName, const std::string & ignoreMediator)
{
	const StringVector & sv = s_groupMap [groupName];

	bool found = false;

	for (StringVector::const_iterator it = sv.begin (); it != sv.end (); ++it)
	{
		const std::string & mediatorName   = *it;
		if (ignoreMediator == mediatorName)
			continue;

		const CuiMediator * const mediator = CuiMediatorFactory::isInstalled () ? CuiMediatorFactory::get (mediatorName.c_str (), false) : 0;

		if (mediator && mediator->isActive ())
		{
			found = true;
			break;
		}
	}

	return found;
}

//----------------------------------------------------------------------

void CuiSharedPageManager::checkPageVisibility     (const std::string & groupName, UIPage & page, UIPage * ignorePage, const std::string & ignoreMediator, bool wantsVisibility)
{
	const bool found = hasActiveMediators (groupName, ignoreMediator);
	
	if (wantsVisibility)
	{
		if (!found)
		{
			const UIBaseObject::UIObjectList & olist = page.GetChildrenRef ();
			for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
			{
				UIBaseObject * const child = *it;
				if (child != ignorePage && child->IsA (TUIWidget))
				{
					UIWidget * const childWidget = static_cast<UIWidget *>(child);
					childWidget->SetVisible (false);
				}
			}
		}
		
		page.SetVisible (true);
	}
	else
	{
		if (!found)
		{
			page.SetVisible (false);
		}
	}
}

//----------------------------------------------------------------------

void CuiSharedPageManager::registerMediatorType  (const std::string & groupName, const std::string & type)
{
	s_groupMap [groupName].push_back (type);
}

//======================================================================
