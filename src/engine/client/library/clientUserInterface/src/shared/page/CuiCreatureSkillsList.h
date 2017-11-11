//======================================================================
//
// CuiCreatureSkillsList.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiCreatureSkillsList_H
#define INCLUDED_CuiCreatureSkillsList_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIList;
class UIText;
class CreatureObject;

// ======================================================================

class CuiCreatureSkillsList :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                       CuiCreatureSkillsList (UIPage & page);

	void                           OnGenericSelectionChanged (UIWidget * context);

	void                           performActivate   ();
	void                           performDeactivate ();

	void                           update            (const CreatureObject & creature);

private:
	virtual                       ~CuiCreatureSkillsList ();
	                               CuiCreatureSkillsList ();
	                               CuiCreatureSkillsList (const CuiCreatureSkillsList & rhs);
	CuiCreatureSkillsList &        operator=                 (const CuiCreatureSkillsList & rhs);

private:

	UIList *                       m_list;
	UIText *                       m_text;

};
//======================================================================

#endif
