package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.ai.ai_combat;
import script.library.ai_lib;
import script.library.buff;
import script.library.camping;
import script.library.combat;
import script.library.combat;
import script.library.consumable;
import script.library.cybernetic;
import script.library.dot;
import script.library.factions;
import script.library.group;
import script.library.luck;
import script.library.metrics;
import script.library.pet_lib;
import script.library.player_structure;
import script.library.prose;
import script.library.pvp;
import script.library.stealth;
import script.library.sui;
import script.library.utils;
import script.library.vehicle;
import script.library.xp;
import script.library.beast_lib;


public class healing extends script.base_script
{
	public healing()
	{
	}
	public static final int VAR_ITEM_REUSE_TIME = 20;
	public static final int ITEM_CHANNEL_HEAL_TICKS = 5;
	
	public static final String VAR_ENHANCER_EFFICIENCY = "healing.enhancement";
	public static final String VAR_ITEM_POWER = "healing.power";
	public static final String VAR_ITEM_TIMER = "healing.item.timer";
	public static final String VAR_PLAYER_HOT_ID = "healing.hot_id";
	
	public static final String MSG_HEAL_OVER_TIME = "handleHealOverTimeTick";
	
	public static final string_id SID_ITEM_TOO_SOON = new string_id("healing", "item_too_soon");
	public static final string_id SID_ITEM_NOT_A_PET = new string_id("healing", "item_not_a_pet");
	public static final string_id SID_ITEM_NOT_YOUR_PET = new string_id("healing", "item_not_your_pet");
	public static final string_id SID_ITEM_NO_DAMAGE = new string_id("healing", "item_no_damage");
	public static final string_id SID_ITEM_PLAYER_HEAL = new string_id("healing", "item_player_heal");
	public static final string_id SID_ITEM_PET_HEAL = new string_id("healing", "item_pet_heal");
	public static final string_id SID_BEAST_MASTER_HEAL = new string_id("healing", "beast_master_heal");
	
	public static final string_id SID_PERFORM_HEAL_DAMAGE_SUCCESS = new string_id("healing", "perform_heal_damage_success");
	public static final string_id SID_PERFORM_HEAL_WOUND_SUCCESS = new string_id("healing", "perform_heal_wound_success");
	public static final string_id SID_PERFORM_REVIVE_SUCCESS = new string_id("healing", "perform_revive_success");
	public static final string_id SID_PERFORM_REVIVE_GROGGINESS = new string_id("healing", "perform_revive_grogginess");
	
	public static final string_id SID_PERFORM_REVIVE_TOO_LONG = new string_id("healing", "perform_revive_too_long");
	public static final string_id SID_PERFORM_REVIVE_JEDI_CONSENT = new string_id("healing", "perform_revive_jedi_consent");
	public static final string_id SID_PERFORM_REVIVE_CONSENT = new string_id("healing", "perform_revive_consent");
	
	public static final string_id SID_PERFORM_CURE_STATE_FAIL = new string_id("healing", "perform_cure_state_fail");
	public static final string_id SID_PERFORM_BUFF_FAIL = new string_id("healing", "perform_buff_fail");
	public static final string_id SID_PERFORM_DEBUFF_FAIL = new string_id("healing", "perform_debuff_fail");
	public static final string_id SID_PERFORM_HOT_FAIL = new string_id("healing", "perform_hot_fail");
	
	public static final float AM_HEAL_WOUND = -1.0f;
	public static final float AM_HEAL_SHOCK = -2.0f;
	public static final float AM_ADD_SHOCK = -3.0f;
	
	public static final float HEARTBEAT_PERFORMANCE = 6.0f;
	
	public static final int OBSERVERS_MAX = 20;
	
	public static final int VAR_HEALDAMAGE_ROUNDTIME = 10;
	public static final float VAR_HEALDAMAGE_ROUNDTIME_CAP = 3.0f;
	
	public static final int VAR_HEALWOUND_ROUNDTIME = 10;
	public static final int VAR_HEALENHANCE_ROUNDTIME = 30;
	
	public static final float VAR_SURGICAL_DROID_RANGE = 20.0f;
	public static final int VAR_MAX_BUFF_TIME = 7200;
	public static final float VAR_BUFF_MOD_ATTACK = 10.0f;
	public static final float VAR_BUFF_MOD_DECAY = 10.0f;
	public static final int VAR_HEALDAMAGE_COST = 100;
	public static final int VAR_HEALWOUND_COST = 250;
	public static final int VAR_HEALSTATE_COST = 150;
	public static final int VAR_HEALENHANCE_COST = 250;
	public static final int VAR_FIRSTAID_COST = 125;
	public static final int VAR_CURE_POISON_COST = 200;
	public static final int VAR_CURE_FIRE_COST = 350;
	public static final int VAR_CURE_DISEASE_COST = 200;
	public static final int VAR_APPLY_POISON_COST = 150;
	public static final int VAR_APPLY_DISEASE_COST = 150;
	public static final int VAR_QUICK_HEAL_COST = 500;
	public static final int VAR_TEND_WOUND_COST = 500;
	public static final int VAR_TEND_DAMAGE_COST = 250;
	public static final int VAR_QUICK_HEAL_WOUND_COST = 10;
	public static final int VAR_QUICK_HEAL_BASE_POWER = 400;
	public static final int VAR_TEND_WOUND_WOUND_COST = 7;
	public static final int VAR_TEND_WOUND_BASE_POWER = 15;
	public static final int VAR_TEND_DAMAGE_WOUND_COST = 5;
	public static final int VAR_TEND_DAMAGE_BASE_POWER = 75;
	public static final int VAR_HEAL_MIND_COST = 100;
	public static final int VAR_HEAL_MIND_WOUND_COST = 40;
	public static final int VAR_HEAL_MIND_BASE_POWER = 500;
	public static final float VAR_EFFECT_DISPLAY_RADIUS = 45.0f;
	public static final float VAR_STIMPACK_DROID_RADIUS = 30.0f;
	public static final int VAR_AUTO_REPAIR_PULSE_INTERVAL = 10;
	public static final int VAR_AUTO_REPAIR_MIN_INTERVAL = 2;
	
	public static final int COST_MIND_REVIVE = 500;
	public static final int REVIVE_TIMER = 1800;
	
	public static final float HEALING_KILL_CREDIT_MULTIPLIER = 0.1f;
	public static final float HEALING_AGGRO_REDUCER = 10.0f;
	public static final float HEAL_OVER_TIME_AGGRO_REDUCER = 0.5f;
	
	public static final String VAR_OBSERVER_BASE = "performance";
	public static final String VAR_OBSERVER_LIST = "performance.observer_ids";
	public static final String VAR_OBSERVER_BOX = "performance.observer_listbox_pId";
	public static final String VAR_OBSERVER_SKILLCHECK = "performance.skill_check";
	public static final String VAR_PERFORMANCE_HAS_DANCE_BUFF = "performance.has_dance_buff";
	public static final String VAR_PERFORMANCE_HAS_MUSIC_BUFF = "performance.has_music_buff";
	
	public static final String VAR_HEALING_CAN_HEALDAMAGE = "healing.can_heal_damage";
	public static final String VAR_HEALING_CAN_HEALWOUND = "healing.can_heal_wound";
	
	public static final String VAR_HEALING_RANGE = "healing.range";
	public static final String VAR_HEALING_AREA = "healing.area";
	public static final String VAR_HEALING_STATE = "healing.state";
	public static final String VAR_HEALING_CURE_DOT = "healing.cure_dot";
	public static final String VAR_HEALING_APPLY_DOT = "healing.apply_dot";
	public static final String VAR_HEALING_DOT_POWER = "healing.dot_power";
	public static final String VAR_HEALING_DOT_POTENCY = "healing.dot_potency";
	public static final String VAR_HEALING_DOT_ID = "healing.dot_id";
	public static final String VAR_HEALING_DOT_ATTRIBUTE = "healing.dot_attribute";
	public static final String VAR_HEALING_DOT_DURATION = "healing.dot_duration";
	public static final String VAR_HEALING_ABSORPTION = "healing.absorption";
	
	public static final String SCRIPT_VAR_MEDICINE_BUFF = "healing.medicine_buff";
	public static final String SCRIPT_VAR_MEDICINE_REBUFF = "healing.medicine_rebuff";
	public static final String SCRIPT_VAR_DANCE_MIND_BUFF = "healing.dance_mind_buff";
	public static final String SCRIPT_VAR_DANCE_MIND_BUFF_TIME = "healing.dance_mind_buff_time";
	public static final String SCRIPT_VAR_MUSIC_MIND_BUFF = "healing.music_mind_buff";
	public static final String SCRIPT_VAR_MUSIC_MIND_BUFF_TIME = "healing.music_mind_buff_time";
	public static final String SCRIPT_VAR_RANGED_MED = "healing.ranged_med";
	public static final String SCRIPT_VAR_DAMAGE_MED = "healing.damage_med";
	
	public static final String LISTBOX_LIST = "lstList";
	public static final String LISTBOX_DATA = "dataList";
	
	public static final string_id UI_OBSERVE = new string_id("ui","observe");
	public static final string_id UI_STOP_OBSERVING = new string_id("ui","stop_observing");
	
	public static final String HEAL_TYPE_MEDICAL_DAMAGE = "medical_damage";
	public static final String HEAL_TYPE_MEDICAL_WOUND = "medical_wound";
	public static final String HEAL_TYPE_MEDICAL_STATE = "medical_state";
	public static final String HEAL_TYPE_MEDICAL_BUFF = "medical_buff";
	public static final String HEAL_TYPE_MEDICAL_REVIVE = "medical_revive";
	public static final String HEAL_TYPE_MEDICAL_FIRSTAID = "medical_firstaid";
	public static final String HEAL_TYPE_MEDICAL_CURE_POISON = "medical_cure_poison";
	public static final String HEAL_TYPE_MEDICAL_CURE_DISEASE = "medical_cure_disease";
	public static final String HEAL_TYPE_MEDICAL_CURE_FIRE = "medical_cure_fire";
	public static final String HEAL_TYPE_MEDICAL_APPLY_POISON = "medical_apply_poison";
	public static final String HEAL_TYPE_MEDICAL_APPLY_DISEASE = "medical_apply_disease";
	public static final String HEAL_TYPE_MEDICAL_QUICK_HEAL = "medical_quick_heal";
	public static final String HEAL_TYPE_MEDICAL_TEND_WOUND = "medical_tend_wound";
	public static final String HEAL_TYPE_MEDICAL_TEND_DAMAGE = "medical_tend_damage";
	public static final String HEAL_TYPE_MEDICAL_HEAL_MIND = "medical_heal_mind";
	public static final String HEAL_TYPE_DANCE_WOUND = "dance_wound";
	public static final String HEAL_TYPE_DANCE_SHOCK = "dance_shock";
	public static final String HEAL_TYPE_DANCE_MIND_BUFF = "dance_mind_buff";
	public static final String HEAL_TYPE_MUSIC_WOUND = "music_wound";
	public static final String HEAL_TYPE_MUSIC_SHOCK = "music_shock";
	public static final String HEAL_TYPE_MUSIC_MIND_BUFF = "music_mind_buff";
	
	public static final String HEAL_TYPE_JUGGLE_WOUND = "juggle_wound";
	public static final String HEAL_TYPE_JUGGLE_SHOCK = "juggle_shock";
	public static final String HEAL_TYPE_JUGGLE_MIND_BUFF = "juggle_mind_buff";
	
	public static final String SCRIPT_DANCE = "systems.healing.healing_dance";
	public static final String SCRIPT_MUSIC = "systems.healing.healing_music";
	public static final String SCRIPT_JUGGLE = "systems.healing.healing_juggle";
	public static final String SCRIPT_MEDIC = "systems.healing.healing_medic";
	public static final String SCRIPT_XENOBIOLOGY = "systems.healing.healing_xenobiology";
	
	public static final string_id SID_NOT_ENOUGH_MIND = new string_id("healing", "not_enough_mind");
	public static final string_id SID_APPLY_POISON_SELF = new string_id("healing", "apply_poison_self");
	public static final string_id SID_APPLY_POISON_OTHER = new string_id("healing", "apply_poison_other");
	public static final string_id SID_APPLY_DISEASE_SELF = new string_id("healing", "apply_disease_self");
	public static final string_id SID_APPLY_DISEASE_OTHER = new string_id("healing", "apply_disease_other");
	public static final string_id SID_RESIST_DOT_OTHER = new string_id("healing", "dot_resist_other");
	public static final string_id SID_MIND_TOO_DRAINED = new string_id("healing", "mind_too_drained");
	public static final string_id SID_YOU_APPLY_FIRST_AID = new string_id("healing", "you_apply_first_aid");
	public static final string_id SID_APPLIES_FIRST_AID = new string_id("healing", "applies_first_aid");
	public static final string_id SID_APPLY_FIRST_AID_SELF = new string_id("healing", "apply_first_aid_self");
	public static final string_id SID_APPLY_POISON_ANTIDOTE = new string_id("healing", "apply_poison_antidote");
	public static final string_id SID_APPLIES_POISON_ANTIDOTE = new string_id("healing", "applies_poison_antidote");
	public static final string_id SID_APPLY_POISON_ANTIDOTE_SELF = new string_id("healing", "apply_poison_antidote_self");
	public static final string_id SID_ATTEMPT_SUPPRESS_FLAMES = new string_id("healing", "attempt_suppress_flames");
	public static final string_id SID_COVERS_YOU_BLANKET = new string_id("healing", "covers_you_blanket");
	public static final string_id SID_COVERS_BLANKET_SELF = new string_id("healing", "cover_blanket_self");
	public static final string_id SID_APPLY_DISEASE_ANTIDOTE = new string_id("healing", "apply_disease_antidote");
	public static final string_id SID_APPLIES_DISEASE_ANTIDOTE = new string_id("healing", "applies_disease_antidote");
	public static final string_id SID_APPLY_DISEASE_ANTIDOTE_SELF = new string_id("healing", "apply_disease_antidote_self");
	public static final string_id SID_CANNOT_RESUS_WITHOUT_KIT = new string_id("healing", "cannot_resus_without_kit");
	public static final string_id SID_CANNOT_RESUS_WITHOUT_TARGET = new string_id("healing", "cannot_resus_without_target");
	public static final string_id SID_CANNOT_RESUS_WITHOUT_PACK = new string_id("healing", "cannot_resus_without_pack");
	public static final string_id SID_TARGET_DEAD_TOO_LONG = new string_id("healing", "target_dead_too_long");
	public static final string_id SID_MUST_HAVE_JEDI_CONSENT = new string_id("healing", "must_have_jedi_consent");
	public static final string_id SID_GROUP_OR_CONSENT_FROM_TARGET = new string_id("healing", "group_or_consent_from_target");
	public static final string_id SID_TEND_WOUNDS_INVALID_TARGET = new string_id("healing", "tend_wounds_invalid_target");
	public static final string_id SID_CANNOT_DO_THAT_TIME = new string_id("healing", "cannot_do_that_time");
	public static final string_id SID_YOU_MUST_WAIT = new string_id("healing", "you_must_wait");
	public static final string_id SID_CANT_HEAL_DAMAGE_OR_STATES = new string_id("healing", "cant_heal_damage_or_states");
	public static final string_id SID_MUST_BE_IN_HOSPITAL_OR_CS = new string_id("healing", "must_be_in_hospital_or_cs");
	public static final string_id SID_MUST_BE_IN_HOSP_OR_CS_OR_DROID = new string_id("healing", "must_be_in_hosp_or_cs_or_droid");
	public static final string_id SID_MUST_WAIT_TO_HEAL_OR_ENHANCE = new string_id("healing", "must_wait_to_heal_or_enhance");
	public static final string_id SID_NO_HEAL_OR_ENHANCE_ABILITY = new string_id("healing", "no_heal_or_enchance_ability");
	public static final string_id SID_NO_WOUNDS_OF_TYPE_SELF = new string_id("healing", "no_wounds_of_type_self");
	public static final string_id SID_NO_WOUNDS_OF_TYPE_TARGET = new string_id("healing", "no_wounds_of_type_target");
	public static final string_id SID_REMOVE_STATE_FROM_TARGET = new string_id("healing", "remove_state_from_target");
	public static final string_id SID_REMOVE_STATE_FROM_YOU = new string_id("healing", "remove_state_from_you");
	public static final string_id SID_REMOVE_STATE_SELF = new string_id("healing", "remove_state_self");
	public static final string_id SID_YOU_ENHANCE_YOUR_ATT = new string_id("healing", "you_enhance_your_att");
	public static final string_id SID_YOU_ENHANCE_TARGETS_ATTRIBUTE = new string_id("healing", "you_enchance_targets_attribute");
	public static final string_id SID_MEDIC_ENHANCES_YOUR_ATTRIBUTE = new string_id("healing", "medic_enchances_your_attribute");
	public static final string_id SID_REAPPLY_ENHANCEMENT_TARGET = new string_id("healing", "reapply_enhancement_target");
	public static final string_id SID_REAPPLIES_ENHANCEMENT_YOU = new string_id("healing", "reapplies_enhancement_you");
	public static final string_id SID_REAPPLY_ENHANCEMENT_SELF = new string_id("healing", "reapply_enhancement_self");
	public static final string_id SID_ENHANCE_ATTRIBUTE_BY_SELF = new string_id("healing", "enhance_attribute_by_self");
	public static final string_id SID_PVP_NO_HELP = new string_id("healing", "pvp_no_help");
	public static final string_id SID_HEAL_ATTRIBUTE_TARGET = new string_id("healing", "heal_attribute_target");
	public static final string_id SID_ATTRIBUTE_HEALED = new string_id("healing", "attribute_healed");
	public static final string_id SID_HEAL_ATTRIB_SELF = new string_id("healing", "heal_attrib_self");
	
	public static final string_id SID_DROID_REPAIR_WOUND_SELF = new string_id("healing", "droid_repair_wound_self");
	public static final string_id SID_DROID_REPAIR_WOUND_OTHER = new string_id("healing", "droid_repair_wound_other");
	public static final string_id SID_DROID_REPAIR_DAMAGE_SELF = new string_id("healing", "droid_repair_damage_self");
	public static final string_id SID_DROID_REPAIR_DAMAGE_OTHER = new string_id("healing", "droid_repair_damage_other");
	
	public static final String RESPONSE_TEXT = "datatables/healing/healing_response.iff";
	public static final String PP_FILE_LOC = "healing_response";
	
	public static final int[] COMBAT_STATES =
	{
		STATE_DIZZY,
		STATE_BLINDED,
		STATE_STUNNED,
		STATE_INTIMIDATED
	};
	
	public static final string_id[] SID_PERFORM_CURE_STATE_SUCCESS =
	{
		new string_id("healing", "perform_cure_state_dizzy_success"),
		new string_id("healing", "perform_cure_state_blinded_success"),
		new string_id("healing", "perform_cure_state_stunned_success"),
		new string_id("healing", "perform_cure_state_intimidated_success")
	};
	public static final String[] ATTRIBUTES = 
	{
		"HEALTH",
		"CONSTITUTION",
		"ACTION",
		"STAMINA",
		"MIND",
		"WILLPOWER"
	};
	
	
	public static boolean performHealDamage(attacker_data attackerData, defender_data[] defenderData, combat_data action_data) throws InterruptedException
	{
		
		LOG("healing_hate","performHealDamage init");
		
		if ((action_data.dotIntensity > 0) && (action_data.dotDuration > 0))
		{
			return performHealOverTime(attackerData, defenderData, action_data);
		}
		
		obj_id medic = attackerData.id;
		int toHeal = action_data.addedDamage;
		int totalDelta = 0;
		
		boolean total_success = false;
		
		toHeal = getExpertiseModifiedHealing(medic, toHeal, action_data);
		
		String actionName = action_data.actionName;
		boolean useTempParticle = false;
		boolean playParticleOnMedic = false;
		
		if (actionName.startsWith("me_bacta_bomb") || actionName.startsWith("me_bacta_grenade"))
		{
			useTempParticle = true;
		}
		
		if (actionName.startsWith("sp_cloaked_recovery"))
		{
			playParticleOnMedic = true;
		}
		
		for (int i = 0; i < defenderData.length; i++)
		{
			testAbortScript();
			if ((!isIdValid(defenderData[i].id) || !exists(defenderData[i].id) || !isMob(defenderData[i].id) || isDead(defenderData[i].id)))
			{
				continue;
			}
			
			if (!isValidHealTarget(medic, defenderData[i].id))
			{
				continue;
			}
			
			toHeal = getHealingAfterReductions(medic, defenderData[i].id, toHeal);
			toHeal = getTargetHealingBonus(medic, defenderData[i].id, toHeal);
			
			if (luck.isLucky(defenderData[i].id, 0.005f))
			{
				float bonus = toHeal * 0.2f;
				if (bonus < 1)
				{
					bonus = 1;
				}
				
				toHeal += bonus;
			}
			
			int hBefore = getAttrib(defenderData[i].id, action_data.attribute);
			boolean success = healDamage(defenderData[i].id, action_data.attribute, toHeal);
			
			if (success)
			{
				int delta = getAttrib(defenderData[i].id, action_data.attribute) - hBefore;
				
				if (delta <= 0)
				{
					continue;
				}
				
				totalDelta += delta;
				
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_PERFORM_HEAL_DAMAGE_SUCCESS);
				pp = prose.setTT(pp, medic);
				pp = prose.setTO(pp, defenderData[i].id);
				pp = prose.setDI(pp, delta);
				sendMedicalSpam(medic, defenderData[i].id, pp, true, true, true, COMBAT_RESULT_MEDICAL);
				
				pp = prose.setStringId(pp, new string_id("healing", "heal_fly"));
				pp = prose.setDI(pp, delta);
				pp = prose.setTO(pp, ATTRIBUTES[action_data.attribute]);
				showFlyTextPrivateProseWithFlags(defenderData[i].id, defenderData[i].id, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				
				if (medic != defenderData[i].id)
				{
					showFlyTextPrivateProseWithFlags(defenderData[i].id, medic, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				}
				
				float hateMod = action_data.hateDamageModifier;
				
				int healingAgroMod = getEnhancedSkillStatisticModifierUncapped(medic, "expertise_agro_healing");
				float agroReductionFact = 1.0f - ((float)healingAgroMod/100.0f);
				float modifiedHate = (delta * agroReductionFact) /HEALING_AGGRO_REDUCER;
				
				if (isPlayer(medic))
				{
					_addMedicalHate(medic, defenderData[i].id, (int)modifiedHate, hateMod);
				}
				
				pvpHelpPerformed(medic, defenderData[i].id);
				
				applyDefenderHealBuffs(medic, defenderData[i].id, action_data);
				applyMedicHealBuffs(medic, action_data);
				location loc = getLocation(defenderData[i].id);
				
				if (useTempParticle)
				{
					obj_id[] players = getAllPlayers(loc, 64);
					if (players != null)
					{
						for (int j = 0; j < players.length; j++)
						{
							testAbortScript();
							if (stealth.hasInvisibleBuff(players[j]))
							{
								continue;
							}
							
							if (actionName.startsWith("me_bacta_bomb"))
							{
								playClientEffectLoc(players[j], "clienteffect/bacta_bomb.cef", loc, 0);
							}
							
							if (actionName.startsWith("me_bacta_grenade"))
							{
								playClientEffectLoc(players[j], "clienteffect/bacta_grenade.cef", loc, 0);
							}
						}
					}
				}
				else if (playParticleOnMedic)
				{
					if (!stealth.hasInvisibleBuff(medic))
					{
						playClientEffectObj(medic, "appearance/pt_heal.prt", medic, "");
					}
				}
				else
				{
					if (!stealth.hasInvisibleBuff(defenderData[i].id))
					{
						playHealDamageEffect(loc);
					}
				}
				
				if (delta == 0 && actionName.indexOf("_sh_") > -1)
				{
					CustomerServiceLog("Heal-Fail", "%TU received a success from heal damage but healed for a delta of 0 on heal: "+actionName, medic);
				}
			}
			
			total_success |= success;
		}
		
		if (totalDelta <= 0)
		{
			return false;
		}
		
		pvp.bfCreditForHealing(medic, totalDelta);
		
		return total_success;
	}
	
	
	public static boolean applyDefenderHealBuffs(obj_id medic, obj_id defender, combat_data actionData) throws InterruptedException
	{
		String actionName = actionData.actionName;
		String buffName = actionData.buffNameTarget;
		float buffStrength = actionData.buffStrengthTarget;
		float buffDuration = actionData.buffDurationTarget;
		
		if (buffName == null || buffName.equals(""))
		{
			return false;
		}
		
		if (!combat.expertiseRandomBuffChance(medic, actionData))
		{
			return false;
		}
		
		if (!isValidHealTarget(defender))
		{
			return false;
		}
		
		buffDuration = combat.getExpertiseBuffDurationMods(medic, actionData, buffName, buffDuration);
		
		combat.combatLog(medic, defender, "applyDefenderHealBuffs", "Applying defender buff - "+ buffName);
		
		return buff.applyBuff(defender, buffName, buffDuration, buffStrength);
	}
	
	
	public static boolean applyMedicHealBuffs(obj_id medic, combat_data actionData) throws InterruptedException
	{
		String actionName = actionData.actionName;
		String buffName = actionData.buffNameSelf;
		float buffStrength = actionData.buffStrengthSelf;
		float buffDuration = actionData.buffDurationSelf;
		
		if (buffName == null || buffName.equals(""))
		{
			return false;
		}
		
		if (!combat.expertiseRandomBuffChance(medic, actionData))
		{
			return false;
		}
		
		buffDuration = combat.getExpertiseBuffDurationMods(medic, actionData, buffName, buffDuration);
		
		combat.combatLog(medic, null, "applyMedicHealBuffs", "Applying medic buff - "+ buffName);
		
		return buff.applyBuff(medic, buffName, buffDuration, buffStrength);
	}
	
	
	public static boolean performHealOverTime(attacker_data attackerData, defender_data[] defenderData, combat_data action_data) throws InterruptedException
	{
		obj_id medic = attackerData.id;
		int maxHeal = action_data.addedDamage;
		int duration = action_data.dotDuration;
		int perTick = action_data.dotIntensity;
		
		String specialLine = action_data.specialLine;
		duration += getEnhancedSkillStatisticModifierUncapped(medic, "expertise_hot_duration_"+ specialLine);
		
		perTick = getExpertiseModifiedHealing(medic, perTick, action_data);
		maxHeal = getExpertiseModifiedHealing(medic, maxHeal, action_data);
		
		int tickLength = (int)(((float)duration) / (((float)maxHeal) / ((float)perTick)));
		
		for (int i = 0; i < defenderData.length; i++)
		{
			testAbortScript();
			if (isDead(defenderData[i].id) || !isValidHealTarget(defenderData[i].id))
			{
				continue;
			}
			
			perTick = getHealingAfterReductions(medic, defenderData[i].id, perTick);
			perTick = getTargetHealingBonus(medic, defenderData[i].id, perTick);
			
			startHealOverTime(medic, defenderData[i].id, action_data.actionName, duration, tickLength, perTick, true);
			
			pvpHelpPerformed(medic, defenderData[i].id);
			location loc = getLocation(defenderData[i].id);
			playHealDamageEffect(loc);
		}
		
		return true;
	}
	
	
	public static int getExpertiseModifiedHealing(obj_id medic, int toHeal, combat_data action_data) throws InterruptedException
	{
		String specialLine = action_data.specialLine;
		float expertiseHealingBonus = 0.0f;
		expertiseHealingBonus += getEnhancedSkillStatisticModifierUncapped(medic, "expertise_healing_all");
		
		if (specialLine != null && !specialLine.equals(""))
		{
			expertiseHealingBonus += getEnhancedSkillStatisticModifierUncapped(medic, "expertise_healing_line_"+ specialLine);
		}
		
		float toHealFloat = toHeal;
		toHealFloat = toHealFloat * (1 + (expertiseHealingBonus/100) );
		toHeal = (int)toHealFloat;
		
		return toHeal;
	}
	
	
	public static int getHealingAfterReductions(obj_id medic, obj_id target, int toHeal) throws InterruptedException
	{
		int healingReduction = getEnhancedSkillStatisticModifierUncapped(target, "expertise_healing_reduction");
		float redux = (float)healingReduction / ((float)healingReduction + 50.0f);
		toHeal = (int)((float)toHeal - ((float)toHeal * redux));
		
		return toHeal;
	}
	
	
	public static int getTargetHealingBonus(obj_id medic, obj_id target, int toHeal) throws InterruptedException
	{
		int healingBonus = getEnhancedSkillStatisticModifierUncapped(target, "expertise_target_healing_bonus");
		toHeal = (int)((float)toHeal * (1.0f + ((float)healingBonus / 100.0f)));
		
		return toHeal;
	}
	
	
	public static boolean canUseAbility(obj_id medic, combat_data actionData) throws InterruptedException
	{
		return true;
	}
	
	
	public static boolean useHealDamageItem(obj_id user, obj_id item) throws InterruptedException
	{
		return useHealDamageItem(user, user, item);
	}
	
	
	public static boolean useHealDamageItem(obj_id user, obj_id item, int attrib) throws InterruptedException
	{
		return useHealDamageItem(user, user, item, attrib);
	}
	
	
	public static boolean useHealDamageItem(obj_id user, obj_id target, obj_id item) throws InterruptedException
	{
		return useHealDamageItem(user, target, item, HEALTH);
	}
	
	
	public static boolean useHealDamageItem(obj_id user, obj_id target, obj_id item, int attrib) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(user))
		{
			return false;
		}
		
		if (!isIdValid(item))
		{
			return false;
		}
		
		if (!isDamaged(target, attrib))
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("healing", "no_damage_fly"));
			
			showFlyTextPrivateProseWithFlags(user, user, pp, 1.5f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
			sendMedicalSpam(user, SID_ITEM_NO_DAMAGE, COMBAT_RESULT_MEDICAL);
			
			return false;
		}
		
		if (!isValidHealTarget(target))
		{
			return false;
		}
		
		if (utils.hasScriptVar(target, VAR_ITEM_TIMER))
		{
			int reuseTime = utils.getIntScriptVar(target, VAR_ITEM_TIMER);
			
			if (reuseTime > getGameTime())
			{
				int time_left = reuseTime - getGameTime();
				
				prose_package ppFly = new prose_package();
				ppFly = prose.setStringId(ppFly, new string_id("healing", "no_damage_fly"));
				
				showFlyTextPrivateProseWithFlags(user, user, ppFly, 1.5f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_ITEM_TOO_SOON);
				pp = prose.setDI(pp, time_left);
				sendMedicalSpam(user, pp, COMBAT_RESULT_MEDICAL);
				
				return false;
				
			}
			
			utils.removeScriptVar(target, VAR_ITEM_TIMER);
		}
		
		if (!hasObjVar(item, VAR_ITEM_POWER))
		{
			return false;
		}
		
		int toHeal = getIntObjVar(item, VAR_ITEM_POWER);
		
		if (luck.isLucky(target, 0.005f))
		{
			float bonus = toHeal * 0.2f;
			if (bonus < 1)
			{
				bonus = 1;
			}
			
			toHeal += bonus;
		}
		
		int delta = healDamage(user, target, attrib, toHeal);
		
		if (delta > 0)
		{
			decrementCount(item);
			
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("healing", "heal_fly"));
			pp = prose.setDI(pp, delta);
			pp = prose.setTO(pp, ATTRIBUTES[attrib]);
			showFlyTextPrivateProseWithFlags(user, user, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
			
			pp = prose.setStringId(pp, SID_ITEM_PLAYER_HEAL);
			pp = prose.setDI(pp, delta);
			pp = prose.setTT(pp, target);
			sendMedicalSpam(user, pp, COMBAT_RESULT_MEDICAL);
			
			int reuseTime = VAR_ITEM_REUSE_TIME;
			utils.setScriptVar(target, VAR_ITEM_TIMER, (getGameTime() + (reuseTime)));
			
			sendCooldownGroupTimingOnly(user, (1824933302), reuseTime);
			
			doHealingAnimationAndEffect(user, target);
		}
		
		return (delta > 0);
	}
	
	
	public static boolean useChannelHealItem(obj_id user, obj_id item) throws InterruptedException
	{
		return useChannelHealItem(user, item, HEALTH);
	}
	
	
	public static boolean useChannelHealItem(obj_id user, obj_id item, int attrib) throws InterruptedException
	{
		if (!isIdValid(user))
		{
			return false;
		}
		
		if (!isIdValid(item))
		{
			return false;
		}
		
		if (!isDamaged(user, attrib))
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("healing", "no_damage_fly"));
			
			showFlyTextPrivateProseWithFlags(user, user, pp, 1.5f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
			sendMedicalSpam(user, SID_ITEM_NO_DAMAGE, COMBAT_RESULT_MEDICAL);
			
			return false;
		}
		
		if (!hasObjVar(item, VAR_ITEM_POWER))
		{
			return false;
		}
		
		int toHeal = getIntObjVar(item, VAR_ITEM_POWER);
		
		int healPerTick = toHeal / ITEM_CHANNEL_HEAL_TICKS;
		
		int tick = 0;
		
		location currentLoc = getLocation(user);
		
		dictionary dict = new dictionary();
		dict.put("user", user);
		dict.put("tick", tick);
		dict.put("item", item);
		dict.put("healPerTick", healPerTick);
		dict.put("healLoc", currentLoc);
		dict.put("attrib", attrib);
		
		messageTo( user, "channelHeal", dict, 0, false);
		
		buff.applyBuff(user, "recent_heal");
		
		decrementCount(item);
		return true;
	}
	
	
	public static boolean useHealPetItem(obj_id user, obj_id pet, obj_id item) throws InterruptedException
	{
		if (!isIdValid(user))
		{
			return false;
		}
		
		if (!isIdValid(pet))
		{
			return false;
		}
		
		if (!isIdValid(item))
		{
			return false;
		}
		
		if (!pet_lib.isCreaturePet(pet))
		{
			sendMedicalSpam(user, SID_ITEM_NOT_A_PET, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		obj_id master = getMaster(pet);
		if (user != master)
		{
			sendMedicalSpam(user, SID_ITEM_NOT_YOUR_PET, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!isDamaged(pet))
		{
			sendMedicalSpam(user, SID_ITEM_NO_DAMAGE, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!isValidHealTarget(pet))
		{
			return false;
		}
		
		if (utils.hasScriptVar(user, VAR_ITEM_TIMER))
		{
			int reuseTime = utils.getIntScriptVar(user, VAR_ITEM_TIMER);
			
			if (reuseTime > getGameTime())
			{
				
				int time_left = reuseTime - getGameTime();
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_ITEM_TOO_SOON);
				pp = prose.setDI(pp, time_left);
				sendMedicalSpam(user, pp, COMBAT_RESULT_MEDICAL);
				return false;
				
			}
			
			utils.removeScriptVar(user, VAR_ITEM_TIMER);
		}
		
		if (!hasObjVar(item, VAR_ITEM_POWER))
		{
			return false;
		}
		
		int power = getIntObjVar(item, VAR_ITEM_POWER);
		
		int delta = healDamage(user, pet, HEALTH, power);
		
		if (delta > 0)
		{
			decrementCount(item);
			
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, SID_ITEM_PET_HEAL);
			pp = prose.setTT(pp, getEncodedName(pet));
			pp = prose.setDI(pp, delta);
			sendMedicalSpam(user, pp, COMBAT_RESULT_MEDICAL);
			
			float reuseTime = VAR_ITEM_REUSE_TIME;
			utils.setScriptVar(user, VAR_ITEM_TIMER, (getGameTime() + (reuseTime)));
			
			sendCooldownGroupTimingOnly(user, (-360620575), reuseTime);
			
			doHealingAnimationAndEffect(user, pet);
		}
		
		return (delta > 0);
	}
	
	
	public static boolean canDoBeastHeal(obj_id user, obj_id pet) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(user);
		
		if (!isIdValid(user) || !exists(user) || isIdNull(user))
		{
			return false;
		}
		
		if (!isIdValid(pet) || !exists(pet) || isIdNull(pet))
		{
			return false;
		}
		
		if (!isIdValid(beast) || !exists(beast) || isIdNull(beast))
		{
			return false;
		}
		
		if (beast != pet)
		{
			sendSystemMessage(user, new string_id("spam", "not_your_beast") );
			return false;
		}
		
		obj_id master = getMaster(pet);
		
		if (user != master)
		{
			sendSystemMessage(user, new string_id("spam", "not_your_beast") );
			return false;
		}
		
		if (!isDamaged(pet))
		{
			sendMedicalSpam(user, SID_ITEM_NO_DAMAGE, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!isValidHealTarget(pet))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean doBeastHeal(obj_id user, obj_id pet, int toHeal) throws InterruptedException
	{
		if (!canDoBeastHeal(user, pet))
		{
			return false;
		}
		
		int delta = healDamage(user, pet, HEALTH, toHeal);
		
		_addMedicalHate(user, pet, toHeal / 10);
		
		if (delta > 0)
		{
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, SID_BEAST_MASTER_HEAL);
			pp = prose.setTT(pp, getEncodedName(pet));
			pp = prose.setDI(pp, delta);
			sendMedicalSpam(user, pp, COMBAT_RESULT_MEDICAL);
			doHealingAnimationAndEffect(user, pet);
		}
		
		return (delta > 0);
	}
	
	
	public static boolean performRevivePlayer(attacker_data attackerData, defender_data[] defenderData, combat_data actionData, boolean weak) throws InterruptedException
	{
		obj_id medic = attackerData.id;
		obj_id[] targets = new obj_id[defenderData.length];
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			targets[i] = defenderData[i].id;
		}
		
		return _performRevivePlayer(medic, targets, null, actionData, weak);
	}
	
	
	public static boolean _performRevivePlayer(obj_id medic, obj_id[] targets, obj_id enhancer, combat_data action_data, boolean weak) throws InterruptedException
	{
		boolean hasRevivedPlayer = false;
		
		for (int i = 0; i < targets.length; i++)
		{
			testAbortScript();
			if (!isPlayer(targets[i]))
			{
				continue;
			}
			
			if (!hasObjVar(targets[i], pclib.VAR_BEEN_COUPDEGRACED))
			{
				continue;
			}
			
			if (!utils.hasScriptVar(targets[i], "pvp_death") && action_data.actionName.equals("me_rv_pvp_area"))
			{
				continue;
			}
			
			int stamp = getIntObjVar(targets[i], pclib.VAR_DEATHBLOW_STAMP);
			if (getGameTime() > (stamp + REVIVE_TIMER))
			{
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_PERFORM_REVIVE_TOO_LONG);
				pp = prose.setTT(pp, medic);
				pp = prose.setTO(pp, targets[i]);
				sendMedicalSpam(medic, targets[i], pp, true, true, false, COMBAT_RESULT_MEDICAL);
				continue;
			}
			
			if (!pvpCanHelp(medic, targets[i]))
			{
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("spam", "revive_no_help_pvp") );
				pp = prose.setTO(pp, targets[i]);
				sendSystemMessageProse(medic, pp);
				continue;
			}
			
			dictionary params = new dictionary();
			params.put("medic", medic);
			if (weak)
			{
				params.put("weak", 1);
			}
			else
			{
				params.put("weak", 0);
			}
			
			pvpHelpPerformed(medic, targets[i]);
			hasRevivedPlayer = true;
			messageTo(targets[i], "showReviveSUI", params, 0, false);
			
			prose_package reviveAttemptPP = new prose_package();
			reviveAttemptPP = prose.setStringId(reviveAttemptPP, new string_id("spam", "revive_attempt") );
			reviveAttemptPP = prose.setTO(reviveAttemptPP, targets[i]);
			sendSystemMessageProse(medic, reviveAttemptPP);
		}
		
		return hasRevivedPlayer;
	}
	
	
	public static void _addMedicalHate(obj_id medic, obj_id target, int hate) throws InterruptedException
	{
		_addMedicalHate(medic, target, hate, 1.0f);
	}
	
	
	public static void _addMedicalHate(obj_id medic, obj_id target, int hate, float hateMod) throws InterruptedException
	{
		LOG("healing_hate","_addMedicalHate init. Target: "+target);
		obj_id[] hateList = getHateList(target);
		
		LOG("healing_hate","_addMedicalHate hateList.length: "+hateList.length);
		LOG("healing_hate","_addMedicalHate hateList[0]: "+hateList[0]);
		
		obj_id[] objHateList = getHateList(hateList[0]);
		LOG("healing_hate","_addMedicalHate obj 0 hateList.length: "+objHateList.length);
		
		if (hateList == null || hateList.length == 0)
		{
			return;
		}
		
		if (hateMod < 0.0f)
		{
			hateMod = 0.0f;
		}
		
		float hateValue = (float)hate;
		hateValue *= hateMod;
		
		LOG("healing_hate","_addMedicalHate hateValue: "+hateValue);
		
		for (int i = 0; i < hateList.length; i++)
		{
			testAbortScript();
			if (!isIdValid(hateList[i]))
			{
				continue;
			}
			
			if (medic == hateList[i])
			{
				continue;
			}
			
			addHate(hateList[i], medic, hateValue);
			addHate(medic, hateList[i], 0.0f);
		}
	}
	
	
	public static float _getEnhancerModifier(obj_id enhancer, float cap) throws InterruptedException
	{
		if (!isIdValid(enhancer))
		{
			return 0.0f;
		}
		
		if (cap <= 0.0f)
		{
			cap = 1.0f;
		}
		
		float enhancement = 0.0f;
		
		if (hasObjVar(enhancer, VAR_ENHANCER_EFFICIENCY))
		{
			enhancement = getFloatObjVar(enhancer, VAR_ENHANCER_EFFICIENCY);
		}
		else
		{
			return 0.0f;
		}
		
		float mod = ((enhancement / 1000.0f) * cap);
		
		return mod;
	}
	
	
	public static float _getSkillModModifier(obj_id medic, String skill_mod, float coeff) throws InterruptedException
	{
		if (coeff <= 0.0f)
		{
			coeff = 1.0f;
		}
		
		float skill = (float)getEnhancedSkillStatisticModifier(medic, skill_mod);
		
		if (skill <= 0)
		{
			return 0.0f;
		}
		
		float mod = 1.0f - (1.0f * (1.0f / ((skill + (1.0f / coeff)) * coeff)));
		
		return mod;
	}
	
	
	public static float _getMedicalDroidModifier(obj_id medic) throws InterruptedException
	{
		float droidMod = 0.0f;
		
		if (pet_lib.isNearMedicalDroid(medic, VAR_SURGICAL_DROID_RANGE))
		{
			droidMod = 0.2f;
			droidMod *= pet_lib.getMedicalDroidMod(medic, VAR_SURGICAL_DROID_RANGE);
		}
		
		return droidMod;
	}
	
	
	public static float _getShockWoundModifier(obj_id target) throws InterruptedException
	{
		
		return 1.0f;
	}
	
	
	public static void _healingActionFollowUp(obj_id medic, obj_id target, int delta, int type) throws InterruptedException
	{
		int[] dArray = new int[1];
		dArray[0] = delta;
		
		if (isPlayer(target))
		{
			if (type == 1)
			{
				grantHealingExperience(dArray, medic, target, HEAL_TYPE_MEDICAL_DAMAGE);
			}
			else
			{
				grantHealingExperience(dArray, medic, target, HEAL_TYPE_MEDICAL_WOUND);
			}
			
			if (medic != target)
			{
				addHealingKillCredit(medic, target, dArray);
			}
		}
	}
	
	
	public static Vector getCombatStates(obj_id target) throws InterruptedException
	{
		Vector vStates = new Vector();
		
		return vStates;
	}
	
	
	public static boolean isDotted(obj_id target, String dot_type) throws InterruptedException
	{
		if (dot_type.equals(dot.DOT_BLEEDING))
		{
			return dot.isBleeding(target);
		}
		else if (dot_type.equals(dot.DOT_POISON))
		{
			return dot.isPoisoned(target);
		}
		else if (dot_type.equals(dot.DOT_DISEASE))
		{
			return dot.isDiseased(target);
		}
		else if (dot_type.equals(dot.DOT_FIRE))
		{
			return dot.isOnFire(target);
		}
		else if (dot_type.equals(dot.DOT_ACID))
		{
			return dot.isAcid(target);
		}
		else if (dot_type.equals(dot.DOT_ENERGY))
		{
			return dot.isEnergy(target);
		}
		
		return false;
	}
	
	
	public static void startHealOverTime(obj_id target, float duration, float tick, int healPerTick) throws InterruptedException
	{
		startHealOverTime(obj_id.NULL_ID, target, "", duration,tick, healPerTick, false);
	}
	
	
	public static void startHealOverTime(obj_id medic, obj_id target, String actionName, float duration, float tick, int healPerTick, boolean combatFollowUp) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		if (tick < 1.0f)
		{
			tick = 1.0f;
		}
		
		int hot_id = rand(-99999, 99999);
		utils.setScriptVar(target, VAR_PLAYER_HOT_ID, hot_id);
		
		int delta = healDamage(medic, target, HEALTH, healPerTick);
		
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, new string_id("healing", "heal_fly"));
		pp = prose.setDI(pp, delta);
		pp = prose.setTO(pp, ATTRIBUTES[HEALTH]);
		showFlyTextPrivateProseWithFlags(target, target, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
		
		if (medic != target)
		{
			showFlyTextPrivateProseWithFlags(target, medic, pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
		}
		
		if (!actionName.equals(""))
		{
			String ascii_id = "perform_"+ actionName + "_success";
			pp = prose.setStringId(pp, new string_id("healing", ascii_id));
			pp = prose.setTT(pp, medic);
			pp = prose.setTO(pp, target);
			sendMedicalSpam(medic, target, pp, true, true, true, COMBAT_RESULT_MEDICAL);
		}
		
		pp = prose.setStringId(pp, SID_PERFORM_HEAL_DAMAGE_SUCCESS);
		pp = prose.setTT(pp, medic);
		pp = prose.setTO(pp, target);
		pp = prose.setDI(pp, delta);
		healing.sendMedicalSpam(medic, target, pp, true, true, true, COMBAT_RESULT_MEDICAL);
		
		dictionary d = new dictionary();
		d.put("duration", duration);
		d.put("tick", tick);
		d.put("heal", healPerTick);
		d.put("id", hot_id);
		d.put("medic", medic);
		if (combatFollowUp)
		{
			d.put("combat", 1);
		}
		else
		{
			d.put("combat", 0);
		}
		
		messageTo(target, MSG_HEAL_OVER_TIME, d, tick, false);
		
		if (actionName.equals("trandoshan_ability_1"))
		{
			buff.applyBuff(target, "trandoshan_ability_1");
		}
		else
		{
			buff.applyBuff(target, "healOverTime");
		}
		
	}
	
	
	public static boolean isAwake(obj_id target) throws InterruptedException
	{
		if (isIncapacitated(target))
		{
			return false;
		}
		
		if (isDead(target))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean validateTarget(obj_id target, int targetCheck) throws InterruptedException
	{
		final int NONE = -1;
		final int STANDARD = 0;
		final int MOB = 1;
		final int CREATURE = 2;
		final int NPC = 3;
		final int DROID = 4;
		final int PVP = 5;
		final int JEDI = 6;
		final int DEAD = 7;
		
		if (targetCheck != DEAD)
		{
			if (isDead(target))
			{
				return false;
			}
		}
		
		switch (targetCheck)
		{
			case MOB:
			return isMob(target) && !vehicle.isVehicle(target);
			
			case CREATURE:
			return ai_lib.isMonster(target);
			
			case NPC:
			return ai_lib.isNpc(target);
			
			case DROID:
			return ai_lib.isDroid(target);
			
			case PVP:
			return pvpCanAttack(getSelf(), target);
			
			case JEDI:
			if (isPlayer(target))
			{
				return isJedi(target);
			}
			else
			{
				return jedi.isLightsaber(getCurrentWeapon(target));
			}
			
			case DEAD:
			return isDead(target);
		}
		
		return true;
	}
	
	
	public static boolean fullHeal(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || (!isPlayer(player)))
		{
			return false;
		}
		
		healShockWound(player, getShockWound(player));
		
		attribute[] maxAttribs = getUnmodifiedMaxAttribs(player);
		addAttribModifier(player, maxAttribs[HEALTH].getType(), maxAttribs[HEALTH].getValue(), 0, 0, MOD_POOL);
		
		return true;
	}
	
	
	public static boolean fullHealEveryone(obj_id creature) throws InterruptedException
	{
		if (!isIdValid(creature))
		{
			return false;
		}
		
		healShockWound(creature, getShockWound(creature));
		
		attribute[] maxAttribs = getUnmodifiedMaxAttribs(creature);
		addAttribModifier(creature, maxAttribs[HEALTH].getType(), maxAttribs[HEALTH].getValue(), 0, 0, MOD_POOL);
		
		return true;
		
	}
	
	
	public static boolean healDamage(obj_id player, int amt) throws InterruptedException
	{
		return healDamage(player, HEALTH, amt);
	}
	
	public static boolean healDamage(obj_id player, int attrib, int amt) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		return addAttribModifier(player, attrib, amt, 0, 0, MOD_POOL);
	}
	
	
	public static int healDamage(obj_id source, obj_id target, int attrib, int amount) throws InterruptedException
	{
		if (!isIdValid(source) || !isIdValid(target) || amount <= 0)
		{
			return 0;
		}
		
		int hBefore = getAttrib(target, attrib);
		
		addAttribModifier(target, attrib, amount, 0, 0, MOD_POOL);
		
		int delta = getAttrib(target, attrib) - hBefore;
		
		if (delta > 0)
		{
			pvp.bfCreditForHealing(source, delta);
		}
		
		return delta;
	}
	
	
	public static boolean healClone(obj_id player, boolean woundPlayer) throws InterruptedException
	{
		if (!isIdValid(player) || (!isPlayer(player)))
		{
			return false;
		}
		
		location loc = getLocation(player);
		boolean adventurePlanet = false;
		int sceneCRC = getStringCrc(loc.area);
		switch ( sceneCRC )
		{
			case (1462210849):
			case (1664678725):
			case (1666541635):
			case (1464468007):
			case (-1344817832):
			case (-108111930):
			case (1372916281):
			case (-1240008136):
			case (659093975) :
			case (-15432037) :
			case (1676106477) :
			case (1475883145) :
			case (-1029289085) :
			case (-111856842) :
			case (1468960591) :
			case (1410580269) :
			case (-1454454247) :
			case (-15259513) :
			case (-1186872851) :
			case (2130374697) :
			adventurePlanet = false;
			break;
			default:
			adventurePlanet = true;
			break;
		}
		
		float woundReduction = 0f;
		
		if (woundPlayer && !adventurePlanet)
		{
			if (utils.hasScriptVar( player, "buff.dessert_gorrnar.value" ))
			{
				
				float eff = utils.getFloatScriptVar(player, "buff.dessert_gorrnar.value");
				buff.removeBuff(player, "dessert_gorrnar");
				
				woundReduction = 1f - (eff / 100f);
			}
		}
		
		float fltThreshold = .70f;
		
		attribute[] maxAttribs = getUnmodifiedMaxAttribs(player);
		
		int attribValue = maxAttribs[HEALTH].getValue();
		float fltTest = (float)attribValue;
		fltTest *= fltThreshold;
		int intMaxWounds = (int)fltTest;
		
		if (intMaxWounds > 0)
		{
			setAttrib(player, HEALTH, attribValue);
		}
		
		if (woundPlayer)
		{
			if (getConfigSetting("GameServer", "enableCybernetics")!=null)
			{
				cybernetic.applyDeathCybernetic( player );
			}
			
		}
		
		return true;
	}
	
	
	public static boolean healClone(obj_id player) throws InterruptedException
	{
		return healClone(player, true);
	}
	
	
	public static void sendMedicalSpam(obj_id medic, string_id message, int spam_type) throws InterruptedException
	{
		sendCombatSpamMessage(medic, message, spam_type);
	}
	
	
	public static void sendMedicalSpam(obj_id medic, obj_id target, string_id message, boolean to_attacker, boolean to_defender, boolean to_others, int spam_type) throws InterruptedException
	{
		if (medic == target)
		{
			sendCombatSpamMessage(medic, target, message, to_attacker, false, to_others, spam_type);
		}
		else
		{
			sendCombatSpamMessage(medic, target, message, to_attacker, to_defender, to_others, spam_type);
		}
	}
	
	
	public static void sendMedicalSpam(obj_id medic, prose_package pp, int spam_type) throws InterruptedException
	{
		sendCombatSpamMessageProse(medic, pp, spam_type);
	}
	
	
	public static void sendMedicalSpam(obj_id medic, obj_id target, prose_package pp, boolean to_attacker, boolean to_defender, boolean to_others, int spam_type) throws InterruptedException
	{
		if (medic == target)
		{
			sendCombatSpamMessageProse(medic, target, pp, to_attacker, false, to_others, spam_type);
		}
		else
		{
			sendCombatSpamMessageProse(medic, target, pp, to_attacker, to_defender, to_others, spam_type);
		}
	}
	
	
	public static String[] assembleHealingMessage(int attrib, int delta, obj_id doctor, obj_id target, int heal_type) throws InterruptedException
	{
		int[] attribsArray = new int[1];
		int[] deltaArray = new int[1];
		
		attribsArray[0] = attrib;
		deltaArray[0] = delta;
		
		return assembleHealingMessage(attribsArray, deltaArray, doctor, target, null, heal_type);
	}
	
	
	public static String[] assembleHealingMessage(int[] attribsArray, int[] deltaArray, obj_id doctor, obj_id target, obj_id medicine) throws InterruptedException
	{
		return assembleHealingMessage(attribsArray, deltaArray, doctor, target, medicine, 1);
	}
	
	
	public static String[] assembleHealingMessage(int[] attribsArray, int[] deltaArray, obj_id doctor, obj_id target, obj_id medicine, int heal_type) throws InterruptedException
	{
		Vector attribs = new Vector();
		if (attribs != null)
		{
			attribs.setSize(attribsArray.length);
			for (int _i = 0; _i < attribsArray.length; ++_i)
			{
				attribs.set(_i, new Integer(attribsArray[_i]));
			}
		}
		Vector delta = new Vector();
		if (delta != null)
		{
			delta.setSize(deltaArray.length);
			for (int _i = 0; _i < deltaArray.length; ++_i)
			{
				delta.set(_i, new Integer(deltaArray[_i]));
			}
		}
		
		if (attribs.size() != delta.size())
		{
			return null;
		}
		
		for (int i = 0; i < delta.size(); i++)
		{
			testAbortScript();
			if (((Integer)(delta.get(i))).intValue() < 1)
			{
				attribs = utils.removeElementAt(attribs, i);
				delta = utils.removeElementAt(delta, i);
				i--;
			}
		}
		
		if (attribs.size() < 1)
		{
			return null;
		}
		
		if (target == doctor)
		{
			target = null;
		}
		
		int responseType = 0;
		int attribDecWise;
		
		if (isIdValid(medicine))
		{
			if (!isHealDamageMedicine(medicine))
			{
				heal_type = 2;
			}
		}
		
		if (heal_type == 1)
		{
			if (attribs.size() == 1)
			{
				if (((Integer)(attribs.get(0))).intValue() == HEALTH)
				{
					if (target == null)
					{
						responseType = 1;
					}
					else
					{
						responseType = 2;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == ACTION)
				{
					if (target == null)
					{
						responseType = 3;
					}
					else
					{
						responseType = 4;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == MIND)
				{
					if (target == null)
					{
						responseType = 5;
					}
					else
					{
						responseType = 6;
					}
				}
				else
				{
					return null;
				}
			}
			else if (attribs.size() == 2)
			{
				attribDecWise = 0;
				attribDecWise = (((Integer)(attribs.get(0))).intValue() + ((Integer)(attribs.get(1))).intValue());
				
				if (attribDecWise == HEALTH+ACTION)
				{
					if (target == null)
					{
						responseType = 7;
					}
					else
					{
						responseType = 8;
					}
				}
				else if (attribDecWise == HEALTH+MIND)
				{
					if (target == null)
					{
						responseType = 9;
					}
					else
					{
						responseType = 10;
					}
				}
				else if (attribDecWise == HEALTH+ACTION+MIND)
				{
					if (target == null)
					{
						responseType = 11;
					}
					else
					{
						responseType = 12;
					}
				}
				else
				{
					return null;
				}
			}
			else if (attribs.size() == 3)
			{
				attribDecWise = 0;
				attribDecWise = (((Integer)(attribs.get(0))).intValue() + ((Integer)(attribs.get(1))).intValue() + ((Integer)(attribs.get(2))).intValue());
				
				if (attribDecWise == HEALTH+ACTION+MIND)
				{
					if (target == null)
					{
						responseType = 13;
					}
					else
					{
						responseType = 14;
					}
				}
				else
				{
					return null;
				}
			}
		}
		else if (heal_type == 2)
		{
			if (attribs.size() == 1)
			{
				if (((Integer)(attribs.get(0))).intValue() == HEALTH)
				{
					if (target == null)
					{
						responseType = 15;
					}
					else
					{
						responseType = 16;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == CONSTITUTION)
				{
					if (target == null)
					{
						responseType = 19;
					}
					else
					{
						responseType = 20;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == ACTION)
				{
					if (target == null)
					{
						responseType = 21;
					}
					else
					{
						responseType = 22;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == STAMINA)
				{
					if (target == null)
					{
						responseType = 25;
					}
					else
					{
						responseType = 26;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == MIND)
				{
					if (target == null)
					{
						responseType = 27;
					}
					else
					{
						responseType = 28;
					}
				}
				else if (((Integer)(attribs.get(0))).intValue() == WILLPOWER)
				{
					if (target == null)
					{
						responseType = 31;
					}
					else
					{
						responseType = 32;
					}
				}
			}
		}
		
		int damage1 = 0;
		int damage2 = 0;
		int damage3 = 0;
		
		string_id response1;
		string_id response2;
		string_id response3;
		string_id otherResponse1;
		string_id otherResponse2;
		string_id otherResponse3;
		
		prose_package pp1 = null;
		prose_package pp2 = null;
		prose_package pp3 = null;
		prose_package otherpp1 = null;
		prose_package otherpp2 = null;
		prose_package otherpp3 = null;
		
		String oob;
		String otherOob;
		String[] message =
		{
			null, null
		};
		
		switch(responseType)
		{
			case 1:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 9, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 2:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 12, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 15, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 3:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 10, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 4:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 13, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 16, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 5:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 11, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 6:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 14, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 17, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 7:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 0, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 3, 1));
				pp2 = prose.getPackage(response2, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 8:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 5, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 3, 1));
				pp2 = prose.getPackage(response2, damage2);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 7, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				otherResponse2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 3, 1));
				otherpp2 = prose.getPackage(otherResponse2, damage2);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				otherOob = packOutOfBandProsePackage(otherOob, otherpp2);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 9:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 0, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp2 = prose.getPackage(response2, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 10:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 4, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp2 = prose.getPackage(response2, damage2);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 7, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				otherResponse2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				otherpp2 = prose.getPackage(otherResponse2, damage2);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				otherOob = packOutOfBandProsePackage(otherOob, otherpp2);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 11:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 4, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp2 = prose.getPackage(response2, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 12:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 6, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp2 = prose.getPackage(response2, damage2);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 8, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				otherResponse2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				otherpp2 = prose.getPackage(otherResponse2, damage2);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				otherOob = packOutOfBandProsePackage(otherOob, otherpp2);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 13:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage3 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 0, 1));
				pp1 = prose.getPackage(response1, damage1);
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 1, 1));
				pp2 = prose.getPackage(response2, damage2);
				response3 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp3 = prose.getPackage(response3, damage3);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				oob = packOutOfBandProsePackage(oob, pp3);
				
				message[1] = oob;
				message[2] = null;
				
				break;
			}
			case 14:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage2 = ((Integer)(delta.get(i))).intValue();
					}
					else if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage3 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 5, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				response2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 1, 1));
				pp2 = prose.getPackage(response2, damage2);
				response3 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				pp3 = prose.getPackage(response3, damage3);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 7, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				otherResponse2 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 1, 1));
				otherpp2 = prose.getPackage(otherResponse2, damage2);
				otherResponse3 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 2, 1));
				otherpp3 = prose.getPackage(otherResponse3, damage3);
				
				oob = packOutOfBandProsePackage(null, pp1);
				oob = packOutOfBandProsePackage(oob, pp2);
				oob = packOutOfBandProsePackage(oob, pp3);
				
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				otherOob = packOutOfBandProsePackage(otherOob, otherpp2);
				otherOob = packOutOfBandProsePackage(otherOob, otherpp3);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 15:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 48, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 16:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == HEALTH)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 51, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 54, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 19:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == CONSTITUTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 19, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 20:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == CONSTITUTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 31, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 40, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 21:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 49, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 22:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == ACTION)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 52, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 55, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 25:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == STAMINA)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 21, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 26:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == STAMINA)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 33, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 42, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 27:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 49, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 28:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == MIND)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 53, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 56, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
			case 31:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == WILLPOWER)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 23, 1));
				pp1 = prose.getPackage(response1, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				
				message[0] = oob;
				message[1] = null;
				
				break;
			}
			case 32:
			{
				for (int i = 0; i < attribs.size(); i++)
				{
					testAbortScript();
					if (((Integer)(attribs.get(i))).intValue() == WILLPOWER)
					{
						damage1 = ((Integer)(delta.get(i))).intValue();
					}
				}
				
				response1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 35, 1));
				pp1 = prose.getPackage(response1, target, damage1);
				
				otherResponse1 = new string_id(PP_FILE_LOC, utils.dataTableGetString(RESPONSE_TEXT, 44, 1));
				otherpp1 = prose.getPackage(otherResponse1, null, null, doctor, damage1);
				
				oob = packOutOfBandProsePackage(null, pp1);
				otherOob = packOutOfBandProsePackage(null, otherpp1);
				
				message[0] = oob;
				message[1] = otherOob;
				
				break;
			}
		}
		
		return message;
	}
	
	
	public static boolean grantHealingExperience(int[] healed_damage, obj_id player, String heal_type) throws InterruptedException
	{
		return grantHealingExperience(healed_damage, player, null, heal_type);
	}
	
	
	public static boolean grantHealingExperience(int[] healed_damage, obj_id player, obj_id target, String heal_type) throws InterruptedException
	{
		if (healed_damage == null || !isIdValid(player) || heal_type == null)
		{
			return false;
		}
		
		int experience = 0;
		int total_healed = 0;
		String exp_type;
		boolean directGrant = false;
		
		for (int i = 0; i < healed_damage.length; i++)
		{
			testAbortScript();
			total_healed = total_healed + healed_damage[i];
		}
		
		if (heal_type.equals(HEAL_TYPE_MEDICAL_DAMAGE))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_WOUND))
		{
			experience = (int)(total_healed * 5f);
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_BUFF))
		{
			experience = total_healed * 2;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_STATE))
		{
			experience = total_healed * 50;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_FIRSTAID))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_POISON))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_FIRE))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_DISEASE))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_POISON))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_DISEASE))
		{
			experience = total_healed;
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_QUICK_HEAL))
		{
			return true;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_HEAL_MIND))
		{
			return true;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_WOUND))
		{
			experience = (int)(total_healed * 2.5f);
			exp_type = xp.MEDICAL;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_DAMAGE))
		{
			return true;
		}
		
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_REVIVE))
		{
			experience = 500;
			exp_type = xp.MEDICAL;
			directGrant = true;
		}
		else
		{
			return false;
		}
		
		if (experience < 1)
		{
			return false;
		}
		
		if (directGrant)
		{
			xp.grantCombatStyleXp(player, exp_type, experience);
		}
		else
		{
			assignHealingCombatCredit(player, target, exp_type, experience);
		}
		
		return true;
	}
	
	
	public static boolean grantHealingExperience(int healed_damage, obj_id player, String heal_type) throws InterruptedException
	{
		return grantHealingExperience(healed_damage, player, null, heal_type);
	}
	
	
	public static boolean grantHealingExperience(int healed_damage, obj_id player, obj_id target, String heal_type) throws InterruptedException
	{
		int[] damage_array =
		{
			healed_damage
		};
		return grantHealingExperience(damage_array, player, target, heal_type);
	}
	
	
	public static boolean assignHealingCombatCredit(obj_id player, obj_id target, String xp_type, int amount) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(player))
		{
			return false;
		}
		
		obj_id[] hateList = getHateList(target);
		
		if (hateList == null || hateList.length == 0)
		{
			return false;
		}
		
		for (int i= 0; i < hateList.length; i++)
		{
			testAbortScript();
			xp.updateCombatXpList(hateList[i], player, xp_type, amount);
		}
		
		return true;
	}
	
	
	public static boolean isDamaged(obj_id player) throws InterruptedException
	{
		return isDamaged(player, HEALTH);
	}
	
	
	public static boolean isDamaged(obj_id player, int attrib) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (getAttrib(player, attrib) != getWoundedMaxAttrib(player, attrib))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isWounded(obj_id player, int wound_type) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (getMaxAttrib(player, HEALTH) == getWoundedMaxAttrib(player, HEALTH))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	
	
	public static boolean performDotRemoval(obj_id medic, obj_id target, String heal_type, obj_id med_obj) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (heal_type == null)
		{
			return false;
		}
		
		if (isIdValid(med_obj))
		{
			String[] skillReq = getStringArrayObjVar(med_obj, consumable.VAR_SKILL_MOD_REQUIRED);
			int[] skillMin = getIntArrayObjVar(med_obj, consumable.VAR_SKILL_MOD_MIN);
			
			for (int i = 0; i < skillReq.length; i++)
			{
				testAbortScript();
				int skillMod = getSkillStatMod(medic, skillReq[i]);
				if (skillMod < skillMin[i])
				{
					String[] reqs = new String[skillReq.length];
					for (int n = 0; n < skillReq.length; n++)
					{
						testAbortScript();
						reqs[n] = skillReq[n] + "[" + getSkillStatMod(medic, skillReq[n]) + "/" + skillMin[n] + "]";
					}
					
					sui.listbox(medic, medic, consumable.MSG_INSUFFICIENT_SKILL, reqs);
					sendMedicalSpam(medic, consumable.SID_INSUFFICIENT_SKILL, COMBAT_RESULT_MEDICAL);
					
					return false;
				}
			}
		}
		
		if (!canPayHealingCost(medic, heal_type, 1.0f))
		{
			sendMedicalSpam(medic, SID_MIND_TOO_DRAINED, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (medic != target)
		{
			if (getDistance(medic, target) > consumable.MAX_AFFECT_DISTANCE)
			{
				sendMedicalSpam(medic, consumable.SID_TARGET_OUT_OF_RANGE, COMBAT_RESULT_OUT_OF_RANGE);
				return false;
			}
		}
		
		int area = 0;
		if (isIdValid(med_obj))
		{
			area = getHealingArea(med_obj);
		}
		
		obj_id[] valid_targets;
		if (area > 0)
		{
			valid_targets = getHealableTargetsInArea(medic, getLocation(target), area);
			if (valid_targets == null)
			{
				return false;
			}
		}
		else
		{
			valid_targets = new obj_id[1];
			valid_targets[0] = target;
		}
		
		applyHealingCost(medic, heal_type, 1.0f);
		
		if (isIdValid(med_obj) && !consumable.decrementCharges(med_obj,medic))
		{
			return false;
		}
		
		boolean cure_success = false;
		float removal_skill;
		String dot_type;
		
		for (int i = 0; i < valid_targets.length; i++)
		{
			testAbortScript();
			if (heal_type.equals(HEAL_TYPE_MEDICAL_FIRSTAID))
			{
				removal_skill = getEnhancedSkillStatisticModifier(medic, "healing_injury_treatment") * 3.0f;
				dot_type = dot.DOT_BLEEDING;
				
				if (dot.isBleeding(valid_targets[i]))
				{
					if (medic != valid_targets[i])
					{
						prose_package ppApplyFirstAid = prose.getPackage(SID_YOU_APPLY_FIRST_AID);
						prose.setTT(ppApplyFirstAid, valid_targets[i]);
						sendMedicalSpam(medic, ppApplyFirstAid, COMBAT_RESULT_MEDICAL);
						
						prose_package ppAppliesFirstAid = prose.getPackage(SID_APPLIES_FIRST_AID);
						prose.setTT(ppAppliesFirstAid, medic);
						sendMedicalSpam(medic, ppAppliesFirstAid, COMBAT_RESULT_MEDICAL);
					}
					else
					{
						sendMedicalSpam(medic, SID_APPLY_FIRST_AID_SELF, COMBAT_RESULT_MEDICAL);
					}
				}
			}
			else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_POISON))
			{
				if (!isIdValid(med_obj))
				{
					return false;
				}
				
				int cure_power = getDotPower(med_obj);
				
				if (isAreaMedicine(med_obj))
				{
					removal_skill = cure_power;
				}
				else
				{
					removal_skill = cure_power * (1.0f + getEnhancedSkillStatisticModifier(medic, "healing_wound_treatment") / 100.0f);
				}
				
				dot_type = dot.DOT_POISON;
				
				if (dot.isPoisoned(valid_targets[i]))
				{
					if (medic != valid_targets[i])
					{
						prose_package ppApplyPoisonAntidote = prose.getPackage(SID_APPLY_POISON_ANTIDOTE);
						prose.setTT(ppApplyPoisonAntidote, valid_targets[i]);
						sendMedicalSpam(medic, ppApplyPoisonAntidote, COMBAT_RESULT_MEDICAL);
						prose_package ppAppliesPoisonAntidote = prose.getPackage(SID_APPLIES_POISON_ANTIDOTE);
						prose.setTT(ppAppliesPoisonAntidote, medic);
						sendMedicalSpam(valid_targets[i], ppAppliesPoisonAntidote, COMBAT_RESULT_MEDICAL);
					}
					else
					{
						sendMedicalSpam(medic, SID_APPLY_POISON_ANTIDOTE_SELF, COMBAT_RESULT_MEDICAL);
					}
				}
			}
			else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_FIRE))
			{
				if (!isIdValid(med_obj))
				{
					return false;
				}
				
				int cure_power = getDotPower(med_obj);
				
				if (isAreaMedicine(med_obj))
				{
					removal_skill = cure_power;
				}
				else
				{
					removal_skill = cure_power * (1.0f + getEnhancedSkillStatisticModifier(medic, "healing_wound_treatment") / 100.0f);
				}
				
				dot_type = dot.DOT_FIRE;
				
				if (dot.isOnFire(valid_targets[i]))
				{
					if (medic != valid_targets[i])
					{
						prose_package ppAttemptSuppress = prose.getPackage(SID_ATTEMPT_SUPPRESS_FLAMES);
						prose.setTT(ppAttemptSuppress, valid_targets[i]);
						sendMedicalSpam(medic, ppAttemptSuppress, COMBAT_RESULT_MEDICAL);
						prose_package ppCoversBlanket = prose.getPackage(SID_COVERS_YOU_BLANKET);
						prose.setTT(ppCoversBlanket, medic);
						sendMedicalSpam(valid_targets[i], ppCoversBlanket, COMBAT_RESULT_MEDICAL);
					}
					else
					{
						sendMedicalSpam(medic, SID_COVERS_BLANKET_SELF, COMBAT_RESULT_MEDICAL);
					}
				}
			}
			else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_DISEASE))
			{
				if (!isIdValid(med_obj))
				{
					return false;
				}
				
				int cure_power = getDotPower(med_obj);
				
				if (isAreaMedicine(med_obj))
				{
					removal_skill = cure_power;
				}
				else
				{
					removal_skill = cure_power * (1.0f + getEnhancedSkillStatisticModifier(medic, "healing_wound_treatment") / 100.0f);
				}
				
				dot_type = dot.DOT_DISEASE;
				
				if (dot.isDiseased(valid_targets[i]))
				{
					if (medic != valid_targets[i])
					{
						prose_package ppApplyDiseaseAntidote = prose.getPackage(SID_APPLY_DISEASE_ANTIDOTE);
						prose.setTT(ppApplyDiseaseAntidote, valid_targets[i]);
						sendMedicalSpam(medic, ppApplyDiseaseAntidote, COMBAT_RESULT_MEDICAL);
						prose_package ppAppliesDiseaseAntidote = prose.getPackage(SID_APPLIES_DISEASE_ANTIDOTE);
						prose.setTT(ppAppliesDiseaseAntidote, medic);
						sendMedicalSpam(valid_targets[i], ppAppliesDiseaseAntidote, COMBAT_RESULT_MEDICAL);
					}
					else
					{
						sendMedicalSpam(medic, SID_APPLY_DISEASE_ANTIDOTE_SELF, COMBAT_RESULT_MEDICAL);
					}
				}
			}
			else
			{
				return false;
			}
			
			int poison_reduced = dot.reduceDotTypeStrength(valid_targets[i], dot_type, (int)removal_skill);
			
			if (poison_reduced != -1)
			{
				if (isPlayer(valid_targets[i]))
				{
					if (isIdValid(med_obj) && isAreaMedicine(med_obj))
					{
						poison_reduced *= 0.3f;
					}
					
					grantHealingExperience(poison_reduced, medic, valid_targets[i], heal_type);
				}
				
				if (medic != valid_targets[i])
				{
					pvpHelpPerformed(medic, valid_targets[i]);
				}
				
				cure_success = true;
			}
		}
		
		return cure_success;
	}
	
	
	public static boolean performDotApplication(obj_id medic, obj_id target, String heal_type, obj_id med_obj) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(med_obj))
		{
			return false;
		}
		
		if (heal_type == null)
		{
			return false;
		}
		
		String[] skillReq = getStringArrayObjVar(med_obj, consumable.VAR_SKILL_MOD_REQUIRED);
		int[] skillMin = getIntArrayObjVar(med_obj, consumable.VAR_SKILL_MOD_MIN);
		
		for (int i = 0; i < skillReq.length; i++)
		{
			testAbortScript();
			int skillMod = getSkillStatMod(medic, skillReq[i]);
			if (skillMod < skillMin[i])
			{
				String[] reqs = new String[skillReq.length];
				for (int n = 0; n < skillReq.length; n++)
				{
					testAbortScript();
					reqs[n] = skillReq[n] + "[" + getSkillStatMod(medic, skillReq[n]) + "/" + skillMin[n] + "]";
				}
				
				sui.listbox(medic, medic, consumable.MSG_INSUFFICIENT_SKILL, reqs);
				sendMedicalSpam(medic, consumable.SID_INSUFFICIENT_SKILL, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		if (!canPayHealingCost(medic, heal_type, 1.0f))
		{
			sendMedicalSpam(medic, SID_MIND_TOO_DRAINED, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		float healing_range = (float)getHealingRange(med_obj);
		float healing_range_mod = (float)getSkillStatMod(medic, "dot_efficiency");
		healing_range = healing_range + ((100.0f + healing_range_mod) / 15.0f);
		float strength_mod = (float)getSkillStatMod(medic, "dot_efficiency");
		
		if (getDistance(medic, target) > healing_range)
		{
			sendMedicalSpam(medic, consumable.SID_TARGET_OUT_OF_RANGE, COMBAT_RESULT_OUT_OF_RANGE);
			return false;
		}
		
		int area = getHealingArea(med_obj);
		
		obj_id[] valid_targets;
		if (area > 0)
		{
			valid_targets = getAttackableTargetsInArea(medic, getLocation(target), area);
			if (valid_targets == null)
			{
				return false;
			}
		}
		else
		{
			valid_targets = new obj_id[1];
			valid_targets[0] = target;
		}
		
		applyHealingCost(medic, heal_type, 1.0f);
		
		float strength = (float)getDotPower(med_obj) * (1.0f + strength_mod / 100.0f);
		int dot_potency = getDotPotency(med_obj);
		String dot_id = getDotId(med_obj);
		int attribute = getDotAttribute(med_obj);
		int duration = getDotDuration(med_obj);
		String dot_type;
		
		if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_POISON))
		{
			dot_type = dot.DOT_POISON;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_DISEASE))
		{
			dot_type = dot_type = dot.DOT_DISEASE;
		}
		else
		{
			return false;
		}
		
		if (!consumable.decrementCharges(med_obj,medic))
		{
			return false;
		}
		
		for (int i = 0; i < valid_targets.length; i++)
		{
			testAbortScript();
			if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_POISON))
			{
				prose_package pp_self = prose.getPackage(SID_APPLY_POISON_SELF, valid_targets[i]);
				prose_package pp_other = prose.getPackage(SID_APPLY_POISON_OTHER, medic, valid_targets[i]);
				sendMedicalSpam(medic, pp_self, COMBAT_RESULT_DEBUFF);
				sendMedicalSpam(valid_targets[i], pp_other, COMBAT_RESULT_DEBUFF);
			}
			else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_DISEASE))
			{
				prose_package pp_self = prose.getPackage(SID_APPLY_DISEASE_SELF, valid_targets[i]);
				prose_package pp_other = prose.getPackage(SID_APPLY_DISEASE_OTHER, medic, valid_targets[i]);
				sendMedicalSpam(medic, pp_self, COMBAT_RESULT_DEBUFF);
				sendMedicalSpam(valid_targets[i], pp_other, COMBAT_RESULT_DEBUFF);
			}
			
			if (isPlayer(valid_targets[i]))
			{
				if (factions.pvpDoAllowedAttackCheck(medic, valid_targets[i]))
				{
					pvpAttackPerformed(medic, valid_targets[i]);
				}
			}
			
			if (dot.applyDotEffect(valid_targets[i], medic, dot_type, dot_id, attribute, dot_potency, (int)strength, duration))
			{
				if (!isPlayer(valid_targets[i]))
				{
					grantHealingExperience((int)strength, medic, valid_targets[i], heal_type);
					
					if (!ai_lib.isInCombat(valid_targets[i]))
					{
						dictionary d = new dictionary();
						d.put("attacker", medic);
						messageTo(target, "handleDefenderCombatAction", d, 0.0f, true);
					}
				}
				else
				{
					obj_id[] defenders = new obj_id[1];
					defenders[0] = valid_targets[i];
					int[] results = new int[1];
					results[0] = 1;
					
					callDefenderCombatAction(defenders, results, medic, getCurrentWeapon(medic));
					
					if (factions.pvpDoAllowedAttackCheck(medic, valid_targets[i]))
					{
						pvpAttackPerformed(medic, valid_targets[i]);
					}
				}
			}
			else
			{
				prose_package pp = prose.getPackage(dot.SID_DOT_RESISTED, valid_targets[i]);
				sendMedicalSpam(medic, pp, COMBAT_RESULT_GENERIC);
				sendMedicalSpam(valid_targets[i], SID_RESIST_DOT_OTHER, COMBAT_RESULT_GENERIC);
			}
		}
		
		return true;
	}
	
	
	public static boolean performFirstAid(obj_id medic, obj_id target) throws InterruptedException
	{
		return performDotRemoval(medic, target, HEAL_TYPE_MEDICAL_FIRSTAID, null);
	}
	
	
	public static boolean performCurePoison(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performDotRemoval(medic, target, HEAL_TYPE_MEDICAL_CURE_POISON, med_obj);
	}
	
	
	public static boolean performCureFire(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performDotRemoval(medic, target, HEAL_TYPE_MEDICAL_CURE_FIRE, med_obj);
	}
	
	
	public static boolean performCureDisease(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performDotRemoval(medic, target, HEAL_TYPE_MEDICAL_CURE_DISEASE, med_obj);
	}
	
	
	public static boolean performApplyPosion(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performDotApplication(medic, target, HEAL_TYPE_MEDICAL_APPLY_POISON, med_obj);
	}
	
	
	public static boolean performApplyDisease(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performDotApplication(medic, target, HEAL_TYPE_MEDICAL_APPLY_DISEASE, med_obj);
	}
	
	
	public static obj_id getRevivePack(obj_id medic) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return null;
		}
		
		obj_id[] contents = utils.getFilteredPlayerContents(medic);
		if ((contents != null) && (contents.length > 0))
		{
			for (int i = 0; i < contents.length; i++)
			{
				testAbortScript();
				obj_id item = contents[i];
				if (isIdValid(item))
				{
					if (isRevivePack(item))
					{
						return item;
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean isRevivePack(obj_id pack) throws InterruptedException
	{
		return isRevivePack(pack, "");
	}
	
	
	public static boolean isRevivePack(obj_id pack, String prefix) throws InterruptedException
	{
		if (!isIdValid(pack))
		{
			return false;
		}
		
		if (getIntObjVar(pack, prefix + consumable.VAR_CONSUMABLE_MED_TYPE) == consumable.MT_REVIVE_PLAYER)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean resuscitatePlayer(obj_id medic, obj_id target, obj_id pack) throws InterruptedException
	{
		if (!isIdValid(medic) || !isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(pack))
		{
			pack = getRevivePack(medic);
			if (!isIdValid(pack))
			{
				sendMedicalSpam(medic, SID_CANNOT_RESUS_WITHOUT_KIT, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		if (medic == target)
		{
			sendMedicalSpam(medic, SID_CANNOT_RESUS_WITHOUT_TARGET, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!isRevivePack(pack))
		{
			pack = getRevivePack(medic);
			if (!isIdValid(pack))
			{
				sendMedicalSpam(medic, SID_CANNOT_RESUS_WITHOUT_PACK, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		if (!hasObjVar(target, pclib.VAR_BEEN_COUPDEGRACED))
		{
			return false;
		}
		
		int stamp = getIntObjVar(target, pclib.VAR_DEATHBLOW_STAMP);
		if (getGameTime() > (stamp + REVIVE_TIMER))
		{
			sendMedicalSpam(medic, SID_TARGET_DEAD_TOO_LONG, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (isJedi(target) && !pclib.hasConsent(medic, target))
		{
			sendMedicalSpam(medic, SID_MUST_HAVE_JEDI_CONSENT, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!group.inSameGroup(medic, target) && !pclib.hasConsent(medic, target))
		{
			sendMedicalSpam(medic, SID_GROUP_OR_CONSENT_FROM_TARGET, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!canPayHealingCost(medic, HEAL_TYPE_MEDICAL_BUFF, 1.0f))
		{
			sendMedicalSpam(medic, SID_MIND_TOO_DRAINED, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		attrib_mod[] am = getAttribModArrayObjVar(pack, consumable.VAR_CONSUMABLE_MODS);
		if ((am == null) || (am.length == 0))
		{
			return false;
		}
		
		if (consumable.consumeItem(medic, target, pack))
		{
			applyHealingCost(medic, HEAL_TYPE_MEDICAL_REVIVE, 1.0f);
			
			int[] att = new int[WILLPOWER + 1];
			for (int i = 0; i < am.length; i++)
			{
				testAbortScript();
				int attrib = am[i].getAttribute();
				float attack = am[i].getAttack();
				float decay = am[i].getDecay();
				int val = am[i].getValue();
				
				if (attack == AM_HEAL_WOUND)
				{
					val *= 2.5;
				}
				else if (decay == MOD_POOL)
				{
					val /= 4;
				}
				
				if (val > 0)
				{
					att[attrib] += val;
				}
			}
			
			grantHealingExperience(att, medic, target, HEAL_TYPE_MEDICAL_REVIVE);
			
			pvpHelpPerformed(medic, target);
			
			messageTo(target, "handlePlayerResuscitated", null, 0, true);
			return true;
		}
		
		return false;
	}
	
	
	public static boolean resuscitatePlayer(obj_id medic, obj_id target) throws InterruptedException
	{
		return resuscitatePlayer(medic, target, null);
	}
	
	
	public static float getDragPlayerRange(obj_id medic) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return -1f;
		}
		
		int mod = getSkillStatMod(medic, "healing_ability");
		
		if (mod > 0f)
		{
			float range = 8f + (22f * ((float)mod/100f));
			if (range > 0f)
			{
				return range;
			}
		}
		
		return -1f;
	}
	
	
	public static boolean playRangedAnimation(obj_id medic, obj_id target, obj_id med_obj, String[] animation) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(med_obj))
		{
			return false;
		}
		
		if (animation == null)
		{
			return false;
		}
		else if (animation.length != 2)
		{
			return false;
		}
		
		if (getPosture(medic) != POSTURE_UPRIGHT)
		{
			setPosture(medic, POSTURE_UPRIGHT);
		}
		
		String strPlaybackName = "throw_grenade";
		float fltDistance = getDistance(medic, target);
		
		if (fltDistance < 10)
		{
			strPlaybackName += "_near";
		}
		else if (fltDistance < 20)
		{
			strPlaybackName += "_medium";
		}
		else
		{
			strPlaybackName += "_far";
		}
		
		if (getHealingArea(med_obj) > 0)
		{
			strPlaybackName += "_"+ animation[0];
		}
		else
		{
			strPlaybackName += "_"+ animation[1];
		}
		
		defender_results[] cbtDefenderResults = new defender_results[1];
		cbtDefenderResults[0] = new defender_results();
		attacker_results cbtAttackerResults = new attacker_results();
		
		cbtDefenderResults[0].id = target;
		cbtDefenderResults[0].endPosture = getPosture(target);
		cbtDefenderResults[0].result = COMBAT_RESULT_HIT;
		
		cbtAttackerResults.id = medic;
		cbtAttackerResults.weapon = null;
		doCombatResults(strPlaybackName, cbtAttackerResults, cbtDefenderResults);
		
		return true;
	}
	
	
	public static void playHealDamageEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				if (isIdValid(players[i]) && exists(players[i]) && !stealth.hasInvisibleBuff(players[i]))
				{
					playClientEffectLoc(players[i], "appearance/pt_heal_2.prt", loc, 0);
				}
			}
		}
		
		return;
	}
	
	
	public static void playHealWoundEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/healing_healwound.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playHealStateEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/healing_healstate.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playHealEnhanceEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/healing_healenhance.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playApplyPoisonEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/dot_apply_poison.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playApplyAreaPoisonEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/dot_apply_area_poison.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playApplyDiseaseEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/dot_apply_disease.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static void playApplyAreaDiseaseEffect(location loc) throws InterruptedException
	{
		if (loc == null)
		{
			return;
		}
		
		obj_id[] players = getAllPlayers(loc, VAR_EFFECT_DISPLAY_RADIUS);
		if (players != null)
		{
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				playClientEffectLoc(players[i], "clienteffect/dot_apply_area_disease.cef", loc, 0);
			}
		}
		
		return;
	}
	
	
	public static boolean canDiagnose(obj_id medic, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (medic == target)
		{
			return false;
		}
		
		if (getDistance(medic, target) > consumable.MAX_AFFECT_DISTANCE)
		{
			sendMedicalSpam(medic, consumable.SID_TARGET_OUT_OF_RANGE, COMBAT_RESULT_OUT_OF_RANGE);
			return false;
		}
		
		if (!pvpCanHelp(medic, target))
		{
			sendMedicalSpam(medic, SID_PVP_NO_HELP, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (!isPlayer(target))
		{
			if (!pet_lib.isCreaturePet(target) && !pet_lib.isNpcPet(target) || ai_lib.isAndroid(target))
			{
				sendMedicalSpam(medic, SID_TEND_WOUNDS_INVALID_TARGET, COMBAT_RESULT_MEDICAL);
				return false;
			}
			else
			{
				if (!factions.pvpDoAllowedHelpCheck(medic, target))
				{
					LOG("HEALING_MESSAGE_ERROR", medic + " ->It would be unwise to help such a patient.");
					sendMedicalSpam(medic, SID_PVP_NO_HELP, COMBAT_RESULT_MEDICAL);
					return false;
				}
				else
				{
					if (!isPlayer(target))
					{
						
						if (!pet_lib.isCreaturePet(target) && !pet_lib.isNpcPet(target) || ai_lib.isAndroid(target))
						{
							LOG("HEALING_MESSAGE_ERROR", "Target must be a player or a creature pet in order to tend wounds.");
							sendMedicalSpam(medic, SID_TEND_WOUNDS_INVALID_TARGET, COMBAT_RESULT_MEDICAL);
							return false;
						}
						else
						{
							return true;
						}
					}
					else
					{
						return true;
					}
				}
			}
		}
		else
		{
			return true;
		}
	}
	
	
	public static void addHealingKillCredit(obj_id medic, obj_id target, int[] delta) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return;
		}
		
		if (!isIdValid(target))
		{
			return;
		}
		
		if (delta == null || delta.length == 0)
		{
			return;
		}
		
		if (group.inSameGroup(medic, target) && getState(target, STATE_COMBAT) == 1)
		{
			int total = 0;
			for (int i = 0; i < delta.length; i++)
			{
				testAbortScript();
				total += delta[i];
			}
			
			if (total > 0)
			{
				obj_id combatTarget = getCombatTarget(target);
				
				if (isIdValid(combatTarget))
				{
					int trackedPermissions = Math.round(total * HEALING_KILL_CREDIT_MULTIPLIER);
					
					if (target != medic)
					{
						dictionary params = new dictionary();
						params.put("healer", medic);
						messageTo(combatTarget, "addEnemyHealer", params, 0.0f, false);
					}
				}
			}
		}
	}
	
	
	public static obj_id findDroidWoundMed(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(inventory))
		{
			return null;
		}
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		if (inv_contents == null || inv_contents.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < inv_contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(inv_contents[i],consumable.VAR_CONSUMABLE_MODS))
			{
				if (hasObjVar(inv_contents[i], consumable.VAR_CONSUMABLE_DROID_MED))
				{
					attrib_mod[] mod_array = getAttribModArrayObjVar(inv_contents[i], consumable.VAR_CONSUMABLE_MODS);
					
					if (mod_array != null)
					{
						for (int j = 0; j < mod_array.length; j++)
						{
							testAbortScript();
							float attack = mod_array[j].getAttack();
							
							if (attack == AM_HEAL_WOUND)
							{
								return inv_contents[i];
							}
						}
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static obj_id findDroidDamageMed(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(inventory))
		{
			return null;
		}
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		if (inv_contents == null || inv_contents.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < inv_contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(inv_contents[i],consumable.VAR_CONSUMABLE_BASE))
			{
				if (hasObjVar(inv_contents[i], consumable.VAR_CONSUMABLE_DROID_MED))
				{
					if (hasObjVar(inv_contents[i], "consumable.energy"))
					{
						return inv_contents[i];
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static int[] distributeHAMWoundHealingPoints(obj_id patient, int total) throws InterruptedException
	{
		
		int[] toHeal = new int[3];
		toHeal[0] = total;
		toHeal[1] = 0;
		toHeal[2] = 0;
		
		return toHeal;
	}
	
	
	public static int[] distributeHAMDamageHealingPoints(obj_id patient, int total) throws InterruptedException
	{
		
		int[] toHeal = new int[3];
		toHeal[0] = total;
		toHeal[1] = 0;
		toHeal[2] = 0;
		
		return toHeal;
	}
	
	
	public static boolean isDroidWounded(obj_id patient) throws InterruptedException
	{
		
		int hWound = (getMaxAttrib(patient, HEALTH) - getWoundedMaxAttrib(patient, HEALTH));
		
		if (hWound > 0)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isDroidDamaged(obj_id patient) throws InterruptedException
	{
		
		int hDamage = (getWoundedMaxAttrib(patient, HEALTH) - getAttrib(patient, HEALTH));
		
		if (hDamage > 0)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean performDroidRepair(obj_id medic, obj_id target, obj_id med_obj, boolean pay_cost) throws InterruptedException
	{
		
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(med_obj))
		{
			return false;
		}
		
		boolean isDroidPetMed = hasObjVar(med_obj, consumable.VAR_CONSUMABLE_DROID_MED);
		boolean isDamageMed = hasObjVar(med_obj, "consumable.energy");
		
		if (!isDroidPetMed)
		{
			return false;
		}
		
		String costType = HEAL_TYPE_MEDICAL_WOUND;
		
		if (isDamageMed)
		{
			costType = HEAL_TYPE_MEDICAL_DAMAGE;
		}
		
		if (pay_cost)
		{
			if (!canPayHealingCost(medic, costType, 1.0f))
			{
				sendMedicalSpam(medic, SID_NOT_ENOUGH_MIND, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		int health_before = getAttrib(target,HEALTH);
		
		if (consumable.consumeItem(medic, target, med_obj))
		{
			if (pay_cost)
			{
				applyHealingCost(medic, costType, 1.0f);
			}
			
			int health_after = getAttrib(target,HEALTH);
			int totalHealed = health_after - health_before;
			
			int[] delta =
			{
				totalHealed
			};
			
			obj_id target_master = getMaster(target);
			
			prose_package pp_self;
			prose_package pp_other;
			
			if (isDamageMed)
			{
				pp_self = prose.getPackage(SID_DROID_REPAIR_DAMAGE_SELF, getEncodedName(target), totalHealed);
				pp_other = prose.getPackage(SID_DROID_REPAIR_DAMAGE_OTHER, getFirstName(medic), getEncodedName(target), totalHealed);
			}
			else
			{
				pp_self = prose.getPackage(SID_DROID_REPAIR_WOUND_SELF, getEncodedName(target), totalHealed);
				pp_other = prose.getPackage(SID_DROID_REPAIR_WOUND_OTHER, getFirstName(medic), getEncodedName(target), totalHealed);
			}
			
			sendMedicalSpam(medic, pp_self, COMBAT_RESULT_MEDICAL);
			
			if ((target_master != null) && (medic != target_master))
			{
				if (isIdValid(target_master))
				{
					sendMedicalSpam(target_master, pp_other, COMBAT_RESULT_MEDICAL);
				}
				
				pvpHelpPerformed(medic, target);
				
				if (isDamageMed)
				{
					addHealingKillCredit(medic, target, delta);
				}
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id findStimpackDispensorDroid(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id[] creatures = getCreaturesInRange(getLocation(player), VAR_STIMPACK_DROID_RADIUS);
		if (creatures != null)
		{
			for (int i = 0; i < creatures.length; i++)
			{
				testAbortScript();
				if (pet_lib.isDroidPet(creatures[i]))
				{
					if (hasObjVar(creatures[i], "module_data.stimpack_capacity"))
					{
						if (getIntObjVar(creatures[i], "module_data.stimpack_supply") > 0)
						{
							if (!utils.hasScriptVar(creatures[i], "module_data.stimpack_recharging"))
							{
								obj_id master = getMaster(creatures[i]);
								if (isIdValid(master))
								{
									if ((master == player) || group.inSameGroup(master, player))
									{
										return creatures[i];
									}
								}
							}
						}
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean isMedicine(obj_id object) throws InterruptedException
	{
		return isMedicine(object, "");
	}
	
	
	public static boolean isMedicine(obj_id object, String prefix) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (hasObjVar(object, prefix + consumable.VAR_CONSUMABLE_MEDICINE))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isRangedMedicine(obj_id object) throws InterruptedException
	{
		return isRangedMedicine(object, "", false);
	}
	
	
	public static boolean isRangedMedicine(obj_id object, String prefix, boolean inDocBag) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (inDocBag)
		{
			if (hasObjVar(object, prefix + "healing.dot_info"))
			{
				int[] healingInfo = getIntArrayObjVar(object, prefix + "healing.dot_info");
				if (healingInfo[0] > 0)
				{
					return true;
				}
			}
			
			return false;
		}
		else
		{
			if (hasObjVar(object, VAR_HEALING_RANGE))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	
	
	public static boolean isAreaMedicine(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (hasObjVar(object, VAR_HEALING_AREA))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isHealDamageMedicine(obj_id medicine) throws InterruptedException
	{
		return isHealDamageMedicine(medicine, "");
	}
	
	
	public static boolean isHealDamageMedicine(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		attrib_mod[] mod_array = getAttribModArrayObjVar(medicine, prefix + consumable.VAR_CONSUMABLE_MODS);
		
		if (mod_array == null)
		{
			return false;
		}
		
		for (int i = 0; i < mod_array.length; i++)
		{
			testAbortScript();
			int attribute = mod_array[i].getAttribute();
			float decay = mod_array[i].getDecay();
			
			if ((attribute != HEALTH) && (attribute != ACTION) && (attribute != MIND))
			{
			}
			
			if (decay != MOD_POOL)
			{
				return false;
			}
		}
		return true;
	}
	
	
	public static boolean isHealStateMedicine(obj_id medicine) throws InterruptedException
	{
		return isHealStateMedicine(medicine, "");
	}
	
	
	public static boolean isHealStateMedicine(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, prefix + VAR_HEALING_STATE))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isBuffMedicine(obj_id medicine) throws InterruptedException
	{
		return isBuffMedicine(medicine, "");
	}
	
	
	public static boolean isBuffMedicine(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (!isMedicine(medicine, prefix))
		{
			return false;
		}
		
		attrib_mod[] mod_array = getAttribModArrayObjVar(medicine, prefix + consumable.VAR_CONSUMABLE_MODS);
		
		if (mod_array == null)
		{
			return false;
		}
		
		for (int i = 0; i < mod_array.length; i++)
		{
			testAbortScript();
			float duration = mod_array[i].getDuration();
			if (duration < 1.0f)
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean isCureDotMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_CURE_DOT))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isApplyDotMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_APPLY_DOT))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isCurePoisonMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_CURE_DOT))
		{
			String type = getStringObjVar(medicine, VAR_HEALING_CURE_DOT);
			if (type.equals(dot.DOT_POISON))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isCureFireMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_CURE_DOT))
		{
			String type = getStringObjVar(medicine, VAR_HEALING_CURE_DOT);
			if (type.equals(dot.DOT_FIRE))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isCureDiseaseMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_CURE_DOT))
		{
			String type = getStringObjVar(medicine, VAR_HEALING_CURE_DOT);
			if (type.equals(dot.DOT_DISEASE))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isApplyPoisonMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_APPLY_DOT))
		{
			String type = getStringObjVar(medicine, VAR_HEALING_APPLY_DOT);
			if (type.equals(dot.DOT_POISON))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isApplyDiseaseMedicine(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return false;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_APPLY_DOT))
		{
			String type = getStringObjVar(medicine, VAR_HEALING_APPLY_DOT);
			if (type.equals(dot.DOT_DISEASE))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	
	public static int getHealingRange(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return 0;
		}
		
		if (isRangedMedicine(medicine))
		{
			return getIntObjVar(medicine, VAR_HEALING_RANGE);
		}
		else
		{
			return 0;
		}
	}
	
	
	public static int getHealingArea(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return 0;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_AREA))
		{
			return getIntObjVar(medicine, VAR_HEALING_AREA);
		}
		else
		{
			return 0;
		}
	}
	
	
	public static String getHealingState(obj_id medicine) throws InterruptedException
	{
		return getHealingState(medicine, "");
	}
	
	
	public static String getHealingState(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return null;
		}
		
		if (isHealStateMedicine(medicine, prefix))
		{
			return getStringObjVar(medicine, prefix + VAR_HEALING_STATE);
		}
		else
		{
			return null;
		}
	}
	
	
	public static String getCureDot(obj_id medicine) throws InterruptedException
	{
		return getCureDot(medicine, "");
	}
	
	
	public static String getCureDot(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return null;
		}
		
		if (hasObjVar(medicine, prefix + VAR_HEALING_CURE_DOT))
		{
			return getStringObjVar(medicine, prefix + VAR_HEALING_CURE_DOT);
		}
		else
		{
			return null;
		}
	}
	
	
	public static String getApplyDot(obj_id medicine) throws InterruptedException
	{
		return getApplyDot(medicine, "");
	}
	
	
	public static String getApplyDot(obj_id medicine, String prefix) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return null;
		}
		
		if (hasObjVar(medicine, prefix + VAR_HEALING_APPLY_DOT))
		{
			return getStringObjVar(medicine, prefix + VAR_HEALING_APPLY_DOT);
		}
		else
		{
			return null;
		}
	}
	
	
	public static int getDotPower(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return -1;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_DOT_POWER))
		{
			return getIntObjVar(medicine, VAR_HEALING_DOT_POWER);
		}
		else
		{
			return -1;
		}
	}
	
	
	public static int getDotPotency(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return -1;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_DOT_POTENCY))
		{
			return getIntObjVar(medicine, VAR_HEALING_DOT_POTENCY);
		}
		else
		{
			return -1;
		}
	}
	
	
	public static String getDotId(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return null;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_DOT_ID))
		{
			return getStringObjVar(medicine, VAR_HEALING_DOT_ID);
		}
		else
		{
			return null;
		}
	}
	
	
	public static int getDotAttribute(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return -1;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_DOT_ATTRIBUTE))
		{
			return getIntObjVar(medicine, VAR_HEALING_DOT_ATTRIBUTE);
		}
		else
		{
			return -1;
		}
	}
	
	
	public static int getDotDuration(obj_id medicine) throws InterruptedException
	{
		if (!isIdValid(medicine))
		{
			return -1;
		}
		
		if (hasObjVar(medicine, VAR_HEALING_DOT_DURATION))
		{
			return getIntObjVar(medicine, VAR_HEALING_DOT_DURATION);
		}
		else
		{
			return -1;
		}
	}
	
	
	public static float getHealingMultiplier(obj_id player, obj_id item, String type) throws InterruptedException
	{
		float healing_skill = 0.0f;
		
		boolean medical_type = false;
		boolean entertainer_type = false;
		
		float random_factor = rand(50, 150);
		
		if (type.equals(HEAL_TYPE_MEDICAL_DAMAGE) || type.equals(HEAL_TYPE_MEDICAL_QUICK_HEAL) || type.equals(HEAL_TYPE_MEDICAL_TEND_DAMAGE))
		{
			medical_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_efficiency");
		}
		else if (type.equals(HEAL_TYPE_MEDICAL_WOUND) || type.equals(HEAL_TYPE_MEDICAL_TEND_WOUND))
		{
			medical_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_efficiency");
		}
		else if (type.equals(HEAL_TYPE_MEDICAL_HEAL_MIND))
		{
			medical_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "dot_efficiency");
		}
		else if (type.equals(HEAL_TYPE_MEDICAL_BUFF))
		{
			medical_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_efficiency");
			
			random_factor = 100;
		}
		else if (type.equals(HEAL_TYPE_DANCE_WOUND))
		{
			entertainer_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_dance_wound");
		}
		else if (type.equals(HEAL_TYPE_DANCE_SHOCK))
		{
			entertainer_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_dance_shock");
		}
		else if (type.equals(HEAL_TYPE_MUSIC_WOUND))
		{
			entertainer_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_music_wound");
		}
		else if (type.equals(HEAL_TYPE_MUSIC_SHOCK))
		{
			entertainer_type = true;
			healing_skill = (float)getEnhancedSkillStatisticModifier(player, "healing_music_shock");
		}
		
		float complexity = 0.0f;
		
		if (item != null || item != obj_id.NULL_ID)
		{
			complexity = getComplexity(item);
		}
		
		float medicalDroidModifier = 0.0f;
		float multiplier = 0.0f;
		
		if (pet_lib.isNearMedicalDroid(player, VAR_SURGICAL_DROID_RANGE))
		{
			medicalDroidModifier = pet_lib.getMedicalDroidMod(player, VAR_SURGICAL_DROID_RANGE);
		}
		
		if (medicalDroidModifier > 0.0f)
		{
			multiplier = (1 + healing_skill / 100) * (random_factor / 100) * medicalDroidModifier;
		}
		else
		{
			multiplier = (1 + healing_skill / 100) * (random_factor / 100);
		}
		
		if (entertainer_type)
		{
			int city_id = city.checkCity(player, false);
			if (city_id > 0 && (city.cityHasSpec(city_id, city.SF_SPEC_ENTERTAINER) || city.cityHasSpec(city_id, city.SF_SPEC_MASTER_HEALING)))
			{
				multiplier *= 1.1f;
			}
		}
		
		if (medical_type)
		{
			int city_id = city.checkCity(player, false);
			if (city_id > 0 && (city.cityHasSpec(city_id, city.SF_SPEC_DOCTOR) || city.cityHasSpec(city_id, city.SF_SPEC_MASTER_HEALING)))
			{
				multiplier *= 1.1f;
			}
		}
		
		return multiplier;
	}
	
	
	public static float getHealingMultiplier(obj_id player, obj_id item) throws InterruptedException
	{
		if (isHealDamageMedicine(item))
		{
			return getHealingMultiplier(player, item, HEAL_TYPE_MEDICAL_DAMAGE);
		}
		else if (isBuffMedicine(item))
		{
			return getHealingMultiplier(player, item, HEAL_TYPE_MEDICAL_BUFF);
		}
		else
		{
			return getHealingMultiplier(player, item, HEAL_TYPE_MEDICAL_WOUND);
		}
	}
	
	
	public static boolean hasEnhancement(obj_id player, int attribute) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return true;
		}
		
		String buff_name = "medical_enhance_"+ attributeToString(attribute).toLowerCase();
		
		if (hasAttribModifier(player, buff_name) || hasSkillModModifier(player, buff_name))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static int getHealEnhanceValue(obj_id player, int attribute) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return -1;
		}
		
		String buff_name = "medical_enhance_"+ attributeToString(attribute).toLowerCase();
		attrib_mod[] all_am = getAttribModifiers(player, attribute);
		
		if (all_am != null && all_am.length > 0)
		{
			for (int i = 0; i < all_am.length; i++)
			{
				testAbortScript();
				if (all_am[i].getName().equals(buff_name))
				{
					return all_am[i].getValue();
				}
			}
		}
		
		if (utils.hasScriptVar(player, "healing." + buff_name))
		{
			return utils.getIntScriptVar(player, "healing." + buff_name);
		}
		
		return 0;
	}
	
	
	public static float applyShockWoundModifier(float multiplier, obj_id player) throws InterruptedException
	{
		float shock = (float)getShockWound(player);
		
		if (shock < 1)
		{
			return multiplier;
		}
		else
		{
			float shock_mult = (1250.0f - shock) / 1000.0f;
			
			if (shock_mult > 1.0f)
			{
				shock_mult = 1.0f;
			}
			else if (shock_mult < 0.25f)
			{
				shock_mult = 0.25f;
			}
			
			multiplier *= shock_mult;
			if (multiplier < 0)
			{
				multiplier = 0;
			}
			
			return multiplier;
		}
	}
	
	
	public static attrib_mod[] modifyMedicineAttributes(attrib_mod[] am, float multiplier) throws InterruptedException
	{
		Vector am_new = new Vector();
		am_new.setSize(0);
		
		for (int i = 0; i < am.length; i++)
		{
			testAbortScript();
			attrib_mod tmp;
			int mod_value;
			
			if (am[i].getAttack() == AM_HEAL_WOUND)
			{
				mod_value = (int)(am[i].getValue() * multiplier);
				
				tmp = utils.createHealWoundAttribMod(am[i].getAttribute(), mod_value);
				am_new = utils.addElement(am_new, tmp);
			}
			else
			{
				if (am[i].getDecay() == MOD_POOL)
				{
					mod_value = (int)(am[i].getValue() * multiplier);
					
					tmp = utils.createHealDamageAttribMod(am[i].getAttribute(), mod_value);
					am_new = utils.addElement(am_new, tmp);
				}
				else
				{
					mod_value = (int)(am[i].getValue() * multiplier);
					
					tmp = new attrib_mod(am[i].getAttribute(), mod_value, am[i].getDuration(), VAR_BUFF_MOD_ATTACK, VAR_BUFF_MOD_DECAY);
					am_new = utils.addElement(am_new, tmp);
				}
			}
		}
		
		return (attrib_mod[])am_new.toArray(new attrib_mod[0]);
	}
	
	
	public static boolean canHealDamage(obj_id player, boolean verbose) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (!hasObjVar(player, VAR_HEALING_CAN_HEALDAMAGE))
		{
			setObjVar(player, VAR_HEALING_CAN_HEALDAMAGE, 0);
		}
		
		int healing_roundtime = getIntObjVar(player, VAR_HEALING_CAN_HEALDAMAGE);
		if (healing_roundtime != 0)
		{
			if (verbose)
			{
				int time_remaining = (healing_roundtime - getGameTime());
				if (time_remaining > 0)
				{
					int[] conv_time = player_structure.convertSecondsTime(time_remaining);
					String time_str = player_structure.assembleTimeRemaining(conv_time);
					prose_package ppNoTime = prose.getPackage(SID_CANNOT_DO_THAT_TIME);
					prose.setTO(ppNoTime, time_str);
					sendMedicalSpam(player, ppNoTime, COMBAT_RESULT_MEDICAL);
				}
				else
				{
					sendMedicalSpam(player, SID_YOU_MUST_WAIT, COMBAT_RESULT_MEDICAL);
				}
			}
			
			return false;
		}
		
		int injury_skill = getSkillStatMod(player, "healing_injury_treatment");
		
		if (injury_skill == 0)
		{
			if (verbose)
			{
				sendMedicalSpam(player, SID_CANT_HEAL_DAMAGE_OR_STATES, COMBAT_RESULT_MEDICAL);
			}
			
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canHealDamage(obj_id player) throws InterruptedException
	{
		return canHealDamage(player, true);
	}
	
	
	public static boolean canHealWound(obj_id player, boolean verbose) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		location loc = getLocation(player);
		if (isIdValid(loc.cell))
		{
			obj_id building = getTopMostContainer(loc.cell);
			if (building != null && building != obj_id.NULL_ID)
			{
				if (!hasObjVar(building, "healing.canhealwound"))
				{
					if (verbose)
					{
						sendMedicalSpam(player, SID_MUST_BE_IN_HOSPITAL_OR_CS, COMBAT_RESULT_MEDICAL);
					}
					
					return false;
				}
			}
		}
		else
		{
			obj_id camp = camping.getCurrentCamp(player);
			
			if (!isIdValid(camp))
			{
				if (!pet_lib.isNearMedicalDroid(player, VAR_SURGICAL_DROID_RANGE))
				{
					if (verbose)
					{
						sendMedicalSpam(player, SID_MUST_BE_IN_HOSP_OR_CS_OR_DROID, COMBAT_RESULT_MEDICAL);
					}
					
					return false;
				}
			}
		}
		
		if (!hasObjVar(player, VAR_HEALING_CAN_HEALWOUND))
		{
			setObjVar(player, VAR_HEALING_CAN_HEALWOUND, 0);
		}
		
		int healing_roundtime = getIntObjVar(player, VAR_HEALING_CAN_HEALWOUND);
		if (healing_roundtime != 0)
		{
			if (verbose)
			{
				int time_remaining = (healing_roundtime - getGameTime());
				if (time_remaining > 0)
				{
					int[] conv_time = player_structure.convertSecondsTime(time_remaining);
					String time_str = player_structure.assembleTimeRemaining(conv_time);
					prose_package ppNoTime = prose.getPackage(SID_CANNOT_DO_THAT_TIME);
					prose.setTO(ppNoTime, time_str);
					sendMedicalSpam(player, ppNoTime, COMBAT_RESULT_MEDICAL);
				}
				else
				{
					sendMedicalSpam(player, SID_MUST_WAIT_TO_HEAL_OR_ENHANCE, COMBAT_RESULT_MEDICAL);
				}
			}
			
			return false;
		}
		
		int injury_skill = getSkillStatMod(player, "healing_wound_treatment");
		
		if (injury_skill == 0)
		{
			if (verbose)
			{
				sendMedicalSpam(player, SID_NO_HEAL_OR_ENHANCE_ABILITY, COMBAT_RESULT_MEDICAL);
			}
			
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canHealWound(obj_id player) throws InterruptedException
	{
		return canHealWound(player, true);
	}
	
	
	public static boolean canPayHealingCost(obj_id player, String heal_type, float modifier) throws InterruptedException
	{
		if (!isIdValid(player) || heal_type == null || heal_type.equals(""))
		{
			return false;
		}
		
		if (modifier <= 0)
		{
			return false;
		}
		
		int mind = getAttrib(player, MIND);
		int cost = 0;
		
		if (heal_type.equals(HEAL_TYPE_MEDICAL_DAMAGE))
		{
			cost = VAR_HEALDAMAGE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_WOUND))
		{
			cost = VAR_HEALWOUND_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_STATE))
		{
			cost = VAR_HEALSTATE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_BUFF))
		{
			cost = VAR_HEALENHANCE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_REVIVE))
		{
			cost = COST_MIND_REVIVE;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_FIRSTAID))
		{
			cost = VAR_FIRSTAID_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_POISON))
		{
			cost = VAR_CURE_POISON_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_FIRE))
		{
			cost = VAR_CURE_FIRE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_DISEASE))
		{
			cost = VAR_CURE_DISEASE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_POISON))
		{
			cost = VAR_APPLY_POISON_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_DISEASE))
		{
			cost = VAR_APPLY_DISEASE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_QUICK_HEAL))
		{
			cost = VAR_QUICK_HEAL_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_WOUND))
		{
			cost = VAR_TEND_WOUND_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_DAMAGE))
		{
			cost = VAR_TEND_DAMAGE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_HEAL_MIND))
		{
			cost = VAR_HEAL_MIND_COST;
		}
		else
		{
			return false;
		}
		
		cost = (int)(cost * modifier);
		
		if (mind < cost)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	
	
	public static float setCanHealDamage(obj_id player, int roundtime) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return -1f;
		}
		
		float newroundtime = 0.0f;
		
		if (!hasObjVar(player, VAR_HEALING_CAN_HEALDAMAGE))
		{
			setObjVar(player, VAR_HEALING_CAN_HEALDAMAGE, 0);
		}
		
		if (roundtime == 0)
		{
			setObjVar(player, VAR_HEALING_CAN_HEALDAMAGE, 0);
		}
		else
		{
			setObjVar(player, VAR_HEALING_CAN_HEALDAMAGE, getGameTime()+ (int)newroundtime);
		}
		
		return newroundtime;
	}
	
	
	public static boolean setCanHealWound(obj_id player, int roundtime) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (!hasObjVar(player, VAR_HEALING_CAN_HEALWOUND))
		{
			setObjVar(player, VAR_HEALING_CAN_HEALWOUND, 0);
		}
		
		if (roundtime == 0)
		{
			setObjVar(player, VAR_HEALING_CAN_HEALWOUND, 0);
		}
		else
		{
			setObjVar(player, VAR_HEALING_CAN_HEALWOUND, getGameTime()+ roundtime);
		}
		
		return true;
	}
	
	
	public static obj_id findHealDamageMedicine(obj_id player, obj_id target) throws InterruptedException
	{
		obj_id med_obj = null;
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(inventory))
		{
			return null;
		}
		
		if (utils.hasScriptVar(player, SCRIPT_VAR_RANGED_MED))
		{
			utils.removeScriptVar(player, SCRIPT_VAR_RANGED_MED);
		}
		
		if (utils.hasScriptVar(player, SCRIPT_VAR_DAMAGE_MED))
		{
			utils.removeScriptVar(player, SCRIPT_VAR_DAMAGE_MED);
		}
		
		String prefix = "";
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag( inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds." + k))
					{
						continue;
					}
				}
				
				if (hasObjVar(inv_contents[i],prefix+consumable.VAR_CONSUMABLE_MODS))
				{
					if (isMedicine(inv_contents[i], prefix))
					{
						if (isHealDamageMedicine(inv_contents[i], prefix))
						{
							if ((!hasObjVar(inv_contents[i], prefix + consumable.VAR_CONSUMABLE_PET_MED)) && (!hasObjVar(inv_contents[i], prefix + consumable.VAR_CONSUMABLE_DROID_MED)))
							{
								if (isIdValid(target) && getDistance(player, target) >= consumable.MAX_AFFECT_DISTANCE)
								{
									if (isRangedMedicine(inv_contents[i],prefix,set_prefix))
									{
										med_obj = inv_contents[i];
									}
									else
									{
										utils.setScriptVar(player, SCRIPT_VAR_DAMAGE_MED, inv_contents[i]);
										utils.removeScriptVar(player, "healing.prefix");
										
										if (set_prefix)
										{
											utils.setScriptVar(player, "healing.prefix", prefix);
										}
									}
								}
								else
								{
									if (!isRangedMedicine(inv_contents[i], prefix, set_prefix))
									{
										med_obj = inv_contents[i];
									}
									else
									{
										utils.setScriptVar(player, SCRIPT_VAR_RANGED_MED, inv_contents[i]);
										utils.removeScriptVar(player, "healing.prefix");
										
										if (set_prefix)
										{
											utils.setScriptVar(player, "healing.prefix", prefix);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		if (med_obj == null)
		{
			if (utils.hasScriptVar(player, SCRIPT_VAR_RANGED_MED))
			{
				med_obj = utils.getObjIdScriptVar(player, SCRIPT_VAR_RANGED_MED);
			}
			else
			{
				if (utils.hasScriptVar(player, SCRIPT_VAR_DAMAGE_MED))
				{
					med_obj = utils.getObjIdScriptVar(player, SCRIPT_VAR_DAMAGE_MED);
				}
			}
			if (utils.hasScriptVar(player, "healing.prefix"))
			{
				prefix = utils.getStringScriptVar(player, "healing.prefix");
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
		}
		
		return med_obj;
	}
	
	
	public static obj_id findHealDamageMedicine(obj_id player) throws InterruptedException
	{
		return findHealDamageMedicine(player, null);
	}
	
	
	public static obj_id findHealWoundMedicine(obj_id player, int wound_type) throws InterruptedException
	{
		obj_id med_obj = null;
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		if ((wound_type < 0) || (wound_type >= NUM_ATTRIBUTES))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		String prefix = "";
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag(inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds." + k))
					{
						continue;
					}
				}
				
				if (hasObjVar(inv_contents[i], prefix + consumable.VAR_CONSUMABLE_MODS))
				{
					if (isMedicine(inv_contents[i], prefix))
					{
						if (!isRevivePack(inv_contents[i], prefix))
						{
							attrib_mod[] mod_array = getAttribModArrayObjVar(inv_contents[i], prefix+consumable.VAR_CONSUMABLE_MODS);
							if (mod_array != null)
							{
								for (int j = 0; j < mod_array.length; j++)
								{
									testAbortScript();
									int attribute = mod_array[j].getAttribute();
									float attack = mod_array[j].getAttack();
									
									if ((attribute == wound_type)&&(attack == AM_HEAL_WOUND))
									{
										med_obj = inv_contents[i];
									}
								}
							}
						}
					}
				}
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
		}
		
		return med_obj;
	}
	
	
	public static obj_id findHealStateMedicine(obj_id player, int state_type) throws InterruptedException
	{
		obj_id med_obj = null;
		
		if (player == null || player == obj_id.NULL_ID)
		{
			return null;
		}
		
		if (state_type < 12 || state_type > 15)
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		String prefix = "";
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag(inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds."+k))
					{
						continue;
					}
				}
				
				if (isMedicine(inv_contents[i], prefix))
				{
					if (isHealStateMedicine(inv_contents[i],prefix))
					{
						String med_state = getHealingState(inv_contents[i],prefix);
						int med_state_type = stringToState(med_state);
						
						if (state_type == med_state_type)
						{
							med_obj = inv_contents[i];
						}
					}
				}
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
		}
		
		return med_obj;
	}
	
	
	public static obj_id findBuffMedicine(obj_id player, int buff_type) throws InterruptedException
	{
		if (player == null || player == obj_id.NULL_ID)
		{
			return null;
		}
		
		obj_id med_obj = null;
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		if ((buff_type < 0)||(buff_type > 10))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		String prefix = "";
		obj_id[] inv_contents = utils.getContents(inventory, false);
		
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag(inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds." + k))
					{
						continue;
					}
				}
				
				if (hasObjVar(inv_contents[i],prefix+consumable.VAR_CONSUMABLE_MODS))
				{
					if (isBuffMedicine(inv_contents[i],prefix))
					{
						attrib_mod[] mod_array = getAttribModArrayObjVar(inv_contents[i], prefix+consumable.VAR_CONSUMABLE_MODS);
						if (mod_array != null)
						{
							for (int j = 0; j < mod_array.length; j++)
							{
								testAbortScript();
								int attribute = mod_array[j].getAttribute();
								if (attribute == buff_type)
								{
									med_obj = inv_contents[i];
								}
							}
						}
					}
				}
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
		}
		
		return med_obj;
	}
	
	
	public static obj_id findCureDotMedicine(obj_id player, String dot_type) throws InterruptedException
	{
		obj_id med_obj = null;
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		if (dot_type == null)
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		String prefix = "";
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag(inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds."+k))
					{
						continue;
					}
				}
				
				if (hasObjVar(inv_contents[i], prefix+VAR_HEALING_CURE_DOT))
				{
					String med_dot_type = getCureDot(inv_contents[i],prefix);
					if (dot_type.equals(med_dot_type))
					{
						med_obj = inv_contents[i];
					}
				}
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
			
		}
		
		return med_obj;
	}
	
	
	public static obj_id findApplyDotMedicine(obj_id player, String dot_type) throws InterruptedException
	{
		obj_id med_obj = null;
		
		if (!isIdValid(player))
		{
			return null;
		}
		
		if (dot_type == null)
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		String prefix = "";
		obj_id[] inv_contents = utils.getContents(inventory, false);
		
		for (int i = 0; (i < inv_contents.length) && (med_obj == null); i++)
		{
			testAbortScript();
			prefix = "";
			int loop_count = 1;
			boolean set_prefix = false;
			
			if (doctor_bag.isDoctorBag(inv_contents[i]))
			{
				set_prefix = true;
				loop_count = doctor_bag.MAX_MEDS;
			}
			
			for (int k=0; (k < loop_count) && (med_obj == null); k++)
			{
				testAbortScript();
				if (set_prefix)
				{
					prefix = "meds."+ k + ".";
					
					if (!hasObjVar(inv_contents[i], "meds."+k))
					{
						continue;
					}
				}
				
				if (hasObjVar(inv_contents[i], prefix+VAR_HEALING_APPLY_DOT))
				{
					String med_dot_type = getApplyDot(inv_contents[i],prefix);
					if (dot_type.equals(med_dot_type))
					{
						med_obj = inv_contents[i];
					}
				}
			}
		}
		
		if (med_obj != null)
		{
			if (doctor_bag.isDoctorBag(med_obj))
			{
				doctor_bag.setSurrogateState(med_obj, prefix);
			}
		}
		
		return med_obj;
	}
	
	
	public static boolean hasCombatState(obj_id player, int state_type) throws InterruptedException
	{
		
		return false;
		
	}
	
	
	public static int findLargestWoundHeal(obj_id patient, obj_id doctor) throws InterruptedException
	{
		return -1;
	}
	
	
	public static int findLargestTendWound(obj_id patient) throws InterruptedException
	{
		return -1;
	}
	
	
	public static int findWorstState(obj_id patient, obj_id doctor) throws InterruptedException
	{
		return -1;
	}
	
	
	public static int stringToAttribute(String attribute_str) throws InterruptedException
	{
		int attribute_int = -1;
		
		if (attribute_str.equals("HEALTH"))
		{
			attribute_int = HEALTH;
		}
		if (attribute_str.equals("CONSTITUTION"))
		{
			attribute_int = CONSTITUTION;
		}
		if (attribute_str.equals("ACTION"))
		{
			attribute_int = ACTION;
		}
		if (attribute_str.equals("STAMINA"))
		{
			attribute_int = STAMINA;
		}
		if (attribute_str.equals("MIND"))
		{
			attribute_int = MIND;
		}
		if (attribute_str.equals("WILLPOWER"))
		{
			attribute_int = WILLPOWER;
		}
		if (attribute_str.equals("POISON"))
		{
			attribute_int = 6;
		}
		if (attribute_str.equals("DISEASE"))
		{
			attribute_int = 7;
		}
		
		return attribute_int;
	}
	
	
	public static String attributeToString(int attribute_int) throws InterruptedException
	{
		if ((attribute_int < 0) || (attribute_int >= NUM_ATTRIBUTES + 2))
		{
			return null;
		}
		
		String[] names =
		{
			"HEALTH", "CONSTITUTION", "ACTION", "STAMINA", "MIND", "WILLPOWER", "POISON", "DISEASE"
		};
		
		return names[attribute_int];
	}
	
	
	public static int stringToState(String state_str) throws InterruptedException
	{
		state_str = state_str.toUpperCase();
		int state_int = -1;
		
		if (state_str.equals("STUNNED"))
		{
			state_int = STATE_STUNNED;
		}
		if (state_str.equals("BLINDED"))
		{
			state_int = STATE_BLINDED;
		}
		if (state_str.equals("DIZZY"))
		{
			state_int = STATE_DIZZY;
		}
		if (state_str.equals("INTIMIDATED"))
		{
			state_int = STATE_INTIMIDATED;
		}
		
		return state_int;
	}
	
	
	public static String stateTypeToString(int state_type) throws InterruptedException
	{
		if ((state_type < 12)||(state_type > 15))
		{
			return null;
		}
		
		String[] names =
		{
			"STUNNED", "BLINDED", "DIZZY", "INTIMIDATED"
		};
		
		return names[state_type - 12];
	}
	
	
	public static boolean performMedicalHealDamage(obj_id medic, obj_id target, obj_id med_obj, boolean pay_cost) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(med_obj))
		{
			return false;
		}
		
		boolean isRanged = isRangedMedicine(med_obj);
		boolean isPetMed = hasObjVar(med_obj, consumable.VAR_CONSUMABLE_PET_MED);
		boolean isDroidPetMed = hasObjVar(med_obj, consumable.VAR_CONSUMABLE_DROID_MED);
		
		if (pay_cost)
		{
			if (!canPayHealingCost(medic, HEAL_TYPE_MEDICAL_DAMAGE, 1.0f))
			{
				sendMedicalSpam(medic, SID_MIND_TOO_DRAINED, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		int health_before = getAttrib(target,HEALTH);
		
		if (consumable.consumeItem(medic, target, med_obj))
		{
			if (pay_cost)
			{
				applyHealingCost(medic, HEAL_TYPE_MEDICAL_DAMAGE, 1.0f);
			}
			
			int health_after = getAttrib(target,HEALTH);
			int health_delta = health_after - health_before;
			
			int[] attribs =
			{
				HEALTH
			};
			int[] delta =
			{
				health_delta
			};
			
			String[] heal_message = assembleHealingMessage(attribs, delta, medic, target, med_obj);
			if (heal_message != null)
			{
				sendCombatSpamMessageOob(medic, target, heal_message[0], true, false, false, COMBAT_RESULT_MEDICAL);
			}
			
			if ((!isPetMed) && (!isDroidPetMed))
			{
				grantHealingExperience(delta, medic, target, HEAL_TYPE_MEDICAL_DAMAGE);
			}
			
			if (medic != target)
			{
				if (heal_message != null)
				{
					sendCombatSpamMessageOob(medic, target, heal_message[1], false, true, false, COMBAT_RESULT_MEDICAL);
				}
				
				pvpHelpPerformed(medic, target);
				
				addHealingKillCredit(medic, target, delta);
			}
			
			return true;
		}
		
		return false;
	}
	
	
	public static boolean performMedicalHealDamage(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		return performMedicalHealDamage(medic, target, med_obj, true);
	}
	
	
	public static obj_id[] getHealableTargetsInArea(obj_id medic, location loc, int radius) throws InterruptedException
	{
		if (loc == null)
		{
			return null;
		}
		
		obj_id[] objects = getCreaturesInRange(loc, radius);
		Vector healable_targets = new Vector();
		healable_targets.setSize(0);
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (!isPlayer(objects[i]))
			{
				if (pet_lib.isCreaturePet(objects[i]))
				{
					healable_targets = utils.addElement(healable_targets, objects[i]);
				}
			}
			else
			{
				if (factions.pvpDoAllowedHelpCheck(medic, objects[i]))
				{
					healable_targets = utils.addElement(healable_targets, objects[i]);
				}
			}
		}
		
		if (healable_targets.size() < 1)
		{
			return null;
		}
		else
		{
			obj_id[] _healable_targets = new obj_id[0];
			if (healable_targets != null)
			{
				_healable_targets = new obj_id[healable_targets.size()];
				healable_targets.toArray(_healable_targets);
			}
			return _healable_targets;
		}
	}
	
	
	public static obj_id[] getAttackableTargetsInArea(obj_id medic, location loc, int radius) throws InterruptedException
	{
		if (loc == null)
		{
			return null;
		}
		
		obj_id[] objects = getCreaturesInRange(loc, radius);
		Vector attackable_targets = new Vector();
		attackable_targets.setSize(0);
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isMob(objects[i]))
			{
				if (factions.pvpDoAllowedAttackCheck(medic, objects[i]))
				{
					if (!isIncapacitated(objects[i]) && !isDead(objects[i]))
					{
						if (!pet_lib.isVehiclePet(objects[i]) && !ai_lib.isAndroid(objects[i]) && ai_lib.aiGetNiche(objects[i])!=NICHE_DROID && ai_lib.aiGetNiche(objects[i])!=NICHE_VEHICLE && !vehicle.isDriveableVehicle(objects[i]))
						{
							attackable_targets = utils.addElement(attackable_targets, objects[i]);
						}
					}
				}
			}
		}
		
		if (attackable_targets.size() < 1)
		{
			return null;
		}
		else
		{
			obj_id[] _attackable_targets = new obj_id[0];
			if (attackable_targets != null)
			{
				_attackable_targets = new obj_id[attackable_targets.size()];
				attackable_targets.toArray(_attackable_targets);
			}
			return _attackable_targets;
		}
	}
	
	
	public static boolean applyHealingCost(obj_id player, String heal_type, float modifier) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (modifier <= 0)
		{
			return false;
		}
		
		int mind = getAttrib(player, MIND);
		int cost = 0;
		
		if (heal_type.equals(HEAL_TYPE_MEDICAL_DAMAGE))
		{
			cost = VAR_HEALDAMAGE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_WOUND))
		{
			cost = VAR_HEALWOUND_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_STATE))
		{
			cost = VAR_HEALSTATE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_BUFF))
		{
			cost = VAR_HEALENHANCE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_REVIVE))
		{
			cost = COST_MIND_REVIVE;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_FIRSTAID))
		{
			cost = VAR_FIRSTAID_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_POISON))
		{
			cost = VAR_CURE_POISON_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_FIRE))
		{
			cost = VAR_CURE_FIRE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_CURE_DISEASE))
		{
			cost = VAR_CURE_DISEASE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_POISON))
		{
			cost = VAR_APPLY_POISON_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_APPLY_DISEASE))
		{
			cost = VAR_APPLY_DISEASE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_QUICK_HEAL))
		{
			cost = VAR_QUICK_HEAL_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_WOUND))
		{
			cost = VAR_TEND_WOUND_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_TEND_DAMAGE))
		{
			cost = VAR_TEND_DAMAGE_COST;
		}
		else if (heal_type.equals(HEAL_TYPE_MEDICAL_HEAL_MIND))
		{
			cost = VAR_HEAL_MIND_COST;
		}
		else
		{
			return false;
		}
		
		cost = (int)(cost * modifier);
		if (mind < cost)
		{
			return false;
		}
		
		return drainAttributes(player, 0, cost);
	}
	
	
	public static boolean performHealState(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		
		return false;
	}
	
	
	public static boolean performHealEnhance(obj_id medic, obj_id target, obj_id med_obj) throws InterruptedException
	{
		if (medic == null || medic == obj_id.NULL_ID)
		{
			return false;
		}
		if (target == null || target == obj_id.NULL_ID)
		{
			return false;
		}
		if (med_obj == null || med_obj == obj_id.NULL_ID)
		{
			return false;
		}
		
		if (!canPayHealingCost(medic, HEAL_TYPE_MEDICAL_BUFF, 1.0f))
		{
			LOG("HEALING_MESSAGE_ERROR", medic + " ->Your mind is too drained to do that.");
			sendMedicalSpam(medic, SID_MIND_TOO_DRAINED, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		if (consumable.consumeItem(medic, target, med_obj))
		{
			applyHealingCost(medic, HEAL_TYPE_MEDICAL_BUFF, 1.0f);
			
			int[] attribute =
			{
				HEALTH
			};
			int duration = 0;
			String attribute_string = "";
			String objvar_name = "";
			int buff_amount = 0;
			int old_buff_amount = 0;
			String medicinebuff = SCRIPT_VAR_MEDICINE_BUFF;
			String medicinerebuff = SCRIPT_VAR_MEDICINE_REBUFF;
			int net_buff_amount = 0;
			
			for (int x = 0; x < attribute.length; x++)
			{
				testAbortScript();
				buff_amount = 0;
				old_buff_amount = 0;
				net_buff_amount = 0;
				
				attribute_string = attributeToString(attribute[x]);
				attribute_string = attribute_string.substring(0,1).toUpperCase() + attribute_string.substring(1).toLowerCase();
				objvar_name = "healing."+ attribute_string.toLowerCase() + "_enhance";
				if (utils.hasScriptVar(target, medicinebuff + "."+x))
				{
					
					buff_amount = utils.getIntScriptVar(target, medicinebuff + "."+x);
					utils.removeScriptVar(target, medicinebuff + "."+x);
				}
				
				if (utils.hasScriptVar(target, medicinerebuff + "."+x))
				{
					
					old_buff_amount = utils.getIntScriptVar(target, medicinerebuff + "."+x);
					utils.removeScriptVar(target, medicinerebuff + "."+x);
				}
				
				net_buff_amount = buff_amount - old_buff_amount;
				
				if (attribute[x] == 9 || attribute[x] == 10)
				{
					attribute_string += " Resistance";
				}
				
				if (net_buff_amount > 0)
				{
					if (x == 0)
					{
						grantHealingExperience(buff_amount, medic, target, HEAL_TYPE_MEDICAL_BUFF);
					}
				}
				
				string_id attribute_string_id = new string_id("att_n", attribute_string.toLowerCase());
				if (medic != target)
				{
					prose_package ppYouEnhanceYour = prose.getPackage(SID_YOU_ENHANCE_YOUR_ATT);
					prose.setTO(ppYouEnhanceYour, attribute_string_id);
					prose.setDI(ppYouEnhanceYour, net_buff_amount);
					sendMedicalSpam(medic, ppYouEnhanceYour, COMBAT_RESULT_BUFF);
					LOG("HEALING_MESSAGE_ERROR", medic + " ->You enhance your "+ attribute_string + " by "+ net_buff_amount + ".");
					if (net_buff_amount > 0)
					{
						prose_package ppYouEnhanceTargetAtt = prose.getPackage(SID_YOU_ENHANCE_TARGETS_ATTRIBUTE);
						prose.setTT(ppYouEnhanceTargetAtt, target);
						prose.setTO(ppYouEnhanceTargetAtt, attribute_string_id);
						prose.setDI(ppYouEnhanceTargetAtt, net_buff_amount);
						sendMedicalSpam(medic, ppYouEnhanceTargetAtt, COMBAT_RESULT_BUFF);
						prose_package ppMedicEnhancesYourAtt = prose.getPackage(SID_MEDIC_ENHANCES_YOUR_ATTRIBUTE);
						prose.setTT(ppMedicEnhancesYourAtt, medic);
						prose.setTO(ppMedicEnhancesYourAtt, attribute_string_id);
						prose.setDI(ppMedicEnhancesYourAtt, net_buff_amount);
						sendMedicalSpam(target, ppMedicEnhancesYourAtt, COMBAT_RESULT_BUFF);
						LOG("LOG_CHANNEL", medic + " ->You enhance "+ getFirstName(target) + "'s "+ attribute_string + " by "+ net_buff_amount + ".");
						LOG("LOG_CHANNEL", target + " ->"+ getFirstName(medic) + " enhances your "+ attribute_string + " by "+ net_buff_amount + ".");
					}
					else
					{
						prose_package ppReapplyTarget = prose.getPackage(SID_REAPPLY_ENHANCEMENT_TARGET);
						prose.setTT(ppReapplyTarget, target);
						prose.setTO(ppReapplyTarget, attribute_string_id);
						sendMedicalSpam(medic, ppReapplyTarget, COMBAT_RESULT_BUFF);
						prose_package ppReappliesYou = prose.getPackage(SID_REAPPLIES_ENHANCEMENT_YOU);
						prose.setTT(ppReappliesYou, medic);
						prose.setTO(ppReappliesYou, attribute_string_id);
						sendMedicalSpam(target, ppReappliesYou, COMBAT_RESULT_BUFF);
						LOG("LOG_CHANNEL", medic + " ->You re-apply "+ getFirstName(target) + "'s "+ attribute_string + " enhancement.");
						LOG("LOG_CHANNEL", target + " ->"+ getFirstName(medic) + " re-applies your "+ attribute_string + " enhancement.");
					}
					
					pvpHelpPerformed(medic, target);
				}
				else
				{
					prose_package ppReapplySelf = prose.getPackage(SID_REAPPLY_ENHANCEMENT_SELF);
					prose.setTO(ppReapplySelf, attribute_string_id);
					sendMedicalSpam(medic, ppReapplySelf, COMBAT_RESULT_BUFF);
					LOG("HEALING_MESSAGE_ERROR", medic + " ->You re-apply your "+ attribute_string + " enhancement.");
					if (net_buff_amount > 0)
					{
						prose_package ppEnhanceBySelf = prose.getPackage(SID_ENHANCE_ATTRIBUTE_BY_SELF);
						prose.setTO(ppEnhanceBySelf, attribute_string_id);
						prose.setDI(ppEnhanceBySelf, net_buff_amount);
						sendMedicalSpam(medic, ppEnhanceBySelf, COMBAT_RESULT_BUFF);
						LOG("LOG_CHANNEL", medic + " ->You enhance your "+ attribute_string + " by "+ net_buff_amount + ".");
					}
				}
			}
			return true;
		}
		
		return false;
	}
	
	
	public static boolean addHealingBuffIcon(obj_id target, int attribute, float duration) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		switch(attribute)
		{
			case HEALTH:
			addBuffIcon(target, "medical_enhance_health", duration);
			break;
			
			case CONSTITUTION:
			addBuffIcon(target, "medical_enhance_constitution", duration);
			break;
			
			case ACTION:
			addBuffIcon(target, "medical_enhance_action", duration);
			break;
			
			case STAMINA:
			addBuffIcon(target, "medical_enhance_stamina", duration);
			break;
			
			case MIND:
			addBuffIcon(target, "performance_enhance_dance_mind", duration);
			break;
			
			case WILLPOWER:
			addBuffIcon(target, "performance_enhance_music_willpower", duration);
			break;
			
			default:
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id findPetDamageMed(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(inventory))
		{
			return null;
		}
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		if (inv_contents == null || inv_contents.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < inv_contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(inv_contents[i],consumable.VAR_CONSUMABLE_MODS))
			{
				if (hasObjVar(inv_contents[i], consumable.VAR_CONSUMABLE_PET_MED))
				{
					if (healing.isHealDamageMedicine(inv_contents[i]))
					{
						return inv_contents[i];
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static obj_id findPetVitalityMed(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!isIdValid(inventory))
		{
			return null;
		}
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		if (inv_contents == null || inv_contents.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < inv_contents.length; i++)
		{
			testAbortScript();
			if (hasObjVar(inv_contents[i],consumable.VAR_CONSUMABLE_BASE))
			{
				if (hasObjVar(inv_contents[i], consumable.VAR_CONSUMABLE_PET_MED))
				{
					if (hasObjVar(inv_contents[i], "consumable.strength"))
					{
						return inv_contents[i];
					}
				}
			}
		}
		
		return null;
	}
	
	
	public static boolean performTendWoundsFromTool(obj_id medic, obj_id target, int attribute, obj_id medikit) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (!isIdValid(medikit))
		{
			return false;
		}
		
		if (medic != target)
		{
			if (getDistance(medic, target) > consumable.MAX_AFFECT_DISTANCE)
			{
				sendMedicalSpam(medic, consumable.SID_TARGET_OUT_OF_RANGE, COMBAT_RESULT_OUT_OF_RANGE);
				return false;
			}
		}
		
		if (!isWounded(target, HEALTH))
		{
			if (medic == target)
			{
				sendMedicalSpam(medic, SID_NO_WOUNDS_OF_TYPE_SELF, COMBAT_RESULT_MEDICAL);
			}
			else
			{
				prose_package ppNoWounds = prose.getPackage(SID_NO_WOUNDS_OF_TYPE_TARGET);
				prose.setTT(ppNoWounds, target);
				sendMedicalSpam(medic, ppNoWounds, COMBAT_RESULT_MEDICAL);
			}
			
			return false;
		}
		
		float healingCostMod = 0f;
		if (hasObjVar(medikit, "medikit.quality"))
		{
			int medikitQuality = getIntObjVar(medikit, "medikit.quality");
			int medikitQualModScratch = (int) (((100-medikitQuality)/2)+50.5);
			float medikitQualMod = ((float) (medikitQualModScratch))/100.0f;
			healingCostMod = medikitQualMod;
		}
		else
		{
			healingCostMod = 0.9f;
		}
		
		if (!hasCertification(medic, medikit))
		{
			string_id strSpam = new string_id("medicine_name", "no_proficiency");
			sendMedicalSpam(medic, strSpam, COMBAT_RESULT_MEDICAL);
			
			healingCostMod = healingCostMod * 2.0f;
		}
		
		if (!applyHealingCost(medic, HEAL_TYPE_MEDICAL_TEND_WOUND, healingCostMod))
		{
			sendMedicalSpam(medic, SID_NOT_ENOUGH_MIND, COMBAT_RESULT_MEDICAL);
			return false;
		}
		
		int attrib_before = getWoundedMaxAttrib(target, HEALTH);
		
		float mult = getHealingMultiplier(medic, null, HEAL_TYPE_MEDICAL_TEND_WOUND);
		int heal_power = (int)(VAR_TEND_WOUND_BASE_POWER * mult);
		
		int attrib_after = getWoundedMaxAttrib(target, HEALTH);
		int attrib_delta = attrib_after - attrib_before;
		
		String attribute_string = attributeToString(HEALTH).toLowerCase();
		string_id attribute_string_id = new string_id("att_n", attribute_string.toLowerCase());
		
		if (medic != target)
		{
			prose_package ppHealAttTarget = prose.getPackage(SID_HEAL_ATTRIBUTE_TARGET);
			prose.setTT(ppHealAttTarget, target);
			prose.setTO(ppHealAttTarget, attribute_string_id);
			prose.setDI(ppHealAttTarget, attrib_delta);
			sendMedicalSpam(medic, ppHealAttTarget, COMBAT_RESULT_MEDICAL);
			prose_package ppAttHealed = prose.getPackage(SID_ATTRIBUTE_HEALED);
			prose.setTT(ppAttHealed, medic);
			prose.setTO(ppAttHealed, attribute_string_id);
			prose.setDI(ppAttHealed, attrib_delta);
			sendMedicalSpam(target, ppAttHealed, COMBAT_RESULT_MEDICAL);
			
			incrementCount(medikit, -1);
			
			grantHealingExperience(attrib_delta, medic, target, HEAL_TYPE_MEDICAL_TEND_WOUND);
			
			pvpHelpPerformed(medic, target);
		}
		else
		{
			prose_package ppHealAttribSelf = prose.getPackage(SID_HEAL_ATTRIB_SELF);
			prose.setTO(ppHealAttribSelf, attribute_string_id);
			prose.setDI(ppHealAttribSelf, attrib_delta);
			sendMedicalSpam(medic, ppHealAttribSelf, COMBAT_RESULT_MEDICAL);
			
			incrementCount(medikit, -1);
		}
		
		return true;
	}
	
	
	public static boolean performQuickHealTool(obj_id medic, obj_id target, boolean tend_damage, obj_id medikit) throws InterruptedException
	{
		if (!isIdValid(medic))
		{
			return false;
		}
		
		if (!isIdValid(target))
		{
			return false;
		}
		
		if (medic != target)
		{
			if (getDistance(medic, target) > consumable.MAX_AFFECT_DISTANCE)
			{
				sendMedicalSpam(medic, consumable.SID_TARGET_OUT_OF_RANGE, COMBAT_RESULT_OUT_OF_RANGE);
				return false;
			}
		}
		
		float healingCostMod = 0f;
		if (hasObjVar(medikit, "medikit.quality"))
		{
			int medikitQuality = getIntObjVar(medikit, "medikit.quality");
			int medikitQualModScratch = (int) (((100-medikitQuality)/2)+50.5);
			float medikitQualMod = ((float) (medikitQualModScratch))/100.0f;
			healingCostMod = medikitQualMod;
		}
		else
		{
			healingCostMod = 0.9f;
		}
		
		if (!hasCertification(medic, medikit))
		{
			string_id strSpam = new string_id("medicine_name", "no_proficiency");
			sendMedicalSpam(medic, strSpam, COMBAT_RESULT_MEDICAL);
			
			healingCostMod = healingCostMod * 2.0f;
		}
		
		if (tend_damage)
		{
			if (!applyHealingCost(medic, HEAL_TYPE_MEDICAL_TEND_DAMAGE, healingCostMod))
			{
				sendMedicalSpam(medic, SID_NOT_ENOUGH_MIND, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		else
		{
			if (!applyHealingCost(medic, HEAL_TYPE_MEDICAL_QUICK_HEAL, healingCostMod))
			{
				sendMedicalSpam(medic, SID_NOT_ENOUGH_MIND, COMBAT_RESULT_MEDICAL);
				return false;
			}
		}
		
		if (tend_damage)
		{
			
		}
		else
		{
			
		}
		
		int health_before = getAttrib(target,HEALTH);
		
		float mult;
		if (tend_damage)
		{
			mult = getHealingMultiplier(medic, null, HEAL_TYPE_MEDICAL_TEND_DAMAGE);
		}
		else
		{
			mult = getHealingMultiplier(medic, null, HEAL_TYPE_MEDICAL_QUICK_HEAL);
		}
		
		int heal_power;
		if (tend_damage)
		{
			heal_power = (int)(VAR_TEND_DAMAGE_BASE_POWER * mult);
		}
		else
		{
			heal_power = (int)(VAR_QUICK_HEAL_BASE_POWER * mult);
		}
		
		attrib_mod am = utils.createHealDamageAttribMod(HEALTH, heal_power);
		utils.addAttribMod(target, am);
		
		int health_after = getAttrib(target,HEALTH);
		int health_delta = health_after - health_before;
		
		int[] attribs =
		{
			HEALTH
		};
		int[] delta =
		{
			health_delta
		};
		
		String[] heal_message = assembleHealingMessage(attribs, delta, medic, target, null);
		
		if (heal_message != null)
		{
			sendCombatSpamMessageOob(medic, target, heal_message[0], true, false, false, COMBAT_RESULT_MEDICAL);
		}
		
		if (medic != target)
		{
			if (heal_message != null)
			{
				sendCombatSpamMessageOob(medic, target, heal_message[1], false, true, false, COMBAT_RESULT_MEDICAL);
			}
			
			if (tend_damage)
			{
				grantHealingExperience(delta, medic, target, HEAL_TYPE_MEDICAL_TEND_DAMAGE);
			}
			else
			{
				grantHealingExperience(delta, medic, target, HEAL_TYPE_MEDICAL_QUICK_HEAL);
			}
			
			incrementCount(medikit, -1);
			
			pvpHelpPerformed(medic, target);
			
			addHealingKillCredit(medic, target, delta);
		}
		
		return true;
	}
	
	
	public static boolean isAdvancedTool(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		String fullPath = getTemplateName(target);
		if (fullPath == null || fullPath.equals(""))
		{
			return false;
		}
		
		if (fullPath.indexOf("medikit_tool_advanced") > -1)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean hasCertification(obj_id objPlayer, obj_id objMedikit) throws InterruptedException
	{
		if (isAdvancedTool(objMedikit))
		{
			if (hasObjVar(objMedikit, "medikit.strCertUsed"))
			{
				String strCommand = getStringObjVar(objMedikit, "medikit.strCertUsed");
				
				if (!hasCommand(objPlayer, strCommand))
				{
					return false;
				}
			}
			else
			{
				string_id strName = getNameStringId(objMedikit);
				if (strName != null)
				{
					String strAscii = strName.getAsciiId();
					String strCert = "cert_"+ strAscii;
					
					if (!hasCommand(objPlayer, strCert))
					{
						return false;
					}
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean isMedikit(obj_id object) throws InterruptedException
	{
		if (!isIdValid(object))
		{
			return false;
		}
		
		if (hasObjVar(object,"medikit.isMedikit"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id findMedikit(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return null;
		}
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (inventory == null)
		{
			return null;
		}
		
		obj_id[] inv_contents = utils.getContents(inventory, false);
		
		if (inv_contents == null || inv_contents.length == 0)
		{
			return null;
		}
		
		for (int i = 0; i < inv_contents.length; i++)
		{
			testAbortScript();
			if (isMedikit(inv_contents[i]))
			{
				return inv_contents[i];
			}
		}
		
		return null;
	}
	
	
	public static void doHealingAnimationAndEffect(obj_id user, obj_id target) throws InterruptedException
	{
		if (user == target)
		{
			doAnimationAction(user, "heal_self");
		}
		else
		{
			doAnimationAction(user, "heal_other");
		}
		
		playClientEffectObj(user, "appearance/pt_heal.prt", target, "");
	}
	
	
	public static boolean isValidHealTarget(obj_id target) throws InterruptedException
	{
		if ((!isIdValid(target) || !exists(target) || isInvulnerable(target) || ai_lib.isDroid(target) || ai_lib.isAndroid(target) || pet_lib.getPetType(target) == pet_lib.PET_TYPE_FAMILIAR))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isValidHealTarget(obj_id healer, obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target) || isInvulnerable(target) || pet_lib.getPetType(target) == pet_lib.PET_TYPE_FAMILIAR)
		{
			return false;
		}
		
		if (ai_lib.isDroid(healer) || ai_lib.isAndroid(healer))
		{
			return true;
		}
		
		if (ai_lib.isDroid(target) || ai_lib.isAndroid(target))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static void applyLifeSiphonHeal(obj_id attacker, obj_id defender, int damage, float percentToHeal, String actionName) throws InterruptedException
	{
		applyLifeSiphonHeal(attacker, defender, damage, percentToHeal, actionName, false);
	}
	
	
	public static void applyLifeSiphonHeal(obj_id attacker, obj_id defender, int damage, float percentToHeal, String actionName, boolean healMaster) throws InterruptedException
	{
		if (percentToHeal > 0)
		{
			int damageToHeal = Math.round(damage * percentToHeal);
			
			damageToHeal = getHealingAfterReductions(attacker, attacker, damageToHeal);
			damageToHeal = getTargetHealingBonus(attacker, attacker, damageToHeal);
			
			healDamage(attacker, attacker, HEALTH, damageToHeal);
			
			prose_package pp = new prose_package();
			pp = prose.setStringId(pp, new string_id("healing", "siphon_fly"));
			pp = prose.setDI(pp, damageToHeal);
			pp = prose.setTO(pp, "Health Leech");
			
			if (actionName.endsWith("master"))
			{
				healMaster = true;
			}
			
			if (isPlayer(attacker))
			{
				prose.setTO(pp, "Drain");
			}
			
			if (isPlayer(attacker))
			{
				showFlyTextPrivateProseWithFlags(attacker, attacker , pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
				
				string_id actionStringId = new string_id("cmd_n", actionName);
				prose_package cp = new prose_package();
				cp = prose.setStringId(cp, new string_id("cbt_spam", "drain_hit"));
				cp = prose.setTU(cp, attacker);
				cp = prose.setTT(cp, defender);
				cp = prose.setDI(cp, damageToHeal);
				
				sendCombatSpamMessageProse(attacker, defender, cp, true, true, true, COMBAT_RESULT_MEDICAL);
			}
			else if (beast_lib.isBeast(attacker))
			{
				showFlyTextPrivateProseWithFlags(attacker, getMaster(attacker) , pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
			}
			
			if (isPlayer(defender))
			{
				showFlyTextPrivateProseWithFlags(attacker, defender , pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
			}
			
			if (isIdValid(getMaster(attacker)))
			{
				attacker = getMaster(attacker);
				
				if (healMaster && exists(attacker) && !isDead(attacker))
				{
					damageToHeal = getHealingAfterReductions(attacker, attacker, damageToHeal);
					damageToHeal = getTargetHealingBonus(attacker, attacker, damageToHeal);
					pp = prose.setDI(pp, damageToHeal);
					
					healDamage(attacker, attacker, HEALTH, damageToHeal);
					
					if (isPlayer(attacker))
					{
						showFlyTextPrivateProseWithFlags(attacker, attacker , pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
					}
					
					if (isPlayer(defender))
					{
						showFlyTextPrivateProseWithFlags(attacker, defender , pp, 2.0f, colors.SEAGREEN, FLY_TEXT_FLAG_IS_HEAL);
					}
				}
			}
			
		}
	}
	
	
	public static int doDiminishingReturns(int toHeal, obj_id medic) throws InterruptedException
	{
		float rightNow = getGameTime();
		
		if (!utils.hasScriptVar(medic, "healing.diminishing_returns.last_heal_time"))
		{
			utils.setScriptVar(medic, "healing.diminishing_returns.last_heal_time", rightNow);
			utils.setScriptVar(medic, "healing.diminishing_returns.heal_total", toHeal);
			buff.applyBuff(medic, "heal_diminishing_returns_1");
			return toHeal;
		}
		
		float lastHealTime = utils.getFloatScriptVar(medic, "healing.diminishing_returns.last_heal_time");
		
		if (rightNow - lastHealTime > 30f)
		{
			utils.setScriptVar(medic, "healing.diminishing_returns.last_heal_time", rightNow);
			utils.setScriptVar(medic, "healing.diminishing_returns.heal_total", toHeal);
			buff.applyBuff(medic, "heal_diminishing_returns_1");
			return toHeal;
		}
		
		int healTotal = utils.getIntScriptVar(medic, "healing.diminishing_returns.heal_total");
		int reductionThreshold = 25000;
		
		if (healTotal <= reductionThreshold)
		{
			utils.setScriptVar(medic, "healing.diminishing_returns.last_heal_time", rightNow);
			utils.setScriptVar(medic, "healing.diminishing_returns.heal_total", toHeal + healTotal);
			buff.applyBuff(medic, "heal_diminishing_returns_1");
			return toHeal;
		}
		
		float healTotalFloat = (float)healTotal;
		float toHealFloat = (float)toHeal;
		float originalHeal = toHeal;
		float diminishedAmount = 1.0f;
		
		healTotalFloat -= reductionThreshold;
		diminishedAmount = healTotalFloat / (100f + (healTotalFloat / 45f)) / 100.0f;
		toHealFloat = toHealFloat * (1.0f - diminishedAmount);
		
		toHeal = (int)toHealFloat;
		
		utils.setScriptVar(medic, "healing.diminishing_returns.last_heal_time", rightNow);
		utils.setScriptVar(medic, "healing.diminishing_returns.heal_total", toHeal + healTotal);
		
		float healNerfRatio = toHealFloat/originalHeal;
		
		if (healNerfRatio > .70f)
		{
			buff.applyBuff(medic, "heal_diminishing_returns_2");
		}
		else if (healNerfRatio > .35f)
		{
			buff.applyBuff(medic, "heal_diminishing_returns_3");
		}
		else
		{
			buff.applyBuff(medic, "heal_diminishing_returns_4");
		}
		
		return toHeal;
	}
}
