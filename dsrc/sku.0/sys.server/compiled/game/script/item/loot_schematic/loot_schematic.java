package script.item.loot_schematic;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.prose;
import script.library.utils;
import script.library.temp_schematic;
import script.library.skill;
import script.library.static_item;
import script.library.beast_lib;


public class loot_schematic extends script.base_script
{
	public loot_schematic()
	{
	}
	public static final String VAR_SCHEMATIC = "loot_schematic.schematic";
	public static final String VAR_SKILL = "loot_schematic.skill";
	public static final String VAR_ABILITY = "loot_schematic.ability";
	public static final String VAR_BEAST_ABILITY = "loot_schematic.beast";
	public static final String VAR_WAYPOINT_NAME = "loot_schematic.waypoint.name";
	public static final String VAR_WAYPOINT_LOC_X = "loot_schematic.waypoint.loc_x";
	public static final String VAR_WAYPOINT_LOC_Y = "loot_schematic.waypoint.loc_y";
	public static final String VAR_WAYPOINT_LOC_Z = "loot_schematic.waypoint.loc_z";
	public static final String VAR_WAYPOINT_LOC_PLANET = "loot_schematic.waypoint.planet";
	public static final String VAR_PET = "loot_schematic.pet";
	public static final String VAR_VEHICLE = "loot_schematic.vehicle";
	public static final String VAR_USES = "loot_schematic.uses";
	public static final String VAR_SKILL_REQ = "loot_schematic.skill_req";
	public static final String VAR_TYPE = "loot_schematic.type";
	
	public static final int TYPE_SCHEMATIC = 1;
	public static final int TYPE_SKILL = 2;
	public static final int TYPE_ABILITY = 3;
	public static final int TYPE_WAYPOINT = 4;
	public static final int TYPE_BEAST_ABILITY = 5;
	
	public static final string_id SID_SCHEMATIC = new string_id("loot_schematic", "schematic");
	public static final string_id SID_SKILL = new string_id("loot_schematic", "skill");
	public static final string_id SID_ABILITY = new string_id("loot_schematic", "ability");
	public static final string_id SID_WAYPOINT = new string_id("loot_schematic", "waypoint");
	public static final string_id SID_BEAST_ABILITY = new string_id("loot_schematic", "beast_ability");
	public static final string_id SID_NOTHING_TO_LEARN = new string_id("loot_schematic", "nothing_to_learn");
	public static final string_id SID_NOT_LEARNABLE_ABILITY = new string_id("loot_schematic", "not_learnable");
	public static final string_id SID_NOT_ENOUGH_SKILL = new string_id("loot_schematic", "not_enough_skill");
	public static final string_id SID_ALREADY_HAVE_SCHEMATIC = new string_id("loot_schematic", "already_have_schematic");
	public static final string_id SID_SCHEMATIC_LEARNED = new string_id("loot_schematic", "schematic_learned");
	public static final string_id SID_USE_SCHEMATIC = new string_id("loot_schematic", "use_schematic");
	public static final string_id SID_MUST_BE_HOLDING = new string_id("loot_schematic", "must_be_holding");
	public static final string_id SID_USE_SKILL = new string_id("loot_schematic", "use_skill");
	public static final string_id SID_USE_ABILITY = new string_id("loot_schematic", "use_ability");
	public static final string_id SID_USE_BEAST_ABILITY = new string_id("loot_schematic", "use_beast_ability");
	public static final string_id SID_USE_WAYPOINT = new string_id("loot_schematic", "use_waypoint");
	public static final string_id SID_USE_PET = new string_id("loot_schematic", "use_pet");
	public static final string_id SID_USE_VEHICLE = new string_id("loot_schematic", "use_vehicle");
	public static final string_id SID_ALREADY_HAVE_SKILL = new string_id("loot_schematic", "already_have_skill");
	public static final string_id SID_UNABLE_TO_LEARN_SKILL = new string_id("loot_schematic", "unable_to_learn_skill");
	public static final string_id SID_ALREADY_HAVE_ABILITY = new string_id("loot_schematic", "already_have_ability");
	public static final string_id SID_UNABLE_TO_ADD_WAYPOINT = new string_id("loot_schematic", "unable_to_add_waypoint");
	public static final string_id SID_WAYPOINT_GRANTED = new string_id("loot_schematic", "waypoint_granted");
	public static final string_id SID_SKILL_GRANTED = new string_id("loot_schematic", "skill_granted");
	public static final string_id SID_SCHEMATIC_ERROR = new string_id("loot_schematic", "schematic_error");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar(self, VAR_SKILL_REQ) && !static_item.isStaticItem(self))
		{
			String skill_req = getStringObjVar(self, VAR_SKILL_REQ);
			
			if (skill_req.startsWith("crafting"))
			{
				String template = getTemplateName(self);
				
				if (template.length() > 0)
				{
					utils.replaceSnowflakeItem(self, template);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		if (!utils.isNestedWithin(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int type = getLootItemType(self);
		switch (type)
		{
			case TYPE_SCHEMATIC:
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE_SCHEMATIC);
			break;
			
			case TYPE_SKILL:
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE_SKILL);
			break;
			
			case TYPE_ABILITY:
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE_ABILITY);
			break;
			
			case TYPE_WAYPOINT:
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE_WAYPOINT);
			break;
			
			case TYPE_BEAST_ABILITY:
			mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE_BEAST_ABILITY);
			break;
			
			default:
			break;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id container = getContainedBy(self);
		if (!isIdValid(container))
		{
			sendSystemMessage(player, SID_MUST_BE_HOLDING);
			return SCRIPT_CONTINUE;
		}
		else if (container != getObjectInSlot(player, "inventory"))
		{
			sendSystemMessage(player, SID_MUST_BE_HOLDING);
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.ITEM_USE)
		{
			int type = getLootItemType(self);
			if (type == -1)
			{
				sendSystemMessage(player, SID_NOTHING_TO_LEARN);
				return SCRIPT_CONTINUE;
			}
			
			if (hasObjVar(self, VAR_SKILL_REQ))
			{
				String skill_req = getStringObjVar(self, VAR_SKILL_REQ);
				if (!hasSkill(player, skill_req))
				{
					
					String classTemplate = getSkillTemplate(player);
					if (!classTemplate.startsWith(skill_req))
					{
						string_id skill_id = utils.unpackString("@skl_n:"+ skill_req);
						prose_package pp = prose.getPackage(SID_NOT_ENOUGH_SKILL, skill_id);
						sendSystemMessageProse(player, pp);
						return SCRIPT_CONTINUE;
					}
				}
			}
			
			switch (type)
			{
				case TYPE_SCHEMATIC:
				if (!hasObjVar(self, VAR_SCHEMATIC))
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				String schematic = getStringObjVar(self, VAR_SCHEMATIC);
				int uses = getIntObjVar(self, VAR_USES);
				
				if (schematic == null || uses == -1)
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				if (hasSchematic(player, schematic))
				{
					sendSystemMessage(player, SID_ALREADY_HAVE_SCHEMATIC);
					return SCRIPT_CONTINUE;
				}
				
				if (uses > 0)
				{
					temp_schematic.grant(player, schematic, uses);
					sendSystemMessage(player, SID_SCHEMATIC_LEARNED);
				}
				else
				{
					if (grantSchematic(player, schematic))
					{
						sendSystemMessage(player, SID_SCHEMATIC_LEARNED);
					}
					else
					{
						sendSystemMessage(player, SID_SCHEMATIC_ERROR);
						return SCRIPT_CONTINUE;
					}
				}
				
				break;
				
				case TYPE_SKILL:
				if (!hasObjVar(self, VAR_SKILL))
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				String item_skill = getStringObjVar(self, VAR_SKILL);
				if (item_skill == null)
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				if (hasSkill(player, item_skill))
				{
					sendSystemMessage(player, SID_ALREADY_HAVE_SKILL);
					return SCRIPT_CONTINUE;
				}
				
				LOG("LOG_CHANNEL", "skill ->"+ item_skill);
				
				if (!skill.grantSkillToPlayer(player, item_skill))
				{
					sendSystemMessage(player, SID_UNABLE_TO_LEARN_SKILL);
					return SCRIPT_CONTINUE;
				}
				
				string_id skill_id = utils.unpackString("@skl_n:"+ item_skill);
				prose_package pp2 = prose.getPackage(SID_SKILL_GRANTED, skill_id);
				sendSystemMessageProse(player, pp2);
				
				break;
				
				case TYPE_ABILITY:
				if (!hasObjVar(self, VAR_ABILITY))
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				String ability = getStringObjVar(self, VAR_ABILITY);
				if (ability == null)
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				if (hasCommand(player, ability))
				{
					sendSystemMessage(player, SID_ALREADY_HAVE_ABILITY);
					return SCRIPT_CONTINUE;
				}
				
				grantCommand(player, ability);
				
				break;
				
				case TYPE_WAYPOINT:
				if (!hasObjVar(self, VAR_WAYPOINT_NAME) || !hasObjVar(self, VAR_WAYPOINT_LOC_X))
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				String name = getStringObjVar(self, VAR_WAYPOINT_NAME);
				int loc_x = getIntObjVar(self, VAR_WAYPOINT_LOC_X);
				int loc_y = getIntObjVar(self, VAR_WAYPOINT_LOC_Y);
				int loc_z = getIntObjVar(self, VAR_WAYPOINT_LOC_Z);
				String planet = getStringObjVar(self, VAR_WAYPOINT_LOC_PLANET);
				
				String loc_str = getStringObjVar(self, VAR_WAYPOINT_LOC_X);
				
				if (name == null || planet == null)
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				location loc = new location(loc_x, loc_y, loc_z, planet);
				
				obj_id waypoint = createWaypointInDatapad(player, loc);
				if (!isIdValid(waypoint))
				{
					sendSystemMessage(player, SID_UNABLE_TO_ADD_WAYPOINT);
					return SCRIPT_CONTINUE;
				}
				
				setWaypointVisible(waypoint, true);
				setWaypointActive(waypoint, true);
				setWaypointName(waypoint, name);
				
				prose_package pp = prose.getPackage(SID_WAYPOINT_GRANTED, name);
				sendSystemMessageProse(player, pp);
				
				break;
				
				case TYPE_BEAST_ABILITY:
				if (!hasObjVar(self, VAR_BEAST_ABILITY))
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				String beastAbility = getStringObjVar(self, VAR_BEAST_ABILITY);
				if (beastAbility == null)
				{
					sendSystemMessage(player, SID_NOTHING_TO_LEARN);
					return SCRIPT_CONTINUE;
				}
				
				if (!beast_lib.isLearnableBeastMasterSkill(beastAbility))
				{
					sendSystemMessage(player, SID_NOT_LEARNABLE_ABILITY);
					return SCRIPT_CONTINUE;
				}
				
				if (beast_lib.hasBeastMasterSkill(player, beastAbility))
				{
					sendSystemMessage(player, SID_ALREADY_HAVE_ABILITY);
					return SCRIPT_CONTINUE;
				}
				
				beast_lib.playerLearnBeastMasterSkill(player, beastAbility);
				
				break;
				
				default:
				sendSystemMessage(player, SID_NOTHING_TO_LEARN);
				return SCRIPT_CONTINUE;
				
			}
			
			destroyObject(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int type = getLootItemType(self);
		if (type < 1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		String[] type_str =
		{
			localize(SID_SCHEMATIC), localize(SID_SKILL), localize(SID_ABILITY), localize(SID_WAYPOINT), localize(SID_ABILITY)
		};
		
		names[idx] = "knowlege_type";
		attribs[idx] = type_str[type - 1];
		idx++;
		
		if (hasObjVar(self, VAR_SKILL_REQ))
		{
			String skill_req = getStringObjVar(self, VAR_SKILL_REQ);
			string_id skill_id = utils.unpackString("@skl_n:"+ skill_req);
			names[idx] = "loot_schematic_skill_required";
			attribs[idx] = localize(skill_id);
			idx++;
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int handlerReInitialize(obj_id self, dictionary params) throws InterruptedException
	{
		Object[] newParams = new Object[1];
		newParams[0] = self;
		utils.callTrigger("OnInitialize", newParams);
		return SCRIPT_CONTINUE;
	}
	
	
	public int getLootItemType(obj_id item) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return -1;
		}
		
		if (hasObjVar(item, VAR_TYPE))
		{
			return getIntObjVar(item, VAR_TYPE);
		}
		else
		{
			
			return 1;
		}
	}
	
	
	public void test(obj_id player) throws InterruptedException
	{
		String selected_skill = utils.getStringScriptVar(player, VAR_SKILL);
		skill.purchaseSkill(player, selected_skill);
	}
}
