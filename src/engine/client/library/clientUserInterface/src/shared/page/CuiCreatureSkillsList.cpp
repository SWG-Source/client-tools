//======================================================================
//
// CuiCreatureSkillsList.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================


#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCreatureSkillsList.h"

#include "UIData.h"
#include "UIList.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIDataSource.h"
#include "clientGame/CreatureObject.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "clientUserInterface/CuiSkillManager.h"

//======================================================================

CuiCreatureSkillsList::CuiCreatureSkillsList (UIPage & page) :
CuiMediator     ("CuiCreatureSkillsList", page),
UIEventCallback (),
m_list          (0),
m_text          (0)
{
	getCodeDataObject (TUIList, m_list,  "list");
	getCodeDataObject (TUIText,  m_text,  "text");
}

//----------------------------------------------------------------------

CuiCreatureSkillsList::~CuiCreatureSkillsList ()
{
	m_list = 0;
	m_text = 0;
}

//----------------------------------------------------------------------

void CuiCreatureSkillsList::performActivate   ()
{
	m_list->AddCallback (this);

	m_list->SelectRow (0);
	m_list->SelectRow (-1);
}

//----------------------------------------------------------------------

void CuiCreatureSkillsList::performDeactivate ()
{
	m_list->RemoveCallback (this);
}

//----------------------------------------------------------------------

void CuiCreatureSkillsList::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_list)
	{
		const UIData * const data = m_list->GetDataAtRow (m_list->GetLastSelectedRow ());
		if (data)
		{
			Unicode::String localDesc;
			IGNORE_RETURN (CuiSkillManager::localizeSkillDescription (data->GetName (), localDesc));
			m_text->SetLocalText (localDesc);
			m_text->SetPreLocalized (true);
		}
	}
}

//----------------------------------------------------------------------

void CuiCreatureSkillsList::update            (const CreatureObject & creature)
{
	UIDataSource * dataSource = m_list->GetDataSource ();
	if (!dataSource)
	{
		m_list->SetDataSource (new UIDataSource);
		dataSource = NON_NULL (m_list->GetDataSource ());
	}
	
	dataSource->Clear ();
	
	const CreatureObject::SkillList & skills = creature.getSkills ();
	
	for (CreatureObject::SkillList::const_iterator it = skills.begin (); it != skills.end (); ++it)
	{
		const SkillObject * const skill = *it;
		
		if (skill)
		{
			UIData * const skillData = new UIData;
			
			Unicode::String localSkillName;
			IGNORE_RETURN (CuiSkillManager::localizeSkillName (*skill, localSkillName));
			
			skillData->SetName (skill->getSkillName ());
			IGNORE_RETURN (skillData->SetProperty (UIList::DataProperties::LOCALTEXT, localSkillName));
			IGNORE_RETURN (dataSource->AddChild (skillData));
		}
	} //lint !e429 //skillData not a leak
	
	m_list->SelectRow (0);
}

//======================================================================
