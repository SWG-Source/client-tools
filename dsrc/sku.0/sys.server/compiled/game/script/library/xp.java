package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.lang.Math;
import java.util.*;
import script.library.beast_lib;
import script.library.buff;
import script.library.collection;
import script.library.combat;
import script.library.gcw;
import script.library.group;
import script.library.performance;
import script.library.pet_lib;
import script.library.space_flags;
import script.library.utils;


public class xp extends script.base_script
{
	public xp()
	{
	}
	public static final float COMBAT_GENERAL_EXCHANGE_RATE = 0.10f;
	public static final float JEDI_GENERAL_EXCHANGE_RATE = 0.18f;
	public static final float SOCIAL_ENTERTAINER_EXCHANGE_RATE = 1.0f;
	public static final float CRAFTING_MERCHANT_EXCHANGE_RATE = 0.25f;
	
	public static final float SQUADLEADER_XP_RATIO = 1.0f;
	public static final float SOCIAL_COMBAT_XP_MOD = 1.0f;
	
	public static final String VAR_SQUADLEADER_BASE = "squadleader";
	public static final String VAR_SQUADLEADER_XP = VAR_SQUADLEADER_BASE + ".xp";
	public static final String VAR_SQUADLEADER_AT_CAP = VAR_SQUADLEADER_BASE + ".atCap";
	public static final String VAR_SQUADLEADER_GROUP_SIZE = VAR_SQUADLEADER_BASE + ".groupSize";
	
	public static final String XP = "xp";
	
	public static final String WEAPON_TYPE = "weaponType";
	
	public static final String GRANT_XP_RESULT_VALUE = "delta";
	public static final String GRANT_XP_RETURN_DATA = "data";
	
	public static final float FACTIONAL_WINNING_XP_BONUS = 1.15f;
	
	public static final String SCOUT = "scout";
	public static final String TRAPPING = "trapping";
	public static final String BOUNTYHUNTER = "bountyhunter";
	public static final String SMUGGLER = "smuggler";
	public static final String SQUADLEADER = "squadleader";
	
	public static final String SLICING = "slicing";
	
	public static final String IMAGEDESIGNER = "imagedesigner";
	public static final String RANGER = "ranger";
	
	public static final String FARMER = "farmer";
	public static final String INDUSTRIALIST = "industralist";
	public static final String BIO_ENGINEER = "bio_engineer";
	public static final String BIO_ENGINEER_DNA_HARVESTING = "bio_engineer_dna_harvesting";
	
	public static final String HARVEST_ORGANIC = "resource_harvesting_organic";
	public static final String HARVEST_INORGANIC = "resource_harvesting_inorganic";
	
	public static final String CRAFTING_GENERAL = "crafting_general";
	public static final String CRAFTING_FOOD_GENERAL = "crafting_food_general";
	public static final String CRAFTING_MEDICINE_GENERAL = "crafting_medical_general";
	public static final String CRAFTING_CLOTHING_ARMOR = "crafting_clothing_armor";
	public static final String CRAFTING_CLOTHING_GENERAL = "crafting_clothing_general";
	public static final String CRAFTING_WEAPONS_GENERAL = "crafting_weapons_general";
	public static final String CRAFTING_WEAPONS_MELEE = "crafting_weapons_melee";
	public static final String CRAFTING_WEAPONS_RANGED = "crafting_weapons_ranged";
	public static final String CRAFTING_WEAPONS_MUNITION = "crafting_weapons_munition";
	public static final String CRAFTING_STRUCTURE_GENERAL = "crafting_structure_general";
	public static final String CRAFTING_DROID_GENERAL = "crafting_droid_general";
	public static final String CRAFTING_SHIPWRIGHT = "shipwright";
	
	public static final String MERCHANT = "merchant";
	
	public static final String CRAFTING_BIO_ENGINEER_CREATURE = "crafting_bio_engineer_creature";
	public static final String CRAFTING_BIO_ENGINEER_TISSUE = "crafting_bio_engineer_tissue";
	
	public static final String COMBAT_JEDI_ONEHANDLIGHTSABER = "combat_meleespecialize_onehandlightsaber";
	public static final String COMBAT_JEDI_TWOHANDLIGHTSABER = "combat_meleespecialize_twohandlightsaber";
	public static final String COMBAT_JEDI_POLEARMLIGHTSABER = "combat_meleespecialize_polearmlightsaber";
	public static final String COMBAT_JEDI_FORCE_POWER = "jedi_force";
	public static final String JEDI_GENERAL = "jedi_general";
	
	public static final String COMBAT_GENERAL = "combat_general";
	
	public static final String COMBAT_MELEESPECIALIZE_UNARMED = "combat_meleespecialize_unarmed";
	public static final String COMBAT_MELEESPECIALIZE_ONEHAND = "combat_meleespecialize_onehand";
	public static final String COMBAT_MELEESPECIALIZE_TWOHAND = "combat_meleespecialize_twohand";
	public static final String COMBAT_MELEESPECIALIZE_POLEARM = "combat_meleespecialize_polearm";
	
	public static final String COMBAT_RANGEDSPECIALIZE_RIFLE = "combat_rangedspecialize_rifle";
	public static final String COMBAT_RANGEDSPECIALIZE_PISTOL = "combat_rangedspecialize_pistol";
	public static final String COMBAT_RANGEDSPECIALIZE_CARBINE = "combat_rangedspecialize_carbine";
	public static final String COMBAT_RANGEDSPECIALIZE_HEAVY = "combat_rangedspecialize_heavy";
	
	public static final String COMBAT_THROWN = "combat_thrown";
	public static final String COMBAT_GRENADE = "combat_grenade";
	
	public static final String CREATUREHANDLER = "creaturehandler";
	public static final String COMBATMEDIC = "combatmedic";
	public static final String MEDICAL = "medical";
	
	public static final String SPACE_COMBAT_GENERAL = "space_combat_general";
	public static final String SPACE_PRESTIGE_IMPERIAL = "prestige_imperial";
	public static final String SPACE_PRESTIGE_REBEL = "prestige_rebel";
	public static final String SPACE_PRESTIGE_PILOT = "prestige_pilot";
	
	public static final String MUSIC = "music";
	public static final String DANCE = "dance";
	public static final String JUGGLING = "juggling";
	public static final String ENTERTAINER = "entertainer";
	
	public static final String QUEST_SOCIAL = "quest_social";
	public static final String QUEST_COMBAT = "quest_combat";
	public static final String QUEST_CRAFTING = "quest_crafting";
	public static final String QUEST_GENERAL = "quest_general";
	
	public static final String APPRENTICESHIP = "apprenticeship";
	
	public static final String PERMISSIONS_ONLY = "permissions_only";
	public static final String RAW_COMBAT = "raw_combat";
	public static final String PARTIAL_COMBAT = "partial_combat";
	public static final String PET_DAMAGE = "pet_damage";
	
	public static final String POLITICAL = "political";
	
	public static final String UNKNOWN = "unknown";
	
	public static final String STF_XP_N = "exp_n";
	
	public static final string_id SID_SCOUT = new string_id(STF_XP_N, SCOUT);
	
	public static final string_id SID_HARVEST_ORGANIC = new string_id(STF_XP_N, HARVEST_ORGANIC);
	public static final string_id SID_HARVEST_INORGANIC = new string_id(STF_XP_N, HARVEST_INORGANIC);
	
	public static final string_id PROSE_GRANT_XP = new string_id("base_player", "prose_grant_xp");
	public static final string_id PROSE_GRANT_GROUP_XP = new string_id("base_player", "prose_grant_group_xp");
	public static final string_id PROSE_GRANT_BUFF_XP = new string_id("base_player", "prose_grant_buff_xp");
	public static final string_id PROSE_GRANT_GROUP_BUFF_XP = new string_id("base_player", "prose_grant_group_buff_xp");
	public static final string_id PROSE_REVOKE_XP = new string_id("base_player", "prose_revoke_xp");
	public static final string_id PROSE_GRANT_XP_BONUS = new string_id("base_player", "prose_grant_xp_bonus");
	
	public static final string_id PROSE_GRANT_XP1 = new string_id("base_player", "prose_grant_xp1");
	public static final string_id PROSE_REVOKE_XP1 = new string_id("base_player", "prose_revoke_xp1");
	
	public static final String VAR_CREDIT_FOR_KILLS = "creditForKills";
	public static final String VAR_NPC_DAMAGE = VAR_CREDIT_FOR_KILLS + ".npcDamage";
	public static final String VAR_ATTACKER_LIST = VAR_CREDIT_FOR_KILLS + ".attackerList";
	public static final String VAR_DAMAGE_TALLY = VAR_CREDIT_FOR_KILLS + ".damageTally";
	public static final String VAR_DAMAGE_COUNT = VAR_CREDIT_FOR_KILLS + ".damageCount";
	public static final String VAR_COMBAT_TIMESTAMP = VAR_CREDIT_FOR_KILLS + ".timestamp";
	
	public static final int MAX_PLAYERS = 30;
	public static final int MAX_DISTANCE = 190;
	public static final double PRIM_KILL_PERCENT = .2;
	public static final double PERCENT_ADJUSTER = .0;
	public static final double PLAYER_RATIO_ADJUST = .1;
	public static final float COMBAT_XP_EXCHANGE = .10f;
	
	public static final float MAX_NPC_DAMAGE_PERCENT = 0.65f;
	public static final float PERCENT_DAMAGE_BAR_BASE = 0.00f;
	
	public static final float GROUP_XP_BONUS = 0.35f;
	public static final float GROUP_XP_DIVIDER = 0.6f;
	public static final int MAX_GROUP_BONUS_COUNT = 8;
	
	public static final float TRADER_XP_MOD = 7.6f;
	public static final float ENTERTAINER_XP_MOD = 3.4f;
	
	public static final String VAR_COMBAT_RESULTS = "combatResults";
	public static final String VAR_TOP_DAMAGE = VAR_COMBAT_RESULTS + ".top_damage";
	public static final String VAR_TOP_GROUP = VAR_COMBAT_RESULTS + ".top_group";
	
	public static final String VAR_TOP_DAMAGERS = VAR_COMBAT_RESULTS + ".top_damagers";
	public static final String VAR_LANDED_DEATHBLOW = VAR_COMBAT_RESULTS + ".landed_deathblow";
	
	public static final String HANDLER_XP_DELEGATED = "xpDelegated";
	
	public static final String TBL_SKILL = "datatables/skill/skills.iff";
	public static final String TBL_SPECIES_XP = "datatables/xp/species.iff";
	public static final String TBL_PLAYER_LEVEL_XP = "datatables/player/player_level.iff";
	
	public static final String CREATURES_TABLE = "datatables/mob/creatures.iff";
	
	public static final string_id SID_INSPIRE_BONUS = new string_id("performance", "perform_inspire_xp_bonus");
	
	public static final string_id SID_FLYTEXT_XP = new string_id("base_player", "prose_flytext_xp");
	public static final string_id SID_FLYTEXT_XP_GROUP = new string_id("base_player", "prose_flytext_xp_group");
	
	public static final int TRIAL_LEVEL_CAP = 25;
	public static final string_id SID_FREE_TRIAL_LEVEL_CAP = new string_id("base_player", "free_trial_level_cap");
	
	
	public static int grant(obj_id target, String xp_type, int amt) throws InterruptedException
	{
		return grant(target, xp_type, amt, true, null, null, null);
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, String callback) throws InterruptedException
	{
		return grant(target, xp_type, amt, true, callback, new dictionary(), getSelf());
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, String callback, dictionary callbackData) throws InterruptedException
	{
		return grant(target, xp_type, amt, true, callback, callbackData, getSelf());
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, String callback, dictionary callbackData, obj_id callbackId) throws InterruptedException
	{
		return grant(target, xp_type, amt, true, callback, callbackData, callbackId);
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, boolean verbose) throws InterruptedException
	{
		return grant(target, xp_type, amt, verbose, null, null, null);
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, boolean verbose, String callback, dictionary callbackData) throws InterruptedException
	{
		return grant(target, xp_type, amt, verbose, callback, callbackData, getSelf());
	}
	
	
	public static int grant(obj_id target, String xp_type, int amt, boolean verbose, String callback, dictionary callbackData, obj_id callbackId) throws InterruptedException
	{
		
		if (!isIdValid(target) || (xp_type == null) || (xp_type.equals("")))
		{
			return 0;
		}
		
		if (xp_type.indexOf(" ") > -1)
		{
			return 0;
		}
		
		amt = applySpeciesXpModifier(target, xp_type, amt);
		
		amt = applyInspirationBuffXpModifier(target, xp_type, amt);
		
		if (amt == 0)
		{
			return 0;
		}
		
		if (grantUnmodifiedExperience(target, xp_type, amt, verbose, callback, callbackData, callbackId))
		{
			return amt;
		}
		else
		{
			return 0;
		}
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt) throws InterruptedException
	{
		return grantUnmodifiedExperience(target, xp_type, amt, true, null, null, null);
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt, boolean verbose) throws InterruptedException
	{
		return grantUnmodifiedExperience(target, xp_type, amt, verbose, null, null, null);
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt, String callback) throws InterruptedException
	{
		return grantUnmodifiedExperience(target, xp_type, amt, true, callback, new dictionary(), getSelf());
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt, String callback, dictionary callbackData) throws InterruptedException
	{
		return grantUnmodifiedExperience(target, xp_type, amt, true, callback, callbackData, getSelf());
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt, String callback, dictionary callbackData, obj_id callbackId) throws InterruptedException
	{
		return grantUnmodifiedExperience(target, xp_type, amt, true, callback, callbackData, callbackId);
	}
	
	
	public static boolean grantUnmodifiedExperience(obj_id target, String xp_type, int amt, boolean verbose, String callback, dictionary callbackData, obj_id callbackId) throws InterruptedException
	{
		
		if (!isIdValid(target) || (xp_type == null) || (xp_type.equals("")))
		{
			return false;
		}
		
		if (xp_type.indexOf(" ") > -1)
		{
			return false;
		}
		
		dictionary params = new dictionary();
		
		params.put("xp_type", xp_type);
		params.put("amt", amt);
		
		if (callback != null && callback.length() > 0)
		{
			params.put("fromId", callbackId);
			params.put("fromCallback", callback);
			if (callbackData == null)
			{
				callbackData = new dictionary();
			}
			params.put("fromCallbackData", callbackData);
		}
		
		messageTo(target, "grantUnmodifiedExperienceOnSelf", params, 0.1f, !target.isAuthoritative());
		
		if (verbose)
		{
			displayXpMsg(target, xp_type, amt);
		}
		
		int currentXp = getExperiencePoints(target, xp_type);
		int xpCap = getExperienceCap(target, xp_type);
		
		return currentXp < xpCap;
	}
	
	
	public static boolean _grantUnmodifiedExperience(obj_id target, String xp_type, int amt, String callback, dictionary callbackData, obj_id callbackId) throws InterruptedException
	{
		
		int delta = 0;
		int prior = getExperiencePoints(target, xp_type);
		
		boolean result = false;
		
		if (grantExperiencePoints(target, xp_type, amt) != XP_ERROR)
		{
			result = true;
			
			int current = getExperiencePoints(target, xp_type);
			
			delta = current - prior;
			
			if (skill_template.isQualifiedForWorkingSkill(target))
			{
				skill_template.earnWorkingSkill(target);
			}
			
			metrics.doXpRateMetrics(target, xp_type, amt);
		}
		
		if (isIdValid(callbackId))
		{
			dictionary returnParams = new dictionary();
			
			returnParams.put(GRANT_XP_RESULT_VALUE, delta);
			if (callbackData != null)
			{
				returnParams.put(GRANT_XP_RETURN_DATA, callbackData);
			}
			messageTo(callbackId, callback, returnParams, 0.1f, !callbackId.isAuthoritative());
		}
		
		return result;
	}
	
	
	public static boolean grantCraftingXpChance(obj_id item, obj_id user, int chance) throws InterruptedException
	{
		
		if (!isIdValid(item))
		{
			return false;
		}
		
		removeObjVar(item, "crafting.creator.xp");
		removeObjVar(item, "crafting.creator.pxp");
		
		return true;
	}
	
	
	public static int applySpeciesXpModifier(obj_id target, String xp_type, int amt) throws InterruptedException
	{
		
		xp_type = toLower(xp_type);
		int mod_xp = (int) (amt * getSpeciesXpModifier(target, xp_type));
		
		if (utils.hasScriptVar(target, "buff.dish_ormachek.value"))
		{
			float xp_mod = utils.getFloatScriptVar(target, "buff.dish_ormachek.value");
			float f_mod = 1f + (xp_mod / 100f);
			
			int dur = 20;
			
			if (utils.hasScriptVar(target, "buff.dish_ormachek.duration"))
			{
				dur = (int) utils.getFloatScriptVar(target, "buff.dish_ormachek.duration");
			}
			
			dur--;
			if (dur <= 0)
			{
				buff.removeBuff(target, "dish_ormachek");
			}
			else
			{
				utils.setScriptVar(target, "buff.dish_ormachek.duration", dur);
			}
			
			mod_xp = (int) (mod_xp * f_mod);
		}
		
		return mod_xp;
	}
	
	
	public static int applyGroupXpModifier(obj_id target, int amt) throws InterruptedException
	{
		
		float mod = 1f;
		int modAmt = amt;
		
		if (amt > 0)
		{
			obj_id gid = getGroupObject(target);
			int activeGroupSize = getActiveGroupSize(gid);
			
			if (activeGroupSize <= 2)
			{
				return modAmt;
			}
			
			mod = getGroupXpModifier(target, activeGroupSize);
			modAmt = Math.round(amt * mod);
			
			modAmt = (int) (modAmt / (1 + ((activeGroupSize - 1) * GROUP_XP_DIVIDER)));
			
		}
		
		utils.setScriptVar(target, "combat.xp.groupBonus", mod);
		
		return modAmt;
	}
	
	
	public static int applyInspirationBuffXpModifier(obj_id target, String xp_type, int amt) throws InterruptedException
	{
		
		if (amt > 0)
		{
			float mod = getInspirationBuffXpModifier(target, xp_type);
			float modAmt = (float) amt * mod;
			
			return Math.round(modAmt);
		}
		else
		{
			return amt;
		}
	}
	
	
	public static float getGroupXpModifier(obj_id target, int activeGroupSize) throws InterruptedException
	{
		
		float bonusMod = 1f;
		
		float groupXPBonus = GROUP_XP_BONUS;
		
		if (activeGroupSize > 1)
		{
			bonusMod += groupXPBonus * activeGroupSize;
		}
		
		return bonusMod;
	}
	
	
	public static float getSpeciesXpModifier(obj_id target, String xp_type) throws InterruptedException
	{
		
		if (!isIdValid(target) || xp_type == null || xp_type.equals(""))
		{
			return 1f;
		}
		
		int species = getSpecies(target);
		
		String[] xpMods = dataTableGetStringColumn(TBL_SPECIES_XP, species);
		
		if (xpMods != null && xpMods.length > 0)
		{
			for (int i = 0; i < xpMods.length; i++)
			{
				testAbortScript();
				if (xpMods[i].startsWith(xp_type))
				{
					String[]s = split(xpMods[i], ':');
					
					if ((s != null) && (s.length == 2))
					{
						int val = utils.stringToInt(s[1]);
						
						if (val != -1)
						{
							return (100f + val) / 100f;
						}
					}
				}
			}
		}
		
		return 1f;
	}
	
	
	public static float getInspirationBuffXpModifier(obj_id target, String xp_type) throws InterruptedException
	{
		
		float mod = 1f;
		
		if (utils.hasScriptVar(target, "buff.xpBonus.types"))
		{
			String[] xpList = utils.getStringArrayScriptVar(target, "buff.xpBonus.types");
			
			if (xpList != null && xpList.length > 0)
			{
				for (int i = 0; i < xpList.length; i++)
				{
					testAbortScript();
					if (xpList[i].equals(xp_type))
					{
						mod += utils.getFloatScriptVar(target, "buff.xpBonus.value");
					}
				}
			}
		}
		
		if (utils.hasScriptVar(target, "buff.xpBonusGeneral.types"))
		{
			String[] xpList = utils.getStringArrayScriptVar(target, "buff.xpBonusGeneral.types");
			
			if (xpList != null && xpList.length > 0)
			{
				for (int i = 0; i < xpList.length; i++)
				{
					testAbortScript();
					if (xpList[i].equals(xp_type))
					{
						mod += utils.getFloatScriptVar(target, "buff.xpBonusGeneral.value");
					}
				}
			}
		}
		
		return mod;
	}
	
	
	public static void grantSquadLeaderXp(obj_id player, int amt) throws InterruptedException
	{
		if (!isIdValid(player) || !player.isLoaded() || amt < 1 || !hasSkill(player, "class_officer_phase1_novice"))
		{
			return;
		}
		
		dictionary resultData = new dictionary();
		
		resultData.put("player", player);
		resultData.put("amt", amt);
		grant(player, SQUADLEADER, amt, false, "grantSquadLeaderXpResult", resultData, player);
		
		return;
	}
	
	
	public static void grantSquadLeaderXpResult(obj_id player, int granted, int amt) throws InterruptedException
	{
		if (granted > 0)
		{
			int slxp = utils.getIntScriptVar(player, VAR_SQUADLEADER_XP);
			
			slxp += granted;
			utils.setScriptVar(player, VAR_SQUADLEADER_XP, slxp);
			
			if (granted < amt)
			{
				utils.setScriptVar(player, VAR_SQUADLEADER_AT_CAP, true);
			}
			
			notifySquadLeaderXp(player);
		}
	}
	
	
	public static void notifySquadLeaderXp(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !player.isLoaded())
		{
			return;
		}
		
		string_id sid_xp = new string_id(STF_XP_N, SQUADLEADER);
		
		int slxp = utils.getIntScriptVar(player, VAR_SQUADLEADER_XP);
		
		if (slxp > 0)
		{
			prose_package pp = getXpProsePackage(SQUADLEADER, slxp);
			
			if (pp != null)
			{
				sendSystemMessageProse(player, pp);
			}
		}
		
		if (utils.hasScriptVar(player, VAR_SQUADLEADER_AT_CAP))
		{
			prose_package ppAtCap = prose.getPackage(new string_id("base_player", "prose_hit_xp_cap"), sid_xp);
			
			sendSystemMessageProse(player, ppAtCap);
		}
		
		utils.removeScriptVarTree(player, VAR_SQUADLEADER_BASE);
	}
	
	
	public static int getRawCombatXP(obj_id player, obj_id npc) throws InterruptedException
	{
		
		if (isMob(npc) && (isPlayer(npc) || isIdValid(getMaster(npc))))
		{
			return 0;
		}
		
		float baseCombatXP = getLevelBasedXP(player, npc);
		
		return Math.round(baseCombatXP);
	}
	
	
	public static int getActiveGroupSize(obj_id gid) throws InterruptedException
	{
		if (!group.isGroupObject(gid))
		{
			return 1;
		}
		
		obj_id[] members = getGroupMemberIds(gid);
		
		if (members == null || members.length == 0)
		{
			return 1;
		}
		
		int count = 0;
		
		for (int i = 0; i < members.length; i++)
		{
			testAbortScript();
			if (members[i].isLoaded() && isPlayer(members[i]))
			{
				count++;
			}
			
			if (count >= MAX_GROUP_BONUS_COUNT)
			{
				break;
			}
		}
		
		return count;
	}
	
	
	public static int getLevelBasedXP(obj_id player, obj_id npc) throws InterruptedException
	{
		int level = getLevel(player);
		int levelDiff = combat.getAiLevelDiff(npc, player);
		
		if (levelDiff > 0)
		{
			level += levelDiff;
		}
		
		int xp = getLevelBasedXP(level);
		
		if (!isMob(npc))
		{
			xp = getIntObjVar(npc, "combat.intCombatXP");
		}
		
		float bonus = 0f;
		
		if (aiIsKiller(npc) || aiIsAggressive(npc) || aiIsAssist(npc))
		{
			bonus += 0.05f;
		}
		
		xp += (int) (xp * bonus);
		
		if (levelDiff < 0)
		{
			
			float maxLevelDiff = 10f;
			
			if (level > 20)
			{
				maxLevelDiff += (int) ((level - 20) / 6);
			}
			
			xp += (int) (xp * (levelDiff / maxLevelDiff));
			
			if (xp < 1)
			{
				xp = 1;
			}
		}
		
		return xp;
	}
	
	
	public static int getLevelBasedXP(int level) throws InterruptedException
	{
		if (level < 1)
		{
			level = 1;
		}
		
		return dataTableGetInt("datatables/mob/stat_balance.iff", level - 1, "XP");
	}
	
	
	public static String getWeaponXpType(int weapon_type) throws InterruptedException
	{
		String xp_type = "unknown_xp";
		
		switch (weapon_type)
		{
			case WEAPON_TYPE_RIFLE:
			xp_type = COMBAT_RANGEDSPECIALIZE_RIFLE;
			break;
			case WEAPON_TYPE_LIGHT_RIFLE:
			xp_type = COMBAT_RANGEDSPECIALIZE_CARBINE;
			break;
			case WEAPON_TYPE_PISTOL:
			xp_type = COMBAT_RANGEDSPECIALIZE_PISTOL;
			break;
			case WEAPON_TYPE_HEAVY:
			case WEAPON_TYPE_GROUND_TARGETTING:
			case WEAPON_TYPE_DIRECTIONAL:
			xp_type = COMBAT_RANGEDSPECIALIZE_RIFLE;
			break;
			case WEAPON_TYPE_1HAND_MELEE:
			xp_type = COMBAT_MELEESPECIALIZE_ONEHAND;
			break;
			case WEAPON_TYPE_2HAND_MELEE:
			xp_type = COMBAT_MELEESPECIALIZE_TWOHAND;
			break;
			case WEAPON_TYPE_UNARMED:
			xp_type = COMBAT_MELEESPECIALIZE_UNARMED;
			break;
			case WEAPON_TYPE_POLEARM:
			xp_type = COMBAT_MELEESPECIALIZE_POLEARM;
			break;
			case WEAPON_TYPE_THROWN:
			xp_type = COMBAT_GRENADE;
			break;
			case WEAPON_TYPE_WT_1HAND_LIGHTSABER:
			xp_type = COMBAT_MELEESPECIALIZE_ONEHAND;
			break;
			case WEAPON_TYPE_WT_2HAND_LIGHTSABER:
			xp_type = COMBAT_MELEESPECIALIZE_TWOHAND;
			break;
			case WEAPON_TYPE_WT_POLEARM_LIGHTSABER:
			xp_type = COMBAT_MELEESPECIALIZE_POLEARM;
			break;
			case combat.WEAPON_TYPE_FORCE_POWER:
			xp_type = COMBAT_MELEESPECIALIZE_UNARMED;
			break;
			default:
			xp_type = UNKNOWN;
			break;
		}
		
		return xp_type;
	}
	
	
	public static String getWeaponStringFromXPType(String strXPType) throws InterruptedException
	{
		if (strXPType.equals(COMBAT_RANGEDSPECIALIZE_RIFLE))
		{
			return "rifle";
		}
		else if (strXPType.equals(COMBAT_RANGEDSPECIALIZE_CARBINE))
		{
			return "carbine";
		}
		
		else if (strXPType.equals(COMBAT_RANGEDSPECIALIZE_PISTOL))
		{
			return "pistol";
		}
		
		else if (strXPType.equals(COMBAT_RANGEDSPECIALIZE_HEAVY))
		{
			return "heavy";
		}
		
		else if (strXPType.equals(COMBAT_MELEESPECIALIZE_ONEHAND))
		{
			return "onehandmelee";
		}
		
		else if (strXPType.equals(COMBAT_MELEESPECIALIZE_TWOHAND))
		{
			return "twohandmelee";
		}
		
		else if (strXPType.equals(COMBAT_MELEESPECIALIZE_UNARMED))
		{
			return "unarmed";
		}
		else if (strXPType.equals(COMBAT_MELEESPECIALIZE_POLEARM))
		{
			return "polearm";
		}
		else if (strXPType.equals(COMBAT_JEDI_ONEHANDLIGHTSABER))
		{
			return "onehandmelee";
		}
		else if (strXPType.equals(COMBAT_JEDI_TWOHANDLIGHTSABER))
		{
			return "twohandmelee";
		}
		
		else if (strXPType.equals(COMBAT_JEDI_POLEARMLIGHTSABER))
		{
			return "polearm";
		}
		return "";
	}
	
	
	public static boolean setupCreditForKills() throws InterruptedException
	{
		return true;
	}
	
	
	public static boolean cleanupCreditForKills() throws InterruptedException
	{
		obj_id self = getSelf();
		
		utils.removeScriptVarTree(self, VAR_CREDIT_FOR_KILLS);
		return true;
	}
	
	
	public static void updateCombatXpList(obj_id target, obj_id attacker, obj_id wpn, int dam) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(attacker) || !isIdValid(wpn))
		{
			return;
		}
		
		int weapon_type = getWeaponType(wpn);
		
		String strTemplateName = getTemplateName(wpn);
		
		if (strTemplateName == null || strTemplateName.equals(""))
		{
			LOG("DESIGNER_FATAL", "xp.updateCombatXpList: unable to getTemplateName(wpn) -> "+ wpn + ":"+ getName(wpn));
			return;
		}
		else
		{
			if (hasObjVar(wpn, WEAPON_TYPE))
			{
				weapon_type = getIntObjVar(wpn, WEAPON_TYPE);
			}
		}
		
		String xp_type = getWeaponXpType(weapon_type);
		
		if (xp_type == null || xp_type.equals(""))
		{
			LOG("DESIGNER_FATAL", "xp.updateCombatXpList: unable to getWeaponXpType(wpn) -> "+ wpn + ":"+ getName(wpn));
			return;
		}
		
		updateCombatXpList(target, attacker, xp_type, dam);
	}
	
	
	public static void updateCombatXpList(obj_id target, obj_id attacker, String xp_type, int dam) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(attacker) || xp_type == null || xp_type.equals(""))
		{
			return;
		}
		
		if (!utils.hasScriptVar(target, VAR_COMBAT_TIMESTAMP))
		{
			utils.setScriptVar(target, VAR_COMBAT_TIMESTAMP, getGameTime());
		}
		
		int tally = utils.getIntScriptVar(target, VAR_DAMAGE_TALLY);
		int newTally = dam + tally;
		
		utils.setScriptVar(target, VAR_DAMAGE_TALLY, newTally);
		
		int hitCount = utils.getIntScriptVar(target, VAR_DAMAGE_COUNT);
		
		hitCount++;
		utils.setScriptVar(target, VAR_DAMAGE_COUNT, hitCount);
		
		if (!isPlayer(attacker) && !pet_lib.isPet(attacker) && !beast_lib.isBeast(attacker) && ai_lib.getDifficultyClass(target) != ai_lib.DIFFICULTY_BOSS)
		{
			int npcDamage = utils.getIntScriptVar(target, VAR_NPC_DAMAGE);
			
			npcDamage += dam;
			utils.setScriptVar(target, VAR_NPC_DAMAGE, npcDamage);
			return;
		}
		
		if (dam > 0)
		{
			Vector attackerList = utils.getResizeableObjIdBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers");
			
			if (attackerList != null && attackerList.size() > 0)
			{
				if (utils.getElementPositionInArray(attackerList, attacker) == -1)
				{
					attackerList = utils.addElement(attackerList, attacker);
					utils.setBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers", attackerList);
				}
			}
			else
			{
				attackerList = utils.addElement(attackerList, attacker);
				utils.setBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers", attackerList);
			}
			
			String basePath = VAR_ATTACKER_LIST + "."+ attacker;
			String damPath = basePath + ".damage";
			
			int totalDamage = dam + utils.getIntScriptVar(target, damPath);
			
			utils.setScriptVar(target, damPath, totalDamage);
			
			String xpListBasePath = basePath + ".xp";
			String xpListPath = xpListBasePath + ".types";
			Vector xpTypes = utils.getResizeableStringBatchScriptVar(target, xpListPath);
			
			if (xpTypes != null && xpTypes.size() > 0)
			{
				if (utils.getElementPositionInArray(xpTypes, xp_type) == -1)
				{
					xpTypes = utils.addElement(xpTypes, xp_type);
					utils.setBatchScriptVar(target, xpListPath, xpTypes);
				}
			}
			else
			{
				xpTypes = utils.addElement(xpTypes, xp_type);
				utils.setBatchScriptVar(target, xpListPath, xpTypes);
			}
			
			String xpTypePath = xpListBasePath + "."+ xp_type;
			int xpTally = dam + utils.getIntScriptVar(target, xpTypePath);
			
			utils.setScriptVar(target, xpTypePath, xpTally);
			
		}
	}
	
	
	public static void removeXpListCombatant(obj_id target, obj_id combatant) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(combatant))
		{
			return;
		}
		
		Vector attackerList = utils.getResizeableObjIdBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers");
		
		if (attackerList == null || attackerList.size() == 0)
		{
			return;
		}
		
		int idx = utils.getElementPositionInArray(attackerList, combatant);
		
		if (idx == -1)
		{
			return;
		}
		
		attackerList = utils.removeElementAt(attackerList, idx);
		
		if (attackerList == null || attackerList.size() == 0)
		{
			return;
		}
		
		utils.setBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers", attackerList);
		utils.removeScriptVarTree(target, VAR_ATTACKER_LIST + "."+ combatant);
	}
	
	
	public static void assessCombatResults(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return;
		}
		
		if (!utils.hasScriptVar(target, VAR_ATTACKER_LIST + ".attackers"))
		{
			return;
		}
		
		if (utils.hasScriptVar(target, "storytellerid") || utils.hasObjVar(target, "storytellerid"))
		{
			return;
		}
		
		int npcDamage = utils.getIntScriptVar(target, VAR_NPC_DAMAGE);
		int tally = utils.getIntScriptVar(target, VAR_DAMAGE_TALLY);
		float npcDamagePercent = npcDamage / (float)tally;
		
		if (npcDamagePercent > MAX_NPC_DAMAGE_PERCENT)
		{
			return;
		}
		
		obj_id[] attackerList = utils.getObjIdBatchScriptVar(target, VAR_ATTACKER_LIST + ".attackers");
		
		if (attackerList == null || attackerList.length == 0)
		{
			return;
		}
		
		attackerList = utils.validateObjIdArray(attackerList);
		
		obj_var[] damArray = new obj_var[attackerList.length];
		
		for (int i = 0; i < attackerList.length; i++)
		{
			testAbortScript();
			damArray[i] = new obj_var(attackerList[i].toString(), utils.getIntScriptVar(target, VAR_ATTACKER_LIST + "." + attackerList[i] + ".damage"));
		}
		
		if (damArray == null || damArray.length == 0)
		{
			return;
		}
		
		obj_var[] killers = list.quickSort(0, attackerList.length - 1, damArray);
		
		if ((killers == null) || (killers.length == 0))
		{
			return;
		}
		
		obj_id[] topDamagers = getTopIndividualAttacker(killers);
		
		if (topDamagers != null && topDamagers.length > 0)
		{
			setObjVar(target, VAR_TOP_DAMAGERS, topDamagers);
		}
		
		dictionary groupDamage = new dictionary();
		int topDamage = Integer.MIN_VALUE;
		int gDam = 0;
		
		for (int i = 0; i < killers.length; i++)
		{
			testAbortScript();
			obj_var attackerDamageVar = killers[i];
			obj_id attacker = utils.stringToObjId(attackerDamageVar.getName());
			
			if (isIdValid(attacker))
			{
				int attackerDamage = attackerDamageVar.getIntData();
				
				gDam = updateGroupDamageDictionary(groupDamage, attacker, attackerDamage, target);
				
				if (gDam > topDamage)
				{
					topDamage = gDam;
				}
			}
		}
		
		Vector primaryKillers = new Vector();
		primaryKillers.setSize(0);
		
		java.util.Enumeration keys = groupDamage.keys();
		
		while (keys.hasMoreElements())
		{
			testAbortScript();
			String key = (String)(keys.nextElement());
			int val = groupDamage.getInt(key);
			
			if (val >= topDamage)
			{
				obj_id tmpId = utils.stringToObjId(key);
				
				if (isIdValid(tmpId))
				{
					primaryKillers = utils.addElement(primaryKillers, tmpId);
					
					if (!group.isGroupObject(tmpId))
					{
						incrementKillMeter(tmpId, 1);
					}
				}
			}
		}
		
		if ((primaryKillers == null) || (primaryKillers.size() == 0))
		{
			return;
		}
		
		int winnerIdx = rand(0, primaryKillers.size() - 1);
		obj_id winner = ((obj_id)(primaryKillers.get(winnerIdx)));
		
		if (!group.isGroupObject(winner) && !isPlayer(winner) && isMob(winner))
		{
			obj_id winnerMaster = getMaster(winner);
			
			if (!isIdNull(winnerMaster) && isIdValid(winnerMaster))
			{
				winner = winnerMaster;
			}
		}
		
		if (group.isGroupObject(winner))
		{
			for (int i = 0; i < killers.length; i++)
			{
				testAbortScript();
				obj_var attackerDamageVar = killers[i];
				obj_id attacker = utils.stringToObjId(attackerDamageVar.getName());
				
				if (isIdValid(attacker))
				{
					if (getGroupObject(attacker) == winner)
					{
						incrementKillMeter(attacker, 1);
					}
				}
			}
		}
		
		setObjVar(target, VAR_TOP_DAMAGE, topDamage);
		setObjVar(target, VAR_TOP_GROUP, winner);
		
		grantQuestKillCredit(winner, target);
		
		if (!hasObjVar(target, "combat.zeroXP"))
		{
			obj_id[] killerList = new obj_id[0];
			if (primaryKillers != null)
			{
				killerList = new obj_id[primaryKillers.size()];
				primaryKillers.toArray(killerList);
			}
			
			killers = grantCombatXp(target, killers);
			
			obj_id[] playerList = getPlayersFromKillerList(killerList);
			pet_lib.addToHarvestDroidArray(target, playerList);
			
			if (killers == null || killers.length == 0)
			{
				setObjVar(target, VAR_TOP_DAMAGE, 0);
				setObjVar(target, VAR_TOP_GROUP, obj_id.NULL_ID);
				
				obj_id[] placeHolder = new obj_id[1];
				placeHolder[0] = obj_id.NULL_ID;
				setObjVar(target, scenario.VAR_PRIMARY_KILLERS, placeHolder);
				return;
			}
		}
		
		setObjVar(target, scenario.VAR_PRIMARY_KILLERS, primaryKillers, resizeableArrayTypeobj_id);
	}
	
	
	public static obj_id[] getPlayersFromKillerList(obj_id[] killerList) throws InterruptedException
	{
		Vector players = new Vector();
		players.setSize(0);
		
		for (int i = 0; i < killerList.length; i++)
		{
			testAbortScript();
			if (group.isGroupObject(killerList[i]))
			{
				obj_id[]groupMembers = utils.getGroupMemberIds(killerList[i]);
				
				for (int k = 0; k < groupMembers.length; k++)
				{
					testAbortScript();
					if (isIdValid(groupMembers[k]) && exists(groupMembers[k]))
					{
						if (pet_lib.isPet(groupMembers[k]))
						{
							utils.addElement(players, getMaster(groupMembers[k]));
						}
						else
						{
							utils.addElement(players, groupMembers[k]);
						}
					}
				}
			}
			else
			{
				if (pet_lib.isPet(killerList[i]))
				{
					utils.addElement(players, getMaster(killerList[i]));
				}
				else
				{
					utils.addElement(players, killerList[i]);
				}
			}
		}
		
		if (players != null)
		{
			killerList = new obj_id[players.size()];
			players.toArray(killerList);
		}
		
		return killerList;
	}
	
	
	public static void grantQuestKillCredit(obj_id winner, obj_id target) throws InterruptedException
	{
		obj_id[] killList;
		
		if (group.isGroupObject(winner))
		{
			killList = getGroupMemberIds(winner);
		}
		else
		{
			killList = new obj_id[1];
			killList[0] = winner;
		}
		
		if (killList.length < 1)
		{
			return;
		}
		
		dictionary params = new dictionary();
		String creatureName = getCreatureName(target);
		
		if (creatureName == null || creatureName.equals(""))
		{
			return;
		}
		
		params.put("creatureName", creatureName);
		params.put("location", getLocation(target));
		params.put("socialGroup", ai_lib.getSocialGroup(target));
		
		params.put("col_faction", dataTableGetString(CREATURES_TABLE, creatureName, "col_faction"));
		params.put("difficultyClass", dataTableGetInt(CREATURES_TABLE, creatureName, "difficultyClass"));
		
		params.put("target", target);
		
		if (hasObjVar(target, "quest_spawner.spawned_by"))
		{
			params.put("spawnedBy", getObjIdObjVar(target, "quest_spawner.spawned_by"));
		}
		
		for (int i = 0; i < killList.length; i++)
		{
			testAbortScript();
			if (!hasObjVar(target, "soloCollection"))
			{
				
				if (!exists(killList[i]) || !isIdValid(killList[i]))
				{
					continue;
				}
				
				float distance = getDistance(target, killList[i]);
				
				if (distance < 128.0f && distance >= 0.0f)
				{
					
					messageTo(killList[i], "receiveCreditForKill", params, 0.0f, false);
				}
			}
			
			else
			{
				obj_id collectionOwner = getObjIdObjVar(target, "soloCollection");
				
				if (!exists(collectionOwner) || !isIdValid(collectionOwner))
				{
					continue;
				}
				
				float distance = getDistance(target, collectionOwner);
				
				if (distance < 128.0f && distance >= 0.0f)
				{
					
					messageTo(collectionOwner, "receiveCreditForKill", params, 0.0f, false);
				}
			}
		}
	}
	
	
	public static int updateGroupDamageDictionary(dictionary groupDamage, obj_id attacker, int attackerDamage, obj_id target) throws InterruptedException
	{
		if (!isIdValid(attacker) || !isIdValid(target) || attackerDamage < 1)
		{
			return attackerDamage;
		}
		
		if (beast_lib.isBeast(attacker))
		{
			obj_id master = getMaster(attacker);
			
			if (isIdValid(master) && exists(master))
			{
				attacker = master;
			}
		}
		
		obj_id attackerGroupId = getGroupObject(attacker);
		
		if (!isIdValid(attackerGroupId))
		{
			int currentPersonalDamage = groupDamage.getInt(attacker.toString());
			int totalPersonalDamage = currentPersonalDamage + attackerDamage;
			
			groupDamage.put(attacker.toString(), totalPersonalDamage);
			
			return totalPersonalDamage;
		}
		else
		{
			int currentGroupDamage = groupDamage.getInt(attackerGroupId.toString());
			int totalGroupDamage = currentGroupDamage + attackerDamage;
			
			groupDamage.put(attackerGroupId.toString(), totalGroupDamage);
			
			return totalGroupDamage;
		}
	}
	
	
	public static obj_var[] grantCombatXp(obj_id target, obj_var[] killers) throws InterruptedException
	{
		
		if (!isIdValid(target) || killers == null || killers.length == 0)
		{
			return null;
		}
		
		Vector ret = new Vector();
		ret.setSize(0);
		
		double damageTally = (double)getIntObjVar(target, VAR_TOP_DAMAGE);
		
		if (damageTally < 0)
		{
			debugServerConsoleMsg(target, "xp::grantCombatXp: damageTally("+ damageTally + ") < 0!! WTF happened in combat?");
			return null;
		}
		
		obj_id killCredit = getObjIdObjVar(target, VAR_TOP_GROUP);
		
		obj_id[] killList;
		
		if (group.isGroupObject(killCredit))
		{
			killList = getGroupMemberIds(killCredit);
		}
		else
		{
			killList = new obj_id[1];
			killList[0] = killCredit;
		}
		
		metrics.doKillMetrics(killCredit, target);
		
		String faction = factions.getFaction(target);
		int levelAmount = getLevel(target) * 10;
		
		obj_id[] allKillers = new obj_id[killers.length];
		
		for (int i = 0; i < killers.length; i++)
		{
			testAbortScript();
			allKillers[i] = utils.stringToObjId(killers[i].getName());
		}
		
		for (int i = 0; i < killers.length; i++)
		{
			testAbortScript();
			obj_var killerVar = killers[i];
			
			obj_id killer = allKillers[i];
			
			obj_id master = obj_id.NULL_ID;
			
			if (beast_lib.isBeast(killer) && !ai_lib.aiIsDead(killer))
			{
				master = getMaster(killer);
				
				if (!master.isLoaded() || master.isBeingDestroyed())
				{
					master = null;
				}
			}
			
			if (!utils.isObjIdInArray(killList, killer))
			{
				if (isIdValid(master))
				{
					if (!utils.isObjIdInArray(killList, master))
					{
						continue;
					}
				}
			}
			
			obj_id gcw_gain_object = null;
			
			if (isIdValid(master))
			{
				if (!utils.isObjIdInArray(allKillers, master))
				{
					gcw_gain_object = master;
				}
				
			}
			else
			{
				gcw_gain_object = killer;
			}
			
			if (isIdValid(gcw_gain_object) && !hasObjVar(target, gcw.GCW_POINT_OVERRIDE))
			{
				gcw.grantModifiedGcwPoints(target, gcw_gain_object, false, gcw.GCW_POINT_TYPE_GROUND_PVE, getName(target));
			}
			int dam = killerVar.getIntData();
			
			checkAndUpdateHuntingMissions(killer, target);
			
			if (isIdValid(killer) && killer.isLoaded() && !killer.isBeingDestroyed() && killer != target)
			{
				int xpTotal = getRawCombatXP(killer, target);
				
				xpTotal = applyGroupXpModifier(killer, xpTotal);
				
				if (isPlayer(killer) && beast_lib.isBeastMaster(killer))
				{
					obj_id beast = beast_lib.getBeastOnPlayer(killer);
					
					if (beast_lib.isValidBeast(beast) && getDistance(killer, beast) < MAX_DISTANCE && xpTotal > 1)
					{
						if (utils.isObjIdInArray(killList, killer) || utils.isObjIdInArray(killList, beast))
						{
							beast_lib.grantBeastExperience(beast);
							
							dictionary params = new dictionary();
							
							params.put("targetId", target);
							obj_id beastBCD = beast_lib.getBeastBCD(beast);
							
							messageTo(beastBCD, "beastKilledSomething", params, 1, false);
						}
					}
				}
				else
				{
					if (beast_lib.isBeast(killer) && beast_lib.isValidPlayer(master) && beast_lib.canBeastGetLevelBasedXP(killer, target))
					{
						
						if (utils.isObjIdInArray(allKillers, master))
						{
							continue;
						}
						else
						{
							beast_lib.grantBeastExperience(killer);
							
							dictionary params = new dictionary();
							
							params.put("targetId", target);
							obj_id beastBCD = beast_lib.getBeastBCD(killer);
							
							messageTo(beastBCD, "beastKilledSomething", params, 1, false);
						}
					}
				}
				
				if (!isPlayer(killer))
				{
					if ((beast_lib.isBeast(killer) || pet_lib.isPet(killer)) && !ai_lib.aiIsDead(killer) && isIdValid(master))
					{
						
						xpTotal = getRawCombatXP(master, target);
						
						ret = utils.addElement(ret, killerVar);
						
						if (!utils.isObjIdInArray(allKillers, master) && utils.isObjIdInArray(allKillers, killer))
						{
							grantCombatStyleXp(master, COMBAT_GENERAL, xpTotal);
							
							displayXpFlyText(master, master, xpTotal);
							displayXpMsg(master, null, xpTotal);
							
							double percentDamage = ((double) dam / damageTally) + PERCENT_ADJUSTER;
							factions.grantCombatFaction(master, target, percentDamage);
						}
					}
				}
				else if (utils.isObjIdInArray(killList, killer))
				{
					
					double percentDamage = ((double) dam / damageTally) + PERCENT_ADJUSTER;
					
					if (getDistance(killer, target) < MAX_DISTANCE)
					{
						grantCombatXpPerAttackType(killer, target, xpTotal);
						
						factions.grantCombatFaction(killer, target, percentDamage);
						
						if (faction != null)
						{
							factions.adjustSocialStanding(killer, faction, -levelAmount);
						}
						
						ret = utils.addElement(ret, killerVar);
					}
				}
				
				utils.removeScriptVar(killer, "combat.xp.groupBonus");
			}
		}
		
		if (ret == null || ret.size() == 0)
		{
			return null;
		}
		
		return (obj_var[])ret.toArray(new obj_var[0]);
	}
	
	
	public static void grantCombatXpPerAttackType(obj_id player, obj_id target, int totalXp) throws InterruptedException
	{
		
		if (!isIdValid(player) || (!isPlayer(player)) || !isIdValid(target) || totalXp < 1)
		{
			return;
		}
		
		String basePath = VAR_ATTACKER_LIST + "."+ player;
		String xpListBasePath = basePath + ".xp";
		String xpListPath = xpListBasePath + ".types";
		
		if (!utils.hasScriptVar(target, xpListPath))
		{
			return;
		}
		
		String[]xpTypes = utils.getStringBatchScriptVar(target, xpListPath);
		
		if (xpTypes == null || xpTypes.length == 0)
		{
			return;
		}
		
		if (hasScript(player, "theme_park.new_player.new_player") || hasObjVar(player, "handlePlayerCombatKill"))
		{
			dictionary webster = new dictionary();
			
			webster.put("xpTypes", xpTypes);
			webster.put("target", target);
			
			messageTo(player, "handlePlayerCombatKill", webster, 1, false);
		}
		
		dictionary d = new dictionary();
		
		String damPath = basePath + ".damage";
		int tally = utils.getIntScriptVar(target, damPath);
		
		tally -= utils.getIntScriptVar(target, xpListBasePath + "."+ PERMISSIONS_ONLY);
		
		if (tally < 1)
		{
			return;
		}
		
		int raw = 0;
		int bonusCombatXp = 0;
		int jediXp = 0;
		int generalXp = 0;
		int totalXpGranted = 0;
		
		for (int i = 0; i < xpTypes.length; i++)
		{
			testAbortScript();
			int val = utils.getIntScriptVar(target, xpListBasePath + "."+ xpTypes[i]);
			float xpPercent = ((float) (val) / (float) (tally));
			
			int intNewTotal = totalXp;
			
			if (xpTypes[i].equals(RAW_COMBAT))
			{
				intNewTotal = (int) (intNewTotal * xpPercent);
				bonusCombatXp += intNewTotal;
			}
			else if (xpTypes[i].equals(PARTIAL_COMBAT))
			{
				intNewTotal = (int) (intNewTotal * xpPercent);
				intNewTotal = (int) (intNewTotal * COMBAT_GENERAL_EXCHANGE_RATE);
				bonusCombatXp += intNewTotal;
			}
			else if (xpTypes[i].equals(COMBAT_GENERAL))
			{
				int amt = (int) (intNewTotal * xpPercent);
				
				if (amt < 1)
				{
					amt = 1;
				}
				
				generalXp += amt;
			}
			else if (isCombatXpType(xpTypes[i]))
			{
				int amt = (int) (intNewTotal * xpPercent);
				
				if (amt < 1)
				{
					amt = 1;
				}
				
				if ((xpTypes[i].equals(COMBAT_JEDI_ONEHANDLIGHTSABER) || xpTypes[i].equals(COMBAT_JEDI_TWOHANDLIGHTSABER) || xpTypes[i].equals(COMBAT_JEDI_POLEARMLIGHTSABER) || xpTypes[i].equals(COMBAT_JEDI_FORCE_POWER) || xpTypes[i].equals(JEDI_GENERAL)))
				{
					if (isJedi(player))
					{
						jediXp += amt;
					}
				}
				else
				{
					raw += amt;
					
					if (!xpTypes[i].equals(COMBAT_THROWN))
					{
						totalXpGranted += grantCombatStyleXp(player, xpTypes[i], amt);
					}
				}
			}
			else if (!xpTypes[i].equals(UNKNOWN) && !xpTypes[i].equals(PERMISSIONS_ONLY) && !xpTypes[i].equals(PET_DAMAGE))
			{
				int amt = (int) (intNewTotal * xpPercent);
				
				if (amt < 1)
				{
					amt = 1;
				}
				
				totalXpGranted += grantCombatStyleXp(player, xpTypes[i], amt);
			}
		}
		
		if (isJedi(player))
		{
			jediXp *= JEDI_GENERAL_EXCHANGE_RATE;
			totalXpGranted += grantCombatStyleXp(player, JEDI_GENERAL, jediXp);
			LOG("jedi", "Granting "+ jediXp + " to "+ player + " type is "+ JEDI_GENERAL);
		}
		else
		{
			generalXp += (int) (raw * COMBAT_GENERAL_EXCHANGE_RATE);
			
			if (generalXp < 0)
			{
				generalXp = 0;
			}
			
			if (bonusCombatXp > 0)
			{
				generalXp += bonusCombatXp;
			}
			
			if (generalXp > 0)
			{
				totalXpGranted += grantCombatStyleXp(player, COMBAT_GENERAL, generalXp);
			}
		}
		
		displayXpFlyText(player, player, totalXpGranted);
		displayXpMsg(player, null, totalXpGranted);
	}
	
	
	public static int grantSocialStyleXp(obj_id player, String xpType, int amount) throws InterruptedException
	{
		amount = Math.round(amount * ENTERTAINER_XP_MOD);
		
		String templateXp = skill_template.getTemplateSkillXpType(player, false);
		
		if (templateXp != null)
		{
			if (isSocialXpType(templateXp))
			{
				amount = grant(player, templateXp, amount, false);
			}
			else
			{
				amount = 0;
			}
		}
		else
		{
			amount = 0;
		}
		
		return amount;
	}
	
	
	public static int grantCraftingQuestXp(obj_id player, int amount) throws InterruptedException
	{
		
		String templateXp = skill_template.getTemplateSkillXpType(player, false);
		
		if (templateXp != null)
		{
			if (isCraftingXpType(templateXp))
			{
				amount = grant(player, templateXp, amount, false);
			}
			else
			{
				
				amount = 0;
			}
		}
		else
		{
			amount = 0;
		}
		
		return amount;
	}
	
	
	public static int grantCraftingStyleXp(obj_id player, String xpType, int amount) throws InterruptedException
	{
		
		amount = Math.round(amount * TRADER_XP_MOD);
		
		int merchantXP = (int) (amount * CRAFTING_MERCHANT_EXCHANGE_RATE);
		float xpRatio = skill_template.NON_TEMPLATE_XP_RATIO;
		
		if (xpType.equals(QUEST_CRAFTING))
		{
			merchantXP = 0;
			xpRatio = skill_template.QUEST_XP_RATIO;
		}
		
		String templateXp = skill_template.getTemplateSkillXpType(player, false);
		
		if (templateXp != null)
		{
			if (isCraftingXpType(templateXp))
			{
				if (!xpType.equals(templateXp))
				{
					amount = (int) (amount * xpRatio);
				}
				
				if (!templateXp.equals(MERCHANT))
				{
					merchantXP = (int) (merchantXP * xpRatio);
				}
				
				amount += merchantXP;
				amount = grant(player, templateXp, amount, false);
			}
			else
			{
				
				amount = 0;
			}
		}
		else
		{
			amount = 0;
		}
		
		return amount;
	}
	
	
	public static int grantCombatStyleXp(obj_id player, String xpType, int amount) throws InterruptedException
	{
		
		float xpRatio = skill_template.NON_TEMPLATE_XP_RATIO;
		
		if (xpType.equals(QUEST_COMBAT))
		{
			xpRatio = skill_template.QUEST_XP_RATIO;
		}
		
		String templateXp = skill_template.getTemplateSkillXpType(player, false);
		
		if (templateXp != null)
		{
			if (isCombatXpType(templateXp))
			{
				if (!xpType.equals(templateXp))
				{
					amount = (int) (amount * xpRatio);
				}
				
				amount = grant(player, templateXp, amount, false);
			}
			else
			{
				
				amount = 0;
			}
		}
		else
		{
			amount = 0;
		}
		
		return amount;
	}
	
	
	public static void displayXpMsg(obj_id player, String xpType, int amt) throws InterruptedException
	{
		
		if (xpType == null)
		{
			xpType = skill_template.getTemplateSkillXpType(player, true);
		}
		
		if (xpType == null || xpType.equals(""))
		{
			return;
		}
		
		if (xpType.equals(skill_template.NO_TEMPLATE_STARTING))
		{
			return;
		}
		
		int currentXp = getExperiencePoints(player, xpType);
		int xpCap = getExperienceCap(player, xpType);
		
		if (currentXp < xpCap)
		{
			float grpMod = 1f;
			
			if (utils.hasScriptVar(player, "combat.xp.groupBonus"))
			{
				grpMod = utils.getFloatScriptVar(player, "combat.xp.groupBonus");
			}
			float inspMod = getInspirationBuffXpModifier(player, xpType);
			
			prose_package pp = xp.getXpProsePackage(xpType, amt, grpMod, inspMod);
			
			if (pp != null)
			{
				
				if (isSpaceScene())
				{
					sendQuestSystemMessage(player, pp);
				}
			}
			
			if (currentXp + amt >= xpCap)
			{
				prose_package ppAtCap = prose.getPackage(new string_id("base_player", "prose_hit_xp_cap"), new string_id(STF_XP_N, xpType));
				
				if (isSpaceScene())
				{
					sendQuestSystemMessage(player, ppAtCap);
				}
			}
		}
	}
	
	
	public static void displayXpFlyText(obj_id player, obj_id target, int amount) throws InterruptedException
	{
		if (isFreeTrialAccount(player))
		{
			int playerLevel = getLevel(player);
			
			if (playerLevel >= TRIAL_LEVEL_CAP)
			{
				debugSpeakMsg(player, "I am a greater than or equal to level "+ xp.TRIAL_LEVEL_CAP);
				prose_package pp = new prose_package();
				prose.setDI(pp, xp.TRIAL_LEVEL_CAP);
				prose.setStringId(pp, SID_FREE_TRIAL_LEVEL_CAP);
				sendSystemMessageProse(player, pp);
				return;
			}
		}
		
		if (amount == 0)
		{
			return;
		}
		
		if (npe.hasReachedMaxTutorialLevel(player))
		{
			return;
		}
		
		prose_package pp = null;
		
		pp = prose.getPackage(SID_FLYTEXT_XP, amount);
		
		float groupModPct = 1.0f;
		
		if (utils.hasScriptVar(player, "combat.xp.groupBonus"))
		{
			float groupBonus = utils.getFloatScriptVar(player, "combat.xp.groupBonus");
			
			groupModPct = getGroupXpModifierPercentageOfMax(groupBonus);
		}
		
		float flyScale = getCombatXpFlyScale(groupModPct);
		color flyColor = getCombatXpFlyColor(groupModPct);
		
		showFlyTextPrivate(target, player, pp, flyScale, flyColor);
	}
	
	
	public static float getGroupXpModifierPercentageOfMax(float groupMod) throws InterruptedException
	{
		float groupModPct = 0;
		
		final float maxGroupMod = GROUP_XP_BONUS * MAX_GROUP_BONUS_COUNT;
		
		if (maxGroupMod > 0)
		{
			groupModPct = (groupMod - 1) / maxGroupMod;
		}
		if (groupModPct > 1)
		{
			groupModPct = 1;
		}
		
		return groupModPct;
	}
	
	
	public static float getCombatXpFlyScale(float groupModPct) throws InterruptedException
	{
		final float minScale = 1.5f;
		final float maxScale = 2.5f;
		final float scaleRange = maxScale - minScale;
		
		float scale = minScale + (scaleRange * groupModPct);
		
		return scale;
	}
	
	
	public static color getCombatXpFlyColor(float groupModPct) throws InterruptedException
	{
		final color minColor = new color(140, 15, 210, 255);
		final color maxColor = new color(180, 60, 240, 255);
		final int redRange = maxColor.getR() - minColor.getR();
		final int greenRange = maxColor.getG() - minColor.getG();
		final int blueRange = maxColor.getB() - minColor.getB();
		
		int red = minColor.getR() + (int) (redRange * groupModPct);
		int green = minColor.getG() + (int) (greenRange * groupModPct);
		int blue = minColor.getB() + (int) (blueRange * groupModPct);
		
		if (red > maxColor.getR())
		{
			red = maxColor.getR();
		}
		if (green > maxColor.getG())
		{
			green = maxColor.getG();
		}
		if (blue > maxColor.getB())
		{
			blue = maxColor.getB();
		}
		
		return new color(red, green, blue, 255);
	}
	
	
	public static boolean isCombatXpType(String xpType) throws InterruptedException
	{
		if (xpType.equals(COMBAT_GENERAL))
		{
			return true;
		}
		if (xpType.equals(COMBAT_RANGEDSPECIALIZE_RIFLE))
		{
			return true;
		}
		if (xpType.equals(COMBAT_RANGEDSPECIALIZE_CARBINE))
		{
			return true;
		}
		if (xpType.equals(COMBAT_RANGEDSPECIALIZE_PISTOL))
		{
			return true;
		}
		if (xpType.equals(COMBAT_RANGEDSPECIALIZE_HEAVY))
		{
			return true;
		}
		if (xpType.equals(COMBAT_MELEESPECIALIZE_ONEHAND))
		{
			return true;
		}
		if (xpType.equals(COMBAT_MELEESPECIALIZE_TWOHAND))
		{
			return true;
		}
		if (xpType.equals(COMBAT_MELEESPECIALIZE_UNARMED))
		{
			return true;
		}
		if (xpType.equals(COMBAT_MELEESPECIALIZE_POLEARM))
		{
			return true;
		}
		if (xpType.equals(COMBAT_THROWN))
		{
			return true;
		}
		if (xpType.equals(COMBAT_GRENADE))
		{
			return true;
		}
		if (xpType.equals(JEDI_GENERAL))
		{
			return true;
		}
		if (xpType.equals(COMBAT_JEDI_ONEHANDLIGHTSABER))
		{
			return true;
		}
		if (xpType.equals(COMBAT_JEDI_TWOHANDLIGHTSABER))
		{
			return true;
		}
		if (xpType.equals(COMBAT_JEDI_POLEARMLIGHTSABER))
		{
			return true;
		}
		if (xpType.equals(COMBAT_JEDI_FORCE_POWER))
		{
			return true;
		}
		if (xpType.equals(MEDICAL))
		{
			return true;
		}
		if (xpType.equals(COMBATMEDIC))
		{
			return true;
		}
		if (xpType.equals(CREATUREHANDLER))
		{
			return true;
		}
		if (xpType.equals(QUEST_COMBAT))
		{
			return true;
		}
		if (xpType.equals(QUEST_GENERAL))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isSocialXpType(String xpType) throws InterruptedException
	{
		if (xpType.equals(MUSIC))
		{
			return true;
		}
		if (xpType.equals(DANCE))
		{
			return true;
		}
		if (xpType.equals(JUGGLING))
		{
			return true;
		}
		if (xpType.equals(ENTERTAINER))
		{
			return true;
		}
		if (xpType.equals(IMAGEDESIGNER))
		{
			return true;
		}
		if (xpType.equals(QUEST_SOCIAL))
		{
			return true;
		}
		if (xpType.equals(QUEST_COMBAT))
		{
			return true;
		}
		if (xpType.equals(QUEST_GENERAL))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean isCraftingXpType(String xpType) throws InterruptedException
	{
		if (xpType.equals(HARVEST_INORGANIC))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_FOOD_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_MEDICINE_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_CLOTHING_ARMOR))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_CLOTHING_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_WEAPONS_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_WEAPONS_MELEE))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_WEAPONS_RANGED))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_WEAPONS_MUNITION))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_STRUCTURE_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_DROID_GENERAL))
		{
			return true;
		}
		if (xpType.equals(CRAFTING_SHIPWRIGHT))
		{
			return true;
		}
		if (xpType.equals(MERCHANT))
		{
			return true;
		}
		if (xpType.equals(QUEST_CRAFTING))
		{
			return true;
		}
		if (xpType.equals(QUEST_GENERAL))
		{
			return true;
		}
		if (xpType.equals("crafting"))
		{
			return true;
		}
		return false;
	}
	
	
	public static String[] getXpTypes(obj_id self) throws InterruptedException
	{
		Vector xpTypes = new Vector();
		xpTypes.setSize(0);
		
		if (dataTableOpen(TBL_SKILL))
		{
			String[]colXpType = dataTableGetStringColumnNoDefaults(TBL_SKILL, "XP_TYPE");
			
			if ((colXpType == null) || (colXpType.length == 0))
			{
				
				return null;
			}
			
			for (int i = 0; i < colXpType.length; i++)
			{
				testAbortScript();
				String type = colXpType[i];
				
				if (type != null && !type.equals("") && !type.equals("unobtainable") && !type.equals("jedi"))
				{
					
					if (utils.getElementPositionInArray(xpTypes, type) == -1)
					{
						xpTypes = utils.addElement(xpTypes, type);
					}
				}
			}
		}
		
		return (String[])xpTypes.toArray(new String[0]);
	}
	
	
	public static void checkAndUpdateHuntingMissions(obj_id objPlayer, obj_id objCreature) throws InterruptedException
	{
		if (!isMob(objCreature))
		{
			
			return;
		}
		
		LOG("missions", "checking for "+ objPlayer);
		obj_id[]objMissions = getMissionObjects(objPlayer);
		
		if (objMissions == null)
		{
			LOG("missions", "null missions");
			return;
		}
		
		for (int intI = 0; intI < objMissions.length; intI++)
		{
			testAbortScript();
			String strMissionType = getMissionType(objMissions[intI]);
			
			LOG("missions", "type is "+ strMissionType);
			
			if (strMissionType.equals("hunting"))
			{
				String strCreatureToKill = getStringObjVar(objMissions[intI], "strCreatureToKill");
				final String strCreatureType = getCreatureName(objCreature);
				
				LOG("missions", "creature to kill is "+ strCreatureToKill);
				LOG("missions", "creature type is "+ strCreatureType);
				
				if (strCreatureType.equals(strCreatureToKill))
				{
					
					int intQuantityToKill = getIntObjVar(objMissions[intI], "intQuantityToKill");
					
					intQuantityToKill = intQuantityToKill - 1;
					
					if (intQuantityToKill <= 0)
					{
						messageTo(objMissions[intI], "huntingSuccess", null, 0, true);
					}
					else
					{
						LOG("missions", "Quanitty Ledft is "+ intQuantityToKill);
						setObjVar(objMissions[intI], "intQuantityToKill", intQuantityToKill);
						string_id strName = new string_id("mob/creature_names", strCreatureType);
						prose_package ppKillsLeft = prose.getPackage(new string_id("mission/mission_generic", "hunting_kills_remaining"), strName, intQuantityToKill);
						
						sendSystemMessageProse(objPlayer, ppKillsLeft);
						return;
					}
				}
				
			}
		}
		return;
	}
	
	
	public static obj_id[] getTopIndividualAttacker(obj_var[] killers) throws InterruptedException
	{
		Vector tempArray = new Vector();
		tempArray.setSize(0);
		
		if (killers == null || killers.length <= 0)
		{
			return null;
		}
		
		obj_var alpha = killers[0];
		obj_id alphaObjId = utils.stringToObjId(alpha.getName());
		
		if (killers.length == 1)
		{
			tempArray = utils.addElement(tempArray, alphaObjId);
		}
		else
		{
			int alphaValue = alpha.getIntData();
			
			obj_var omega = killers[killers.length - 1];
			obj_id omegaObjId = utils.stringToObjId(omega.getName());
			int omegaValue = omega.getIntData();
			
			if (alphaValue > omegaValue)
			{
				tempArray = utils.addElement(tempArray, alphaObjId);
				
				for (int a = 1; a < killers.length; a++)
				{
					testAbortScript();
					obj_var next = killers[a];
					int nextValue = next.getIntData();
					
					if (nextValue == alphaValue)
					{
						tempArray = utils.addElement(tempArray, utils.stringToObjId(next.getName()));
					}
					else
					{
						break;
					}
				}
			}
			else if (alphaValue < omegaValue)
			{
				tempArray = utils.addElement(tempArray, omegaObjId);
				
				for (int om = (killers.length - 2); om >= 0; om--)
				{
					testAbortScript();
					obj_var next = killers[om];
					int nextValue = next.getIntData();
					
					if (nextValue == omegaValue)
					{
						tempArray = utils.addElement(tempArray, utils.stringToObjId(next.getName()));
					}
					else
					{
						break;
					}
				}
			}
		}
		
		obj_id[] topAttacker = utils.toStaticObjIdArray(tempArray);
		
		return topAttacker;
	}
	
	
	public static boolean displayForceSensitiveXP(obj_id player, String type) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("force_sensitive", "xp.convertForceSensitiveXP -- player is invalid.");
			return false;
		}
		
		if (type == null)
		{
			LOG("force_sensitive", "xp.convertForceSensitiveXP -- type is null.");
			return false;
		}
		
		int index = dataTableSearchColumnForString(type, "fs_type", "datatables/quest/force_sensitive/xp_conversion.iff");
		
		if (index < 0)
		{
			LOG("force_sensitive", "xp.convertForceSensitiveXP -- can't find type "+ type + " in xp datatable.");
			return false;
		}
		
		Vector valid_xp_types = new Vector();
		valid_xp_types.setSize(0);
		
		for (int i = index; i < dataTableGetNumRows("datatables/quest/force_sensitive/xp_conversion.iff"); i++)
		{
			testAbortScript();
			dictionary row = dataTableGetRow("datatables/quest/force_sensitive/xp_conversion.iff", i);
			String fs_type = row.getString("fs_type");
			
			if (fs_type == null || !fs_type.equals(type))
			{
				break;
			}
			
			String xp_type = row.getString("xp_type");
			
			if (xp_type != null && xp_type.length() > 0)
			{
				
				if (getExperiencePoints(player, xp_type) > 0)
				{
					valid_xp_types.add(xp_type);
				}
			}
		}
		
		if (valid_xp_types.size() > 0)
		{
			String[]dsrc = new String[valid_xp_types.size()];
			utils.setScriptVar(player, "force_sensitive.xp_convert", valid_xp_types);
			utils.setScriptVar(player, "force_sensitive.xp_convert_type", type);
			
			for (int i = 0; i < valid_xp_types.size(); i++)
			{
				testAbortScript();
				dsrc[i] = "@exp_n:" + ((String)(valid_xp_types.get(i)));
			}
			
			if (!hasScript(player, "quest.force_sensitive.fs_xp_convert"))
			{
				attachScript(player, "quest.force_sensitive.fs_xp_convert");
			}
			
			String type_name = localize(new string_id("exp_n", "fs_"+ type));
			
			int pid = sui.listbox(player, player, "Select the experience you wish to convert to "+ type_name + ".", sui.OK_CANCEL, "Experience Conversion", dsrc, "msgFSXPConvertSelected");
			
			utils.setScriptVar(player, "force_sensitive.sui_pid", pid);
		}
		else
		{
			sendSystemMessage(player, new string_id("quest/force_sensitive/utils", "convert_not_enough_for_conversion"));
		}
		
		return true;
	}
	
	
	public static prose_package getXpProsePackage(String xpType, int xpAmt) throws InterruptedException
	{
		return getXpProsePackage(xpType, xpAmt, 1f, 1f);
	}
	
	
	public static prose_package getXpProsePackage(String xpType, int xpAmt, float grpMod, float inspMod) throws InterruptedException
	{
		prose_package pp = new prose_package();
		
		string_id sid_xp = new string_id(STF_XP_N, xpType);
		
		prose.setTO(pp, sid_xp);
		
		float grpBonus = (grpMod * 100) - 100;
		float inspBonus = (inspMod * 100) - 100;
		
		String grpBonusString = (new Integer(Math.round(grpBonus))).toString();
		String inspBonusString = (new Integer(Math.round(inspBonus))).toString();
		
		prose.setDI(pp, Math.abs(xpAmt));
		
		if (xpAmt >= 0)
		{
			if (inspBonus > 0)
			{
				if (grpBonus > 0)
				{
					prose.setStringId(pp, PROSE_GRANT_GROUP_BUFF_XP);
					prose.setTT(pp, grpBonusString);
					prose.setTU(pp, inspBonusString);
				}
				else
				{
					prose.setStringId(pp, PROSE_GRANT_BUFF_XP);
					prose.setTT(pp, inspBonusString);
				}
			}
			else
			{
				if (grpBonus > 0)
				{
					prose.setStringId(pp, PROSE_GRANT_GROUP_XP);
					prose.setTT(pp, grpBonusString);
				}
				else
				{
					if (xpAmt == 1)
					{
						prose.setStringId(pp, PROSE_GRANT_XP1);
					}
					else
					{
						prose.setStringId(pp, PROSE_GRANT_XP);
					}
				}
			}
		}
		else
		{
			if (xpAmt == 1)
			{
				prose.setStringId(pp, PROSE_REVOKE_XP1);
			}
			else
			{
				prose.setStringId(pp, PROSE_REVOKE_XP);
			}
		}
		
		return pp;
	}
	
	
	public static int grantXpByTemplate(obj_id player, int amount) throws InterruptedException
	{
		
		String template = getSkillTemplate(player);
		
		String xpType = xp.QUEST_COMBAT;
		
		if (!isIdValid(player) || template == null)
		{
			return 0;
		}
		
		if (template.startsWith("trader"))
		{
			grantCraftingStyleXp(player, xp.QUEST_CRAFTING, amount);
			xpType = xp.QUEST_CRAFTING;
		}
		else if (template.startsWith("entertainer"))
		{
			grantSocialStyleXp(player, xp.QUEST_SOCIAL, amount);
			xpType = xp.QUEST_SOCIAL;
		}
		else
		{
			grantCombatStyleXp(player, xp.QUEST_COMBAT, amount);
			xpType = xp.QUEST_COMBAT;
		}
		
		xp.displayXpFlyText(player, player, amount);
		xp.displayXpMsg(player, xpType, amount);
		
		return amount;
	}
	
	
	public static int grantUnmodifiedXpByTemplate(obj_id player, int amount) throws InterruptedException
	{
		
		String template = getSkillTemplate(player);
		
		String xpType = xp.QUEST_COMBAT;
		
		if (!isIdValid(player) || template == null)
		{
			return 0;
		}
		
		if (template.startsWith("trader"))
		{
			xpType = xp.QUEST_CRAFTING;
		}
		else if (template.startsWith("entertainer"))
		{
			xpType = xp.QUEST_SOCIAL;
		}
		else
		{
			xpType = xp.QUEST_COMBAT;
		}
		
		String templateXp = skill_template.getTemplateSkillXpType(player, false);
		
		if (templateXp != null && amount > 0)
		{
			amount = grant(player, templateXp, amount, false);
		}
		
		xp.displayXpFlyText(player, player, amount);
		xp.displayXpMsg(player, xpType, amount);
		
		return amount;
	}
	
	
	public static int grantUnmodifiedXPPercentageOfLevel(obj_id player, float percentage) throws InterruptedException
	{
		int playerLevel = getLevel(player);
		if (playerLevel < 90)
		{
			int xpForCurrentLevel = dataTableGetInt(xp.TBL_PLAYER_LEVEL_XP, playerLevel - 1, "xp_required");
			int xpForNextLevel = dataTableGetInt(xp.TBL_PLAYER_LEVEL_XP, playerLevel, "xp_required");
			
			float xpGrantedFloat = (xpForNextLevel - xpForCurrentLevel)*percentage/100;
			int xpGranted = (int)xpGrantedFloat;
			
			if (grantUnmodifiedXpByTemplate(player, xpGranted) > 0)
			{
				return xpGranted;
			}
		}
		
		return 0;
	}
	
	
	public static void applyHealingCredit(obj_id self) throws InterruptedException
	{
		deltadictionary scriptVars = self.getScriptVars();
		Vector healers = scriptVars.getResizeableObjIdArray("healers");
		if (healers != null)
		{
			Iterator i = healers.iterator();
			dictionary params = new dictionary();
			params.put("target", self);
			while (i.hasNext())
			{
				testAbortScript();
				obj_id healer = (obj_id)i.next();
				if (isIdValid(healer))
				{
					messageTo(healer, "receiveCreditForCombatHealing", params, 0.0f, false);
				}
			}
		}
	}
	
	
	public static void grantMissionXp(obj_id player, int missionLevel) throws InterruptedException
	{
		if (isIdValid(player) && missionLevel > 0)
		{
			int xpToGrantForRealsies = getMissionXpAmount(player, missionLevel);
			
			if (xpToGrantForRealsies > 0)
			{
				
				grantCombatStyleXp(player, COMBAT_GENERAL, xpToGrantForRealsies);
				displayXpFlyText(player, player, xpToGrantForRealsies);
				
				int huySaidToUseTTForTheSecondNumber = ((missions.DAILY_MISSION_XP_REWARD - missions.getPlayerDailyCount(player)) -1);
				
				prose_package pp = new prose_package();
				prose.setStringId(pp, missions.DAILY_REWARD_XP);
				prose.setDI(pp, xpToGrantForRealsies);
				prose.setTT(pp, ""+huySaidToUseTTForTheSecondNumber);
				sendSystemMessageProse(player, pp);
				
			}
		}
	}
	
	
	public static int getMissionXpAmount(obj_id player, int missionLevel) throws InterruptedException
	{
		int xpToGrant = 0;
		int playerLevel = getLevel(player);
		int xpForCurrentLevel = dataTableGetInt(TBL_PLAYER_LEVEL_XP, playerLevel - 1, "xp_required");
		int xpForNextLevel = dataTableGetInt(TBL_PLAYER_LEVEL_XP, playerLevel, "xp_required");
		int levelDivisor = 0;
		
		int levelDelta = Math.abs(missionLevel - playerLevel);
		
		LOG("NewMission", "getMissionXpAmountLevelDelta: "+ levelDelta);
		
		if (xpForNextLevel <= 0)
		{
			return xpToGrant;
		}
		
		int xpForLevel = xpForNextLevel - xpForCurrentLevel;
		
		if (levelDelta > 15)
		{
			levelDivisor += levelDelta;
		}
		
		if (xpForLevel > 0)
		{
			
			int missionXpDivisor = missions.DAILY_MISSION_XP_LOW;
			
			if (playerLevel >= 70)
			{
				missionXpDivisor = missions.DAILY_MISSION_XP_MEDIUM;
			}
			
			if (playerLevel >= 80)
			{
				missionXpDivisor = missions.DAILY_MISSION_XP_HIGH;
			}
			
			missionXpDivisor += levelDivisor;
			
			if (missionXpDivisor > 0)
			{
				xpToGrant = xpForLevel / missionXpDivisor;
				
				int sanityXpAmount = xpForLevel / missions.DAILY_MISSION_XP_SANITY;
				if (xpToGrant > sanityXpAmount)
				{
					xpToGrant = sanityXpAmount;
				}
			}
		}
		return xpToGrant;
	}
	
	
	public static int grantCollectionXP(obj_id player, String collectionName) throws InterruptedException
	{
		float xpToGrant = 0;
		
		int playerLevel = getLevel(player);
		if (playerLevel == 90)
		{
			return 0;
		}
		
		int xpForCurrentLevel = dataTableGetInt(TBL_PLAYER_LEVEL_XP, playerLevel - 1, "xp_required");
		int xpForNextLevel = dataTableGetInt(TBL_PLAYER_LEVEL_XP, playerLevel, "xp_required");
		
		float xpForLevel = xpForNextLevel - xpForCurrentLevel;
		
		float xpModifier = dataTableGetFloat(collection.COLLECTION_REWARD_TABLE, collectionName, "xpModifier");
		
		long repeatSlotValue = getCollectionSlotValue(player, collectionName + "_tracker") - 1;
		if (repeatSlotValue > 0)
		{
			float repeatXpModifier = repeatCollectionXpModifier(player, repeatSlotValue);
			xpToGrant = xpModifier * xpForLevel * repeatXpModifier;
			return (int)xpToGrant;
		}
		
		xpToGrant = xpModifier * xpForLevel;
		
		return (int)xpToGrant;
	}
	
	
	public static int grantCollectionSpaceXP(obj_id player, String collectionName) throws InterruptedException
	{
		if (!isValidId(player))
		{
			return 0;
		}
		
		if (!space_skill.hasSpaceSkills(player))
		{
			return 0;
		}
		
		String profPrefix = space_flags.getProfessionPrefix(player);
		if (profPrefix == null || profPrefix.equals(""))
		{
			return 0;
		}
		
		float xpToGrant = 0.0f;
		int xpForCurrentLevel = -1;
		int xpForNextLevel = -1;
		
		if (space_flags.hasCompletedTierFour(player))
		{
			String tierFourSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER4_INDEXSTART];
			String masterSkill = profPrefix+"_master";
			xpForCurrentLevel = dataTableGetInt(TBL_SKILL, tierFourSkill, "XP_COST");
			xpForNextLevel = dataTableGetInt(TBL_SKILL, masterSkill, "XP_COST");
		}
		else if (space_flags.hasCompletedTierThree(player))
		{
			String tierThreeSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER3_INDEXSTART];
			String tierFourSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER4_INDEXSTART];
			xpForCurrentLevel = dataTableGetInt(TBL_SKILL, tierThreeSkill, "XP_COST");
			xpForNextLevel = dataTableGetInt(TBL_SKILL, tierFourSkill, "XP_COST");
		}
		else if (space_flags.hasCompletedTierTwo(player))
		{
			String tierTwoSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER2_INDEXSTART];
			String tierThreeSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER3_INDEXSTART];
			xpForCurrentLevel = dataTableGetInt(TBL_SKILL, tierTwoSkill, "XP_COST");
			xpForNextLevel = dataTableGetInt(TBL_SKILL, tierThreeSkill, "XP_COST");
			
		}
		else if (space_flags.hasCompletedTierOne(player))
		{
			String tierOneSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER1_INDEXSTART];
			String tierTwoSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER2_INDEXSTART];
			xpForCurrentLevel = dataTableGetInt(TBL_SKILL, tierOneSkill, "XP_COST");
			xpForNextLevel = dataTableGetInt(TBL_SKILL, tierTwoSkill, "XP_COST");
		}
		else
		{
			String tierOneSkill = profPrefix+space_flags.SKILL_NAMES[space_flags.TIER1_INDEXSTART];
			xpForCurrentLevel = 0;
			xpForNextLevel = dataTableGetInt(TBL_SKILL, tierOneSkill, "XP_COST");
		}
		
		if (xpForCurrentLevel == -1 || xpForNextLevel == -1)
		{
			return 0;
		}
		
		float xpForLevel = xpForNextLevel - xpForCurrentLevel;
		float xpModifier = dataTableGetFloat(collection.COLLECTION_REWARD_TABLE, collectionName, "xpModifier");
		
		long repeatSlotValue = getCollectionSlotValue(player, collectionName + "_tracker") - 1;
		if (repeatSlotValue > 0)
		{
			float repeatXpModifier = repeatCollectionXpModifier(player, repeatSlotValue);
			xpToGrant = xpModifier * xpForLevel * repeatXpModifier;
			return (int)xpToGrant;
		}
		
		xpToGrant = xpModifier * xpForLevel;
		return (int)xpToGrant;
	}
	
	
	public static float repeatCollectionXpModifier(obj_id player, long repeatSlotValue) throws InterruptedException
	{
		float repeatMultiplier = 0.0f;
		
		float adjustedRepeatSlotValue = (float)repeatSlotValue / 10.0f;
		repeatMultiplier = 1.0f - adjustedRepeatSlotValue;
		if (repeatMultiplier < 0.1f)
		{
			
			repeatMultiplier = 0.1f;
		}
		return repeatMultiplier;
	}
}
