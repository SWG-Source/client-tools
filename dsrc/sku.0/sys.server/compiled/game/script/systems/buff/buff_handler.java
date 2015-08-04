package script.systems.buff;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.armor;
import script.library.buff;
import script.library.camping;
import script.library.chat;
import script.library.collection;
import script.library.combat;
import script.library.colors;
import script.library.create;
import script.library.dot;
import script.library.expertise;
import script.library.event_perk;
import script.library.factions;
import script.library.groundquests;
import script.library.group;
import script.library.healing;
import script.library.holiday;
import script.library.jedi;
import script.library.locations;
import script.library.movement;
import script.library.pclib;
import script.library.proc;
import script.library.prose;
import script.library.regions;
import script.library.skill;
import script.library.stealth;
import script.library.sui;
import script.library.trial;
import script.library.utils;
import script.library.weapons;
import script.library.beast_lib;
import script.library.performance;
import script.library.xp;


public class buff_handler extends script.base_script
{
	public buff_handler()
	{
	}
	public static final float ATTACK_RATE_DEPRICATED = 0.0f;
	public static final float DECAY_RATE_DEPRICATED = 0.0f;
	
	public static final String SKILLS_DATATABLE = "datatables/skill/skills.iff";
	public static final String DATATABLE_BUFF_BUILDER = "datatables/buff/buff_builder.iff";
	
	public static final String INITIAL_GENERAL_PROTECTION = "amor.unmodifiedArmorValue";
	public static final String DATATABLE_COSTUME = "datatables/event/costume.iff";
	public static final String DATATABLE_RADAR_HUE = "datatables/buff_data/radar_hue.iff";
	
	public static final int IMMUNITY_TO_POISON = 1;
	public static final int IMMUNITY_TO_DISEASE = 2;
	public static final int IMMUNITY_TO_FIRE = 3;
	public static final int IMMUNITY_TO_BLEEDING = 4;
	public static final int IMMUNITY_TO_ALL_DOTS = 5;
	public static final int IMMUNITY_TO_ACID = 6;
	public static final int IMMUNITY_TO_ENERGY = 7;
	public static final int IMMUNITY_TO_STUN = 12;
	
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		if (buff.hasBuff(self, "channel_healing"))
		{
			buff.removeBuff(self, "channel_healing");
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int attribAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int attribute = ATTRIB_ERROR;
		
		LOG("expertise", "attribAddBuffHandler effectName: "+ effectName + " subtype: "+ subtype + " value: "+ value);
		
		if (subtype.equals("health"))
		{
			attribute = HEALTH;
		}
		else if (subtype.equals("constitution"))
		{
			attribute = CONSTITUTION;
		}
		else if (subtype.equals("action"))
		{
			attribute = ACTION;
		}
		else if (subtype.equals("stamina"))
		{
			attribute = STAMINA;
		}
		else if (subtype.equals("mind"))
		{
			attribute = MIND;
		}
		else if (subtype.equals("willpower"))
		{
			attribute = WILLPOWER;
		}
		
		if (attribute == ATTRIB_ERROR)
		{
			return SCRIPT_OVERRIDE;
		}
		
		attrib_mod am = new attrib_mod(effectName, attribute, (int)value, duration, ATTACK_RATE_DEPRICATED, DECAY_RATE_DEPRICATED, true, false, false);
		addAttribModifier(self, am);
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int attribRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasAttribModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int attribPercentAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int attribute = ATTRIB_ERROR;
		
		if (subtype.equals("health"))
		{
			attribute = HEALTH;
		}
		else if (subtype.equals("constitution"))
		{
			attribute = CONSTITUTION;
		}
		else if (subtype.equals("action"))
		{
			attribute = ACTION;
		}
		else if (subtype.equals("stamina"))
		{
			attribute = STAMINA;
		}
		else if (subtype.equals("mind"))
		{
			attribute = MIND;
		}
		else if (subtype.equals("willpower"))
		{
			attribute = WILLPOWER;
		}
		
		if (attribute == ATTRIB_ERROR)
		{
			return SCRIPT_OVERRIDE;
		}
		
		long stack = buff.getBuffStackCount(self, buffName) > 1 ? buff.getBuffStackCount(self, buffName) : 1;
		value *= (int)stack;
		
		value = 100.0f + value;
		value /= 100.0f;
		
		int maxAttrib = getUnmodifiedMaxAttrib(self, attribute);
		float newAttrib = (float)maxAttrib * value;
		int delta = (int)newAttrib - maxAttrib;
		
		attrib_mod am = new attrib_mod(effectName, attribute, delta, duration, ATTACK_RATE_DEPRICATED, DECAY_RATE_DEPRICATED, true, false, false);
		addAttribModifier(self, am);
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int attribPercentRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasAttribModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int skillAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		long stack = buff.getBuffStackCount(self, buffName) > 1 ? buff.getBuffStackCount(self, buffName) : 1;
		value *= (int)stack;
		
		addSkillModModifier(self, effectName, subtype, (int)value, duration, false, true);
		
		if ((subtype.startsWith("constitution"))||(subtype.startsWith("stamina")))
		{
			messageTo(self, "recalcPools", null, 1, false);
		}
		else if (subtype.startsWith("expertise_innate_protection_"))
		{
			messageTo(self, "recalcArmor", null, 1, false);
		}
		
		if (subtype.startsWith("movement_resist"))
		{
			messageTo(self, "check_movement_immunity", null, 0.1f, false);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int skillRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		if (effectName.lastIndexOf("_") > 0)
		{
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		}
		
		stopClientEffectObjByLabel(self, effectName);
		combat.removeCombatBuffEffect(self, effectName);
		
		if ((subtype.startsWith("constitution"))||(subtype.startsWith("stamina")))
		{
			
			messageTo(self, "recalcPools", null, 1, false);
		}
		else if (subtype.startsWith("expertise_innate_protection_"))
		{
			messageTo(self, "recalcArmor", null, 1, false);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int armorBreakAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		long stack = buff.getBuffStackCount(self, buffName) > 1 ? buff.getBuffStackCount(self, buffName) : 1;
		value *= (int)stack;
		
		int armorScriptVarValue = 0;
		if (stack == 1)
		{
			armorScriptVarValue = utils.getIntScriptVar(self, armor.SCRIPTVAR_CACHED_GENERAL_PROTECTION);
			utils.setScriptVar(self, INITIAL_GENERAL_PROTECTION, armorScriptVarValue);
		}
		else
		{
			armorScriptVarValue = utils.getIntScriptVar(self, INITIAL_GENERAL_PROTECTION);
		}
		
		if (subtype.equals("player") && isPlayer(self))
		{
			
			float baseMod = getSkillStatisticModifier(self, "expertise_innate_protection");
			baseMod += (float)armorScriptVarValue;
			
			baseMod += getEnhancedSkillStatisticModifierUncapped(self, "expertise_innate_protection_all");
			
			float modIncrease = baseMod * (value/100.0f);
			
			addSkillModModifier(self, effectName, "expertise_innate_protection_all", (int)modIncrease, duration, false, false);
			
			if (utils.isProfession(caster, utils.BOUNTY_HUNTER))
			{
				buff.applyBuff(self, caster, "bh_crit_hit_vuln");
			}
			
		}
		
		else if (subtype.equals("mob") && !isPlayer(self))
		{
			addSkillModModifier(self, effectName, "expertise_innate_protection_all", (int)value, duration, false, true);
			
			if (utils.isProfession(caster, utils.BOUNTY_HUNTER))
			{
				buff.applyBuff(self, caster, "bh_crit_hit_vuln");
			}
		}
		
		messageTo(self, "recalcArmor", null, 1, false);
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int armorBreakRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		if (effectName.lastIndexOf("_") > 0)
		{
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		}
		
		stopClientEffectObjByLabel(self, effectName);
		
		combat.removeCombatBuffEffect(self, effectName);
		messageTo(self, "recalcArmor", null, 1, false);
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int skillPercentAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		float baseMod = getSkillStatisticModifier(self, subtype.substring(0, (subtype.lastIndexOf("_"))));
		baseMod += getEnhancedSkillStatisticModifierUncapped(self, subtype);
		float modIncrease = baseMod * (value/100.0f);
		
		addSkillModModifier(self, effectName, subtype, (int)modIncrease, duration, false, false);
		
		if ((subtype.startsWith("constitution"))||(subtype.startsWith("stamina")))
		{
			messageTo(self, "recalcPools", null, .25f, false);
			
		}
		else if (subtype.startsWith("expertise_innate_protection_"))
		{
			messageTo(self, "recalcArmor", null, .25f, false);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int skillPercentRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		
		combat.removeCombatBuffEffect(self, effectName);
		
		if ((subtype.startsWith("constitution"))||(subtype.startsWith("stamina")))
		{
			
			messageTo(self, "recalcPools", null, .25f, false);
		}
		else if (subtype.startsWith("expertise_innate_protection_"))
		{
			messageTo(self, "recalcArmor", null, .25f, false);
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int check_movement_immunity(obj_id self, dictionary params) throws InterruptedException
	{
		movement.updateMovementImmunity(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int recalcPools(obj_id self, dictionary params) throws InterruptedException
	{
		if (beast_lib.isBeast(self))
		{
			beast_lib.initializeBeastStats(beast_lib.getBeastBCD(self), self);
		}
		else
		{
			if (isPlayer(self))
			{
				skill.recalcPlayerPools(self, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int recalcArmor(obj_id self, dictionary params) throws InterruptedException
	{
		if (beast_lib.isBeast(self))
		{
			beast_lib.initializeBeastStats(beast_lib.getBeastBCD(self), self);
		}
		else
		{
			armor.recalculateArmorForPlayer(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int movementAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (value == 0)
		{
			value = -1;
		}
		
		if (movement.hasMovementModifier(self, effectName))
		{
			movement.removeMovementModifier(self, effectName, false);
		}
		
		movement.applyMovementModifier(self, effectName, value);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int movementRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		
		if (movement.hasMovementModifier(self, effectName))
		{
			movement.removeMovementModifier(self, effectName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int exclusiveProxyAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int[] buffs = buff.getAllBuffs(self);
		
		if (subtype != null && !subtype.equals(""))
		{
			String[] buffsToApply = split(subtype, ',');
			
			for (int i = 0; i < buffsToApply.length; ++i)
			{
				testAbortScript();
				buff.applyBuff(self, caster, buffsToApply[i]);
			}
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < buffs.length; i++)
		{
			testAbortScript();
			String thisBuffName = buff.getBuffNameFromCrc(buffs[i]);
			
			if (thisBuffName.startsWith("exclusive_proxy_"))
			{
				String actualBuff = thisBuffName.substring(16, thisBuffName.length());
				buff.applyBuff(self, self, actualBuff);
			}
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int exclusiveProxyRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int excludeSelfAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int[] buffs = buff.getAllBuffs(self);
		
		for (int i = 0; i < buffs.length; i++)
		{
			testAbortScript();
			String thisBuffName = buff.getBuffNameFromCrc(buffs[i]);
			String actualBuff = "";
			
			if (thisBuffName.startsWith("exclude_self_"))
			{
				actualBuff = thisBuffName.substring(13, thisBuffName.length());
			}
			else
			{
				continue;
			}
			
			obj_id gid = getGroupObject(self);
			
			if (isIdValid(gid))
			{
				obj_id[] groupMembers = getGroupMemberIds(gid);
				
				for (int j = 0; j < groupMembers.length; j++)
				{
					testAbortScript();
					
					float dist = getDistance(groupMembers[j], self);
					if (dist > buff.GROUP_BUFF_DISTANCE)
					{
						continue;
					}
					
					if (!pvpCanHelp(self, groupMembers[j]))
					{
						
						prose_package pp = new prose_package();
						pp.stringId = new string_id("spam", "group_buff_fail_pvp");
						pp.target.set(groupMembers[j]);
						sendSystemMessageProse(self, pp);
						continue;
					}
					
					if (groupMembers[j] != self)
					{
						buff.applyBuff(groupMembers[j], self, actualBuff);
					}
				}
			}
			
			buff.removeBuff(self, thisBuffName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int excludeSelfRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int delayAttackAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, buff.DEBUFF_STATE_PARALYZED + "."+ effectName, duration);
		return SCRIPT_CONTINUE;
	}
	
	
	public int delayAttackRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, buff.DEBUFF_STATE_PARALYZED + "."+ effectName);
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		return SCRIPT_CONTINUE;
	}
	
	
	public int suppressionAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int suppressionRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeSlowDownEffect(self);
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int scriptVarAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		utils.setScriptVar(self, "buff."+ effectName + ".value", value);
		return SCRIPT_CONTINUE;
	}
	
	
	public int scriptVarRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		utils.removeScriptVarTree(self, "buff."+ effectName);
		stopClientEffectObjByLabel(self, effectName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpBonusAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpBonusRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "buff.xpBonus");
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpBonusGeneralAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int playerLevel = getLevel(self);
		if (playerLevel < 90)
		{
			CustomerServiceLog("buff", "xpBonusGeneralAddBuff Buff used by player: "+self+" Name: "+getName(self)+ " Player Level: "+playerLevel+" Effect: "+effectName+" subtype:"+subtype+" duration: "+duration+" value: "+value+" buffName: "+buffName+" caster: "+caster);
			String[] xpArray =
			{
				"crafting", "combat_general", "entertainer", "space_combat_general", "chronicles"
			};
			
			utils.setScriptVar(self, "buff.xpBonusGeneral.types", xpArray);
			utils.setScriptVar(self, "buff.xpBonusGeneral.value", value / 100);
		}
		else if (subtype.equals("tcg_xp_buff"))
		{
			CustomerServiceLog("buff", "xpBonusGeneralAddBuff Buff (TCG SPECIFIC BUFF OBJECT) used by player: "+self+" Name: "+getName(self)+ " Player Level: "+playerLevel+". Since player is 90th Lvl, this player receives a random collection object.");
			obj_id collectionItem = collection.grantRandomCollectionItem(self, "datatables/loot/loot_items/collectible/magseal_loot.iff", "collections");
			if (!isValidId(collectionItem) || !exists(collectionItem))
			{
				CustomerServiceLog("buff", "xpBonusGeneralAddBuff Buff (TCG SPECIFIC BUFF OBJECT) used by player: "+self+" Name: "+getName(self)+ " Player Level: "+playerLevel+". The collection system reports that a collectible object WAS NOT recieved by the player due to an internal error. Please notify SWG design.");
			}
			
			CustomerServiceLog("buff", "xpBonusGeneralAddBuff Buff (TCG SPECIFIC BUFF OBJECT) used by player: "+self+" Name: "+getName(self)+ " Player Level: "+playerLevel+". The collection system reports that the collectible object: "+getName(collectionItem)+" "+collectionItem+" was recieved by the player.");
			buff.removeBuff(self, buffName);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpBonusGeneralRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "buff.xpBonusGeneral");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpGrantedGeneralAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int playerLevel = getLevel(self);
		
		if (playerLevel < 90)
		{
			int xpGranted = xp.grantUnmodifiedXPPercentageOfLevel(self, value);
			
			prose_package pp = new prose_package();
			prose.setStringId(pp, new string_id("collection","reward_xp_amount"));
			prose.setDI(pp, xpGranted);
			sendSystemMessageProse(self, pp);
		}
		else if (subtype.equals("tcg_xp_buff"))
		{
			obj_id collectionItem = collection.grantRandomCollectionItem(self, "datatables/loot/loot_items/collectible/magseal_loot.iff", "collections");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int xpGrantedGeneralRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int craftBonusAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float mod = 0.02f;
		
		if (subtype.equals("artisan"))
		{
			mod = 0.01f;
		}
		
		int intValue = (int)value;
		
		utils.setScriptVar(self, "buff.craftBonus.types", intValue);
		utils.setScriptVar(self, "buff.craftBonus.value", mod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int craftBonusRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "buff.craftBonus");
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stopClientEffectObjByLabel(self, effectName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forcePowerAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		addSkillModModifier(self, effectName, subtype, (int)value, duration, false, false);
		
		messageTo(self,"handleRecalculateForce", null, 0.1f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forcePowerRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		combat.removeCombatBuffEffect(self, effectName);
		stopClientEffectObjByLabel(self, effectName);
		jedi.recalculateForcePower(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void procBuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (!utils.hasScriptVar( self, "procBuffEffects" ))
		{
			String[] procBuffEffects = new String[1];
			procBuffEffects[0] = effectName;
			utils.setScriptVar( self, "procBuffEffects", procBuffEffects );
		}
		else
		{
			Vector procBuffEffects = utils.getResizeableStringArrayScriptVar(self, "procBuffEffects");
			if (utils.getElementPositionInArray(procBuffEffects, effectName) == -1)
			{
				procBuffEffects.addElement(effectName);
				utils.setScriptVar( self, "procBuffEffects", procBuffEffects );
			}
		}
		
		proc.buildCurrentProcList(self);
		return;
	}
	
	
	public void procBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (utils.hasScriptVar( self, "procBuffEffects" ))
		{
			Vector procBuffEffects = utils.getResizeableStringArrayScriptVar(self, "procBuffEffects");
			procBuffEffects.removeElement(effectName);
			if (procBuffEffects.size() < 1)
			{
				utils.removeScriptVar(self, "procBuffEffects");
			}
			else
			{
				utils.setScriptVar(self, "procBuffEffects", procBuffEffects);
			}
		}
		
		stopClientEffectObjByLabel(self, effectName);
		
		proc.buildCurrentProcList(self);
		return;
	}
	
	
	public void reactiveBuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (!utils.hasScriptVar( self, "reacBuffEffects" ))
		{
			String[] reacBuffEffects = new String[1];
			reacBuffEffects[0] = effectName;
			utils.setScriptVar( self, "reacBuffEffects", reacBuffEffects );
		}
		else
		{
			Vector reacBuffEffects = utils.getResizeableStringArrayScriptVar(self, "reacBuffEffects");
			if (utils.getElementPositionInArray(reacBuffEffects, effectName) == -1)
			{
				reacBuffEffects.addElement(effectName);
				utils.setScriptVar( self, "reacBuffEffects", reacBuffEffects );
			}
		}
		
		proc.buildCurrentReacList(self);
		return;
	}
	
	
	public void reactiveBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (utils.hasScriptVar( self, "reacBuffEffects" ))
		{
			Vector reacBuffEffects = utils.getResizeableStringArrayScriptVar(self, "reacBuffEffects");
			reacBuffEffects.removeElement(effectName);
			if (reacBuffEffects.size() < 1)
			{
				utils.removeScriptVar(self, "reacBuffEffects");
			}
			else
			{
				utils.setScriptVar(self, "reacBuffEffects", reacBuffEffects);
			}
		}
		
		stopClientEffectObjByLabel(self, effectName);
		
		proc.buildCurrentReacList(self);
		return;
	}
	
	
	public int stanceAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.playStanceVisual(self, effectName);
		
		if (subtype.equals("expertise_stance"))
		{
			int glanceMod = (int)getSkillStatisticModifier(self, "expertise_stance_glance_buff_line");
			utils.setScriptVar(self, "expertise_stance_glance", glanceMod);
			
			int stanceConstitutionBonus = (int)getSkillStatisticModifier(self, "expertise_stance_constitution");
			
			if (stanceConstitutionBonus > 0)
			{
				skillAddBuffHandler(self, "stanceConstitution", "constitution_modified", duration, stanceConstitutionBonus, "", null);
			}
			
			int stanceSaberShackle = (int)getSkillStatisticModifier(self, "expertise_stance_saber_throw_snare_chance");
			
			if (stanceSaberShackle == 25)
			{
				buff.applyBuff(self, self, "fs_saber_shackle_1");
			}
			else if (stanceSaberShackle == 50)
			{
				buff.applyBuff(self, self, "fs_saber_shackle_2");
			}
			else if (stanceSaberShackle == 75)
			{
				buff.applyBuff(self, self, "fs_saber_shackle_3");
			}
			else if (stanceSaberShackle == 100)
			{
				buff.applyBuff(self, self, "fs_saber_shackle_4");
			}
			
			int stanceSoothingAura = (int)getSkillStatisticModifier(self, "expertise_stance_healing_line_fs_heal");
			
			if (stanceSoothingAura == 25)
			{
				buff.applyBuff(self, self, "fs_soothing_aura_1");
			}
			else if (stanceSoothingAura == 50)
			{
				buff.applyBuff(self, self, "fs_soothing_aura_2");
			}
			else if (stanceSoothingAura == 75)
			{
				buff.applyBuff(self, self, "fs_soothing_aura_3");
			}
			else if (stanceSoothingAura == 100)
			{
				buff.applyBuff(self, self, "fs_soothing_aura_4");
			}
			
			int stanceAnticipateAggression = (int)getSkillStatisticModifier(self, "expertise_stance_anticipate_aggression");
			
			if (stanceAnticipateAggression == 4)
			{
				buff.applyBuff(self, self, "fs_anticipate_aggression_1");
			}
			else if (stanceAnticipateAggression == 8)
			{
				buff.applyBuff(self, self, "fs_anticipate_aggression_2");
			}
			
			int stanceAddToAction = (int)getSkillStatisticModifier(self, "expertise_stance_damage_add_to_action");
			
			if (stanceAddToAction == 2)
			{
				buff.applyBuff(self, self, "fs_reactive_response_1");
			}
			else if (stanceAddToAction == 4)
			{
				buff.applyBuff(self, self, "fs_reactive_response_2");
			}
			
			int stanceForceClarity = (int)getSkillStatisticModifier(self, "expertise_stance_fs_force_clarity");
			
			if (stanceForceClarity > 0)
			{
				addSkillModModifier(self, "expertise_fs_force_clarity_1_proc", "expertise_fs_force_clarity_1_proc", (int)stanceForceClarity, -1, false, true);
				
				messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
			}
			
			int stancePerceptiveSentinel = (int)getSkillStatisticModifier(self, "expertise_stance_perceptive_sentinel");
			
			if (stancePerceptiveSentinel == 1)
			{
				buff.applyBuff(self, self, "fs_perceptive_sentinel_1");
			}
			else if (stancePerceptiveSentinel == 2)
			{
				buff.applyBuff(self, self, "fs_perceptive_sentinel_2");
			}
			else if (stancePerceptiveSentinel == 3)
			{
				buff.applyBuff(self, self, "fs_perceptive_sentinel_3");
			}
			else if (stancePerceptiveSentinel == 4)
			{
				buff.applyBuff(self, self, "fs_perceptive_sentinel_4");
			}
		}
		
		if (subtype.equals("expertise_focus"))
		{
			int critMod = (int)getSkillStatisticModifier(self, "expertise_focus_critical_buff_line");
			utils.setScriptVar(self, "expertise_stance_critical", critMod);
			
			float stanceStaminaBonus = (float)getSkillStatisticModifier(self, "expertise_focus_stamina");
			float stanceSrengthBonus = (float)getSkillStatisticModifier(self, "expertise_focus_strength");
			
			if (stanceStaminaBonus > 0)
			{
				skillAddBuffHandler(self, "focusStamina", "stamina_modified", duration, stanceStaminaBonus, "", null);
			}
			
			if (stanceSrengthBonus > 0)
			{
				skillAddBuffHandler(self, "focusStrength", "strength_modified", duration, stanceSrengthBonus, "", null);
			}
			
			int focusDamageIncrease = (int)getSkillStatisticModifier(self, "expertise_focus_damage_increase");
			
			if (focusDamageIncrease == 2)
			{
				buff.applyBuff(self, self, "fs_ruthless_precision_1");
			}
			else if (focusDamageIncrease == 4)
			{
				buff.applyBuff(self, self, "fs_ruthless_precision_2");
			}
			else if (focusDamageIncrease == 6)
			{
				buff.applyBuff(self, self, "fs_ruthless_precision_3");
			}
			else if (focusDamageIncrease == 8)
			{
				buff.applyBuff(self, self, "fs_ruthless_precision_4");
			}
			
			int focusActionGain = (int)getSkillStatisticModifier(self, "expertise_focus_damage_add_to_action");
			
			if (focusActionGain == 2)
			{
				buff.applyBuff(self, self, "fs_tempt_hatred_1");
			}
			else if (focusActionGain == 4)
			{
				buff.applyBuff(self, self, "fs_tempt_hatred_2");
			}
			
			int wracking_energy = getEnhancedSkillStatisticModifierUncapped(self, "expertise_fs_dm_armor_bypass");
			
			if (wracking_energy > 0)
			{
				buff.applyBuff(self, self, "fs_wracking_energy_"+wracking_energy / 25);
			}
			
			int imp_drain = getEnhancedSkillStatisticModifierUncapped(self, "expertise_fs_imp_drain");
			
			if (imp_drain == 33)
			{
				buff.applyBuff(self, self, "fs_imp_force_drain_1");
			}
			else if (imp_drain == 66)
			{
				buff.applyBuff(self, self, "fs_imp_force_drain_2");
			}
			else if (imp_drain == 100)
			{
				buff.applyBuff(self, self, "fs_imp_force_drain_3");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stanceRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		int stanceCritical = utils.getIntScriptVar(self, "expertise_stance_critical");
		int stanceGlance = utils.getIntScriptVar(self, "expertise_stance_glance");
		
		utils.removeScriptVarTree(self, "expertise_stance_glance");
		utils.removeScriptVarTree(self, "expertise_stance_critical");
		removeAttribOrSkillModModifier(self, "stanceConstitution");
		removeAttribOrSkillModModifier(self, "focusStamina");
		removeAttribOrSkillModModifier(self, "focusStrength");
		
		messageTo(self, "recalcPools", null, 1, false);
		
		buff.removeBuff(self, "fs_saber_shackle_1");
		buff.removeBuff(self, "fs_saber_shackle_2");
		buff.removeBuff(self, "fs_saber_shackle_3");
		buff.removeBuff(self, "fs_saber_shackle_4");
		
		buff.removeBuff(self, "fs_soothing_aura_1");
		buff.removeBuff(self, "fs_soothing_aura_2");
		buff.removeBuff(self, "fs_soothing_aura_3");
		buff.removeBuff(self, "fs_soothing_aura_4");
		
		buff.removeBuff(self, "fs_anticipate_aggression_1");
		buff.removeBuff(self, "fs_anticipate_aggression_2");
		
		buff.removeBuff(self, "fs_reactive_response_1");
		buff.removeBuff(self, "fs_reactive_response_2");
		
		buff.removeBuff(self, "fs_perceptive_sentinel_1");
		buff.removeBuff(self, "fs_perceptive_sentinel_2");
		buff.removeBuff(self, "fs_perceptive_sentinel_3");
		buff.removeBuff(self, "fs_perceptive_sentinel_4");
		
		buff.removeBuff(self, "fs_saber_reflect");
		
		buff.removeBuff(self, "fs_ruthless_precision_1");
		buff.removeBuff(self, "fs_ruthless_precision_2");
		buff.removeBuff(self, "fs_ruthless_precision_3");
		buff.removeBuff(self, "fs_ruthless_precision_4");
		
		buff.removeBuff(self, "fs_tempt_hatred_1");
		buff.removeBuff(self, "fs_tempt_hatred_2");
		
		buff.removeBuff(self, "fs_wracking_energy_1");
		buff.removeBuff(self, "fs_wracking_energy_2");
		buff.removeBuff(self, "fs_wracking_energy_3");
		buff.removeBuff(self, "fs_wracking_energy_4");
		
		buff.removeBuff(self, "fs_imp_force_drain_1");
		buff.removeBuff(self, "fs_imp_force_drain_2");
		buff.removeBuff(self, "fs_imp_force_drain_3");
		buff.removeBuff(self, "fs_imp_force_drain_4");
		
		removeAttribOrSkillModModifier(self, "expertise_fs_force_clarity_1_proc");
		messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		combat.removeCombatBuffEffect(self, effectName);
		utils.removeScriptVarTree(self, "stance."+ subtype);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dancePartyAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String[] danceList =
		{
			"poplock",
			"poplock2",
			"popular",
			"popular2",
			"bunduki",
			"bunduki2",
			"breakdance",
			"breakdance2",
			"peiyi",
			"freestyle",
			"freestyle2",
			"jazzy",
			"jazzy2"
		};
		
		String randomDance = danceList[rand(0, danceList.length - 1)];
		performance.effect(self, performance.PERFORMANCE_EFFECT_DANCE_FLOOR, 1);
		obj_id[] everyone = getPlayerCreaturesInRange(getLocation(self), 20.f );
		
		for (int i = 0; i < everyone.length; i++)
		{
			testAbortScript();
			
			if (!combat.isInCombat(everyone[i]))
			{
				utils.setScriptVar(everyone[i], "event.dance_party", 1);
				performance.startDance(everyone[i], randomDance);
				performance.changeDance(everyone[i], randomDance);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dancePartyRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int missByLuckAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int luckPercentage = getSkillStatisticModifier(caster, "expertise_miss_by_luck");
		
		float missIncrease = luckPercentage + 4.0f;
		
		int luckMod = getEnhancedSkillStatisticModifierUncapped(caster, "luck");
		float luck = (float)getEnhancedSkillStatisticModifierUncapped(caster, "luck_modified") + luckMod;
		
		float missAmount = luck * (missIncrease / 100.0f);
		
		skillAddBuffHandler(self, "missByLuck", "combat_all_attack_miss", duration, missAmount, "", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int missByLuckRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "missByLuck");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hitByLuckAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int luckPercentage = getSkillStatisticModifier(caster, "expertise_hit_by_luck");
		
		float hitIncrease = luckPercentage + 4.0f;
		
		int luckMod = getEnhancedSkillStatisticModifierUncapped(caster, "luck");
		float luck = (float)getEnhancedSkillStatisticModifierUncapped(caster, "luck_modified") + luckMod;
		
		float hitAmount = luck * (hitIncrease / 100.0f);
		
		skillAddBuffHandler(self, "hitByLuck", "hit_table_defender_add_hit", duration, hitAmount, "", null);
		
		luckPercentage = getSkillStatisticModifier(caster, "expertise_increase_hit_by_luck");
		
		hitAmount = luck * (luckPercentage / 100.0f);
		
		skillAddBuffHandler(self, "increaseHitByLuck", "hit_table_defender_add_hit", duration, hitAmount, "", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hitByLuckRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "hitByLuck");
		removeAttribOrSkillModModifier(self, "increaseHitByLuck");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pistolWhipAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int pistolWhipStun = getSkillStatisticModifier(caster, "expertise_stun_line_sm_pistol_whip");
		
		if (pistolWhipStun > 0 && rand(0,99) < pistolWhipStun)
		{
			movementAddBuffHandler(self, effectName, subtype, duration, value, buffName, caster);
		}
		else
		{
			buff.removeBuff(self, "sm_pistol_whip");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pistolWhipRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		movementRemoveBuffHandler(self, effectName, subtype, duration, value, buffName, caster);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int slyLieAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int dodgeBonus = (int)getSkillStatisticModifier(self, "expertise_half_truth");
		
		if (dodgeBonus > 0)
		{
			skillAddBuffHandler(self, "slyLieDodge", "combat_dodge", duration, dodgeBonus, "", null);
		}
		
		int strikethroughBonus = (int)getSkillStatisticModifier(self, "expertise_innocent_cargo");
		
		if (strikethroughBonus > 0)
		{
			skillAddBuffHandler(self, "innocentCargoStrikethrough", "combat_strikethrough_chance", duration, strikethroughBonus, "", null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int slyLieRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		removeAttribOrSkillModModifier(self, "slyLieDodge");
		removeAttribOrSkillModModifier(self, "innocentCargoStrikethrough");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fastTalkAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int agilityBonus = (int)getSkillStatisticModifier(self, "expertise_fake_id");
		
		if (agilityBonus > 0)
		{
			skillAddBuffHandler(self, "fastTalkAgility", "agility_modified", duration, agilityBonus, "", null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fastTalkRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		removeAttribOrSkillModModifier(self, "fastTalkAgility");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvpAuraBuffSelfAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		attachScript (self, "player.gcw.pvp_aura_buff_controller");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvpAuraBuffSelfRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		detachScript (self, "player.gcw.pvp_aura_buff_controller");
		removeObjVar (self, "pvp_aura_buff.faction");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int nextHitCritAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "nextCritHit", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int nextHitCritRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "nextCritHit");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critDoubleDamageAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "critDoubleDamage", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critDoubleDamageRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "critDoubleDamage");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critRootAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "critRoot", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critRootRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "critRoot");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critOnceAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		Vector effectNames = new Vector();
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (utils.hasScriptVar(self, "critRemoveBuffNames"))
		{
			effectNames = utils.getResizeableStringArrayScriptVar(self, "critRemoveBuffNames");
		}
		
		utils.addElement(effectNames, effectName);
		
		utils.setScriptVar(self, "critRemoveBuffNames", effectNames);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int critOnceRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		Vector effectNames = new Vector();
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		if (utils.hasScriptVar(self, "critRemoveBuffNames"))
		{
			effectNames = utils.getResizeableStringArrayScriptVar(self, "critRemoveBuffNames");
		}
		
		int i = 0;
		
		while (i < effectNames.size())
		{
			testAbortScript();
			if (effectName.equals(((String)(effectNames.get(i)))))
			{
				utils.removeElementAt(effectNames, i);
			}
			else
			{
				i++;
			}
		}
		
		if (effectNames.size() == 0)
		{
			utils.removeScriptVar(self, "critRemoveBuffNames");
		}
		else
		{
			utils.setScriptVar(self, "critRemoveBuffNames", effectNames);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int junkDealerAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id player = utils.getObjIdScriptVar(self, "junkDealerBuffer");
		
		utils.removeScriptVar(self, "junkDealerBuffer");
		
		if (!isIdValid(player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int precisionBonus = (int)getSkillStatisticModifier(player, "expertise_buff_under_the_counter");
		
		if (precisionBonus > 0)
		{
			skillAddBuffHandler(self, "junkDealerPrecision", "precision_modified", duration, precisionBonus, "", null);
		}
		
		int damageDecrease = (int)getSkillStatisticModifier(player, "expertise_buff_best_deal_ever");
		
		if (damageDecrease > 0)
		{
			skillAddBuffHandler(self, "junkDealerDamageDecrease", "damage_decrease_percentage", duration, damageDecrease, "", null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int junkDealerRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "junkDealerPrecision");
		removeAttribOrSkillModModifier(self, "junkDealerDamageDecrease");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoSnareBonusAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!isIdValid(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		String[] modifiers = movement.getAllModifiers(self);
		
		boolean snared = false;
		
		if (modifiers != null)
		{
			for (int i = 0; i < modifiers.length; i++)
			{
				testAbortScript();
				if (movement.getType(modifiers[i]) == movement.MT_SNARE)
				{
					snared = true;
				}
			}
		}
		
		if (!snared)
		{
			buff.removeBuff(self, "co_youll_regret_that");
			
			return SCRIPT_OVERRIDE;
		}
		
		int innateArmorBonus = (int)getSkillStatisticModifier(self, "expertise_youll_regret_that");
		
		if (innateArmorBonus > 0)
		{
			skillAddBuffHandler(self, "commandoInnateArmorBonus", "expertise_innate_protection_all", duration, innateArmorBonus, "", null);
			messageTo(self, "recalcArmor", null, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoSnareBonusRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "commandoInnateArmorBonus");
		messageTo(self, "recalcArmor", null, .25f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoFlashBangAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		obj_id owner = utils.getObjIdScriptVar(self, "buffOwner."+ getStringCrc(effectName.toLowerCase()));
		
		if (!isIdValid(owner))
		{
			buff.removeBuff(self, effectName);
			return SCRIPT_CONTINUE;
		}
		
		int precisionPenalty = (int)getSkillStatisticModifier(owner, "expertise_co_flash_bang");
		
		if (precisionPenalty > 0)
		{
			skillAddBuffHandler(self, "commandoFlashBang", "precision_modified", duration, (precisionPenalty * -1), "", null);
		}
		else
		{
			buff.removeBuff(self, effectName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoFlashBangRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "commandoFlashBang");
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoMuscleSpasmAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		obj_id owner = utils.getObjIdScriptVar(self, "buffOwner."+ getStringCrc(effectName.toLowerCase()));
		
		if (!isIdValid(owner))
		{
			buff.removeBuff(self, effectName);
			return SCRIPT_CONTINUE;
		}
		
		int glancePenalty = (int)getSkillStatisticModifier(owner, "expertise_co_muscle_spasm");
		
		if (glancePenalty > 0)
		{
			skillAddBuffHandler(self, "commandoMuscleSpasm", "glancing_blow_vulnerable", duration, glancePenalty, "", null);
		}
		else
		{
			buff.removeBuff(self, effectName);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoMuscleSpasmRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "commandoMuscleSpasm");
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoRiddleArmorAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String tempEffectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		obj_id owner = utils.getObjIdScriptVar(self, "buffOwner."+ getStringCrc(tempEffectName.toLowerCase()));
		
		if (!isIdValid(owner))
		{
			buff.removeBuff(self, tempEffectName);
			return SCRIPT_CONTINUE;
		}
		
		value -= getSkillStatisticModifier(owner, "expertise_riddle_armor");
		
		skillAddBuffHandler(self, effectName, "expertise_innate_protection_all", duration, value, "", null);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandoRiddleArmorRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, effectName);
		messageTo(self, "recalcArmor", null, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int radarInvisAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		setVisibleOnMapAndRadar(self, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int radarInvisRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		setVisibleOnMapAndRadar(self, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onTargetAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("expertise_on_target"))
		{
			
			int posSecuredImproved = (int)getSkillStatisticModifier(self, "expertise_action_line_co_imp_pos_sec");
			
			if (posSecuredImproved == 10)
			{
				buff.applyBuff(self, self, "co_pos_sec_action_1");
			}
			else if (posSecuredImproved == 20)
			{
				buff.applyBuff(self, self, "co_pos_sec_action_2");
			}
			else if (posSecuredImproved == 30)
			{
				buff.applyBuff(self, self, "co_pos_sec_action_3");
			}
			
			int posSecuredBurstFire = (int)getSkillStatisticModifier(self, "expertise_co_pos_secured_line_burst_fire_proc");
			
			if (posSecuredBurstFire == 10)
			{
				buff.applyBuff(self, self, "co_pos_sec_proc_1");
				messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
			}
			else if (posSecuredBurstFire == 20)
			{
				buff.applyBuff(self, self, "co_pos_sec_proc_2");
				messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
			}
			
			int posSecuredOnTarget = (int)getSkillStatisticModifier(self, "expertise_co_pos_secured_line_critical");
			
			if (posSecuredOnTarget == 2)
			{
				buff.applyBuff(self, self, "co_pos_sec_critical_1");
			}
			else if (posSecuredOnTarget == 4)
			{
				buff.applyBuff(self, self, "co_pos_sec_critical_2");
			}
			else if (posSecuredOnTarget == 6)
			{
				buff.applyBuff(self, self, "co_pos_sec_critical_3");
			}
			else if (posSecuredOnTarget == 8)
			{
				buff.applyBuff(self, self, "co_pos_sec_critical_4");
			}
			
			int posSecuredBoO = (int)getSkillStatisticModifier(self, "expertise_co_pos_secured_line_armor");
			
			if (posSecuredBoO == 1000)
			{
				buff.applyBuff(self, self, "co_base_of_operations");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onTargetRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasSkillModModifier(self, effectName))
		{
			removeAttribOrSkillModModifier(self, effectName);
		}
		
		buff.removeBuff(self, "co_pos_sec_action_1");
		buff.removeBuff(self, "co_pos_sec_action_2");
		buff.removeBuff(self, "co_pos_sec_action_3");
		
		buff.removeBuff(self, "co_pos_sec_proc_1");
		buff.removeBuff(self, "co_pos_sec_proc_2");
		messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
		
		buff.removeBuff(self, "co_pos_sec_critical_1");
		buff.removeBuff(self, "co_pos_sec_critical_2");
		buff.removeBuff(self, "co_pos_sec_critical_3");
		buff.removeBuff(self, "co_pos_sec_critical_4");
		
		if (buff.hasBuff(self, "co_base_of_operations"))
		{
			obj_id baseOwner = buff.getBuffOwner(self, "co_base_of_operations");
			
			if ((baseOwner == self) || !isIdValid(baseOwner))
			{
				buff.removeBuff(self, "co_base_of_operations");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int immunityAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		String prefixBuilder = "";
		int wholeValue = (int)value;
		if (subtype.equals("dot_immunity"))
		{
			
			if (wholeValue == IMMUNITY_TO_POISON)
			{
				buff.performBuffDotImmunity(self, buff.DOT_POISON);
				
				if (dot.removeDotsOfType(self, dot.DOT_POISON))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "poisoned", cureDotMsg);
				}
				prefixBuilder = "dot.poison";
			}
			else if (wholeValue == IMMUNITY_TO_DISEASE)
			{
				buff.performBuffDotImmunity(self, buff.DOT_DISEASE);
				
				if (dot.removeDotsOfType(self, dot.DOT_DISEASE))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "diseased", cureDotMsg);
				}
				prefixBuilder = "dot.disease";
			}
			else if (wholeValue == IMMUNITY_TO_FIRE)
			{
				buff.performBuffDotImmunity(self, buff.DOT_FIRE);
				
				if (dot.removeDotsOfType(self, dot.DOT_FIRE))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "on fire", cureDotMsg);
				}
				prefixBuilder = "dot.fire";
			}
			else if (wholeValue == IMMUNITY_TO_BLEEDING)
			{
				buff.performBuffDotImmunity(self, buff.DOT_BLEEDING);
				
				if (dot.removeDotsOfType(self, dot.DOT_BLEEDING))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "bleeding", cureDotMsg);
				}
				prefixBuilder = "dot.bleeding";
			}
			else if (wholeValue == IMMUNITY_TO_ACID)
			{
				buff.performBuffDotImmunity(self, buff.DOT_ACID);
				
				if (dot.removeDotsOfType(self, dot.DOT_ACID))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "acid splattered", cureDotMsg);
				}
				prefixBuilder = "dot.acid";
			}
			else if (wholeValue == IMMUNITY_TO_ENERGY)
			{
				buff.performBuffDotImmunity(self, buff.DOT_ENERGY);
				
				if (dot.removeDotsOfType(self, dot.DOT_ENERGY))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "electrified", cureDotMsg);
				}
				prefixBuilder = "dot.energy";
			}
			else if (wholeValue == IMMUNITY_TO_ALL_DOTS)
			{
				buff.performBuffDotImmunity(self, "all");
				
				if (dot.removeDotsOfType(self, dot.DOT_POISON))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "poisoned", cureDotMsg);
				}
				if (dot.removeDotsOfType(self, dot.DOT_DISEASE))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "diseased", cureDotMsg);
				}
				if (dot.removeDotsOfType(self, dot.DOT_FIRE))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "on fire", cureDotMsg);
				}
				if (dot.removeDotsOfType(self, dot.DOT_BLEEDING))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "bleeding", cureDotMsg);
				}
				if (dot.removeDotsOfType(self, dot.DOT_ACID))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "acid splattered", cureDotMsg);
				}
				if (dot.removeDotsOfType(self, dot.DOT_ENERGY))
				{
					string_id cureDotMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "electrified", cureDotMsg);
				}
				
				prefixBuilder = "dot.all";
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else if (subtype.equals("movement_immunity"))
		{
			
			if (wholeValue == movement.MT_SNARE)
			{
				
				if (removeAllModifiersOfType(self, movement.MT_SNARE))
				{
					string_id cureMovementMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "snared", cureMovementMsg);
				}
				prefixBuilder = "movement.snare";
				
			}
			else if (wholeValue == movement.MT_ROOT)
			{
				
				if (removeAllModifiersOfType(self, movement.MT_ROOT))
				{
					string_id cureMovementMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "rooted", cureMovementMsg);
				}
				prefixBuilder = "movement.root";
				
			}
			else if (wholeValue == movement.MT_ALL)
			{
				removeAllModifiersOfType(self, movement.MT_SNARE);
				removeAllModifiersOfType(self, movement.MT_ROOT);
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else if (subtype.equals("state_immunity"))
		{
			
			if (wholeValue == buff.STATE_STUNNED)
			{
				
				if (buff.removeAllBuffsOfStateType(self, buff.STATE_STUNNED))
				{
					string_id cureStateMsg = new string_id ("spam", "cure_dot");
					messageDetrimentalRemoved(self, "stunned", cureStateMsg);
				}
				prefixBuilder = "state.stun";
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else if (subtype.equals("debuff_purge"))
		{
			int numToPurge = wholeValue;
			
			int[] allBuffs = buff.getAllBuffs(self);
			
			for (int i = 0; i < allBuffs.length; i++)
			{
				testAbortScript();
				String curBuff = buff.getBuffNameFromCrc(allBuffs[i]);
				
				if (numToPurge < 1)
				{
					return SCRIPT_CONTINUE;
				}
				
				if (buff.isDebuff(curBuff) && !buff.isDotIconOnlyBuff(curBuff) && buff.canBeDispelled(curBuff) && !buff.isBuffDot(curBuff))
				{
					
					long stack = buff.getBuffStackCount(self, curBuff);
					if (stack > 1 && numToPurge > 1)
					{
						if (stack > numToPurge)
						{
							buff.decrementBuffStack(self, curBuff, numToPurge);
							return SCRIPT_CONTINUE;
						}
						else
						{
							buff.removeBuff(self, curBuff);
							numToPurge -= stack;
						}
						
					}
					else
					{
						buff.removeBuff(self, curBuff);
						numToPurge--;
					}
				}
			}
			
		}
		else if (subtype.equals("buff_purge"))
		{
			int numToPurge = wholeValue;
			
			int[] allBuffs = buff.getAllBuffs(self);
			
			for (int i = 0; i < allBuffs.length; i++)
			{
				testAbortScript();
				String curBuff = buff.getBuffNameFromCrc(allBuffs[i]);
				
				if (numToPurge < 1)
				{
					return SCRIPT_CONTINUE;
				}
				
				if (!buff.isDebuff(curBuff) && !buff.isDotIconOnlyBuff(curBuff) && buff.canBeDispelled(curBuff))
				{
					buff.removeBuff(self, curBuff);
					numToPurge--;
				}
			}
			
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!prefixBuilder.equals(""))
		{
			utils.setScriptVar(self, "immunity."+ prefixBuilder, wholeValue);
			
		}
		else
		{
			
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotReductionAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("dot_reduction_acid") || subtype.equals("dot_reduction_all") || subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_ACID, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ACID);
		}
		if (subtype.equals("dot_reduction_heat") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_FIRE, (int)value);
			dot.removeDotsOfType(self, dot.DOT_FIRE);
		}
		if (subtype.equals("dot_reduction_cold") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_COLD, (int)value);
			dot.removeDotsOfType(self, dot.DOT_COLD);
		}
		if (subtype.equals("dot_reduction_electrical") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_ELECTRICITY, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ELECTRICITY);
		}
		if (subtype.equals("dot_reduction_kinetic") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_KINETIC, (int)value);
			dot.removeDotsOfType(self, dot.DOT_KINETIC);
		}
		if (subtype.equals("dot_reduction_energy") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_elemental"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_ENERGY, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ENERGY);
		}
		if (subtype.equals("dot_reduction_poison") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_natural"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_POISON, (int)value);
			dot.removeDotsOfType(self, dot.DOT_POISON);
		}
		if (subtype.equals("dot_reduction_disease") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_natural"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_DISEASE, (int)value);
			dot.removeDotsOfType(self, dot.DOT_DISEASE);
		}
		if (subtype.equals("dot_reduction_bleeding") || subtype.equals("dot_reduction_all")|| subtype.equals("dot_reduction_natural"))
		{
			buff.reduceBuffDotStackCount(self, buff.DOT_BLEEDING, (int)value);
			dot.removeDotsOfType(self, dot.DOT_BLEEDING);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotReductionRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotDivisorAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("dot_divisor_acid") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_ACID, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ACID);
		}
		if (subtype.equals("dot_divisor_heat") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_FIRE, (int)value);
			dot.removeDotsOfType(self, dot.DOT_FIRE);
		}
		if (subtype.equals("dot_divisor_cold") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_COLD, (int)value);
			dot.removeDotsOfType(self, dot.DOT_COLD);
		}
		if (subtype.equals("dot_rdivisor_electrical") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_ELECTRICITY, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ELECTRICITY);
		}
		if (subtype.equals("dot_divisor_kinetic") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_KINETIC, (int)value);
			dot.removeDotsOfType(self, dot.DOT_KINETIC);
		}
		if (subtype.equals("dot_divisor_energy") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_elemental"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_ENERGY, (int)value);
			dot.removeDotsOfType(self, dot.DOT_ENERGY);
		}
		if (subtype.equals("dot_divisor_poison") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_natural"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_POISON, (int)value);
			dot.removeDotsOfType(self, dot.DOT_POISON);
		}
		if (subtype.equals("dot_divisor_disease") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_natural"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_DISEASE, (int)value);
			dot.removeDotsOfType(self, dot.DOT_DISEASE);
		}
		if (subtype.equals("dot_divisor_bleeding") || subtype.equals("dot_divisor_all") || subtype.equals("dot_divisor_natural"))
		{
			buff.divideBuffDotStackCount(self, buff.DOT_BLEEDING, (int)value);
			dot.removeDotsOfType(self, dot.DOT_BLEEDING);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotDivisorRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int immunityRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		String prefixBuilder = "";
		int wholeValue = (int)value;
		if (subtype.equals("dot_immunity"))
		{
			if (wholeValue == IMMUNITY_TO_POISON)
			{
				prefixBuilder = "dot.poison";
			}
			else if (wholeValue == IMMUNITY_TO_FIRE)
			{
				prefixBuilder = "dot.fire";
			}
			else if (wholeValue == IMMUNITY_TO_DISEASE)
			{
				prefixBuilder = "dot.disease";
			}
			else if (wholeValue == IMMUNITY_TO_BLEEDING)
			{
				prefixBuilder = "dot.bleeding";
			}
			else if (wholeValue == IMMUNITY_TO_ALL_DOTS)
			{
				prefixBuilder = "dot.all";
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else if (subtype.equals("movement_immunity"))
		{
			if (wholeValue == movement.MT_SNARE)
			{
				prefixBuilder = "movement.snare";
			}
			else if (wholeValue == movement.MT_ROOT)
			{
				prefixBuilder = "movement.root";
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else if (subtype.equals("state_immunity"))
		{
			if (wholeValue == buff.STATE_STUNNED)
			{
				prefixBuilder = "state.stun";
			}
			else
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!prefixBuilder.equals(""))
		{
			utils.removeScriptVarTree(self, "immunity."+ prefixBuilder);
			
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
			stopClientEffectObjByLabel(self, effectName);
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertiseImmunityAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!buff.isInStance(self))
		{
			
			buff.removeBuff(self, "fs_sh_0");
			buff.removeBuff(self, "fs_sh_1");
			buff.removeBuff(self, "fs_sh_2");
			buff.removeBuff(self, "fs_sh_3");
			
			buff.removeBuff(self, "fs_dot_immunity_recourse");
			
			return SCRIPT_OVERRIDE;
		}
		
		int wholeValue = (int)value;
		
		int immunityType = (int)getSkillStatisticModifier(self, subtype + wholeValue);
		
		if (immunityType != 0)
		{
			return immunityAddBuffHandler(self, effectName, subtype, duration, value, "", null);
		}
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int expertiseImmunityRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return immunityRemoveBuffHandler(self, effectName, subtype, duration, value, "", null);
	}
	
	
	public int expertiseChannelActionHealAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "buff_handler.lastForsakeFearPulse", getGameTime());
		utils.setScriptVar(self, "buff_handler.totalForsakeFearPulses", 0);
		utils.removeScriptVar(self, "buff_handler.channelForsakeFearCancelled");
		utils.removeScriptVar(self, "buff_handler.channelForsakeFearSuccessful");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertiseChannelActionHealRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int cancelled = utils.getIntScriptVar(self, "buff_handler.channelForsakeFearCancelled");
		
		int successful = utils.getIntScriptVar(self, "buff_handler.channelForsakeFearSuccessful");
		
		if (cancelled == 0 && successful == 1)
		{
			
			channelForsakeFear(self, effectName, true);
		}
		
		utils.removeScriptVar(self, "buff_handler.channelForsakeFearCancelled");
		utils.removeScriptVar(self, "buff_handler.totalForsakeFearPulses");
		utils.removeScriptVar(self, "buff_handler.lastForsakeFearPulse");
		utils.removeScriptVar(self, "buff_handler.channelForsakeFearSuccessful");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onIncapHealAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "buff_handler."+subtype, value);
		return SCRIPT_CONTINUE;
	}
	
	
	public int onIncapHealRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, "buff_handler."+subtype);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int healEffectAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("healing_action"))
		{
			healing.healDamage(self, ACTION, (int)value);
			return SCRIPT_CONTINUE;
		}
		
		else if (subtype.equals("healing_health"))
		{
			healing.healDamage(caster, self, HEALTH, (int)value);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int healEffectRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int buildabuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "performance.buildabuff.buffComponentKeys") || !utils.hasScriptVar(self, "performance.buildabuff.buffComponentValues"))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		String[] buffComponentKeys = utils.getStringArrayScriptVar(self, "performance.buildabuff.buffComponentKeys");
		int[] buffComponentValues = utils.getIntArrayScriptVar(self, "performance.buildabuff.buffComponentValues");
		obj_id bufferId = utils.getObjIdScriptVar(self, "performance.buildabuff.bufferId");
		
		{
			CustomerServiceLog("SuspectedCheaterChannel: ", "Entered BuildABuffAdd Handler, dumping info...");
			CustomerServiceLog("SuspectedCheaterChannel: ", "BufferId: "+ bufferId + " IsValid: "+ isIdValid(bufferId) + " Exists: "+ exists(bufferId) + " isInternalDecay: "+ utils.hasScriptVar(self, "buffDecay"));
			for (int i = 0; i < buffComponentKeys.length; ++i)
			{
				testAbortScript();
				CustomerServiceLog("SuspectedCheaterChannel: ", "BuffComponentKey["+ i + "] = "+ buffComponentKeys[i]);
			}
			for (int i = 0; i < buffComponentValues.length; ++i)
			{
				testAbortScript();
				CustomerServiceLog("SuspectedCheaterChannel: ", "BuffComponentValues["+ i + "] = "+ buffComponentValues[i]);
			}
		}
		
		boolean internalDecay = false;
		if (utils.hasScriptVar(self, "buffDecay"))
		{
			internalDecay = true;
			
			utils.removeScriptVar(self, "buffDecay");
		}
		
		CustomerServiceLog("SuspectedCheaterChannel: ", "Internal Decay = "+ internalDecay);
		
		int improv = 0;
		if (isIdValid(bufferId) && exists(bufferId))
		{
			improv = (int)getSkillStatisticModifier(bufferId, "expertise_en_improv");
		}
		
		if (internalDecay == true)
		{
			if (utils.hasScriptVar(self, "decayImprov"))
			{
				improv = utils.getIntScriptVar(self, "decayImprov");
			}
			
		}
		else
		{
			utils.setScriptVar(self, "decayImprov", improv);
			
		}
		
		int improvDance = 0;
		if (improv == 1 && isIdValid(bufferId) && exists(bufferId))
		{
			if (!camping.isInEntertainmentCamp(bufferId))
			{
				location loc = getLocation(bufferId);
				if (isIdValid(loc.cell))
				{
					obj_id building = getTopMostContainer(loc.cell);
					if (!hasObjVar(building, "healing.canhealshock") && !hasObjVar(loc.cell, "healing.canhealshock"))
					{
						region[] regs = getRegionsWithGeographicalAtPoint(getLocation(bufferId), regions.GEO_THEATER);
						if (regs == null || regs.length == 0)
						{
							improvDance = 1;
						}
					}
				}
			}
		}
		
		if (internalDecay == true)
		{
			if (utils.hasScriptVar(self, "decayImprovDance"))
			{
				improvDance = utils.getIntScriptVar(self, "decayImprovDance");
				CustomerServiceLog("SuspectedCheaterChannel: ", "Using stored ImprovDance: "+ improvDance);
				
			}
			
		}
		else
		{
			utils.setScriptVar(self, "decayImprovDance", improvDance);
			CustomerServiceLog("SuspectedCheaterChannel: ", "Storing improvDance: "+ improvDance);
			
		}
		
		int actualPointsToSpend = 8;
		if (isIdValid(bufferId) && exists(bufferId))
		{
			actualPointsToSpend += getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_inspire_base_point_increase");
		}
		
		int attemptingToSpendPoints = 0;
		for (int i = 0; i < buffComponentKeys.length; i++)
		{
			testAbortScript();
			
			if (internalDecay == true)
			{
				break;
			}
			
			dictionary buffData = dataTableGetRow(DATATABLE_BUFF_BUILDER, buffComponentKeys[i]);
			int cost = buffData.getInt("COST") * buffComponentValues[i];
			attemptingToSpendPoints += cost;
			
			if (attemptingToSpendPoints > actualPointsToSpend && internalDecay == false)
			{
				sendSystemMessage(bufferId, "ERROR: Invalid data. Discrepancy logs generated.", null);
				CustomerServiceLog("SuspectedCheaterChannel: ", "Player ("+getName(bufferId)+" : "+ bufferId + ") has tried to spend more points on inspiration buffs then they have ("+attemptingToSpendPoints+")! The buff failed.");
				
				return SCRIPT_CONTINUE;
			}
		}
		
		if (buffComponentKeys.length == buffComponentValues.length && buffComponentKeys.length > 0 && buffComponentValues.length > 0)
		{
			for (int i = 0; i < buffComponentKeys.length; i++)
			{
				testAbortScript();
				dictionary buffData = dataTableGetRow(DATATABLE_BUFF_BUILDER, buffComponentKeys[i]);
				String effect = buffData.getString("AFFECTS");
				String category = buffData.getString("CATEGORY");
				
				int effectAmount = buffData.getInt("AFFECT_AMOUNT");
				int maxTimes = buffData.getInt("MAX_TIMES_APPLIED");
				
				if (buffComponentValues[i] > maxTimes)
				{
					if (internalDecay == false)
					{
						CustomerServiceLog("SuspectedCheaterChannel: ", "Player ("+getName(bufferId)+" : "+ bufferId + ") has passed an out of bounds build-a-buff value ("+ buffComponentValues[i] + ") which has a cap of ("+ maxTimes + "). HAX!");
						sendSystemMessage(bufferId, "ERROR: Capping invalid buff values. Discrepancy logs generated.", null);
					}
					buffComponentValues[i] = maxTimes;
				}
				
				float buffValue = effectAmount * buffComponentValues[i];
				
				if (improvDance == 1)
				{
					buffValue = (buffValue * 0.2f);
				}
				
				if (category.equals("attributes") || category.equals("resistances") || category.equals("combat"))
				{
					
					if (category.equals("attributes"))
					{
						float attribModifier = 0.0f;
						
						if (isIdValid(bufferId) && exists(bufferId))
						{
							attribModifier = (float)(getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_inspire_attrib_increase")/100.0f);
						}
						
						if (internalDecay == true)
						{
							if (utils.hasScriptVar(self, "decayAttribMod"))
							{
								attribModifier = utils.getFloatScriptVar(self, "decayAttribMod");
								CustomerServiceLog("SuspectedCheaterChannel: ", "Using stored attribMod: "+ attribModifier);
								
							}
							
						}
						else
						{
							CustomerServiceLog("SuspectedCheaterChannel: ", "Storing attribMod: "+ attribModifier);
							
							utils.setScriptVar(self, "decayAttribMod", attribModifier);
							
						}
						
						buffValue *= 1.0f + attribModifier;
					}
					
					else if (category.equals("resistances"))
					{
						float resistModifier = 0.0f;
						
						if (isIdValid(bufferId) && exists(bufferId))
						{
							resistModifier = (float)(getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_inspire_resist_increase")/100.0f);
						}
						
						if (internalDecay == true)
						{
							if (utils.hasScriptVar(self, "decayResistMod"))
							{
								resistModifier = utils.getFloatScriptVar(self, "decayResistMod");
							}
							
						}
						else
						{
							utils.setScriptVar(self, "decayResistMod", resistModifier);
							
						}
						
						buffValue *= 1.0f + resistModifier;
					}
					
					else if (category.equals("combat"))
					{
						float combatModifier = 0.0f;
						
						if (isIdValid(bufferId) && exists(bufferId))
						{
							combatModifier = getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_combat_buff_increase");
						}
						
						if (internalDecay == true)
						{
							if (utils.hasScriptVar(self, "decayCombatMod"))
							{
								combatModifier = utils.getFloatScriptVar(self, "decayCombatMod");
							}
							
						}
						else
						{
							utils.setScriptVar(self, "decayCombatMod", combatModifier);
							
						}
						
						buffValue += combatModifier;
					}
					{
						
						CustomerServiceLog("SuspectedCheaterChannel: ", "Adding Stat Modifier: "+ "buildabuff_"+ effect + " Value = "+ buffValue + " Duration = "+ duration);
						
					}
					
					addSkillModModifier(self, "buildabuff_"+ effect, effect, (int)buffValue, duration, false, true);
					
					if ((effect.startsWith("constitution"))||(effect.startsWith("stamina")))
					{
						messageTo(self, "recalcPools", null, .25f, false);
					}
					else if (effect.startsWith("expertise_innate_protection_"))
					{
						messageTo(self, "recalcArmor", null, .25f, false);
					}
				}
				else if (category.equals("trade"))
				{
					
					float tradeModifier = 0.0f;
					
					if (isIdValid(bufferId) && exists(bufferId))
					{
						tradeModifier = (float)(getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_inspire_trader_increase")/100.0f);
					}
					
					if (internalDecay == true)
					{
						if (utils.hasScriptVar(self, "decayTradeMod"))
						{
							tradeModifier = utils.getFloatScriptVar(self, "decayTradeMod");
						}
						
					}
					else
					{
						utils.setScriptVar(self, "decayTradeMod", tradeModifier);
						
					}
					
					buffValue *= 1.0f + tradeModifier;
					
					String[] xpArray =
					{
						"crafting", "combat_general", "entertainer", "space_combat_general"
					};
					
					utils.setScriptVar(self, "buff.xpBonus.types", xpArray);
					utils.setScriptVar(self, "buff.xpBonus.value", buffValue/100);
					{
						
						CustomerServiceLog("SuspectedCheaterChannel: ", "Adding Stat Modifier: "+ "buildabuff_"+ effect + " Value = "+ buffValue + " Duration = "+ duration);
						
					}
					
					addSkillModModifier(self, "buildabuff_"+ effect, effect, (int)buffValue, duration, false, true);
				}
				else if (category.equals("misc"))
				{
					if (effect.equals("movement_speed"))
					{
						if (value == 0)
						{
							value = 1;
						}
						
						if (movement.hasMovementModifier(self, "buildabuff_movement_speed"))
						{
							movement.removeMovementModifier(self, "buildabuff_movement_speed", false);
						}
						
						movement.applyMovementModifier(self, "buildabuff_movement_speed", buffValue);
					}
					else if (effect.equals("reactive_second_chance"))
					{
						int playerLevel = getLevel(self);
						float reactiveModifier = 0.0f;
						
						if (isIdValid(bufferId) && exists(bufferId))
						{
							reactiveModifier = getEnhancedSkillStatisticModifierUncapped(bufferId, "expertise_en_inspire_proc_chance_increase");
						}
						
						if (internalDecay == true)
						{
							if (utils.hasScriptVar(self, "decayReactiveMod"))
							{
								reactiveModifier = utils.getFloatScriptVar(self, "decayReactiveMod");
							}
							
						}
						else
						{
							utils.setScriptVar(self, "decayReactiveMod", reactiveModifier);
							
						}
						
						buffValue += reactiveModifier;
						
						if (playerLevel > 69)
						{
							addSkillModModifier(self, "expertise_buildabuff_heal_3_reac", "expertise_buildabuff_heal_3_reac", (int)buffValue, duration, false, true);
						}
						else if (playerLevel > 39 && playerLevel < 70)
						{
							addSkillModModifier(self, "expertise_buildabuff_heal_2_reac", "expertise_buildabuff_heal_2_reac", (int)buffValue, duration, false, true);
						}
						else
						{
							addSkillModModifier(self, "expertise_buildabuff_heal_1_reac", "expertise_buildabuff_heal_1_reac", (int)buffValue, duration, false, true);
						}
						
						messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
					}
					else if (effect.equals("flush_with_success"))
					{
						
						String[] xpArray =
						{
							"crafting", "combat_general", "entertainer", "space_combat_general", "chronicles"
						};
						
						utils.setScriptVar(self, "buff.xpBonus.types", xpArray);
						utils.setScriptVar(self, "buff.xpBonus.value", buffValue/100.0f);
					}
					else
					{
						{
							
							CustomerServiceLog("SuspectedCheaterChannel: ", "Adding Stat Modifier: "+ "buildabuff_"+ effect + " Value = "+ buffValue + " Duration = "+ duration);
							
						}
						
						addSkillModModifier(self, "buildabuff_"+ effect, effect, (int)buffValue, duration, false, true);
					}
				}
			}
			
			if (!buff.hasBuff(self, "col_ent_invis_buff_tracker"))
			{
				collection.entertainerBuffCollection(self, bufferId, duration);
			}
		}
		else
		{
			sendSystemMessageTestingOnly(self, "Malformed buildabuff data passed to buildabuffAddBuffHandler.");
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		if (hasScript(self, "systems.buff_builder.buff_builder_cancel"))
		{
			detachScript(self, "systems.buff_builder.buff_builder_cancel");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int buildabuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String[] baseModList = dataTableGetStringColumn(DATATABLE_BUFF_BUILDER, "AFFECTS");
		
		for (int i = 0; i < baseModList.length; i++)
		{
			testAbortScript();
			if (hasSkillModModifier(self, "buildabuff_"+ baseModList[i]))
			{
				removeAttribOrSkillModModifier(self, "buildabuff_"+ baseModList[i]);
			}
		}
		
		if (hasSkillModModifier(self, "expertise_buildabuff_heal_1_reac"))
		{
			removeAttribOrSkillModModifier(self, "expertise_buildabuff_heal_1_reac");
		}
		
		if (hasSkillModModifier(self, "expertise_buildabuff_heal_2_reac"))
		{
			removeAttribOrSkillModModifier(self, "expertise_buildabuff_heal_2_reac");
		}
		
		if (hasSkillModModifier(self, "expertise_buildabuff_heal_3_reac"))
		{
			removeAttribOrSkillModModifier(self, "expertise_buildabuff_heal_3_reac");
		}
		
		messageTo(self, "recalcPools", null, 1, false);
		messageTo(self, "recalcArmor", null, .25f, false);
		messageTo(self, "cacheExpertiseProcReacList", null, 2, false);
		
		utils.removeScriptVar(self, "performance.buildabuff.modNames");
		utils.removeScriptVar(self, "performance.buildabuff.modValues");
		utils.removeScriptVarTree(self, "buff.xpBonus");
		
		if (movement.hasMovementModifier(self, "buildabuff_movement_speed"))
		{
			movement.removeMovementModifier(self, "buildabuff_movement_speed");
		}
		
		if (hasScript(self, "systems.buff_builder.buff_builder_cancel"))
		{
			detachScript(self, "systems.buff_builder.buff_builder_cancel");
		}
		
		trial.bumpSession(self, "displayDefensiveMods");
		messageTo(self, "setDisplayOnlyDefensiveMods", trial.getSessionDict(self, "displayDefensiveMods") , 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int meDoomAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		showFlyText(self, new string_id("set_bonus", "doom_fly"), 2, colors.BLACK);
		
		obj_id doomOwner = utils.getObjIdScriptVar(self, "me_doom.doom_owner");
		int doomStage = utils.getIntScriptVar(self, "me_doom.doom_stage");
		
		if (!isIdValid(doomOwner))
		{
			utils.removeScriptVar(self, "me_doom.doom_owner");
			utils.removeScriptVar(self, "me_doom.doom_stage");
			return SCRIPT_CONTINUE;
		}
		
		if (doomStage > 3 || doomStage < 1)
		{
			utils.removeScriptVar(self, "me_doom.doom_owner");
			utils.removeScriptVar(self, "me_doom.doom_stage");
			return SCRIPT_CONTINUE;
		}
		
		if (doomStage == 2)
		{
			dot.applyDotEffect(self, doomOwner, "bleeding", "me_doom_bleed", HEALTH, 100, 800, 5, true, null);
		}
		
		if (doomStage == 3)
		{
			dot.applyDotEffect(self, doomOwner, "fire", "me_doom_bleed", HEALTH, 100, 800, 5, true, null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int meDoomRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id doomOwner = utils.getObjIdScriptVar(self, "me_doom.doom_owner");
		int doomStage = utils.getIntScriptVar(self, "me_doom.doom_stage");
		
		if (!isIdValid(doomOwner))
		{
			utils.removeScriptVar(self, "me_doom.doom_owner");
			utils.removeScriptVar(self, "me_doom.doom_stage");
			return SCRIPT_CONTINUE;
		}
		
		if (doomStage > 3 || doomStage < 1)
		{
			utils.removeScriptVar(self, "me_doom.doom_owner");
			utils.removeScriptVar(self, "me_doom.doom_stage");
			return SCRIPT_CONTINUE;
		}
		
		if (doomStage == 1)
		{
			utils.setScriptVar(self, "me_doom.doom_stage", 2);
			buff.applyBuff(self, self, "me_doom", 10.0f);
		}
		
		if (doomStage == 2)
		{
			utils.setScriptVar(self, "me_doom.doom_stage", 3);
			buff.applyBuff(self, self, "me_doom", 10.0f);
		}
		
		if (doomStage == 3)
		{
			utils.removeScriptVar(self, "me_doom.doom_owner");
			utils.removeScriptVar(self, "me_doom.doom_stage");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cacheExpertiseProcReacList(obj_id self, dictionary params) throws InterruptedException
	{
		expertise.cacheExpertiseProcReacList(self);
		proc.buildCurrentReacList(self);
		proc.buildCurrentProcList(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void healAttribPercent(obj_id who, int stat, int percent) throws InterruptedException
	{
		int currentStatValue = getAttrib(who, stat);
		int maxStatValue = getMaxAttrib(who, stat);
		
		if (currentStatValue < maxStatValue)
		{
			int newStatValue = currentStatValue + (int)(maxStatValue * (float)(percent / 100));
			
			if (newStatValue > maxStatValue)
			{
				setAttrib(who, stat, maxStatValue);
			}
			else
			{
				setAttrib(who, stat, newStatValue);
			}
		}
	}
	
	
	public int channelForsakeFear(obj_id player, String buffName, boolean finishChannel) throws InterruptedException
	{
		
		int lastForsakeFearPulse = utils.getIntScriptVar(player, "buff_handler.lastForsakeFearPulse");
		int currentForsakeFearPulses = utils.getIntScriptVar(player, "buff_handler.totalForsakeFearPulses");
		int gameTime = getGameTime();
		
		int newTotal = gameTime - lastForsakeFearPulse + currentForsakeFearPulses;
		
		if (finishChannel || (newTotal > 10))
		{
			newTotal = 10;
		}
		
		if (newTotal > currentForsakeFearPulses)
		{
			
			if (group.isGrouped(player))
			{
				Vector party = group.getPCMembersInRange(player, 32f);
				Vector channelToMembers = new Vector();
				if (party != null)
				{
					for (int i = 0; i < party.size(); i++)
					{
						testAbortScript();
						obj_id who = (obj_id)party.elementAt(i);
						
						if ((obj_id)buff.getBuffOwner(who, buffName) == player)
						{
							healAttribPercent(who, ACTION, 6 * (newTotal - currentForsakeFearPulses));
							playClientEffectObj(who, "appearance/pt_jedi_forsake_fear.prt", who, "");
						}
					}
				}
			}
			
			healAttribPercent(player, ACTION, 6 * (newTotal - currentForsakeFearPulses));
			playClientEffectObj(player, "appearance/pt_jedi_forsake_fear.prt", player, "");
		}
		
		if (!finishChannel)
		{
			dictionary parms = new dictionary();
			
			parms.put("player", player);
			parms.put("buffName", buffName);
			
			utils.setScriptVar(player, "buff_handler.lastForsakeFearPulse", gameTime);
			utils.setScriptVar(player, "buff_handler.totalForsakeFearPulses", newTotal);
			
			messageTo(player, "checkChannelForsakeFear", parms, 1, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkChannelForsakeFear(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId("player");
		String buffName = params.getString("buffName");
		
		if ((utils.hasScriptVar(player, "buff_handler.channelForsakeFearCancelled") || utils.hasScriptVar(player, "buff_handler.channelForsakeFearSuccessful") || !hasObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR)))
		{
			return SCRIPT_CONTINUE;
		}
		
		channelForsakeFear(player, buffName, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int channelForsakeFearCountdownHandler(obj_id self, dictionary params) throws InterruptedException
	{
		int pid = params.getInt("id");
		obj_id player = params.getObjId("player");
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			
			utils.setScriptVar(player, "buff_handler.channelForsakeFearCancelled", 1);
			
			detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
			
			return SCRIPT_CONTINUE;
		}
		
		else if (bp == sui.BP_REVERT)
		{
			
			int event = params.getInt("event");
			
			if (event == sui.CD_EVENT_LOCOMOTION)
			{
				utils.setScriptVar(player, "buff_handler.channelForsakeFearCancelled", 1);
			}
			else if (event == sui.CD_EVENT_INCAPACITATE)
			{
				utils.setScriptVar(player, "buff_handler.channelForsakeFearCancelled", 1);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		int test_pid = getIntObjVar(player, sui.COUNTDOWNTIMER_SUI_VAR);
		
		if (pid != test_pid)
		{
			return SCRIPT_CONTINUE;
		}
		
		forceCloseSUIPage(pid);
		
		detachScript(player, sui.COUNTDOWNTIMER_PLAYER_SCRIPT);
		
		utils.setScriptVar(player, "buff_handler.channelForsakeFearSuccessful", 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionDrainAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		drainAttributes(self, (int)value, 0);
		
		if (!buff.hasBuff(self, "action_drain_immunity"))
		{
			buff.applyBuff(self, self, "action_drain_immunity");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionDrainRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionBurnAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "buff.action_burn.value", value);
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionBurnRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, "buff.action_burn.value");
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionRegenAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int actionMax = getMaxAction(self);
		dictionary data = new dictionary();
		
		data.put("actionMax", actionMax);
		data.put("buffName", "sp_action_regen");
		data.put("ticks", duration);
		data.put("currentTick", 0);
		
		messageTo(self, "actionRegenBuff", data, 1f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionRegenRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int actionRegenBuff(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		String buffName = params.getString("buffName");
		
		if (!buff.hasBuff(self, buffName))
		{
			return SCRIPT_CONTINUE;
		}
		
		int maxAction = params.getInt("actionMax");
		
		float ticks = params.getFloat("ticks");
		
		float healAmount = maxAction / ticks;
		
		healing.healDamage(self, ACTION, (int)healAmount);
		
		int currentTick = params.getInt("currentTick");
		
		if (currentTick < ticks)
		{
			params.put("currentTick", ++currentTick);
			messageTo(self, "actionRegenBuff", params, 1f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bodyguardDefenderAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("shield_master_pet"))
		{
			obj_id master = getMaster(self);
			if (!isIdValid(master))
			{
				buff.removeBuff(self, "bm_shield_master_pet");
				return SCRIPT_CONTINUE;
			}
			
			buff.applyBuff(master, self, "bm_shield_master_player");
			return SCRIPT_CONTINUE;
		}
		
		if (subtype.equals("protect_master"))
		{
			obj_id master = getMaster(self);
			
			if (isIdValid(master) && exists(master))
			{
				utils.setScriptVar(master, combat.DAMAGE_REDIRECT, self);
				buff.applyBuff(master, self, "bodyguard");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bodyguardDefenderRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("shield_master_pet"))
		{
			obj_id master = getMaster(self);
			if (isIdValid(master) && exists(master))
			{
				buff.removeBuff(master, "bm_shield_master_player");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (subtype.equals("protect_master"))
		{
			obj_id master = getMaster(self);
			
			if (!isIdValid(master))
			{
				master = trial.getParent(self);
			}
			
			if (isIdValid(master) && exists(master))
			{
				buff.removeBuff(master, "bodyguard");
				utils.removeScriptVar(master, combat.DAMAGE_REDIRECT);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cooldownModifyAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subtype.equals("cooldown_execute_all"))
		{
			String[] commandList = getCommandListingForPlayer(self);
			
			if (commandList == null || commandList.length == 0)
			{
				debugSpeakMsg(self, "Command list was null or empty");
				return SCRIPT_CONTINUE;
			}
			
			for (int i=0; i<commandList.length; i++)
			{
				testAbortScript();
				combat_data cd = combat_engine.getCombatData(commandList[i]);
				
				if (cd == null)
				{
					continue;
				}
				
				String cooldownGroup = cd.cooldownGroup;
				int groupCrc = getStringCrc(cooldownGroup);
				
				float coolDownLeft = getCooldownTimeLeft(self, groupCrc);
				
				if (coolDownLeft < value)
				{
					sendCooldownGroupTimingOnly(self, groupCrc, value);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cooldownModifyRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int vulnerabilityAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String type = "heat";
		boolean exclusive = false;
		
		if (subType.indexOf("exclusive") > -1)
		{
			exclusive = true;
		}
		
		if (subType.endsWith("acid"))
		{
			type = "acid";
		}
		
		if (subType.endsWith("cold"))
		{
			type = "cold";
		}
		
		if (subType.endsWith("electricity"))
		{
			type = "electrical";
		}
		
		utils.setScriptVar(self, "elemental_vulnerability.type_"+type, type);
		utils.setScriptVar(self, "elemental_vulnerability.type_"+type+".value", value);
		utils.setScriptVar(self, "elemental_vulnerability.type_"+type+".exclusive", exclusive);
		
		return SCRIPT_CONTINUE;
	}
	
	public void clog(String message) throws InterruptedException
	{
		debugSpeakMsg(getSelf(), message);
	}
	
	
	public int vulnerabilityRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String type = "heat";
		
		if (subType.endsWith("acid"))
		{
			type = "acid";
		}
		
		if (subType.endsWith("cold"))
		{
			type = "cold";
		}
		
		if (subType.endsWith("electricity"))
		{
			type = "electrical";
		}
		
		utils.removeScriptVar(self, "elemental_vulnerability.type_"+type);
		utils.removeScriptVar(self, "elemental_vulnerability.type_"+type+".value");
		utils.removeScriptVar(self, "elemental_vulnerability.type_"+type+".exclusive");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeIncapWeakenAddBuffHandler(obj_id self, String effectName, String subType, float duartion, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.removeBuff(self, "incapWeaken");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeIncapWeakenRemoveBuffHandler(obj_id self, String effectName, String subType, float duartion, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkvaContagionAddBuffHandler(obj_id self, String effectName, String subType, float duartion, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (buff.hasBuff(self, "axkva_contagion_recourse"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		playClientEffectObj(self, "appearance/pt_contagion_debuff.prt", self, "", null, "contagion");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkvaContagionRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.applyBuff(self, self, "axkva_contagion_recourse");
		stopClientEffectObjByLabel(self, self, "contagion");
		
		obj_id axkva = getFirstObjectWithScript(getLocation(self), 250.0f, "theme_park.heroic.axkva_min.axkva");
		
		if (!isIdValid(axkva) || ai_lib.isDead(axkva))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary dict = new dictionary();
		dict.put("player", self);
		messageTo(axkva, "contagion_bomb", dict, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bodyguardMasterAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int bodyguardMasterRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int onNextAttackAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "next_hit_damage_bonus", value);
		obj_id master = utils.getObjIdScriptVar(self, "temp.master");
		if (isIdValid(master) && exists(master))
		{
			buff.applyBuff(master, self, "bm_preperation");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onNextAttackRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, "bm_preperation");
		return SCRIPT_CONTINUE;
	}
	
	
	public int damageDealtModAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, "damageDealtMod.value", value);
		utils.setScriptVar(self, "damageDealtMod.scale", getScale(self));
		float enragedScale = (float)(getScale(self) * 1.3);
		setScale(self, enragedScale);
		return SCRIPT_CONTINUE;
	}
	
	
	public int damageDealtModRemoveBuffHandler(obj_id self, String effectName, String subType, float druation, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float scale = utils.getFloatScriptVar(self, "damageDealtMod.scale");
		if (scale > 0)
		{
			setScale(self, scale);
		}
		
		utils.removeScriptVarTree(self, "damageDealtMod");
		return SCRIPT_CONTINUE;
	}
	
	
	public int weaponSpeedModAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id weapon = getCurrentWeapon(self);
		float currentAttackSpeed = getWeaponAttackSpeed(weapon);
		utils.setScriptVar(self, "recordedAttackSpeed", ""+weapon+"-"+currentAttackSpeed);
		float newAttackSpeed = currentAttackSpeed - (currentAttackSpeed * value);
		
		if (isIdValid(weapon))
		{
			setWeaponAttackSpeed(weapon, newAttackSpeed);
			weapons.setWeaponData(weapon);
			utils.setScriptVar(weapon, "isCreatureWeapon", 1);
		}
		else
		{
			utils.removeScriptVar(self, "recordedAttackSpeed");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int weaponSpeedModRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "recordedAttackSpeed"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String weaponRecord = utils.getStringScriptVar(self, "recordedAttackSpeed");
		String[] parse = split(weaponRecord, '-');
		if (parse.length < 2)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id weapon = utils.stringToObjId(parse[0]);
		float weaponSpeed = utils.stringToFloat(parse[1]);
		
		if (isIdValid(weapon))
		{
			setWeaponAttackSpeed(weapon, weaponSpeed);
			weapons.setWeaponData(weapon);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandGrantAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subType == null || subType.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasCommand(self, subType))
		{
			grantCommand(self, subType);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int commandGrantRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (subType == null || subType.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasCommand(self, subType))
		{
			revokeCommand(self, subType);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGroupMembersChanged(obj_id self, obj_id gid, obj_id[] groupMembers, obj_id[] addedMembers, obj_id[] removedMembers) throws InterruptedException
	{
		int[] buffList = buff.getOwnedGroupBuffs(self);
		
		if (buffList == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (buffList.length < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary data = new dictionary();
		data.put("owner", self);
		data.put("buffList", buffList);
		data.put("durList", buff.getGroupBuffDuration(self, buffList));
		data.put("strList", buff.getGroupBuffStrength(self, buffList));
		
		if (addedMembers != null && addedMembers.length > 0)
		{
			data.put("isAdding", true);
			
			for (int i = 0; i < addedMembers.length; i++)
			{
				testAbortScript();
				if (addedMembers[i] == self)
				{
					
					for (int j = 0; j < groupMembers.length; j++)
					{
						testAbortScript();
						if (groupMembers[j] == self)
						{
							continue;
						}
						
						messageTo(groupMembers[j], "setGroupBuffs", data, 0f, false);
						
						addTriggerVolumeEventSource("group_buff_breach", groupMembers[j]);
					}
					continue;
				}
				
				messageTo(addedMembers[i], "setGroupBuffs", data, 0f, false);
				
				addTriggerVolumeEventSource("group_buff_breach", addedMembers[i]);
			}
		}
		
		if (removedMembers != null && removedMembers.length > 0)
		{
			data.put("isAdding", false);
			
			for (int i = 0; i < removedMembers.length; i++)
			{
				testAbortScript();
				if (removedMembers[i] == self)
				{
					
					for (int j = 0; j < groupMembers.length; j++)
					{
						testAbortScript();
						if (groupMembers[j] == self)
						{
							continue;
						}
						
						messageTo(groupMembers[j], "setGroupBuffs", data, 0f, false);
						
						removeTriggerVolumeEventSource("group_buff_breach", groupMembers[j]);
					}
					continue;
				}
				
				messageTo(removedMembers[i], "setGroupBuffs", data, 0f, false);
				
				removeTriggerVolumeEventSource("group_buff_breach", removedMembers[i]);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGroupDisbanded(obj_id self, obj_id group) throws InterruptedException
	{
		int[] buffList = buff.getGroupBuffEffects(self);
		if (buffList != null && buffList.length > 0)
		{
			buff.removeGroupBuffEffect(self, buffList);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int OnRemovedFromGroup(obj_id self, obj_id group) throws InterruptedException
	{
		int[] buffList = buff.getGroupBuffEffects(self);
		if (buffList != null && buffList.length > 0)
		{
			buff.removeGroupBuffEffect(self, buffList);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setGroupBuffs(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id owner = params.getObjId("owner");
		int[] buffList = params.getIntArray("buffList");
		float[] durList = params.getFloatArray("durList");
		float[] strList = params.getFloatArray("strList");
		boolean isAdding = params.getBoolean("isAdding");
		
		if (isAdding)
		{
			
			if (owner != self)
			{
				
				float dist = getDistance(self, owner);
				if (dist > buff.GROUP_BUFF_DISTANCE)
				{
					return SCRIPT_CONTINUE;
				}
				
				if (!pvpCanHelp(owner, self))
				{
					
					prose_package pp = new prose_package();
					pp.stringId = new string_id("spam", "group_buff_fail_pvp");
					pp.target.set(self);
					sendSystemMessageProse(owner, pp);
					
					return SCRIPT_CONTINUE;
				}
				
				if (buffList != null && buffList.length > 0)
				{
					buff.addGroupBuffEffect(self, owner, buffList, strList, durList);
				}
			}
		}
		else
		{
			
			if (owner != self)
			{
				if (buffList != null && buffList.length > 0)
				{
					buff.removeGroupBuffEffect(self, buffList);
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int groupAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		int[] buffList = new int[1];
		buffList[0] = getStringCrc(effectName.toLowerCase());
		
		if (isMob(self) && !isPlayer(self) && isIdValid(caster) && self == caster)
		{
			obj_id[] buffAi = getCreaturesInRange(getLocation(self), 45.0f);
			
			if (buffAi != null && buffAi.length > 0)
			{
				for (int i=0; i<buffAi.length; i++)
				{
					testAbortScript();
					if (!isMob(buffAi[i]) && !isPlayer(buffAi[i]) || isDead(buffAi[i]))
					{
						continue;
					}
					
					if (factions.shareSocialGroup(self, buffAi[i]) || factions.isInFriendlyFaction(self, buffAi[i]))
					{
						buff.applyBuff(buffAi[i], self, effectName, duration, value);
					}
				}
			}
			return SCRIPT_CONTINUE;
		}
		
		String var = "groupBuff."+buffList[0];
		if (!utils.hasScriptVar(self, var))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = utils.getObjIdScriptVar(self, var);
		
		if (beast_lib.isBeastMaster(self))
		{
			obj_id beast = beast_lib.getBeastOnPlayer(self);
			
			if (isIdValid(beast) && !isIdNull(beast) && !isDead(beast))
			{
				buff.applyBuff(beast, owner, effectName, duration, value);
			}
		}
		
		if (owner != self)
		{
			return SCRIPT_CONTINUE;
		}
		
		String group2 = buff.getStringGroupTwo(effectName);
		
		if (group2 != null && !group2.equals("") && group2.indexOf( "aura" ) > -1)
		{
			group2 = group2.substring(0, (group2.lastIndexOf("_")));
			
			int[] group2Buffs = buff.getGroup2BuffsOnTarget(self, group2);
			
			int auraCount = 0;
			
			for (int i = 0; i < group2Buffs.length; ++i)
			{
				testAbortScript();
				float tempDuration = buff.getBuffTimeRemaining(self, group2Buffs[i]);
				
				if (tempDuration == -1)
				{
					++auraCount;
				}
			}
			
			int aurasAbleToMaintain = getEnhancedSkillStatisticModifierUncapped(self, "expertise_aura_maintain");
			if ((aurasAbleToMaintain - auraCount) >= 1)
			{
				buff.applyBuff(self, owner, effectName, -1);
			}
		}
		
		boolean newTriggerVolume = !hasTriggerVolume(self, "group_buff_breach");
		
		if (newTriggerVolume)
		{
			createTriggerVolume("group_buff_breach", buff.GROUP_BUFF_DISTANCE, false);
		}
		
		obj_id gid = getGroupObject(self);
		
		if (!isIdValid(gid))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary data = new dictionary();
		data.put("owner", self);
		data.put("buffList", buffList);
		data.put("durList", buff.getGroupBuffDuration(self, buffList));
		data.put("strList", buff.getGroupBuffStrength(self, buffList));
		data.put("isAdding", true);
		
		obj_id[] groupMembers = getGroupMemberIds(gid);
		for (int i = 0; i < groupMembers.length; i++)
		{
			testAbortScript();
			if (groupMembers[i] == self)
			{
				continue;
			}
			
			if (newTriggerVolume)
			{
				addTriggerVolumeEventSource("group_buff_breach", groupMembers[i]);
			}
			
			messageTo(groupMembers[i], "setGroupBuffs", data, 0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int groupRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int[] ownedBuffs = buff.getOwnedGroupBuffs(self);
		boolean lastGroupBuff = (ownedBuffs == null || ownedBuffs.length == 0);
		
		obj_id gid = getGroupObject(self);
		
		if (!isIdValid(gid))
		{
			removeTriggerVolume("group_buff_breach");
			return SCRIPT_CONTINUE;
		}
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		
		int[] buffList = new int[1];
		buffList[0] = getStringCrc(effectName.toLowerCase());
		
		String var = "groupBuff."+buffList[0];
		if (!utils.hasScriptVar(self, var))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = utils.getObjIdScriptVar(self, var);
		utils.removeScriptVar(self, var);
		if (owner != self)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary data = new dictionary();
		data.put("owner", self);
		data.put("buffList", buffList);
		data.put("isAdding", false);
		
		obj_id[] groupMembers = getGroupMemberIds(gid);
		for (int i = 0; i < groupMembers.length; i++)
		{
			testAbortScript();
			if (groupMembers[i] == self)
			{
				continue;
			}
			
			if (lastGroupBuff)
			{
				removeTriggerVolumeEventSource("group_buff_breach", groupMembers[i]);
			}
			
			messageTo(groupMembers[i], "setGroupBuffs", data, 0f, false);
		}
		
		if (lastGroupBuff)
		{
			removeTriggerVolume("group_buff_breach");
		}
		
		stopClientEffectObjByLabel(self, effectName);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		
		if (!volumeName.equals("group_buff_breach"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!group.inSameGroup(self, breacher))
		{
			removeTriggerVolumeEventSource("group_buff_breach", breacher);
			return SCRIPT_CONTINUE;
		}
		
		if (!pvpCanHelp(self, breacher))
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] buffList = buff.getOwnedGroupBuffs(self);
		
		float[] durList = buff.getGroupBuffDuration(self, buffList);
		float[] strList = buff.getGroupBuffStrength(self, buffList);
		buff.addGroupBuffEffect(breacher, self, buffList, strList, durList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		
		if (!volumeName.equals("group_buff_breach"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] buffList = buff.getOwnedGroupBuffs(self);
		
		buff.removeGroupBuffEffect(breacher, buffList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLogout(obj_id self) throws InterruptedException
	{
		if (hasTriggerVolume(self, "group_buff_breach"))
		{
			obj_id groupId = getGroupObject(self);
			
			if (!isIdValid(groupId))
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] groupMembers = getGroupMemberIds(groupId);
			
			if (groupMembers == null || groupMembers.length <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			int[] buffList = buff.getOwnedGroupBuffs(self);
			
			for (int i = 0; i < groupMembers.length; ++i)
			{
				testAbortScript();
				buff.removeGroupBuffEffect(groupMembers[i], buffList);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void invisBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stealth.invisBuffRemoved(self, effectName);
	}
	
	
	public void invisBuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int costumeBuff = buff.getBuffOnTargetFromGroup(self, "shapechange");
		if (costumeBuff != 0)
		{
			buff.removeBuff(self, buffName);
			sendSystemMessage(self, new string_id("spam", "costume_not_while_disguised"));
			return;
		}
		
		effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		stealth.invisBuffAdded(self, effectName);
		stopClientEffectObjByLabel(self, effectName);
	}
	
	
	public void noBreakInvisRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String invis = stealth.getInvisBuff(self);
		
		if (invis == null || invis.equals(""))
		{
			stealth.invisBuffRemoved(self, "");
		}
	}
	
	
	public int messageDetrimentalRemoved(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id me = params.getObjId("self");
		String detriment = params.getString("detriment");
		string_id msgId = params.getStringId("msgId");
		
		messageDetrimentalRemoved(me, detriment, msgId);
		return SCRIPT_CONTINUE;
	}
	
	
	public void messageDetrimentalRemoved(obj_id self, String detriment, string_id msgId) throws InterruptedException
	{
		prose_package ppRemoved = new prose_package();
		prose.setTT(ppRemoved, detriment);
		prose.setStringId(ppRemoved, msgId);
		
		sendSystemMessageProse(self, ppRemoved);
	}
	
	
	public int OnChangedPosture(obj_id self, int before, int after) throws InterruptedException
	{
		if (buff.hasBuff(self, "bh_take_cover"))
		{
			buff.removeBuff(self, "bh_take_cover");
			sendSystemMessage(self, new string_id("spam", "lost_cover") );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean removeAllModifiersOfType(obj_id target, int type) throws InterruptedException
	{
		String[] mods = movement.getAllModifiers(target);
		
		if (mods == null || mods.length == 0)
		{
			return false;
		}
		
		boolean removed = false;
		for (int i = 0; i < mods.length; i++)
		{
			testAbortScript();
			if (movement.getType(mods[i])==type)
			{
				if (utils.hasScriptVar(target, movement.MOVEMENT_OBJVAR + "." + mods[i] + ".time") && buff.canBeDispelled(mods[i]))
				{
					utils.removeScriptVarTree(target, movement.MOVEMENT_OBJVAR + "."+ mods[i]);
					combat.removeCombatMovementModifierEffect(target, mods[i]);
					removed = true;
				}
			}
		}
		
		if (removed)
		{
			movement.refresh(target);
			return removed;
		}
		else
		{
			return false;
		}
	}
	
	
	public void channelHealAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int myAttribute = getAttributeType(subtype);
		boolean worked = healing.useChannelHealItem(self, self, myAttribute);
		int pid = sui.smartCountdownTimerSUI(self, self, "channel_heal", null, 0, 12, "", 0, 0);
		utils.setScriptVar(self, "channelHeal.suiPid", pid);
	}
	
	
	public void channelHealRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int pid = utils.getIntScriptVar(self, "channelHeal.suiPid");
		forceCloseSUIPage(pid);
		utils.removeScriptVarTree(self,"channelHeal");
	}
	
	
	public int getAttributeType(String subtype) throws InterruptedException
	{
		int attribute = 0;
		if (subtype.equals("health"))
		{
			attribute = HEALTH;
		}
		else if (subtype.equals("constitution"))
		{
			attribute = CONSTITUTION;
		}
		else if (subtype.equals("action"))
		{
			attribute = ACTION;
		}
		else if (subtype.equals("stamina"))
		{
			attribute = STAMINA;
		}
		else if (subtype.equals("mind"))
		{
			attribute = MIND;
		}
		else if (subtype.equals("willpower"))
		{
			attribute = WILLPOWER;
		}
		
		return attribute;
	}
	
	
	public void bmBeastFamilyAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (!isPlayer(self))
		{
			obj_id master = getMaster(self);
			
			if (!isPlayer(master))
			{
				return;
			}
			
			if (!buff.hasBuff(master, buffName))
			{
				buff.applyBuff(master, self, buffName);
			}
			
			return;
		}
		
		obj_id player = self;
		
		obj_id beast = beast_lib.getBeastOnPlayer(player);
		
		if (!isIdValid(beast) || !exists(beast))
		{
			buff.removeBuff(player, buffName);
			return;
		}
		
		obj_id bcd = beast_lib.getBeastBCD(beast);
		String beastName = beast_lib.getBeastType(bcd);
		dictionary beastDict = utils.dataTableGetRow(beast_lib.BEASTS_TABLE, beastName);
		String specialAttackFamily = beastDict.getString("special_attack_family");
		
		if (!specialAttackFamily.equals(subtype) && !subtype.equals("all"))
		{
			
			buff.removeBuff(player, buffName);
			return;
		}
		
		buff.applyBuff(beast, self, buffName);
		buff.applyBuff(player, self, buffName);
		return;
	}
	
	
	public void bmBeastFamilyRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (!isPlayer(self))
		{
			buff.removeBuff(self, buffName);
			
			obj_id master = getMaster(self);
			
			if (!isPlayer(master))
			{
				return;
			}
			
			if (buff.hasBuff(master, buffName))
			{
				buff.removeBuff(master, buffName);
			}
			
			return;
		}
		
		obj_id player = self;
		
		obj_id beast = beast_lib.getBeastOnPlayer(player);
		
		if (!isIdValid(beast) || !exists(beast))
		{
			
			buff.removeBuff(player, buffName);
			return;
		}
		
		buff.removeBuff(player, buffName);
		
		buff.removeBuff(beast, buffName);
		return;
	}
	
	
	public String getInitialBuffName(String effectName) throws InterruptedException
	{
		
		String[] spiltEffectName = split(effectName, '_');
		int loopTimes = spiltEffectName.length - 1;
		String initialBuffName = spiltEffectName[0];
		for (int i = 1; i < loopTimes; ++i)
		{
			testAbortScript();
			initialBuffName += "_"+ spiltEffectName[i];
		}
		
		return initialBuffName;
	}
	
	
	public void bmBeastXpBuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return;
	}
	
	
	public void bmBeastXpBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		utils.removeScriptVarTree(self, "beastBuff");
		return;
	}
	
	
	public int combatStunAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		obj_id crystal = create.object("object/building/heroic/axkva_empty_crystal.iff", getLocation(self));
		setObjVar(crystal, "player", self);
		obj_id axkva = utils.getObjIdScriptVar(self, "axkva_min");
		trial.setParent(axkva, crystal, false);
		setLocation(self, getLocation(crystal));
		attachScript(crystal, "theme_park.heroic.axkva_min.crystal_prison");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int combatStunRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, "axkva_crystal");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int krixResonanceAddBuffHandler(obj_id self, String effectName, String subType, float duartion, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		playClientEffectObj(self, "appearance/pt_contagion_debuff.prt", self, "", null, "resonance");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int krixResonanceRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		stopClientEffectObjByLabel(self, "resonance");
		
		obj_id krix = getFirstObjectWithScript(getLocation(self), 250.0f, "theme_park.heroic.star_destroyer.krix");
		
		if (!isIdValid(krix) || ai_lib.isDead(krix))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary dict = new dictionary();
		dict.put("player", self);
		messageTo(krix, "contagion_bomb", dict, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int delayedAddBuff(obj_id self, dictionary params) throws InterruptedException
	{
		String buffName = params.getString("buffName");
		if (buffName == null || buffName.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		buff.applyBuff(self, self, buffName);
		return SCRIPT_CONTINUE;
	}
	
	
	public int lureshContagionAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int lureshContagionRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id[] monkey = trial.getObjectsInDungeonWithObjVar(trial.getTop(self), "skreeg_id");
		
		if (monkey == null || monkey.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id luresh = monkey[0];
		
		if (!isIdValid(luresh) || !exists(luresh) || isDead(luresh))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] players = trial.getValidTargetsInCell(trial.getTop(self), "r3");
		
		if (players == null || players.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id topAction = null;
		int currentAction = 0;
		
		for (int i=0; i<players.length; i++)
		{
			testAbortScript();
			int thisPlayer = getAction(players[i]);
			
			if (thisPlayer > currentAction)
			{
				topAction = players[i];
			}
		}
		
		if (!isIdValid(topAction))
		{
			topAction = self;
		}
		
		queueCommand(luresh, (-1048892018), topAction, "", COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int razorBurstHandlerAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int buffCrc = buff.getBuffOnTargetFromGroup(self, "rb_vulnerability");
		
		if (buffCrc == 0)
		{
			buff.applyBuff(self, self, "razor_burst_vulnerability_1");
			return SCRIPT_CONTINUE;
		}
		
		String myBuffName = buff.getBuffNameFromCrc(buffCrc);
		
		String[] parse = split(myBuffName, '_');
		
		int level = utils.stringToInt(parse[parse.length -1]);
		
		level = level > 9 ? 10 : level + 1;
		
		buff.applyBuff(self, self, "razor_burst_vulnerability_"+level);
		return SCRIPT_CONTINUE;
	}
	
	
	public int razorBurstHandlerRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretakerHandlerAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int buffCrc = buff.getBuffOnTargetFromGroup(self, "ct_debuff");
		
		if (buffCrc == 0)
		{
			buff.applyBuff(self, self, "caretaker_debuff_1");
			return SCRIPT_CONTINUE;
		}
		
		String myBuffName = buff.getBuffNameFromCrc(buffCrc);
		
		String[] parse = split(myBuffName, '_');
		int level = utils.stringToInt(parse[parse.length -1]);
		
		level = level > 9 ? 10 : level + 1;
		
		buff.applyBuff(self, self, "caretaker_debuff_"+level);
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretakerHandlerRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int closedFistBurnHandlerAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		playClientEffectObj(self, "appearance/pt_state_onfire.prt", self, "", null, "closed_fist_burn");
		messageTo(self, "handle_closed_fist_burn", null, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handle_closed_fist_burn(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "closed_fist_burn"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id closed_fist = getFirstObjectWithScript(getLocation(self), 250.0f, "theme_park.heroic.exar_kun.closed_fist");
		
		if (!isIdValid(closed_fist) || !exists(closed_fist) || isDead(closed_fist))
		{
			buff.removeBuff(self, "closed_fist_burn");
			return SCRIPT_CONTINUE;
		}
		
		location tarLoc = getLocation(self);
		String targetData = ""+tarLoc.x+" "+tarLoc.y+" "+tarLoc.z+ " "+tarLoc.cell+ " "+tarLoc.x+ " "+tarLoc.y+ " "+tarLoc.z;
		queueCommand(closed_fist, (1856722390), self, targetData, COMMAND_PRIORITY_DEFAULT);
		messageTo(self, "handle_closed_fist_burn", null, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int closedFistBurnHandlerRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		stopClientEffectObjByLabel(self, "closed_fist_burn");
		return SCRIPT_CONTINUE;
	}
	
	
	public int closedFistBurnDebuffAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int buffCrc = buff.getBuffOnTargetFromGroup(self, "cf_burn_debuff");
		
		if (buffCrc == 0)
		{
			buff.applyBuff(self, self, "closed_fist_burn_debuff_1");
			return SCRIPT_CONTINUE;
		}
		
		String myBuffName = buff.getBuffNameFromCrc(buffCrc);
		
		String[] parse = split(myBuffName, '_');
		int level = utils.stringToInt(parse[parse.length -1]);
		
		level = level > 2 ? 3 : level + 1;
		
		buff.applyBuff(self, self, "closed_fist_burn_debuff_"+level);
		return SCRIPT_CONTINUE;
	}
	
	
	public int closedFistBurnDebuffRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int kunChaosHandlerAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int orderDebuffCrc = buff.getBuffOnTargetFromGroup(self, "kun_order_debuff");
		int chaosDebuffCrc = buff.getBuffOnTargetFromGroup(self, "kun_chaos_debuff");
		
		if (orderDebuffCrc == 0)
		{
			if (chaosDebuffCrc == 0)
			{
				buff.applyBuff(self, self, "kun_chaos_debuff_1");
				return SCRIPT_CONTINUE;
			}
			
			String chaosBuffName = buff.getBuffNameFromCrc(chaosDebuffCrc);
			
			String[] parse = split(chaosBuffName, '_');
			int level = utils.stringToInt(parse[parse.length -1]);
			
			level = level > 9 ? 10 : level + 1;
			
			buff.applyBuff(self, self, "kun_chaos_debuff_"+level);
			return SCRIPT_CONTINUE;
		}
		else
		{
			String orderBuffName = buff.getBuffNameFromCrc(orderDebuffCrc);
			
			String[] parse = split(orderBuffName, '_');
			int level = utils.stringToInt(parse[parse.length -1]);
			
			if (level == 1)
			{
				buff.removeBuff(self, "kun_order_debuff_1");
				buff.applyBuff(self, self, "kun_chaos_debuff_1");
				return SCRIPT_CONTINUE;
			}
			
			level = level - 1;
			
			buff.applyBuff(self, self, "kun_order_debuff_"+level);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kunChaosHandlerRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int kunOrderHandlerAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int orderDebuffCrc = buff.getBuffOnTargetFromGroup(self, "kun_order_debuff");
		int chaosDebuffCrc = buff.getBuffOnTargetFromGroup(self, "kun_chaos_debuff");
		
		if (chaosDebuffCrc == 0)
		{
			if (orderDebuffCrc == 0)
			{
				buff.applyBuff(self, self, "kun_order_debuff_1");
				return SCRIPT_CONTINUE;
			}
			
			String orderBuffName = buff.getBuffNameFromCrc(orderDebuffCrc);
			
			String[] parse = split(orderBuffName, '_');
			int level = utils.stringToInt(parse[parse.length -1]);
			
			level = level > 9 ? 10 : level + 1;
			
			buff.applyBuff(self, self, "kun_order_debuff_"+level);
			return SCRIPT_CONTINUE;
		}
		else
		{
			String chaosBuffName = buff.getBuffNameFromCrc(chaosDebuffCrc);
			
			String[] parse = split(chaosBuffName, '_');
			int level = utils.stringToInt(parse[parse.length -1]);
			
			if (level == 1)
			{
				buff.removeBuff(self, "kun_chaos_debuff_1");
				buff.applyBuff(self, self, "kun_order_debuff_1");
				return SCRIPT_CONTINUE;
			}
			
			level = level - 1;
			
			buff.applyBuff(self, self, "kun_chaos_debuff_"+level);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kunOrderHandlerRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int setSpecialTypeAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String special_type_name = subType.substring(15, subType.length());
		setObjVar(self, "specialNicheType", special_type_name);
		return SCRIPT_CONTINUE;
	}
	
	
	public int setSpecialTypeRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasObjVar(self, "specialNicheType"))
		{
			removeObjVar(self, "specialNicheType");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kunOpenSparkAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		playClientEffectObj(self, "appearance/pt_electric_flare_green.prt", self, "head", null, "kun_spark");
		trial.bumpSession(self, "spark_handler");
		messageTo(self, "openSparkHandler", trial.getSessionDict(self, "spark_handler"), 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int openSparkHandler(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params, "spark_handler"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id openHand = getFirstObjectWithScript(getLocation(self), 100.0f, "theme_park.heroic.exar_kun.open_hand");
		
		if (!isIdValid(openHand) || !exists(openHand) || isDead(openHand))
		{
			buff.removeBuff(self, "kun_open_spark");
			return SCRIPT_CONTINUE;
		}
		
		location selfLoc = getLocation(self);
		location tesla = (location)selfLoc.clone();
		tesla.x = -11.7f;
		tesla.y = 11.0f;
		tesla.z = -64.6f;
		
		float travelDistance = getDistance(selfLoc, tesla);
		
		float speed = travelDistance / 2.0f;
		
		createClientProjectile(self, "object/weapon/ranged/pistol/shared_pistol_green_bolt.iff", tesla, selfLoc, speed, 2.0f, false, 255, 0, 0, 255);
		
		String targetData = ""+selfLoc.x+" "+selfLoc.y+" "+selfLoc.z+" "+selfLoc.cell+" "+selfLoc.x+" "+selfLoc.y+" "+selfLoc.z;
		queueCommand(openHand, (744605409), self, targetData, COMMAND_PRIORITY_DEFAULT);
		messageTo(self, "openSparkHandler", trial.getSessionDict(self, "spark_handler"), 4.0f, false);
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int kunOpenSparkRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		stopClientEffectObjByLabel(self, "kun_spark");
		return SCRIPT_CONTINUE;
	}
	
	
	public int callPickupCraftAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int callPickupCraftRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		obj_id pickupCraft = utils.getObjIdScriptVar(self,"instantTravelShip.pickupCraft");
		if (isIdValid(pickupCraft))
		{
			messageTo(pickupCraft, "cleanupShip", null, 0, false);
		}
		
		utils.removeScriptVarTree(self,"instantTravelShip");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_choke_handlerAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int buffCrc = getStringCrc(subtype.toLowerCase());
		obj_id owner = utils.getObjIdScriptVar(self, "buffOwner."+buffCrc);
		
		int imp_choke = getEnhancedSkillStatisticModifierUncapped(owner, "expertise_fs_imp_choke");
		imp_choke = imp_choke > 2 ? 2 : imp_choke;
		
		if (imp_choke > 0)
		{
			buff.applyBuff(self, self, "fs_imp_choke_"+imp_choke);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_choke_handlerRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int jediStatueLightDebuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String buffToApply = "jedi_statue_light_debuff_light";
		if (buff.isInFocus(self))
		{
			buffToApply = "jedi_statue_light_debuff_dark";
		}
		
		buff.applyBuff(self, self, buffToApply);
		return SCRIPT_CONTINUE;
	}
	
	
	public int jediStatueLightDebuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int jediStatueDarkDebuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String buffToApply = "jedi_statue_dark_debuff_light";
		if (buff.isInFocus(self))
		{
			buffToApply = "jedi_statue_dark_debuff_dark";
		}
		
		buff.applyBuff(self, self, buffToApply);
		return SCRIPT_CONTINUE;
	}
	
	
	public int jediStatueDarkDebuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_flurry_procAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.applyBuff(self, self, "attack_override_fs_dm_1|fs_flurry_1");
		buff.applyBuff(self, self, "attack_override_fs_dm_2|fs_flurry_2");
		buff.applyBuff(self, self, "attack_override_fs_dm_3|fs_flurry_3");
		buff.applyBuff(self, self, "attack_override_fs_dm_4|fs_flurry_4");
		buff.applyBuff(self, self, "attack_override_fs_dm_5|fs_flurry_5");
		buff.applyBuff(self, self, "attack_override_fs_dm_6|fs_flurry_6");
		buff.applyBuff(self, self, "attack_override_fs_dm_7|fs_flurry_7");
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_flurry_procRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		String[] buffList = 
		{
			"attack_override_fs_dm_1|fs_flurry_1",
			"attack_override_fs_dm_2|fs_flurry_2",
			"attack_override_fs_dm_3|fs_flurry_3",
			"attack_override_fs_dm_4|fs_flurry_4",
			"attack_override_fs_dm_5|fs_flurry_5",
			"attack_override_fs_dm_6|fs_flurry_6",
			"attack_override_fs_dm_7|fs_flurry_7"
		};
		
		buff.removeBuffs(self, buffList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceThrowAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int buffCrc = getStringCrc(subtype.toLowerCase());
		obj_id owner = utils.getObjIdScriptVar(self, "buffOwner."+buffCrc);
		
		if (!buff.isInStance(owner))
		{
			return SCRIPT_CONTINUE;
		}
		
		int stanceSaberShackle = (int)getSkillStatisticModifier(owner, "expertise_buff_chance_line_fs_force_throw");
		stanceSaberShackle = stanceSaberShackle > 100 ? 100 : stanceSaberShackle;
		
		if (stanceSaberShackle <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int rootChance = (stanceSaberShackle / 25) * 4;
		
		if (rand(0, 99) < rootChance)
		{
			buff.applyBuff(self, owner,"fs_force_throw_root");
			buff.applyBuff(self, owner, getStringCrc("fs_force_throw_"+ (stanceSaberShackle / 25)), duration);
			return SCRIPT_CONTINUE;
		}
		
		if (rand(0, 99) < stanceSaberShackle)
		{
			buff.applyBuff(self, owner, getStringCrc("fs_force_throw_"+ (stanceSaberShackle / 25)), duration);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceThrowRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int aggroChannelAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!exists(caster) || !isIdValid(caster))
		{
			
			buff.removeBuff(self, buffName);
			return SCRIPT_CONTINUE;
		}
		prose_package pp = new prose_package();
		String casterName = "";
		if (isPlayer(caster))
		{
			casterName = getFirstName(caster);
		}
		else
		{
			casterName = "@"+ getName(caster);
		}
		
		prose.setTT(pp, casterName);
		
		if (subtype.equals("target"))
		{
			
			buff.applyBuff(caster, self, "aggroChannelself");
			prose.setStringId(pp, new string_id("squad_leader", "aggro_channel_applied_target"));
			sendSystemMessageProse(self, pp);
			
		}
		if (subtype.equals("self"))
		{
			prose.setStringId(pp, new string_id("squad_leader", "aggro_channel_applied_self"));
			sendSystemMessageProse(self, pp);
			utils.setScriptVar(self, buff.AGGRO_TRANSFER_TO, caster);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int aggroChannelRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (!exists(caster) || !isIdValid(caster))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (subtype.equals("target"))
		{
			
			buff.removeBuff(caster, "aggroChannelself");
		}
		if (subtype.equals("self"))
		{
			obj_id buffed = utils.getObjIdScriptVar(self, buff.AGGRO_TRANSFER_TO);
			buff.removeBuff(buffed, "aggroChannelTarget");
			utils.removeScriptVar(self, buff.AGGRO_TRANSFER_TO);
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		long stack = buff.getBuffStackCount(self, buffName) > 1 ? buff.getBuffStackCount(self, buffName) : 1;
		
		dot.applyBuffDotEffect(self, caster, subtype, buffName + "_"+ subtype, stack, HEALTH, 100, (int)value, (int)duration, true, buffName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dotRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		dot.removeBuffDotEffect(self, subtype, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertiseDamageDecreaseAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int damageDecrease = (int)getSkillStatisticModifier(self, "expertise_damage_decrease_percentage");
		if (damageDecrease > 0)
		{
			skillAddBuffHandler(self, effectName, "damage_decrease_percentage", duration, damageDecrease, "", null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertiseDamageDecreaseRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (effectName.lastIndexOf("_") > 0)
		{
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		}
		
		stopClientEffectObjByLabel(self, effectName);
		
		int damageDecrease = (int)getSkillStatisticModifier(self, "expertise_damage_decrease_percentage");
		
		if (damageDecrease > 0)
		{
			skillRemoveBuffHandler(self, effectName, "damage_decrease_percentage", duration, damageDecrease, "", null);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onAttackRemoveAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		Vector removeBuffs = new Vector();
		removeBuffs.setSize(0);
		
		if (utils.hasScriptVar(self, buff.ON_ATTACK_REMOVE))
		{
			removeBuffs = utils.getResizeableStringArrayScriptVar(self, buff.ON_ATTACK_REMOVE);
		}
		
		if (removeBuffs != null && removeBuffs.size() > 0)
		{
			removeBuffs.add(buffName);
		}
		else if (removeBuffs.indexOf(buffName) < 0)
		{
			removeBuffs.add(buffName);
		}
		
		utils.setScriptVar(self, buff.ON_ATTACK_REMOVE, removeBuffs);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onAttackRemoveRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		Vector removeBuffs = new Vector();
		removeBuffs.setSize(0);
		
		if (utils.hasScriptVar(self, buff.ON_ATTACK_REMOVE))
		{
			removeBuffs = utils.getResizeableStringArrayScriptVar(self, buff.ON_ATTACK_REMOVE);
		}
		else
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.removeScriptVar(self, buff.ON_ATTACK_REMOVE);
		
		if (removeBuffs.indexOf(buffName) > -1)
		{
			removeBuffs.remove(buffName);
		}
		
		if (removeBuffs != null || removeBuffs.size() > 0)
		{
			utils.setScriptVar(self, buff.ON_ATTACK_REMOVE, removeBuffs);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int supression_handlerAddBuffHandler(obj_id self, String effectName, String subytype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float improvedSpeed = (float)getEnhancedSkillStatisticModifierUncapped(caster, "expertise_supression_speed");
		int level = Math.round(improvedSpeed / 10.0f);
		buff.applyBuff(self, caster, "co_supressing_fire_"+level);
		return SCRIPT_CONTINUE;
	}
	
	
	public int supression_handlerRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int movementSupressingEffectAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float snareResist = 0.0f;
		if (isMob(self) && !isPlayer(self))
		{
			snareResist = (float)getEnhancedSkillStatisticModifierUncapped(self, "movement_resist_snare");
		}
		
		snareResist = 1.0f - (snareResist / 100.0f);
		snareResist = snareResist < 0.0f ? 0.0f : snareResist;
		snareResist = snareResist > 1.0f ? 1.0f : snareResist;
		
		value *= snareResist;
		
		value = value > 89.0f ? 89.0f : value;
		addSlowDownEffect(caster, self, 96.0f, 0.01f, value, duration);
		return SCRIPT_CONTINUE;
	}
	
	
	public int movementSupressingEffectRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int damageImmuneAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.performBuffDotImmunity(self, "all");
		addSkillModModifier(self, "damageImmuneDotResistAll", "dot_resist_all", 100, duration, false, true);
		addSkillModModifier(self, "damageImmuneDamageImmune", "damage_immune", 1, duration, false, true);
		
		if (sui.hasEventFlag(self, sui.CD_EVENT_DAMAGE_IMMUNE))
		{
			sui.cancelCountdownTimer(self, sui.CD_EVENT_DAMAGE_IMMUNE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int damageImmuneRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		removeAttribOrSkillModModifier(self, "damageImmuneDotResistAll");
		removeAttribOrSkillModModifier(self, "damageImmuneDamageImmune");
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhFlawlessAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.applyBuff(self, "bh_flawless_proc_chance_1");
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhFlawlessRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.removeBuff(self, "bh_flawless_proc_chance_1");
		return SCRIPT_CONTINUE;
	}
	
	
	public int mtpMeatlumpAngryAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		ai_lib.setMood(self, chat.MOOD_ANGRY);
		chat.setChatMood(self, chat.MOOD_ANGRY);
		
		attachScript(self, "theme_park.meatlump.hideout.angry_meatlump");
		return SCRIPT_CONTINUE;
	}
	
	
	public int mtpMeatlumpAngryRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int mtpMeatlumpHappyAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		ai_lib.setMood(self, chat.MOOD_HAPPY);
		chat.setChatMood(self, chat.MOOD_HAPPY);
		
		removeObjVar(self, "angryMeatlump");
		detachScript(self, "theme_park.meatlump.hideout.angry_meatlump");
		return SCRIPT_CONTINUE;
	}
	
	
	public int mtpMeatlumpHappyRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int eventParticleAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String particleName = buff.getParticle(buffName);
		
		dictionary params = new dictionary();
		params.put("effectName", effectName);
		params.put("particleName", particleName);
		params.put("buffName", buffName);
		messageTo(self, "eventParticleReload", params, 15.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int eventParticleReload(obj_id self, dictionary params) throws InterruptedException
	{
		String particleName = params.getString("particleName");
		String buffName = params.getString("buffName");
		String effectName = params.getString("effectName");
		
		stopClientEffectObjByLabel(self, effectName);
		
		if (buff.hasBuff(self, buffName))
		{
			playClientEffectObj(self, particleName, self, "");
			messageTo(self, "eventParticleReload", params, 15.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int eventParticleRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (effectName.lastIndexOf("_") > 0)
		{
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		}
		
		stopClientEffectObjByLabel(self, effectName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int costumeAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt((int)value, "INDEX", DATATABLE_COSTUME);
		if (row < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String costume = dataTableGetString(DATATABLE_COSTUME, row, "APPEARANCE");
		if (getGender(self) == GENDER_FEMALE)
		{
			String female_costume = dataTableGetString(DATATABLE_COSTUME, row, "FEMALE_APPEARANCE");
			if (female_costume != null && female_costume.length() > 0 && !female_costume.equals("none"))
			{
				costume = female_costume;
			}
		}
		
		if (costume == null || costume.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id unequipedWeapon = utils.unequipWeaponHand(self);
		if (isIdValid(unequipedWeapon))
		{
			setObjVar(self, "costume.unequipedWeapon", unequipedWeapon);
		}
		
		setObjectAppearance(self, costume);
		
		attachScript(self, "event.halloween.trick_or_treater");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int costumeRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		revertObjectAppearance(self);
		detachScript(self, "event.halloween.trick_or_treater");
		
		if (hasObjVar(self, "costume.unequipedWeapon"))
		{
			obj_id unequipedWeapon = getObjIdObjVar(self, "costume.unequipedWeapon");
			if (isIdValid(unequipedWeapon))
			{
				if (exists(unequipedWeapon) && utils.isNestedWithin(unequipedWeapon, self))
				{
					obj_id alreadyEquipped = getObjectInSlot(self, "hold_r");
					if (!isIdValid(alreadyEquipped))
					{
						equip(unequipedWeapon, self);
					}
				}
			}
			
			removeObjVar(self, "costume.unequipedWeapon");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int costumeBurningEffectAddBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		playClientEffectObj(self, "appearance/pt_state_onfire.prt", self, "", null, "costume_burning_effect");
		
		messageTo(self, "handleCostumeBurningBuffEffect", null, 15, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int costumeBurningEffectRemoveBuffHandler(obj_id self, String effectName, String subType, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		stopClientEffectObjByLabel(self, "costume_burning_effect");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCostumeBurningBuffEffect(obj_id self, dictionary params) throws InterruptedException
	{
		int[] costumeBurningBuffs = buff.getAllBuffsByEffect(self, "costume_burning_effect");
		if (costumeBurningBuffs != null && costumeBurningBuffs.length > 0)
		{
			stopClientEffectObjByLabel(self, "costume_burning_effect");
			playClientEffectObj(self, "appearance/pt_state_onfire.prt", self, "", null, "costume_burning_effect");
			
			messageTo(self, "handleCostumeBurningBuffEffect", null, 15, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNewbieTutorialResponse(obj_id self, String action) throws InterruptedException
	{
		if (action.equals("clientReady"))
		{
			int[] costumeBurningBuffs = buff.getAllBuffsByEffect(self, "costume_burning_effect");
			if (costumeBurningBuffs != null && costumeBurningBuffs.length > 0)
			{
				stopClientEffectObjByLabel(self, "costume_burning_effect");
				playClientEffectObj(self, "appearance/pt_state_onfire.prt", self, "", null, "costume_burning_effect");
				
				messageTo(self, "handleCostumeBurningBuffEffect", null, 15, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int halloweenCooldownAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float yaw = getYaw(self);
		utils.setScriptVar(self, "yaw", yaw);
		suspendMovement(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int halloweenCooldownRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		float yaw = utils.getFloatScriptVar(self, "yaw");
		setYaw(self, yaw);
		utils.removeScriptVar(self, "yaw");
		resumeMovement(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int halloweenCoinLimitAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int now = getCalendarTime();
		int secondsUntil = secondsUntilNextDailyTime(10, 0, 0);
		int then = now + secondsUntil;
		
		if (hasObjVar(self, event_perk.COUNTER_TIMESTAMP))
		{
			then = getIntObjVar(self, event_perk.COUNTER_TIMESTAMP);
			if (now > then)
			{
				removeObjVar(self, event_perk.COUNTER_TIMESTAMP);
				buff.removeBuff(self, "event_halloween_coin_limit");
				return SCRIPT_CONTINUE;
			}
		}
		
		if (!hasObjVar(self, event_perk.COUNTER_TIMESTAMP))
		{
			setObjVar(self, event_perk.COUNTER_TIMESTAMP, then);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int halloweenCoinLimitRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (hasObjVar(self, event_perk.COUNTER_TIMESTAMP))
		{
			int now = getCalendarTime();
			int then = getIntObjVar(self, event_perk.COUNTER_TIMESTAMP);
			if (now >= then)
			{
				removeObjVar(self, event_perk.COUNTER_TIMESTAMP);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lifedayCompetitiveBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		
		if (lifedayRunning != null && !lifedayRunning.equals("false"))
		{
			
			obj_id tatooine = getPlanetByName("tatooine");
			
			setObjVar(self, "lifeday.locked_out", 1);
			string_id times_up = new string_id("event/life_day", "time_is_up");
			sendSystemMessage(self, times_up);
			
			if (!isIdValid(tatooine) || !exists(tatooine))
			{
				return SCRIPT_CONTINUE;
			}
			
			dictionary params = new dictionary();
			params.put ("player", self);
			messageTo(tatooine, "lifeDayScoreBoardUpdate", params, 0.0f, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int scoreBoardCheck(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "lifeday.token_counter"))
		{
			int playerScore = getIntObjVar(self, "lifeday.token_counter");
			String playerName = getName(self);
			String scoreBoardEmpty = params.getString("scoreBoardEmpty");
			
			String playerFaction = "null";
			
			if (hasObjVar(self, "lifeday.neutral_imperial"))
			{
				playerFaction = "Imperial Mercenary";
			}
			if (hasObjVar(self, "lifeday.neutral_rebel"))
			{
				playerFaction = "Rebel Mercenary";
			}
			if (factions.isImperial(self))
			{
				playerFaction = "Imperial Soldier";
			}
			if (factions.isRebel(self))
			{
				playerFaction = "Rebel Soldier";
			}
			
			if (scoreBoardEmpty.equals ("true"))
			{
				updateScore (self, playerName, playerScore, playerFaction, "One", "true");
			}
			
			else
			{
				int thirdPlaceScore = params.getInt("thirdPlaceScore");
				int secondPlaceScore = params.getInt("secondPlaceScore");
				int firstPlaceScore = params.getInt("firstPlaceScore");
				
				if (playerScore < thirdPlaceScore)
				{
					return SCRIPT_CONTINUE;
				}
				
				else if (playerScore >= firstPlaceScore)
				{
					updateScore (self, playerName, playerScore, playerFaction, "One", "false");
				}
				
				else if (playerScore >= secondPlaceScore)
				{
					updateScore (self, playerName, playerScore, playerFaction, "Two", "false");
				}
				
				else
				{
					updateScore (self, playerName, playerScore, playerFaction, "Three", "false");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void updateScore(obj_id self, String playerName, int playerScore, String playerFaction, String position, String scoreBoardEmpty) throws InterruptedException
	{
		
		obj_id tatooine = getPlanetByName("tatooine");
		
		int oneScore = 0;
		String oneName = "";
		String oneFaction = "";
		obj_id oneId = null;
		
		int twoScore = 0;
		String twoName = "";
		String twoFaction = "";
		obj_id twoId = null;
		
		if (hasObjVar(tatooine, "lifeday.positionOne.playerScore"))
		{
			oneScore = getIntObjVar (tatooine, "lifeday.positionOne.playerScore");
			oneName = getStringObjVar (tatooine, "lifeday.positionOne.playerName");
			oneFaction = getStringObjVar (tatooine, "lifeday.positionOne.playerFaction");
			oneId = getObjIdObjVar (tatooine, "lifeday.positionOne.playerObjId");
			
			if (hasObjVar(tatooine, "lifeday.positionTwo.playerScore"))
			{
				twoScore = getIntObjVar (tatooine, "lifeday.positionTwo.playerScore");
				twoName = getStringObjVar (tatooine, "lifeday.positionTwo.playerName");
				twoFaction = getStringObjVar (tatooine, "lifeday.positionTwo.playerFaction");
				twoId = getObjIdObjVar (tatooine, "lifeday.positionTwo.playerObjId");
			}
		}
		
		if (playerFaction.equals ("null"))
		{
			return;
		}
		
		if (scoreBoardEmpty.equals ("true"))
		{
			setObjVar (tatooine, "lifeday.emptyScoreBoard", "false");
		}
		
		if (position.equals ("One"))
		{
			if (isIdValid (twoId))
			{
				setObjVar (tatooine, "lifeday.positionThree.playerScore", twoScore);
				setObjVar (tatooine, "lifeday.positionThree.playerName", twoName);
				setObjVar (tatooine, "lifeday.positionThree.playerObjId", twoId);
				setObjVar (tatooine, "lifeday.positionThree.playerFaction", twoFaction);
			}
			if (isIdValid (oneId))
			{
				setObjVar (tatooine, "lifeday.positionTwo.playerScore", oneScore);
				setObjVar (tatooine, "lifeday.positionTwo.playerName", oneName);
				setObjVar (tatooine, "lifeday.positionTwo.playerObjId", oneId);
				setObjVar (tatooine, "lifeday.positionTwo.playerFaction", oneFaction);
			}
		}
		
		if (position.equals ("Two"))
		{
			if (isIdValid (twoId))
			{
				setObjVar (tatooine, "lifeday.positionThree.playerScore", twoScore);
				setObjVar (tatooine, "lifeday.positionThree.playerName", twoName);
				setObjVar (tatooine, "lifeday.positionThree.playerObjId", twoId);
				setObjVar (tatooine, "lifeday.positionThree.playerFaction", twoFaction);
			}
		}
		
		setObjVar (tatooine, "lifeday.emptyScoreBoard", "false");
		setObjVar (tatooine, "lifeday.position"+position+".playerScore", playerScore);
		setObjVar (tatooine, "lifeday.position"+position+".playerName", playerName);
		setObjVar (tatooine, "lifeday.position"+position+".playerObjId", self);
		setObjVar (tatooine, "lifeday.position"+position+".playerFaction", playerFaction);
		
		return;
	}
	
	
	public void checkLifeDayData(obj_id planet) throws InterruptedException
	{
		if (!hasObjVar(planet, "lifeday.time_stamp"))
		{
			newLifeDayTimeStamp(planet);
		}
		
		if (!hasObjVar(planet, "lifeday.emptyScoreBoard"))
		{
			setObjVar(planet, "lifeday.emptyScoreBoard", "true");
		}
		
		if (hasObjVar(planet, "lifeday.positionOne.playerScore"))
		{
			if (!newLifeDayDay(planet))
			{
				setObjVar(planet, "lifeday.emptyScoreBoard", "false");
			}
		}
		
		if (newLifeDayDay(planet))
		{
			removeObjVar(planet, "lifeday");
			setObjVar(planet, "lifeday.emptyScoreBoard", "true");
			newLifeDayTimeStamp(planet);
		}
	}
	
	
	public boolean newLifeDayDay(obj_id planet) throws InterruptedException
	{
		if (hasObjVar(planet, "lifeday.time_stamp"))
		{
			int now = getCalendarTime();
			int then = getIntObjVar(planet, "lifeday.time_stamp");
			
			if (now > then)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	
	
	public void newLifeDayTimeStamp(obj_id planet) throws InterruptedException
	{
		int now = getCalendarTime();
		int secondsUntil = secondsUntilNextDailyTime(10, 0, 0);
		int then = now + secondsUntil;
		setObjVar(planet, "lifeday.time_stamp", then);
	}
	
	
	public int procResistAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int procResistRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampaBossIceThrowPrepAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		sendWampaBossWarning(self, "wampa_boss_ice_throw_prep");
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampaBossIceThrowPrepRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		queueCommand(caster, (-1448600302), self, "", COMMAND_PRIORITY_IMMEDIATE);
		
		obj_id[] otherPlayers = trial.getValidTargetsInRadiusIgnoreLOS(self, 8.0f);
		if (otherPlayers != null && otherPlayers.length > 0)
		{
			for (int i = 0; i < otherPlayers.length; i++)
			{
				testAbortScript();
				obj_id nextPlayer = otherPlayers[i];
				if (isIdValid(nextPlayer) && nextPlayer != self)
				{
					buff.applyBuff(nextPlayer, caster, "wampa_boss_ice_throw");
					sendSystemMessage(nextPlayer, new string_id ("dungeon_strings", "wampa_boss_ice_throw"));
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampaBossTauntaunThrowPrepAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		sendWampaBossWarning(self, "wampa_boss_tauntaun_throw_prep");
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampaBossTauntaunThrowPrepRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		queueCommand(caster, (156825053), self, "", COMMAND_PRIORITY_IMMEDIATE);
		
		obj_id[] otherPlayers = trial.getValidTargetsInRadiusIgnoreLOS(self, 8.0f);
		if (otherPlayers != null && otherPlayers.length > 0)
		{
			for (int i = 0; i < otherPlayers.length; i++)
			{
				testAbortScript();
				obj_id nextPlayer = otherPlayers[i];
				if (isIdValid(nextPlayer) && nextPlayer != self)
				{
					buff.applyBuff(nextPlayer, caster, "wampa_boss_tauntaun_throw");
					sendSystemMessage(nextPlayer, new string_id ("dungeon_strings", "wampa_boss_tauntaun_throw"));
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void sendWampaBossWarning(obj_id target, String messageId) throws InterruptedException
	{
		String targetName = getName(target);
		
		obj_id[] playersInArea = getPlayerCreaturesInRange(target, 250.0f);
		if (playersInArea != null && playersInArea.length > 0)
		{
			for (int i = 0; i < playersInArea.length; i++)
			{
				testAbortScript();
				obj_id player = playersInArea[i];
				if (isIdValid(player) && exists(player))
				{
					string_id message = new string_id ("dungeon_strings", messageId);
					prose_package pp = prose.getPackage(message, target, target);
					prose.setTO(pp, targetName);
					
					sendSystemMessageProse(player, pp);
				}
			}
		}
		return;
	}
	
	
	public int iceblockEffectAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		dictionary webster = new dictionary();
		webster.put("caster", caster);
		webster.put("buffName", buffName);
		messageTo(self, "handleDoIceblock", webster, 0.25f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDoIceblock(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		String buffName = params.getString("buffName");
		if (isIncapacitated(self) || isDead(self))
		{
			buff.removeBuff(self, buffName);
		}
		else
		{
			if (!utils.hasScriptVar(self, "iceblock") && buff.hasBuff(self, buffName))
			{
				obj_id caster = params.getObjId("caster");
				obj_id iceblock = create.object("object/tangible/dungeon/hoth/ice_block.iff", getLocation(self));
				if (isIdValid(iceblock))
				{
					if (isIdValid(caster))
					{
						trial.setParent(caster, iceblock, false);
					}
					setLocation(self, getLocation(iceblock));
					
					utils.setScriptVar(self, "iceblock", iceblock);
					utils.setScriptVar(iceblock, "iceblocked", self);
					utils.setScriptVar(iceblock, "buffName", buffName);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int iceblockEffectRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id iceblock = utils.getObjIdScriptVar(self, "iceblock");
		trial.cleanupObject(iceblock);
		
		utils.removeScriptVar(self, "iceblock");
		return SCRIPT_CONTINUE;
	}
	
	
	public int barricadeDefenderAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.setScriptVar(self, combat.DAMAGE_REDIRECT, caster);
		return SCRIPT_CONTINUE;
	}
	
	
	public int barricadeDefenderRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVar(self, combat.DAMAGE_REDIRECT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int snowspeederInstructionsAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		string_id message = new string_id("theme_park/heroic", "hoth_snowspeeder_instructions");
		String templateOverride = "object/mobile/dressed_echo_base_luke_skywalker.iff";
		
		obj_id instructor = self;
		if (isIdValid(caster))
		{
			instructor = caster;
		}
		
		utils.messagePlayer(instructor, self, message, templateOverride);
		return SCRIPT_CONTINUE;
	}
	
	
	public int snowspeederInstructionsRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.applyBuff(self, "snowspeeder_instructions_block");
		return SCRIPT_CONTINUE;
	}
	
	
	public int radarHueAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		int row = dataTableSearchColumnForInt((int)value, "index", DATATABLE_RADAR_HUE);
		if (row < 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int red = dataTableGetInt(DATATABLE_RADAR_HUE, row, "red");
		int green = dataTableGetInt(DATATABLE_RADAR_HUE, row, "green");
		int blue = dataTableGetInt(DATATABLE_RADAR_HUE, row, "blue");
		
		if (red < 0)
		{
			red = 0;
		}
		
		if (red > 255)
		{
			red = 255;
		}
		
		if (green < 0)
		{
			green = 0;
		}
		
		if (green > 255)
		{
			green = 255;
		}
		
		if (blue < 0)
		{
			blue = 0;
		}
		
		if (blue > 255)
		{
			blue = 255;
		}
		
		setOverrideMapColor(self, red, green, blue);
		return SCRIPT_CONTINUE;
	}
	
	
	public int radarHueRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		clearOverrideMapColor(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhShieldsChargedAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhShieldsChargedRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhShieldsAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		attachScript(self, "player.skill.bh_shields");
		String shield_buff = "bh_shields";
		buff.applyBuff(self, shield_buff);
		_decrementBuffStack(self, getStringCrc(shield_buff.toLowerCase()), -98);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bhShieldsRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_covert_masteryAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_covert_masteryRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	public int randomCollectionItemAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		obj_id collectionItem = collection.grantRandomCollectionItem(self, "datatables/loot/loot_items/collectible/magseal_loot.iff", "collections");
		buff.removeBuff(self, buffName);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int randomCollectionItemRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwBonusGeneralAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		CustomerServiceLog("buff", "gcwBonusGeneral Buff used by player: "+self+" Name: "+getName(self)+ " Effect: "+effectName+" subtype:"+subtype+" duration: "+duration+" value: "+value+" buffName: "+buffName+" caster: "+caster);
		utils.setScriptVar(self, "buff.gcwBonusGeneral.value", value / 100);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwBonusGeneralRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		utils.removeScriptVarTree(self, "buff.gcwBonusGeneral");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int battlefieldCommuncationsGlowAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		buff.removeBuff(self, "battlefield_radar_invisibility");
		
		if (stealth.hasInvisibleBuff(self))
		{
			buff.removeBuff(self, stealth.getInvisBuff(self));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int battlefieldCommuncationsGlowRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (effectName.lastIndexOf("_") > 0)
		{
			effectName = effectName.substring(0, (effectName.lastIndexOf("_")));
		}
		
		stopClientEffectObjByLabel(self, self, effectName);
		
		buff.applyBuff(self, "battlefield_radar_invisibility");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayImperialRecruitmentAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayImperialRecruitmentRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (buff.hasBuff(self, holiday.BUFF_IMPERIAL_RECRUITMENT_COUNTER))
		{
			buff.removeBuff(self, holiday.BUFF_IMPERIAL_RECRUITMENT_COUNTER);
		}
		modifyCollectionSlotValue(self, holiday.IMPERIAL_RECRUITING_COUNTER_SLOT, 1);
		play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_COMPLETED);
		setObjVar(self, holiday.EMPIRE_DAY_RECRUITMENT_LOCKED_OUT, true);
		holiday.setEventLockOutTimeStamp(self, holiday.EMPIRE_DAY_RECRUITMENT_TIMESTAMP);
		
		if (hasObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialRecruitmentRemoveBuffHandler: Player: ("+self+") is IMPERIAL and updating leader board because this is the second buff.");
			if (!updateEmpireDayLeaderBoard(self, holiday.IMPERIAL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialRecruitmentRemoveBuffHandler: updateEmpireDayLeaderBoard reports that updating score for Player: ("+self+") FAILED.");
			}
			removeObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialRecruitmentRemoveBuffHandler: Player: ("+self+") is finishing their first buff. The leader board will not be updated.");
			setObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER, true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayRebelResistanceAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayRebelResistanceRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (buff.hasBuff(self, holiday.BUFF_REBEL_RESISTANCE_COUNTER))
		{
			buff.removeBuff(self, holiday.BUFF_REBEL_RESISTANCE_COUNTER);
		}
		modifyCollectionSlotValue(self, holiday.REBEL_RESISTANCE_COUNTER_SLOT, 1);
		play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_COMPLETED);
		setObjVar(self, holiday.EMPIRE_DAY_RESISTANCE_LOCKED_OUT, true);
		holiday.setEventLockOutTimeStamp(self, holiday.EMPIRE_DAY_RESISTANCE_TIMESTAMP);
		
		if (hasObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelResistanceRemoveBuffHandler: Player: ("+self+") is REBEL and updating leader board because this is the second buff.");
			if (!updateEmpireDayLeaderBoard(self, holiday.REBEL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelResistanceRemoveBuffHandler: updateEmpireDayLeaderBoard reports that updating score for Player: ("+self+") FAILED.");
			}
			removeObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelResistanceRemoveBuffHandler: Player: ("+self+") is finishing their first buff. The leader board will not be updated.");
			setObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER, true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayImperialPropagandaAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayImperialPropagandaRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (buff.hasBuff(self, holiday.BUFF_IMPERIAL_ANTIPROPAGANDA_COUNTER))
		{
			buff.removeBuff(self, holiday.BUFF_IMPERIAL_ANTIPROPAGANDA_COUNTER);
		}
		
		modifyCollectionSlotValue(self, holiday.IMPERIAL_ANTIPROP_COUNTER_SLOT, 1);
		play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_COMPLETED);
		setObjVar(self, holiday.EMPIRE_DAY_PROPAGANDA_LOCKED_OUT, true);
		holiday.setEventLockOutTimeStamp(self, holiday.EMPIRE_DAY_PROPAGANDA_TIMESTAMP);
		
		if (hasObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialPropagandaRemoveBuffHandler: Player: ("+self+") is IMPERIAL and updating leader board because this is the second buff.");
			if (!updateEmpireDayLeaderBoard(self, holiday.IMPERIAL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialPropagandaRemoveBuffHandler: updateEmpireDayLeaderBoard reports that updating score for Player: ("+self+") FAILED.");
			}
			removeObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayImperialPropagandaRemoveBuffHandler: Player: ("+self+") is finishing their first buff. The leader board will not be updated.");
			setObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER, true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayRebelVandalAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int empireDayRebelVandalRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		
		if (buff.hasBuff(self, holiday.BUFF_REBEL_PLAYER_VANDAL_COUNTER))
		{
			buff.removeBuff(self, holiday.BUFF_REBEL_PLAYER_VANDAL_COUNTER);
		}
		
		modifyCollectionSlotValue(self, holiday.REBEL_VANDAL_COUNTER_SLOT, 1);
		play2dNonLoopingSound(self, groundquests.MUSIC_QUEST_COMPLETED);
		setObjVar(self, holiday.EMPIRE_DAY_VANDAL_LOCKED_OUT, true);
		holiday.setEventLockOutTimeStamp(self, holiday.EMPIRE_DAY_VANDAL_TIMESTAMP);
		
		if (hasObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelVandalRemoveBuffHandler: Player: ("+self+") is REBEL updating leader board because this is the second buff.");
			if (!updateEmpireDayLeaderBoard(self, holiday.REBEL_PLAYER))
			{
				CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelVandalRemoveBuffHandler: updateEmpireDayLeaderBoard reports that updating score for Player: ("+self+") FAILED.");
			}
			removeObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "buff_handler.empireDayRebelVandalRemoveBuffHandler: Player: ("+self+") is finishing their first buff. The leader board will not be updated.");
			setObjVar(self, holiday.EMPIRE_DAY_BUFF_TRACKER, true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean updateEmpireDayLeaderBoard(obj_id player, int faction) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Updating leaderboard with Player: "+player+" of faction: "+faction+"score");
		
		if (!hasObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Updating leaderboard with Player: "+player+" HAS NO SCORE. FAILING UPDATE");
			return false;
		}
		
		int playerScore = getIntObjVar(player, holiday.PLAYER_EMPIRE_DAY_SCORE);
		if (playerScore <= 0)
		{
			CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Updating leaderboard with Player: "+player+" HAS A CORRUPT SCORE. FAILING UPDATE");
			return false;
		}
		
		String playerName = getPlayerFullName(player);
		if (playerName == null || playerName.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Updating leaderboard with Player: "+player+" HAS AN INVALID NAME. FAILING UPDATE");
			return false;
		}
		
		obj_id tatooine = getPlanetByName("tatooine");
		if (!isIdValid(tatooine) || !exists(tatooine))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Could not find Tatooine OID.");
			return false;
		}
		
		int factionPassed = holiday.IMPERIAL_PLAYER;
		if (faction == holiday.REBEL_PLAYER)
		{
			factionPassed = holiday.REBEL_PLAYER;
		}
		
		CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: FACTION PASSED: "+factionPassed);
		
		if (!holiday.setEmpireDayLeaderScores(tatooine, player, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, playerScore, playerName, factionPassed))
		{
			CustomerServiceLog("holidayEvent", "buff_handler.updateEmpireDayLeaderBoard: Player: ("+player+") "+playerName+ " was unable to attain leader board data. Their score was: "+playerScore);
			return false;
		}
		return true;
	}
	
	
	public int deathTroopersAfflictedTossAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		messageTo(self, "createAfflicted", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createAfflicted(obj_id self, dictionary params) throws InterruptedException
	{
		location loc = getLocation(self);
		
		obj_id afflicted = create.object("outbreak_afflicted_lvl_90", loc);
		
		debugSpeakMsg(self, "Eww. Gooy! "+ afflicted);
		
		if (isIdValid(afflicted))
		{
			setHate(self, afflicted, 0);
			setHate(afflicted, self, 0);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int outbreakStackingInfectionAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "outbreak.innoculated") || !isInBlackwingInfectionedArea(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (value < 1)
		{
			value = 1.0f;
		}
		
		if (buff.getBuffStackCount(self, buffName) <= 1)
		{
			dictionary webster = trial.getSessionDict(self, "outbreak_stacking_infection");
			webster.put("value", value);
			messageTo(self, "handleOutbreakStackingInfection", webster, value, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int outbreakStackingInfectionRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		trial.bumpSession(self, "outbreak_stacking_infection");
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleOutbreakStackingInfection(obj_id self, dictionary params) throws InterruptedException
	{
		if (!trial.verifySession(self, params, "outbreak_stacking_infection"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (( utils.hasScriptVar(self, "outbreak.innoculated") || buff.hasBuff(self, "death_troopers_inoculation") || buff.hasBuff(self, "death_troopers_infection_3") || !isInBlackwingInfectionedArea(self) ))
		{
			utils.setScriptVar(self, "outbreak.innoculated", true);
			buff.removeBuff(self, "death_troopers_infection_2");
			return SCRIPT_CONTINUE;
		}
		
		float value = params.getFloat("value");
		if (value < 1)
		{
			value = 1.0f;
		}
		
		int buffCrc = getStringCrc("death_troopers_infection_2");
		
		buff_data bdata = combat_engine.getBuffData(buffCrc);
		int buffMaxStacks = bdata.maxStacks;
		long buffStack = buff.getBuffStackCount(self, buffCrc);
		if (buffStack >= buffMaxStacks)
		{
			
		}
		else
		{
			buff.applyBuff(self, self, "death_troopers_infection_2");
		}
		
		dictionary webster = trial.getSessionDict(self, "outbreak_stacking_infection");
		webster.put("value", value);
		messageTo(self, "handleOutbreakStackingInfection", webster, value, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isInBlackwingInfectionedArea(obj_id self) throws InterruptedException
	{
		if (locations.isInRegion(self, "@dathomir_region_names:black_mesa"))
		{
			return true;
		}
		
		obj_id building = getTopMostContainer(self);
		if (isIdValid(building) && getTemplateName(building).equals("object/building/dathomir/imperial_blackwing_facility.iff"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public int killPlayerAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		if (!isDead(self))
		{
			setPosture(self, POSTURE_INCAPACITATED);
			pclib.killPlayer(self, self, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwSpyDestroyPatrolExplosivesAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwSpyDestroyPatrolExplosivesRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwGeneralResistanceStackAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwGeneralResistanceStackRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwFatigueDecay(obj_id self, dictionary params) throws InterruptedException
	{
		int gcwFatigueTime = params.getInt("gcwFatigueTime");
		int gcwFatigueScriptTime = utils.getIntScriptVar(self, "gcw.fatigueTime");
		
		LOG("buff_handler", "gcwFatigueDecay gcwFatigueTime: "+ gcwFatigueTime + " gcwFatigueScriptTime: "+ gcwFatigueScriptTime);
		
		if (gcwFatigueTime != gcwFatigueScriptTime)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (buff.hasBuff(self, "gcw_fatigue"))
		{
			long buffStack = buff.getBuffStackCount(self, "gcw_fatigue");
			
			buff.removeBuff(self, "gcw_fatigue");
			
			if (buffStack > 1)
			{
				buff.applyBuff(self, "gcw_fatigue", (int)buffStack - 1);
			}
			
			if (buff.hasBuff(self, "gcw_fatigue"))
			{
				int gameTime = getGameTime();
				
				params.put("gcwFatigueTime", gameTime);
				utils.setScriptVar(self, "gcw.fatigueTime", gameTime);
				
				messageTo(self, "gcwFatigueDecay", params, 120.0f, false);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwNpcEntertainerBuffAddBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwNpcEntertainerBuffRemoveBuffHandler(obj_id self, String effectName, String subtype, float duration, float value, String buffName, obj_id caster) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
}
