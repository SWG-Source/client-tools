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


public class destroy_food_supplies extends script.base_script
{
	public destroy_food_supplies()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String VAR_PREFIX = "meatlump_food_supplies";
	public static final String PID_NAME = VAR_PREFIX + ".pid";
	public static final String CALIBRATION_GOAL = VAR_PREFIX + ".goal";
	public static final String CALIBRATION_TRIES = VAR_PREFIX + ".tries";
	public static final String CALIBRATION_MAX_TRIES = VAR_PREFIX + ".max_tries";
	
	public static final String CAPTION = "Mix Biological Yeast";
	public static final String DEVICE_TEMPLATE = "object/tangible/meatlump/event/slicing_device_meatlump_food.iff";
	public static final String DEVICE_OBJVAR = "puzzle";
	
	public static final string_id SID_OPEN = new string_id("meatlump/meatlump", "food_calibration_use");
	public static final string_id YOU_FAILED = new string_id("meatlump/meatlump", "you_failed");
	public static final string_id YOU_CANCELED_EARLY = new string_id("meatlump/meatlump", "you_canceled_early");
	public static final string_id YOU_HAVE_DEBUFF = new string_id("meatlump/meatlump", "you_have_debuff");
	public static final string_id YOU_NEED_DEVICE = new string_id("meatlump/meatlump", "you_need_device");
	
	public static final String CALIBRATION_ATTEMPTS_REMAINING = "@meatlump/meatlump:food_calibration_attempts_remaining";
	public static final String CALIBRATION_SUCCESS = "@meatlump/meatlump:food_calibration_success";
	public static final String CALIBRATION_FAILURE = "@meatlump/meatlump:food_calibration_failure";
	public static final String CALIBRATION_DESCRIPTION = "@meatlump/meatlump:food_calibration_description";
	public static final String SUI_SLIDER1 = "@meatlump/meatlump:food_calibration_slider1";
	public static final String SUI_SLIDER2 = "@meatlump/meatlump:food_calibration_slider2";
	public static final String SUI_SLIDER3 = "@meatlump/meatlump:food_calibration_slider3";
	public static final String FOOD_PUZZLE_BUFF = "food_supplies_puzzle_buff";
	
	public static final int DEFAULT_TRIES = 10;
	public static final int BUFF_TRIES_INCREASE = 10;
	
	public static final int[] DEFAULT_GOAL_CURRENT_ARRAY =  
	{
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
		else if (buff.hasBuff(player, "food_supplies_puzzle_downer"))
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
		yeastAdjustmentPuzzle(collectionItem, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean yeastAdjustmentPuzzle(obj_id collectionItem, obj_id player) throws InterruptedException
	{
		blog("gyroReceiverPuzzle - INIT");
		if (!isIdValid(collectionItem) || !exists(collectionItem))
		{
			return false;
		}
		else if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		dictionary params = new dictionary();
		int pid = createSUIPage("/Script.calibration.game3", collectionItem, player);
		sui.setPid(player, pid, PID_NAME);
		setSUIAssociatedLocation(pid, collectionItem);
		setSUIMaxRangeToObject(pid, collection.MAX_RANGE_TO_COLLECT);
		params.put("callingPid", pid);
		
		blog("gyroReceiverPuzzle - PID: "+pid);
		
		int[] goal = new int[DEFAULT_GOAL_CURRENT_ARRAY.length];
		System.arraycopy(DEFAULT_GOAL_CURRENT_ARRAY, 0, goal, 0, DEFAULT_GOAL_CURRENT_ARRAY.length);
		
		if (goal.length != DEFAULT_GOAL_CURRENT_ARRAY.length)
		{
			return false;
		}
		
		for (int i = 0; i < goal.length; i++)
		{
			testAbortScript();
			goal[i] = rand(0, 100);
		}
		
		int calibrationTries = DEFAULT_TRIES;
		int calibrationMaxTries = DEFAULT_TRIES;
		
		if (buff.hasBuff(player, FOOD_PUZZLE_BUFF))
		{
			calibrationTries += BUFF_TRIES_INCREASE;
			calibrationMaxTries += BUFF_TRIES_INCREASE;
			blog("HASBUFF: TRUE");
		}
		blog("HASBUFF?");
		
		utils.setScriptVar(collectionItem, CALIBRATION_GOAL, goal);
		utils.setScriptVar(collectionItem, CALIBRATION_TRIES, calibrationTries);
		utils.setScriptVar(collectionItem, CALIBRATION_MAX_TRIES, calibrationMaxTries);
		
		setSUIProperty(pid, "top.sliders.1.slider", "Value", "100");
		setSUIProperty(pid, "top.sliders.2.slider", "Value", "100");
		setSUIProperty(pid, "top.sliders.3.slider", "Value", "100");
		
		setSUIProperty(pid, "top.sliders.1.title", "Text", SUI_SLIDER1);
		setSUIProperty(pid, "top.sliders.2.title", "Text", SUI_SLIDER2);
		setSUIProperty(pid, "top.sliders.3.title", "Text", SUI_SLIDER3);
		
		setSUIProperty(pid, "bg.caption.lbltitle", "Text", CAPTION);
		setSUIProperty(pid, "description.desc", "Text", CALIBRATION_DESCRIPTION);
		setSUIProperty(pid, "description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " 100%");
		
		for (int i = 0; i < goal.length; i++)
		{
			testAbortScript();
			float pct = (float)goal[i] / 100.0f;
			int dec = (int)(255*pct);
			String hex = Integer.toHexString(dec);
			if (hex.length() == 1)
			{
				hex = "0"+ hex;
			}
			String hexValue = "#"+ hex + hex + hex;
			setSUIProperty(pid, "top.bars.server."+ (i+1), "Color", hexValue);
		}
		
		subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "%buttonOK%", "yeastReceiverPuzzleCallback");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "%buttonOK%", "top.sliders.1.slider", "Value");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "%buttonOK%", "top.sliders.2.slider", "Value");
		subscribeToSUIPropertyForEvent(pid, sui_event_type.SET_onButton, "%buttonOK%", "top.sliders.3.slider", "Value");
		subscribeToSUIEvent(pid, sui_event_type.SET_onClosedCancel, "%button0%", "closeSui");
		
		setSUIAssociatedObject(pid, player);
		setSUIMaxRangeToObject(pid, 10.0f);
		showSUIPage(pid);
		blog("gyroReceiverPuzzle - SUI Created");
		flushSUIPage(pid);
		return true;
	}
	
	
	public int yeastReceiverPuzzleCallback(obj_id self, dictionary params) throws InterruptedException
	{
		blog("gyroReceiverPuzzleCallback - INIT");
		
		String widgetName = params.getString("eventWidgetName");
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int pid = sui.getPid(player, PID_NAME);
		blog("gyroReceiverPuzzleCallback - PID: "+pid);
		if (widgetName.equalsIgnoreCase("btnCancel") || widgetName.equalsIgnoreCase(""))
		{
			if (buff.applyBuff(player, "food_supplies_puzzle_downer"))
			{;
			}
			{
				sendSystemMessage(player, YOU_CANCELED_EARLY);
			}
			blog("gyroReceiverPuzzleCallback - btnCancel");
			removePlayerVars(player);
			forceCloseSUIPage(pid);
			return SCRIPT_CONTINUE;
		}
		blog("gyroReceiverPuzzleCallback - buttonOK");
		
		int[] current = new int[DEFAULT_GOAL_CURRENT_ARRAY.length];
		System.arraycopy(DEFAULT_GOAL_CURRENT_ARRAY, 0, current, 0, DEFAULT_GOAL_CURRENT_ARRAY.length);
		
		int[] goal = utils.getIntArrayScriptVar(self, CALIBRATION_GOAL);
		int tries = utils.getIntScriptVar(self, CALIBRATION_TRIES);
		int max_tries = utils.getIntScriptVar(self, CALIBRATION_MAX_TRIES);
		
		boolean win = true;
		blog("gyroReceiverPuzzleCallback - loop start");
		
		for (int i = 0; i < current.length; i++)
		{
			testAbortScript();
			current[i] = utils.stringToInt(params.getString("top.sliders." + (i+1) + ".slider.Value"));
			int delta = goal[i] - current[i];
			if (delta < -5 || delta > 5)
			{
				win = false;
			}
			
			float pct = (float)current[i] / 100.0f;
			int dec = (int)(255*pct);
			String hex = Integer.toHexString(dec);
			if (hex.length() == 1)
			{
				hex = "0"+ hex;
			}
			String hexValue = "#"+ hex + hex + hex;
			setSUIProperty(pid, "top.bars.player."+ (i+1), "Color", hexValue);
		}
		blog("gyroReceiverPuzzleCallback - loop finish");
		
		tries--;
		int integrity = (int)(((float)tries / (float)max_tries) * 100);
		
		if (win)
		{
			blog("gyroReceiverPuzzleCallback - YOU WIN");
			rewardPlayer(self, player);
		}
		else if (tries <= 0)
		{
			blog("gyroReceiverPuzzleCallback - I LOSE");
			
			subscribeToSUIEvent(pid, sui_event_type.SET_onButton, "%buttonOK%", "noCallback");
			setSUIProperty(pid, "%buttonOK%", "Visible", "false");
			setSUIProperty(pid, "top.sliders.1.slider", "GetsInput", "false");
			setSUIProperty(pid, "top.sliders.2.slider", "GetsInput", "false");
			setSUIProperty(pid, "top.sliders.3.slider", "GetsInput", "false");
			setSUIProperty(pid, "description.desc", "Text", CALIBRATION_FAILURE);
			
			for (int i = 0; i < current.length; i++)
			{
				testAbortScript();
				
				float pct = (float)current[i] / 100.0f;
				int dec = (int)(255*pct);
				String hex = Integer.toHexString(dec);
				if (hex.length() == 1)
				{
					hex = "0"+ hex;
				}
				String hexValue = "#"+ hex + "0000";
				setSUIProperty(pid, "top.bars.player."+ (i+1), "Color", hexValue);
				setSUIProperty(pid, "top.bars.server."+ (i+1), "Color", hexValue);
			}
			setSUIProperty(pid, "description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " "+ integrity + "%");
			if (buff.applyBuff(player, "food_supplies_puzzle_downer"))
			{;
			}
			{
				sendSystemMessage(player, YOU_FAILED);
			}
			closeOldWindow(player);
			removePlayerVars(player);
		}
		else
		{
			blog("gyroReceiverPuzzleCallback - Try again");
			setSUIProperty(pid, "description.attempts", "Text", CALIBRATION_ATTEMPTS_REMAINING + " "+ integrity + "%");
		}
		
		utils.setScriptVar(self, CALIBRATION_TRIES, tries);
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
		
		if (buff.applyBuff(player, "food_supplies_puzzle_downer"))
		{;
		}
		{
			blog("closeSui applying food_supplies_puzzle_downer");
			
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
