// ActionsSkill.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ActionsSkill_H
#define	_INCLUDED_ActionsSkill_H

//-----------------------------------------------------------------------

#include <qobject.h>

//-----------------------------------------------------------------------

class ActionHack;

//-----------------------------------------------------------------------

class ActionsSkill : public QObject
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762
public:
	ActionsSkill();
	~ActionsSkill();

	struct DragMessages
	{
		static const char* const SKILL_DRAGGED;
	};

	//lint -save
	//lint -e1925 // public data member
	ActionHack*   addProfessionToCategory;
	ActionHack*   addSkillToProfession;
	ActionHack*   edit;
	ActionHack*   erase;
	ActionHack*   insertCategory;
	ActionHack*   revert;
	ActionHack*   revertAll;
	ActionHack*   save;
	ActionHack*   saveAll;
	ActionHack*   submit;
	ActionHack*   submitAll;
	ActionHack*   eraseExperienceRequirementEntry;
	ActionHack*   eraseSpeciesRequirementEntry;
	ActionHack*   eraseMissionsRequirementEntry;
	ActionHack*   eraseStatisticsRequirementEntry;
	ActionHack*   eraseFactionsRequirementEntry;
	ActionHack*   eraseCommandsEntry;
	ActionHack*   eraseStatModsEntry;
	ActionHack*   eraseFactionModsEntry;
	ActionHack*   revokeSkill;


	//lint -restore

private:
	ActionsSkill & operator = (const ActionsSkill & rhs);
	ActionsSkill(const ActionsSkill & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ActionsSkill_H
