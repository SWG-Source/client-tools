package script.systems.combat;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.buff;
import script.library.combat;
import script.library.jedi;
import script.library.proc;
import script.library.prose;
import script.library.static_item;
import script.library.sui;
import script.library.trace;
import script.library.utils;
import script.library.weapons;


public class combat_weapon extends script.base_script
{
	public combat_weapon()
	{
	}
	public static final java.text.NumberFormat floatFormat = new java.text.DecimalFormat("###.##");
	public static final java.text.NumberFormat noDecimalFormat = new java.text.DecimalFormat("###");
	public static final java.text.NumberFormat percentFormat = new java.text.DecimalFormat("###.#%");
	
	public static final String PID_NAME = "wpConversion";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info item) throws InterruptedException
	{
		
		if (!utils.isNestedWithinAPlayer(self))
		{
			return SCRIPT_CONTINUE;
		}
		LOG("dismantle","is nested");
		
		if (utils.isEquipped(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (static_item.isStaticItem(self) && !hasObjVar(self, "wp.canBeDisMantled"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (static_item.isDynamicItem(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (weapons.isCoredWeapon(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (jedi.isLightsaber(self))
		{
			return SCRIPT_CONTINUE;
		}
		LOG("dismantle","not saber");
		
		if (weapons.isProfWheelSchemWeapon(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("dismantle","not profession wheel");
		
		obj_id bioLink = getBioLink(self);
		if (isIdValid(bioLink) && utils.stringToLong(bioLink.toString()) != 1)
		{
			if (bioLink != player)
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		int management_root = item.addRootMenu (menu_info_types.SERVER_MENU5, weapons.SID_WEAPON_TO_SCHEM);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.SERVER_MENU5)
		{
			
			if (!utils.isNestedWithinAPlayer(self))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (utils.isEquipped(self))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (static_item.isStaticItem(self) && !hasObjVar(self, "wp.canBeDisMantled"))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (static_item.isDynamicItem(self))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (weapons.isCoredWeapon(self))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (jedi.isLightsaber(self))
			{
				return SCRIPT_CONTINUE;
			}
			LOG("dismantle","not saber");
			
			if (weapons.isProfWheelSchemWeapon(self))
			{
				return SCRIPT_CONTINUE;
			}
			LOG("dismantle","not profession wheel");
			
			obj_id bioLink = getBioLink(self);
			if (isIdValid(bioLink) && utils.stringToLong(bioLink.toString()) != 1)
			{
				if (bioLink != player)
				{
					return SCRIPT_CONTINUE;
				}
			}
			
			if (sui.hasPid(player, PID_NAME))
			{
				int pid = sui.getPid(player, PID_NAME);
				forceCloseSUIPage(pid);
			}
			
			int pid = sui.inputbox(self, player, "@"+ weapons.SID_CONVERT_PROMPT, sui.OK_CANCEL, "@"+ weapons.SID_CONVERT_TITLE, sui.INPUT_NORMAL, null, "handleConvertSchemSui");
			
			sui.setPid(player, pid, PID_NAME);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void expertiseRangeModify(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return;
		}
		
		if (!hasObjVar(self, "weapon.original_max_range"))
		{
			float weaponRange = 0f;
			
			if (!static_item.isStaticItem(self))
			{
				
				if (!hasObjVar(self, "dynamic_item.intLevelRequired"))
				{
					dictionary weaponDat = weapons.getWeaponDat(self);
					
					if (weaponDat == null)
					{
						return;
					}
					
					weaponRange = (float)weapons.getMaxRangeDistance(weaponDat);
				}
				else
				{
					weaponRange = static_item.getDynamicWeaponRange(self);
				}
			}
			else
			{
				String staticItemName = static_item.getStaticItemName(self);
				weaponRange = (float)dataTableGetInt("datatables/item/master_item/weapon_stats.iff", staticItemName, "max_range_distance");
			}
			
			if (weaponRange > 0)
			{
				setObjVar(self, "weapon.original_max_range", weaponRange);
			}
			else
			{
				CustomerServiceLog("weap_conversion_bad", "POSSIBLE :BROKEN WEAPON: COULD NOT GET MAX RANGE FOR "+ self + " template: "+ getTemplateName(self));
				return;
			}
		}
		
		obj_id holder = getContainedBy(self);
		
		if (!isIdNull(holder) && isPlayer(holder))
		{
			if (self == getObjectInSlot(holder, "hold_r"))
			{
				weapons.adjustWeaponRangeForExpertise(holder, self, true);
			}
			
			else
			{
				weapons.adjustWeaponRangeForExpertise(holder, self, false);
			}
		}
		
		else
		{
			weapons.adjustWeaponRangeForExpertise(null, self, false);
		}
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "weaponConversion", null, 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int weaponConversion(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id owner = utils.getContainingPlayerOrCreature( self );
		int niche = ai_lib.aiGetNiche(owner);
		
		if (niche != 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "isCreatureWeapon"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "QAWeapon"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (static_item.isStaticItem(self))
		{
			expertiseRangeModify(self);
			
			return SCRIPT_CONTINUE;
		}
		else
		{
			
			int currentVersion = getConversionId(self);
			int masterVersion = 0;
			
			boolean isCoredWeapon = weapons.isCoredWeapon(self);
			
			if (isCoredWeapon)
			{
				masterVersion = weapons.CORED_WEAPON_CONVERSION_VERSION;
			}
			else
			{
				masterVersion = weapons.CONVERSION_VERSION;
			}
			
			if (currentVersion != masterVersion)
			{
				if (static_item.isDynamicItem(self))
				{
					
					weapons.staticDynamicWeaponConversion(self);
				}
				else if (isCoredWeapon)
				{
					
					weapons.coredWeaponConversion(self);
				}
				else
				{
					
					weapons.clickCombatWeaponConversion(self);
				}
			}
		}
		
		weapons.validateWeaponDamageType(self);
		
		weapons.validateWeaponRange(self);
		
		weapons.setWeaponData(self);
		
		expertiseRangeModify(self);
		
		if (weapons.checkForIllegalStorytellerWeapon(owner, self))
		{
			weapons.handleIllegalStorytellerWeapon(owner, self, "OnInitialize");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		if (isPlayer(destContainer) && !combat.hasCertification(destContainer, self))
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("spam", "weapon_no_cert"));
			pp = prose.setTT(pp, self);
			sendSystemMessageProse(destContainer, pp);
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTransferred(obj_id self, obj_id sourceContainer, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		expertiseRangeModify(self);
		
		if (isPlayer(destContainer))
		{
			setDamageSkillMods( destContainer, self );
			proc.buildCurrentProcList(destContainer);
		}
		else if (isPlayer(sourceContainer))
		{
			setDamageSkillMods( sourceContainer, null );
			proc.buildCurrentProcList(sourceContainer);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setDamageSkillMods(obj_id player, obj_id weapon) throws InterruptedException
	{
		int intMultiplier = combat.PLAYER_ATTACKER_DAMAGE_LEVEL_MULTIPLIER;
		int intBaseDamage = combat.PLAYER_COMBAT_BASE_DAMAGE;
		
		int weaponMinDamage = 0;
		int weaponMaxDamage = 0;
		if (isIdValid(weapon))
		{
			weaponMinDamage = getWeaponMinDamage(weapon);
			weaponMaxDamage = getWeaponMaxDamage(weapon);
		}
		int playerLevel = getLevel( player );
		
		int minDamage = weaponMinDamage + ( playerLevel * intMultiplier );
		int maxDamage = intBaseDamage + weaponMaxDamage + ( playerLevel * intMultiplier );
		
		int oldMin = getSkillStatisticModifier(player, "minDamage");
		applySkillStatisticModifier(player, "minDamage", 0-oldMin);
		int oldMax = getSkillStatisticModifier(player, "maxDamage");
		applySkillStatisticModifier(player, "maxDamage", 0-oldMax);
		
		applySkillStatisticModifier(player, "minDamage", minDamage);
		applySkillStatisticModifier(player, "maxDamage", maxDamage);
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		
		String at = "@obj_attr_n:";
		boolean staticItem = static_item.isStaticItem(self);
		
		int free = getFirstFreeIndex(names);
		if (free == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		String template = getTemplateName(self);
		if (template != null)
		{
			dictionary itemData = new dictionary();
			
			if (staticItem)
			{
				itemData = static_item.getMasterItemDictionary(self);
			}
			
			int levelRequired = -1;
			if (hasObjVar(self, weapons.OBJVAR_WP_LEVEL))
			{
				levelRequired = getIntObjVar(self, weapons.OBJVAR_WP_LEVEL);
			}
			else
			{
				
				levelRequired = dataTableGetInt(combat.WEAPON_LEVEL_TABLE, template, "weapon_level");
				if (staticItem)
				{
					levelRequired = itemData.getInt("required_level");
				}
				if (static_item.isDynamicItem(self))
				{
					levelRequired = getIntObjVar(self, "dynamic_item.intLevelRequired");
				}
			}
			
			names[free] = "healing_combat_level_required";
			attribs[free++] = "" + levelRequired;
			
			names[free] = "tooltip.healing_combat_level_required";
			attribs[free++] = "" + levelRequired;
			
			String skillRequired = dataTableGetString(combat.WEAPON_LEVEL_TABLE, template, "secondary_restriction");
			
			if (staticItem)
			{
				skillRequired = itemData.getString("required_skill");
			}
			
			String skillRequiredAttribute;
			if (skillRequired != null && !skillRequired.equals(""))
			{
				skillRequiredAttribute = "@ui_roadmap:title_"+ skillRequired;
			}
			else
			{
				skillRequiredAttribute = "None";
			}
			
			if (static_item.isDynamicItem(self))
			{
				skillRequiredAttribute = "None";
			}
			
			names[free] = "skillmodmin";
			attribs[free++] = skillRequiredAttribute;
			
			names[free] = "tooltip.skillmodmin";
			attribs[free++] = skillRequiredAttribute;
			
			int speciesRequired = dataTableGetInt(combat.WEAPON_LEVEL_TABLE, template, "species_restriction");
			if (speciesRequired > 0)
			{
				names[free] = "race_requirement";
				attribs[free++] = "@weapon_detail:species_" + speciesRequired;
				
				names[free] = "tooltip.race_requirement";
				attribs[free++] = "@weapon_detail:species_" + speciesRequired;
			}
			
			if (staticItem)
			{
				
				int tier = itemData.getInt("tier");
				names[free] = "tier";
				attribs[free++] = "" + tier;
				
				names[free] = "tooltip.tier";
				attribs[free++] = "" + tier;
			}
		}
		
		weapon_data weaponData = getWeaponData(self);
		if (weaponData != null)
		{
			
			{
				String wpn_damage_type = "cat_wpn_damage.wpn_damage_type";
				String wpn_damage_min = "cat_wpn_damage.wpn_damage_min";
				String wpn_damage_max = "cat_wpn_damage.wpn_damage_max";
				String wpn_damage_radius = "cat_wpn_damage.wpn_damage_radius";
				String wpn_damage_wound = "cat_wpn_damage.wpn_wound_chance";
				String wpn_elemental_type = "cat_wpn_damage.wpn_elemental_type";
				String wpn_elemental_value = "cat_wpn_damage.wpn_elemental_value";
				String wpn_category = "cat_wpn_damage.wpn_category";
				
				final int damageType = weaponData.damageType;
				names[free] = wpn_damage_type;
				attribs[free++] = at + "armor_eff_" + weapons.getDamageTypeString(damageType);
				
				names[free] = "cat_wpn_damage.wpn_category";
				attribs[free++] = at + "wpn_category_" + weaponData.weaponType;
				
				names[free] = "tooltip.wpn_damage_type";
				attribs[free++] = at + "armor_eff_" + weapons.getDamageTypeString(damageType);
				
				names[free] = "cat_wpn_damage.wpn_attack_speed";
				attribs[free++] = floatFormat.format(weaponData.attackSpeed);
				
				names[free] = "cat_wpn_damage.damage";
				String weaponDamage = Integer.toString(weaponData.minDamage) + " - "+ Integer.toString(weaponData.maxDamage);
				attribs[free++] = weaponDamage;
				
				names[free] = "tooltip.damage";
				attribs[free++] = weaponDamage;
				
				final int elementalType = weaponData.elementalType;
				int elementalValue = 0;
				if (elementalType > 0)
				{
					elementalValue = weaponData.elementalValue;
					if (elementalValue > 0)
					{
						names[free] = wpn_elemental_type;
						attribs[free++] = at + weapons.getDamageTypeString(elementalType);
						names[free] = wpn_elemental_value;
						attribs[free++] = Integer.toString(elementalValue);
						
						names[free] = "tooltip.wpn_elemental_type";
						attribs[free++] = at + weapons.getDamageTypeString(elementalType);
						names[free] = "tooltip.wpn_elemental_value";
						attribs[free++] = Integer.toString(elementalValue);
					}
				}
				
				names[free] = "cat_wpn_damage.weapon_dps";
				float avg = (weaponData.minDamage + weaponData.maxDamage) / 2;
				
				avg += (elementalValue * 2);
				attribs[free++] = "" + (int)(avg / weaponData.attackSpeed);
				
			}
			
			{
				names[free] = "cat_wpn_other.wpn_range";
				String weaponRange = noDecimalFormat.format(weaponData.minRange) + "-"+ noDecimalFormat.format(weaponData.maxRange) + "m";
				attribs[free++] = weaponRange;
				
				names[free] = "tooltip.wpn_range";
				attribs[free++] = weaponRange;
				
			}
			
		}
		
		if (staticItem)
		{
			dictionary itemData = static_item.getStaticItemWeaponDictionary(self);
			String procEffect = itemData.getString("proc_effect");
			if (procEffect != null && !procEffect.equals(""))
			{
				names[free] = utils.packStringId( new string_id ( "proc/proc", "proc_name" ));
				attribs[free++] = utils.packStringId( new string_id ( "ui_buff", procEffect));
			}
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public obj_id getContainingMobile(obj_id item) throws InterruptedException
	{
		obj_id containedBy = getContainedBy(item);
		
		if (!isIdValid(containedBy))
		{
			return null;
		}
		else if (isPlayer(containedBy) || isMob(containedBy))
		{
			return containedBy;
		}
		
		return getContainingMobile(containedBy);
	}
	
	
	public int handleConvertSchemSui(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
		
		int pageId = params.getInt("pageId");
		
		if (!sui.hasPid(player, PID_NAME))
		{
			forceCloseSUIPage(pageId);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has somehow gotten two conversion windows while attempting to convert their old weapon("+self+") to a new cored schematic. Bailing out now");
			return SCRIPT_CONTINUE;
		}
		
		int pid = sui.getPid(player, PID_NAME);
		
		if (pageId != pid)
		{
			forceCloseSUIPage(pageId);
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has somehow gotten two conversion windows while attempting to convert their old weapon("+self+") to a new cored schematic. Bailing out now");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithinAPlayer(self))
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has moved their weapon into a container that is not a player while attempting to convert their old weapon("+self+") to a new cored schematic. Bailing out now");
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isEquipped(self))
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has equipped their old weapon("+self+") while attempting to convert it int a new cored schematic. Bailing out now");
			return SCRIPT_CONTINUE;
		}
		
		obj_id playerContained = utils.getContainingPlayer(self);
		
		if (playerContained != player)
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") traded their old weapon("+self+") to "+ getFirstName(playerContained) + "("+playerContained+") while attempting to convert it int a new cored schematic. They are prolly trying to exploit, bailing out now");
			return SCRIPT_CONTINUE;
		}
		
		obj_id bioLink = getBioLink(self);
		if (isIdValid(bioLink) && utils.stringToLong(bioLink.toString()) != 1)
		{
			if (bioLink != player)
			{
				forceCloseSUIPage(pid);
				sui.removePid(player, PID_NAME);
				CustomerServiceLog("armor_conversion", "Player "+getFirstName(player)+"("+player+") tried to deconstruct an old weapon("+self+") that is biolinked to "+ getFirstName(bioLink) + "("+bioLink+"). They are prolly trying to exploit, bailing out now");
				return SCRIPT_CONTINUE;
			}
		}
		
		String response = sui.getInputBoxText(params);
		
		if (!response.toLowerCase().equals("deconstruct"))
		{
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
			sendSystemMessage(player, weapons.SID_CONVERT_INVALID_RESPONSE);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has entered '"+ response + "' instead of 'deconstruct' so the weapon will not be deconstructed.");
			return SCRIPT_CONTINUE;
		}
		
		if (weapons.turnWeaponIntoSchem(player, self))
		{
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") has decided to convert their old weapon("+self+") to a new cored schematic.");
			sui.removePid(player, PID_NAME);
			sendSystemMessage(player, weapons.SID_CONVERT_CONVERT_SUCCESS);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		else
		{
			sendSystemMessage(player, weapons.SID_CONVERT_CONVERT_FAIL);
			CustomerServiceLog("new_weapon_conversion", "Player "+getFirstName(player)+"("+player+") attempted to convert their old weapon("+self+") to a new cored schematic, and it failed.");
			sui.removePid(player, PID_NAME);
			return SCRIPT_CONTINUE;
		}
	}
}
