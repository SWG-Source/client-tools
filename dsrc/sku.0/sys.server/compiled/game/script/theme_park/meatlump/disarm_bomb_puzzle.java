package script.theme_park.meatlump;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.buff;
import script.library.collection;
import script.library.consumable;
import script.library.sui;
import script.library.stealth;
import script.library.utils;
import java.lang.System;
import java.util.Random;


public class disarm_bomb_puzzle extends script.base_script
{
	public disarm_bomb_puzzle()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String VAR_PREFIX = "meatlump_target_map";
	public static final String PID_NAME = VAR_PREFIX + ".pid";
	public static final String CURRENTLY_DEFUSING = VAR_PREFIX + ".defusing";
	public static final String BOMB_TIMER = VAR_PREFIX + ".bomb_timer";
	public static final String TIMER_RUNNING = VAR_PREFIX + ".timer_running";
	public static final String RED_WIRE = VAR_PREFIX + ".red_wire";
	public static final String BLACK_WIRE = VAR_PREFIX + ".black_wire";
	public static final String GREEN_WIRE = VAR_PREFIX + ".green_wire";
	public static final String YELLOW_WIRE = VAR_PREFIX + ".yellow_wire";
	public static final String BROWN_WIRE = VAR_PREFIX + ".brown_wire";
	public static final String WHITE_WIRE = VAR_PREFIX + ".white_wire";
	public static final String YELLOW_BROWN_WIRE = VAR_PREFIX + ".yellow_brown_wire";
	public static final String RED_BLACK_WIRE = VAR_PREFIX + ".red_black_wire";
	public static final String YELLOW_GREEN_WIRE = VAR_PREFIX + ".yellow_green_wire";
	public static final String BLACK_WHITE_WIRE = VAR_PREFIX + ".black_white_wire";
	public static final String WIRE_ARRAY = VAR_PREFIX + ".wire_array";
	public static final String COLOR_ARRAY = VAR_PREFIX + ".color_array";
	public static final String CUT_ARRAY = VAR_PREFIX + ".cut_array";
	public static final String BUTTON_NUMBER = VAR_PREFIX + ".button_number";
	public static final String BEING_DEFUSED = VAR_PREFIX + ".being_defused";
	public static final String TIME_DEFUSED = VAR_PREFIX + ".time_defused";
	public static final String PLAYER_MAX_TIMER = VAR_PREFIX + ".player_max_timer";
	public static final String PLAYER_BUFF = VAR_PREFIX + ".player_buff";
	
	public static final String CAPTION = "Disarm Bomb";
	public static final String DEVICE_TEMPLATE = "object/tangible/meatlump/event/slicing_device_meatlump_bomb.iff";
	public static final String DEVICE_OBJVAR = "puzzle";
	public static final String BOMB_PUZZLE_BUFF = "bomb_defuse_puzzle_buff";
	
	public static final string_id SID_OPEN = new string_id("meatlump/meatlump", "meatlump_defuse_bomb");
	public static final string_id BOMB_INTRO_TEXT = new string_id("meatlump/meatlump", "bomb_intro_text");
	public static final string_id TOOK_TOO_LONG = new string_id("meatlump/meatlump", "took_too_long");
	public static final string_id CURRENTLY_BEING_DEFUSED = new string_id("meatlump/meatlump", "currently_being_defused");
	public static final string_id YOU_FAILED = new string_id("meatlump/meatlump", "you_failed");
	public static final string_id YOU_CANCELED_EARLY = new string_id("meatlump/meatlump", "you_canceled_early");
	public static final string_id YOU_HAVE_DEBUFF = new string_id("meatlump/meatlump", "you_have_debuff");
	public static final string_id YOU_NEED_DEVICE = new string_id("meatlump/meatlump", "you_need_device");
	
	public static final int MAX_RANGE_TO_COLLECT = 3;
	public static final int DEFAULT_BOMB_TIMER = 65;
	public static final int BUFF_TIMER_INCREASE = 10;
	public static final int BUTTON_PENALTY = 5;
	public static final int INITIAL_BUTTON_NUMBER = 0;
	public static final int DEFUSE_TIME_OUT_SECONDS = 120;
	public static final String[] WIRE_LIST = 
	{
		"Power Source \\#FF3300+\\#FFFFFF ", 
		"Power Source \\#222222-\\#FFFFFF ", 
		"Explosive \\#FF3300+\\#FFFFFF ", 
		"Explosive \\#222222-\\#FFFFFF ", 
		"Detonator \\#FF3300+\\#FFFFFF ", 
		"Detonator \\#222222-\\#FFFFFF ", 
		"Initiation System \\#FF3300+\\#FFFFFF ", 
		"Initiation System \\#222222-\\#FFFFFF ", 
		"Tamper System \\#FF3300+\\#FFFFFF ", 
		"Tamper System \\#222222-\\#FFFFFF "
	};
	
	public static final String[] COLOR_LIST = 
	{
		"\\#FF3300Red\\#FFFFFF", 
		"\\#222222Black\\#FFFFFF", 
		"\\#996600Brown\\#FFFFFF", 
		"\\#FFFF00Yellow\\#FFFFFF", 
		"White", 
		"\\#99FF33Green\\#FFFFFF", 
		"\\#FF3300Red \\#222222Black\\#FFFFFF", 
		"\\#FFFF00Yellow \\#99FF33Green\\#FFFFFF", 
		"\\#FFFF00Yellow \\#996600Brown\\#FFFFFF", 
		"\\#222222Black \\#FFFFFFWhite"
	};
	
	public static final String[] CUT_LIST = 
	{
		"cut_red", 
		"cut_black", 
		"cut_brown", 
		"cut_yellow", 
		"cut_white", 
		"cut_green", 
		"cut_red_black", 
		"cut_yellow_green", 
		"cut_yellow_brown", 
		"cut_black_white"
	};
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		
		mi.addRootMenu(menu_info_types.ITEM_USE, SID_OPEN);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		obj_id collectionItem = self;
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, BEING_DEFUSED))
		{
			sendSystemMessage(player, CURRENTLY_BEING_DEFUSED);
			return SCRIPT_CONTINUE;
		}
		else if (!collection.canCollectCollectible(player, self))
		{
			return SCRIPT_CONTINUE;
		}
		else if (buff.hasBuff(player, "bomb_defuse_puzzle_downer"))
		{
			sendSystemMessage(player, YOU_HAVE_DEBUFF);
			return SCRIPT_CONTINUE;
		}
		else if (!consumable.decrementObjectInventoryOrEquipped(player, DEVICE_TEMPLATE, DEVICE_OBJVAR))
		{
			sendSystemMessage(player, YOU_NEED_DEVICE);
			return SCRIPT_CONTINUE;
		}
		
		closeOldWindow(player);
		blog("OnObjectMenuSelect");
		createBombUI(collectionItem, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean createBombUI(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		blog("createBombUI - init");
		if (!utils.hasScriptVar(player, CURRENTLY_DEFUSING))
		{
			blog("createBombUI - CURRENTLY_DEFUSING = false");
			boolean varsSet = initializePlayer(collectionItem, player);
			if (!varsSet)
			{
				return false;
			}
			
			utils.setScriptVar(player, CURRENTLY_DEFUSING, true);
		}
		else
		{
			closeOldWindow(player);
		}
		
		blog("createBombUI - Getting Bomb Data");
		String questionnaireText = getBombData(collectionItem, player);
		if (questionnaireText == null || questionnaireText.equals(""))
		{
			return false;
		}
		
		blog("createBombUI - CREATING SUI");
		
		dictionary params = new dictionary();
		int pid = createSUIPage("/Script.disarmBomb", collectionItem, player);
		setSUIAssociatedLocation(pid, collectionItem);
		setSUIMaxRangeToObject(pid, 8);
		params.put("callingPid", pid);
		sui.setPid(player, pid, PID_NAME);
		
		setSUIProperty(pid, "details.lblPrompt", "LocalText", questionnaireText);
		setSUIProperty(pid, "bg.caption.lblTitle", "Text", CAPTION);
		setSUIProperty(pid, "details.lblPrompt", "Editable", "false");
		setSUIProperty(pid, "details.lblPrompt", "GetsInput", "false");
		
		setSUIProperty(pid, "btnCancel", "Visible", "true");
		
		setSUIProperty(pid, "cut_red", "Visible", utils.getStringScriptVar(player, RED_WIRE));
		setSUIProperty(pid, "cut_green", "Visible", utils.getStringScriptVar(player, GREEN_WIRE));
		setSUIProperty(pid, "cut_yellow", "Visible", utils.getStringScriptVar(player, YELLOW_WIRE));
		setSUIProperty(pid, "cut_brown", "Visible", utils.getStringScriptVar(player, BROWN_WIRE));
		setSUIProperty(pid, "cut_white", "Visible", utils.getStringScriptVar(player, WHITE_WIRE));
		setSUIProperty(pid, "cut_yellow_brown", "Visible", utils.getStringScriptVar(player, YELLOW_BROWN_WIRE));
		setSUIProperty(pid, "cut_red_black", "Visible", utils.getStringScriptVar(player, RED_BLACK_WIRE));
		setSUIProperty(pid, "cut_yellow_green", "Visible", utils.getStringScriptVar(player, YELLOW_GREEN_WIRE));
		setSUIProperty(pid, "cut_black_white", "Visible", utils.getStringScriptVar(player, BLACK_WHITE_WIRE));
		setSUIProperty(pid, "cut_black", "Visible", utils.getStringScriptVar(player, BLACK_WIRE));
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_red", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_red", "cut_red", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_black", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_black", "cut_black", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_green", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_green", "cut_green", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_yellow", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_yellow", "cut_yellow", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_brown", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_brown", "cut_brown", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_yellow_brown", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_yellow_brown", "cut_yellow_brown", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_red_black", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_red_black", "cut_red_black", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_yellow_green", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_yellow_green", "cut_yellow_green", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_white", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_white", "cut_white", "LocalText");
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "cut_black_white", "handleDialogInput");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "cut_black_white", "cut_black_white", "LocalText");
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedCancel, "%button0%", "closeSui");
		
		showSUIPage(pid);
		flushSUIPage(pid);
		blog("createBombUI - FINISHED CREATING SUI");
		
		utils.setScriptVar(collectionItem, BEING_DEFUSED, player);
		utils.setScriptVar(player, TIME_DEFUSED, getGameTime());
		
		messageTo(collectionItem, "removeDefuseVar", params, DEFUSE_TIME_OUT_SECONDS, false);
		
		return true;
	}
	
	
	public boolean initializePlayer(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		
		boolean hasBuff = false;
		if (buff.hasBuff(player, BOMB_PUZZLE_BUFF))
		{
			hasBuff = true;
		}
		
		blog("HASBUFF: "+hasBuff);
		
		blog("initializePlayer - init");
		
		utils.setScriptVar(player, RED_WIRE, "true");
		utils.setScriptVar(player, GREEN_WIRE, "true");
		utils.setScriptVar(player, YELLOW_WIRE, "true");
		utils.setScriptVar(player, BROWN_WIRE, "true");
		utils.setScriptVar(player, WHITE_WIRE, "true");
		utils.setScriptVar(player, YELLOW_BROWN_WIRE, "true");
		utils.setScriptVar(player, RED_BLACK_WIRE, "true");
		utils.setScriptVar(player, YELLOW_GREEN_WIRE, "true");
		utils.setScriptVar(player, BLACK_WHITE_WIRE, "true");
		utils.setScriptVar(player, BLACK_WIRE, "true");
		
		Random rWire = new Random();
		String[] wireArray = new String[WIRE_LIST.length];
		System.arraycopy(WIRE_LIST, 0, wireArray, 0, WIRE_LIST.length);
		
		Random rColor = new Random();
		String[] colorArray = new String[COLOR_LIST.length];
		System.arraycopy(COLOR_LIST, 0, colorArray, 0, COLOR_LIST.length);
		String[] cutArray = new String[CUT_LIST.length];
		System.arraycopy(CUT_LIST, 0, cutArray, 0, CUT_LIST.length);
		
		for (int i=0; i<wireArray.length; i++)
		{
			testAbortScript();
			int randomPosition = rWire.nextInt(WIRE_LIST.length);
			String temp = wireArray[i];
			wireArray[i] = wireArray[randomPosition];
			wireArray[randomPosition] = temp;
		}
		
		for (int i=0; i<colorArray.length; i++)
		{
			testAbortScript();
			int randomPosition = rColor.nextInt(COLOR_LIST.length);
			String colorTemp = colorArray[i];
			String cutTemp = cutArray[i];
			colorArray[i] = colorArray[randomPosition];
			cutArray[i] = cutArray[randomPosition];
			colorArray[randomPosition] = colorTemp;
			cutArray[randomPosition] = cutTemp;
		}
		
		if (wireArray == null || colorArray == null || cutArray == null)
		{
			return false;
		}
		else if (wireArray.length != colorArray.length || wireArray.length != cutArray.length)
		{
			return false;
		}
		
		utils.setScriptVar(player, WIRE_ARRAY, wireArray);
		utils.setScriptVar(player, COLOR_ARRAY, colorArray);
		utils.setScriptVar(player, CUT_ARRAY, cutArray);
		utils.setScriptVar(player, BUTTON_NUMBER, INITIAL_BUTTON_NUMBER);
		
		int modifiedTimer = DEFAULT_BOMB_TIMER;
		if (hasBuff)
		{
			modifiedTimer += BUFF_TIMER_INCREASE;
		}
		
		utils.setScriptVar(player, PLAYER_MAX_TIMER, modifiedTimer);
		utils.setScriptVar(player, PLAYER_BUFF, hasBuff);
		
		return true;
	}
	
	
	public String getBombData(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return null;
		}
		else if (!utils.hasScriptVar(player, PLAYER_MAX_TIMER))
		{
			return null;
		}
		
		blog("getBombData - init");
		
		String questionnaireText = "Timer: ";
		
		int timer = utils.getIntScriptVar(player, PLAYER_MAX_TIMER);
		if (timer < 0)
		{
			return null;
		}
		else if (utils.hasScriptVar(player, BOMB_TIMER))
		{
			blog("getBombData - Timer already ticking");
			
			timer = utils.getIntScriptVar(player, BOMB_TIMER) - 1;
			utils.setScriptVar(player, BOMB_TIMER, timer);
			if (timer < 0)
			{
				blowUpBomb(collectionItem, player);
			}
		}
		else
		{
			utils.setScriptVar(player, BOMB_TIMER, timer);
		}
		
		questionnaireText += ""+ timer;
		
		questionnaireText += sui.newLine(2);
		questionnaireText += localize(BOMB_INTRO_TEXT);
		questionnaireText += sui.newLine(2);
		
		String[] wires = utils.getStringArrayScriptVar(player, WIRE_ARRAY);
		String[] colors = utils.getStringArrayScriptVar(player, COLOR_ARRAY);
		
		if (wires == null || colors == null)
		{
			return null;
		}
		else if (wires.length != colors.length)
		{
			return null;
		}
		
		for (int i=0; i<colors.length; i++)
		{
			testAbortScript();
			questionnaireText += ""+ (i+1) +". "+ wires[i] + " is "+ colors[i] + sui.newLine();
		}
		
		blog("getBombData - returning to caller");
		return questionnaireText;
		
	}
	
	
	public boolean blowUpBomb(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		else if (!isIdValid(collectionItem) || !exists(collectionItem))
		{
			return false;
		}
		
		if (buff.applyBuff(player, "bomb_defuse_puzzle_downer"))
		{;
		}
		sendSystemMessage(player, YOU_FAILED);
		
		dictionary params = new dictionary();
		params.put("player", player);
		
		blog("blowUpBomb - closing SUI");
		location loc = getLocation(collectionItem);
		playClientEffectLoc(collectionItem, "clienteffect/combat_explosion_lair_large.cef", loc, 0);
		
		setPosture(player, POSTURE_INCAPACITATED);
		int damageAmount = getAttrib(player, HEALTH) + 50;
		damage(player, DAMAGE_KINETIC, HIT_LOCATION_BODY, damageAmount);
		
		closeOldWindow(player);
		removePlayerVars(player);
		
		obj_id spawner = getObjIdObjVar(collectionItem, "spawn.spawner");
		if (isValidId(spawner) && exists(spawner))
		{
			messageTo(spawner, "handleTangibleRespawn", null, 30, false);
		}
		
		destroyObject(collectionItem);
		
		return true;
	}
	
	
	public int handleDialogInput(obj_id self, dictionary params) throws InterruptedException
	{
		blog("handleDialogInput - init");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleDialogInput - player validated");
		blog("handleDialogInput - params: "+params);
		
		boolean hasBuff = false;
		if (utils.hasScriptVar(player, PLAYER_BUFF))
		{
			hasBuff = utils.getBooleanScriptVar(player, PLAYER_BUFF);
		}
		
		String wire = "";
		if (params.getString("cut_red.LocalText") != null)
		{
			wire += "cut_red";
			if (hasBuff)
			{
				utils.setScriptVar(player, RED_WIRE, "false");
			}
		}
		else if (params.getString("cut_black.LocalText") != null)
		{
			wire += "cut_black";
			if (hasBuff)
			{
				utils.setScriptVar(player, BLACK_WIRE, "false");
			}
		}
		else if (params.getString("cut_green.LocalText") != null)
		{
			wire += "cut_green";
			if (hasBuff)
			{
				utils.setScriptVar(player, GREEN_WIRE, "false");
			}
		}
		else if (params.getString("cut_yellow.LocalText") != null)
		{
			wire += "cut_yellow";
			if (hasBuff)
			{
				utils.setScriptVar(player, YELLOW_WIRE, "false");
			}
		}
		else if (params.getString("cut_brown.LocalText") != null)
		{
			wire += "cut_brown";
			if (hasBuff)
			{
				utils.setScriptVar(player, BROWN_WIRE, "false");
			}
		}
		else if (params.getString("cut_yellow_brown.LocalText") != null)
		{
			wire += "cut_yellow_brown";
			if (hasBuff)
			{
				utils.setScriptVar(player, YELLOW_BROWN_WIRE, "false");
			}
		}
		else if (params.getString("cut_red_black.LocalText") != null)
		{
			wire += "cut_red_black";
			if (hasBuff)
			{
				utils.setScriptVar(player, RED_BLACK_WIRE, "false");
			}
		}
		else if (params.getString("cut_yellow_green.LocalText") != null)
		{
			wire += "cut_yellow_green";
			if (hasBuff)
			{
				utils.setScriptVar(player, YELLOW_GREEN_WIRE, "false");
			}
		}
		else if (params.getString("cut_white.LocalText") != null)
		{
			wire += "cut_white";
			if (hasBuff)
			{
				utils.setScriptVar(player, WHITE_WIRE, "false");
			}
		}
		else if (params.getString("cut_black_white.LocalText") != null)
		{
			wire += "cut_black_white";
			if (hasBuff)
			{
				utils.setScriptVar(player, BLACK_WHITE_WIRE, "false");
			}
		}
		
		if (wire == null || wire.equals(""))
		{
			blog("createBombUI - wire == null");
			messageTo(self, "closeSui", params, 0, false);
		}
		if (!utils.hasScriptVar(player, BUTTON_NUMBER))
		{
			return SCRIPT_CONTINUE;
		}
		
		int button = utils.getIntScriptVar(player, BUTTON_NUMBER);
		String[] cuts = utils.getStringArrayScriptVar(player, CUT_ARRAY);
		if (cuts == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!cuts[button].equals(wire))
		{
			blowUpBomb(self, player);
			return SCRIPT_CONTINUE;
		}
		
		if (button >= cuts.length - 1)
		{
			blog("createBombUI - PLAYER WINS");
			
			rewardPlayer(self, player);
			return SCRIPT_CONTINUE;
		}
		blog("createBombUI - DECREMENTING TIMER");
		
		int timer = utils.getIntScriptVar(player, BOMB_TIMER);
		timer = timer - BUTTON_PENALTY;
		utils.setScriptVar(player, BOMB_TIMER, timer);
		
		if (!utils.hasScriptVar(player, TIMER_RUNNING))
		{
			blog("createBombUI - Starting the TIMER");
			utils.setScriptVar(player, TIMER_RUNNING, true);
			messageTo(self, "refreshSuiBombTimerData", params, 1, false);
		}
		utils.setScriptVar(player, BUTTON_NUMBER, button + 1);
		blog("createBombUI - Re-calling createBombUI");
		createBombUI(self, player);
		blog("createBombUI - DONE Re-calling createBombUI");
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean rewardPlayer(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("rewardPlayer - init");
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectionItem) || !exists(player))
		{
			return false;
		}
		
		utils.removeScriptVar(collectionItem, BEING_DEFUSED);
		
		blog("rewardPlayer - validation complete");
		
		if (!collection.rewardPlayerCollectionSlot(player, collectionItem))
		{
			blog("rewardPlayer - rewardPlayerCollectionSlot = FAIL!!!");
			closeOldWindow(player);
			removePlayerVars(player);
			return false;
		}
		
		blog("rewardPlayer - rewardPlayerCollectionSlot = SUCCESS");
		closeOldWindow(player);
		removePlayerVars(player);
		
		return true;
	}
	
	
	public int refreshSuiBombTimerData(obj_id self, dictionary params) throws InterruptedException
	{
		blog("refreshSuiBombTimerData - init");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("refreshSuiBombTimerData - player id: "+player);
		
		if (!sui.hasPid(player, PID_NAME))
		{
			blog("refreshSuiBombTimerData - no PID");
			return SCRIPT_CONTINUE;
		}
		blog("refreshSuiBombTimerData - player has a pid scriptvar");
		int currentPid = sui.getPid(player, PID_NAME);
		blog("refreshSuiBombTimerData - currentPid: "+ currentPid);
		
		String bombData = getBombData(self, player);
		
		if (bombData == null || bombData.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		setSUIProperty(currentPid, "details.lblPrompt", "LocalText", bombData);
		flushSUIPage(currentPid);
		messageTo(self, "refreshSuiBombTimerData", params, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeDefuseVar(obj_id self, dictionary params) throws InterruptedException
	{
		blog("removeDefuseVar - init");
		if (utils.hasScriptVar(self, BEING_DEFUSED))
		{
			obj_id player = utils.getObjIdScriptVar(self, BEING_DEFUSED);
			if (utils.hasScriptVar(player, TIME_DEFUSED))
			{
				int playerTime = utils.getIntScriptVar(player, TIME_DEFUSED);
				int currentTime = getGameTime();
				blog("removeDefuseVar - playerTime: "+playerTime+" currentTime: "+currentTime+" playerTime - currentTime = "+(currentTime - playerTime));
				if ((currentTime - playerTime) < DEFUSE_TIME_OUT_SECONDS)
				{
					return SCRIPT_CONTINUE;
				}
			}
			blog("removeDefuseVar - valid player");
			
			if (utils.hasScriptVar(player, BOMB_TIMER) && utils.hasScriptVar(player, PLAYER_MAX_TIMER))
			{
				int timer = utils.getIntScriptVar(player, BOMB_TIMER);
				int maxtimer = utils.getIntScriptVar(player, PLAYER_MAX_TIMER);
				
				if (timer < maxtimer)
				{
					blog("removeDefuseVar - has bomb timer");
					return SCRIPT_CONTINUE;
				}
			}
			blog("closeSui player validated");
			blog("removeDefuseVar - has bomb timer");
			
			closeOldWindow(player);
			removePlayerVars(player);
			sendSystemMessage(player, TOOK_TOO_LONG);
		}
		
		blog("removeDefuseVar - removing scriptvar");
		
		utils.removeScriptVar(self, BEING_DEFUSED);
		return SCRIPT_CONTINUE;
	}
	
	
	public int closeSui(obj_id self, dictionary params) throws InterruptedException
	{
		blog("closeSui - init");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int maxTimer = utils.getIntScriptVar(player, PLAYER_MAX_TIMER);
		if (maxTimer < 0)
		{
			return SCRIPT_CONTINUE;
		}
		else if (utils.hasScriptVar(player, BOMB_TIMER))
		{
			int currentTimer = utils.getIntScriptVar(player, BOMB_TIMER);
			if (currentTimer < 0)
			{
				return SCRIPT_CONTINUE;
			}
			else if (currentTimer < maxTimer)
			{
				blowUpBomb(self, player);
				return SCRIPT_CONTINUE;
			}
		}
		
		if (utils.hasScriptVar(self, BEING_DEFUSED))
		{
			utils.removeScriptVar(self, BEING_DEFUSED);
		}
		
		if (buff.applyBuff(player, "bomb_defuse_puzzle_downer"))
		{;
		}
		{
			sendSystemMessage(player, YOU_CANCELED_EARLY);
		}
		
		blog("closeSui player validated");
		closeOldWindow(player);
		removePlayerVars(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public void closeOldWindow(obj_id player) throws InterruptedException
	{
		blog("closeOldWindow - init");
		
		int pid = sui.getPid(player, PID_NAME);
		blog("closeOldWindow - pid: "+pid);
		if (pid > -1)
		{
			blog("closeOldWindow - force closing: "+pid);
			forceCloseSUIPage(pid);
			sui.removePid(player, PID_NAME);
		}
	}
	
	
	public void removePlayerVars(obj_id player) throws InterruptedException
	{
		utils.removeScriptVarTree(player, VAR_PREFIX);
		utils.removeObjVar(player, VAR_PREFIX);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG("minigame",msg);
		}
		return true;
	}
}
