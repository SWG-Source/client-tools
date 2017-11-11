// ActionsSkill.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "SwgGodClient/FirstSwgGodClient.h"
#include "ActionHack.h"
#include "ActionsSkill.h"
#include "ActionsSkill.moc"
#include "IconLoader.h"
#include "MainFrame.h"

//-----------------------------------------------------------------------

const char * const ActionsSkill::DragMessages::SKILL_DRAGGED = "SkillBrowser::DragMessages::SKILL_DRAGGED";

//----------------------------------------------------------------------

namespace
{
	
	namespace Captions
	{
		const char * const addProfessionToCategory         = "Add a new profession to the currently selected category, then add it to source control and make editable";
		const char * const addSkillToProfession            = "Add a new skill to the currently selected profession, then add it to source control and make editable";
		const char * const erase                           = "Delete a skill from source control and the local filesystem";
		const char * const edit                            = "Check out skill from source control and make editable";		
		const char * const insertCategory                  = "Insert a new category, add it to source control and make editable";
		const char * const revert                          = "Revert skill to version in source control";
		const char * const revertAll                       = "Revert all skills to versions in source control";
		const char * const save                            = "Save the skill locally, and if a server is using the local file, reloads the skill on the server";
		const char * const saveAll                         = "Save all changed skills locally, and if a server uses the local files, reload the skills on the server";
		const char * const submit                          = "Submit skill to source control";
		const char * const submitAll                       = "Submit all changed skills to source control";
		const char * const eraseExperienceRequirementEntry = "Erase the XP requirement entry";
		const char * const eraseSpeciesRequirementEntry    = "Erase the species requirement entry";
		const char * const eraseMissionsRequirementEntry   = "Erase the mission requirement entry";
		const char * const eraseStatisticsRequirementEntry = "Erase the statistic requirement entry";
		const char * const eraseFactionsRequirementEntry   = "Erase the faction requirement entry";
		const char * const eraseCommandsEntry              = "Erase the command entry";
		const char * const eraseStatModsEntry              = "Erase the statistic modofication entry";
		const char * const eraseFactionModsEntry           = "Erase the faction standing modofication entry";
		const char * const revokeSkill                     = "Revoke this skill from the currently selected object.";
	}
}

//-----------------------------------------------------------------------

ActionsSkill::ActionsSkill() :
QObject(),
addProfessionToCategory(0),
addSkillToProfession(0),
edit(0),
erase(0),
insertCategory(0),
revert(0),
revertAll(0),
save(0),
saveAll(0),
submit(0),
submitAll(0),
eraseExperienceRequirementEntry(0),
eraseSpeciesRequirementEntry(0),
eraseMissionsRequirementEntry(0),
eraseStatisticsRequirementEntry(0),
eraseFactionsRequirementEntry(0),
eraseCommandsEntry(0),
eraseStatModsEntry(0),
eraseFactionModsEntry(0),
revokeSkill(0)
{
	QWidget * const p = &MainFrame::getInstance();

	addProfessionToCategory         = new ActionHack(Captions::addProfessionToCategory,          IL_PIXMAP(hi16_mime_document2),    "&Add Profession To Category", 0, p, "skill_add_profession");
	addSkillToProfession            = new ActionHack(Captions::addSkillToProfession,             IL_PIXMAP(hi16_mime_document2),    "&Add Skill To Profession", 0, p, "skill_add_skill");
	erase                           = new ActionHack(Captions::erase,                            IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "skill_edit");
	edit                            = new ActionHack(Captions::edit,                             IL_PIXMAP(hi16_action_edit),       "&Edit", 0, p, "skill_edit");
	insertCategory                  = new ActionHack(Captions::insertCategory,                   IL_PIXMAP(hi16_action_edit),       "&Insert Category", 0, p, "skill_insert_category");
	revert                          = new ActionHack(Captions::revert,                           IL_PIXMAP(hi16_action_revert),     "Revert", 0, p, "skill_revert");
	revertAll                       = new ActionHack(Captions::revertAll,                        IL_PIXMAP(hi16_action_revert),     "Revert All", 0, p, "skill_revert_all");
	save                            = new ActionHack(Captions::save,                             IL_PIXMAP(hi16_action_redo),       "&Save", 0, p, "skill_save");
	saveAll                         = new ActionHack(Captions::saveAll,                          IL_PIXMAP(hi16_action_redo),       "Save&All", 0, p, "skill_save_all");
	submit                          = new ActionHack(Captions::submit,                           IL_PIXMAP(hi16_action_redo),       "Submit", 0, p, "skill_submit");
	submitAll                       = new ActionHack(Captions::submitAll,                        IL_PIXMAP(hi16_action_redo),       "&Submit All", 0, p, "skill_submit_all");
	eraseExperienceRequirementEntry = new ActionHack(Captions::eraseExperienceRequirementEntry,  IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "req_xp_erase");
	eraseSpeciesRequirementEntry    = new ActionHack(Captions::eraseSpeciesRequirementEntry,     IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "req_species_erase");
	eraseMissionsRequirementEntry   = new ActionHack(Captions::eraseMissionsRequirementEntry,    IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "req_missions_erase");
	eraseStatisticsRequirementEntry = new ActionHack(Captions::eraseStatisticsRequirementEntry,  IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "req_statistics_erase");
	eraseFactionsRequirementEntry   = new ActionHack(Captions::eraseFactionsRequirementEntry,    IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "req_factions_erase");
	eraseCommandsEntry              = new ActionHack(Captions::eraseCommandsEntry,               IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "commands_erase");
	eraseStatModsEntry              = new ActionHack(Captions::eraseStatModsEntry,               IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "stat_mods_erase");
	eraseFactionModsEntry           = new ActionHack(Captions::eraseFactionModsEntry,            IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "faction_mods_erase");
	revokeSkill                     = new ActionHack(Captions::revokeSkill,                      IL_PIXMAP(hi16_action_editdelete), "Delete", 0, p, "revoke_skill");
}

//-----------------------------------------------------------------------

ActionsSkill::ActionsSkill(const ActionsSkill & source) :
QObject(source),
addProfessionToCategory(0),
addSkillToProfession(0),
edit(0),
erase(0),
insertCategory(0),
revert(0),
revertAll(0),
save(0),
saveAll(0),
submit(0),
submitAll(0),
eraseExperienceRequirementEntry(0),
eraseSpeciesRequirementEntry(0),
eraseMissionsRequirementEntry(0),
eraseStatisticsRequirementEntry(0),
eraseFactionsRequirementEntry(0),
eraseCommandsEntry(0),
eraseStatModsEntry(0),
eraseFactionModsEntry(0),
revokeSkill(0)
{

}

//-----------------------------------------------------------------------

ActionsSkill::~ActionsSkill()
{	
	addProfessionToCategory = 0;
	addSkillToProfession = 0;
	edit = 0;
	erase = 0;
	insertCategory = 0;
	revert = 0;
	revertAll = 0;
	save = 0;
	saveAll = 0;
	submit = 0;
	submitAll = 0;
	eraseExperienceRequirementEntry = 0;
	eraseSpeciesRequirementEntry = 0;
	eraseMissionsRequirementEntry = 0;
	eraseStatisticsRequirementEntry = 0;
	eraseFactionsRequirementEntry = 0;
	eraseCommandsEntry = 0;
	eraseStatModsEntry = 0;
	eraseFactionModsEntry = 0;
	revokeSkill = 0;
}

//-----------------------------------------------------------------------

ActionsSkill & ActionsSkill::operator = (const ActionsSkill & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}//lint !e1745 // the assignment operator is disabled

//-----------------------------------------------------------------------

