package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.callable;
import script.library.pet_lib;
import script.library.utils;
import script.library.attrib;
import script.library.skill;
import script.library.ai_lib;
import script.library.space_flags;
import script.library.performance;
import script.library.combat;
import script.library.luck;
import script.library.skill;


public class factions extends script.base_script
{
	public factions()
	{
	}
	public static final int RESIGN_TIME = 0;
	public static final int COVERT_TIME = 300;
	
	public static final String VAR_RESIGNING = "faction_recruiter.resigning";
	
	public static final float BOTHAN_NEGATIVE_MOD = 0.90f;
	
	public static final String FACTION = "faction";
	public static final float FACTION_RATING_MIN = -5000.0f;
	public static final float FACTION_RATING_MAX = +5000.0f;
	public static final float NON_ALIGNED_FACTION_MAX = +1000.0f;
	public static final float FACTION_RATING_DECLARABLE_MIN = 200.0f;
	public static final float FACTION_RATING_SYMPATHIZER_MIN = 750.0f;
	public static final float FACTION_RATING_INVALID = 0.0f;
	public static final float MAX_FACTION_KILL_REWARD = 30.0f;
	public static final int NEWLY_DECLARED_INTERVAL = 300;
	
	public static final int AD_HOC_FACTION = -1;
	public static final int NPC_FACTION_START_INDEX = 2;
	
	public static final int REACTION_LIKE = 2;
	public static final int REACTION_POSITIVE = 1;
	public static final int REACTION_NEUTRAL = 0;
	public static final int REACTION_NEGATIVE = -1;
	public static final int REACTION_DISLIKE = -2;
	
	public static final int STATUS_FRIEND = 0;
	public static final int STATUS_NEUTRAL = 1;
	public static final int STATUS_ENEMY = 2;
	
	public static final String FACTION_IMPERIAL = "Imperial";
	public static final String FACTION_REBEL = "Rebel";
	public static final String FACTION_HUTT = "Hutt";
	public static final String FACTION_NEUTRAL = "Neutral";
	
	public static final int FACTION_FLAG_UNKNOWN = 0;
	public static final int FACTION_FLAG_REBEL = 1;
	public static final int FACTION_FLAG_IMPERIAL = 2;
	public static final int FACTION_FLAG_NEUTRAL = 3;
	
	public static final String IN_ADHOC_PVP_AREA = "pvp_faction.in_adhoc_pvp_area";
	
	public static final String FACTION_TABLE = "datatables/faction/faction.iff";
	
	public static final string_id PROSE_AWARD_FACTION = new string_id("base_player","prose_award_faction");
	public static final string_id PROSE_LOSE_FACTION = new string_id("base_player","prose_lose_faction");
	public static final string_id PROSE_MAX_FACTION = new string_id("base_player","prose_max_faction");
	public static final string_id PROSE_MIN_FACTION = new string_id("base_player","prose_min_faction");
	
	public static final string_id PROSE_TEF = new string_id("base_player","prose_tef");
	
	public static final String KASHYYYK = "Kashyyyk";
	public static final String HSSKOR = "Hsskor";
	
	public static final String TBL_RANK = "datatables/faction/rank.iff";
	public static final String GCW_RANK_TABLE = "datatables/gcw/gcw_rank.iff";
	
	public static final String COL_INDEX = "INDEX";
	public static final String COL_NAME = "NAME";
	public static final String COL_COST = "COST";
	public static final String COL_DELEGATE_MULTIPLIER = "DELEGATE_MULTIPLIER";
	
	public static final int MAXIMUM_RANK = 15;
	
	public static final String DATATABLE_ALLOWED_XP_TYPES = "datatables/npc/faction_recruiter/allowed_xp_types.iff";
	
	public static final String SCRIPT_PLAYER_RECRUITER = "npc.faction_recruiter.player_recruiter";
	public static final String SCRIPT_FACTION_ITEM = "npc.faction_recruiter.faction_item";
	
	public static final String VAR_TRAINING_SELECTION = "faction_recruiter.training_selection";
	public static final String VAR_TRAINING_COST = "faction_recruiter.cost";
	public static final String VAR_TRAINING_XP = "faction_recruiter.xp";
	public static final String VAR_AVAILABLE_ITEMS = "faction_recruiter.available_items";
	public static final String VAR_FACTION = "faction_recruiter.faction";
	public static final String VAR_DECLARED = "faction_recruiter.declared";
	public static final String VAR_FACTION_HIRELING = "faction_recruiter.faction_hireling";
	public static final String VAR_PLAYER = "faction_recruiter.player";
	public static final String VAR_NEWLY_DECLARED = "faction_recruiter.newly_declared";
	
	public static final string_id SID_RESIGN_COMPLETE = new string_id("faction_recruiter", "resign_complete");
	public static final string_id SID_COVERT_COMPLETE = new string_id("faction_recruiter", "covert_complete");
	public static final string_id SID_INVALID_AMOUNT_ENTERED = new string_id("faction_recruiter", "invalid_amount_entered");
	public static final string_id SID_NOT_ENOUGH_STANDING_SPEND = new string_id("faction_recruiter", "not_enough_standing_spend");
	public static final string_id SID_AMOUNT_TOO_SMALL = new string_id("faction_recruiter", "amount_to_spend_too_small");
	public static final string_id SID_EXPERIENCE_GRANTED = new string_id("faction_recruiter", "training_experience_granted");
	public static final string_id SID_ITEM_PURCHASED = new string_id("faction_recruiter", "item_purchase_complete");
	public static final string_id SID_ACQUIRE_HIRELING = new string_id("faction_recruiter", "hireling_purchase_complete");
	public static final string_id SID_TOO_MANY_HIRELINGS = new string_id("faction_recruiter", "too_many_hirelings");
	public static final string_id SID_HIRELING_RELEASED = new string_id("faction_recruiter", "hireling_released");
	public static final string_id SID_INVENTORY_FULL = new string_id("faction_recruiter", "inventory_full");
	public static final string_id SID_DATAPAD_FULL = new string_id("faction_recruiter", "datapad_full");
	public static final string_id SID_ORDER_PURCHASED = new string_id("faction_recruiter", "order_purchase_complete");
	public static final string_id SID_SCHEMATIC_PURCHASED = new string_id("faction_recruiter", "schematic_purchase_complete");
	public static final string_id SID_SCHEMATIC_DUPLICATE = new string_id("faction_recruiter", "schematic_duplicate");
	public static final string_id SID_MUST_BE_DECLARED = new string_id("faction_recruiter", "must_be_declared");
	public static final string_id SID_MUST_BE_FACTION_MEMBER = new string_id("faction_recruiter", "must_be_faction_member");
	public static final string_id SID_MUST_BE_DECLARED_USE = new string_id("faction_recruiter", "must_be_declared_use");
	public static final string_id SID_MUST_BE_FACTION_MEMBER_USE = new string_id("faction_recruiter", "must_be_faction_member_use");
	public static final string_id SID_MUST_BE_NOTONLEAVE = new string_id("faction_recruiter", "must_be_not_onleave");
	public static final string_id SID_ACHIEVED_RANK = new string_id("faction_recruiter", "achieved_rank");
	public static final string_id SID_DEMOTED_RANK = new string_id("faction_recruiter", "demoted_rank");
	public static final string_id SID_SUI_DELEGATE_FACTION = new string_id("faction_recruiter", "sui_delegate_faction");
	public static final string_id SID_SUI_DELEGATE_FACTION_TEXT = new string_id("faction_recruiter", "sui_delegate_faction_text");
	public static final string_id SID_SUI_DELEGATE_FACTION_TITLE = new string_id("faction_recruiter", "sui_delegate_faction_title");
	public static final string_id SID_SUI_DELEGATE_FACTION_CAT = new string_id("faction_recruiter", "sui_faction_delegate_cat");
	public static final string_id SID_SUI_FACTION_RESIG_TITLE = new string_id("faction_recruiter", "sui_faction_resig_title");
	public static final string_id SID_SUI_FACTION_RESIG_TEXT = new string_id("faction_recruiter", "sui_faction_resig_text");
	
	public static final string_id SID_INVALID_TRAINING_TYPE = new string_id("faction_recruiter", "invalid_training_type");
	public static final string_id SID_NO_RANK_NAME = new string_id("faction_recruiter", "no_rank_name");
	
	public static final string_id SID_INSPIRE_BONUS = new string_id("performance", "perform_inspire_faction_bonus");
	
	public static final string_id SID_FACTION_ADJUSTED_LOWER = new string_id("error_message", "faction_adjusted_lower");
	public static final string_id SID_FACTION_INVALID_LOOKAT = new string_id("error_message", "faction_invalid_lookat");
	public static final string_id SID_FACTION_CANT_GIVE_YOURSELF = new string_id("error_message", "faction_cant_give_yourself");
	public static final string_id SID_FACTION_ONLY_OTHER_PLAYERS = new string_id("error_message", "faction_only_other_players");
	public static final string_id SID_FACTION_NONE_TO_GIVE = new string_id("error_message", "faction_none_to_give");
	public static final string_id SID_FACTION_LACK_POINTS_TO_GIVE = new string_id("error_message", "faction_lack_points_to_give");
	public static final string_id SID_FACTION_TARGET_AT_CAP = new string_id("error_message", "faction_target_at_cap");
	public static final string_id SID_FACTION_ONLY_DELEGATE_N_POINTS = new string_id("error_message", "faction_only_delegate_n_points");
	public static final string_id SID_FACTION_DELEGATE_X_TO_TARGET = new string_id("error_message", "faction_delegate_x_to_target");
	public static final string_id SID_FACTION_DELEGATE_SYNTAX = new string_id("error_message", "faction_delegate_syntax");
	
	public static final String SCRIPTVAR_FACTIONAL_HELPER_SUI_ID = "factionalHelperSuiId";
	public static final String SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES = "factionalHelperSuiChoices";
	
	public static final String MERC_END_COVERT_IMPERIAL = "covert_imperial_mercenary_end";
	public static final String MERC_END_OVERT_IMPERIAL = "overt_imperial_mercenary_end";
	public static final String MERC_END_COVERT_REBEL = "covert_rebel_mercenary_end";
	public static final String MERC_END_OVERT_REBEL = "overt_rebel_mercenary_end";
	public static final String MERC_BEGIN_COVERT_IMPERIAL = "covert_imperial_mercenary_begin";
	public static final String MERC_BEGIN_OVERT_IMPERIAL = "overt_imperial_mercenary_begin";
	public static final String MERC_BEGIN_COVERT_REBEL = "covert_rebel_mercenary_begin";
	public static final String MERC_BEGIN_OVERT_REBEL = "overt_rebel_mercenary_begin";
	
	public static final string_id SID_MERC_IMPERIAL_SF_BEGIN = new string_id("gcw", "merc_imperial_special_forces_begin");
	public static final string_id SID_MERC_IMPERIAL_SF_END = new string_id("gcw", "merc_imperial_special_forces_end");
	public static final string_id SID_MERC_IMPERIAL_COMBATANT_BEGIN = new string_id("gcw", "merc_imperial_combatant_begin");
	public static final string_id SID_MERC_IMPERIAL_COMBATANT_END = new string_id("gcw", "merc_imperial_combatant_end");
	public static final string_id SID_MERC_REBEL_SF_BEGIN = new string_id("gcw", "merc_rebel_special_forces_begin");
	public static final string_id SID_MERC_REBEL_SF_END = new string_id("gcw", "merc_rebel_special_forces_end");
	public static final string_id SID_MERC_REBEL_COMBATANT_BEGIN = new string_id("gcw", "merc_rebel_combatant_begin");
	public static final string_id SID_MERC_REBEL_COMBATANT_END = new string_id("gcw", "merc_rebel_combatant_end");
	
	public static final string_id SID_MERC_IMPERIAL_SF_GRANTED = new string_id("gcw", "merc_imperial_sf_status_granted");
	public static final string_id SID_MERC_IMPERIAL_SF_TERMINATED = new string_id("gcw", "merc_imperial_sf_status_terminated");
	public static final string_id SID_MERC_IMPERIAL_COMBATANT_GRANTED = new string_id("gcw", "merc_imperial_combatant_status_granted");
	public static final string_id SID_MERC_IMPERIAL_COMBATANT_TERMINATED = new string_id("gcw", "merc_imperial_combatant_status_terminated");
	public static final string_id SID_MERC_REBEL_SF_GRANTED = new string_id("gcw", "merc_rebel_sf_status_granted");
	public static final string_id SID_MERC_REBEL_SF_TERMINATED = new string_id("gcw", "merc_rebel_sf_status_terminated");
	public static final string_id SID_MERC_REBEL_COMBATANT_GRANTED = new string_id("gcw", "merc_rebel_combatant_status_granted");
	public static final string_id SID_MERC_REBEL_COMBATANT_TERMINATED = new string_id("gcw", "merc_rebel_combatant_status_terminated");
	
	
	public static void goCovertWithDelay(obj_id objPlayer, float fltDelay) throws InterruptedException
	{
		
		if (!canGoCovert(objPlayer))
		{
			return;
		}
		setObjVar(objPlayer, "intChangingFactionStatus", 1);
		pvpPrepareToBeCovert(objPlayer);
		messageTo(objPlayer, "msgGoCovert", null, fltDelay, true);
		return;
	}
	
	public static void goCovert(obj_id objPlayer) throws InterruptedException
	{
		if (!hasObjVar(objPlayer, "intChangingFactionStatus"))
		{
			return;
		}
		removeObjVar(objPlayer, "intChangingFactionStatus");
		
		if (!canGoCovert(objPlayer))
		{
			return;
		}
		string_id strSpam = new string_id("faction_recruiter", "covert_complete");
		sendSystemMessage(objPlayer, strSpam);
		
		buff.removeAllAuraBuffs(objPlayer);
		
		pvpMakeCovert(objPlayer);
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+objPlayer+"|PLAYER NAME:"+getName(objPlayer)+"|ZONE:"+getCurrentSceneName()+"|Player has been set to covert");
		return;
	}
	
	public static void goOvertWithDelay(obj_id objPlayer, float fltDelay) throws InterruptedException
	{
		setObjVar(objPlayer, "intChangingFactionStatus", 1);
		pvpPrepareToBeDeclared(objPlayer);
		messageTo(objPlayer, "msgGoOvert", null, fltDelay, true);
		return;
	}
	
	
	public static void goOvert(obj_id objPlayer) throws InterruptedException
	{
		if (!hasObjVar(objPlayer, "intChangingFactionStatus"))
		{
			return;
		}
		
		removeObjVar(objPlayer, "intChangingFactionStatus");
		string_id strSpam = new string_id("faction_recruiter", "overt_complete");
		sendSystemMessage(objPlayer, strSpam);
		
		buff.removeAllAuraBuffs(objPlayer);
		pvpMakeDeclared(objPlayer);
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+objPlayer+"|PLAYER NAME:"+getName(objPlayer)+"|ZONE:"+getCurrentSceneName()+"|Player has been set to overt");
		return;
	}
	
	public static void goOnLeaveWithDelay(obj_id objPlayer, float fltDelay) throws InterruptedException
	{
		
		if (!canGoOnLeave(objPlayer))
		{
			return;
			
		}
		setObjVar(objPlayer, "intChangingFactionStatus", 1);
		pvpPrepareToBeNeutral(objPlayer);
		messageTo(objPlayer, "msgGoOnLeave", null, fltDelay, true);
		return;
	}
	
	
	public static void goOnLeave(obj_id objPlayer) throws InterruptedException
	{
		
		if (!hasObjVar(objPlayer, "intChangingFactionStatus"))
		{
			return;
		}
		removeObjVar(objPlayer, "intChangingFactionStatus");
		if (!canGoOnLeave(objPlayer))
		{
			return;
		}
		
		string_id strSpam = new string_id("faction_recruiter", "on_leave_complete");
		sendSystemMessage(objPlayer, strSpam);
		
		buff.removeAllAuraBuffs(objPlayer);
		
		pvpMakeOnLeave(objPlayer);
		CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+objPlayer+"|PLAYER NAME:"+getName(objPlayer)+"|ZONE:"+getCurrentSceneName()+"|Player has been set to On Leave");
	}
	
	public static void unequipFactionEquipment(obj_id self, boolean checkForDeclaredObjVar) throws InterruptedException
	{
		obj_id[] items = getContents(self);
		if (items == null)
		{
			LOG("LOG_CHANNEL", "player_recruiter::msgGoCovert -- "+ self + "'s contents are null.");
			return;
		}
		
		obj_id inv = getObjectInSlot(self, "inventory");
		for (int i = 0; i < items.length; i++)
		{
			testAbortScript();
			if (hasScript(items[i], factions.SCRIPT_FACTION_ITEM))
			{
				if (!checkForDeclaredObjVar || hasObjVar(items[i], factions.VAR_DECLARED))
				{
					
					String strTemplate = getTemplateName(items[i]);
					if (!strTemplate.endsWith("marine/armor_marine_backpack.iff"))
					{
						putInOverloaded(items[i], inv);
					}
				}
			}
		}
		
		obj_id appearanceInv = getAppearanceInventory(self);
		if (!isIdValid(appearanceInv))
		{
			LOG("LOG_CHANNEL", "Player "+ self + " has no valid appearance inventory. Skipping unequip faction appearance items...");
			return;
		}
		obj_id[] appInvItems = getContents(appearanceInv);
		if (appInvItems == null)
		{
			LOG("LOG_CHANNEL", "Player "+ self + " has an appearance inventory, but the contents were null. Skipping appearance inventory faction unequip...");
			return;
		}
		for (int i = 0; i < appInvItems.length; i++)
		{
			testAbortScript();
			if (hasScript(appInvItems[i], factions.SCRIPT_FACTION_ITEM))
			{
				if (!checkForDeclaredObjVar || hasObjVar(appInvItems[i], factions.VAR_DECLARED))
				{
					putInOverloaded(appInvItems[i], inv);
				}
			}
		}
		return;
	}
	
	
	public static boolean isDeclared(obj_id target) throws InterruptedException
	{
		if (pvpGetType( target ) == PVPTYPE_DECLARED)
		{
			int myFactionHash = pvpGetAlignedFaction( target );
			if (myFactionHash == (221551254) || myFactionHash == (221551254))
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean isCovert(obj_id target) throws InterruptedException
	{
		int pvpType = pvpGetType( target );
		if (pvpType == PVPTYPE_COVERT || pvpType == PVPTYPE_DECLARED)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static String getFaction(obj_id target) throws InterruptedException
	{
		if (isPlayer(target))
		{
			if (pvpGetType(target)==PVPTYPE_NEUTRAL)
			{
				return null;
			}
			return getFactionNameByHashCode( pvpGetAlignedFaction( target ) );
		}
		else
		{
			return getStringObjVar( target, FACTION );
		}
	}
	
	
	public static int getFactionFlag(obj_id target) throws InterruptedException
	{
		if (isRebel(target))
		{
			return FACTION_FLAG_REBEL;
		}
		else if (isImperial(target))
		{
			return FACTION_FLAG_IMPERIAL;
		}
		else
		{
			return FACTION_FLAG_NEUTRAL;
		}
	}
	
	
	public static boolean isRebelHelper(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			return false;
		}
		
		int currentMercenaryFaction = pvpNeutralGetMercenaryFaction(target);
		
		if (0 == currentMercenaryFaction)
		{
			return false;
		}
		
		if ((370444368) == currentMercenaryFaction)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isImperialHelper(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			return false;
		}
		
		int currentMercenaryFaction = pvpNeutralGetMercenaryFaction(target);
		
		if (0 == currentMercenaryFaction)
		{
			return false;
		}
		
		if ((-615855020) == currentMercenaryFaction)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isRebelorRebelHelper(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			return false;
		}
		
		if (isRebelHelper(target) || getFactionFlag(target) == FACTION_FLAG_REBEL)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isImperialorImperialHelper(obj_id target) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target))
		{
			return false;
		}
		
		if (isImperialHelper(target) || getFactionFlag(target) == FACTION_FLAG_IMPERIAL)
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean isSameFactionorFactionHelper(obj_id target, obj_id target2) throws InterruptedException
	{
		if (!isIdValid(target) || !exists(target) || !isIdValid(target2) || !exists(target2))
		{
			return false;
		}
		
		if ((isImperialorImperialHelper(target) && isImperialorImperialHelper(target2)) || isRebelorRebelHelper(target) && isRebelorRebelHelper(target2))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static float getFactionStanding(obj_id target, int intFaction) throws InterruptedException
	{
		return getFactionStanding(target, getFactionNameByHashCode(intFaction));
	}
	
	public static float getFactionStanding(obj_id target, String factionName) throws InterruptedException
	{
		if (!exists(target))
		{
			return 0.0f;
		}
		
		if (!isPlayer( target ))
		{
			if (getFaction( target) == factionName)
			{
				return FACTION_RATING_MAX;
			}
			else
			{
				return 0.0f;
			}
		}
		
		if (!hasObjVar( target, FACTION + "." + factionName ))
		{
			return 0.0f;
		}
		else
		{
			return getFloatObjVar( target, FACTION + "." + factionName );
		}
		
	}
	
	
	public static float getFactionMax(obj_id target, int intFactionId) throws InterruptedException
	{
		return getFactionMax(target, getFactionNameByHashCode(intFactionId));
	}
	
	
	public static float getFactionMax(obj_id target, String factionName) throws InterruptedException
	{
		return FACTION_RATING_MAX;
	}
	
	
	public static void validateFactionStanding(obj_id player, String factionName) throws InterruptedException
	{
		float currentStanding = getFactionStanding(player, factionName);
		float maxStanding = getFactionMax(player, factionName);
		if (currentStanding > maxStanding)
		{
			setFactionStanding(player, factionName, maxStanding);
			prose_package ppFactionLower = prose.getPackage(SID_FACTION_ADJUSTED_LOWER);
			prose.setTO(ppFactionLower, new string_id("faction/faction_names", factionName));
			prose.setDI(ppFactionLower, (int)maxStanding);
			sendSystemMessageProse(player, ppFactionLower);
		}
	}
	
	
	public static String getRankName(int rank, String faction) throws InterruptedException
	{
		return utils.packStringId(getRankNameStringId(rank, faction));
	}
	
	
	public static string_id getRankNameStringId(int rank, String faction) throws InterruptedException
	{
		String rankName = constructRankName(rank, faction);
		if (rankName != null && rankName.length() > 0)
		{
			return new string_id("gcw_rank",rankName);
		}
		
		CustomerServiceLog("faction", "WARNING: factions.getRankNameStringId() called with rank "+ rank +
		" that has no rank name associated with it");
		return SID_NO_RANK_NAME;
	}
	
	
	public static String constructRankName(int rank, String faction) throws InterruptedException
	{
		return toLower(faction)+"_rank"+rank;
	}
	
	
	public static int getRankCost(int rank) throws InterruptedException
	{
		return dataTableGetInt(TBL_RANK, rank, COL_COST);
	}
	
	
	public static int getAvailableFactionPoints(obj_id target, String factionName) throws InterruptedException
	{
		if (isIdValid(target) && isPlayer(target) && factionName != null && !factionName.equals(""))
		{
			int standing = (int)getFactionStanding(target, factionName);
			if (standing > 0)
			{
				int available = standing - (int)FACTION_RATING_DECLARABLE_MIN;
				if (available > 0)
				{
					return available;
				}
			}
		}
		
		return 0;
	}
	
	
	public static int getAvailableFactionPoints(obj_id target, int faction) throws InterruptedException
	{
		if (isIdValid(target) && isPlayer(target))
		{
			String factionName = getFactionNameByHashCode(faction);
			return getAvailableFactionPoints(target, factionName);
		}
		
		return 0;
	}
	
	
	public static void delegateFactionPoints(obj_id self, obj_id target, String params) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			sendSystemMessage(self, SID_FACTION_INVALID_LOOKAT);
			return;
		}
		
		if (target == self)
		{
			sendSystemMessage(self, SID_FACTION_CANT_GIVE_YOURSELF);
			return;
		}
		
		if (!isPlayer(target))
		{
			sendSystemMessage(self, SID_FACTION_ONLY_OTHER_PLAYERS);
			return;
		}
		
		if (!isDeclared(self))
		{
			sendSystemMessage(self, new string_id("base_player", "must_be_declared"));
			return;
		}
		
		int fac = pvpGetAlignedFaction(self);
		String sFac = getFaction(self);
		
		int available = getAvailableFactionPoints(self, fac);
		if (available <= 0)
		{
			sendSystemMessage(self, SID_FACTION_NONE_TO_GIVE);
			return;
		}
		
		if (utils.hasScriptVar(self, "delegate.pid"))
		{
			int oldpid = utils.getIntScriptVar(self, "delegate.pid");
			sui.closeSUI(self, oldpid);
			utils.removeScriptVarTree(self, "delegate");
		}
		
		int rank = pvpGetCurrentGcwRank(self);
		String faction = getFaction(self);
		
		int ratioFrom = dataTableGetInt(TBL_RANK, rank, "DELEGATE_RATIO_FROM");
		int ratioTo = dataTableGetInt(TBL_RANK, rank, "DELEGATE_RATIO_TO");
		if (ratioFrom <= 0 || ratioTo <= 0)
		{
			return;
		}
		
		float multiplier = (float)ratioTo/(float)ratioFrom;
		
		if (params == null || params.equals(""))
		{
			
			String title = utils.packStringId(SID_SUI_DELEGATE_FACTION);
			prose_package ppSuiTitle = prose.getPackage(SID_SUI_DELEGATE_FACTION_TEXT);
			prose.setTT(ppSuiTitle, getRankNameStringId(rank, faction));
			prose.setTO(ppSuiTitle, ""+ ratioFrom + ":"+ ratioTo);
			prose.setDI(ppSuiTitle, available);
			String prompt = " \0"+ packOutOfBandProsePackage(null, ppSuiTitle);
			
			int pid = sui.transfer(self, self, prompt, title, utils.packStringId(SID_SUI_DELEGATE_FACTION_TITLE), Math.round(available), utils.packStringId(SID_SUI_DELEGATE_FACTION_CAT), 0, "handleDelegateSui", ratioFrom, ratioTo);
			if (pid > -1)
			{
				utils.setScriptVar(self, "delegate.pid", pid);
				utils.setScriptVar(self, "delegate.target", target);
			}
			
			return;
		}
		else
		{
			java.util.StringTokenizer st = new java.util.StringTokenizer(params);
			String sAmt = st.nextToken();
			int amt = utils.stringToInt(sAmt);
			if (amt > 0)
			{
				int required = (int)Math.ceil(amt/multiplier);
				if (required > available)
				{
					prose_package ppLackPointsToGive = prose.getPackage(SID_FACTION_LACK_POINTS_TO_GIVE);
					prose.setTT(ppLackPointsToGive, target);
					prose.setTO(ppLackPointsToGive, ""+ amt);
					prose.setDI(ppLackPointsToGive, required);
					sendSystemMessageProse(self, ppLackPointsToGive);
				}
				else
				{
					int targetMax = (int)getFactionMax(target, sFac);
					int targetCur = (int)getFactionStanding(target, sFac);
					int targetDelta = targetMax - targetCur;
					if (targetDelta == 0)
					{
						prose_package ppAtCap = prose.getPackage(SID_FACTION_TARGET_AT_CAP);
						prose.setTT(ppAtCap, target);
						sendSystemMessageProse(self, ppAtCap);
						return;
					}
					
					if (amt > targetDelta)
					{
						prose_package ppOnlyDelegate = prose.getPackage(SID_FACTION_ONLY_DELEGATE_N_POINTS);
						prose.setTT(ppOnlyDelegate, target);
						prose.setDI(ppOnlyDelegate, targetDelta);
						sendSystemMessageProse(self, ppOnlyDelegate);
						return;
					}
					
					if (addUnmodifiedFactionStanding(self, sFac, -required))
					{
						addUnmodifiedFactionStanding(target, sFac, amt);
						prose_package ppDelegate = prose.getPackage(SID_FACTION_DELEGATE_X_TO_TARGET);
						prose.setTT(ppDelegate, target);
						prose.setDI(ppDelegate, amt);
						prose.setTO(ppDelegate, ""+ required);
						sendSystemMessageProse(self, ppDelegate);
						CustomerServiceLog("faction_perk", getFirstName(self) + "("+ self + ") delegated "+ amt + " points to "+ getFirstName(target) + " for a cost of "+ required + " points.");
					}
				}
				
				return;
			}
		}
		
		sendSystemMessage(self, SID_FACTION_DELEGATE_SYNTAX);
	}
	
	
	public static void resignFromFaction(obj_id player, int intFaction) throws InterruptedException
	{
		resignFromFaction(player, getFactionNameByHashCode(intFaction));
	}
	
	public static void resignFromFaction(obj_id player, String resign_faction) throws InterruptedException
	{
		if (isInAdhocPvpArea(player))
		{
			return;
		}
		
		if (isOnLeave(player))
		{
			string_id strSpam = new string_id("faction_recruiter", "resign_on_leave");
			sendSystemMessage(player, strSpam);
			removeObjVar(player, "intChangingFactionStatus");
			return;
		}
		
		if (!hasObjVar(player, "intChangingFactionStatus"))
		{
			return;
		}
		
		removeObjVar(player, "intChangingFactionStatus");
		if (!isIdValid(player) || !isPlayer(player) || resign_faction == null || resign_faction.equals(""))
		{
			return;
		}
		
		buff.removeAllAuraBuffs(player);
		
		if (space_flags.isRebelPilot(player))
		{
			space_skill.retire(player, space_skill.REBEL);
		}
		else if (space_flags.isImperialPilot(player))
		{
			space_skill.retire(player, space_skill.IMPERIAL);
		}
		
		CustomerServiceLog("player_faction", "|PLAYER:"+player+" |PLAYER NAME:"+getName(player)+" |PLAYER LEFT FACTION: "+resign_faction+" |OLD RANK WAS: "+pvpGetCurrentGcwRank(player));
		
		pvpMakeNeutral(player);
		pvpSetAlignedFaction(player, 0);
		
		String factionNormalized = toLower(resign_faction);
		if (factionNormalized.equals("imperial") || factionNormalized.equals("rebel"))
		{
			setObjVar(player, "lastFactionResignedFrom", resign_faction);
			setObjVar(player, "lastFactionResignedTime", getGameTime());
		}
		
		float factionStanding = getFactionStanding(player, resign_faction);
		
		if (factionStanding > -2500)
		{
			setFactionStanding(player, resign_faction, -2500);
		}
		
		unequipFactionEquipment(player, false);
		
		releaseFactionHirelings(player);
		
		prose_package pp = prose.getPackage(SID_RESIGN_COMPLETE, resign_faction);
		sendSystemMessageProse(player, pp);
	}
	
	
	public static void resignFromFaction(obj_id player) throws InterruptedException
	{
		resignFromFaction(player, getFaction(player));
	}
	
	
	public static boolean addFactionStanding(obj_id target, int intFaction, float value) throws InterruptedException
	{
		return addFactionStanding(target, getFactionNameByHashCode(intFaction), value);
		
	}
	
	public static boolean addFactionStanding(obj_id target, String factionName, float value) throws InterruptedException
	{
		if (!isIdValid(target))
		{
			return false;
		}
		
		return addUnmodifiedFactionStanding(target, factionName, value) && grantFactionBonus(target, factionName, value);
	}
	
	
	public static boolean addUnmodifiedFactionStanding(obj_id target, String factionName, float value, boolean verbose) throws InterruptedException
	{
		if (!isIdValid(target) || factionName == null || factionName.equals("") || value == 0.0f)
		{
			return false;
		}
		
		if (!isPlayer( target ))
		{
			setFaction( target, factionName );
			return false;
		}
		
		int factionNum = getFactionNumber(factionName);
		if (factionNum == AD_HOC_FACTION)
		{
			return false;
		}
		
		int playerAllowed = dataTableGetInt(FACTION_TABLE, factionNum, "playerAllowed");
		if (playerAllowed != 1)
		{
			return false;
		}
		
		if (utils.hasScriptVar(target, "buff.general_inspiration.value"))
		{
			float factionBonus = utils.getFloatScriptVar(target, "buff.general_inspiration.value");
			factionBonus /= 100.0f;
			factionBonus += 1.0f;
			
			value *= factionBonus;
		}
		
		float oldRating = getFactionStanding( target, factionName );
		float newRating = oldRating + value;
		
		float factionMax = getFactionMax( target, factionName );
		if (newRating < FACTION_RATING_MIN)
		{
			newRating = FACTION_RATING_MIN;
		}
		else if (newRating >= factionMax)
		{
			newRating = factionMax;
		}
		
		float actual_value = value;
		if (oldRating + value > newRating)
		{
			actual_value = newRating-oldRating;
		}
		
		if (newRating == oldRating)
		{
			return false;
		}
		
		if (newRating != 0.0f)
		{
			setObjVar(target, FACTION + "."+ factionName, newRating );
			if (verbose)
			{
				if ((int)value > 0)
				{
					prose_package pp = prose.getPackage(PROSE_AWARD_FACTION, getLocalizedFactionName( factionName ), (int)actual_value);
					sendSystemMessageProse(target, pp);
				}
				else
				{
					actual_value = actual_value * -1;
					prose_package pp = prose.getPackage( PROSE_LOSE_FACTION, getLocalizedFactionName( factionName ), (int)actual_value );
					sendSystemMessageProse( target, pp );
				}
			}
		}
		else
		{
			removeObjVar(target, FACTION + "."+ factionName );
		}
		
		if (verbose && oldRating != newRating)
		{
			if (newRating == factionMax)
			{
				prose_package pp = prose.getPackage(PROSE_MAX_FACTION, getLocalizedFactionName( factionName ));
				sendSystemMessageProse(target, pp);
			}
			else if (newRating == FACTION_RATING_MIN)
			{
				prose_package pp = prose.getPackage(PROSE_MIN_FACTION, getLocalizedFactionName( factionName ));
				sendSystemMessageProse(target, pp);
			}
		}
		
		return true;
	}
	
	
	public static boolean addUnmodifiedFactionStanding(obj_id target, String factionName, float value) throws InterruptedException
	{
		return addUnmodifiedFactionStanding(target, factionName, value, true);
	}
	
	
	public static boolean grantFactionBonus(obj_id target, String factionName, float value) throws InterruptedException
	{
		if (!isIdValid(target) || factionName == null || factionName.equals("") || value == 0.0f)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static float getFactionStoryReward(obj_id target, String factionName, float value) throws InterruptedException
	{
		float bonus = 0;
		int myFac = pvpGetAlignedFaction(target);
		
		if (value > 0 && myFac == (370444368) && getFactionNameByHashCode(myFac).equals(factionName))
		{
			if (pvpGetType( target ) != PVPTYPE_NEUTRAL)
			{
				bonus = (value * 0.10f);
			}
		}
		
		return bonus;
	}
	
	
	public static void setFactionStanding(obj_id target, String factionName, float value) throws InterruptedException
	{
		if (factionName == null || factionName.equals(""))
		{
			return;
		}
		
		if (!isPlayer( target ))
		{
			setFaction( target, factionName );
			return;
		}
		
		int factionNum = getFactionNumber(factionName);
		if (factionNum == AD_HOC_FACTION || factionNum < 0)
		{
			return;
		}
		
		int playerAllowed = dataTableGetInt(FACTION_TABLE, factionNum, "playerAllowed");
		if (playerAllowed != 1)
		{
			return;
		}
		
		float factionMax = getFactionMax( target, factionName );
		if (value < FACTION_RATING_MIN)
		{
			value = FACTION_RATING_MIN;
		}
		else if (value >= factionMax)
		{
			value = factionMax;
		}
		
		if (value != 0.0f)
		{
			if ((utils.getPlayerProfession(target) == utils.SMUGGLER) && (factionName.equals("underworld")))
			{
				smuggler.checkSmugglerTitleGrants(target, value);
				smuggler.checkRewardQuestGrants(target, value);
				
				messageTo(target, "applySmugglingBonuses", null, 1.0f, false);
			}
			
			if ((utils.getPlayerProfession(target) == utils.BOUNTY_HUNTER) && (factionName.equals("underworld")))
			{
				smuggler.checkBountyTitleGrants(target, value);
			}
			
			setObjVar(target, FACTION + "."+ factionName, value );
		}
		else
		{
			removeObjVar(target, FACTION + "."+ factionName );
			messageTo(target, "removeSmugglingBonuses", null, 1.0f, false);
		}
		
		armor.recalculateArmorForPlayer(target);
		obj_id tempWeapon = getObjectInSlot(target, "hold_r");
		
		if (isIdValid(tempWeapon))
		{
			weapons.adjustWeaponRangeForExpertise(target, tempWeapon, true);
		}
		
	}
	
	
	public static void setFaction(obj_id target, String factionName) throws InterruptedException
	{
		if (factionName == null || factionName.equals(""))
		{
			return;
		}
		
		setFaction( target, factionName, true );
	}
	
	
	public static void setFaction(obj_id target, String factionName, int pvpOnly) throws InterruptedException
	{
		setFaction(target, factionName, true, pvpOnly);
	}
	
	
	public static void setFaction(obj_id target, String factionName, boolean isAggro) throws InterruptedException
	{
		setFaction(target, factionName, isAggro, 0);
	}
	
	
	public static void setFaction(obj_id target, String factionName, boolean isAggro, int pvpOnly) throws InterruptedException
	{
		if (factionName == null || factionName.equals(""))
		{
			return;
		}
		
		if (isPlayer( target ))
		{
			return;
		}
		
		boolean pvp_only = pvpOnly == 1 ? true : false;
		
		if (factionName.equals( FACTION_IMPERIAL ))
		{
			setAttributeAttained( target, attrib.IMPERIAL );
			setAttributeInterested( target, attrib.REBEL );
			ai_lib.setDefaultCalmMood( target, "npc_imperial");
			if (pvp_only)
			{
				setObjVar(target, "huysMagicSpecialForcesOnlyObjvar", true);
			}
		}
		else if (factionName.equals( FACTION_REBEL ))
		{
			setAttributeAttained( target, attrib.REBEL );
			setAttributeInterested( target, attrib.IMPERIAL );
			if (pvp_only)
			{
				setObjVar(target, "huysMagicSpecialForcesOnlyObjvar", true);
			}
		}
		
		setObjVar(target, FACTION, factionName);
		pvpMakeNeutral(target);
		removeObjVar( target, "ai.faction.nonAggro");
		
		int factionNum = getFactionNumber(factionName);
		if (factionNum == AD_HOC_FACTION)
		{
			if (!isAggro)
			{
				LOG("gcw", "1");
				pvpSetAlignedFaction(target, (221551254));
				pvpMakeDeclared(target);
				setObjVar( target, "ai.faction.nonAggro", true );
			}
			LOG("gcw", "2");
			return;
		}
		LOG("gcw", "10");
		
		int factionHashCode = dataTableGetInt(FACTION_TABLE, factionNum, "pvpFaction");
		if (factionHashCode == 0)
		{
			return;
		}
		LOG("gcw", "setting aligned faction to "+factionHashCode);
		LOG("gcw", "making "+target+" covert");
		pvpSetAlignedFaction(target, factionHashCode);
		pvpMakeDeclared(target);
	}
	
	
	public static boolean isNeutral(obj_id target) throws InterruptedException
	{
		
		int intPlayerFaction = pvpGetAlignedFaction(target);
		if (intPlayerFaction == 0)
		{
			return true;
		}
		if (pvpGetType(target)==PVPTYPE_NEUTRAL)
		{
			return true;
		}
		return false;
	}
	
	
	public static String getFactionNameByHashCode(int hashCode) throws InterruptedException
	{
		
		if (hashCode == 0)
		{
			return null;
		}
		
		int[] columns = dataTableGetIntColumn(FACTION_TABLE, "pvpFaction");
		for (int i = 0; i < columns.length; i++)
		{
			testAbortScript();
			if (hashCode == columns[i])
			{
				return dataTableGetString(FACTION_TABLE, i, "factionName");
			}
		}
		return null;
	}
	
	
	public static int getFactionNumber(String factionName) throws InterruptedException
	{
		if (factionName == null)
		{
			return AD_HOC_FACTION;
		}
		return dataTableSearchColumnForString(factionName, "factionName", FACTION_TABLE);
	}
	
	
	public static int getFactionStatus(obj_id ai, obj_id target) throws InterruptedException
	{
		int result = STATUS_NEUTRAL;
		final int factionReaction = getFactionReaction(ai, target);
		
		if (factionReaction < REACTION_NEUTRAL)
		{
			result = STATUS_ENEMY;
		}
		else if (factionReaction > REACTION_NEUTRAL)
		{
			result = STATUS_FRIEND;
		}
		
		return result;
	}
	
	
	public static int getFactionReaction(obj_id npc, obj_id target) throws InterruptedException
	{
		PROFILER_START("library.faction.getFactionReaction");
		
		int result = REACTION_NEUTRAL;
		
		String npcFaction = getFaction(npc);
		if (npcFaction != null)
		{
			
			if (isPlayer(target))
			{
				if (!isNeutral(npc) && (!isNeutral(target)))
				{
					
					String yourFaction = getDeclaredFaction( target );
					String myFaction = getDeclaredFaction( npc );
					if (yourFaction == null)
					{
						LOG("DESIGNER_FATAL", "In faction.getFactionReaction, getDeclaredFaction returned null for yourFaction. obj_id, "+target );
						PROFILER_STOP("library.faction.getFactionReaction");
						return REACTION_NEUTRAL;
					}
					
					if (myFaction == null)
					{
						LOG("DESIGNER_FATAL", "In faction.getFactionReaction, getDeclaredFaction returned null for myFaction. obj_id, "+npc );
						PROFILER_STOP("library.faction.getFactionReaction");
						return REACTION_NEUTRAL;
					}
					
					if (yourFaction.equals( myFaction ))
					{
						PROFILER_STOP("library.faction.getFactionReaction");
						return REACTION_POSITIVE;
					}
					else
					{
						PROFILER_STOP("library.faction.getFactionReaction");
						return REACTION_NEGATIVE;
					}
				}
				
				int npcFactionNum = getFactionNumber(npcFaction);
				if (npcFactionNum != AD_HOC_FACTION)
				{
					boolean playerAllowed = (dataTableGetInt(FACTION_TABLE, npcFactionNum, "playerAllowed") == 1);
					if (playerAllowed)
					{
						float playerFaction = getFactionStanding(target, npcFaction);
						if (playerFaction > (FACTION_RATING_MAX * .75f))
						{
							result = REACTION_POSITIVE;
						}
						else if (playerFaction > (FACTION_RATING_MAX * .5f))
						{
							result = REACTION_LIKE;
						}
						else if (playerFaction < (FACTION_RATING_MIN * .75f))
						{
							result = REACTION_NEGATIVE;
						}
						else if (playerFaction < (FACTION_RATING_MIN * .5f))
						{
							result = REACTION_DISLIKE;
						}
					}
				}
			}
			else
			{
				String targetFaction = getFaction( target );
				if (targetFaction != null)
				{
					
					int npcFactionNum = getFactionNumber(npcFaction);
					if (npcFactionNum == AD_HOC_FACTION)
					{
						if (npcFaction.equals(targetFaction) && !hasObjVar(npc, "ai.faction.nonAggro"))
						{
							result = REACTION_POSITIVE;
						}
					}
					else
					{
						int targetFactionNum = getFactionNumber(targetFaction);
						if (targetFactionNum == AD_HOC_FACTION)
						{
							result = REACTION_NEUTRAL;
						}
						else
						{
							String enemies = dataTableGetString( FACTION_TABLE, npcFaction, "enemies");
							if (enemies == null)
							{
								result = REACTION_NEUTRAL;
							}
							else
							{
								String[] enemiesList = split( enemies, ',');
								if (utils.getElementPositionInArray(enemiesList, targetFaction )!=-1)
								{
									result = REACTION_NEGATIVE;
								}
								else
								{
									result = REACTION_NEUTRAL;
								}
							}
						}
					}
				}
			}
		}
		
		PROFILER_STOP("library.faction.getFactionReaction");
		return result;
	}
	
	
	public static String getDeclaredFaction(obj_id target) throws InterruptedException
	{
		if (!isDeclared( target))
		{
			return null;
		}
		
		if (!isPlayer(target))
		{
			return getStringObjVar( target, FACTION );
		}
		
		int factionHashCode = pvpGetAlignedFaction(target);
		return getFactionNameByHashCode( factionHashCode );
	}
	
	
	public static boolean isAggro(obj_id npc) throws InterruptedException
	{
		String factionName = getFaction( npc );
		if (factionName == null)
		{
			return false;
		}
		
		int factionNum = getFactionNumber( factionName );
		if (factionNum == AD_HOC_FACTION)
		{
			return (!hasObjVar( npc, "ai.faction.nonAggro" ));
		}
		
		return (dataTableGetInt(FACTION_TABLE, factionNum, "isAggro") == 1);
	}
	
	
	public static void clearFaction(obj_id npc) throws InterruptedException
	{
		if (isPlayer(npc))
		{
			return;
		}
		
		removeObjVar( npc, FACTION );
		pvpMakeNeutral(npc);
	}
	
	
	public static boolean awardFactionStanding(obj_id player, String factionName, int value) throws InterruptedException
	{
		if ((player == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		if (value > 0)
		{
			if (luck.isLucky(player, 0.01f))
			{
				float bonus = value * 0.2f;
				if (bonus < 1)
				{
					bonus = 1;
				}
				
				value += bonus;
			}
		}
		
		if (addFactionStanding(player, factionName, value))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static void grantCombatFaction(obj_id killer, obj_id target, double percentDamage) throws InterruptedException
	{
		
		if (!isIdValid(target) || !isIdValid(killer) || percentDamage < 0.f || percentDamage > 1.f)
		{
			return;
		}
		
		if (target == killer)
		{
			return;
		}
		
		if (!isPlayer( killer ))
		{
			return;
		}
		
		String killerFaction = getFaction( killer );
		String targetFaction = getFaction( target );
		
		if ((killerFaction == null) || (targetFaction == null))
		{
			return;
		}
		
		float killerStanding = getFactionStanding( killer, killerFaction );
		float targetStanding = getFactionStanding( target, targetFaction );
		
		if (isPlayer( killer) && pvpGetType( killer ) == PVPTYPE_NEUTRAL)
		{
			return;
		}
		
		if (isPlayer( target ) && pvpGetType( target ) == PVPTYPE_NEUTRAL)
		{
			return;
		}
		
		if (!pvpAreFactionsOpposed(pvpGetAlignedFaction(killer), pvpGetAlignedFaction(target)))
		{
			return;
		}
		
		int killerDiff = getLevel( killer );
		int targetDiff = getLevel( target );
		
		gcw.incrementGCWStanding(killer, target);
		
		if (isPlayer( target ))
		{
			
			if (targetStanding <= 0)
			{
				return;
			}
			
			int kRank = pvpGetCurrentGcwRank(killer);
			int tRank = pvpGetCurrentGcwRank(target);
			
			float diffFactor = 1f + (tRank - kRank)/(2*MAXIMUM_RANK);
			
			int actualReward = (int) (MAX_FACTION_KILL_REWARD * percentDamage * diffFactor);
			int factionLoss = (int) (-1.5f * actualReward);
			
			if (hasSkill( target, "class_smuggler_phase2_novice" ))
			{
				actualReward /= 2;
				factionLoss /= 2;
			}
			
			if (pvpGetType(killer)==PVPTYPE_DECLARED)
			{
				actualReward = actualReward * 2;
			}
			awardFactionStanding( killer, killerFaction, actualReward );
			addFactionStanding( killer, targetFaction, factionLoss );
			
			addFactionStanding( target, targetFaction, factionLoss );
			
		}
		else
		{
			
			if (targetDiff > 25)
			{
				targetDiff = 25;
			}
			
			if (killerDiff > 25)
			{
				killerDiff = 25;
			}
			
			int diffDelta = (targetDiff - killerDiff) + 25;
			float diffFactor = diffDelta / 50.f;
			
			int actualReward = (int) (MAX_FACTION_KILL_REWARD * percentDamage * diffFactor);
			int factionLoss = (int) (-1.5f * actualReward);
			
			awardFactionStanding( killer, killerFaction, actualReward );
			addFactionStanding( killer, targetFaction, factionLoss );
			
		}
		
	}
	
	
	public static void adjustSocialStanding(obj_id target, String faction, int amount) throws InterruptedException
	{
		if (!isPlayer( target ))
		{
			return;
		}
		
		if (faction == null || faction.equals(""))
		{
			return;
		}
		
		if (!isIdValid(target))
		{
			return;
		}
		
		if (amount == 0)
		{
			return;
		}
		
		if (faction.equals(FACTION_IMPERIAL) || faction.equals(FACTION_REBEL))
		{
			return;
		}
		
		int factionNum = getFactionNumber(faction);
		float combatFactor = 1.0f;
		if (factionNum != AD_HOC_FACTION)
		{
			combatFactor = dataTableGetFloat(FACTION_TABLE, factionNum, "combatFactor");
		}
		amount *= combatFactor;
		
		addFactionStanding( target, faction, amount );
		
		String allies = dataTableGetString( FACTION_TABLE, faction, "allies");
		if (allies != null)
		{
			String[] alliesList = split( allies, ',');
			for (int i = 0; i < alliesList.length; i++)
			{
				testAbortScript();
				
				if (!alliesList[i].equals(FACTION_IMPERIAL) && !alliesList[i].equals(FACTION_REBEL))
				{
					addFactionStanding( target, alliesList[i], amount );
				}
			}
		}
		
		amount = amount/2;
		if (amount >= -1 && amount <= 1)
		{
			return;
		}
		
		String enemies = dataTableGetString( FACTION_TABLE, faction, "enemies");
		if (enemies == null)
		{
			return;
		}
		String[] enemiesList = split( enemies, ',');
		for (int i = 0; i < enemiesList.length; i++)
		{
			testAbortScript();
			
			if (!enemiesList[i].equals(FACTION_IMPERIAL) && !enemiesList[i].equals(FACTION_REBEL))
			{
				addFactionStanding( target, enemiesList[i], -amount );
			}
		}
	}
	
	
	public static string_id getLocalizedFactionName(String factionName) throws InterruptedException
	{
		return ( new string_id( "faction/faction_names", toLower(factionName) ) );
	}
	
	
	public static boolean setTemporaryEnemyFlag(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target))
		{
			return false;
		}
		
		String sTargetFaction = getFactionNameByHashCode(pvpGetAlignedFaction(target));
		if ((sTargetFaction == null) || (sTargetFaction.equals("")))
		{
			return false;
		}
		
		if (!pvpIsEnemy(target, player))
		{
			if (pvpCanAttack(target, player))
			{
				pvpAttackPerformed(player, target);
				
				string_id sidFaction = getLocalizedFactionName(sTargetFaction);
				prose_package ppTEF = prose.getPackage(PROSE_TEF, sidFaction);
				sendSystemMessageProse(player, ppTEF);
			}
		}
		
		return true;
	}
	
	
	public static boolean areFactionsOpposed(String faction1, String faction2) throws InterruptedException
	{
		int facNum1 = getFactionNumber(faction1);
		int facNum2 = getFactionNumber(faction2);
		if (facNum1 == AD_HOC_FACTION || facNum2 == AD_HOC_FACTION)
		{
			return false;
		}
		
		int hc1 = dataTableGetInt(FACTION_TABLE, facNum1, "pvpFaction");
		int hc2 = dataTableGetInt(FACTION_TABLE, facNum2, "pvpFaction");
		if (hc1 == 0 || hc2 == 0)
		{
			return false;
		}
		
		return pvpAreFactionsOpposed(hc1, hc2);
	}
	
	
	public static obj_id getFactionParentObject() throws InterruptedException
	{
		obj_id objParent = getPlanetByName("tatooine");
		return objParent;
	}
	
	
	public static void changeFactionPoints(String strFaction, int intPoints) throws InterruptedException
	{
		obj_id objParent = getFactionParentObject();
		int intOldPoints = getIntObjVar(objParent, "gcw."+strFaction);
		intPoints = intPoints + intOldPoints;
		setObjVar(objParent, "gcw."+strFaction, intPoints);
	}
	
	
	public static int getFactionPointStanding(String strFaction) throws InterruptedException
	{
		obj_id objParent = getFactionParentObject();
		int intPoints = getIntObjVar(objParent, strFaction);
		return intPoints;
	}
	
	
	public static void shiftPointsTo(String strFaction, int intPoints) throws InterruptedException
	{
		String strRebel = FACTION_REBEL;
		String strImperial = FACTION_IMPERIAL;
		if (strFaction.equals(strRebel))
		{
			changeFactionPoints(factions.FACTION_REBEL, intPoints);
			changeFactionPoints(factions.FACTION_IMPERIAL, intPoints*-1);
			
		}
		if (strFaction.equals(strImperial))
		{
			changeFactionPoints(factions.FACTION_REBEL, intPoints);
			changeFactionPoints(factions.FACTION_IMPERIAL, intPoints*-1);
		}
		return;
		
	}
	
	
	public static boolean isFactionWinning(String strFaction) throws InterruptedException
	{
		obj_id objParent = getFactionParentObject();
		
		String strRebel = FACTION_REBEL;
		String strImperial = FACTION_IMPERIAL;
		int intRebel = getIntObjVar(objParent, "gcw."+strRebel);
		int intImperial = getIntObjVar(objParent, "gcw."+strImperial);
		if (strFaction.equals(strRebel))
		{
			if (intRebel > intImperial)
			{
				return true;
				
			}
			return false;
			
		}
		if (strFaction.equals(strImperial))
		{
			if (intImperial > intRebel)
			{
				return true;
				
			}
			return false;
			
		}
		return false;
		
	}
	
	
	public static boolean setRank(obj_id player, int rank) throws InterruptedException
	{
		return false;
	}
	
	
	public static boolean releaseFactionHirelings(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		obj_id hireling = callable.getCallable(player, callable.CALLABLE_TYPE_COMBAT_OTHER);
		
		if (hasObjVar(hireling, VAR_FACTION_HIRELING))
		{
			pet_lib.releasePet(hireling);
			
			prose_package pp = prose.getPackage(SID_HIRELING_RELEASED, player, hireling);
			sendSystemMessageProse(player, pp);
			
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean isNewlyDeclared(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, VAR_NEWLY_DECLARED))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean canUseFactionItem(obj_id player, obj_id item) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(item))
		{
			CustomerServiceLog("Faction", "WARNING: factions.canUseFactionItem called with invalid player (%TU) or item ("+ item + ")", player);
			return false;
		}
		
		String faction;
		if (hasObjVar(item, VAR_FACTION))
		{
			faction = getStringObjVar(item, VAR_FACTION);
		}
		else
		{
			CustomerServiceLog("Faction", "WARNING: factions::canUseFactionItem -- "+ item + " is a faction item with no faction objvar.");
			return false;
		}
		
		int player_faction_id = pvpGetAlignedFaction(player);
		if (player_faction_id == 0 || pvpGetType(player) == PVPTYPE_NEUTRAL)
		{
			prose_package pp = prose.getPackage(SID_MUST_BE_FACTION_MEMBER, faction);
			sendSystemMessageProse(player, pp);
			return false;
		}
		
		String player_faction = factions.getFactionNameByHashCode(player_faction_id);
		if (player_faction == null)
		{
			return false;
		}
		if (!player_faction.equals(faction))
		{
			prose_package pp = prose.getPackage(SID_MUST_BE_FACTION_MEMBER, faction);
			sendSystemMessageProse(player, pp);
			return false;
		}
		return true;
	}
	
	
	public static void awardKashyyykFaction(obj_id player, int ammount) throws InterruptedException
	{
		if (!isPlayer(player))
		{
			return;
		}
		
		if (ammount == 0)
		{
			return;
		}
		
		validateBalance(player, KASHYYYK);
		
		float hsskorAmmount = getFactionStanding(player, HSSKOR);
		float kashyyykAmmount = getFactionStanding(player, KASHYYYK);
		
		if (kashyyykAmmount < 0)
		{
			return;
		}
		
		float gain = kashyyykAmmount + (float)ammount;
		float lose = hsskorAmmount - (float)ammount;
		
		setObjVar(player, FACTION + "."+ KASHYYYK, gain );
		setObjVar(player, FACTION + "."+ HSSKOR, lose);
		
		prose_package pp = prose.getPackage(PROSE_AWARD_FACTION, getLocalizedFactionName( KASHYYYK ), (int)ammount);
		sendSystemMessageProse(player, pp);
		
		ammount *= -1;
		pp = prose.getPackage( PROSE_LOSE_FACTION, getLocalizedFactionName( HSSKOR ), (int)ammount );
		sendSystemMessageProse(player, pp );
		
		return;
	}
	
	
	public static void awardHsskorFaction(obj_id player, int ammount) throws InterruptedException
	{
		if (!isPlayer(player))
		{
			return;
		}
		
		if (ammount == 0)
		{
			return;
		}
		
		validateBalance(player, HSSKOR);
		
		float hsskorAmmount = getFactionStanding(player, HSSKOR);
		float kashyyykAmmount = getFactionStanding(player, KASHYYYK);
		
		if (hsskorAmmount < 0)
		{
			return;
		}
		
		float gain = hsskorAmmount + (float)ammount;
		float lose = kashyyykAmmount - (float)ammount;
		
		setObjVar(player, FACTION + "."+ HSSKOR, gain );
		setObjVar(player, FACTION + "."+ KASHYYYK, lose);
		
		prose_package pp = prose.getPackage(PROSE_AWARD_FACTION, getLocalizedFactionName( HSSKOR ), (int)ammount);
		sendSystemMessageProse(player, pp);
		
		ammount *= -1;
		pp = prose.getPackage( PROSE_LOSE_FACTION, getLocalizedFactionName( KASHYYYK ), (int)ammount );
		sendSystemMessageProse(player, pp );
		
		return;
	}
	
	
	public static void validateBalance(obj_id player, String faction) throws InterruptedException
	{
		String checkHsskor = FACTION + "."+ HSSKOR;
		String checkKashyyyk = FACTION + "."+ KASHYYYK;
		float hsskorBalance = 0f;
		float kashyyykBalance = 0f;
		
		if (hasObjVar(player, checkHsskor))
		{
			if (!hasObjVar(player, checkKashyyyk))
			{
				setObjVar(player, checkKashyyyk, -1*getFactionStanding(player, HSSKOR));
			}
		}
		
		if (hasObjVar(player, checkKashyyyk))
		{
			if (!hasObjVar(player, checkHsskor))
			{
				setObjVar(player, checkHsskor, -1*getFactionStanding(player, KASHYYYK));
			}
		}
		
		hsskorBalance = getFloatObjVar(player, checkHsskor);
		kashyyykBalance = getFloatObjVar(player, checkKashyyyk);
		
		if (hsskorBalance != -1*kashyyykBalance)
		{
			if (faction.equals(HSSKOR))
			{
				setObjVar(player, checkKashyyyk, -1*getFactionStanding(player, HSSKOR));
			}
			else
			{
				setObjVar(player, checkHsskor, -1*getFactionStanding(player, KASHYYYK));
			}
		}
		
		return;
	}
	
	
	public static boolean isUnaligned(obj_id player) throws InterruptedException
	{
		String hsskor = FACTION + "."+ HSSKOR;
		String kashyyyk = FACTION + "."+ KASHYYYK;
		
		validateBalance(player, HSSKOR);
		
		if (!hasObjVar(player, hsskor ) && !hasObjVar(player, kashyyyk))
		{
			return true;
		}
		
		if (getFloatObjVar(player, hsskor) != 0)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean isAlignedKashyyyk(obj_id player) throws InterruptedException
	{
		if (isUnaligned(player))
		{
			return false;
		}
		
		return (getFactionStanding(player, KASHYYYK) > 0);
	}
	
	
	public static boolean isAlignedHsskor(obj_id player) throws InterruptedException
	{
		if (isUnaligned(player))
		{
			return false;
		}
		
		return (getFactionStanding(player, HSSKOR) > 0);
	}
	
	
	public static boolean isImperial(obj_id objPlayer) throws InterruptedException
	{
		int intPlayerFaction = pvpGetAlignedFaction(objPlayer);
		if (intPlayerFaction==(-615855020))
		{
			return true;
		}
		return false;
		
	}
	
	public static boolean isRebel(obj_id objPlayer) throws InterruptedException
	{
		int intPlayerFaction = pvpGetAlignedFaction(objPlayer);
		if (intPlayerFaction==(370444368))
		{
			return true;
		}
		return false;
	}
	
	public static boolean isOnLeave(obj_id objPlayer) throws InterruptedException
	{
		int intFaction = pvpGetAlignedFaction(objPlayer);
		if ((intFaction != 0) && pvpGetType(objPlayer) == PVPTYPE_NEUTRAL)
		{
			return true;
		}
		return false;
	}
	
	public static boolean isActiveImperial(obj_id objPlayer) throws InterruptedException
	{
		return (isImperial(objPlayer) && !isOnLeave(objPlayer));
	}
	
	public static boolean isActiveRebel(obj_id objPlayer) throws InterruptedException
	{
		return (isRebel(objPlayer) && !isOnLeave(objPlayer));
	}
	
	
	public static boolean isGatedByJoinTimer(obj_id player, String factionJoining) throws InterruptedException
	{
		factionJoining = toLower(factionJoining);
		
		if (!factionJoining.equals("imperial") && !factionJoining.equals("rebel"))
		{
			return false;
		}
		
		if (hasObjVar(player, "lastFactionResignedFrom") && hasObjVar(player, "lastFactionResignedTime"))
		{
			
			String lastFactionResignedFrom = getStringObjVar(player, "lastFactionResignedFrom");
			
			if (!toLower(lastFactionResignedFrom).equals(factionJoining))
			{
				
				int now = getGameTime();
				int lastFactionResignedTime = getIntObjVar(player, "lastFactionResignedTime");
				int interval = now - lastFactionResignedTime;
				
				int minIntervalToJoin = 60 * 60 * 24 * 5;
				
				if (interval < minIntervalToJoin)
				{
					return true;
				}
			}
		}
		
		return false;
		
	}
	
	
	public static boolean canJoinFaction(obj_id player, int faction_id) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::canJoinFaction -- player is null");
			return false;
		}
		if (faction_id == 0)
		{
			LOG("LOG_CHANNEL", "faction_recruiter::canJoinFaction -- faction is 0");
			return false;
		}
		String faction = factions.getFactionNameByHashCode(faction_id);
		if (faction == null)
		{
			LOG("LOG_CHANNEL", "faction_recruiter::canJoinFaction -- can not find "+ faction_id + " in the faction datatable");
			return false;
		}
		
		if (isGatedByJoinTimer(player, faction))
		{
			return false;
		}
		
		int player_faction_id = pvpGetAlignedFaction(player);
		if (player_faction_id != 0)
		{
			
			if (player_faction_id == faction_id && pvpGetType(player) == PVPTYPE_NEUTRAL)
			{
				player_faction_id = 0;
			}
			else
			{
				return false;
			}
		}
		
		return true;
	}
	
	public static boolean isFactionMember(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::isFactionMember -- player is null");
			return false;
		}
		if (npc == null || npc == obj_id.NULL_ID)
		{
			LOG("LOG_CHANNEL", "faction_recruiter::isFactionMember -- npc is null");
			return false;
		}
		
		int player_faction_id = pvpGetAlignedFaction(player);
		int npc_faction_id = pvpGetAlignedFaction(npc);
		if (player_faction_id == 0)
		{
			return false;
		}
		
		if (player_faction_id == npc_faction_id)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	
	public static boolean joinFaction(obj_id player, int faction_id, boolean returnFromReserves) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::joinFaction -- player is null");
			return false;
		}
		
		if (faction_id == 0)
		{
			LOG("LOG_CHANNEL", "faction_recruiter::joinFaction -- faction is 0");
			return false;
		}
		int intFaction = pvpGetAlignedFaction(player);
		if (intFaction != 0)
		{
			
			return false;
		}
		CustomerServiceLog("factions", player+" named "+getName(player)+" joined "+getFactionNameByHashCode(faction_id));
		if (!canJoinFaction(player, faction_id))
		{
			return false;
		}
		
		buff.removeAllAuraBuffs(player);
		
		pvpSetAlignedFaction(player, faction_id);
		pvpMakeCovert(player);
		LOG("LOG_CHANNEL", "faction_recruiter::joinFaction -- "+ player + " has joined faction "+ faction_id);
		return true;
	}
	
	
	public static boolean joinFaction(obj_id player, int faction_id) throws InterruptedException
	{
		return joinFaction(player, faction_id, false);
	}
	
	
	public static boolean isOnLeaveFromFriendlyFaction(obj_id objPlayer, obj_id objNPC) throws InterruptedException
	{
		if (pvpGetAlignedFaction(objPlayer)== pvpGetAlignedFaction(objNPC)&&(pvpGetType(objPlayer)==PVPTYPE_NEUTRAL))
		{
			return true;
		}
		return false;
	}
	
	public static boolean isInEnemyFaction(obj_id objPlayer, obj_id objNPC) throws InterruptedException
	{
		int intFaction = pvpGetAlignedFaction(objPlayer);
		if ((intFaction != 0)&&(intFaction!= pvpGetAlignedFaction(objNPC)))
		{
			return true;
		}
		return false;
	}
	
	public static boolean isInFriendlyFaction(obj_id objPlayer, obj_id objNPC) throws InterruptedException
	{
		if (pvpGetAlignedFaction(objPlayer)== pvpGetAlignedFaction(objNPC))
		{
			return true;
		}
		return false;
	}
	
	public static boolean isOnLeaveFromEnemyFaction(obj_id objPlayer, obj_id objNPC) throws InterruptedException
	{
		int intFaction = pvpGetAlignedFaction(objPlayer);
		if ((intFaction != 0)&&(intFaction!= pvpGetAlignedFaction(objNPC)&&(pvpGetType(objPlayer)==PVPTYPE_NEUTRAL)))
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean qualifiesForPromotion(obj_id objPlayer, int intFaction) throws InterruptedException
	{
		
		int intPlayerFaction = pvpGetAlignedFaction(objPlayer);
		if (intPlayerFaction != intFaction)
		{
			return false;
		}
		
		int current_rank = pvpGetCurrentGcwRank(objPlayer);
		if (current_rank >= factions.MAXIMUM_RANK)
		{
			return false;
		}
		int cost = factions.getRankCost(current_rank + 1);
		int faction_standing = (int)factions.getFactionStanding(objPlayer, intFaction);
		if (faction_standing >= cost)
		{
			return true;
		}
		return false;
	}
	
	
	public static void applyPromotion(obj_id objPlayer, int intFaction) throws InterruptedException
	{
		if (qualifiesForPromotion(objPlayer, intFaction))
		{
			int current_rank = pvpGetCurrentGcwRank(objPlayer);
			int cost = factions.getRankCost(current_rank + 1);
			int faction_standing = (int)factions.getFactionStanding(objPlayer, intFaction);
			if (factions.addFactionStanding(objPlayer, intFaction, -cost))
			{
				CustomerServiceLog( "player_faction", "PLAYER-FACTION ALTERED|TIME:"+getGameTime()+"|PLAYER:"+objPlayer+"|PLAYER NAME:"+getName(objPlayer)+"|ZONE:"+getCurrentSceneName()+"|Player has purchased rank for "+cost);
				factions.setRank(objPlayer, current_rank + 1);
			}
			
		}
		return;
	}
	
	
	public static boolean isSmuggler(obj_id player) throws InterruptedException
	{
		if (hasSkill( player, "class_smuggler_phase3_novice" ))
		{
			return true;
		}
		else if (hasSkill( player, "class_smuggler_phase4_novice" ))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean canBuyFaction(obj_id objPlayer, obj_id objNPC) throws InterruptedException
	{
		int intNPCFaction = pvpGetAlignedFaction(objNPC);
		
		String rec_faction = getFactionNameByHashCode(intNPCFaction);
		int intPlayerFaction = pvpGetAlignedFaction(objPlayer);
		
		String oppfaction = "";
		if (rec_faction.equals("Imperial"))
		{
			oppfaction = "Rebel";
		}
		else
		{
			oppfaction = "Imperial";
		}
		float oppstanding = factions.getFactionStanding( objPlayer, oppfaction );
		float ourstanding = factions.getFactionStanding( objPlayer, intPlayerFaction);
		
		float fltFactionMax = factions.getFactionMax(objPlayer, intNPCFaction);
		if (oppstanding > 200)
		{
			return false;
		}
		else if (ourstanding >= fltFactionMax)
		{
			return false;
		}
		return true;
	}
	
	
	public static void buyFaction(obj_id objPlayer, obj_id objNPC, int intAmount) throws InterruptedException
	{
		
		dictionary outp = new dictionary();
		money.requestPayment( objPlayer, objNPC, intAmount, "payBribe", outp, true );
		return;
	}
	
	
	public static boolean leaveFaction(obj_id player, int faction_id) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			LOG("LOG_CHANNEL", "faction_recruiter::joinFaction -- player is null");
			return false;
		}
		
		if (faction_id == 0)
		{
			LOG("LOG_CHANNEL", "faction_recruiter::joinFaction -- faction is 0");
			return false;
		}
		
		String title = utils.packStringId(SID_SUI_FACTION_RESIG_TITLE);
		String prompt = utils.packStringId(SID_SUI_FACTION_RESIG_TEXT);
		int pid = sui.inputbox(player, player, prompt, title, "handleLeaveFactionSui", "");
		return (pid > -1);
	}
	
	
	public static boolean isPVPStatusChanging(obj_id objPlayer) throws InterruptedException
	{
		if (hasObjVar(objPlayer, "intChangingFactionStatus"))
		{
			return true;
		}
		return false;
		
	}
	
	
	public static boolean pvpDoAllowedAttackCheck(obj_id objPlayer, obj_id objTarget) throws InterruptedException
	{
		if (!isPlayer(objPlayer))
		{
			LOG("combat", "return 1");
			return pvpCanAttack(objPlayer, objTarget);
		}
		if (!pvpCanAttack(objPlayer, objTarget))
		{
			if (hasObjVar(objPlayer, "intChangingFactionStatus"))
			{
				LOG("combat", "return 2");
				return false;
			}
			
			int intMyFaction = pvpGetAlignedFaction(objPlayer);
			int intYourFaction = pvpGetAlignedFaction(objTarget);
			if (intMyFaction == intYourFaction)
			{
				LOG("combat", "return 3");
				return false;
			}
			else
			{
				int intMyPVPType = pvpGetType(objPlayer);
				int intYourPVPType = pvpGetType(objTarget);
				if (!isPlayer(objTarget)&&(!pet_lib.isPet(objTarget)))
				{
					
					if (intMyPVPType == PVPTYPE_NEUTRAL)
					{
						
						if (intYourPVPType == PVPTYPE_NEUTRAL)
						{
							return false;
						}
						if (intYourPVPType == PVPTYPE_COVERT)
						{
							LOG("combat", "return 5");
							doTransitionSui(objPlayer, objTarget, 30f);
							return false;
							
						}
						if (intYourPVPType == PVPTYPE_DECLARED)
						{
							LOG("combat", "return 6");
							
							return false;
							
						}
					}
					else if (intMyPVPType == PVPTYPE_COVERT)
					{
						if (intYourPVPType == PVPTYPE_NEUTRAL)
						{
							LOG("combat", "return 7");
							return false;
						}
						if (intYourPVPType == PVPTYPE_COVERT)
						{
							LOG("combat", "return 8");
							return true;
						}
						if (intYourPVPType == PVPTYPE_DECLARED)
						{
							LOG("combat", "return 9");
							
							return false;
						}
					}
					else if (intMyPVPType == PVPTYPE_DECLARED)
					{
						if (intYourPVPType == PVPTYPE_NEUTRAL)
						{
							LOG("combat", "return 10");
							return false;
						}
						if (intYourPVPType == PVPTYPE_COVERT)
						{
							LOG("combat", "return 11");
							return true;
						}
						if (intYourPVPType == PVPTYPE_DECLARED)
						{
							LOG("combat", "return 12");
							return true;
						}
					}
					
				}
				LOG("combat", "return 13");
				return false;
				
			}
		}
		else
		{
			LOG("combat", "return 14");
			return true;
		}
		
	}
	
	
	public static void doTransitionSui(obj_id objPlayer, obj_id objTarget, float fltDelay) throws InterruptedException
	{
		
		if (hasObjVar(objPlayer, "intChangingFactionStatus"))
		{
			return;
		}
		int intFaction = pvpGetAlignedFaction(objPlayer);
		if (intFaction == 0)
		{
			return;
		}
		
		int intMyPVPType = pvpGetType(objPlayer);
		int intYourPVPType = pvpGetType(objTarget);
		string_id strTitleId = new string_id("gcw", "gcw_status_change");
		String strTitle = utils.packStringId(strTitleId);
		
		if (intMyPVPType == PVPTYPE_NEUTRAL)
		{
			if (intYourPVPType == PVPTYPE_COVERT)
			{
				
				string_id strSpam = new string_id("gcw", "gcw_status_change_covert");
				String strPrompt = utils.packStringId(strSpam);
				int pid = sui.inputbox(objPlayer, objPlayer, strPrompt, strTitle, "handleGoCovert", "");
				return;
			}
			else if (intYourPVPType == PVPTYPE_DECLARED)
			{
				string_id strSpam = new string_id("gcw", "gcw_status_change_overt");
				String strPrompt = utils.packStringId(strSpam);
				int pid = sui.inputbox(objPlayer, objPlayer, strPrompt, strTitle, "handleGoOvert", "");
				return;
				
			}
			else
			{
				sendSystemMessageTestingOnly(objPlayer, "Error # 4 : You got a PVP error message. Your PVP Type is "+intMyPVPType+". Your faction is"+pvpGetAlignedFaction(objPlayer)+". Your target is "+objTarget+" their pvp type is "+pvpGetType(objTarget)+" and their faction is "+pvpGetAlignedFaction(objTarget));
			}
			
		}
		else if (intMyPVPType == PVPTYPE_COVERT)
		{
			if (intYourPVPType == PVPTYPE_COVERT)
			{
				sendSystemMessageTestingOnly(objPlayer, "Error # 1 : You got a PVP error message. Your PVP Type is "+intMyPVPType+". Your faction is"+pvpGetAlignedFaction(objPlayer)+". Your target is "+objTarget+" their pvp type is "+pvpGetType(objTarget)+" and their faction is "+pvpGetAlignedFaction(objTarget));
			}
			else if (intYourPVPType == PVPTYPE_DECLARED)
			{
				
				string_id strSpam = new string_id("gcw", "gcw_status_change_overt");
				String strPrompt = utils.packStringId(strSpam);
				int pid = sui.inputbox(objPlayer, objPlayer, strPrompt, strTitle, "handleGoOvert", "");
				return;
			}
			else
			{
				sendSystemMessageTestingOnly(objPlayer, "Error # 3 : You got a PVP error message. Your PVP Type is "+intMyPVPType+". Your faction is"+pvpGetAlignedFaction(objPlayer)+". Your target is "+objTarget+" their pvp type is "+pvpGetType(objTarget)+" and their faction is "+pvpGetAlignedFaction(objTarget));
			}
			
		}
		else
		{
			
			sendSystemMessageTestingOnly(objPlayer, "Error # 2 : You got a PVP error message. Your PVP Type is "+intMyPVPType+". Your faction is"+pvpGetAlignedFaction(objPlayer)+". Your target is "+objTarget+" their pvp type is "+pvpGetType(objTarget)+" and their faction is "+pvpGetAlignedFaction(objTarget));
		}
		
	}
	
	
	public static boolean pvpDoAllowedHelpCheck(obj_id objPlayer, obj_id objTarget) throws InterruptedException
	{
		
		if (!isPlayer(objPlayer))
		{
			LOG("combat", "return 1");
			return pvpCanHelp(objPlayer, objTarget);
		}
		
		if (!pvpCanHelp(objPlayer, objTarget))
		{
			if (hasObjVar(objPlayer, "intChangingFactionStatus"))
			{
				LOG("combat", "return 2");
				return false;
			}
			
			int intMyFaction = pvpGetAlignedFaction(objPlayer);
			int intYourFaction = pvpGetAlignedFaction(objTarget);
			if (intMyFaction != intYourFaction)
			{
				LOG("combat", "return 3");
				return false;
			}
			else
			{
				int intMyPVPType = pvpGetType(objPlayer);
				int intYourPVPType = pvpGetType(objTarget);
				
				if (intMyPVPType == PVPTYPE_NEUTRAL)
				{
					if (intYourPVPType == PVPTYPE_COVERT)
					{
						LOG("combat", "return 5");
						doTransitionSui(objPlayer, objTarget, 30f);
						return false;
						
					}
				}
				
			}
			return false;
		}
		else
		{
			LOG("combat", "return 14");
			return true;
		}
		
	}
	
	
	public static boolean canGoOnLeave(obj_id objPlayer) throws InterruptedException
	{
		
		if (isInAdhocPvpArea(objPlayer))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean canGoCovert(obj_id objPlayer) throws InterruptedException
	{
		if (isInAdhocPvpArea(objPlayer))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static prose_package getFactionProsePackage(String factionName, int actualValue) throws InterruptedException
	{
		prose_package pp = null;
		
		if (actualValue > 0)
		{
			pp = prose.getPackage(PROSE_AWARD_FACTION, getLocalizedFactionName(factionName), actualValue);
		}
		else
		{
			pp = prose.getPackage(PROSE_LOSE_FACTION, getLocalizedFactionName(factionName), -actualValue);
		}
		
		return pp;
	}
	
	public static final String IGNORE_PLAYER = "ignorePlayer";
	
	public static boolean ignorePlayer(obj_id attacker, obj_id target) throws InterruptedException
	{
		if (isPlayer(attacker) || (isMob(attacker) && isIdValid(getMaster(attacker)) && isPlayer(getMaster(attacker))))
		{
			if (utils.hasScriptVar(target, IGNORE_PLAYER))
			{
				return true;
			}
		}
		else
		{
			if (isPlayer(target) || (isMob(target) && isIdValid(getMaster(target)) && isPlayer(getMaster(target))))
			{
				if (utils.hasScriptVar(attacker, IGNORE_PLAYER))
				{
					return true;
				}
			}
		}
		
		if (utils.hasScriptVar(target, IGNORE_PLAYER))
		{
			if (isPlayer(attacker) || (isIdValid(getMaster(attacker)) && isPlayer(getMaster(attacker))))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static void setIgnorePlayer(obj_id npc) throws InterruptedException
	{
		utils.setScriptVar(npc, IGNORE_PLAYER, true);
	}
	
	
	public static boolean isInAdhocPvpArea(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (utils.hasScriptVar(player, IN_ADHOC_PVP_AREA))
		{
			return utils.getBooleanScriptVar(player, IN_ADHOC_PVP_AREA);
		}
		
		return false;
	}
	
	
	public static void removeAllPvpSkills(obj_id player) throws InterruptedException
	{
		skill.revokeSkill (player, "pvp_imperial_retaliation_ability");
		skill.revokeSkill (player, "pvp_imperial_adrenaline_ability");
		skill.revokeSkill (player, "pvp_imperial_unstoppable_ability");
		skill.revokeSkill (player, "pvp_imperial_last_man_ability");
		skill.revokeSkill (player, "pvp_imperial_aura_buff_self");
		skill.revokeSkill (player, "pvp_imperial_airstrike_ability");
		skill.revokeSkill (player, "pvp_rebel_retaliation_ability");
		skill.revokeSkill (player, "pvp_rebel_adrenaline_ability");
		skill.revokeSkill (player, "pvp_rebel_unstoppable_ability");
		skill.revokeSkill (player, "pvp_rebel_last_man_ability");
		skill.revokeSkill (player, "pvp_rebel_aura_buff_self");
		skill.revokeSkill (player, "pvp_rebel_airstrike_ability");
		return;
	}
	
	
	public static boolean shareSocialGroup(obj_id creatureOne, obj_id creatureTwo) throws InterruptedException
	{
		String socialOne = getStringObjVar(creatureOne, "socialGroup");
		String socialTwo = getStringObjVar(creatureTwo, "socialGroup");
		
		if (socialOne.equals(socialTwo))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static boolean areCreaturesAllied(obj_id creatureOne, obj_id creatureTwo) throws InterruptedException
	{
		String factionOne = getStringObjVar(creatureOne, "faction");
		String factionTwo = getStringObjVar(creatureTwo, "faction");
		
		if (factionOne == null || factionOne.equals("") || factionTwo == null || factionTwo.equals(""))
		{
			return false;
		}
		
		if (factionOne.equals(factionTwo))
		{
			return true;
		}
		
		int row = dataTableSearchColumnForString(factionOne, "factionName", "datatables/faction/faction.iff");
		
		if (row == -1)
		{
			return false;
		}
		
		String allies = dataTableGetString("datatables/faction/faction.iff", row, "allies");
		
		if (allies == null || allies.equals(""))
		{
			return false;
		}
		
		String[] alliesParse = split(allies, ',');
		
		for (int i=0; i<alliesParse.length; i++)
		{
			testAbortScript();
			if (alliesParse[i].equals(factionTwo))
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean isPlayerSameGcwFactionAsSchedulerObject(obj_id player, obj_id object) throws InterruptedException
	{
		LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject init");
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (!isValidId(object) || !exists(object))
		{
			return false;
		}
		LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject basic validation completed");
		
		if (!utils.hasScriptVar(object, "faction"))
		{
			return false;
		}
		LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject has faction int script var");
		
		int intFaction = utils.getIntScriptVar(object, "faction");
		
		if (intFaction != FACTION_FLAG_IMPERIAL && intFaction != FACTION_FLAG_REBEL)
		{
			return false;
		}
		
		LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject faction int script var is a valid number");
		
		if (0 != pvpGetAlignedFaction(player))
		{
			LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject FACTIONED player");
			
			if (intFaction != factions.getFactionFlag(player))
			{
				return false;
			}
		}
		
		else
		{
			
			LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject NEUTRAL player");
			
			int currentMercenaryFaction = pvpNeutralGetMercenaryFaction(player);
			LOG("faction_check","isPlayerSameGcwFactionAsSchedulerObject currentMercenaryFaction: "+currentMercenaryFaction);
			if (0 == currentMercenaryFaction)
			{
				return false;
			}
			
			if ((-615855020) == currentMercenaryFaction && intFaction != FACTION_FLAG_IMPERIAL)
			{
				return false;
			}
			if ((370444368) == currentMercenaryFaction && intFaction != FACTION_FLAG_REBEL)
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	public static boolean canChangeNeutralMercenaryStatus(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (0 != pvpGetAlignedFaction(player))
		{
			return false;
		}
		
		if (0 != pvpNeutralGetMercenaryFaction(player))
		{
			return false;
		}
		
		if (hasMessageTo(player, "executeFactionalHelperChoice"))
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean neutralMercenaryStatusMenu(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		utils.removeScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES);
		
		if (utils.hasScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID))
		{
			int savedPageId = utils.getIntScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID);
			utils.removeScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID);
			
			forceCloseSUIPage(savedPageId);
		}
		
		if (0 != pvpGetAlignedFaction(player))
		{
			sendSystemMessage(player, "You must be a Civilian to be a factional helper.", "");
			return false;
		}
		
		if (hasMessageTo(player, "executeFactionalHelperChoice"))
		{
			sendSystemMessage(player, "You currently have an outstanding factional helper request.", "");
			return false;
		}
		
		int currentMercenaryFaction = pvpNeutralGetMercenaryFaction(player);
		if (0 != currentMercenaryFaction)
		{
			boolean isDeclared = pvpNeutralIsMercenaryDeclared(player);
			if ((-615855020) == currentMercenaryFaction)
			{
				Vector choicesDisplay = new Vector();
				Vector choicesInternal = new Vector();
				if (isDeclared)
				{
					String configSetting = getConfigSetting("GameServer", "enableCovertImperialMercenary");
					if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
					{
						choicesDisplay.addElement("I want to help the Imperials as a Combatant.");
						choicesInternal.addElement("covert_imperial_mercenary_begin");
					}
					
					choicesDisplay.addElement("I want to stop being a Special Forces Imperial helper.");
					choicesInternal.addElement("overt_imperial_mercenary_end");
				}
				else
				{
					choicesDisplay.addElement("I want to stop being a Combatant Imperial helper.");
					choicesInternal.addElement("covert_imperial_mercenary_end");
					
					String configSetting = getConfigSetting("GameServer", "enableOvertImperialMercenary");
					if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
					{
						choicesDisplay.addElement("I want to help the Imperials as a Special Forces.");
						choicesInternal.addElement("overt_imperial_mercenary_begin");
					}
				}
				
				int pid = sui.listbox(player, player, "What would you like to do?", sui.OK_CANCEL, "Factional Helper", choicesDisplay, "handleFactionalHelperChoice", true, false);
				utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES, choicesInternal);
				utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID, pid);
				
				return true;
			}
			else if ((370444368) == currentMercenaryFaction)
			{
				Vector choicesDisplay = new Vector();
				Vector choicesInternal = new Vector();
				if (isDeclared)
				{
					String configSetting = getConfigSetting("GameServer", "enableCovertRebelMercenary");
					if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
					{
						choicesDisplay.addElement("I want to help the Rebels as a Combatant.");
						choicesInternal.addElement("covert_rebel_mercenary_begin");
					}
					
					choicesDisplay.addElement("I want to stop being a Special Forces Rebel helper.");
					choicesInternal.addElement("overt_rebel_mercenary_end");
				}
				else
				{
					choicesDisplay.addElement("I want to stop being a Combatant Rebel helper.");
					choicesInternal.addElement("covert_rebel_mercenary_end");
					
					String configSetting = getConfigSetting("GameServer", "enableOvertRebelMercenary");
					if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
					{
						choicesDisplay.addElement("I want to help the Rebels as a Special Forces.");
						choicesInternal.addElement("overt_rebel_mercenary_begin");
					}
				}
				
				int pid = sui.listbox(player, player, "What would you like to do?", sui.OK_CANCEL, "Factional Helper", choicesDisplay, "handleFactionalHelperChoice", true, false);
				utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES, choicesInternal);
				utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID, pid);
				
				return true;
			}
		}
		
		final int timeNow = getCalendarTime();
		final int timeCooldown = (isGod(player) ? 10 : 1800);
		int secondsUntilCanHelpRebel = 0;
		int secondsUntilCanHelpImperial = 0;
		
		if (hasObjVar(player, "factionalHelper.timeStopHelpingRebel"))
		{
			int time = getIntObjVar(player, "factionalHelper.timeStopHelpingRebel");
			if ((time > 0) && ((time + timeCooldown) > timeNow))
			{
				secondsUntilCanHelpImperial = time + timeCooldown - timeNow;
			}
		}
		
		else if (hasObjVar(player, "factionalHelper.timeStopHelpingImperial"))
		{
			int time = getIntObjVar(player, "factionalHelper.timeStopHelpingImperial");
			if ((time > 0) && ((time + timeCooldown) > timeNow))
			{
				secondsUntilCanHelpRebel = time + timeCooldown - timeNow;
			}
		}
		
		Vector choicesDisplay = new Vector();
		Vector choicesInternal = new Vector();
		
		if (secondsUntilCanHelpImperial <= 0)
		{
			String configSetting = getConfigSetting("GameServer", "enableCovertImperialMercenary");
			if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
			{
				choicesDisplay.addElement("I want to help the Imperials as a Combatant.");
				choicesInternal.addElement("covert_imperial_mercenary_begin");
			}
			
			configSetting = getConfigSetting("GameServer", "enableOvertImperialMercenary");
			if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
			{
				choicesDisplay.addElement("I want to help the Imperials as a Special Forces.");
				choicesInternal.addElement("overt_imperial_mercenary_begin");
			}
		}
		
		if (secondsUntilCanHelpRebel <= 0)
		{
			String configSetting = getConfigSetting("GameServer", "enableCovertRebelMercenary");
			if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
			{
				choicesDisplay.addElement("I want to help the Rebels as a Combatant.");
				choicesInternal.addElement("covert_rebel_mercenary_begin");
			}
			
			configSetting = getConfigSetting("GameServer", "enableOvertRebelMercenary");
			if ((configSetting != null) && (configSetting.length() > 0) && !configSetting.toLowerCase().equals("false") && !configSetting.equals("0"))
			{
				choicesDisplay.addElement("I want to help the Rebels as a Special Forces.");
				choicesInternal.addElement("overt_rebel_mercenary_begin");
			}
		}
		
		if (choicesDisplay.size() <= 0)
		{
			sendSystemMessage(player, "The factional helper functionality is currently disabled.", "");
			return false;
		}
		
		String prompt = "What would you like to do?";
		if (secondsUntilCanHelpImperial > 0)
		{
			prompt += "\n\n";
			prompt += "(You must wait "+ (secondsUntilCanHelpImperial / 60) + "m:"+ (secondsUntilCanHelpImperial % 60) + "s before you can become an Imperial factional helper.)";
		}
		else if (secondsUntilCanHelpRebel > 0)
		{
			prompt += "\n\n";
			prompt += "(You must wait "+ (secondsUntilCanHelpRebel / 60) + "m:"+ (secondsUntilCanHelpRebel % 60) + "s before you can become a Rebel factional helper.)";
		}
		
		int pid = sui.listbox(player, player, prompt, sui.OK_CANCEL, "Factional Helper", choicesDisplay, "handleFactionalHelperChoice", true, false);
		utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_CHOICES, choicesInternal);
		utils.setScriptVar(player, SCRIPTVAR_FACTIONAL_HELPER_SUI_ID, pid);
		return true;
	}
	
	
	public static boolean setNeturalMercenaryCovert(obj_id player, int factionFlag) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (factionFlag < FACTION_FLAG_REBEL || factionFlag > FACTION_FLAG_IMPERIAL)
		{
			return false;
		}
		
		String covertType = "";
		
		if (factionFlag == FACTION_FLAG_REBEL)
		{
			sendSystemMessage(player, SID_MERC_REBEL_COMBATANT_BEGIN);
			covertType = MERC_BEGIN_COVERT_REBEL;
		}
		else if (factionFlag == FACTION_FLAG_IMPERIAL)
		{
			sendSystemMessage(player, SID_MERC_IMPERIAL_COMBATANT_BEGIN);
			covertType = MERC_BEGIN_COVERT_IMPERIAL;
		}
		
		if (covertType == null || covertType.length() <= 0)
		{
			return false;
		}
		
		dictionary messageToParams = new dictionary();
		messageToParams.put("mercenary_type", covertType);
		
		if (isGod(player))
		{
			sendSystemMessage(player, "Reducing wait to 5 seconds ***BECAUSE YOU ARE IN GOD MODE***.", "");
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 5.0f, false);
		}
		else
		{
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 60.0f, false);
		}
		
		return true;
	}
	
	
	public static boolean setNeturalMercenaryOvert(obj_id player, int factionFlag) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (factionFlag < FACTION_FLAG_REBEL || factionFlag > FACTION_FLAG_IMPERIAL)
		{
			return false;
		}
		
		String overtType = "";
		
		if (factionFlag == FACTION_FLAG_REBEL)
		{
			sendSystemMessage(player, SID_MERC_REBEL_SF_BEGIN);
			overtType = MERC_BEGIN_OVERT_REBEL;
		}
		else if (factionFlag == FACTION_FLAG_IMPERIAL)
		{
			sendSystemMessage(player, SID_MERC_IMPERIAL_SF_BEGIN);
			overtType = MERC_BEGIN_OVERT_IMPERIAL;
		}
		
		if (overtType == null || overtType.length() <= 0)
		{
			return false;
		}
		
		dictionary messageToParams = new dictionary();
		messageToParams.put("mercenary_type", overtType);
		
		if (isGod(player))
		{
			sendSystemMessage(player, "Reducing wait to 5 seconds ***BECAUSE YOU ARE IN GOD MODE***.", "");
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 5.0f, false);
		}
		else
		{
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 60.0f, false);
		}
		
		return true;
	}
	
	
	public static boolean removeNeturalMercenary(obj_id player, int factionFlag) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (factionFlag < FACTION_FLAG_REBEL || factionFlag > FACTION_FLAG_IMPERIAL)
		{
			return false;
		}
		
		boolean isDeclared = pvpNeutralIsMercenaryDeclared(player);
		String type = "";
		
		if (factionFlag == FACTION_FLAG_REBEL)
		{
			if (isDeclared)
			{
				sendSystemMessage(player, SID_MERC_REBEL_SF_END);
				type = MERC_END_OVERT_REBEL;
			}
			else
			{
				sendSystemMessage(player, SID_MERC_REBEL_COMBATANT_END);
				type = MERC_END_COVERT_REBEL;
			}
		}
		else if (factionFlag == FACTION_FLAG_IMPERIAL)
		{
			if (isDeclared)
			{
				sendSystemMessage(player, SID_MERC_IMPERIAL_SF_END);
				type = MERC_END_OVERT_IMPERIAL;
			}
			else
			{
				sendSystemMessage(player, SID_MERC_IMPERIAL_COMBATANT_END);
				type = MERC_END_COVERT_IMPERIAL;
			}
		}
		
		if (type == null || type.length() <= 0)
		{
			return false;
		}
		
		dictionary messageToParams = new dictionary();
		messageToParams.put("mercenary_type", type);
		
		if (isGod(player))
		{
			sendSystemMessage(player, "Reducing wait to 5 seconds ***BECAUSE YOU ARE IN GOD MODE***.", "");
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 5.0f, false);
		}
		else
		{
			messageTo(player, "executeFactionalHelperChoice", messageToParams, 60.0f, false);
		}
		
		return true;
	}
}
