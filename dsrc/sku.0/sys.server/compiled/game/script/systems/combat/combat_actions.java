package script.systems.combat;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai_combat;
import script.library.ai_lib;
import script.library.bounty_hunter;
import script.library.armor;
import script.library.beast_lib;
import script.library.buff;
import script.library.callable;
import script.library.chat;
import script.library.colors;
import script.library.combat;
import script.library.corpse;
import script.library.craftinglib;
import script.library.create;
import script.library.dot;
import script.library.factions;
import script.library.faction_perk;
import script.library.grenade;
import script.library.groundquests;
import script.library.group;
import script.library.healing;
import script.library.innate;
import script.library.instance;
import script.library.locations;
import script.library.movement;
import script.library.performance;
import script.library.pet_lib;
import script.library.prose;
import script.library.smuggler;
import script.library.space_dungeon;
import script.library.space_utils;
import script.library.squad_leader;
import script.library.stealth;
import script.library.sui;
import script.library.trial;
import script.library.utils;
import script.library.vehicle;
import script.library.weapons;
import script.library.heavyweapons;

import script.systems.missions.dynamic.bounty_probe_droid;
import java.lang.Math;



public class combat_actions extends script.systems.combat.combat_base
{
	public combat_actions()
	{
	}
	String DEFAULT_EGG = "object/tangible/gravestone/gravestone05.tpf";
	public static final String COMBAT_TABLE = "datatables/combat/combat_data.iff";
	public static final float DAMAGE_STRENGTH_CUTOFF = .5f;
	
	
	public int OnChangedPosture(obj_id self, int oldPosture, int newPosture) throws InterruptedException
	{
		
		if (newPosture == POSTURE_INCAPACITATED || newPosture == POSTURE_DEAD || newPosture == POSTURE_KNOCKED_DOWN)
		{
			return SCRIPT_CONTINUE;
		}
		
		setState(self, STATE_IMMOBILIZED, false);
		stealth.OnPostureChanged(self, oldPosture, newPosture);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnEnteredCombat(obj_id self) throws InterruptedException
	{
		combat.cacheCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnExitedCombat(obj_id self) throws InterruptedException
	{
		combat.clearCachedCombatData(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePistolLauncherTargeting(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePistolLauncherTargeting", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireAcidBeam(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireAcidBeam", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireLavaCannon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireLavaCannon", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireLavaCannonGeneric(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireLavaCannonGeneric", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireFlameThrowerLight(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireFlameThrowerLight", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePlasmaFlameThrower(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePlasmaFlameThrower", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireIceGun(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireIceGun", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireRocketLauncher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireRocketLauncher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireRocketLauncherGeneric(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireRocketLauncherGeneric", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireAcidRifle(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireAcidRifle", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireStunCannon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireStunCannon", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireParticleBeam(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireParticleBeam", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireVoidRocketLauncher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireVoidRocketLauncher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireLightningBeam(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireLightningBeam", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireAcidBeamAvatar(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireAcidBeamAvatar", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireRepublicFlameThrower(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireRepublicFlameThrower", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireRepublicFlameThrowerGeneric(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireRepublicFlameThrowerGeneric", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePistolLauncher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePistolLauncher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireElitePistolLauncher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireElitePistolLauncher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePistolLauncherGeneric(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePistolLauncherGeneric", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePistolLauncherMedium(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePistolLauncherMedium", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireCrusaderHeavyRifle(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireCrusaderHeavyRifle", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePvpHeavy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePvpHeavy", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireHeavyShotgun(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireHeavyShotgun", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int banner_buff_commando(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("banner_buff_commando", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stand(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (ai_lib.isAiDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIncapacitated(self) && !isDead(self))
		{
			setPosture(self, POSTURE_UPRIGHT);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int standFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (space_utils.isInStation(self))
		{
			LOG("space", "Leaving station!");
			queueCommand(self, (616891114), null,"", COMMAND_PRIORITY_FRONT);
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kneel(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIncapacitated(self) && !isDead(self))
		{
			setPosture(self, POSTURE_CROUCHED);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int kneelFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int prone(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIncapacitated(self) && !isDead(self))
		{
			setPosture(self, POSTURE_PRONE);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int proneFail(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int veteranPlayerBuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("veteranPlayerBuff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceRun(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("forceRun", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_detect_camouflage_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDetectCamouflage(self) || !combatStandardAction("bh_detect_camouflage_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float detectSkill = getEnhancedSkillStatisticModifier(self, "detect_hidden");
		float distance = stealth.BASE_DETECT_CAMOUFLAGE_DISTANCE + (detectSkill / 20);
		
		float detectChance = 50.0f;
		
		stealth.detectCamouflage(self, true, true, distance, detectChance);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_ae_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_ae_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_ae_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_ae_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_dm_cc_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_dm_cc_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_dm_cc_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_dm_cc_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_del_dm_cc_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_del_dm_cc_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_8");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm") / 10;
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_flawless_strike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_flawless_strike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_flawless_strike");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_bh_flawless_strike");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int set_bonus_bh_utility_a_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("set_bonus_bh_utility_a_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int set_bonus_bh_utility_a_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("set_bonus_bh_utility_a_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int set_bonus_bh_utility_a_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("set_bonus_bh_utility_a_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doDireAbility(obj_id attacker, obj_id defender, int direType) throws InterruptedException
	{
		int bonusChance = 0;
		
		switch(direType)
		{
			case 0:
			int direSnare = buff.getBuffOnTargetFromGroup(defender, "dire_snare");
			if (direSnare != 0)
			{
				return;
			}
			bonusChance = getEnhancedSkillStatisticModifierUncapped(attacker, "bh_dire_snare");
			break;
			case 1:
			bonusChance = getEnhancedSkillStatisticModifierUncapped(attacker, "bh_dire_root");
			break;
			default:
			return;
		}
		
		if (bonusChance > 0)
		{
			int roll = rand(1, 99);
			int total = roll+bonusChance;
			
			if (total > 99)
			{
				
				if (direType == 0)
				{
					buff.applyBuff(defender, attacker, "bh_dire_snare_1");
					showFlyText(defender, new string_id("set_bonus", "dire_snare_fly"), 2, colors.BLACK);
				}
				else
				{
					queueCommand(attacker, (-2097394013), defender, "", COMMAND_PRIORITY_IMMEDIATE);
					showFlyTextPrivate(attacker, attacker, new string_id("set_bonus", "dire_trap_fly"), 2, colors.LEMONCHIFFON);
				}
			}
		}
		return;
	}
	
	
	public boolean successfulFastAttack(obj_id attacker, String specialLine) throws InterruptedException
	{
		return successfulFastAttack(attacker, specialLine, "fast_attack_fly");
	}
	
	
	public boolean successfulFastAttack(obj_id attacker, String specialLine, String customFlyReference) throws InterruptedException
	{
		boolean fastAttackOrWhat = false;
		
		int bonusChance = getEnhancedSkillStatisticModifierUncapped(attacker, "fast_attack_line_"+ specialLine);
		
		if (bonusChance > 0)
		{
			int roll = rand(1, 99);
			int total = roll+bonusChance;
			
			if (total > 99)
			{
				fastAttackOrWhat = true;
				showFlyText(attacker, new string_id("set_bonus", customFlyReference), 2, colors.LEMONCHIFFON);
				
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("set_bonus", customFlyReference + "_cbspam"));
				pp = prose.setTT(pp, attacker);
				sendCombatSpamMessageProse(attacker, pp, COMBAT_RESULT_GENERIC);
			}
		}
		
		return fastAttackOrWhat;
	}
	
	
	public int bh_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		if (successfulFastAttack(self, "dm_cc"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		if (successfulFastAttack(self, "dm_cc"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		if (successfulFastAttack(self, "dm_cc"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bh_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bh_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bh_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bh_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_kill_trap_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDisarmTrap(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		stealth.disarmTrap(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_ae_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_ae_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_ae_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_ae_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_ae_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_ae_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_cc_1_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_cc_1_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_cc_1_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_cc_1_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_cc_1_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_cc_1_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_feeling_lucky_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_feeling_lucky_recourse"))
		{
			buff.applyBuff(self, self, "sm_feeling_lucky_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_lucky_break_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_lucky_break_recourse"))
		{
			buff.applyBuff(self, self, "sm_lucky_break_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_double_fire_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_double_fire_recourse"))
		{
			buff.applyBuff(self, self, "sm_double_fire_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_false_hope_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_false_hope_recourse"))
		{
			buff.applyBuff(self, self, "sm_false_hope_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_break_the_deal_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_break_the_deal_recourse"))
		{
			buff.applyBuff(self, self, "sm_break_the_deal_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_melee_stun_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_melee_stun_recourse"))
		{
			buff.applyBuff(self, self, "sm_melee_stun_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int maladyRecourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "insidiousMalady_1"))
		{
			buff.applyBuff(self, self, "insidiousMalady_1");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int maladyRecourse_1(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "insidiousMalady_2"))
		{
			buff.applyBuff(self, self, "insidiousMalady_2");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int maladyRecourse_2(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "insidiousMalady_3"))
		{
			buff.applyBuff(self, self, "insidiousMalady_3");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int maladyRecourse_3(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "insidiousMalady_4"))
		{
			buff.applyBuff(self, self, "insidiousMalady_4");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_cc_2_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_cc_2_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_cc_2_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_cc_2_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_del_ae_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_del_ae_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "co_grenade"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_fld_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_fld_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_fld_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_fld_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
			
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_tracer_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_tracer_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_tracer_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_tracer_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_tracer_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_tracer_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_tracer_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_tracer_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_grenade_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_grenade_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_grenade_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_grenade_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_grenade_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_grenade_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_shock_grenade_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_shock_grenade_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_enrage_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_enrage_1", self, target, params, "", "") || !buff.hasBuff(self, "set_bonus_commando_utility_a_3"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "co_enrage_1", "healthCost");
		
		int currentHealth = getHealth(self);
		int modifiedHealth = currentHealth - healthCost;
		
		attrib_mod[] healthMods = getHealthModifiers(self);
		int healthModsValue = 0;
		
		for (int i = 0; i < healthMods.length; i++)
		{
			testAbortScript();
			healthModsValue += healthMods[i].getValue();
		}
		
		if (healthModsValue < 0)
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (modifiedHealth < 1)
		{
			showFlyTextPrivate(self, self, new string_id("spam", "low_health"), 1.0f, colors.RED);
			return SCRIPT_OVERRIDE;
		}
		
		setHealth(self, modifiedHealth);
		incrementKillMeter(self, 10);
		
		playClientEffectObj(self, "clienteffect/medic_reckless_stimulation.cef", self, "root");
		
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, new string_id("set_bonus", "enraged_cmb_spam"));
		pp = prose.setTT(pp, self);
		sendCombatSpamMessageProse(self, pp, COMBAT_RESULT_GENERIC);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_stand_fast(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_stand_fast", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("co_stand_fast");
		LOG ("stand_fast", "baseCooldownTime is "+ baseCooldownTime);
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_co_stand_fast");
		LOG ("stand_fast", "cooldownTimeMod is "+ cooldownTimeMod);
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int eggDetonated(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "objEgg");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean hasEgg(obj_id player) throws InterruptedException
	{
		obj_id egg = utils.getObjIdScriptVar(player, "objEgg");
		
		if (isValidId(egg) && utils.hasScriptVar(egg, "objOwner"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public boolean detonateEgg(obj_id self, boolean eggCreated) throws InterruptedException
	{
		
		if (eggCreated && hasEgg(self))
		{
			dictionary particleParams = new dictionary();
			dictionary attackParams = new dictionary();
			
			obj_id objEgg = utils.getObjIdScriptVar(self, "objEgg");
			
			particleParams = utils.getDictionaryScriptVar(objEgg, "handleDelayedParticle");
			attackParams = utils.getDictionaryScriptVar(objEgg, "handleDelayedAttack");
			
			messageTo(objEgg, "handleDelayedParticle", particleParams, 0, false);
			messageTo(objEgg, "handleDelayedAttack", attackParams, 0, false);
			
			utils.removeScriptVar(self, "objEgg");
			
			return true;
		}
		
		return false;
	}
	
	
	public int co_remote_detonator_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		boolean eggCreated = hasEgg(self);
		
		if (!eggCreated && !combatStandardAction("co_remote_detonator_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!eggCreated && hasEgg(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!detonateEgg(self, eggCreated))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_remote_detonator_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		boolean eggCreated = hasEgg(self);
		
		if (!eggCreated && !combatStandardAction("co_remote_detonator_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!eggCreated && hasEgg(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!detonateEgg(self, eggCreated))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_remote_detonator_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		boolean eggCreated = hasEgg(self);
		
		if (!eggCreated && !combatStandardAction("co_remote_detonator_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!eggCreated && hasEgg(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!detonateEgg(self, eggCreated))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_remote_detonator_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		boolean eggCreated = hasEgg(self);
		
		if (!eggCreated && !combatStandardAction("co_remote_detonator_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!eggCreated && hasEgg(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!detonateEgg(self, eggCreated))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_remote_detonator_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		boolean eggCreated = hasEgg(self);
		
		if (!eggCreated && !combatStandardAction("co_remote_detonator_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!eggCreated && hasEgg(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!detonateEgg(self, eggCreated))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_stim_armor(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_stim_armor", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kill_meter_co_it_burns_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kill_meter_co_it_burns_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kill_meter_co_armor_splash_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kill_meter_co_armor_splash_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kill_meter_co_youll_regret_that_reac(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kill_meter_co_youll_regret_that_reac", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_co_burst_fire_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_co_burst_fire_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_position_secured(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (buff.toggleStance(self, "co_position_secured"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("co_position_secured", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		vehicle.storeAllVehicles(self);
		
		showFlyText(self, new string_id("spam", "co_position_secured"), 2, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_suppressing_fire(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_suppressing_fire", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_riddle_armor(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_riddle_armor", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("co_riddle_armor");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_co_riddle_armor");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_armor_cracker(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_armor_cracker", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("co_armor_cracker");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_co_riddle_armor");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ofAggroChannel(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!group.inSameGroup(self, target) || target == self)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_aggro_channel", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_mirror_armor(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_mirror_armor", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		combat.mirrorArmor(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("en_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("en_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("en_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_conceal_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_conceal_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_conceal_device_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_conceal_device_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_neutralize_device_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDisarmTrap(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		stealth.disarmTrap(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hide_device_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hide_device_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_saber_reflect_buff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInStance(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_stance"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_saber_reflect_buff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_saber_reflect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInStance(self) || !buff.hasBuff(self, "fs_saber_reflect"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_saber_reflect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyTextPrivate(self, self, new string_id("combat_effects", "saber_reflect"), 1.5f, colors.ORANGERED);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int saberBlock(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("saberBlock", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sense_danger_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDetectCamouflage(self) || !combatStandardAction("fs_sense_danger_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float detectSkill = getEnhancedSkillStatisticModifier(self, "detect_hidden");
		float distance = stealth.BASE_DETECT_CAMOUFLAGE_DISTANCE + (detectSkill / 20);
		
		float detectChance = 50.0f;
		
		stealth.detectCamouflage(self, true, true, distance, detectChance);
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_forsake_fear(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInStance(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_stance"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_forsake_fear", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		string_id promptString = new string_id("healing", "fs_forsake_fear_channel");
		
		int flags = sui.CD_EVENT_INCAPACITATE;
		
		int pid = sui.smartCountdownTimerSUI(self, self, "channelForsakeFearCountdown", promptString, 0, 10, "channelForsakeFearCountdownHandler", 1f, flags);
		
		utils.setScriptVar(self, "buff_handler.ForsakeFearSUIPID", pid);
		
		dictionary parms = new dictionary();
		
		parms.put("player", self);
		parms.put("buffName", "fs_forsake_fear");
		
		messageTo(self, "checkChannelForsakeFear", parms, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_force_spark(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_force_spark", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_ae_dm_cc_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_ae_dm_cc_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_ae_dm_cc_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_ae_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_maelstrom_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_maelstrom_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_maelstrom_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_maelstrom_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_maelstrom_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_maelstrom_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_maelstrom_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_maelstrom_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_maelstrom_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_maelstrom_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_buff_ca_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (buff.toggleStance(self, "fs_buff_ca_1"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_buff_ca_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_buff_def_1_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (buff.toggleStance(self, "fs_buff_def_1_1"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_buff_def_1_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_flurry_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_flurry_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_flurry_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_flurry_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_flurry_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_flurry_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_flurry_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_flurry_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sweep_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_sweep_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_sweep_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_sweep") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_mind_trick_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_mind_trick_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		ai_lib.mindTrick(self, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_mind_trick_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_mind_trick_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		combat_data actionData = combat_engine.getCombatData("fs_mind_trick_2");
		float actionRadius = actionData.coneLength;
		
		actionRadius += getEnhancedSkillStatisticModifierUncapped(self, "expertise_area_size_single_fs_mind_trick_2");
		
		obj_id[] defenders = pvpGetTargetsInRange(self, target, actionRadius);
		
		for (int i = 0; i < defenders.length; i++)
		{
			testAbortScript();
			ai_lib.mindTrick(self, defenders[i]);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int fs_dm_cc_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_dm_cc_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_crit_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_cc_crit_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_cc_crit_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_cc_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_crit_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_cc_crit_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_cc_crit_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_cc_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_crit_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_cc_crit_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_cc_crit_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_cc_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_crit_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_cc_crit_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_cc_crit_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_cc_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_dm_cc_crit_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fs_dm_cc_crit_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "fs_powers", "fury_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("fs_dm_cc_crit_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_fs_cc_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_hermetic_touch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!buff.isInStance(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_hermetic_touch", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_set_heroic_taunt_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!buff.hasBuff(self, "set_bonus_jedi_utility_b_3"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_set_heroic_taunt_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_ae_heal_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_ae_heal_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_buff_health_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_health"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_buff_health_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_dm_dot_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_dm_dot_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		float baseCooldownTime = getBaseCooldownTime("me_dm_dot_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	public int me_fld_dm_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_fld_dm_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_fld_dm_dot_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_fld_dm_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_fld_dm_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_fld_dm_dot_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_fld_dm_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_fld_dm_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_fld_dm_dot_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_dot");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_hot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_hot_1", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_hot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_hot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_hot_2", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_hot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_hot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_hot_3", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_hot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_rv_ooc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_rv_ooc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_revive", "quick_revive_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_rv_ooc");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_revive");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_rv_area(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_rv_area", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_revive", "quick_revive_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_rv_area");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_revive");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_rv_combat(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("me_rv_combat", self, self, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_revive", "quick_revive_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_rv_combat");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_aoe_revive");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod * 60) );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_rv_pvp_single(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || space_dungeon.isInSpaceDungeon(self) || instance.isInInstanceArea(self) || !utils.hasScriptVar(target, "pvp_death"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_rv_pvp_single", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_revive", "quick_revive_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_rv_pvp_single");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_revive");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_rv_pvp_area(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (space_dungeon.isInSpaceDungeon(self) || instance.isInInstanceArea(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_rv_pvp_area", self, self, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_revive", "quick_revive_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_rv_pvp_area");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_aoe_revive");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod * 60) );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_drag_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!group.isGrouped(self))
		{
			sendSystemMessage(self, new string_id("spam", "drag_not_grouped") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_drag_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		corpse.dragPlayerCorpse(self, corpse.getDraggableCorpsesInRange(self, true) );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_blood_cleaners_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.hasBuff(target, "incapWeaken"))
		{
			if (target == self)
			{
				sendSystemMessage(self, new string_id("spam", "remove_weakness_self_fail") );
			}
			else
			{
				sendSystemMessage(self, new string_id("spam", "remove_weakness_other_fail") );
			}
			
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_blood_cleaners_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		buff.removeBuff(target, "incapWeaken");
		playClientEffectObj(target, "appearance/pt_heal_2.prt", target, "root");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_bomb_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_1", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_bomb_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_bomb_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_2", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_bomb_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_bomb_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_3", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_bomb_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_bomb_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_4", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_bomb_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_bomb_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_5", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_bomb_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_1", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_2", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_3", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_4", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_5", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_ampule_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_ampule_6", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_ampule_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cure_affliction_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_cure_affliction_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_serotonin_boost_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_serotonin_boost_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_grenade_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_grenade_1", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_grenade_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_grenade_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_grenade_2", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_grenade_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_grenade_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_grenade_3", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_grenade_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_grenade_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_grenade_4", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_grenade_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_grenade_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target) || !isInAttackRange(self, target, "me_bacta_bomb_5", false))
		{
			target = self;
		}
		
		if (!combatStandardAction("me_bacta_grenade_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (target != self && pvpCanHelp(self, target) && successfulFastAttack(self, "me_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_action_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_action_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_action_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_action_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_action_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_action_2", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_action_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_action_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_action_3", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_1", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_1", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_2", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_2", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_3", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_3", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_4", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_4", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_5", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_5", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_reckless_stimulation_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (hasFullActionPool(self))
		{
			showFlyTextPrivate(self, self, new string_id("spam", "full_action_pool"), 2, colors.LEMONCHIFFON);
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "cloning_sickness"))
		{
			sendSystemMessage(self, new string_id("spam", "not_well_enough") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_reckless_stimulation_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int healthCost = (int)dataTableGetFloat(COMBAT_TABLE, "me_reckless_stimulation_6", "healthCost");
		int actionGain = dataTableGetInt(COMBAT_TABLE, "me_reckless_stimulation_6", "addedDamage");
		
		healthToActionConversion(self, healthCost, actionGain);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_stasis_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		int stasisBuff = buff.getBuffOnTargetFromGroup(target, "stasis");
		if (stasisBuff != 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!buff.canApplyBuff(target, "me_stasis_1"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id testTarget = getIntendedTarget(self);
		
		if ((isIdValid(testTarget) && ( (group.inSameGroup(self, testTarget) && pvpCanHelp(self, testTarget) ) || pvpCanAttack(self, testTarget) ) ))
		{
			target = testTarget;
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (getDistance(self, target) > 48.0f)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_stasis_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		buff.applyBuff(target, self, "me_stasis_1");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_stasis_self_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int stasisBuff = buff.getBuffOnTargetFromGroup(self, "stasis");
		if (stasisBuff != 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!buff.canApplyBuff(self, "me_stasis_self_1"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_stasis_self_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("me_stasis_self_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_stasis_self");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod*60) );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_serotonin_purge_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_serotonin_purge_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_induce_insanity_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_induce_insanity_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_bacta_resistance_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_bacta_resistance_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_electrolyte_drain_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_electrolyte_drain_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_traumatize_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(target, "action_drain_immunity"))
		{
			sendSystemMessage(self, new string_id("spam", "immune_to_that") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_traumatize_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_traumatize_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(target, "action_drain_immunity"))
		{
			sendSystemMessage(self, new string_id("spam", "immune_to_that") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_traumatize_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_traumatize_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(target, "action_drain_immunity"))
		{
			sendSystemMessage(self, new string_id("spam", "immune_to_that") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_traumatize_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_traumatize_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(target, "action_drain_immunity"))
		{
			sendSystemMessage(self, new string_id("spam", "immune_to_that") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_traumatize_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_traumatize_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(target, "action_drain_immunity"))
		{
			sendSystemMessage(self, new string_id("spam", "immune_to_that") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_traumatize_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_thyroid_rupture_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (vehicle.isVehicle(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("me_thyroid_rupture_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDoom(self, target);
		
		if (successfulFastAttack(self, "me_debuff", "fast_attack_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_strength_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_strength_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_strength_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_strength_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_strength_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_strength_2", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_strength_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_strength_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_strength_3", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_burst_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_burst_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_burst_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_burst_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_burst_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_burst_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_burst_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_burst_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_burst_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_burst_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_agility_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_agility_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_agility_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_agility_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_agility_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_agility_2", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_agility_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_agility_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_agility_3", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cranial_smash_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_cranial_smash_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cranial_smash_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_cranial_smash_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cranial_smash_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_cranial_smash_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cranial_smash_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_cranial_smash_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_cranial_smash_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_cranial_smash_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_evasion_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("me_evasion_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doMedicEvasion(self);
		
		float baseCooldownTime = getBaseCooldownTime("me_evasion_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_me_evasion");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_precision_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_precision_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_precision_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_precision_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_precision_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_precision_2", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_precision_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_precision_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_precision_3", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_block_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_melee_gb_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_block_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_enhance_dodge_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_ranged_gb_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIdValid(target) || !pvpCanHelp(self, target))
		{
			target = self;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_enhance_dodge_1", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_buff_health_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_health_0"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_buff_health_2", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int me_buff_health_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !pvpCanHelp(self, target) || vehicle.isVehicle(target) || isDead(target))
		{
			target = self;
		}
		
		if (!buff.canApplyBuff(target, "me_buff_health_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		boolean performed_buff = performMedicGroupBuff(self, target, "me_buff_health_3", params);
		
		if (!performed_buff)
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_dueterium_rounds_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_dueterium_rounds_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_poison_knuckle_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_poison_knuckle_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doDoom(obj_id attacker, obj_id defender) throws InterruptedException
	{
		int bonusChance = getEnhancedSkillStatisticModifierUncapped(attacker, "me_doom_chance");
		
		if (bonusChance > 0)
		{
			int roll = rand(1, 99);
			int total = roll+bonusChance;
			
			if (roll + bonusChance > 99 && !utils.hasScriptVar(defender, "me_doom.doom_owner"))
			{
				utils.setScriptVar(defender, "me_doom.doom_owner", attacker);
				utils.setScriptVar(defender, "me_doom.doom_stage", 1);
				
				buff.applyBuff(defender, attacker, "me_doom", 18.0f, 1.0f);
				
				if (hasCommand(attacker, "me_bacta_resistance_1"))
				{
					buff.applyBuff(defender, attacker, "me_bacta_resistance_1");
				}
				
				if (hasCommand(attacker, "me_electrolyte_drain_1"))
				{
					buff.applyBuff(defender, attacker, "me_electrolyte_drain_1");
				}
				
				if (hasCommand(attacker, "me_induce_insanity_1"))
				{
					buff.applyBuff(defender, attacker, "me_rheumatic_calamity_1");
				}
				
				if (hasCommand(attacker, "me_thyroid_rupture_1"))
				{
					buff.applyBuff(defender, attacker, "me_thyroid_rupture_1");
				}
			}
		}
		
		return;
	}
	
	
	public int of_buff_def_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_2");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_3");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_ae_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_ae_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_ae_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_ae_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_ae_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_ae_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_del_ae_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package p = prose.getPackage(new string_id("spam", "artillery_coming"));
		commPlayers(self, "object/mobile/npe/npe_hutt_minion.iff", "sound/sys_comm_other.snd", 5f, self, p);
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_del_ae_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package p = prose.getPackage(new string_id("spam", "hailfire_coming"));
		commPlayers(self, "object/mobile/npe/npe_hutt_minion.iff", "sound/sys_comm_other.snd", 5f, self, p);
		playClientEffectLoc(self, "clienteffect/combat_pt_aerialstrike.cef", getLocation(target), 0);
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_del_ae_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package p = prose.getPackage(new string_id("spam", "orbitalstrike_coming"));
		commPlayers(self, "object/mobile/npe/npe_hutt_minion.iff", "sound/sys_comm_other.snd", 5f, self, p);
		playClientEffectLoc(self, "clienteffect/combat_pt_orbitalstrike.cef", getLocation(target), 0);
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_del_ae_dm_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_del_ae_dm_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_del_ae_dm_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_del_ae_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_pistol_dm(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_pistol_dm", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int level = getLevel(self);
		if (level >= 74)
		{
			doInspiredAction(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_pistol_bleed(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_pistol_bleed", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int level = getLevel(self);
		if (level >= 74)
		{
			doInspiredAction(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_rally_point_off(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		location eggLoc = combat.getCommandGroundTargetLocation(params);
		
		location myLoc = getLocation(self);
		
		float distance = getDistance(myLoc, eggLoc);
		
		obj_id objEgg = createObject("object/tangible/space/content_infrastructure/generic_egg_small.iff", eggLoc);
		
		if (!isIdValid(objEgg))
		{
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(objEgg, "objOwner", self);
		utils.setScriptVar(self, "objEgg", objEgg);
		utils.setScriptVar(objEgg, "triggerBuff", "of_rally_point_off");
		int duration = combat.EGG_AURA_DURATION;
		duration += (int)getEnhancedSkillStatisticModifierUncapped(self, "rally_point_duration");
		utils.setScriptVar(objEgg, "triggerDuration", duration);
		utils.setScriptVar(objEgg, "triggerPrt", "appearance/pt_rally_point_red.prt");
		
		attachScript(objEgg, "systems.combat.combat_trigger_area");
		
		combat_data cData = combat_engine.getCombatData("of_rally_point_off");
		obj_id weapon = getCurrentWeapon(self);
		weapon_data wData = getWeaponData(weapon);
		
		int[] actionCost = combat.getActionCost(self, wData, cData);
		
		combat.drainCombatActionAttributes(self, actionCost);
		
		float baseCooldownTime = getBaseCooldownTime("of_rally_point_off");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		if (group.isGrouped(self))
		{
			obj_id groupId = getGroupObject(self);
			obj_id[] members = getGroupMemberIds(groupId);
			prose_package pp = new prose_package();
			
			prose.setStringId(pp, new string_id("squad_leader", "of_set_rally_point"));
			prose.setTT(pp, self);
			
			if (members != null && members.length > 0)
			{
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					combat.sendCombatSpamMessageProse(members[i], pp);
				}
			}
			squad_leader.sendSquadLeaderCommand( self, "Rallypoint has been set!");
		}
		else
		{
			combat.sendCombatSpamMessage(self, new string_id("squad_leader", "self_set_rally_point"));
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_rally_point_def(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location eggLoc = combat.getCommandGroundTargetLocation(params);
		
		location myLoc = getLocation(self);
		
		float distance = getDistance(myLoc, eggLoc);
		
		obj_id objEgg = createObject("object/tangible/space/content_infrastructure/generic_egg_small.iff", eggLoc);
		
		if (!isIdValid(objEgg))
		{
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(objEgg, "objOwner", self);
		utils.setScriptVar(self, "objEgg", objEgg);
		utils.setScriptVar(objEgg, "triggerBuff", "of_rally_point_def");
		int duration = combat.EGG_AURA_DURATION;
		duration += (int)getEnhancedSkillStatisticModifierUncapped(self, "rally_point_duration");
		utils.setScriptVar(objEgg, "triggerDuration", duration);
		utils.setScriptVar(objEgg, "triggerPrt", "appearance/pt_rally_point_blue.prt");
		
		attachScript(objEgg, "systems.combat.combat_trigger_area");
		
		combat_data cData = combat_engine.getCombatData("of_rally_point_def");
		obj_id weapon = getCurrentWeapon(self);
		weapon_data wData = getWeaponData(weapon);
		
		int[] actionCost = combat.getActionCost(self, wData, cData);
		
		combat.drainCombatActionAttributes(self, actionCost);
		
		float baseCooldownTime = getBaseCooldownTime("of_rally_point_def");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		if (group.isGrouped(self))
		{
			obj_id groupId = getGroupObject(self);
			obj_id[] members = getGroupMemberIds(groupId);
			prose_package pp = new prose_package();
			
			prose.setStringId(pp, new string_id("squad_leader", "of_set_rally_point"));
			prose.setTT(pp, self);
			
			if (members != null && members.length > 0)
			{
				for (int i = 0; i < members.length; ++i)
				{
					testAbortScript();
					combat.sendCombatSpamMessageProse(members[i], pp);
				}
			}
			squad_leader.sendSquadLeaderCommand( self, "Rallypoint has been set!");
		}
		else
		{
			combat.sendCombatSpamMessage(self, new string_id("squad_leader", "self_set_rally_point"));
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_8");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_sh_0");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_heal");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_sh_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_heal");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_sh_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_heal");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doInspiredAction(self);
		
		if (successfulFastAttack(self, "of_heal", "quick_heal_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_sh_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_heal");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_drop_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_drop_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 0))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_drop_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_4"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_4");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_5"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_5");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_6"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_6");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_7"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_7");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_8"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_8");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_buff_def_9(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_buff_def_9"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_buff_def_9", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_buff_def_9");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_1"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_2"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_2");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_3"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_3");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_4"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_4");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_5"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_5");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_focus_fire_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.canApplyBuff(self, "of_focus_fire_6"))
		{
			sendSystemMessage(self, new string_id("spam", "buff_wont_stack") );
			sendCombatSpamMessage(self, new string_id("spam", "buff_wont_stack"), COMBAT_RESULT_GENERIC);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_focus_fire_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_focus_fire_6");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_2");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_3");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_4");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_5");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_inspiration_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_inspiration_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_inspiration_6");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_scatter_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_scatter_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_scatter_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_charge_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_charge_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_charge_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 1))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 2))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 3))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 4))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 5))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_medical_sup_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_medical_sup_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 6))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_medical_sup_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 7))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 8))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 9))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 10))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 11))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_tactical_sup_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!canCallSupplyDrop(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_tactical_sup_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callSupplyDrop(self, 12))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_tactical_sup_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_reinforcements_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (pet_lib.hasMaxStoredPetsOfType (self, pet_lib.PET_TYPE_NPC))
		{
			sendSystemMessage (self, pet_lib.SID_SYS_CALL_MAX_DATAPAD);
			return SCRIPT_OVERRIDE;
		}
		
		if (!canCallSupplyDrop(self) || pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC))
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean droidPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_DROID);
		boolean npcPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC);
		if (droidPets || npcPets)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_reinforcements_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!callSupplyDrop(self, 13))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_reinforcements_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_reinforcements_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (pet_lib.hasMaxStoredPetsOfType (self, pet_lib.PET_TYPE_NPC))
		{
			sendSystemMessage (self, pet_lib.SID_SYS_CALL_MAX_DATAPAD);
			return SCRIPT_OVERRIDE;
		}
		
		if (!canCallSupplyDrop(self) || pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC))
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean droidPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_DROID);
		boolean npcPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC);
		if (droidPets || npcPets)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_reinforcements_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!callSupplyDrop(self, 14))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_reinforcements_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_reinforcements_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (pet_lib.hasMaxStoredPetsOfType (self, pet_lib.PET_TYPE_NPC))
		{
			sendSystemMessage (self, pet_lib.SID_SYS_CALL_MAX_DATAPAD);
			return SCRIPT_OVERRIDE;
		}
		
		if (!canCallSupplyDrop(self) || pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC))
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean droidPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_DROID);
		boolean npcPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC);
		if (droidPets || npcPets)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_reinforcements_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!callSupplyDrop(self, 15))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_reinforcements_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_reinforcements_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (pet_lib.hasMaxStoredPetsOfType (self, pet_lib.PET_TYPE_NPC))
		{
			sendSystemMessage (self, pet_lib.SID_SYS_CALL_MAX_DATAPAD);
			return SCRIPT_OVERRIDE;
		}
		
		if (!canCallSupplyDrop(self) || pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC))
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean droidPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_DROID);
		boolean npcPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC);
		if (droidPets || npcPets)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_reinforcements_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!callSupplyDrop(self, 16))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_reinforcements_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_reinforcements_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (pet_lib.hasMaxStoredPetsOfType (self, pet_lib.PET_TYPE_NPC))
		{
			sendSystemMessage (self, pet_lib.SID_SYS_CALL_MAX_DATAPAD);
			return SCRIPT_OVERRIDE;
		}
		
		if (!canCallSupplyDrop(self) || pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC))
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean droidPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_DROID);
		boolean npcPets = pet_lib.hasMaxPets(self, pet_lib.PET_TYPE_NPC);
		if (droidPets || npcPets)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_reinforcements_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!callSupplyDrop(self, 17))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_reinforcements_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sup") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_firepower_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_firepower_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_firepower_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_firepower") * 60;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_deb_def_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_deb_def_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_deb_def_8");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_paint");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_decapitate_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_decapitate_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_vortex_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_vortex_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_drillmaster_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_drillmaster_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_drillmaster_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "cooldown_percent_of_group_buff");
		baseCooldownTime = baseCooldownTime - (baseCooldownTime * (cooldownTimeMod/100) );
		
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0.0f;
		}
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_stimulator_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_stimulator_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_stimulator_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_purge");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_purge_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_purge_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_stimulator_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_purge");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void doInspiredAction(obj_id officer) throws InterruptedException
	{
		int bonusChance = getEnhancedSkillStatisticModifierUncapped(officer, "of_inspired_action_chance");
		
		if (bonusChance > 0)
		{
			int roll = rand(1, 99);
			int total = roll+bonusChance;
			
			if (total > 99)
			{
				buff.applyBuff(officer, officer, "of_inspiration_6");
				showFlyText(officer, new string_id("set_bonus", "inspired_action_fly"), 2, colors.LEMONCHIFFON);
				
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, new string_id("set_bonus", "inspired_action_fly_cbspam"));
				pp = prose.setTT(pp, officer);
				sendCombatSpamMessageProse(officer, pp, COMBAT_RESULT_GENERIC);
			}
		}
		
		return;
		
	}
	
	
	public int of_last_words_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "of_last_words_recourse"))
		{
			buff.applyBuff(self, self, "of_last_words_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_disarm_trap_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDisarmTrap(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		stealth.disarmTrap(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_melee_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_melee_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int sm_ae_cover_fire(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_cover_fire", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_pin_down(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_pin_down", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_ae_dm_cc_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_ae_dm_cc_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_del_dm_cc_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_del_dm_cc_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_fast_draw(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_fast_draw", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_precision_strike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_precision_strike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_melee_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_melee_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sm_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_melee_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_concussion_shot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_concussion_shot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dizzy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dizzy", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int sm_dm_cc_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_cc_melee_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_cc_melee_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doDireAbility(self, target, 0);
		
		float baseCooldownTime = getBaseCooldownTime("sm_dm_cc_melee_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_dm_cc") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_dm_dot_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_dm_dot_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_bad_odds_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_bad_odds_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_bad_odds_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_bad_odds_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_bad_odds_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_bad_odds_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_bad_odds_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_bad_odds_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_bad_odds_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_bad_odds_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_summon_smuggler(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_summon_smuggler", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callFavor(self, 0))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_summon_medic(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_summon_medic", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callFavor(self, 1))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_sly_lie(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!smuggler.canSlyLie(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_sly_lie", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!smuggler.doSlyLie(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_fast_talk(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (isIdValid(target) && !smuggler.canFastTalk(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_fast_talk", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!smuggler.doFastTalk(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_skullduggery(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_skullduggery", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_impossible_odds(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_impossible_odds", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_nerf_herder(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_nerf_herder", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_pistol_whip_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_pistol_whip_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_pistol_whip_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_pistol_whip_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_pistol_whip_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_pistol_whip_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_pistol_whip_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_pistol_whip_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_shoot_first_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (isBeingHuntedByBountyHunter(self, target))
		{
			pvpSetPersonalEnemyFlag(self, target);
			pvpSetPersonalEnemyFlag(target, self);
		}
		
		if (!combatStandardAction("sm_shoot_first_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_shoot_first_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (isBeingHuntedByBountyHunter(self, target))
		{
			pvpSetPersonalEnemyFlag(self, target);
			pvpSetPersonalEnemyFlag(target, self);
		}
		
		if (!combatStandardAction("sm_shoot_first_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_shoot_first_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (isBeingHuntedByBountyHunter(self, target))
		{
			pvpSetPersonalEnemyFlag(self, target);
			pvpSetPersonalEnemyFlag(target, self);
		}
		
		if (!combatStandardAction("sm_shoot_first_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_shoot_first_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (isBeingHuntedByBountyHunter(self, target))
		{
			pvpSetPersonalEnemyFlag(self, target);
			pvpSetPersonalEnemyFlag(target, self);
		}
		
		if (!combatStandardAction("sm_shoot_first_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_shoot_first_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (isBeingHuntedByBountyHunter(self, target))
		{
			pvpSetPersonalEnemyFlag(self, target);
			pvpSetPersonalEnemyFlag(target, self);
		}
		
		if (!combatStandardAction("sm_shoot_first_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_inspect_cargo(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id player = self;
		smuggler.inspectCorpseForContraband(player, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_spot_a_sucker_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_spot_a_sucker_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			int glanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_glance_chance");
			int criticalMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_critical_decrease");
			int damageDecrease = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_damage_decrease_chance");
			
			if (damageDecrease >= 40)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_7");
			}
			else if (damageDecrease >= 20)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_6");
			}
			else if (glanceMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_5");
			}
			else if (glanceMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_4");
			}
			else if (criticalMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_3");
			}
			else if (criticalMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_2");
			}
			else
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_1_1");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_spot_a_sucker_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_spot_a_sucker_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			int glanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_glance_chance");
			int criticalMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_critical_decrease");
			int damageDecrease = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_damage_decrease_chance");
			
			if (damageDecrease >= 40)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_7");
			}
			else if (damageDecrease >= 20)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_6");
			}
			else if (glanceMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_5");
			}
			else if (glanceMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_4");
			}
			else if (criticalMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_3");
			}
			else if (criticalMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_2");
			}
			else
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_2_1");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_spot_a_sucker_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_spot_a_sucker_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			int glanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_glance_chance");
			int criticalMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_critical_decrease");
			int damageDecrease = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_damage_decrease_chance");
			
			if (damageDecrease >= 40)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_7");
			}
			else if (damageDecrease >= 20)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_6");
			}
			else if (glanceMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_5");
			}
			else if (glanceMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_4");
			}
			else if (criticalMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_3");
			}
			else if (criticalMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_2");
			}
			else
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_3_1");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_spot_a_sucker_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_spot_a_sucker_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			int glanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_glance_chance");
			int criticalMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_critical_decrease");
			int damageDecrease = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_spot_a_sucker_damage_decrease_chance");
			
			if (damageDecrease >= 40)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_7");
			}
			else if (damageDecrease >= 20)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_6");
			}
			else if (glanceMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_5");
			}
			else if (glanceMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_4");
			}
			else if (criticalMod >= 50)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_3");
			}
			else if (criticalMod >= 25)
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_2");
			}
			else
			{
				buff.applyBuff(target, self, "sm_spot_a_sucker_4_1");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_off_the_cuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_off_the_cuff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int doubleDeal = (int)getSkillStatisticModifier(self, "expertise_double_deal_chance");
		
		if (rand(0,99) < doubleDeal)
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0);
			showFlyTextPrivate(self, self, new string_id("combat_effects", "double_deal"), 1.5f, colors.ORANGERED);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_end_of_the_line(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_end_of_the_line", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_card_ranged_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_double_hit"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!combatStandardAction("expertise_sm_card_ranged_proc", self, target, params, "", ""))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_card_melee_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.hasBuff(self, "sm_double_hit"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!combatStandardAction("expertise_sm_card_melee_proc", self, target, params, "", ""))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean callJunkDealer(obj_id self) throws InterruptedException
	{
		location loc = getLocation(self);
		
		if (!isIdValid(loc.cell))
		{
			int x = rand(-3, 3);
			int z = rand(-3, 3);
			
			loc.x += x;
			loc.z += z;
		}
		
		obj_id buddy = create.object("junk_dealer_smuggler", loc, getLevel(self));
		
		if (!isIdValid(buddy))
		{
			return false;
		}
		
		if (!isIdValid(loc.cell))
		{
			loc.x += 30;
			setHomeLocation(buddy, loc);
		}
		
		setMaster(buddy, self);
		utils.setScriptVar(self, "junk_dealer_smuggler", buddy);
		
		return true;
	}
	
	
	public int sm_off_the_books(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "junk_dealer_smuggler"))
		{
			obj_id dealer = utils.getObjIdScriptVar(self, "junk_dealer_smuggler");
			
			if (isIdValid(dealer) && exists(dealer))
			{
				sendSystemMessage(self, new string_id("spam", "dealer_already_exists"));
				return SCRIPT_OVERRIDE;
			}
			else
			{
				utils.removeScriptVar(self, "junk_dealer_smuggler");
			}
		}
		
		if (!combatStandardAction("sm_off_the_books", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (!callJunkDealer(self))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_inside_information(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_inside_information", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id infoTarget;
		
		if (isIdValid(target) && isPlayer(target))
		{
			infoTarget = target;
			
			if (factions.getFactionStatus(self, target) == factions.STATUS_ENEMY)
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			infoTarget = self;
		}
		
		if (bounty_hunter.isSpammingBountyCheck(self, false))
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "sm_calling_contact_too_often"));
			
			return SCRIPT_OVERRIDE;
		}
		
		if (infoTarget != self && bounty_hunter.isSpammingBountyCheck(infoTarget, false))
		{
			sendSystemMessage(self, new string_id("bounty_hunter", "sm_calling_contact_too_often_target"));
			
			return SCRIPT_OVERRIDE;
		}
		
		if (isDead(infoTarget) || isIncapacitated(infoTarget))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int amount = getIntObjVar(infoTarget, "bounty.amount");
		
		if (amount <= 0)
		{
			prose_package pp = new prose_package ();
			pp.stringId = new string_id ("bounty_hunter", "sm_no_bounty_target_player");
			prose.setTT(pp, getName(self));
			
			if (infoTarget != self)
			{
				sendSystemMessageProse(infoTarget, pp);
				sendSystemMessage(self, new string_id("bounty_hunter", "sm_no_bounty_target"));
				
				utils.setScriptVar(infoTarget, "bountyCheckFloodControl", getGameTime() + 600);
			}
			else
			{
				sendSystemMessage(self, new string_id("bounty_hunter", "sm_no_bounty_self"));
			}
			
			utils.setScriptVar(self, "bountyCheckFloodControl", getGameTime() + 60);
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] bounties = getJediBounties(infoTarget);
		
		if (bounties != null && bounties.length > 0)
		{
			prose_package pp = new prose_package ();
			pp.stringId = new string_id ("bounty_hunter", "sm_bounty_amount_target_with_bounties");
			prose.setDI(pp, amount);
			
			sendSystemMessageProse(infoTarget, pp);
			
			for (int i = 0; i < bounties.length; i++)
			{
				testAbortScript();
				obj_id bountyHunter = bounties[i];
				
				if (isIdValid(bountyHunter))
				{
					dictionary messageParams = new dictionary();
					messageParams.put("smuggler", infoTarget);
					
					messageTo(bountyHunter, "handleSmugglerGetLocationInfo", messageParams, 1.0f, false);
				}
			}
			
			utils.setScriptVar(self, "bountyCheckFloodControl", getGameTime() + 60);
		}
		else
		{
			prose_package pp = new prose_package ();
			pp.stringId = new string_id ("bounty_hunter", "sm_bounty_amount_target_no_bounties");
			prose.setDI(pp, amount);
			
			sendSystemMessageProse(infoTarget, pp);
			
			utils.setScriptVar(self, "bountyCheckFloodControl", getGameTime() + 60);
		}
		
		if (infoTarget != self)
		{
			prose_package pm = new prose_package();
			pm.stringId = new string_id ("bounty_hunter", "sm_bounty_notified_target");
			prose.setTT(pm, getName(infoTarget));
			
			sendSystemMessageProse(self, pm);
			
			utils.setScriptVar(infoTarget, "bountyCheckFloodControl", getGameTime() + 600);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_false_hope(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_false_hope", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sm_false_hope");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_sm_false_hope");
		
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_break_the_deal(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_break_the_deal", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_narrow_escape_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_narrow_escape_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_narrow_escape_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_narrow_escape_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_narrow_escape_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_narrow_escape_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_narrow_escape_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_narrow_escape_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_buff_invis_ally_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if ((!stealth.canPerformCamouflageAlly(self, target) || !combatStandardAction("sm_buff_invis_ally_1", self, target, params, "", "") || !isInAttackRange(self, target, "sm_buff_invis_ally_1", false)))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int minPower = 10;
		int maxPower = 30;
		float difference = maxPower - minPower;
		float ratio = getLevel(self)/90;
		
		int duration = (int) Math.ceil(minPower + (difference * ratio));
		
		buff.applyBuff(target, self, "invis_sm_buff_invis_1");
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_how_are_you(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sm_how_are_you", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doMedicEvasion(self);
		
		int randomPhrase = rand(0,75);
		
		if (randomPhrase < 5)
		{
			chat.chat(self, chat.CHAT_SAY, new string_id("set_bonus", "sm_blather_"+randomPhrase));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_ranged_shot_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_sm_ranged_shot_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_melee_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_sm_melee_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_melee_stun_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_sm_melee_stun_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_sm_feeling_lucky_chance_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_sm_feeling_lucky_chance_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_buff_invis_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canPerformForceCloak(self) || !combatStandardAction("fs_buff_invis_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int steal(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!stealth.canSteal(self, target) || !combatStandardAction("steal", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		stealth.steal(self, target);
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_buff_invis_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		dictionary blankMsg = new dictionary();
		if (hasSkill(self, "expertise_sp_without_a_trace_1"))
		{
			if (!stealth.canPerformWithoutTrace(self) || !combatStandardAction("sp_without_a_trace", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
			utils.setScriptVar(self, "sp_without_a_trace", 1);
			messageTo(self, "withoutTraceTimerExpired", blankMsg, 30, false);
			
			if (successfulFastAttack(self, "sp_perfect", "perfect_opportunity_fly"))
			{
				setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
				buff.applyBuff(self, self, "sp_set_perfect_opportunity");
				return SCRIPT_CONTINUE;
			}
			
			if (successfulFastAttack(self, "sp_smoke", "flawless_getaway_fly"))
			{
				setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
				return SCRIPT_CONTINUE;
			}
			float baseCooldownTime = getBaseCooldownTime("sp_buff_invis_1");
			if (baseCooldownTime < 0)
			{
				baseCooldownTime = 0;
			}
			
			float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_smoke");
			setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
			
			return SCRIPT_CONTINUE;
		}
		
		if (!stealth.canPerformSmokeGrenade(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_buff_invis_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(self, "sp_smoke_bomb", 1);
		messageTo(self, "smokeBombTimerExpired", blankMsg, 10, false);
		
		if (successfulFastAttack(self, "sp_perfect", "perfect_opportunity_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			buff.applyBuff(self, self, "sp_set_perfect_opportunity");
			return SCRIPT_CONTINUE;
		}
		
		if (successfulFastAttack(self, "sp_smoke", "flawless_getaway_fly"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_buff_invis_1");
		if (baseCooldownTime < 0)
		{
			baseCooldownTime = 0;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_smoke");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_burst_of_shadows(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_burst_of_shadows", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_burst_of_shadows");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_burst_shadows");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_reveal_shadows(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canDetectCamouflage(self) || !combatStandardAction("sp_reveal_shadows", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float detectSkill = getEnhancedSkillStatisticModifier(self, "detect_hidden");
		float distance = stealth.BASE_DETECT_CAMOUFLAGE_DISTANCE + (detectSkill / 20);
		
		float detectChance = 75.0f;
		
		int distanceMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_sp_reveal_shadows_distance_increase");
		int detectMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_sp_reveal_shadows_detect_chance_increase");
		
		distance = distance + distanceMod;
		detectChance = detectChance + detectMod;
		
		stealth.detectCamouflage(self, true, true, distance, detectChance);
		
		float baseCooldownTime = getBaseCooldownTime("sp_reveal_shadows");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_reveal_shadows");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_avoid_damage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_avoid_damage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_preparation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_preparation", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_preparation");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_preparation");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_buff_stealth_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		String invis = stealth.getInvisBuff(self);
		if (invis != null && invis.equals("invis_sp_buff_stealth_1"))
		{
			buff.removeBuff(self, invis);
			return SCRIPT_OVERRIDE;
		}
		
		if (invis != null && (invis.equals("invis_sp_buff_invis_1")))
		{
			
			buff.removeBuff(self, "invis_sp_buff_invis_1");
			buff.applyBuff(self, self, "invis_sp_buff_stealth_1");
			return SCRIPT_OVERRIDE;
		}
		
		if (!stealth.canPerformStealth(self) || !buff.canApplyBuff(self, "invis_sp_buff_stealth_1") || !combatStandardAction("sp_buff_stealth_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_decoy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canPerformSmokeGrenade(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id decoy = stealth.createDecoy(self);
		
		if (!isIdValid(decoy))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id[] haters = getWhoIsTargetingMe(self);
		int diversionLevel = getEnhancedSkillStatisticModifierUncapped(self, "expertise_improved_decoy");
		combat.sendCombatSpamMessage(self, new string_id("spam", "decoy_created"));
		
		if (haters == null || haters.length == 0)
		{
			
			if (diversionLevel == 2)
			{
				buff.applyBuff(self, self, "invis_sp_diversion_stealth");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		for (int i=0; i<haters.length; i++)
		{
			testAbortScript();
			if (!isIdValid(haters[i]) || !exists(haters[i]))
			{
				continue;
			}
			
			obj_id[] hateList = getHateList(haters[i]);
			
			if (hateList == null || hateList.length == 0)
			{
				continue;
			}
			
			for (int k=0; k<hateList.length; k++)
			{
				testAbortScript();
				if (!isIdValid(hateList[k]) || !exists(hateList[k]))
				{
					continue;
				}
				
				if (hateList[k] == self)
				{
					setHate(haters[i], decoy, getHate(haters[i], self) * 2);
					setTarget(haters[i], decoy);
				}
			}
		}
		
		if (diversionLevel == 2)
		{
			buff.applyBuff(self, self, "invis_sp_diversion_stealth");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_assassins_mark(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_assassins_mark", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_assassins_mark_reac_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		obj_id newTarget = self;
		obj_id newAttacker = target;
		if (!combatStandardAction("sp_assassins_mark_reac_proc", newAttacker, newTarget, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cc_dot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_cc_dot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_improved_cc_dot_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_improved_cc_dot_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_improved_cc_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_improved_cc_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_improved_cc_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_improved_cc_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_improved_cc_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_improved_cc_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_1");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_2");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dm_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dm_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "sp_dm"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_CONTINUE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("sp_dm_8");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_sp_dm");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_dot_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_dot_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_melee_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_melee_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_hd_range_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_hd_range_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_melee_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_melee_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_stealth_ranged_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_stealth_ranged_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_run_its_course(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("sp_run_its_course", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_fld_debuff_ca(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_fld_debuff_ca", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_fldmot_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int crippleChanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_sp_cripple_trap_proc_increase");
		
		int roll = rand(1,100);
		if (roll < crippleChanceMod)
		{
			if (!combatStandardAction("sp_fldmot_1_snare", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			if (!combatStandardAction("sp_fldmot_1", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_fldmot_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int crippleChanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_sp_cripple_trap_proc_increase");
		
		int roll = rand(1,100);
		if (roll < crippleChanceMod)
		{
			if (!combatStandardAction("sp_fldmot_2_snare", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			if (!combatStandardAction("sp_fldmot_2", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_fldmot_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int crippleChanceMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_sp_cripple_trap_proc_increase");
		
		int roll = rand(1,100);
		if (roll < crippleChanceMod)
		{
			if (!combatStandardAction("sp_fldmot_3_snare", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			if (!combatStandardAction("sp_fldmot_3", self, target, params, "", ""))
			{
				return SCRIPT_OVERRIDE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_sh_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_sh_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_sh_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_sh_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_sh_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_sh_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_sh_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_sh_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cloaked_recovery_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_cloaked_recovery_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cloaked_recovery_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_cloaked_recovery_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cloaked_recovery_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_cloaked_recovery_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cloaked_recovery_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_cloaked_recovery_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_cloaked_recovery_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.isDamaged(self))
		{
			sendSystemMessage(self, new string_id("healing", "no_damage_to_heal_self"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_cloaked_recovery_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleClearBuffEffect(obj_id self, dictionary params) throws InterruptedException
	{
		int buffCrc = params.getInt("buffCrc");
		if (buffCrc != 0)
		{
			stopClientEffectObjByLabel(self, buff.getBuffNameFromCrc(buffCrc));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int failDefaultAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_OVERRIDE;
	}
	
	
	public int failSpecialAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int failProc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_riposte(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInStance(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_riposte", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyTextPrivate(self, self, new string_id("combat_effects", "riposte"), 1.5f, colors.ORANGERED);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_fs_force_clarity_1_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_fs_force_clarity_1_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int meleeHit(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("meleeHit", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int rangedShot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("rangedShot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int saberHit(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("saberHit", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_cyber_shock(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_cyber_shock", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_fire_attack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("procCommand", "proc found");
		if (!combatStandardAction("proc_fire_attack", self, target, params, "", ""))
		{
			LOG("procCommand", "proc failed in combat standard");
			return SCRIPT_OVERRIDE;
		}
		
		LOG("procCommand","proc worked in script");
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_generic_test(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_generic_test", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int reac_generic_test(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("reac_generic_test", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_generic_test_proc(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_generic_test_proc", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_generic_test_reac(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_generic_test_reac", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_fire_attack_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_fire_attack_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_a_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_a_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_a_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_a_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_a_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_a_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_b_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_b_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_coyn(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_coyn", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_bracelet_melee(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_bracelet_melee", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_bracelet_range(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_bracelet_range", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_tow_proc_bracelet_combo(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_tow_proc_bracelet_combo", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_fire(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_fire", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_cold(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_cold", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_melee_poison_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_melee_poison_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_range_heat_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_range_heat_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_melee_poison_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_melee_poison_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_content_charged_proc_range_heat_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_content_charged_proc_range_heat_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_old_light_jedi_gift(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_old_light_jedi_gift", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_old_dark_jedi_gift(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_old_dark_jedi_gift", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_innate_assault_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_innate_assault_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_return_fire_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_return_fire_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bh_taunt_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id topHateTarget = getHateTarget(target);
		
		if (topHateTarget != self)
		{
			combat.doBhTaunt(self, target);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_taunt_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_taunt_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_taunt_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_taunt_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_taunt_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_taunt_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_fumble_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_fumble_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_stun_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_stun_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_stun_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_stun_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_stun_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_stun_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_stun_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_stun_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_stun_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_stun_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_prescience(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_prescience", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		buff.applyBuff(self, target, "bh_prescience");
		obj_id[] viewers = new obj_id[2];
		viewers[0] = self;
		viewers[1] = target;
		playClientEffectObj(viewers, "appearance/pt_arrow_disc_temp.prt", target, "");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_shields_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_shields_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_3");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_4");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_5");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_6");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_7(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_7", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_7");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dm_crit_8(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dm_crit_8", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_dm_crit_8");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_dm_crit") / 10;
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_intimidate_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_intimidate_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dread_strike_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dread_strike_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dread_strike_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dread_strike_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dread_strike_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dread_strike_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dread_strike_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dread_strike_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dread_strike_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dread_strike_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		removeObjVar(self, "ai.combat.oneShotAction");
		removeObjVar(self, "oneShotActionComplete");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_relentless_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_relentless_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_sniper_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_sniper_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_cover_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_cover_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_sprint_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_sprint_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_heal_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_heal_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_heal_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_heal_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_heal_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_heal_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_heal_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_heal_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_armor_duelist_heal_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_armor_duelist_heal_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_return_fire_command_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_return_fire_command_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("bh_return_fire_command_1");
		
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getSkillStatisticModifier(self, "expertise_cooldown_line_bh_return_fire");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bh_dire_root_area_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bh_dire_root_area_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_damage_shield_a(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_damage_shield_a", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_damage_shield_b(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_damage_shield_b", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_damage_shield_c(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_damage_shield_c", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_damage_shield_d(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_damage_shield_d", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_damage_shield_e(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_damage_shield_e", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_gcw_base_damage_a(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_gcw_base_damage_a", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_gcw_base_damage_b(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_gcw_base_damage_b", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_gcw_base_damage_c(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_gcw_base_damage_c", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_gcw_base_damage_d(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_gcw_base_damage_d", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_gcw_base_damage_e(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_gcw_base_damage_e", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_heal_a(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_heal_a", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_heal_b(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_heal_b", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_heal_c(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_heal_c", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_heal_d(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_heal_d", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_heal_e(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_heal_e", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_action_a(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_action_a", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_action_b(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_action_b", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_action_c(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_action_c", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_action_d(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_action_d", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_base_reactive_action_e(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("gcw_base_reactive_action_e", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int throwGrenade(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		Long lngId;
		
		try
		{
			lngId = new Long(params);
		}
		catch (NumberFormatException err)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objGrenade = obj_id.getObjId(lngId.longValue());
		
		if (hasObjVar(objGrenade, "intUsed"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int intCount = getCount(objGrenade);
		intCount = intCount - 1;
		
		String strTemplate = getSharedObjectTemplateName(objGrenade);
		dictionary dctGrenadeCommand = dataTableGetRow("datatables/timer/template_command_mapping.iff", strTemplate);
		String strCommand = dctGrenadeCommand.getString("COMMAND");
		
		if (!combatStandardAction(strCommand, self, target, objGrenade, "", null, false ))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			if (intCount <= 0)
			{
				setObjVar(objGrenade, "intUsed", 1);
				destroyObject(objGrenade);
			}
			else
			{
				setCount(objGrenade, intCount);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int failThrowGrenade(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		Long lngId;
		
		try
		{
			lngId = new Long(params);
		}
		catch (NumberFormatException err)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id grenade = obj_id.getObjId(lngId.longValue());
		
		removeObjVar(grenade, "intUsed");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cheapShot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("cheapShot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int blastAway(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("blastAway", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hipShot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("hipShot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int assault(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("assault", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int crippleShot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("crippleShot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceThrow(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("forceThrow", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ambush(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ambush", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int demolition(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("demolition", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stunGrenade(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("stunGrenade", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int barrage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("barrage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceStrike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("forceStrike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int actOfWar(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("actOfWar", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int terminateTarget(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("terminateTarget", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int targetAnatomy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("targetAnatomy", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int neurotoxin(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("neurotoxin", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int onehandThrow(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("onehandThrow", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int twohandThrow(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("twohandThrow", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int polearmThrow(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("polearmThrow", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stealth(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canPerformStealth(self) || !combatStandardAction("stealth", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int smokeGrenade(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canPerformSmokeGrenade(self) || !combatStandardAction("smokeGrenade", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int entrench(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("entrench", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int centerOfBeing(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("centerOfBeing", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceFocus(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("forceFocus", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int paintTarget(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("paintTarget", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int blueGlowie(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		string_id SHAPECHANGE = new string_id("spam","not_while_shapechanged");
		int shapechange = buff.getBuffOnTargetFromGroup(self, "shapechange");
		if (shapechange != 0)
		{
			sendSystemMessage(self, SHAPECHANGE);
			return SCRIPT_OVERRIDE;
		}
		
		if (combat.isInCombat(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (getState(self, STATE_GLOWING_JEDI) == 0)
		{
			setState(self, STATE_GLOWING_JEDI, true);
		}
		else
		{
			setState(self, STATE_GLOWING_JEDI, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int proc_pvp_despair(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("proc_pvp_despair", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_aura_buff_self(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_aura_buff_self", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_aura_buff_rebel_self(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_aura_buff_rebel_self", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_retaliation_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_retaliation_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_retaliation_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_retaliation_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_adrenaline_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_adrenaline_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_adrenaline_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_adrenaline_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_unstoppable_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_unstoppable_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_unstoppable_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_unstoppable_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_retaliation_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_retaliation_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_retaliation_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_retaliation_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_adrenaline_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_adrenaline_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_adrenaline_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_adrenaline_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_unstoppable_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_unstoppable_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_unstoppable_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_unstoppable_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_last_man_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_last_man_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int command_pvp_last_man_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("command_pvp_last_man_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_last_man_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_last_man_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_last_man_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("pvp_last_man_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_airstrike_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("pvp_airstrike_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package p = prose.getPackage(new string_id("spam", "pvp_airstrike_imperial"));
		commPlayers(self, "object/mobile/space_comm_imperial_tiefighter_01.iff", "sound/sys_comm_other.snd", 5f, self, p);
		playClientEffectLoc(self, "clienteffect/pvp_imperial_airstrike.cef", getLocation(target), 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int pvp_airstrike_rebel_ability(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("pvp_airstrike_rebel_ability", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		prose_package p = prose.getPackage(new string_id("spam", "pvp_airstrike_rebel"));
		commPlayers(self, "object/mobile/space_comm_rebel_xwing_02.iff", "sound/sys_comm_other.snd", 5f, self, p);
		playClientEffectLoc(self, "clienteffect/pvp_rebel_airstrike.cef", getLocation(target), 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gallop(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id objMount = getMountId(self);
		if (!isIdValid(objMount))
		{
			string_id strSpam = new string_id("combat_effects", "not_mounted");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		else if (vehicle.isDriveableVehicle(objMount))
		{
			string_id strSpam = new string_id("combat_effects", "not_mounted");
			sendSystemMessage(self, strSpam);
			return SCRIPT_OVERRIDE;
		}
		
		dictionary dctMountInfo = pet_lib.getMountMovementInfo(objMount);
		if (dctMountInfo == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objControlDevice = callable.getCallableCD(objMount);
		
		if (utils.hasScriptVar(objControlDevice, "mount.intGalloping"))
		{
			string_id strSpam = new string_id("combat_effects", "already_galloping");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(objControlDevice, "mount.intTired"))
		{
			string_id strSpam = new string_id("combat_effects", "mount_tired");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		pet_lib.setMountBurstRunMovementRate(self, objMount);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gallopStop(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id objMount = getMountId(self);
		if (!isIdValid(objMount))
		{
			string_id strSpam = new string_id("combat_effects", "cant_stop_gallop_not_mounted");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
			
		}
		
		if (!pet_lib.isGalloping(objMount))
		{
			string_id strSpam = new string_id("combat_effects", "cant_stop_galloping_not_galloping");
			sendSystemMessage(self, strSpam);
			return SCRIPT_CONTINUE;
		}
		
		obj_id objControlDevice = callable.getCallableCD(objMount);
		messageTo(objControlDevice, "removeGallop", null, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_holographic_image(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (combat.isInCombat(self))
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_holo_combat"));
			return SCRIPT_OVERRIDE;
		}
		
		if (performance.hasMaxHolo(self))
		{
			sendSystemMessage(self, new string_id("spam", "holo_maximum_reached"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!performance.createHolographicBackup(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_holographic_recall(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "holoMessageTime"))
		{
			sendSystemMessage(self, new string_id("spam", "no_holo_recall"));
			return SCRIPT_OVERRIDE;
		}
		
		performance.holographicCleanup(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_void_dance(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int voidEffectMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "eexpertise_buff_en_void_dance_buff");
		
		if ((voidEffectMod == 5) && (!combatStandardAction("en_void_dance_1", self, target, params, "", "")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		else if ((voidEffectMod == 10) && (!combatStandardAction("en_void_dance_2", self, target, params, "", "")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		else if ((voidEffectMod == 15) && (!combatStandardAction("en_void_dance_3", self, target, params, "", "")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		else if (!combatStandardAction("en_void_dance", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_strike_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_strike_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_thrill(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		int thrillEffectMod = (int)getEnhancedSkillStatisticModifierUncapped(self, "expertise_en_debuff_thrill_increase");
		
		if ((thrillEffectMod == 10) && (!combatStandardAction("en_thrill_1", self, target, params, "", "")))
		{
			return SCRIPT_OVERRIDE;
		}
		else if ((thrillEffectMod == 20) && (!combatStandardAction("en_thrill_2", self, target, params, "", "")))
		{
			return SCRIPT_OVERRIDE;
		}
		else if (!combatStandardAction("en_thrill", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_project_will_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_project_will_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_sweeping_pirouette_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_sweeping_pirouette_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_unhealthy_fixation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_unhealthy_fixation", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_spiral_kick_0(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_spiral_kick_0", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_spiral_kick_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_spiral_kick_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_spiral_kick_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_spiral_kick_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_spiral_kick_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_spiral_kick_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int en_spiral_kick_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("en_spiral_kick_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int pingObject(obj_id self, dictionary params) throws InterruptedException
	{
		
		location locTest = getLocation(self);
		obj_id objSender = params.getObjId("objSender");
		dictionary dctParams = new dictionary();
		dctParams.put("locTest", locTest);
		obj_id objTopMostContainer = getTopMostContainer(self);
		if (isIdValid(objTopMostContainer))
		{
			dctParams.put("objTopMost", objTopMostContainer);
			dctParams.put("locTopMost", getLocation(objTopMostContainer));
		}
		
		dctParams.put("objSender", self);
		messageTo(objSender, "pingResponse", dctParams, 0, false);
		LOG("npe", self+" at "+locTest+" responded to ping");
		return SCRIPT_CONTINUE;
	}
	
	
	public int bountyTrackTarget(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = utils.stringToObjId(params);
		
		if (!isIdValid(droid))
		{
			return SCRIPT_OVERRIDE;
		}
		else
		{
			bounty_hunter.probeDroidTrackTarget(self, droid);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyCritImmunityBuff(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "criticalImmunity"))
		{
			buff.applyBuff(self, self, "criticalImmunity");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyStunImmunity(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "stun_immune"))
		{
			buff.applyBuff(self, self, "stun_immune");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int applyVortexSnare(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "of_vortex_snare"))
		{
			buff.applyBuff(self, self, "of_vortex_snare");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public float getBaseCooldownTime(String commandName) throws InterruptedException
	{
		float cooldownTime = dataTableGetFloat("datatables/command/command_table.iff", commandName, "cooldownTime");
		
		return cooldownTime;
	}
	
	
	public int fs_dot_immunity_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "fs_dot_immunity_recourse"))
		{
			buff.applyBuff(self, self, "fs_dot_immunity_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int throwFan(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("throwFan", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creature_milking_buff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creature_milking_buff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int lair_egg_buff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("lair_egg_buff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_buildabuff_heal_1_reac(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_buildabuff_heal_1_reac", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_buildabuff_heal_2_reac(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_buildabuff_heal_2_reac", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expertise_buildabuff_heal_3_reac(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("expertise_buildabuff_heal_3_reac", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int forceCrystalForce(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("procCommand", "proc found");
		if (!combatStandardAction("forceCrystalForce", self, target, params, "", ""))
		{
			LOG("procCommand", "proc failed in combat standard");
			return SCRIPT_OVERRIDE;
		}
		
		LOG("procCommand","proc worked in script");
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_soothing_comfort_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		
		if (!exists(beast) || isIdNull(beast))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			sendSystemMessage(self, new string_id("spam", "no_beast_out") );
			return SCRIPT_OVERRIDE;
		}
		
		target = beast;
		
		if (!isDead(beast) == false)
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			sendSystemMessage(self, new string_id("spam", "pet_beyond_healing") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_soothing_comfort_1", self, target, params, "", ""))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_mend_pet_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		
		if (!exists(beast) || isIdNull(beast))
		{
			sendSystemMessage(self, new string_id("spam", "no_beast_out") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		target = beast;
		
		if (isDead(beast))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			sendSystemMessage(self, new string_id("spam", "pet_beyond_healing") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.canDoBeastHeal(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_mend_pet_1", self, target, params, "", ""))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		int toHeal = dataTableGetInt(COMBAT_TABLE, "bm_mend_pet_1", "addedDamage");
		healing.doBeastHeal(self, target, toHeal);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_mend_pet_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		
		if (!exists(beast) || isIdNull(beast))
		{
			sendSystemMessage(self, new string_id("spam", "no_beast_out") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		target = beast;
		
		if (isDead(beast))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			sendSystemMessage(self, new string_id("spam", "pet_beyond_healing") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.canDoBeastHeal(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_mend_pet_2", self, target, params, "", ""))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		int toHeal = dataTableGetInt(COMBAT_TABLE, "bm_mend_pet_2", "addedDamage");
		healing.doBeastHeal(self, target, toHeal);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_mend_pet_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		
		if (!exists(beast) || isIdNull(beast))
		{
			sendSystemMessage(self, new string_id("spam", "no_beast_out") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		target = beast;
		
		if (isDead(beast))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			sendSystemMessage(self, new string_id("spam", "pet_beyond_healing") );
			return SCRIPT_OVERRIDE;
		}
		
		if (!healing.canDoBeastHeal(self, target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_mend_pet_3", self, target, params, "", ""))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		int toHeal = dataTableGetInt(COMBAT_TABLE, "bm_mend_pet_3", "addedDamage");
		healing.doBeastHeal(self, target, toHeal);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_revive_pet_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		
		if (!exists(beast) || isIdNull(beast))
		{
			sendSystemMessage(self, new string_id("spam", "no_beast_out") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		target = beast;
		
		if (!isDead(beast))
		{
			sendSystemMessage(self, new string_id("spam", "beast_not_dead") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		if (getDistance(getLocation(self), getLocation(beast)) > 7.0f)
		{
			sendSystemMessage(self, new string_id("spam", "stand_next_to_corpse") );
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_revive_pet_1", self, target, params, "", ""))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
			return SCRIPT_OVERRIDE;
		}
		
		messageTo(self, "channelRevivePet", null, 0, false);
		
		float baseCooldownTime = getBaseCooldownTime("bm_revive_pet_1");
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_bm_pet_revive_time");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - cooldownTimeMod);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_paralytic_poison_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "bm_paralytic_poison_recourse"))
		{
			buff.applyBuff(self, self, "bm_paralytic_poison_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePostureRestore(obj_id self, dictionary params) throws InterruptedException
	{
		stopClientEffectObjByLabel(self, "state_knockdown");
		
		if (getPosture(self) == POSTURE_KNOCKED_DOWN)
		{
			setPostureClientImmediate(self, utils.getIntScriptVar(self, "combat.intKnockdownTime.posture"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int performHateTick(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id attacker = params.getObjId("attacker");
		int hate = params.getInt("hate");
		int tick = params.getInt("tick");
		int duration = params.getInt("duration");
		
		addHate(attacker, self, 0.0f);
		addHate(self, attacker, hate);
		
		if (duration < getGameTime())
		{
			messageTo(self, "performHateTick", params, tick, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int combatTarget(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		setTarget(self, target);
		
		return SCRIPT_OVERRIDE;
	}
	
	
	public int updateCrowdControlEffect(obj_id self, dictionary params) throws InterruptedException
	{
		boolean effectActive = false;
		
		int[] buffList = buff.getAllBuffs(self);
		
		if (buffList == null || buffList.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		prose_package pp = new prose_package();
		
		for (int i = 0; i < buffList.length; i++)
		{
			testAbortScript();
			if (movement.isStunEffect(buff.getBuffNameFromCrc(buffList[i])))
			{
				effectActive = true;
				pp = prose.setStringId(pp, new string_id("combat_effects", "go_stunned"));
				showFlyTextPrivateProseWithFlags(self, self, pp, 1.5f, colors.LAWNGREEN, FLY_TEXT_FLAG_IS_SNARE );
				
			}
			if (movement.isRoot(buffList[i]))
			{
				effectActive = true;
				pp = prose.setStringId(pp, new string_id("combat_effects", "go_rooted"));
				showFlyTextPrivateProseWithFlags(self, self, pp, 1.5f, colors.LAWNGREEN, FLY_TEXT_FLAG_IS_SNARE );
			}
			
			if (movement.isSnare(buffList[i]))
			{
				effectActive = true;
				pp = prose.setStringId(pp, new string_id("combat_effects", "go_snare"));
				showFlyTextPrivateProseWithFlags(self, self, pp, 1.5f, colors.LAWNGREEN, FLY_TEXT_FLAG_IS_SNARE );
				
			}
		}
		
		if (effectActive)
		{
			messageTo (self, "updateCrowdControlEffect", null, 4f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureMeleeAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureMeleeAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureRangedAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureRangedAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureBlindAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureBlindAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureDizzyAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureDizzyAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureStunAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureStunAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureIntimidationAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureIntimidationAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureKnockdownAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureKnockdownAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creaturePostureDownAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creaturePostureDownAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creaturePostureUpAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creaturePostureUpAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaCombo(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaCombo", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaBleeding(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaBleeding", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaDisease(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaDisease", self, target, params, "creatureAreaDiseaseSuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaPoison(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaPoison", self, target, params, "creatureAreaPoisonSuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaKnockdown(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureAreaKnockdown", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creaturePoisonAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creaturePoisonAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureFireAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureFireAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureSnareAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("creatureSnareAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaDiseaseSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] defenders = params.getObjIdArray("defenders");
		
		for (int i = 0; i < defenders.length; i++)
		{
			testAbortScript();
			dot.applyDotEffect(defenders[i], self, dot.DOT_DISEASE, "creatureDiseaseAttack", HEALTH, 100, 300, 300);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int creatureAreaPoisonSuccess(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id[] defenders = params.getObjIdArray("defenders");
		
		for (int i = 0; i < defenders.length; i++)
		{
			testAbortScript();
			if (dot.canApplyDotType(defenders[i], dot.DOT_POISON))
			{
				dot.applyDotEffect(defenders[i], self, dot.DOT_POISON, "creaturePoisonAttack", HEALTH, 100, 150, 300);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int knockdownRecovery(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("knockdownRecovery", self, target, params, "knockdownRecoverySuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stunRecovery(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("stunRecovery", self, target, params, "stunRecoverySuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int blindRecovery(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("blindRecovery", self, target, params, "blindRecoverySuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dizzyRecovery(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("dizzyRecovery", self, target, params, "dizzyRecoverySuccess", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int iceCreamRecourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "ice_cream_recourse"))
		{
			buff.applyBuff(self, self, "ice_cream_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dazeRecourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "dazeBlockDebuff"))
		{
			buff.applyBuff(self, self, "dazeBlockDebuff");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int mezRecourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "mezBlockDebuff"))
		{
			buff.applyBuff(self, self, "mezBlockDebuff");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int seetingAngerRecourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "seetingAnger_1"))
		{
			buff.applyBuff(self, self, "seethingAnger_1");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int seetingAngerRecourse_1(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "seetingAnger_2"))
		{
			buff.applyBuff(self, self, "seethingAnger_2");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int seetingAngerRecourse_2(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "seetingAnger_3"))
		{
			buff.applyBuff(self, self, "seethingAnger_3");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int seetingAngerRecourse_3(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "seetingAnger_4"))
		{
			buff.applyBuff(self, self, "seethingAnger_4");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petSnareAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petSnareAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petPinAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petPinAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petBleedAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petBleedAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petIntimidateAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petIntimidateAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petStunAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petStunAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petBlindAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petBlindAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petKnockdownAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petKnockdownAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int petDefenseBreakAttack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("petDefenseBreakAttack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int insidiousMalady(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("insidiousMalady", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int euphoria(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("euphoria", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int rampage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("rampage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int seethingAnger(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("seethingAnger", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cloakOfLava(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("cloakOfLava", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int sherKarRage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sherKarRage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int decrepitStrike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("decrepitStrike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int sk_shockWave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sk_shockWave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_charge_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_charge_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_charge_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_charge_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_charge_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_charge_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_charge_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_charge_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_charge_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_charge_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_trample(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_trample", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bm_bite_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_bite_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bite_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("bm_bite_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bite_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bite_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bite_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bite_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bite_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bite_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_puncture_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_puncture_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_puncture_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_puncture_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_puncture_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_puncture_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_hamstring_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_hamstring_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_hamstring_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_hamstring_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_hamstring_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_hamstring_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_hamstring_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_hamstring_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_hamstring_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_hamstring_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_claw_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_claw_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_claw_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_claw_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_claw_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_claw_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_claw_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_claw_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_claw_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_claw_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_slash_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_slash_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_slash_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_slash_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_slash_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_slash_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_slash_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_slash_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_slash_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_slash_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_flank_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_flank_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_flank_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_flank_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_flank_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_flank_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bolster_armor_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bolster_armor_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bolster_armor_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bolster_armor_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bolster_armor_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bolster_armor_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bolster_armor_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bolster_armor_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_bolster_armor_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_bolster_armor_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_dampen_pain_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_dampen_pain_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_dampen_pain_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_dampen_pain_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_dampen_pain_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_dampen_pain_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_dampen_pain_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_dampen_pain_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_dampen_pain_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_dampen_pain_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_shield_master(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_shield_master", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_deflective_hide(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_deflective_hide", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_defensive_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_defensive_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int bm_defensive_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_defensive_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int bm_defensive_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_defensive_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int bm_defensive_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_defensive_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	public int bm_defensive_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_defensive_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_poison_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_poison_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int bm_damage_poison_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_poison_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int bm_damage_poison_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_poison_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int bm_damage_poison_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_poison_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	public int bm_damage_poison_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_poison_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_disease_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_disease_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_disease_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_disease_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_disease_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_disease_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_disease_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_disease_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_damage_disease_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_damage_disease_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_paralytic_poison(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_paralytic_poison", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enfeeble_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enfeeble_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enfeeble_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enfeeble_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enfeeble_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enfeeble_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enfeeble_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enfeeble_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enfeeble_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enfeeble_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_preperation(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_preperation", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_stomp_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_stomp_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_stomp_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_stomp_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_stomp_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_stomp_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_stomp_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_stomp_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_stomp_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_stomp_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_shaken_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_shaken_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_shaken_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_shaken_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_shaken_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_shaken_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_wing_buffet_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_wing_buffet_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_wing_buffet_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_wing_buffet_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_wing_buffet_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_wing_buffet_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_wing_buffet_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_wing_buffet_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_wing_buffet_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_wing_buffet_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_spit_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_spit_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_spit_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_spit_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_spit_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_spit_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_spit_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_spit_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_spit_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_spit_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_kick_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_kick_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_kick_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_kick_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_kick_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_kick_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_kick_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_kick_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_kick_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_kick_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_enrage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_enrage", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_frenzy(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_frenzy", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_provoke_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		target = isIdValid(getTarget(beast)) ? getTarget(beast) : getIntendedTarget(self);
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_provoke_1", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.doBmProvokeCommand(beast, target, 1);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_provoke_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		target = isIdValid(getTarget(beast)) ? getTarget(beast) : getIntendedTarget(self);
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_provoke_2", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.doBmProvokeCommand(beast, target, 2);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_provoke_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		target = isIdValid(getTarget(beast)) ? getTarget(beast) : getIntendedTarget(self);
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_provoke_3", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.doBmProvokeCommand(beast, target, 3);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_provoke_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		target = isIdValid(getTarget(beast)) ? getTarget(beast) : getIntendedTarget(self);
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_provoke_4", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.doBmProvokeCommand(beast, target, 4);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_provoke_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		target = isIdValid(getTarget(beast)) ? getTarget(beast) : getIntendedTarget(self);
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("bm_provoke_5", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.doBmProvokeCommand(beast, target, 5);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_taunt(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		int stanceBuff = buff.getBuffOnTargetFromGroup(self, "fsDefStance");
		if (stanceBuff == 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		target = isIdValid(getIntendedTarget(self)) ? getIntendedTarget(self): getTarget(self) ;
		
		if (!isIdValid(target) || !exists(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!ai_lib.isTauntable(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isPlayer(target) || beast_lib.isBeast(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id topHateTarget = getHateTarget(target);
		
		if (topHateTarget == self)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!combatStandardAction("fs_taunt", self, target, params, "", "", false))
		{
			return SCRIPT_OVERRIDE;
		}
		
		combat.dsFsTaunt(self, target);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int togglePetAutoRepeatOn(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		beast_lib.setAbilityAutoRepeat(beast_lib.getBeastOnPlayer(self), params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int togglePetAutoRepeatOff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		beast_lib.unsetAbilityAutoRepeat(beast_lib.getBeastOnPlayer(self), params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int toggleBeastDefensive(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		if (!isIdValid(beast) || !exists(beast))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isDead(beast))
		{
			sendSystemMessage(self, beast_lib.SID_BEAST_CANT_WHEN_DEAD);
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.setBeastDefensive(beast, self, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int toggleBeastPassive(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id beast = beast_lib.getBeastOnPlayer(self);
		if (!isIdValid(beast) || !exists(beast))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isDead(beast))
		{
			sendSystemMessage(self, beast_lib.SID_BEAST_CANT_WHEN_DEAD);
			return SCRIPT_OVERRIDE;
		}
		
		beast_lib.setBeastDefensive(beast, self, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_forage_enzyme_buff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_forage_enzyme_buff", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int devastating_strike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("devastating_strike", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int human_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!combatStandardAction("human_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_human_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int wookiee_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("wookiee_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_wookiee_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int rodian_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("rodian_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_rodian_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bothan_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.canPerformSmokeGrenade(self) || !combatStandardAction("bothan_ability_1", self, target, params, "", "") || !buff.canApplyBuff(target, "invis_bothan_ability_1"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ithorian_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ithorian_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float level = getLevel(self);
		float blockValue = level * (1000.0f/90.0f);
		
		buff.applyBuff(self, self, "ithorian_ability_1", 30.0f,blockValue);
		showFlyText(self, new string_id("spam", "fly_ithorian_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int twilek_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("twilek_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_twilek_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sullustan_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sullustan_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_sullustan_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int moncal_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("moncal_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		showFlyText(self, new string_id("spam", "fly_moncal_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int trandoshan_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("trandoshan_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int level = getLevel(self);
		int healPerTick = level * 20;
		
		healing.startHealOverTime(self, self, "trandoshan_ability_1", 4.0f, 1.0f, healPerTick, true);
		showFlyText(self, new string_id("spam", "fly_trandoshan_ability_1"), 1.5f, colors.LEMONCHIFFON);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int zabrak_ability_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("zabrak_ability_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int level = getLevel(self);
		int amount = level * 40;
		int actionBefore = getAttrib(self, ACTION);
		
		int healthDelta = healing.healDamage(self, self, HEALTH, amount);
		healing.healDamage(self, ACTION, amount);
		
		int actionDelta = getAttrib(self, ACTION) - actionBefore;
		
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, new string_id("spam", "zabrak_equilibrium_spam"));
		pp = prose.setTT(pp, self);
		pp = prose.setDI(pp, healthDelta);
		healing.sendMedicalSpam(self, self, pp, true, true, true, COMBAT_RESULT_MEDICAL);
		
		prose_package ppp = new prose_package();
		pp = prose.setStringId(ppp, new string_id("spam", "zabrak_equilibrium_spam_action"));
		pp = prose.setTT(ppp, self);
		pp = prose.setDI(ppp, actionDelta);
		healing.sendMedicalSpam(self, self, ppp, true, true, true, COMBAT_RESULT_MEDICAL);
		
		showFlyText(self, new string_id("spam", "fly_zabrak_ability_1"), 1.5f, colors.LEMONCHIFFON);
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_reward_comlink(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!faction_perk.executeComlinkReinforcements(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int trample_random(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("trample_random", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		setLocation(self, getLocation(utils.getObjIdScriptVar(self, combat.CHARGE_TARGET)));
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int bodyguard(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bodyguard", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int nandina_heal(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		obj_id[] spawn_id = trial.getObjectsInDungeonWithObjVar(trial.getTop(self), "spawn_id");
		
		if (spawn_id == null || spawn_id.length == 0)
		{
			
			return SCRIPT_CONTINUE;
		}
		obj_id gorvo = null;
		
		for (int i=0; i<spawn_id.length; i++)
		{
			testAbortScript();
			if (getStringObjVar(spawn_id[i], "spawn_id").equals("gorvo"))
			{
				gorvo = spawn_id[i];
			}
		}
		
		if (!isIdValid(gorvo) || ai_lib.isDead(gorvo))
		{
			return SCRIPT_CONTINUE;
		}
		
		int healingReduction = getEnhancedSkillStatisticModifierUncapped(gorvo, "expertise_healing_reduction");
		float redux = (float)healingReduction / ((float)healingReduction + 75.0f);
		int toHeal = (int)(50000.0f - (50000.0f * redux));
		toHeal = toHeal < 1 ? 1 : toHeal;
		
		healing.healDamage(gorvo, HEALTH, toHeal);
		playClientEffectLoc(gorvo, "clienteffect/bacta_bomb.cef", getLocation(target), 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lelli_bleed(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("lelli_bleed", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lelli_ambush(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		setInvulnerable(self, true);
		setCreatureCoverVisibility(self, false);
		utils.setScriptVar(self, combat.DEFAULT_ATTACK_OVERRIDE, "null_attack");
		messageTo(self, "dropMines", trial.getSessionDict(self, "ambush"), 1.0f, false);
		messageTo(self, "performAmbush", trial.getSessionDict(self, "ambush"), 2.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_bomb_explode(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_bomb_explode", self, target, params, "", "", true) && !utils.hasScriptVar(self, "exploded"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		utils.setScriptVar(self, "exploded", 1);
		
		obj_id dungeon = getTopMostContainer(self);
		
		if (isIdValid(dungeon))
		{
			
			dictionary messageParams = trial.getSessionDict(dungeon);
			messageParams.put("npc", self);
			
			messageTo(dungeon, "killNPC", messageParams, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_droideka_shield(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_droideka_shield", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_shield(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_shield", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_droideka_electrify(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_droideka_electrify", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		setHate(self, target, 1);
		
		playClientEffectObj(self, "appearance/pt_ig88_electrify.prt", self, "");
		playClientEffectObj(target, "appearance/pt_ig88_electrify_target.prt", target, "");
		
		removeObjVar(self, "ai.combat.oneShotAction");
		removeObjVar(self, "oneShotActionComplete");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_flame_thrower(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		location where = getLocation(target);
		
		obj_id[] defenders = pvpGetTargetsInCone(self, self, where, 64, 10);
		
		Vector filteredDefenders = new Vector();
		filteredDefenders.setSize(0);
		
		if (defenders != null && defenders.length > 0)
		{
			for (int i = 0; i < defenders.length; i++)
			{
				testAbortScript();
				if (isIdValid(defenders[i]) && !isDead(defenders[i]) && isPlayer(defenders[i]))
				{
					filteredDefenders = utils.addElement(filteredDefenders, defenders[i]);
				}
			}
		}
		
		if (filteredDefenders != null && filteredDefenders.size() > 0)
		{
			target = ((obj_id)(filteredDefenders.get(rand(0, filteredDefenders.size() - 1))));
		}
		
		if (!combatStandardAction("ig88_flame_thrower", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_flame_thrower_vertical(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return SCRIPT_OVERRIDE;
		}
		
		location where = getLocation(target);
		
		obj_id[] defenders = pvpGetTargetsInCone(self, self, where, 64, 10);
		
		Vector filteredDefenders = new Vector();
		filteredDefenders.setSize(0);
		
		if (defenders != null && defenders.length > 0)
		{
			for (int i = 0; i < defenders.length; i++)
			{
				testAbortScript();
				if (isIdValid(defenders[i]) && !isDead(defenders[i]) && isPlayer(defenders[i]))
				{
					filteredDefenders = utils.addElement(filteredDefenders, defenders[i]);
				}
			}
		}
		
		if (filteredDefenders != null && filteredDefenders.size() > 0)
		{
			target = ((obj_id)(filteredDefenders.get(rand(0, filteredDefenders.size() - 1))));
		}
		
		if (!combatStandardAction("ig88_flame_thrower_vertical", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id getRocketTarget(obj_id self, obj_id target) throws InterruptedException
	{
		obj_id dungeon = getTopMostContainer(self);
		
		if (!isIdValid(dungeon))
		{
			return target;
		}
		
		obj_id[] targets = trial.getNonStealthedTargetsInCell(dungeon, "r1");
		
		if (targets == null || targets.length <= 0)
		{
			dictionary sessionDict = new dictionary();
			
			messageTo(dungeon, "ig88_failed", sessionDict, 0, false);
			
			return target;
		}
		
		target = targets[rand(0, targets.length - 1)];
		
		if (!isIdValid(target))
		{
			debugSpeakMsg(self, "I couldn't find a valid target.");
			return target;
		}
		
		return target;
	}
	
	
	public int gcw_rocket_launch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_rocket_launch", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		playClientEffectObj(self, "appearance/pt_gcw_rocket_launch.prt", self, "");
		playClientEffectObj(self, "sound/wep_heavy_rocket_launcher.snd", self, "");
		
		playClientEffectLoc(target, "appearance/pt_ig88_rocket_target.prt", getLocation(target), 0f);
		
		dictionary rocketParams = new dictionary();
		
		rocketParams.put("loc", getLocation(target));
		
		messageTo(self, "gcw_rocket_hit", rocketParams, 2f, false);
		
		removeObjVar(self, "ai.combat.oneShotAction");
		removeObjVar(self, "oneShotActionComplete");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_rocket_launcher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_rocket_launcher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_rocket_hit(obj_id self, dictionary params) throws InterruptedException
	{
		location loc = params.getLocation("loc");
		
		playClientEffectLoc(self, "appearance/pt_gcw_rocket_hit.prt", loc, 0f);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_rocket_launch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		target = getRocketTarget(self, target);
		
		LOG("combat_actions", "gcw ig88_rocket_launch 1");
		
		if (!combatStandardAction("ig88_rocket_launch", self, target, params, "", ""))
		{
			LOG("combat_actions", "gcw ig88_rocket_launch FAIL self: "+ self + " target: "+ target + " params: "+ params);
			return SCRIPT_OVERRIDE;
		}
		
		playClientEffectObj(self, "appearance/pt_ig88_rocket_launch.prt", self, "");
		playClientEffectObj(self, "sound/wep_heavy_rocket_launcher.snd", self, "");
		
		playClientEffectLoc(target, "appearance/pt_ig88_rocket_target.prt", getLocation(target), 0f);
		
		dictionary rocketParams = new dictionary();
		
		rocketParams.put("loc", getLocation(target));
		
		messageTo(self, "ig88_rocket_hit", rocketParams, 2f, false);
		
		removeObjVar(self, "ai.combat.oneShotAction");
		removeObjVar(self, "oneShotActionComplete");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_rocket_launcher(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ig88_rocket_launcher", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_rocket_hit(obj_id self, dictionary params) throws InterruptedException
	{
		location loc = params.getLocation("loc");
		
		playClientEffectLoc(self, "appearance/pt_ig88_rocket_hit.prt", loc, 0f);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_shockwave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(self);
		
		String strLoc = ""+ loc.x + " "+ loc.y + " "+ loc.z + " "+ loc.cell + " "+ loc.x + " "+ loc.y + " "+ loc.z;
		
		if (!combatStandardAction("ig88_shockwave", self, target, strLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ig88_grenade(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(self);
		
		String strLoc = ""+ loc.x + " "+ loc.y + " "+ loc.z + " "+ loc.cell + " "+ loc.x + " "+ loc.y + " "+ loc.z;
		
		if (!combatStandardAction("ig88_grenade", self, target, strLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int nandina_fester(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("nandina_fester", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lelli_mine(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id[] hateList = getHateList(self);
		
		Vector vectorList = new Vector(Arrays.asList(hateList));
		vectorList = utils.shuffleArray(vectorList);
		
		for (int i=0; i<hateList.length && i < 4; i++)
		{
			testAbortScript();
			combatStandardAction("lelli_mine", self, hateList[i], params, "", "");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lelli_chain_cleave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("lelli_chain_cleave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kimaru_storm_pulse(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kimaru_storm_pulse", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kimaru_generate_storm(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id[] hateList = getHateList(self);
		location loc1 = new location(-63.6f, 16.f, 36.4f, "dungeon1", getCellId(trial.getTop(self), "r2"));
		location loc2 = new location(-69.8f, 15.0f, 9.6f, "dungeon1", getCellId(trial.getTop(self), "r2"));
		location loc3 = new location(-44.4f, 9f, 1.5f, "dungeon1", getCellId(trial.getTop(self), "r2"));
		location loc4 = new location(-42.7f, 10.3f, 20.9f, "dungeon1", getCellId(trial.getTop(self), "r2"));
		location[] locList =
		{
			loc1, loc2, loc3, loc4
		};
		
		for (int i=0; i<4; i++)
		{
			testAbortScript();
			obj_id storm = create.object("kimaru_force_storm", locList[i]);
			playClientEffectLoc(hateList[0], "appearance/pt_kimaru_force_storm_birth.prt", locList[i], 0.0f);
			setMaster(storm, self);
			utils.setScriptVar(storm, trial.KIMARU_HATE_LIST, hateList);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kimaru_flame_patch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id[] hateList = getHateList(self);
		location loc = getLocation(hateList[rand(0, hateList.length -1)]);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("kimaru_flame_patch", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int heat_aspect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("heat_aspect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int acid_aspect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("acid_aspect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int elec_aspect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("elec_aspect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cold_aspect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("cold_aspect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_contagion_bomb(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_contagion_bomb", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_contagion(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_contagion", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_force_storm(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_force_storm", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_whirl_blast(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_whirl_blast", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int suin_rampage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("suin_rampage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int suin_healing_debuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("suin_healing_debuff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int axkva_warden_flame(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_warden_flame", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int macy_ywing_bomb(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("macy_ywing_bomb", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kav_tuken_killer(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kav_tuken_killer", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int king_rend(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("king_rend", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int king_head_crack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("king_head_crack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int king_sandstorm(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("king_sandstorm", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_drain(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("axkva_drain", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int axkva_crystalize(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id[] hateList = getHateList(self);
		obj_id topHate = getHateTarget(self);
		obj_id target1 = null;
		obj_id target2 = null;
		
		Vector targets = new Vector();
		targets.setSize(0);
		
		if (hateList == null || hateList.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i=0; i<hateList.length; i++)
		{
			testAbortScript();
			if (hateList[i] != topHate && !buff.hasBuff(hateList[i], "axkva_crystalize"))
			{
				targets.add(hateList[i]);
			}
		}
		
		if (targets == null || targets.size() == 0)
		{
			if (!buff.hasBuff(topHate, "axkva_crystalize"))
			{
				target1 = topHate;
			}
		}
		else
		{
			target1 = ((obj_id)(targets.get(rand(0, targets.size() -1))));
			targets.remove(target1);
			
			if (targets.size() > 0)
			{
				target2 = ((obj_id)(targets.get(rand(0, targets.size() -1))));
			}
		}
		
		if (isIdValid(target1))
		{
			utils.setScriptVar(target1, "axkva_min", self);
			buff.applyBuff(target1, self, "axkva_crystalize");
		}
		
		if (isIdValid(target2))
		{
			utils.setScriptVar(target2, "axkva_min", self);
			buff.applyBuff(target2, self, "axkva_crystalize");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int stasis_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "stasis_recourse"))
		{
			buff.applyBuff(self, self, "stasis_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int grenadier_cold(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(target);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("grenadier_cold", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		clearOneShotAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int grenadier_heat(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		location loc = getLocation(target);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("grenadier_heat", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		clearOneShotAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int grenadier_acid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(target);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("grenadier_acid", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		clearOneShotAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int grenadier_kinetic(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(target);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("grenadier_kinetic", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		clearOneShotAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void clearOneShotAction(obj_id self) throws InterruptedException
	{
		setObjVar(self, "ai.combat.oneShotAction", "non_attack");
	}
	
	
	public int krix_burn(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("krix_burn", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ito_nerve_gas(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ito_nerve_gas", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		playClientEffectLoc(self, "appearance/pt_sd_ito_nerve_gas.prt", getLocation(self), 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kenkirk_droid_blast(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(target);
		
		String strLoc = ""+ loc.x + " "+ loc.y + " "+ loc.z + " "+ loc.cell + " "+ loc.x + " "+ loc.y + " "+ loc.z;
		
		if (!combatStandardAction("kenkirk_droid_blast", self, target, strLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		playClientEffectLoc(target, "appearance/pt_sd_kenkirk_hallway_strike.prt", getLocation(target), 0f);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sd_obliterate(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sd_obliterate", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int krix_megaburn(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("krix_megaburn", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int krix_megapatch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("krix_megapatch", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int vicious_swing(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("vicious_swing", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int sp_action_regen(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_action_regen", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sm_modify_pistol_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id module = utils.getStaticItemInInventory(self, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "pistol_module_missing"));
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "sm_modify_pistol_1") || buff.hasBuff(self, "sm_modify_pistol_2") || buff.hasBuff(self, "sm_modify_pistol_3"))
		{
			sendSystemMessage(self, new string_id("spam", "pistol_buff_exists"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_modify_pistol_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doAnimationAction(self, "reload");
		
		decrementCount(module);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int sm_modify_pistol_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id module = utils.getStaticItemInInventory(self, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "pistol_module_missing"));
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "sm_modify_pistol_1") || buff.hasBuff(self, "sm_modify_pistol_2") || buff.hasBuff(self, "sm_modify_pistol_3"))
		{
			sendSystemMessage(self, new string_id("spam", "pistol_buff_exists"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_modify_pistol_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doAnimationAction(self, "reload");
		
		decrementCount(module);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int sm_modify_pistol_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id module = utils.getStaticItemInInventory(self, "item_reward_modify_pistol_01_01");
		
		if (!isIdValid(module) || getCount(module) <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "pistol_module_missing"));
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "sm_modify_pistol_1") || buff.hasBuff(self, "sm_modify_pistol_2") || buff.hasBuff(self, "sm_modify_pistol_3"))
		{
			sendSystemMessage(self, new string_id("spam", "pistol_buff_exists"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sm_modify_pistol_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doAnimationAction(self, "reload");
		
		decrementCount(module);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int kun_one_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_one_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_two_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_two_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_three_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_three_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_four_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_four_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_five_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_five_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_six_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_six_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_seven_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_seven_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_eight_sacrifice(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_eight_sacrifice", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int open_cultist_drain(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("open_cultist_drain", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		dictionary dict = new dictionary();
		dict.put("target", target);
		messageTo(self, "handle_drain_anim", dict, 0.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bm_siphon_master(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("bm_siphon_master", self, target, params, "", "", true))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int minder_luresh_drain(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("minder_luresh_drain", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int closed_fist_cleave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("closed_fist_cleave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int closed_fist_toss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("closed_fist_toss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int closed_fist_burn(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("closed_fist_burn", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int closed_fist_burn_damage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("closed_fist_burn_damage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lasher_increase_pain(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("lasher_increase_pain", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int subduer_decrease_damage(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("subduer_decrease_damage", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lasher_strike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("lasher_strike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int subduer_strike(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("subduer_strike", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int tomb_guard_cleave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("tomb_guard_cleave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int tomb_guard_toss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("tomb_guard_toss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretaker_blast(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("caretaker_blast", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretaker_drain(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("caretaker_drain", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretaker_shield(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("caretaker_shield", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretaker_shield_reflect(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("caretaker_shield_reflect", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int caretaker_vapors(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("caretaker_vapors", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int ct_guard_buff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("ct_guard_buff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_order_set(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_order_set", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_chaos_set(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_chaos_set", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_order_discipline(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_order_discipline", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_chaos_discord(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_chaos_discord", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int executioner_swipe(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("executioner_swipe", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_wrath_cold(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_wrath_cold", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_wrath_heat(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_wrath_heat", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_wrath_electrical(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_wrath_electrical", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_wrath_acid(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_wrath_acid", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_wrath_ward(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_wrath_ward", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_veng_slam(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_veng_slam", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_veng_wave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_veng_wave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_open_coil(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_open_coil", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_word_coil(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_word_coil", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_word_flurry(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_word_flurry", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int kun_word_shaken(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_word_shaken", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int kun_sword_multi(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		String[] actions = 
		{
			"kun_word_flurry",
			"kun_sword_break",
			"kun_sword_cleave",
			"kun_sword_defensive"
		};
		
		queueCommand(self, getStringCrc(actions[rand(0, actions.length - 1)].toLowerCase()), getHateTarget(self), "", COMMAND_PRIORITY_DEFAULT);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_sword_break(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_sword_break", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int kun_sword_cleave(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_sword_cleave", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int kun_sword_defensive(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("kun_sword_defensive", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dire_root_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "dire_root_recourse"))
		{
			buff.applyBuff(self, self, "dire_root_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int dire_snare_recourse(obj_id self, dictionary params) throws InterruptedException
	{
		if (!buff.hasBuff(self, "dire_snare_recourse"))
		{
			buff.applyBuff(self, self, "dire_snare_recourse");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int bountycheck(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id originalTarget = target;
		if (bounty_hunter.canCheckForBounty(self, target))
		{
			doAnimationAction(self, "anims.PLAYER_DRAW_DATAPAD");
			
			if (bounty_hunter.checkForPresenceOfBounty(self, target))
			{
				location spawnLoc = getLocation(target);
				setHealth(target, -5000);
				trial.cleanupObject(target);
				bounty_hunter.spawnFugitive(self, spawnLoc);
			}
			else
			{
				utils.setScriptVar(target,"bountyCheck",self);
				return SCRIPT_OVERRIDE;
			}
		}
		else
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isGod(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!exists(originalTarget))
		{
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_OVERRIDE;
	}
	
	
	public int fs_drain_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_drain_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_drain_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_drain_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_drain_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_drain_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_drain_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_drain_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fs_drain_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!buff.isInFocus(self))
		{
			sendSystemMessage(self, new string_id("spam", "fs_command_no_focus"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("fs_drain_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int col_jedi_statue_light_debuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("col_jedi_statue_light_debuff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int col_jedi_statue_dark_debuff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("col_jedi_statue_dark_debuff", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id weapon = getCurrentWeapon(self);
		String finalAttackName = heavyweapons.getHeavyWeaponDotName(self, weapon, true);
		
		if (finalAttackName == null || finalAttackName.equals(""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction(finalAttackName, self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_ae_hw_dot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id weapon = getCurrentWeapon(self);
		String finalAttackName = heavyweapons.getHeavyWeaponDotName(self, weapon, false);
		
		if (finalAttackName == null || finalAttackName.equals(""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction(finalAttackName, self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_4(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_4", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_5(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_5", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_hw_dm_crit_6(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_hw_dm_crit_6", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_cluster_bomb(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_cluster_bomb", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		location centerLoc = getLocation(target);
		int extraBomb = getEnhancedSkillStatisticModifierUncapped(self, "expertise_co_cluster_bomblet");
		
		int bombCount = 4 + extraBomb;
		
		for (int i=0; i<bombCount; i++)
		{
			testAbortScript();
			float randx = rand(4.0f, 6.0f);
			float randz = rand(4.0f, 6.0f);
			randx = rand(0,1) == 1 ? randx * - 1 : randx;
			randz = rand(0,1) == 1 ? randz * - 1 : randz;
			
			float x = centerLoc.x + randx;
			float z = centerLoc.z + randz;
			
			String targetLoc = x+" "+centerLoc.y+" "+z+" "+centerLoc.cell+" "+x+" "+centerLoc.y+" "+z;
			queueCommand(self, (-1949154824), target, targetLoc, COMMAND_PRIORITY_IMMEDIATE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_cluster_bomblet(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_cluster_bomblet", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_killing_spree(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_killing_spree", self, target, params, "", ""))
		{
			
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_empowered_attack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_empowered_attack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_first_aid_training(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_first_aid_training", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_double_time(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_double_time", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (buff.hasBuff(self, "co_position_secured"))
		{
			buff.removeBuff(self, "co_position_secured");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int co_killing_spree_attack(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("co_killing_spree_attack", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_bolt(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_CONTINUE;
		}
		
		obj_id speeder = getMountId(self);
		if (utils.getIntScriptVar(speeder, "airborne") != 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		float height = vehicle.getHoverHeight(speeder);
		location farTarget = utils.findLocInFrontOfTarget(speeder, height * 25.0f);
		obj_id[] allTar = getObjectsInCone(speeder, farTarget, height * 25.0f, 2.0f);
		
		Vector validTargets = new Vector();
		validTargets.setSize(0);
		
		if (allTar == null || allTar.length == 0)
		{
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle2", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			play2dNonLoopingSound(self, "sound/hoth_snowspeeder_blaster_fire_01.snd");
			
			return SCRIPT_CONTINUE;
		}
		for (int i=0; i<allTar.length; i++)
		{
			testAbortScript();
			
			if (allTar[i] == self || allTar[i] == speeder)
			{
				continue;
			}
			
			if (!pvpCanAttack(self, allTar[i]))
			{
				continue;
			}
			if (isDead(allTar[i]))
			{
				continue;
			}
			
			validTargets.add(allTar[i]);
		}
		
		if (validTargets == null || validTargets.size() == 0)
		{
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle2", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			play2dNonLoopingSound(self, "sound/hoth_snowspeeder_blaster_fire_01.snd");
			
			return SCRIPT_CONTINUE;
		}
		
		float minRange = height * 3.0f;
		for (int i=0; i<validTargets.size(); i++)
		{
			testAbortScript();
			if (ai_lib.isVehicle(((obj_id)(validTargets.get(i)))) && getDistance(speeder, target) > minRange)
			{
				target = ((obj_id)(validTargets.get(i)));
				break;
			}
		}
		
		if (!isIdValid(target))
		{
			target = ((obj_id)(validTargets.get(rand(0, validTargets.size() - 1))));
		}
		
		if (!isIdValid(target) || !exists(target))
		{
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", speeder, "muzzle2", farTarget, 200.0f, 1.0f, false, 0,0,0,0);
			play2dNonLoopingSound(self, "sound/hoth_snowspeeder_blaster_fire_01.snd");
			
			return SCRIPT_CONTINUE;
		}
		
		queueCommand(self, (-1310636694), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		float travelDistance = getDistance(target, self);
		
		createClientProjectileObjectToObject(self, "object/weapon/ranged/turret/shared_turret_energy.iff", getMountId(self), "muzzle", target, "", 200.0f, 1.0f, false, 0, 0, 0, 0);
		createClientProjectileObjectToObject(self, "object/weapon/ranged/turret/shared_turret_energy.iff", getMountId(self), "muzzle2", target, "", 200.0f, 1.0f, false, 0, 0, 0, 0);
		play2dNonLoopingSound(self, "sound/hoth_snowspeeder_blaster_fire_01.snd");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_shoot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("hoth_speeder_shoot", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_up(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id mount = getMountId(self);
		float height = vehicle.getHoverHeight(mount);
		int active = utils.getIntScriptVar(mount, "airborne");
		
		if (active == 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (height > 12.0f)
		{
			return SCRIPT_OVERRIDE;
		}
		
		height = height + 2.0f;
		height = height > 16.0f ? 16.0f : height;
		
		vehicle.setDampingHeight(mount, 100.0f);
		vehicle.setGlide(mount, 100.0f);
		
		vehicle.setHoverHeight(mount, height);
		
		messageTo(self, "hoth_speeder_up_reset", null, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_up_reset(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id mount = getMountId(self);
		vehicle.setDampingHeight(mount, 0.1f);
		vehicle.setGlide(mount, 2.5f);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_down(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id mount = getMountId(self);
		float height = vehicle.getHoverHeight(mount);
		
		int active = utils.getIntScriptVar(mount, "airborne");
		
		if (active == 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (height < 1.0f)
		{
			return SCRIPT_OVERRIDE;
		}
		
		height = height - 2.0f;
		height = height < 5.0f ? 5.0f : height;
		
		vehicle.setDampingHeight(mount, 100.0f);
		vehicle.setGlide(mount, 100.0f);
		
		vehicle.setHoverHeight(mount, height);
		
		messageTo(self, "hoth_speeder_down_reset", null, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_down_reset(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id mount = getMountId(self);
		vehicle.setDampingHeight(mount, 0.1f);
		vehicle.setGlide(mount, 2.5f);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_sapper_place(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_sapper_detonate(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("hoth_sapper_detonate", self, target, params, "", ""))
		{
			stopCombat(self);
			clearHateList(self);
			return SCRIPT_OVERRIDE;
		}
		
		kill(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_commando_rocket(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		return SCRIPT_CONTINUE;
	}
	
	public int hoth_scout_shield(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingBattlefieldVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		
		obj_id vehicleId = getMountId(self);
		
		int stasisBuff = buff.getBuffOnTargetFromGroup(vehicleId, "stasis");
		if (stasisBuff != 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!buff.canApplyBuff(vehicleId, "hoth_scout_shield"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("hoth_scout_shield", vehicleId, self, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_scout_cannon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("hoth_scout_cannon", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		location eggLoc = combat.getCommandGroundTargetLocation(params);
		location myLoc = getLocation(self);
		
		float travelDistance = getDistance(myLoc, eggLoc);
		float speed = travelDistance;
		
		playClientEffectObj(self, "sound/wep_veh_blaster.snd", self, "");
		createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", getMountId(self), "muzzle3", eggLoc, 150.0f, 1.0f, false, 0, 0, 0, 0);
		createClientProjectileObjectToLocation(self, "object/weapon/ranged/turret/shared_turret_energy.iff", getMountId(self), "muzzle", eggLoc, 150.0f, 1.0f, false, 0, 0, 0, 0);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_scout_ear_gun(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("hoth_scout_ear_gun", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		float travelDistance = getDistance(target, self);
		float speed = travelDistance;
		
		playClientEffectObj(self, "sound/wep_veh_blaster.snd", self, "");
		createClientProjectileObjectToObject(self, "object/weapon/ranged/rifle/shared_rifle_dlt20a.iff", getMountId(self), "muzzle2", target, "", 150.0f, 1.0f, false, 0, 0, 0, 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_scout_ear_gun_switch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		removeDefaultAttackOverride(self);
		overrideDefaultAttack(self, "hoth_scout_ear_gun");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_scout_rocket_switch(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		removeDefaultAttackOverride(self);
		overrideDefaultAttack(self, "hoth_scout_rocket");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_scout_rocket(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			removeDefaultAttackOverride(self);
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("hoth_scout_rocket", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		location eggLoc = combat.getCommandGroundTargetLocation(params);
		location myLoc = getLocation(self);
		createClientProjectileObjectToLocation(self, "object/weapon/ranged/rifle/shared_quest_rifle_flame_thrower.iff", getMountId(self), "muzzle4", eggLoc, 150.0f, 2.0f, false, 0, 0, 0, 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_atat_shoot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("hoth_atat_shoot", self, target, params, "", ""))
		{
			String targetName = getCreatureName(target);
			if (targetName == null)
			{
				targetName = getPlayerName(target);
			}
			LOG("slapHappy", "ATAT has Damaged: "+ target + " ( "+ targetName + " )");
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_atat_vehicle_shoot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			LOG("maggie", "hoth_atat_vehicle_shoot:::: TARGET is INVALID");
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(self))
		{
			LOG("maggie", "hoth_atat_vehicle_shoot:::: SELF is INVALID");
			return SCRIPT_CONTINUE;
		}
		
		if (!combatStandardAction("hoth_atat_vehicle_shoot", self, target, params, "", ""))
		{
			LOG("maggie", "hoth_atat_vehicle_shoot:::: Hit Called");
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_takeoff(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id mount = getMountId(self);
		
		vehicle.setMinimumSpeed(mount, 29.99f);
		vehicle.setMaximumSpeed(mount, 30.0f);
		vehicle.setTurnRateMin(mount, 28.0f);
		vehicle.setTurnRateMax(mount, 28.0f);
		vehicle.setAccelMin(mount, 25.0f);
		vehicle.setAccelMax(mount, 25.0f);
		vehicle.setDecel(mount, 300.0f);
		vehicle.setDampingRoll(mount, 1.0f);
		vehicle.setDampingPitch(mount, 1.0f);
		vehicle.setDampingHeight(mount, 0.1f);
		vehicle.setGlide(mount, 2.5f);
		vehicle.setBankingAngle(mount, 50.0f);
		vehicle.setHoverHeight(mount, 10.0f);
		vehicle.setAutoLevelling(mount, 0.25f);
		vehicle.setStrafe(mount, false);
		utils.setScriptVar(mount, "airborne", 1);
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_speeder_land(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!vehicle.isRidingVehicle(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		obj_id mount = getMountId(self);
		vehicle.initializeVehicle(mount);
		
		utils.setScriptVar(mount, "airborne", 0);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_ai_speeder_shoot(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (!isIdValid(target))
		{
			LOG("maggie", "hoth_ai_speeder_shoot:::: TARGET is INVALID");
			return SCRIPT_CONTINUE;
		}
		if (!isIdValid(self))
		{
			LOG("maggie", "hoth_ai_speeder_shoot:::: SELF is INVALID");
			return SCRIPT_CONTINUE;
		}
		
		if (!combatStandardAction("hoth_ai_speeder_shoot", self, target, params, "", ""))
		{
			LOG("maggie", "hoth_ai_speeder_shoot:::: Hit Called");
			return SCRIPT_CONTINUE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampa_boss_ice_throw_prep(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("wampa_boss_ice_throw_prep", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampa_boss_ice_throw(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("wampa_boss_ice_throw", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampa_boss_tauntaun_throw_prep(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (combatStandardAction("wampa_boss_tauntaun_throw_prep", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int wampa_boss_tauntaun_throw(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("wampa_boss_tauntaun_throw", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int hoth_atat_mine(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		location loc = getLocation(self);
		String targetLoc = ""+loc.x+" "+loc.y+" "+loc.z+" "+loc.cell+" "+loc.x+" "+loc.y+" "+loc.z;
		
		if (!combatStandardAction("hoth_atat_mine", self, target, targetLoc, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int vader_force_choke(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (isPlayer(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("vader_force_choke", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean performMedicGroupBuff(obj_id medic, obj_id target, String buffName, String params) throws InterruptedException
	{
		if (!isIdValid(medic) || !exists(medic))
		{
			return false;
		}
		if (!isIdValid(target) || !exists(target))
		{
			return false;
		}
		if (buffName == null || buffName.length() <= 0)
		{
			return false;
		}
		if (params == null || params.length() <= 0)
		{
			return false;
		}
		
		obj_id tgId = getGroupObject(target);
		obj_id sgId = getGroupObject(medic);
		boolean performed_buff = false;
		
		if (!isIdValid(tgId) || !isIdValid(sgId) || sgId != tgId)
		{
			performed_buff |= combatStandardAction(buffName, medic, target, params, "", "");
			obj_id beast = beast_lib.getBeastOnPlayer(target);
			if (isIdValid(beast) && exists(beast) && !isDead(beast) && getDistance(medic, beast) < 72.0f)
			{
				performed_buff |= combatStandardAction(buffName, medic, beast, params, "", "");
			}
			
			if (!performed_buff)
			{
				return false;
			}
		}
		else
		{
			obj_id[] members = getGroupMemberIds(sgId);
			
			for (int i=0; i<members.length; i++)
			{
				testAbortScript();
				if (isIdValid(members[i]) && exists(members[i]) && !isDead(members[i]) && canSee(medic, members[i]) && getDistance(medic, members[i]) < 72.0f)
				{
					performed_buff |= combatStandardAction(buffName, medic, members[i], params, "", "");
					obj_id beast = beast_lib.getBeastOnPlayer(members[i]);
					if (isIdValid(beast) && exists(beast) && !isDead(beast) && getDistance(medic, beast) < 72.0f)
					{
						performed_buff |= combatStandardAction(buffName, medic, beast, params, "", "");
					}
					
				}
			}
		}
		return performed_buff;
	}
	
	
	public int sp_smoke_mirrors(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("sp_smoke_mirrors", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int sp_shifty_setup(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!stealth.hasInvisibleBuff(self) && !buff.hasBuff(self, "sp_smoke_mirrors"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("sp_shifty_setup", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireCr1BlastCannon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("fireCr1BlastCannon", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_flame_jet_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.fire_potency");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (1393995298), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_flame_jet_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.fire_potency");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (1582663931), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_flame_jet_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.fire_potency");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (1519686988), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_flame_jet_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_flame_jet_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		String template = getTemplateName(self);
		
		if (template.equals("object/mobile/magnaguard.iff"))
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet_magnaguard.prt", self, "root", null, "flamejet");
		}
		else
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet.prt", self, "root", null, "flamejet");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_flame_jet_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_flame_jet_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		String template = getTemplateName(self);
		
		if (template.equals("object/mobile/magnaguard.iff"))
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet_magnaguard.prt", self, "root", null, "flamejet");
		}
		else
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet.prt", self, "root", null, "flamejet");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_flame_jet_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_flame_jet_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		String template = getTemplateName(self);
		
		if (template.equals("object/mobile/magnaguard.iff"))
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet_magnaguard.prt", self, "root", null, "flamejet");
		}
		else
		{
			playClientEffectObj(self, "appearance/pt_droid_flame_jet.prt", self, "root", null, "flamejet");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_droideka_shield_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.shield_heatsink");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("droid_droideka_shield_1", droid, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_droideka_shield_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.shield_heatsink");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("droid_droideka_shield_2", droid, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_droideka_shield_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.shield_heatsink");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("droid_droideka_shield_3", droid, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_battery_dump_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.dump_capacitors");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (1901658878), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_battery_dump_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.dump_capacitors");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (2082087975), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_battery_dump_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.dump_capacitors");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (2027633040), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_battery_dump_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_battery_dump_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_battery_dump_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_battery_dump_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_battery_dump_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_battery_dump_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_regenerative_plating_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.quickset_metal");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1431788883), droid, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_regenerative_plating_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.quickset_metal");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1477737356), droid, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_regenerative_plating_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.quickset_metal");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1557489213), droid, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_regenerative_plating_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_regenerative_plating_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_regenerative_plating_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_regenerative_plating_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_regenerative_plating_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_regenerative_plating_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_electrical_shock_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.arc_projector");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (958746220), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_electrical_shock_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.arc_projector");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (879126709), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_electrical_shock_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.arc_projector");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (816280834), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_electrical_shock_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_electrical_shock_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_electrical_shock_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_electrical_shock_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_electrical_shock_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_electrical_shock_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_torturous_needle_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.pain_inducer");
		
		if (commandPotency <= 0)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1109390517), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_torturous_needle_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.pain_inducer");
		
		if (commandPotency < 50)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1331483246), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int droid_torturous_needle_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		obj_id droid = pet_lib.validateDroidCommand(self);
		
		if (!isIdValid(droid))
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		int commandPotency = getIntObjVar(droid, "module_data.pain_inducer");
		
		if (commandPotency < 100)
		{
			sendSystemMessage(self, new string_id("spam", "cant_use_droid_command"));
			return SCRIPT_OVERRIDE;
		}
		
		queueCommand(droid, (-1268633563), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_torturous_needle_1(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_torturous_needle_1", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_torturous_needle_2(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_torturous_needle_2", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int server_droid_torturous_needle_3(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("server_droid_torturous_needle_3", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_afflicted_toss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("death_troopers_afflicted_toss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_death_wing_afflict(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("death_troopers_death_wing_afflict", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		doAnimationAction(target, "heavy_cough_vomit");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_afflicted_splat(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("death_troopers_afflicted_splat", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_afflicted_konk(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("death_troopers_afflicted_konk", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_dm_boss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_dm_boss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		float baseCooldownTime = getBaseCooldownTime("of_dm_boss");
		if (baseCooldownTime < 0)
		{
			return SCRIPT_OVERRIDE;
		}
		
		float cooldownTimeMod = getEnhancedSkillStatisticModifierUncapped(self, "expertise_cooldown_line_of_sure");
		setCommandTimerValue(self, TIMER_COOLDOWN, baseCooldownTime - (cooldownTimeMod/10));
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_ae_dm_boss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("of_ae_dm_boss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		doInspiredAction(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int of_del_ae_dm_boss(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		
		if (getTopMostContainer(self) != self)
		{
			sendSystemMessage(self, new string_id("spam", "cant_do_indoors"));
			return SCRIPT_OVERRIDE;
		}
		
		if (!combatStandardAction("of_del_ae_dm_boss", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		playClientEffectLoc(self, "clienteffect/combat_pt_orbitalstrike_low_pt.cef", getLocation(target), 0);
		
		if (successfulFastAttack(self, "of_aoe"))
		{
			setCommandTimerValue(self, TIMER_COOLDOWN, 0f);
		}
		
		doInspiredAction(self);
		
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
	
	
	public boolean hasVaharaCodyAntiVirus(obj_id self) throws InterruptedException
	{
		if (utils.hasScriptVar(self, "outbreak.innoculated") || buff.hasBuff(self, "death_troopers_inoculation"))
		{
			return true;
		}
		
		return false;
	}
	
	
	public int death_troopers_infection_1(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(self, "death_troopers_infection_1_delay", null, 0.5f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_infection_1_delay(obj_id self, dictionary params) throws InterruptedException
	{
		if (isInBlackwingInfectionedArea(self))
		{
			if (!hasVaharaCodyAntiVirus(self))
			{
				buff.applyBuff(self, self, "death_troopers_infection_2");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_infection_2(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(self, "death_troopers_infection_2_delay", null, 0.5f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_infection_2_delay(obj_id self, dictionary params) throws InterruptedException
	{
		if (isInBlackwingInfectionedArea(self))
		{
			if (!hasVaharaCodyAntiVirus(self))
			{
				trial.bumpSession(self, "outbreak_stacking_infection");
				buff.applyBuff(self, self, "death_troopers_infection_3");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_infection_3(obj_id self, dictionary params) throws InterruptedException
	{
		messageTo(self, "death_troopers_infection_3_delay", null, 0.5f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_infection_3_delay(obj_id self, dictionary params) throws InterruptedException
	{
		if (isInBlackwingInfectionedArea(self))
		{
			if (!hasVaharaCodyAntiVirus(self))
			{
				buff.applyBuff(self, self, "death_troopers_infection_3");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int death_troopers_inoculation(obj_id self, dictionary params) throws InterruptedException
	{
		if (isInBlackwingInfectionedArea(self))
		{
			if (utils.hasScriptVar(self, "outbreak.innoculated"))
			{
				utils.removeScriptVar(self, "outbreak.innoculated");
			}
			
			buff.applyBuff(self, self, "death_troopers_infection_1");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcw_atst_attack_tower(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		LOG("gcw_vehicle", "gcw_atst_attack_tower");
		
		if (!combatStandardAction("gcw_atst_attack_tower", self, target, params, "", ""))
		{
			LOG("gcw_vehicle", "gcw_atst_attack_tower 2");
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int firePulseCannon(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!combatStandardAction("firePulseCannon", self, target, params, "", ""))
		{
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
}
