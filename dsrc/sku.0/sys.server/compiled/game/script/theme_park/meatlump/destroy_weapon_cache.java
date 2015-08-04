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


public class destroy_weapon_cache extends script.base_script
{
	public destroy_weapon_cache()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String VAR_PREFIX = "meatlump_weapon_cache";
	public static final String PID_NAME = VAR_PREFIX + ".pid";
	public static final String CURRENTLY_CALIBRATION = VAR_PREFIX + ".calibration";
	public static final String CALIBRATION_GOAL = VAR_PREFIX + ".goal";
	public static final String CALIBRATION_CURRENT = VAR_PREFIX + ".current";
	public static final String CALIBRATION_TRIES = VAR_PREFIX + ".tries";
	public static final String CALIBRATION_MAX_TRIES = VAR_PREFIX + ".max_tries";
	
	public static final String CAPTION = "Calibrate Power Cell Abatement";
	public static final String OBJVAR_SLOT_NAME = "collection.slotName";
	public static final String DEVICE_TEMPLATE = "object/tangible/meatlump/event/slicing_device_meatlump_weapon.iff";
	public static final String DEVICE_OBJVAR = "puzzle";
	public static final String WEAPON_PUZZLE_BUFF = "weapons_cache_puzzle_buff";
	
	public static final string_id SID_OPEN = new string_id("meatlump/meatlump", "weapon_calibration_use");
	public static final String CALIBRATION_ATTEMPTS_REMAINING = "@meatlump/meatlump:weapon_calibration_attempts_remaining";
	public static final String CALIBRATION_SUCCESS = "@meatlump/meatlump:weapon_calibration_success";
	public static final String CALIBRATION_FAILURE = "@meatlump/meatlump:weapon_calibration_failure";
	public static final String CALIBRATION_DESCRIPTION = "@meatlump/meatlump:weapon_calibration_description";
	
	public static final string_id MSG_CALIBRATION_ABORTED = new string_id("quest/force_sensitive/fs_crafting", "phase1_msg_calibration_aborted");
	public static final string_id YOU_FAILED = new string_id("meatlump/meatlump", "you_failed");
	public static final string_id YOU_CANCELED_EARLY = new string_id("meatlump/meatlump", "you_canceled_early");
	public static final string_id YOU_HAVE_DEBUFF = new string_id("meatlump/meatlump", "you_have_debuff");
	public static final string_id YOU_NEED_DEVICE = new string_id("meatlump/meatlump", "you_need_device");
	
	public static final int DEFAULT_TRIES = 10;
	public static final int BUFF_TRIES_INCREASE = 10;
	
	public static final String[] CONFIG_PLAYER_BUTTONS =  
	{
		"top.triangles.player.right.1",
		"top.triangles.player.right.2",
		"top.triangles.player.right.3",
		"top.triangles.player.left.2",
		"top.triangles.player.left.3",
		"top.triangles.player.left.1"
	};
	
	public static final String[] CONFIG_SERVER_BUTTONS =  
	{
		"top.triangles.server.right.1",
		"top.triangles.server.right.2",
		"top.triangles.server.right.3",
		"top.triangles.server.left.2",
		"top.triangles.server.left.3",
		"top.triangles.server.left.1"
	};
	
	public static final int[] DEFAULT_GOAL_CURRENT_ARRAY =  
	{
		0, 
		0, 
		0, 
		0, 
		0, 
		0
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
		else if (!collection.canCollectCollectible(player, self))
		{
			return SCRIPT_CONTINUE;
		}
		else if (buff.hasBuff(player, "weapons_cache_puzzle_downer"))
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
		createConfigUI(collectionItem, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean createConfigUI(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("createConfigUI - INIT");
		
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectionItem) || !exists(player))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		int pid = createSUIPage("/Script.calibration.game4", collectionItem, player);
		sui.setPid(player, pid, PID_NAME);
		setSUIAssociatedLocation(pid, collectionItem);
		setSUIMaxRangeToObject(pid, collection.MAX_RANGE_TO_COLLECT);
		params.put("callingPid", pid);
		
		blog("createConfigUI - sending to initializeCalibration");
		
		if (!initializeCalibration(collectionItem, player))
		{
			return false;
		}
		
		setSUIProperty(pid, "bg.caption.lbltitle", "Text", CAPTION);
		setSUIProperty(pid, "top.description.desc", "Text", CALIBRATION_DESCRIPTION);
		setSUIProperty(pid, "top.description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " 100%");
		
		int[] goal = utils.getIntArrayScriptVar(player, CALIBRATION_GOAL);
		if (goal == null)
		{
			return false;
		}
		
		blog("createConfigUI - goal.length: "+goal.length);
		
		for (int i = 0; i < goal.length; i++)
		{
			testAbortScript();
			if (goal[i] == 1)
			{
				setSUIProperty(pid, CONFIG_SERVER_BUTTONS[i], "Color", "#000000");
			}
			
			setSUIProperty(pid, CONFIG_SERVER_BUTTONS[i], "IsCancelButton", "false");
			setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "IsCancelButton", "false");
		}
		
		setSUIProperty(pid, "bg.mmc.close", "IsCancelButton", "true");
		
		for (int i = 0; i < CONFIG_PLAYER_BUTTONS.length; i++)
		{
			testAbortScript();
			subscribeToSUIEvent(pid, sui_event_type.SET_onButton, CONFIG_PLAYER_BUTTONS[i], "configProcessorPuzzleCallback");
		}
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedCancel, "%button0%", "closeSui");
		
		setSUIAssociatedObject(pid, player);
		setSUIMaxRangeToObject(pid, 10.0f);
		showSUIPage(pid);
		return true;
	}
	
	
	public boolean initializeCalibration(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("initializeCalibration - INIT");
		
		if (!isValidId(collectionItem) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectionItem) || !exists(player))
		{
			return false;
		}
		
		blog("initializeCalibration - creating stuff");
		
		int[] goal = new int[DEFAULT_GOAL_CURRENT_ARRAY.length];
		System.arraycopy(DEFAULT_GOAL_CURRENT_ARRAY, 0, goal, 0, DEFAULT_GOAL_CURRENT_ARRAY.length);
		int[] current = new int[DEFAULT_GOAL_CURRENT_ARRAY.length];
		System.arraycopy(DEFAULT_GOAL_CURRENT_ARRAY, 0, current, 0, DEFAULT_GOAL_CURRENT_ARRAY.length);
		int lastr = -1;
		int r = -1;
		boolean mixed = false;
		
		while (!mixed)
		{
			testAbortScript();
			
			for (int i = 0; i < DEFAULT_GOAL_CURRENT_ARRAY.length; i++)
			{
				testAbortScript();
				do
				{
					testAbortScript();
					r = rand(0, 5);
				}
				while(r == lastr);
				lastr = r;
				goal = toggleButton(goal, r);
				
			}
			
			for (int j = 0; j < DEFAULT_GOAL_CURRENT_ARRAY.length; j++)
			{
				testAbortScript();
				if (goal[j] != current[j])
				{
					mixed = true;
				}
			}
		}
		
		int calibrationTries = DEFAULT_TRIES;
		int calibrationMaxTries = DEFAULT_TRIES;
		
		if (buff.hasBuff(player, WEAPON_PUZZLE_BUFF))
		{
			calibrationTries += BUFF_TRIES_INCREASE;
			calibrationMaxTries += BUFF_TRIES_INCREASE;
			blog("HASBUFF: TRUE");
		}
		blog("HASBUFF?");
		
		blog("initializeCalibration - saving data");
		utils.setScriptVar(player, CALIBRATION_GOAL, goal);
		utils.setScriptVar(player, CALIBRATION_CURRENT, current);
		utils.setScriptVar(player, CALIBRATION_TRIES, calibrationTries);
		utils.setScriptVar(player, CALIBRATION_MAX_TRIES, calibrationMaxTries);
		return true;
	}
	
	
	public int[] toggleButton(int[] config, int button) throws InterruptedException
	{
		if (config == null || button < 0)
		{
			return null;
		}
		
		blog("toggleButton - init - BUTTON: "+button);
		
		int secondary1 = -1;
		int secondary2 = -1;
		
		switch(button) 
		{
			case 0: secondary1 = 3;
			secondary2 = 4;
			break;
			case 1: secondary1 = 4;
			secondary2 = 5;
			break;
			case 2: secondary1 = 3;
			secondary2 = 5;
			break;
			case 3: secondary1 = 0;
			secondary2 = 2;
			break;
			case 4: secondary1 = 0;
			secondary2 = 1;
			break;
			case 5: secondary1 = 1;
			secondary2 = 2;
			break;
		}
		
		if (secondary1 == -1 || secondary2 == -1)
		{
			return null;
		}
		blog("secondary1: "+secondary1);
		blog("secondary2: "+secondary2);
		
		for (int i = 0; i < config.length; i++)
		{
			testAbortScript();
			blog("config"+i+ " "+secondary2);
		}
		
		if (config[button] == 0)
		{
			config[button] = 1;
		}
		else
		{
			config[button] = 0;
		}
		
		if (config[secondary1] == 0)
		{
			config[secondary1] = 1;
		}
		else
		{
			config[secondary1] = 0;
		}
		
		if (config[secondary2] == 0)
		{
			config[secondary2] = 1;
		}
		else
		{
			config[secondary2] = 0;
		}
		
		blog("toggleButton - returning config");
		return config;
	}
	
	
	public int configProcessorPuzzleCallback(obj_id self, dictionary params) throws InterruptedException
	{
		blog("configProcessorPuzzleCallback - INIT");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		String widgetName = params.getString("eventWidgetName");
		blog("configProcessorPuzzleCallback widgetName - "+widgetName);
		
		int pid = sui.getPid(player, PID_NAME);
		if (pid <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("configProcessorPuzzleCallback pid - "+pid);
		
		if (widgetName == null || widgetName.equals(""))
		{
			blog("configProcessorPuzzleCallback widgetName = null ");
			if (buff.applyBuff(player, "weapons_cache_puzzle_downer"))
			{;
			}
			{
				sendSystemMessage(player, YOU_CANCELED_EARLY);
			}
			
			removePlayerVars(player);
			forceCloseSUIPage(pid);
			return SCRIPT_CONTINUE;
		}
		
		blog("configProcessorPuzzleCallback widget != null");
		
		int index = -1;
		
		for (int i = 0; i < CONFIG_PLAYER_BUTTONS.length; i++)
		{
			testAbortScript();
			setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "Color", "#FFFFFF");
			if (widgetName.equalsIgnoreCase(CONFIG_PLAYER_BUTTONS[i]))
			{
				index = i;
				break;
			}
		}
		
		blog("configProcessorPuzzleCallback index: "+index);
		if (index < 0 || index > 5)
		{
			return SCRIPT_CONTINUE;
		}
		
		int[] current = utils.getIntArrayScriptVar(player, CALIBRATION_CURRENT);
		int[] goal = utils.getIntArrayScriptVar(player, CALIBRATION_GOAL);
		int tries = utils.getIntScriptVar(player, CALIBRATION_TRIES);
		int max_tries = utils.getIntScriptVar(player, CALIBRATION_MAX_TRIES);
		
		if (current == null || goal == null)
		{
			blog("configProcessorPuzzleCallback current or goal == null");
			removePlayerVars(player);
			return SCRIPT_CONTINUE;
		}
		blog("configProcessorPuzzleCallback - About to toggle buttons");
		current = toggleButton(current, index);
		if (current == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < CONFIG_PLAYER_BUTTONS.length; i++)
		{
			testAbortScript();
			if (current[i] == 1)
			{
				blog("configProcessorPuzzleCallback setting to BLACK: "+CONFIG_PLAYER_BUTTONS[i]);
				setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "Color", "#000000");
			}
			else
			{
				setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "Color", "#FFFFFF");
			}
		}
		
		tries--;
		int integrity = (int)(((float)tries / (float)max_tries) * 100);
		blog("configProcessorPuzzleCallback integrity: "+integrity);
		boolean win = true;
		for (int i = 0; i < current.length; i++)
		{
			testAbortScript();
			if (current[i] != goal[i])
			{
				win = false;
			}
		}
		
		blog("configProcessorPuzzleCallback win: "+win);
		
		if (win)
		{
			blog("configProcessorPuzzleCallback THE WIN ");
			setSUIProperty(pid, "top.description.desc", "Text", CALIBRATION_SUCCESS);
			for (int i = 0; i < CONFIG_PLAYER_BUTTONS.length; i++)
			{
				testAbortScript();
				
				subscribeToSUIEvent(pid, sui_event_type.SET_onButton, CONFIG_PLAYER_BUTTONS[i], "noCallback");
				setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "GetsInput", "false");
			}
			
			rewardPlayer(self, player);
		}
		else if (tries <= 0)
		{
			if (buff.applyBuff(player, "weapons_cache_puzzle_downer"))
			{;
			}
			{
				sendSystemMessage(player, YOU_FAILED);
			}
			
			blog("configProcessorPuzzleCallback YOU LOSE ");
			setSUIProperty(pid, "top.description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " "+ integrity + "%");
			setSUIProperty(pid, "top.description.desc", "Text", CALIBRATION_FAILURE);
			for (int i = 0; i < CONFIG_PLAYER_BUTTONS.length; i++)
			{
				testAbortScript();
				
				subscribeToSUIEvent(pid, sui_event_type.SET_onButton, CONFIG_PLAYER_BUTTONS[i], "noCallback");
				setSUIProperty(pid, CONFIG_PLAYER_BUTTONS[i], "GetsInput", "false");
			}
			closeOldWindow(player);
			removePlayerVars(player);
		}
		else
		{
			blog("configProcessorPuzzleCallback DECREMENT ");
			
			setSUIProperty(pid, "top.description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " "+ integrity + "%");
		}
		
		utils.setScriptVar(player, CALIBRATION_CURRENT, current);
		utils.setScriptVar(player, CALIBRATION_TRIES, tries);
		
		flushSUIPage(pid);
		
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
	
	
	public int closeSui(obj_id self, dictionary params) throws InterruptedException
	{
		blog("closeSui - init");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (buff.applyBuff(player, "weapons_cache_puzzle_downer"))
		{;
		}
		{
			blog("closeSui applying weapon_cache_puzzle_downer");
			
			sendSystemMessage(player, YOU_CANCELED_EARLY);
		}
		
		blog("closeSui");
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
