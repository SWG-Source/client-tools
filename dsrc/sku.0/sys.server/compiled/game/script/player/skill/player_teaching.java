package script.player.skill;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.skill;
import script.library.sui;
import script.library.prose;
import script.library.group;
import script.library.utils;
import script.library.xp;


public class player_teaching extends script.base_script
{
	public player_teaching()
	{
	}
	public static final String VAR_TEACHING = "teaching";
	public static final String VAR_VALID_SKILLS = "teaching.valid_skills";
	public static final String VAR_SKILL_TO_LEARN = "teaching.skill_to_learn";
	public static final String VAR_TEACHER = "teaching.teacher";
	public static final String VAR_STUDENT = "teaching.student";
	public static final String VAR_SKILL_COST = "teaching.experience";
	
	public static final string_id SID_TEACH = new string_id("sui", "teach");
	public static final string_id SID_STUDENT_TOO_FAR = new string_id("teaching", "student_too_far");
	public static final string_id SID_STUDENT_TOO_FAR_TARGET = new string_id("teaching", "student_too_far_target");
	public static final string_id SID_STUDENT_HAS_OFFER_TO_LEARN = new string_id("teaching", "student_has_offer_to_learn");
	public static final string_id SID_OFFER_GIVEN = new string_id("teaching", "offer_given");
	public static final string_id SID_OFFER_REFUSED = new string_id("teaching", "offer_refused");
	public static final string_id SID_TEACHER_TOO_FAR = new string_id("teaching", "teacher_too_far");
	public static final string_id SID_TEACHER_TOO_FAR_TARGET = new string_id("teaching", "teacher_too_far_target");
	public static final string_id SID_TEACHER_SKILL_LEARNED = new string_id("teaching", "teacher_skill_learned");
	public static final string_id SID_STUDENT_SKILL_LEARNED = new string_id("teaching", "student_skill_learned");
	public static final string_id SID_EXPERIENCE_RECEIVED = new string_id("teaching", "experience_received");
	public static final string_id SID_LEARNING_FAILED = new string_id("teaching", "learning_failed");
	public static final string_id SID_TEACHING_FAILED = new string_id("teaching", "teaching_failed");
	public static final string_id SID_NO_TARGET = new string_id("teaching", "no_target");
	public static final string_id SID_NO_TEACH_SELF = new string_id("teaching", "no_teach_self");
	public static final string_id SID_STUDENT_DEAD = new string_id("teaching", "student_dead");
	public static final string_id SID_NO_SKILLS = new string_id("teaching", "no_skills");
	public static final string_id SID_NO_SKILLS_FOR_STUDENT = new string_id("teaching", "no_skills_for_student");
	public static final string_id SID_NOT_IN_SAME_GROUP = new string_id("teaching", "not_in_same_group");
	
	public static final float TEACHING_RANGE = 10.0f;
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogin(obj_id self) throws InterruptedException
	{
		
		if (utils.hasScriptVar(self, VAR_TEACHING))
		{
			utils.removeScriptVarTree(self, VAR_TEACHING);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgTeachSkillSelected(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_teaching::msgTeachSkillSelected -- "+ params);
		
		if (!utils.hasScriptVar(self, VAR_STUDENT))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id student = utils.getObjIdScriptVar(self, VAR_STUDENT);
		if (!isIdValid(student))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] valid_skills = utils.getStringBatchScriptVar(self, VAR_VALID_SKILLS);
		if (valid_skills == null || valid_skills.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVarTree(self, VAR_TEACHING);
		
		String button = params.getString("buttonPressed");
		if (button.equals("Cancel"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!student.isLoaded())
		{
			LOG("LOG_CHANNEL", self + " ->Your student must be nearby in order to teach.");
			sendSystemMessage(self, SID_STUDENT_TOO_FAR);
			return SCRIPT_CONTINUE;
		}
		
		String student_name = getFirstName(student);
		String teacher_name = getFirstName(self);
		
		location loc_student = getLocation(student);
		location loc_teacher = getLocation(self);
		
		float distance = loc_student.distance(loc_teacher);
		if (distance > TEACHING_RANGE)
		{
			LOG("LOG_CHANNEL", self + " ->You are too far away from "+ student_name + " to teach.");
			prose_package pp = prose.getPackage(SID_STUDENT_TOO_FAR_TARGET, student);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(student, VAR_TEACHING))
		{
			LOG("LOG_CHANNEL", self + " ->"+ student_name + " already has an offer to learn.");
			prose_package pp = prose.getPackage(SID_STUDENT_HAS_OFFER_TO_LEARN, student);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		int row_selected = sui.getListboxSelectedRow(params);
		if (row_selected == -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		String selected_skill = valid_skills[row_selected];
		
		if (selected_skill == null)
		{
			LOG("LOG_CHANNEL", "player_teaching::msgTeachSkillSelected -- selected skill was null for "+ self);
			return SCRIPT_CONTINUE;
		}
		
		dictionary xp_cost = getSkillPrerequisiteExperience(selected_skill);
		LOG("LOG_CHANNEL", "xp_cost ->"+ xp_cost);
		StringBuffer sb = new StringBuffer();
		
		java.util.Enumeration e = xp_cost.keys();
		while (e.hasMoreElements())
		{
			testAbortScript();
			String xp_type = (String)(e.nextElement());
			if (xp_type.length() > 0)
			{
				int xp = xp_cost.getInt(xp_type);
				
				string_id xp_type_id = utils.unpackString("@exp_n:"+ xp_type);
				String xp_type_string = localize(xp_type_id);
				sb.append(xp + " "+ xp_type_string);
			}
		}
		String skill_cost = sb.toString();
		if (skill_cost.length() < 1)
		{
			skill_cost = "0";
		}
		
		LOG("LOG_CHANNEL", self + " ->You offer to teach "+ student_name + " "+ selected_skill + ".");
		string_id skill_id = utils.unpackString("@skl_n:"+ selected_skill);
		LOG("LOG_CHANNEL", "skill_id ->"+ skill_id);
		prose_package pp = prose.getPackage(SID_OFFER_GIVEN, self, student, skill_id, 0);
		sendSystemMessageProse(self, pp);
		
		utils.setScriptVar(student, VAR_SKILL_TO_LEARN, selected_skill);
		utils.setScriptVar(student, VAR_TEACHER, self);
		utils.setScriptVar(student, VAR_SKILL_COST, skill_cost);
		String selected_skill_string = localize(skill_id);
		String prompt = teacher_name + " has offered to teach you "+ selected_skill_string + " ("+ skill_cost + " experience cost.)";
		
		sui.msgbox(student, student, prompt, sui.YES_NO, "msgTeachSkillConfirmed");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgTeachSkillConfirmed(obj_id self, dictionary params) throws InterruptedException
	{
		
		LOG("LOG_CHANNEL", "player_teaching::msgTeachSkillConfirmed");
		
		if (!utils.hasScriptVar(self, VAR_TEACHER))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id teacher = utils.getObjIdScriptVar(self, VAR_TEACHER);
		String selected_skill = utils.getStringScriptVar(self, VAR_SKILL_TO_LEARN);
		String skill_cost = utils.getStringScriptVar(self, VAR_SKILL_COST);
		
		utils.removeScriptVarTree(self, VAR_TEACHING);
		
		String button = params.getString("buttonPressed");
		if (button.equals("Cancel"))
		{
			if (teacher.isLoaded())
			{
				LOG("LOG_CHANNEL", teacher + " ->"+ getFirstName(self) + " has refused your offer to teach.");
				prose_package pp = prose.getPackage(SID_OFFER_REFUSED, self);
				sendSystemMessageProse(teacher, pp);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(teacher))
		{
			LOG("LOG_CHANNEL", "player_teaching::msgTeachSkillConfirmed -- teacher was null for student "+ self);
			return SCRIPT_CONTINUE;
		}
		
		if (selected_skill == null)
		{
			LOG("LOG_CHANNEL", "player_teaching::msgTeachSkillConfirmed -- selected skill was null for "+ teacher + " teaching "+ self);
			return SCRIPT_CONTINUE;
		}
		
		if (!teacher.isLoaded())
		{
			LOG("LOG_CHANNEL", self + " ->Your teacher must be nearby in order to learn.");
			sendSystemMessage(self, SID_TEACHER_TOO_FAR);
			return SCRIPT_CONTINUE;
		}
		
		String student_name = getFirstName(self);
		String teacher_name = getFirstName(teacher);
		
		location loc_student = getLocation(self);
		location loc_teacher = getLocation(teacher);
		
		float distance = loc_student.distance(loc_teacher);
		if (distance > TEACHING_RANGE)
		{
			LOG("LOG_CHANNEL", self + " ->You are too far away from "+ teacher_name + " to learn.");
			prose_package pp = prose.getPackage(SID_TEACHER_TOO_FAR_TARGET, teacher);
			sendSystemMessageProse(self, pp);
			utils.removeScriptVarTree(self, VAR_TEACHING);
			return SCRIPT_CONTINUE;
		}
		
		if (skill.purchaseSkill(self, selected_skill))
		{
			string_id skill_id = utils.unpackString("@skl_n:"+ selected_skill);
			LOG("LOG_CHANNEL", "skill_id ->"+ skill_id);
			LOG("LOG_CHANNEL", self + " ->You learn "+ selected_skill + " from "+ teacher_name + ".");
			LOG("LOG_CHANNEL", teacher + " ->"+ student_name + " learns "+ selected_skill + " from you.");
			prose_package pp = prose.getPackage(SID_STUDENT_SKILL_LEARNED, teacher, skill_id);
			sendSystemMessageProse(self, pp);
			pp = prose.getPackage(SID_TEACHER_SKILL_LEARNED, self, skill_id);
			sendSystemMessageProse(teacher, pp);
			
			int exp = 0;
			
			if (isJedi(teacher) && isJedi(self) && skill_cost.equals("0") && selected_skill.startsWith("jedi_"))
			{
				exp = 40;
			}
			
			if (exp > 0)
			{
				
			}
		}
		else
		{
			LOG("LOG_CHANNEL", self + " -> Learning failed.");
			LOG("LOG_CHANNEL", teacher + " -> Teaching failed.");
			sendSystemMessage(self, SID_LEARNING_FAILED);
			sendSystemMessage(teacher, SID_TEACHING_FAILED);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int teach(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("LOG_CHANNEL", "player_teaching::teach");
		
		if (utils.hasScriptVar(self, VAR_STUDENT))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(target))
		{
			target = getLookAtTarget(self);
			if (!isIdValid(target))
			{
				LOG("LOG_CHANNEL", self + " ->Whom do you want to teach?");
				sendSystemMessage(self, SID_NO_TARGET);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (target == self)
		{
			LOG("LOG_CHANNEL", self + " ->You cannot teach yourself.");
			sendSystemMessage(self, SID_NO_TEACH_SELF);
			return SCRIPT_CONTINUE;
		}
		
		if (isDead(target) || isIncapacitated(target))
		{
			LOG("LOG_CHANNEL", self + " ->"+ getFirstName(target) + " does not feel like being taught right now.");
			prose_package pp = prose.getPackage(SID_STUDENT_DEAD, target);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		LOG("LOG_CHANNEL", "target ->"+ target);
		
		String teacher_name = getFirstName(self);
		String target_name = getFirstName(target);
		
		location loc_teacher = getLocation(self);
		location loc_student = getLocation(target);
		float distance = loc_teacher.distance(loc_student);
		
		if (distance > TEACHING_RANGE)
		{
			LOG("LOG_CHANNEL", self + " ->You must be closer in order to teach.");
			sendSystemMessage(self, SID_STUDENT_TOO_FAR);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(target, VAR_TEACHING))
		{
			LOG("LOG_CHANNEL", self + " ->"+ target_name + " already has an offer to learn.");
			prose_package pp = prose.getPackage(SID_STUDENT_HAS_OFFER_TO_LEARN, target);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		String[] teacher_skills = getSkillListingForPlayer(self);
		if (teacher_skills == null)
		{
			LOG("LOG_CHANNEL", self + " ->You have no skills to teach.");
			sendSystemMessage(self, SID_NO_SKILLS);
			return SCRIPT_CONTINUE;
		}
		
		String[] qual_skills = skill.getQualifiedTeachableSkills(target, self);
		
		if (qual_skills == null)
		{
			LOG("LOG_CHANNEL", self + " -> You have no skills that "+ target_name + " can currently learn.");
			prose_package pp = prose.getPackage(SID_NO_SKILLS_FOR_STUDENT, target);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		Vector valid_skills = new Vector();
		valid_skills.setSize(0);
		for (int i = 0; i < qual_skills.length; i++)
		{
			testAbortScript();
			int idx = utils.getElementPositionInArray(teacher_skills, qual_skills[i]);
			if (idx != -1)
			{
				
				if ((qual_skills[i].indexOf("novice")) == -1 && (qual_skills[i].indexOf("force_sensitive")) == -1)
				{
					valid_skills = utils.addElement(valid_skills, qual_skills[i]);
				}
			}
		}
		
		if (valid_skills.size() < 1)
		{
			LOG("LOG_CHANNEL", self + " -> You have no skills that "+ target_name + " can currently learn.");
			prose_package pp = prose.getPackage(SID_NO_SKILLS_FOR_STUDENT, target);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		if (!group.inSameGroup(self, target))
		{
			LOG("LOG_CHANNEL", self + " ->You must be within the same group as "+ target_name + " in order to teach.");
			prose_package pp = prose.getPackage(SID_NOT_IN_SAME_GROUP, target);
			sendSystemMessageProse(self, pp);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, VAR_STUDENT, target);
		utils.setBatchScriptVar(self, VAR_VALID_SKILLS, valid_skills);
		
		String[] valid_skills_id = new String[valid_skills.size()];
		for (int i = 0; i < valid_skills.size(); i++)
		{
			testAbortScript();
			valid_skills_id[i] = "@skl_n:" + ((String)(valid_skills.get(i)));
		}
		
		sui.listbox(self, self, "Select a skill to teach.", sui.OK_CANCEL, "Select Skill", valid_skills_id, "msgTeachSkillSelected");
		
		return SCRIPT_CONTINUE;
	}
	
}
