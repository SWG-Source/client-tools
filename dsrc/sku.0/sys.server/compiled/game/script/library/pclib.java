package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.structure;
import script.library.sui;
import script.library.corpse;
import script.library.jedi;
import script.library.utils;
import script.library.prose;
import script.library.healing;
import script.library.battlefield;
import script.library.combat;
import script.library.factions;
import script.library.dot;
import script.library.group;
import script.library.pvp;
import script.library.ai_lib;
import script.library.innate;
import script.library.player_stomach;
import script.library.slots;
import script.library.money;
import script.library.player_structure;
import script.library.meditation;
import script.library.city;
import script.library.force_rank;
import script.library.missions;
import script.library.bounty_hunter;
import script.library.smuggler;
import script.library.buff;
import script.ai.ai_combat;
import java.util.Arrays;
import java.util.Vector;
import java.util.Enumeration;


public class pclib extends script.base_script
{
	public pclib()
	{
	}
	public static final int MAX_NEWBIE_DEATHS = 3;
	
	public static final String VAR_NEWBIE_BASE = "noob";
	
	public static final String VAR_NEWBIE_DEATH = VAR_NEWBIE_BASE + ".death";
	
	public static final String VAR_NEWBIE_CONFIRM_BASE = VAR_NEWBIE_BASE + ".confirm";
	public static final String VAR_NEWBIE_CONFIRM_INSURE = VAR_NEWBIE_CONFIRM_BASE + ".insure";
	public static final String VAR_NEWBIE_CONFIRM_BIND = VAR_NEWBIE_CONFIRM_BASE + ".bind";
	
	public static final string_id SID_NEWBIE_STATUS_EXPIRED = new string_id("base_player","newbie_expired");
	
	public static final String HANDLER_NONE = "noHandler";
	
	public static final float RANGE_CORPSE_DRAG_NORMAL = 40.0f;
	public static final float RANGE_CORPSE_DRAG_INTERIOR = 100.0f;
	
	public static final string_id SID_TARGET_NOT_PLAYER = new string_id("error_message","target_not_player");
	public static final string_id SID_TARGET_SELF_DISALLOWED = new string_id("error_message","target_self_disallowed");
	
	public static final String VAR_SAFE_LOGOUT = "safeLogout";
	
	public static final String DICT_COST = "cost";
	
	public static final String VAR_FACILITY_CLONING = "isCloningFacility";
	
	public static final string_id SID_ISF_BANK = new string_id("error_message","insufficient_funds_bank");
	public static final string_id SID_ISF_CASH = new string_id("error_message","insufficient_funds_cash");
	
	public static final String DICT_KILLER = "killer";
	
	public static final String VAR_REVIVE_BASE = "revive";
	public static final String VAR_REVIVE_OPTIONS = "revive.options";
	public static final String VAR_REVIVE_CLONE = "revive.cloneLocs";
	public static final String VAR_REVIVE_SPAWN = "revive.spawnLocs";
	public static final String VAR_REVIVE_DAMAGE = "revive.damage";
	
	public static final String VAR_SUI_CLONE = "sui.clone";
	
	public static final String VAR_CONSENT_TO_BASE = "consentTo";
	public static final String VAR_CONSENT_TO_ID = "consentTo.id";
	public static final String VAR_CONSENT_TO_NAME = "consentTo.name";
	
	public static final String VAR_CONSENT_FROM_BASE = "consentFrom";
	public static final String VAR_CONSENT_FROM_ID = "consentFrom.id";
	public static final String VAR_CONSENT_FROM_NAME = "consentFrom.name";
	
	public static final string_id PROSE_CONSENT = new string_id("base_player","prose_consent");
	public static final string_id PROSE_UNCONSENT = new string_id("base_player","prose_unconsent");
	public static final string_id PROSE_CONSENT_ALREADY_CONSENTING = new string_id("base_player","prose_consent_already_consenting");
	public static final string_id PROSE_CONSENT_LIST_FULL = new string_id("base_player","prose_consent_list_full");
	
	public static final int LIMIT_CONSENT_TO = 5;
	
	public static final String DICT_CONSENTER_ID = "consenterId";
	public static final String DICT_CONSENTER_NAME = "consenterName";
	
	public static final String HANDLER_CONSENT_TO_LOGOUT = "handleConsentToLogout";
	
	public static final String HANDLER_RECEIVED_CONSENT = "handleReceivedConsent";
	public static final String HANDLER_RECEIVED_UNCONSENT = "handleReceivedUnconsent";
	
	public static final string_id SID_TARGET_OFFLINE = new string_id("error_message","target_offline");
	
	public static final string_id SID_CMD_BAD_TARGET = new string_id("error_message","cmd_bad_target");
	
	public static final int INSURE_COST_BASE = 65;
	
	public static final float INSURE_CONST = 0.015f;
	
	public static final String VAR_TO_INSURE = "toInsure";
	
	public static final String HANDLER_INSURE_REQUEST = "handleInsureRequest";
	
	public static final String HANDLER_INSURE_SUCCESS = "handleInsureSuccess";
	public static final String HANDLER_INSURE_FAILURE = "handleInsureFailure";
	
	public static final String HANDLER_BULK_INSURE_SUCCESS = "handleBulkInsureSuccess";
	public static final String HANDLER_BULK_INSURE_FAILURE = "handleBulkInsureFailure";
	
	public static final String HANDLER_CASH_INSURE = "handleCashInsure";
	
	public static final String HANDLER_REQUESTED_INSURANCE = "handleRequestedInsurance";
	
	public static final int BATCH_SIZE = 10;
	
	public static final String DICT_ITEM_ID = "itemId";
	
	public static final String VAR_INSURE_UI_OPTIONS = "insureUIOptions";
	
	public static final string_id SID_INSURE_UI_ERROR = new string_id("error_message","insure_ui_error");
	public static final string_id SID_NO_INSURABLES = new string_id("error_message","no_insurables");
	public static final string_id SID_NOT_ALL_INSURABLE = new string_id("error_message","not_all_insurable");
	
	public static final string_id SID_INSURE_SUCCESS = new string_id("base_player","insure_success");
	public static final string_id PROSE_INSURE_SUCCESS = new string_id("base_player","prose_insure_success");
	
	public static final string_id SID_BULK_INSURE_ERRORS_TITLE = new string_id("error_message","bulk_insure_errors_title");
	public static final string_id SID_BULK_INSURE_ERRORS_PROMPT = new string_id("error_message","bulk_insure_errors_prompt");
	
	public static final string_id PROSE_NSF_TO_INSURE = new string_id("error_message","prose_nsf_insure");
	
	public static final string_id PROSE_ITEM_ALREADY_INSURED = new string_id("error_message","prose_item_already_insured");
	public static final string_id PROSE_ITEM_UNINSURABLE = new string_id("error_message","prose_item_uninsurable");
	
	public static final string_id SID_NO_CORPSE_PRODUCED = new string_id("base_player", "no_corpse_produced");
	public static final string_id PROSE_NEWBIE_INSURED = new string_id("base_player", "prose_newbie_insured");
	
	public static final string_id SID_LAST_NEWBIE_INSURE = new string_id("base_player", "last_newbie_insure");
	
	public static final String DICT_SCRIPT_NAME = "scriptName";
	public static final String DICT_OBJVAR_NAME = "objVarName";
	
	public static final String DICT_XP_TYPE = "xpType";
	public static final String DICT_XP_AMOUNT = "xpAmount";
	
	public static final String HANDLER_ATTACH_SCRIPT = "handleAttachScript";
	public static final String HANDLER_DETACH_SCRIPT = "handleDetachScript";
	
	public static final String HANDLER_REMOVE_OBJVAR = "handleRemoveObjVar";
	
	public static final String HANDLER_GRANT_XP = "handleGrantXp";
	
	public static final string_id SID_CANNOT_ATTACK_TARGET = new string_id("error_message","target_not_attackable");
	public static final string_id SID_TARGET_NOT_INCAPACITATED = new string_id("error_message","target_not_incapacitated");
	
	public static final string_id SID_TARGET_ALREADY_DEAD = new string_id("error_message","target_already_dead");
	public static final string_id PROSE_TARGET_ALREADY_DEAD = new string_id("error_message","prose_target_already_dead");
	
	public static final string_id SID_KILLER_TARGET_DEAD = new string_id("base_player","killer_target_dead");
	public static final string_id SID_VICTIM_DEAD = new string_id("base_player","victim_dead");
	
	public static final string_id PROSE_TARGET_DEAD = new string_id("base_player","prose_target_dead");
	public static final string_id PROSE_VICTIM_DEAD = new string_id("base_player","prose_victim_dead");
	
	public static final string_id SID_BIND_REMOVED = new string_id("base_player","bind_removed");
	
	public static final string_id SID_YOU_HAVE_CONSENT_FROM = new string_id("base_player","you_have_conent_from");
	public static final string_id SID_YOU_DO_NOT_HAVE_CONSENT = new string_id("base_player","you_do_not_have_consent");
	
	public static final string_id SID_DEFEATED_BOUNTY_HUNTER = new string_id("base_player","defeated_bounty_hunter");
	
	public static final String HANDLER_PLAYER_DEATH = "handlePlayerDeath";
	
	public static final String HANDLER_CLONE_RESPAWN = "handleCloneRespawn";
	
	public static final float TIME_DEATH = 0.25f;
	
	public static final String VAR_CORPSE_BASE = "corpse";
	public static final String VAR_CORPSE_ID = "corpse.id";
	public static final String VAR_CORPSE_KILLER = "corpse.killer";
	public static final String VAR_CORPSE_STAMP = "corpse.stamp";
	
	public static final String VAR_BEEN_COUPDEGRACED = "beenCoupDeGraced";
	public static final String VAR_DEATHBLOW_KILLER = VAR_BEEN_COUPDEGRACED+".killer";
	public static final String VAR_DEATHBLOW_STAMP = VAR_BEEN_COUPDEGRACED+".stamp";
	
	public static final String DATATABLE_AI_SPECIES = "datatables/ai/species.iff";
	public static final String DATATABLE_COL_SKELETON = "Skeleton";
	
	public static final String SKELETON_HUMAN = "human";
	
	public static final int NC_DEATH = 0;
	
	public static final String HANDLER_STAT_MIGRATION = "handleStatMigration";
	
	public static final float TIP_WIRE_SURCHARGE = 0.05f;
	
	public static final String VAR_TIP_BASE = "tip";
	public static final String VAR_TIP_TARGET = VAR_TIP_BASE + ".target";
	public static final String VAR_TIP_TARGET_NAME = VAR_TIP_BASE + ".targetName";
	public static final String VAR_TIP_AMT = VAR_TIP_BASE + ".amt";
	public static final String VAR_TIP_SUI = VAR_TIP_BASE + ".sui";
	
	public static final string_id SID_TIP_CLEAR = new string_id("base_player","tip_clear");
	public static final string_id SID_TIP_ABORT = new string_id("base_player","tip_abort");
	public static final string_id PROSE_TIP_ABORT = new string_id("base_player","prose_tip_abort");
	
	public static final string_id SID_TIP_ERROR = new string_id("base_player","tip_error");
	
	public static final string_id SID_TIP_TARGET_OFFLINE = new string_id("base_player", "tip_target_offline");
	public static final string_id SID_TIP_NSF = new string_id("base_player", "tip_nsf");
	
	public static final string_id SID_TIP_SYNTAX = new string_id("base_player", "tip_syntax");
	
	public static final string_id SID_TIP_WIRE_TITLE = new string_id("base_player", "tip_wire_title");
	public static final string_id SID_TIP_WIRE_PROMPT = new string_id("base_player", "tip_wire_prompt");
	
	public static final string_id PROSE_INVALID_TIP_PARAM = new string_id("base_player", "prose_tip_invalid_param");
	public static final string_id PROSE_INVALID_TIP_AMT = new string_id("base_player", "prose_tip_invalid_amt");
	public static final string_id PROSE_TIP_NSF_CASH = new string_id("base_player", "prose_tip_nsf_cash");
	public static final string_id PROSE_TIP_NSF_BANK = new string_id("base_player", "prose_tip_nsf_bank");
	public static final string_id PROSE_TIP_NSF_WIRE = new string_id("base_player", "prose_tip_nsf_wire");
	public static final string_id PROSE_TIP_RANGE = new string_id("base_player", "prose_tip_range");
	
	public static final string_id PROSE_TIP_PASS_SELF = new string_id("base_player", "prose_tip_pass_self");
	public static final string_id PROSE_TIP_PASS_TARGET = new string_id("base_player", "prose_tip_pass_target");
	
	public static final string_id SID_WIRE_PASS_SELF = new string_id("base_player", "wire_pass_self");
	public static final string_id PROSE_WIRE_PASS_SELF = new string_id("base_player", "prose_wire_pass_self");
	public static final string_id PROSE_WIRE_PASS_TARGET = new string_id("base_player", "prose_wire_pass_target");
	
	public static final string_id SID_MAY_NOT_TIP_TARGET = new string_id("base_player", "may_not_tip_target");
	public static final string_id SID_ONLY_TIP_VALID_TARGETS = new string_id("base_player", "only_tip_valid_targets");
	
	public static final string_id SID_WIRE_MAIL_SUBJECT = new string_id("base_player", "wire_mail_subject");
	public static final string_id PROSE_WIRE_MAIL_FROM = new string_id("base_player", "prose_wire_mail_from");
	
	public static final string_id PROSE_WIRE_MAIL_SELF = new string_id("base_player", "prose_wire_mail_self");
	public static final string_id PROSE_WIRE_MAIL_TARGET = new string_id("base_player", "prose_wire_mail_target");
	
	public static final string_id SID_COVERT_RESPAWN_UNALIGNED = new string_id("base_player", "covert_respawn_unaligned");
	public static final string_id SID_COVERT_RESPAWN_UNFRIENDLY = new string_id("base_player", "covert_respawn_unfriendly");
	public static final string_id SID_COVERT_RESPAWN_OUTSIDE = new string_id("base_player", "covert_respawn_outside");
	
	public static final String TBL_INSURE_DECAY_EVENTS = "datatables/player/insure_decay_event.iff";
	public static final String COL_EVENT = "EVENT";
	public static final String COL_INSURED = "INSURED";
	public static final String COL_UNINSURED = "UNINSURED";
	public static final String COL_DO_UNINSURE = "DO_UNINSURE";
	
	public static final int MIN_JEDI_WAIT_TIME = 60;
	public static final String OBJVAR_JEDI_SKILL_REQUIREMENTS = "jedi.skillsNeeded";
	
	public static final float DECAY_RATE = 0.002f;
	public static final String DECAY_REMAINDER = "decay.remainder";
	
	public static final float MIN_CLONING_SICKNESS_COST = 100;
	public static final float MAX_CLONING_SICKNESS_COST = 5000;
	
	
	public static int getCloningSicknessCureCost(obj_id player) throws InterruptedException
	{
		
		float minCost = MIN_CLONING_SICKNESS_COST;
		float maxCost = MAX_CLONING_SICKNESS_COST;
		int city_id = city.checkCity(player, false);
		if (city_id > 0 && city.cityHasSpec(city_id, city.SF_SPEC_CLONING))
		{
			minCost = MIN_CLONING_SICKNESS_COST/2;
			maxCost = MAX_CLONING_SICKNESS_COST/2;
		}
		
		float level = getLevel(player);
		float costRatio = level / 90f;
		float cost = minCost + ((maxCost - minCost) * costRatio);
		return (int)cost;
	}
	
	
	public static boolean canAffordCloningSicknessCure(obj_id player) throws InterruptedException
	{
		int cost = getCloningSicknessCureCost(player);
		int balance = getTotalMoney(player);
		
		if (cost > balance)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static void cureCloningSickness(obj_id player) throws InterruptedException
	{
		if (!canAffordCloningSicknessCure(player))
		{
			sendSystemMessage(player, new string_id("spam", "not_enough_cash_4_cure"));
			return;
		}
		
		int cost = getCloningSicknessCureCost(player);
		if (cost > 0)
		{
			money.requestPayment(player, money.ACCT_CLONING, cost, "none", null, false);
		}
		
		playClientEffectObj(player, "appearance/pt_cure_cloning_sickness.prt", player, "");
		buff.removeBuff(player, "cloning_sickness");
		playMusic(player, "sound/vo_meddroid_01.snd");
	}
	
	
	public static obj_id grantWayPoint(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(player) || (!isPlayer(player)) || !isIdValid(target))
		{
			return null;
		}
		
		obj_id waypoint = createWaypointInDatapad(player, target);
		
		if (!isIdValid(waypoint))
		{
			return null;
		}
		
		setWaypointActive(waypoint, true);
		setWaypointVisible(waypoint, true);
		
		return waypoint;
	}
	
	
	public static boolean giveTip(obj_id player, obj_id target, String targetName, int amt, boolean useCash) throws InterruptedException
	{
		
		if (!isIdValid(player) || !isIdValid(target) || targetName == null)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " -- Tip aborted: Target or tipper is invalid or Target Name is null.");
			if (isIdValid(player))
			{
				sendSystemMessage( player, new string_id( "error_message", "tip_target_bad") );
			}
			return false;
		}
		
		CustomerServiceLog( "Trade", " Processing Tip, Player: "+ player + " "+ getName(player) + " Target: "+ target + " "+ getName(target) + " Amount: "+ amt + " Cash: "+ useCash );
		
		if (target.isLoaded() && !isPlayer(target))
		{
			if (targetName == null || targetName.equals(""))
			{
				targetName = utils.getStringName(target);
			}
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " -- Tip aborted: Target isn't a player.");
			prose_package ppNoTipTarget = prose.getPackage(SID_MAY_NOT_TIP_TARGET);
			prose.setTT(ppNoTipTarget, target);
			sendSystemMessageProse( player, ppNoTipTarget );
			sendSystemMessage( player, SID_ONLY_TIP_VALID_TARGETS );
			return false;
		}
		
		if (amt < 1)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " -- Tip aborted: Tip amount is less than 1.");
			prose_package invalidAmt = prose.getPackage( PROSE_INVALID_TIP_AMT, amt );
			sendSystemMessageProse( player, invalidAmt );
			return false;
		}
		if (amt > 1000000)
		{
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " -- Tip aborted: Tip amount is greater than 1000000.");
			prose_package invalidAmt = prose.getPackage( PROSE_INVALID_TIP_AMT, amt );
			sendSystemMessageProse( player, invalidAmt );
			return false;
		}
		
		if (!target.isLoaded())
		{
			useCash = false;
		}
		
		if (useCash)
		{
			float dist = getDistance( getLocation(player), getLocation(target) );
			if (dist > 15f)
			{
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " -- Tip changing to bank transfer due to distance. Distance: "+ dist );
				prose_package tooFar = prose.getPackage( PROSE_TIP_RANGE, target );
				sendSystemMessageProse( player, tooFar );
				useCash = false;
			}
		}
		
		dictionary d = new dictionary();
		d.put("target", target);
		d.put("amt", amt);
		d.put("useCash", useCash);
		d.put("targetName", targetName);
		if (useCash)
		{
			
			int cash = getCashBalance( player );
			if (amt > cash)
			{
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Tip aborted: Insufficient cash. Amt: "+ amt + " Cash: "+ cash );
				prose_package nsfCash = prose.getPackage( PROSE_TIP_NSF_CASH, target, amt );
				sendSystemMessageProse( player, nsfCash );
				return false;
			}
			
			CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Transferring Cash! Amt: "+ amt );
			
			if (utils.isFreeTrial(player, target))
			{
				doTipLogging(player, target, amt);
			}
			
			boolean success = transferCashTo( player, target, amt, "handleTipSuccess", "handleTipFail", d );
			if (!success)
			{
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Tip aborted: transferCashTo failed. Amt: "+ amt );
				sendSystemMessage( player, new string_id( "base_player", "cash_transfer_failed") );
				return false;
			}
		}
		else
		{
			
			int bank = getBankBalance( player );
			if (amt > bank)
			{
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Tip aborted: Insufficient bank balance. Amt: "+ amt + " Bank: "+ bank );
				prose_package nsfBank = prose.getPackage( PROSE_TIP_NSF_BANK, target, amt );
				sendSystemMessageProse( player, nsfBank );
				return false;
			}
			
			int pid = showTipSui( player, target, targetName, amt );
			if (pid < 0)
			{
				CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Tip aborted: Unable to create bank tip SUI window.");
				sendSystemMessageTestingOnly( player, "Tip aborted: Unable to create bank tip SUI window.");
				return false;
			}
		}
		
		return true;
	}
	
	
	public static int showTipSui(obj_id player, obj_id target, String targetName, int amt) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target) || (targetName == null) || (targetName.equals("")))
		{
			return -1;
		}
		
		CustomerServiceLog( "Trade", " Tip -- Player: "+ player + " "+ getName(player) + " Target: "+ target + " -- Showing bank tip wire UI.");
		String title = utils.packStringId(SID_TIP_WIRE_TITLE);
		String prompt = utils.packStringId(SID_TIP_WIRE_PROMPT);
		int cBox = sui.msgbox( player, player, prompt, sui.YES_NO, title, "handleWireConfirm");
		if (cBox > -1)
		{
			utils.setScriptVar( player, VAR_TIP_SUI, cBox );
			utils.setScriptVar( player, VAR_TIP_TARGET, target );
			utils.setScriptVar( player, VAR_TIP_TARGET_NAME, targetName );
			utils.setScriptVar( player, VAR_TIP_AMT, amt );
		}
		
		return cBox;
	}
	
	
	public static void cleanupTipSui(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		utils.removeScriptVar( player, VAR_TIP_TARGET );
		utils.removeScriptVar( player, VAR_TIP_TARGET_NAME );
		utils.removeScriptVar( player, VAR_TIP_AMT );
		utils.removeScriptVar( player, VAR_TIP_SUI );
	}
	
	
	public static void doTipLogging(obj_id player, obj_id target, int amt) throws InterruptedException
	{
		int trackTipNumIn = 0;
		int trackTipAmtIn = 0;
		int trackTipNumOut = 0;
		int trackTipAmtOut = 0;
		
		if (!utils.hasScriptVar(player, utils.TIP_OUT_NUM))
		{
			utils.setScriptVar(player, utils.TIP_OUT_NUM, 1);
			utils.setScriptVar(player, utils.TIP_OUT_AMMOUNT, amt);
			utils.setScriptVar(player, utils.TIP_OUT_THACK, getGameTime());
			trackTipNumOut = 1;
			trackTipAmtOut = amt;
		}
		else
		{
			int tipNum = utils.getIntScriptVar(player, utils.TIP_OUT_NUM) +1;
			int tipAmt = utils.getIntScriptVar(player, utils.TIP_OUT_AMMOUNT) + amt;
			utils.setScriptVar(player, utils.TIP_OUT_NUM, tipNum);
			utils.setScriptVar(player, utils.TIP_OUT_AMMOUNT, tipAmt);
			trackTipNumOut = tipNum;
			trackTipAmtOut = tipAmt;
		}
		
		if (!utils.hasScriptVar(target, utils.TIP_IN_NUM))
		{
			utils.setScriptVar(target, utils.TIP_IN_NUM, 1);
			utils.setScriptVar(target, utils.TIP_IN_AMMOUNT, amt);
			utils.setScriptVar(target, utils.TIP_IN_THACK, getGameTime());
			trackTipNumIn = 1;
			trackTipAmtIn = amt;
		}
		else
		{
			int tipNum = utils.getIntScriptVar(target, utils.TIP_IN_NUM) +1;
			int tipAmt = utils.getIntScriptVar(target, utils.TIP_IN_AMMOUNT) + amt;
			utils.setScriptVar(target, utils.TIP_IN_NUM, tipNum);
			utils.setScriptVar(target, utils.TIP_IN_AMMOUNT, tipAmt);
			trackTipNumIn = tipNum;
			trackTipAmtIn = tipAmt;
		}
		
		if (trackTipNumIn > utils.TIP_NUM_MAX || trackTipAmtIn > utils.TIP_AMT_MAX || trackTipNumOut > utils.TIP_NUM_MAX || trackTipAmtOut > utils.TIP_AMT_MAX)
		{
			int pTimeHack = utils.getIntScriptVar(player, utils.TIP_OUT_THACK);
			int tTimeHack = utils.getIntScriptVar(target, utils.TIP_IN_THACK);
			pTimeHack = getGameTime() - pTimeHack;
			tTimeHack = getGameTime() - tTimeHack;
			
			CustomerServiceLog("exploit", "Free Trial Tipping -- Suspicious activity -- %TU has made "+trackTipNumOut+" tips for a total of "+trackTipAmtOut+" over the course of "+pTimeHack+"seconds, %TT has recieved "+trackTipNumIn+" tips for a total of "+trackTipAmtIn+" over the course of "+tTimeHack+" seconds.",player,target);
		}
	}
	
	
	public static boolean consent(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target))
		{
			return false;
		}
		
		if (target == player)
		{
			
			sendSystemMessage(player, SID_TARGET_SELF_DISALLOWED);
			return false;
		}
		
		if (!isPlayer(target))
		{
			
			sendSystemMessage(player, SID_TARGET_NOT_PLAYER);
			return false;
		}
		
		Vector consentTo = getResizeableObjIdArrayObjVar(player, VAR_CONSENT_TO_ID);
		if (consentTo == null)
		{
			Vector newConsentTo = new Vector();
			newConsentTo.setSize(0);
			consentTo = newConsentTo;
		}
		
		if (utils.getElementPositionInArray(consentTo, target) > -1)
		{
			prose_package pp = prose.getPackage(PROSE_CONSENT_ALREADY_CONSENTING, getPlayerFullName(target));
			sendSystemMessageProse(player, pp);
			return false;
		}
		else if (consentTo.size() >= LIMIT_CONSENT_TO)
		{
			prose_package pp = prose.getPackage(PROSE_CONSENT_LIST_FULL, LIMIT_CONSENT_TO);
			sendSystemMessageProse(player, pp);
			return false;
		}
		else
		{
			consentTo = utils.addElement(consentTo, target);
			
			obj_id[] corpses = getObjIdArrayObjVar(player, VAR_CORPSE_ID);
			if ((corpses != null) && (corpses.length > 0))
			{
				corpse.grantCorpseConsent(corpses, target);
			}
			
			if (setObjVar(player, VAR_CONSENT_TO_ID, consentTo, resizeableArrayTypeobj_id))
			{
				dictionary d = new dictionary();
				d.put(DICT_CONSENTER_ID, player);
				
				prose_package pp = prose.getPackage(PROSE_CONSENT, getPlayerFullName(target));
				sendSystemMessageProse(player, pp);
				
				messageTo(target, HANDLER_RECEIVED_CONSENT, d, 0, true);
				return true;
			}
		}
		
		return false;
	}
	
	
	public static boolean unconsent(obj_id player, obj_id target) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target))
		{
			return false;
		}
		
		if (target == player)
		{
			
			sendSystemMessage(player, SID_TARGET_SELF_DISALLOWED);
			return false;
		}
		
		obj_id[] corpses = getObjIdArrayObjVar(player, VAR_CORPSE_ID);
		if ((corpses != null) && (corpses.length > 0))
		{
			corpse.revokeCorpseConsent(corpses, target);
		}
		else
		{
			removeObjVar(player, VAR_CORPSE_ID);
		}
		
		Vector consentTo = getResizeableObjIdArrayObjVar(player, VAR_CONSENT_TO_ID);
		if (consentTo != null)
		{
			int idx = utils.getElementPositionInArray(consentTo, target);
			if (idx > -1)
			{
				consentTo = utils.removeElementAt(consentTo, idx);
				
				if (consentTo.size() == 0)
				{
					removeObjVar(player, VAR_CONSENT_TO_BASE);
				}
				else
				{
					setObjVar(player, VAR_CONSENT_TO_ID, consentTo, resizeableArrayTypeobj_id);
				}
				
				prose_package pp = prose.getPackage(PROSE_UNCONSENT, getPlayerFullName(target));
				sendSystemMessageProse(player, pp);
				
				dictionary d = new dictionary();
				d.put(DICT_CONSENTER_ID, player);
				
				messageTo(target, HANDLER_RECEIVED_UNCONSENT, d, 0, true);
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
		
		return true;
	}
	
	
	public static boolean hasConsent(obj_id player, obj_id target, boolean verbose) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(target))
		{
			return false;
		}
		
		if (target == player)
		{
			
			return false;
		}
		
		boolean isConsented = false;
		
		obj_id[] consentFrom = getObjIdArrayObjVar(player, VAR_CONSENT_FROM_ID);
		if ((consentFrom == null) || (consentFrom.length == 0))
		{
			
		}
		else
		{
			if (utils.getElementPositionInArray(consentFrom, target) > -1)
			{
				isConsented = true;
			}
		}
		
		if (verbose)
		{
			if (isConsented)
			{
				prose_package ppYouHaveConsentFrom = prose.getPackage(SID_YOU_HAVE_CONSENT_FROM);
				prose.setTT(ppYouHaveConsentFrom, target);
				sendSystemMessageProse(player, ppYouHaveConsentFrom);
			}
			else
			{
				prose_package ppYouDoNotHaveConsent = prose.getPackage(SID_YOU_DO_NOT_HAVE_CONSENT);
				prose.setTT(ppYouDoNotHaveConsent, target);
				sendSystemMessageProse(player, ppYouDoNotHaveConsent);
			}
		}
		
		return isConsented;
	}
	
	
	public static boolean hasConsent(obj_id player, obj_id target) throws InterruptedException
	{
		return hasConsent(player, target, false);
	}
	
	
	public static boolean relinquishConsents(obj_id player) throws InterruptedException
	{
		if ((player == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		if (hasObjVar(player, VAR_CONSENT_FROM_BASE))
		{
			obj_id[] consentFromList = getObjIdArrayObjVar(player, VAR_CONSENT_FROM_ID);
			if ((consentFromList == null) || (consentFromList.length == 0))
			{
				return false;
			}
			
			dictionary d = new dictionary();
			d.put(corpse.DICT_PLAYER_ID, player);
			
			boolean litmus = true;
			for (int i = 0; i < consentFromList.length; i++)
			{
				testAbortScript();
				litmus &= messageTo(consentFromList[i], HANDLER_CONSENT_TO_LOGOUT, d, 0, true);
			}
			removeObjVar(player, VAR_CONSENT_FROM_BASE);
			return litmus;
		}
		return false;
	}
	
	
	public static location getEffectiveDeathLocation(obj_id player) throws InterruptedException
	{
		
		location loc = getLocation(player);
		if (!loc.area.startsWith("space_"))
		{
			return loc;
		}
		
		loc = getLocationObjVar(player, "space.launchLoc");
		if (loc != null)
		{
			return loc;
		}
		
		return new location(0.0f, 0.0f, 0.0f, "tatooine");
	}
	
	
	public static void killPlayer(obj_id player, obj_id killer) throws InterruptedException
	{
		killPlayer(player, killer, true);
	}
	
	
	public static void killPlayer(obj_id player, obj_id killer, boolean usePVPRules) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(killer) || (!isPlayer(player)))
		{
			
			return;
		}
		
		int dam = -(getAttrib(player, HEALTH) + 50);
		addAttribModifier(player, HEALTH, dam, 0f, 0f, MOD_POOL);
		coupDeGrace(player, killer, false, usePVPRules);
	}
	
	
	public static void coupDeGrace(obj_id victim, obj_id killer, boolean playAnim) throws InterruptedException
	{
		coupDeGrace(victim, killer, playAnim, true);
		
	}
	
	public static void coupDeGrace(obj_id victim, obj_id killer, boolean playAnim, boolean usePVPRules) throws InterruptedException
	{
		
		if (!isIdValid(victim) || !isIdValid(killer))
		{
			return;
		}
		
		if (utils.hasScriptVar(victim, "death.beingCoupDeGraced"))
		{
			int expireTime = utils.getIntScriptVar(victim, "death.beingCoupDeGraced");
			int currentTime = getGameTime();
			if (currentTime <= expireTime)
			{
				return;
			}
			else
			{
				utils.removeScriptVar(victim, "death.beingCoupDeGraced");
			}
		}
		
		if (!victim.isAuthoritative() || !killer.isAuthoritative())
		{
			
			requestSameServer(victim, killer);
			int expireTime = getGameTime() + 60;
			utils.setScriptVar(victim, "death.beingCoupDeGraced", expireTime);
			
			int numberOfTries = 1;
			dictionary dict = new dictionary();
			dict.put("numberOfTries", numberOfTries);
			dict.put("victim", victim);
			dict.put("killer", killer);
			dict.put("playAnim", playAnim);
			dict.put("usePVPRules", usePVPRules);
			
			messageTo(victim, "msgCoupDeGraceAuthoritativeCheck", dict, 2, false);
			return;
		}
		
		if (isDead(victim))
		{
			prose_package pp = prose.getPackage(PROSE_TARGET_ALREADY_DEAD, victim);
			sendSystemMessageProse(killer, pp);
			
			return;
		}
		
		if (!isIncapacitated(victim))
		{
			sendSystemMessage(killer, SID_TARGET_NOT_INCAPACITATED);
			return;
		}
		
		utils.setScriptVar(victim, "lastKiller", killer);
		
		if (!hasObjVar(victim, VAR_BEEN_COUPDEGRACED))
		{
			setObjVar(victim, VAR_DEATHBLOW_KILLER, killer);
			setObjVar(victim, VAR_DEATHBLOW_STAMP, getGameTime());
		}
		
		gcw.releaseGcwPointCredit(victim);
		
		obj_id master = null;
		obj_id pvpKiller = killer;
		
		if (!isPlayer(killer))
		{
			if (isMob(killer))
			{
				master = getMaster(killer);
				
				if (isIdValid(master) && isPlayer(master))
				{
					pvpKiller = master;
				}
			}
		}
		
		if (isPlayer(pvpKiller))
		{
			utils.setScriptVar(victim, "pvp_death", 1);
			
			pvp.bfCreditForKill(pvpKiller);
			pvp.bfCreditForDeath(victim, pvpKiller);
		}
		
		boolean onBattlefield = false;
		
		region r = battlefield.getBattlefield(victim);
		if (r == null)
		{
			if (usePVPRules)
			{
				if (!pvpCanAttack(killer, victim) && (killer != victim))
				{
					sendSystemMessage(killer, SID_CANNOT_ATTACK_TARGET);
					return;
				}
			}
		}
		else
		{
			onBattlefield = true;
			if (!pvpIsEnemy(killer, victim) && (killer != victim))
			{
				sendSystemMessage(killer, SID_CANNOT_ATTACK_TARGET);
				return;
			}
		}
		
		if (playAnim)
		{
			playDeathBlowAnimation(victim, killer);
		}
		
		boolean dueling = pvpIsDueling(victim, killer);
		
		if (getPosture(victim) != POSTURE_DEAD)
		{
			if (!setPosture(victim, POSTURE_DEAD))
			{
				
			}
		}
		
		if (killer != victim)
		{
			prose_package ppToKiller = prose.getPackage(PROSE_TARGET_DEAD, victim);
			combat.sendCombatSpamMessageProse(killer, victim, ppToKiller, true, false, true);
		}
		
		if (onBattlefield)
		{
			
			if (battlefield.isBattlefieldActive(r))
			{
				battlefield.sendBattlefieldMessage(r, getFirstName(victim) + " has been eliminated by "+ getFirstName(killer));
				
				obj_id bf_object = battlefield.getMasterObjectFromRegion(r);
				dictionary params = new dictionary();
				params.put("master_object", bf_object);
				messageTo(victim, "msgBattlefieldDeath", params, battlefield.DEATH_TIME, true);
				
				battlefield.registerBattlefieldKill(killer, victim, bf_object);
				
				return;
			}
		}
		else
		{
			
		}
		playerDeath(victim, killer, dueling);
	}
	
	
	public static void playDeathBlowAnimation(obj_id victim, obj_id killer) throws InterruptedException
	{
		if (!isIdValid(victim) || !isIdValid(killer))
		{
			return;
		}
		
		String skeleton = "";
		if (dataTableOpen(DATATABLE_AI_SPECIES))
		{
			int species = getSpecies(killer);
			if (species == -1)
			{
				
			}
			else
			{
				skeleton = dataTableGetString(DATATABLE_AI_SPECIES, species, DATATABLE_COL_SKELETON);
			}
		}
		
		if (skeleton != null && !skeleton.equals("") && (killer != victim))
		{
			if (isPlayer(killer) || (skeleton.equals(SKELETON_HUMAN)))
			{
				
				String strPlaybackScript = "";
				obj_id objWeapon = getCurrentWeapon(killer);
				int intWeaponType = getWeaponType(objWeapon);
				int intWeaponCategory = combat.getWeaponCategory(intWeaponType);
				
				if (intWeaponCategory == combat.RANGED_WEAPON)
				{
					
					strPlaybackScript = "ranged_coup_de_grace";
					
				}
				else
				{
					
					if (intWeaponType == WEAPON_TYPE_UNARMED)
					{
						strPlaybackScript = "unarmed_coup_de_grace";
						
					}
					else
					{
						
						strPlaybackScript = "melee_coup_de_grace";
					}
					
				}
				
				attacker_results cbtAttackerResults = new attacker_results();
				defender_results[] cbtDefenderResults = new defender_results[1];
				cbtDefenderResults[0] = new defender_results();
				
				cbtAttackerResults.id = killer;
				cbtAttackerResults.endPosture = getPosture(killer);
				cbtAttackerResults.weapon = objWeapon;
				
				cbtDefenderResults[0].endPosture = POSTURE_DEAD;
				cbtDefenderResults[0].result = COMBAT_RESULT_HIT;
				cbtDefenderResults[0].id = victim;
				doCombatResults(strPlaybackScript, cbtAttackerResults, cbtDefenderResults);
			}
		}
	}
	
	
	public static void coupDeGrace(obj_id victim, obj_id killer) throws InterruptedException
	{
		coupDeGrace (victim, killer, true);
	}
	
	
	public static boolean playerDeath(obj_id player, obj_id killer, boolean dueling) throws InterruptedException
	{
		
		if (!isIdValid(player) || killer == null)
		{
			return false;
		}
		
		if (!isDead(player))
		{
			sendSystemMessageTestingOnly(killer, "apparently, "+ getName(player) + " is still not dead");
			return false;
		}
		
		if (killer == obj_id.NULL_ID || player == killer)
		{
			sendSystemMessage(player, SID_VICTIM_DEAD);
		}
		else
		{
			prose_package ppToVictim = prose.getPackage(PROSE_VICTIM_DEAD, killer);
			sendSystemMessageProse(player, ppToVictim);
		}
		
		float factionMod = 1.0f;
		
		if (!isPlayer(killer) && isMob(killer) && isIdValid(getMaster(killer)))
		{
			killer = getMaster(killer);
		}
		
		if (isPlayer(killer) && player != killer)
		{
			if (utils.isProfession(killer, utils.BOUNTY_HUNTER) && isBeingHuntedByBountyHunter(player, killer))
			{
				bounty_hunter.winBountyMission(killer, player);
				smuggler.checkSmugglerMissionBountyFailure(player, killer);
			}
			else if (utils.isProfession(player, utils.BOUNTY_HUNTER) && isBeingHuntedByBountyHunter(killer, player))
			{
				bounty_hunter.loseBountyMission(player, killer);
			}
			else
			{
				if ((getTotalMoney(player) >= bounty_hunter.MIN_BOUNTY_SET) && (!dueling) && (getLevel(killer) >= 20))
				{
					bounty_hunter.showSetBountySUI(player, killer);
				}
			}
			
			pvp.incrementPlayerDeathBounty(killer, player);
			
			if (isPlayer(player))
			{
				guildUpdateGuildWarKillTracking(killer, player);
			}
		}
		
		factions.grantCombatFaction( killer, player, factionMod );
		
		dot.removeAllDots(player);
		
		clearAllHate(player);
		
		playMusic(player, "sound/music_player_death.snd");
		
		obj_id gid = getGroupObject(player);
		if (isIdValid(gid))
		{
			group.notifyDeath(gid, player);
		}
		messageTo(player, HANDLER_PLAYER_DEATH, null, TIME_DEATH, true);
		dictionary params = new dictionary();
		params.put("victim", player);
		messageTo(killer, "handleKillerDeathBlow", params, 1, false);
		return true;
	}
	
	
	public static void clearAllHate(obj_id self) throws InterruptedException
	{
		
	}
	
	
	public static boolean playerRevive(obj_id player) throws InterruptedException
	{
		return playerRevive(player, getWorldLocation(player), getLocation(player), 0);
	}
	
	
	public static boolean playerRevive(obj_id player, location reviveLoc, location spawnLoc, int damage) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		if (!isDead(player))
		{
			LOG("playerRevive", "playerRevive: player is not dead!");
			return false;
		}
		
		if (reviveLoc == null || spawnLoc == null)
		{
			return false;
		}
		
		setObjVar(player, "fullHealClone", true);
		boolean warped = sendToCloneSpawn(player, HANDLER_CLONE_RESPAWN, reviveLoc, spawnLoc);
		
		if (!warped)
		{
			sendSystemMessage(player, cloninglib.SID_RESPAWN_CURRENT_LOCATION);
			messageTo(player, HANDLER_CLONE_RESPAWN, null, 2, true);
		}
		
		return true;
	}
	
	
	public static boolean equipClone(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		obj_id inv = utils.getInventoryContainer(player);
		if (!isIdValid(inv))
		{
			return false;
		}
		
		obj_id[] items = utils.getFilteredPlayerContents(player);
		if ((items != null) && (items.length > 0))
		{
			for (int i = 0; i < items.length; i++)
			{
				testAbortScript();
				int got = getGameObjectType(items[i]);
				if (isGameObjectTypeOf(got, GOT_clothing) || isGameObjectTypeOf(got, GOT_armor))
				{
					equip(items[i], player);
				}
			}
		}
		
		return true;
	}
	
	
	public static boolean autoInsureItem(obj_id item) throws InterruptedException
	{
		if (!isIdValid(item))
		{
			return false;
		}
		
		return setAutoInsured(item);
	}
	
	
	public static boolean resurrectPlayer(obj_id target) throws InterruptedException
	{
		return resurrectPlayer(target, false);
	}
	
	
	public static boolean resurrectPlayer(obj_id target, boolean playCloneAnimation) throws InterruptedException
	{
		if (!isIdValid(target) || !isPlayer(target))
		{
			return false;
		}
		
		int posture = getPosture(target);
		
		if (posture == POSTURE_DEAD)
		{
			clearEffectsForDeath(target);
			
			if (utils.hasScriptVar(target, VAR_SUI_CLONE))
			{
				int suiClone = utils.getIntScriptVar(target, VAR_SUI_CLONE);
				if (suiClone > -1)
				{
					sui.closeSUI(target, suiClone);
				}
			}
			
			utils.removeScriptVar(target, VAR_SUI_CLONE);
			utils.removeScriptVar(target, VAR_REVIVE_OPTIONS);
			
			trimLastKiller(target);
			
			pvpRemoveAllTempEnemyFlags(target);
			
			removeObjVar(target, VAR_BEEN_COUPDEGRACED);
			
			queueCommand(target, (-1465754503), target, "", COMMAND_PRIORITY_IMMEDIATE);
			
			if (playCloneAnimation)
			{
				playClientEffectObj(target, "clienteffect/player_clone_compile.cef", target, null);
			}
			
		}
		
		healing.fullHealEveryone(target);
		
		setAttrib(target, HEALTH, getMaxAttrib(target, HEALTH));
		setAttrib(target, ACTION, getMaxAttrib(target, ACTION));
		
		dot.removeAllDots(target);
		
		return true;
	}
	
	
	public static boolean clearCombatData(obj_id target) throws InterruptedException
	{
		queueCommand(target, (-1465754503), target, "", COMMAND_PRIORITY_IMMEDIATE);
		
		setCombatTarget(target, null);
		stopCombat(target);
		
		obj_id[] objEnemies = getHateList(target);
		for (int i = 0; i < objEnemies.length; i++)
		{
			testAbortScript();
			if (!isIdValid(objEnemies[i]) || !exists(objEnemies[i]))
			{
				continue;
			}
			
			if (!isPlayer(objEnemies[i]))
			{
				if (isIdValid(objEnemies[i]) && exists(objEnemies[i]) && isIdValid(target) && exists(target))
				{
					
				}
			}
			else
			{
				setCombatTarget(objEnemies[i], null);
			}
		}
		
		removeObjVar(target, "intBurstRunning");
		removeObjVar(target, "combat.intLastBurstRunTime");
		utils.removeScriptVar(target, "combat.intKnockdownTime");
		utils.removeScriptVar(target, "combat.intPostureChangeTime");
		utils.removeScriptVarTree(target, "combat.combatDelay");
		
		removeObjVar(target, innate.VAR_ROAR);
		removeObjVar(target, innate.VAR_VITALIZE);
		removeObjVar(target, innate.VAR_EQUILIBRIUM);
		removeObjVar(target, innate.VAR_REGENERATION);
		
		pvpRemoveAllTempEnemyFlags(target);
		
		return true;
	}
	
	
	public static boolean clearBuffData(obj_id target) throws InterruptedException
	{
		return buff.removeAllBuffs(target);
	}
	
	
	public static void trimLastKiller(obj_id self) throws InterruptedException
	{
		Vector killers = utils.getResizeableObjIdBatchObjVar(self, VAR_CORPSE_KILLER);
		if (killers != null && killers.size() > 0)
		{
			killers = utils.removeElementAt(killers, killers.size() - 1);
		}
		
		if (killers == null || killers.size() == 0)
		{
			removeObjVar(self, VAR_CORPSE_KILLER);
		}
		else
		{
			utils.setResizeableBatchObjVar(self, VAR_CORPSE_KILLER, killers);
		}
	}
	
	
	public static boolean checkCovertRespawn(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		String pFac = factions.getFaction(player);
		if (factions.isDeclared(player) && pFac != null && !pFac.equals(""))
		{
			obj_id topMost = getTopMostContainer(player);
			if (isIdValid(topMost))
			{
				if (topMost == player)
				{
					covertRespawn(player, "outside");
				}
				else
				{
					String sFac = factions.getFaction(topMost);
					if (sFac == null || sFac.equals("") || !sFac.equals(pFac))
					{
						covertRespawn(player, sFac);
					}
				}
			}
		}
		
		return false;
	}
	
	
	public static void covertRespawn(obj_id player, String faction) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		if (hasScript (player, "theme_park.dungeon.corvette.timer") || getLocation(player).area.equals("adventure2"))
		{
			return;
		}
		
		pvpMakeCovert(player);
		removeObjVar(player, "travelingCovert");
		
		if (faction == null || faction.equals(""))
		{
			sendSystemMessage(player, SID_COVERT_RESPAWN_UNALIGNED);
		}
		else if (faction.equals("outside"))
		{
			sendSystemMessage(player, SID_COVERT_RESPAWN_OUTSIDE);
		}
		else
		{
			sendSystemMessage(player, SID_COVERT_RESPAWN_UNFRIENDLY);
		}
	}
	
	
	public static void destroyPlayerEquipment(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		boolean isTrandoshan = isSpecies(player, SPECIES_TRANDOSHAN);
		
		for (int i = 0; i < slots.EQ_SLOTS.length; ++i)
		{
			testAbortScript();
			
			if (isTrandoshan && slots.EQ_SLOTS[i] == slots.SHOES)
			{
				continue;
			}
			
			obj_id object = getObjectInSlot(player, slots.EQ_SLOTS[i]);
			if (isIdValid(object))
			{
				destroyObject(object);
			}
		}
	}
	
	
	public static void destroyPlayerInventory(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		obj_id inventory = getObjectInSlot(player, utils.SLOT_INVENTORY);
		if (!isIdValid(inventory))
		{
			return;
		}
		
		obj_id[] contents = getContents(inventory);
		if (contents == null)
		{
			return;
		}
		
		for (int i = 0; i < contents.length; ++i)
		{
			testAbortScript();
			if (isIdValid(contents[i]))
			{
				destroyObject(contents[i]);
			}
		}
	}
	
	
	public static void destroyPlayerBank(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return;
		}
		
		obj_id inventory = getObjectInSlot(player, utils.SLOT_BANK);
		if (!isIdValid(inventory))
		{
			return;
		}
		
		obj_id[] contents = getContents(inventory);
		if (contents == null)
		{
			return;
		}
		
		for (int i = 0; i < contents.length; ++i)
		{
			testAbortScript();
			if (isIdValid(contents[i]))
			{
				destroyObject(contents[i]);
			}
		}
	}
	
	
	public static void damageAndDecayItem(obj_id item, float percent) throws InterruptedException
	{
		
	}
	
	
	public static void damageAndDecayItem(obj_id item, int amount) throws InterruptedException
	{
		
	}
	
	
	public static boolean sendToCloneSpawn(obj_id player, String callback, location world, location spawn) throws InterruptedException
	{
		
		if ((player == null) || (world == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		if (spawn == null)
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, null, world.x, world.y, world.z, callback);
		}
		else if (spawn.cell == null || spawn.cell == obj_id.NULL_ID)
		{
			
			warpPlayer(player, spawn.area, spawn.x, spawn.y, spawn.z, null, spawn.x, spawn.y, spawn.z, callback);
		}
		else
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, spawn.cell, spawn.x, spawn.y, spawn.z, callback);
		}
		return true;
	}
	
	
	public static boolean sendToAnyLocation(obj_id player, location world, location interior) throws InterruptedException
	{
		
		if ((player == null) || (world == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		if (interior == null || interior.cell == null || interior.cell == obj_id.NULL_ID)
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, null, world.x, world.y, world.z);
		}
		else
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, interior.cell, interior.x, interior.y, interior.z);
		}
		return true;
	}
	
	
	public static boolean sendToAnyLocation(obj_id player, String callback, location world, location interior) throws InterruptedException
	{
		
		if ((player == null) || (world == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		utils.setScriptVar(player, "waitingOnCloneRespawn", 1);
		
		if (interior == null || interior.cell == null || interior.cell == obj_id.NULL_ID)
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, null, world.x, world.y, world.z, callback);
		}
		else
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, interior.cell, interior.x, interior.y, interior.z, callback);
		}
		return true;
	}
	
	
	public static boolean sendToAnyLocation(obj_id player, location world, location interior, String interiorCell) throws InterruptedException
	{
		
		if ((player == null) || (world == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		if (interior == null || interiorCell == null)
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, null, world.x, world.y, world.z);
		}
		else
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, interior.cell, interiorCell, interior.x, interior.y, interior.z);
		}
		return true;
	}
	
	
	public static boolean sendToAnyLocation(obj_id player, String callback, location world, location interior, String interiorCell) throws InterruptedException
	{
		
		if ((player == null) || (world == null) || (!isPlayer(player)))
		{
			return false;
		}
		
		utils.setScriptVar(player, "waitingOnCloneRespawn", 1);
		
		if (interior == null || interiorCell == null)
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, null, world.x, world.y, world.z, callback);
		}
		else
		{
			
			warpPlayer(player, world.area, world.x, world.y, world.z, interior.cell, interiorCell, interior.x, interior.y, interior.z, callback);
		}
		return true;
	}
	
	
	public static boolean clearEffectsForDeath(obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return false;
		}
		
		dot.removeAllDots(player);
		
		removeAllAttribModifiers(player);
		
		for (int i = 0; i < NUM_ATTRIBUTES; i++)
		{
			testAbortScript();
			String objvarName = "healing."+ healing.attributeToString(i).toLowerCase() + "_enhance";
			if (hasObjVar(player, objvarName))
			{
				removeObjVar(player, objvarName);
			}
			
			if (utils.hasScriptVar(player, objvarName))
			{
				utils.removeScriptVar(player, objvarName);
			}
		}
		
		removeObjVar(player, "spice");
		utils.setScriptVar(player, "numPukes", 100 );
		detachScript(player, "player.player_spice");
		
		if (hasObjVar(player, meditation.VAR_POWERBOOST_ACTIVE))
		{
			removeObjVar(player, meditation.VAR_POWERBOOST_ACTIVE);
		}
		
		setPosture(player, POSTURE_INCAPACITATED);
		return true;
	}
	
	
	public static boolean isContainedByPlayer(obj_id player, obj_id item) throws InterruptedException
	{
		if ((player == null) || (item == null))
		{
			return false;
		}
		
		Vector containers = new Vector();
		containers.setSize(0);
		
		obj_id c = getContainedBy(item);
		if ((c == obj_id.NULL_ID) || (c == null))
		{
			return false;
		}
		
		while ((c != obj_id.NULL_ID) && (c != null))
		{
			testAbortScript();
			containers = utils.addElement(containers, c);
			c = getContainedBy(c);
		}
		
		if (utils.getElementPositionInArray(containers, player) > -1)
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean msgAttachScript(obj_id player, String scriptName) throws InterruptedException
	{
		if ((player == null) || (scriptName.equals("")))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		
		params.put(DICT_SCRIPT_NAME, scriptName);
		
		return messageTo(player, HANDLER_ATTACH_SCRIPT, params, 0, true);
	}
	
	
	public static boolean msgDetachScript(obj_id player, String scriptName) throws InterruptedException
	{
		if ((player == null) || (scriptName.equals("")))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		
		params.put(DICT_SCRIPT_NAME, scriptName);
		
		return messageTo(player, HANDLER_DETACH_SCRIPT, params, 0, true);
	}
	
	
	public static boolean msgRemoveObjVar(obj_id player, String objVarName) throws InterruptedException
	{
		if ((player == null) || (objVarName.equals("")))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		
		params.put(DICT_OBJVAR_NAME, objVarName);
		
		return messageTo(player, HANDLER_REMOVE_OBJVAR, params, 0, true);
	}
	
	
	public static boolean msgGrantXP(obj_id player, String xpType, int xpAmount) throws InterruptedException
	{
		if ((player == null) || (xpType.equals("")))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		
		params.put(DICT_XP_TYPE, xpType);
		params.put(DICT_XP_AMOUNT, xpAmount);
		
		return messageTo(player, HANDLER_GRANT_XP, params, 0, true);
	}
	
	
	public static boolean notifyGM(obj_id player, int nc) throws InterruptedException
	{
		if (player == null)
		{
			return false;
		}
		
		switch (nc)
		{
			case NC_DEATH:
			sendSystemMessagePlanetTestingOnly("DEATH-related error for player "+ getName(player) + " ("+ player + "). Please alert a GM on their behalf.");
			break;
			
			default:
			sendSystemMessagePlanetTestingOnly("Unknown error for player "+ getName(player) + " ("+ player + "). Please alert a GM on their behalf.");
			break;
		}
		
		return true;
	}
	
	
	public static void requestWaypointDestroy(obj_id target, obj_id wp) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(wp))
		{
			return;
		}
		
		dictionary params = new dictionary();
		params.put("wp", wp);
		
		messageTo(target, "handleWaypointDestroyRequest", params, 1f, true);
	}
	
	
	public static void requestWaypointRename(obj_id target, obj_id wp, String name) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(wp) || name == null || name.equals(""))
		{
			return;
		}
		
		dictionary params = new dictionary();
		params.put("wp", wp);
		params.put("name", name);
		
		messageTo(target, "handleWaypointRenameRequest", params, 1f, true);
	}
	
	
	public static void requestWaypointRename(obj_id target, obj_id wp, string_id sid_name) throws InterruptedException
	{
		if (!isIdValid(target) || !isIdValid(wp) || sid_name == null)
		{
			return;
		}
		
		String name = utils.packStringId(sid_name);
		
		requestWaypointRename(target, wp, name);
	}
	
	
	public static String[] buildSortedResourceList(obj_id[] resources, String resourceClass, int branchLevel) throws InterruptedException
	{
		Vector resourceList = new Vector();
		resourceList.setSize(0);
		
		for (int i = 0; i < resources.length; i++)
		{
			testAbortScript();
			if (!isResourceDerivedFrom(resources[i], resourceClass))
			{
				continue;
			}
			
			String parent = getResourceClass(resources[i]);
			String child = null;
			
			if (parent == null)
			{
				continue;
			}
			
			while (!parent.equals(resourceClass))
			{
				testAbortScript();
				child = parent;
				parent = getResourceParentClass(child);
			}
			
			if (child == null)
			{
				child = "\\#pcontrast1 "+getResourceName(resources[i])+"\\#.";
			}
			
			for (int j = 0; j < branchLevel; j++)
			{
				testAbortScript();
				child = " "+ child;
			}
			
			if (resourceList.indexOf(child) == -1)
			{
				resourceList.add(child);
			}
		}
		
		for (int i = 0; i < resourceList.size(); i++)
		{
			testAbortScript();
			String parent = ((String)(resourceList.get(i))).trim();
			String[] childBranch = buildSortedResourceList(resources, parent, branchLevel+1);
			
			for (int j = 0; j < childBranch.length; j++)
			{
				testAbortScript();
				resourceList.add(++i, childBranch[j]);
			}
		}
		
		return (String[])resourceList.toArray(new String[0]);
	}
	
	
	public static String createResourcePlanetReport(String[] resourceList, String planet, String resourceClass) throws InterruptedException
	{
		String report = "Incoming planetary survey report...\n\n"+
		"\\#pcontrast3 Planet: \\#pcontrast1 "+toUpper(planet, 0)+"\n"+
		"\\#pcontrast3 Resource Class: \\#pcontrast1 "+getClassString(resourceClass)+"\n\n"+
		"\\#pcontrast3 Resources located...\\#.\n\n";
		
		for (int i = 0; i < resourceList.length; i++)
		{
			testAbortScript();
			String resourceName = resourceList[i].trim();
			
			if (resourceName.startsWith("\\#"))
			{
				resourceName = resourceName.substring(13, resourceName.length()-3);
			}
			
			obj_id resourceId = getResourceTypeByName(resourceName);
			
			if (resourceId == null)
			{
				report += " "+getTab(resourceList[i])+getClassString(resourceName)+"\n";
			}
			else
			{
				report += " "+resourceList[i]+"\n";
			}
		}
		
		report += "\n\n";
		
		return report;
	}
	
	
	public static String getClassString(String className) throws InterruptedException
	{
		final String resourceTable = "datatables/resource/resource_tree.iff";
		
		String classString = "";
		
		int row = dataTableSearchColumnForString(className, 1, resourceTable);
		int column = 2;
		while ((classString == null || classString.length() == 0) && column <= 9)
		{
			testAbortScript();
			classString = dataTableGetString(resourceTable, row, column);
			column++;
		}
		
		return classString;
	}
	
	
	public static String getTab(String str) throws InterruptedException
	{
		String tab = "";
		int idx = 0;
		
		while (str.charAt(idx) == ' ')
		{
			testAbortScript();
			tab += " ";
			idx++;
		}
		
		return tab;
	}
	
}
