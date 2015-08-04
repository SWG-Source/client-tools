package script.event;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.HashSet;
import script.library.collection;
import script.library.house_pet;
import script.library.prose;
import script.library.smuggler;
import script.library.static_item;
import script.library.space_utils;
import script.library.sui;
import script.library.utils;


public class dianoga_dumpster extends script.base_script
{
	public dianoga_dumpster()
	{
	}
	public static final boolean LOGGING_ON = false;
	public static final String LOGGING_CATEGORY = "sarlacc";
	
	public static final string_id SID_MENU_FEED = new string_id("sarlacc_minigame", "mnu_feed");
	public static final string_id SID_MENU_REWARD = new string_id("sarlacc_minigame", "mnu_reward_dianoga");
	public static final string_id SID_SIFT_SARLACC_BOX = new string_id("sarlacc_minigame", "must_sift_sarlacc_box_dianoga");
	public static final string_id SID_SARLACC_FED = new string_id("sarlacc_minigame", "dianoga_fed");
	public static final string_id SID_FOUND_REWARD = new string_id("sarlacc_minigame", "sifted_reward");
	public static final string_id SID_SARLACC_NOTHING_FOUND = new string_id("sarlacc_minigame", "no_junk_inventory_dianoga");
	public static final string_id SID_VERIFICATION_MSG = new string_id("sarlacc_minigame", "verify_consumption_dianoga");
	public static final string_id SID_ITEM_NOT_FED = new string_id("sarlacc_minigame", "item_not_fed_dianoga");
	public static final string_id SID_ITEM_NOT_BIOLINK_FED = new string_id("sarlacc_minigame", "item_not_biolink_fed_dianoga");
	public static final string_id SID_ITEM_NOT_INVENTORY = new string_id("sarlacc_minigame", "item_not_inventory");
	public static final string_id SID_ITEM_EQUIPPED = new string_id("sarlacc_minigame", "item_not_equipped_dianoga");
	public static final string_id SID_ITEM_NOT_VALID = new string_id("sarlacc_minigame", "item_not_valid_dianoga");
	public static final string_id SID_OUT_OF_RANGE = new string_id("sarlacc_minigame", "out_of_range_dianoga");
	public static final string_id SID_QUEST_ITEM = new string_id("sarlacc_minigame", "item_quest_item_dianoga");
	public static final string_id SID_MENU_GOD_FEED = new string_id("sarlacc_minigame", "use_cs_tool");
	public static final string_id SID_GOD_FEED_MESSAGE = new string_id("sarlacc_minigame", "god_feed_message_dianoga");
	public static final string_id SID_GOD_FEED_NO_FOOD = new string_id("sarlacc_minigame", "god_no_food_message_dianoga");
	public static final string_id SID_FEEDING_DUE_TO_GODMODE = new string_id("sarlacc_minigame", "fed_due_to_godmode_dianoga");
	public static final string_id SID_GOD_ALLOW_PLAYER_SIFT = new string_id("sarlacc_minigame", "god_allow_player_sift_dianoga");
	public static final string_id SID_SOUND_ON = new string_id("sarlacc_minigame", "dianoga_sound_on");
	public static final string_id SID_SOUND_OFF = new string_id("sarlacc_minigame", "dianoga_sound_off");
	public static final string_id SID_MENU_SOUND_OFF = new string_id("sarlacc_minigame", "dianoga_sound_menu_off");
	public static final string_id SID_MENU_SOUND_ON = new string_id("sarlacc_minigame", "dianoga_sound_menu_on");
	
	public static final String FEED_TITLE = "@sarlacc_minigame:edible_title_dianoga";
	public static final String FEED_PROMPT = "@sarlacc_minigame:edible_prompt_dianoga";
	public static final String BTN_SELL = "@sarlacc_minigame:btn_feed";
	
	public static final String SOUND_SIFT = "sound/sarlacc_drawer_open.snd";
	public static final String SOUND_CUTE = "sound/dianoga_dumpster.snd";
	
	public static final String ANIMATON_GROWL = "vocalize";
	public static final String ANIMATON_FEED = "eat";
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		customerSvcLogVerbose(self, "Dianoga: "+self+" is being destroyed.");
		
		house_pet.stopPlayingMusic(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setUpFirstTime(self);
		messageTo(self, "checkTriggerVolume", null, 2, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if (!isValidId(getOwner(self)) && utils.isNestedWithinAPlayer(self))
		{
			house_pet.setObjectOwner(self);
			return SCRIPT_CONTINUE;
		}
		blog("Sarlacc - OnObjectMenuSelect - init");
		if (hasObjVar(self, house_pet.SARLACC_LAST_FED))
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" has been initialized. Attempting to update Dianoga data.");
			if (!updateSarlaccPet(self))
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" could not be updated properly or the update attempt was too soon.");
			}
		}
		
		messageTo(self, "checkTriggerVolume", null, 2, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTransferred(obj_id self, obj_id sourceContainer, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		if (!isValidId(getOwner(self)))
		{
			house_pet.setObjectOwner(self);
		}
		
		if (!house_pet.validateNpcPlacementInStructure(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!utils.isInHouseCellSpace(self) && !space_utils.isNestedWithinPobShip(self))
		{
			return SCRIPT_CONTINUE;
		}
		if (utils.isNestedWithin(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isValidId(getOwner(self)))
		{
			house_pet.setObjectOwner(self);
		}
		
		if (player != getOwner(self) && !isGod(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		mi.addRootMenu(menu_info_types.EXAMINE, new string_id("ui_radial", "examine"));
		
		if (hasObjVar(self, house_pet.NO_SARLACC_SOUND))
		{
			mi.addRootMenu(menu_info_types.SERVER_MENU4, SID_MENU_SOUND_ON);
		}
		else
		{
			mi.addRootMenu(menu_info_types.SERVER_MENU4, SID_MENU_SOUND_OFF);
		}
		
		if (isGod(player))
		{
			if (!hasObjVar(self, house_pet.SARLACC_REWARD_AVAILABLE))
			{
				mi.addRootMenu(menu_info_types.SERVER_MENU3, SID_MENU_GOD_FEED);
			}
			else
			{
				sendSystemMessage(player, SID_GOD_ALLOW_PLAYER_SIFT);
			}
			return SCRIPT_CONTINUE;
		}
		
		else if (!isGod(player) && hasObjVar(self, house_pet.SARLACC_REWARD_AVAILABLE))
		{
			mi.addRootMenu(menu_info_types.SERVER_MENU2, SID_MENU_REWARD);
			return SCRIPT_CONTINUE;
		}
		
		else if (getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE) >= house_pet.SARLACC_HUNGRY)
		{
			mi.addRootMenu(menu_info_types.SERVER_MENU1, SID_MENU_FEED);
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean setUpFirstTime(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		setObjVar(self, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_STARVING);
		setObjVar(self, house_pet.SARLACC_BORN, getCalendarTime());
		setObjVar(self, house_pet.SARLACC_PHASE_START, getCalendarTime());
		setObjVar(self, house_pet.SARLACC_FEEDING_ITERATION, 0);
		
		if (!utils.isNestedWithinAPlayer(self) && !hasObjVar(self, house_pet.CHILD_OBJ_ID))
		{
			messageTo(self, "createSarlaccPet", null, 0, false);
		}
		
		customerSvcLogVerbose(self, "Dianoga: "+self+" has been set up for the very first time.");
		return true;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		blog("Sarlacc - OnObjectMenuSelect - init");
		if (utils.isNestedWithin(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (player != getOwner(self) && !isGod(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, house_pet.SARLACC_LAST_FED))
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" has been feed previously and used via radial option. Attempting to update Dianoga data.");
			if (!updateSarlaccPet(self))
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" could not be updated properly or the update attempt was too soon.");
			}
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			if (getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE) < house_pet.SARLACC_HUNGRY)
			{
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" has selected the feed option via radial.");
			
			utils.removeScriptVar(self, house_pet.SARLACC_AVOID_REPEATED_UPDATES);
			if (!updateSarlaccPet(self))
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" could not be updated properly or the update attempt was too soon.");
			}
			
			getEdibleContents(player, self);
			sendDirtyObjectMenuNotification(self);
			
			return SCRIPT_CONTINUE;
		}
		else if (item == menu_info_types.SERVER_MENU2)
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" has a reward for the owner: "+player+" and the owner has selected the reward option via radial.");
			
			utils.removeScriptVar(self, house_pet.SARLACC_AVOID_REPEATED_UPDATES);
			if (!updateSarlaccPet(self))
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" could not be updated properly or the update attempt was too soon.");
			}
			
			getPlayerReward(player, self);
			sendDirtyObjectMenuNotification(self);
			return SCRIPT_CONTINUE;
		}
		else if (item == menu_info_types.SERVER_MENU3)
		{
			if (!isGod(player))
			{
				return SCRIPT_CONTINUE;
			}
			
			sendSystemMessage(player, SID_GOD_FEED_MESSAGE);
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is being fed by CS or God Player: "+player+".");
			
			utils.removeScriptVar(self, house_pet.SARLACC_AVOID_REPEATED_UPDATES);
			
			if (!getEdibleContents(player, self))
			{
				sendSystemMessage(player, SID_GOD_FEED_NO_FOOD);
			}
			sendDirtyObjectMenuNotification(self);
			return SCRIPT_CONTINUE;
		}
		else if (item == menu_info_types.SERVER_MENU4)
		{
			if (hasObjVar(self, house_pet.NO_SARLACC_SOUND))
			{
				removeObjVar(self, house_pet.NO_SARLACC_SOUND);
				sendSystemMessage(player, SID_SOUND_ON);
			}
			else
			{
				setObjVar(self, house_pet.NO_SARLACC_SOUND, true);
				sendSystemMessage(player, SID_SOUND_OFF);
			}
		}
		sendDirtyObjectMenuNotification(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (hasObjVar(self, house_pet.SARLACC_CURRENT_PHASE))
		{
			int currentStatus = getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE);
			if (currentStatus < house_pet.SARLACC_FED || currentStatus > house_pet.SARLACC_DEATH)
			{
				blog("getSarlaccCollectionColumn currentStatus : "+currentStatus);
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			names[idx] = "status";
			attribs[idx] = "@obj_attr_n:" + house_pet.SARLACC_PHASES[currentStatus];
			idx++;
			
			if (hasObjVar(self, house_pet.SARLACC_LAST_FED))
			{
				int lastFed = getIntObjVar(self, house_pet.SARLACC_LAST_FED);
				if (lastFed < 0)
				{
					return super.OnGetAttributes(self, player, names, attribs);
				}
				
				names[idx] = "last_fed";
				attribs[idx] = getCalendarTimeStringLocal(lastFed);
				idx++;
				
				int succesFeeds = getIntObjVar(self, house_pet.SARLACC_FEEDING_ITERATION);
				if (succesFeeds > 0)
				{
					names[idx] = "times_fed";
					attribs[idx] = ""+succesFeeds;
					idx++;
				}
				
				names[idx] = "next_feed";
				if (currentStatus == 0)
				{
					attribs[idx] = getCalendarTimeStringLocal(lastFed + house_pet.getUpdateWeekly(self));
				}
				else if (currentStatus == 1)
				{
					attribs[idx] = "Now";
				}
				else
				{
					attribs[idx] = "Hopefully Soon";
				}
				
				idx++;
				
				names[idx] = "time_now";
				attribs[idx] = getCalendarTimeStringLocal(getCalendarTime());
				idx++;
			}
			else
			{
				
				names[idx] = "last_fed";
				attribs[idx] = "Never";
				idx++;
				
				names[idx] = "next_feed";
				attribs[idx] = "Hopefully Soon";
				idx++;
			}
		}
		
		if (hasObjVar(self, house_pet.SARLACC_BORN))
		{
			int birthDay = getIntObjVar(self, house_pet.SARLACC_BORN);
			if (birthDay <= 0)
			{
				return super.OnGetAttributes(self, player, names, attribs);
			}
			
			names[idx] = "born";
			attribs[idx] = getCalendarTimeStringLocal(birthDay);
			idx++;
		}
		
		if (isValidId(getOwner(self)))
		{
			obj_id owner = getOwner(self);
			
			names[idx] = "owner";
			attribs[idx] = getPlayerFullName(owner);
			idx++;
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(breacher) || !isPlayer(breacher))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(breacher, "breathe"))
		{
			if (utils.getObjIdScriptVar(breacher, "breathe") == self)
			{
				return SCRIPT_CONTINUE;
			}
			
		}
		if (breacher == getOwner(self))
		{
			dictionary params = new dictionary();
			params.put("player", breacher);
			
			if (!hasObjVar(self, house_pet.NO_SARLACC_SOUND))
			{
				messageTo(self, "playCute", params, 1, false);
			}
			
			messageTo(self, "growlAnimation", params, 0, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id breacher) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!isIdValid(breacher) || !isPlayer(breacher))
		{
			return SCRIPT_CONTINUE;
		}
		stopClientEffectObjByLabel(breacher, self, house_pet.PET_SOUND_LABEL);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSarlaccEatSui(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("handleSarlaccEatSui - Init");
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (sui.hasPid(player, "confirm_sui"))
		{
			int pid = sui.getPid(player, "confirm_sui");
			forceCloseSUIPage(pid);
		}
		
		int idx = sui.getListboxSelectedRow(params);
		int bp = sui.getIntButtonPressed(params);
		if (idx < 0)
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" selected an invalid option in the list of edibles.");
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		if (bp == sui.BP_CANCEL)
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" selected cancel.");
			
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		else
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" selected a valid edible item.");
			
			obj_id[] junkFood = utils.getObjIdBatchScriptVar(player, house_pet.EDIBLES_IDS);
			
			if ((junkFood == null) || (junkFood.length == 0))
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" selected something but the list of items came back corrupt or invalid.");
				cleanupSui(player);
				return SCRIPT_CONTINUE;
			}
			
			if (idx > junkFood.length - 1)
			{
				CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" selected an invalid option in the list of edibles.");
				cleanupSui(player);
				return SCRIPT_CONTINUE;
			}
			blog("verifySarlaccEatSui: Setting junk item script var: "+junkFood[idx]);
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" has selected item: "+junkFood[idx]+" name: "+getName(junkFood[idx])+" from the UI list.");
			
			utils.setScriptVar(player, "junkfood.junk_item", junkFood[idx]);
			blog("verifySarlaccEatSui: ScriptVar: "+utils.getObjIdScriptVar(player, "junkfood.junk_item"));
			int pid = sui.msgbox(self, player, utils.packStringId(SID_VERIFICATION_MSG), sui.YES_NO, "verifySarlaccEatSui");
			sui.setPid(player, pid, "confirm_sui");
		}
		cleanupSui(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int verifySarlaccEatSui(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("verifySarlaccEatSui: Init.");
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player) || !exists(player))
		{
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		if (bp == sui.BP_CANCEL)
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" has cancelled the feeding via the verificaiton UI.");
			blog("verifySarlaccEatSui: Selected Cancel or No.");
			return SCRIPT_CONTINUE;
		}
		
		blog("verifySarlaccEatSui: Getting junk item from script var.");
		if (!utils.hasScriptVar(player, "junkfood.junk_item"))
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" somehow lost their item variable needed to feeding the sarlacc.");
			blog("verifySarlaccEatSui: No Junk item Found");
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		obj_id junkFood = utils.getObjIdScriptVar(player, "junkfood.junk_item");
		blog("verifySarlaccEatSui: Junk item: "+junkFood);
		
		if (!isValidId(junkFood) || !exists(junkFood))
		{
			CustomerServiceLog("sarlacc_minigame: ","Dianoga: "+self+" is hungry and the owner: "+player+" somehow deleted the item he was about to feed to the sarlacc.");
			
			blog("verifySarlaccEatSui: Junk item Invalid..bailing out");
			
			sendSystemMessage(player, SID_ITEM_NOT_VALID);
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		blog("verifySarlaccEatSui: Junk Confirmed, eating.");
		
		if (!eatJunkItem(self, player, junkFood))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(player) + " OID: "+ player + ") FAILED TO FEED Dianoga: " +self+" item ("+ getName(junkFood) + " OID: "+ junkFood + ").");
		}
		
		cleanupSui(player);
		return SCRIPT_CONTINUE;
	}
	
	
	public int growlAnimation(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params == null || params.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction(self, ANIMATON_GROWL);
		return SCRIPT_CONTINUE;
	}
	
	
	public int playCute(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params == null || params.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, house_pet.NO_SARLACC_SOUND))
		{
			return SCRIPT_CONTINUE;
		}
		
		play2dNonLoopingSound(player, SOUND_CUTE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkTriggerVolume(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasTriggerVolume(self, house_pet.PET_TRIG_VOLUME))
		{
			float range = 2f;
			createTriggerVolume(house_pet.PET_TRIG_VOLUME, range, true);
			setObjVar(self, house_pet.MUSIC_RANGE, range);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeRepeatAvoidanceScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("sarlacc_minigame: ","Sarlacc - removeRepeatAvoidanceScriptVar removing lockout scriptvar");
		
		utils.removeScriptVar(self, house_pet.SARLACC_AVOID_REPEATED_UPDATES);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean updateSarlaccPet(obj_id sarlacc) throws InterruptedException
	{
		if (!isValidId(sarlacc) || !exists(sarlacc))
		{
			return false;
		}
		
		blog("updateSarlaccPet - RECEIVED Update request - Current phase: "+getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE)+" time: "+ getCalendarTime());
		if (utils.hasScriptVar(sarlacc, house_pet.SARLACC_AVOID_REPEATED_UPDATES))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" avoiding repeated calls to this function. Bailing early.");
			return false;
		}
		else
		{
			utils.setScriptVar(sarlacc, house_pet.SARLACC_AVOID_REPEATED_UPDATES, true);
			messageTo(sarlacc, "removeRepeatAvoidanceScriptVar", null, 60, false);
		}
		
		customerSvcLogVerbose(sarlacc, "updateSarlaccPet - RECEIVED MESSAGE TO UPDATE Dianoga: "+sarlacc+" at: time: "+ getCalendarTime());
		
		if (!hasObjVar(sarlacc, house_pet.SARLACC_LAST_FED))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has never been fed. Exiting updateSarlaccPet.");
			blog("updateSarlaccPet - no last feed time");
			return false;
		}
		else
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has been either been fed previously or has a valid empty variable. Continuing debug procedures.");
		}
		
		if (!hasObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has NO CURRENT PHASE VAR. Exiting updateSarlaccPet.");
			blog("updateSarlaccPet - no phase for sarlacc");
			return false;
		}
		else
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has a current phase, continuing debug procedures.");
		}
		
		if (!hasObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has NO FEEDING ITERATION VAR. Exiting updateSarlaccPet.");
			blog("updateSarlaccPet - no feeding iteration for sarlacc");
			return false;
		}
		else
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has a feeding iteration, continuing debug procedures.");
		}
		
		int currentPhase = getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE);
		
		if (currentPhase == house_pet.SARLACC_DEATH)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" we haven't even started updating yet and the Dianoga is currently DEAD.");
		}
		else if (currentPhase == house_pet.SARLACC_STARVING)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" we haven't even started updating yet and the Dianoga is currently STARVING.");
		}
		else if (currentPhase == house_pet.SARLACC_HUNGRY)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" we haven't even started updating yet and the Dianoga is currently HUNGRY.");
		}
		else if (currentPhase == house_pet.SARLACC_FED)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" we haven't even started updating yet and the Dianoga is currently FED.");
		}
		
		if (getIntObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION) >= house_pet.SARLACC_PHASE_WEEKS)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is currently dead.");
			String collectionCol = getSarlaccCollectionColumn(sarlacc);
			if (collectionCol == null || collectionCol.equals(""))
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" reward is about to be issued but a correct column was not returned. Setting reward to pick from generic_collectible");
				blog("updateSarlaccPet - getSarlaccCollectionColumn FAILED!");
				collectionCol = "generic_collectible";
			}
			else
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" reward is about to be issued. The random column returned was:"+collectionCol);
			}
			
			blog("updateSarlaccPet - getSarlaccCollectionColumn: "+collectionCol);
			customerSvcLogVerbose(sarlacc, "updateSarlaccPet - Collection column received. We are going to reset the Feeding iteration, Feeding list and set the Dianoga to reward the player. Before we do this, lets get verbose data. ");
			
			setObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION, 0);
			
			removeObjVar(sarlacc, house_pet.SARLACC_FEED_ARRAY);
			
			setObjVar(sarlacc, house_pet.SARLACC_REWARD_AVAILABLE, true);
			
			setObjVar(sarlacc, house_pet.SARLACC_REWARD_COLUMN, collectionCol);
			
			updateSarlaccPet(sarlacc);
			return true;
		}
		
		int currentGameTime = getCalendarTime();
		int updateWeek = house_pet.getUpdateWeekly(sarlacc);
		int updateDay = house_pet.getUpdateDaily(sarlacc);
		int currentIteration = getIntObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION);
		int lastFed = getIntObjVar(sarlacc, house_pet.SARLACC_LAST_FED);
		
		if (currentPhase >= house_pet.SARLACC_DEATH)
		{
			
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is currently dead.");
			setObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_DEATH);
			return false;
		}
		
		CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is not dead. We are continuing the updated.");
		
		if (currentPhase < house_pet.SARLACC_FED)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" current phase was -1.");
			blog("updateSarlaccPet - currentPhase WAS -1");
			currentPhase = house_pet.SARLACC_FED;
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR))
		{
			int verVar = getIntObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR);
			if (verVar != house_pet.SARLACC_VERSION)
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has no version number so the Dianoga is being stamped for the first time. The Dianoga also gets the free chance variable (or it resets this variable).");
				setObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR, house_pet.SARLACC_VERSION);
				setObjVar(sarlacc, house_pet.SARLACC_FREE_CHANCE, false);
			}
		}
		else
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has no version number so the Dianoga is being stamped for the first time. ");
			setObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR, house_pet.SARLACC_VERSION);
		}
		
		if (currentGameTime < (lastFed + updateWeek))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has received an update before the request too soon. Bailing out.");
			blog("updateSarlaccPet - Last Fed Timer: "+(lastFed + updateWeek)+" currentGameTime: "+currentGameTime);
			return false;
		}
		
		CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - It has been at least a week since the Dianoga: "+sarlacc+" has had food.");
		
		if (currentGameTime < (lastFed + (updateWeek * 2)))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is supposed to be hungry but has time before being Very Hungry. We are going to check the current phase.");
			
			if (currentPhase != house_pet.SARLACC_HUNGRY)
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" had a bugged current phase of: "+currentPhase+" so we will correct that now.");
				setObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_HUNGRY);
			}
			else
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" had a bugged current phase is Hungry which is correct.");
			}
			
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is currently hungry but still has time before moving to very hungry. Bailing out.");
			return false;
		}
		
		if ((currentGameTime >= (lastFed + updateWeek) && currentGameTime < (lastFed + (updateWeek * 2))))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" was phase "+currentPhase+" and is now being set to HUNGRY.");
			setObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_HUNGRY);
			return true;
		}
		
		CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has been neglected by owner. Checking for penalties.");
		
		if (currentGameTime >= (lastFed + (updateWeek * 2)) && (!hasObjVar(sarlacc, house_pet.SARLACC_FREE_CHANCE) || !getBooleanObjVar(sarlacc, house_pet.SARLACC_FREE_CHANCE)))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - It has been at least 2 weeks since the Dianoga: "+sarlacc+" has had food but the player hasn't used up their free chance yet. We'll take care of this when the player feed this sarlacc.");
			
			setObjVar(sarlacc, house_pet.SARLACC_PENALTY_VAR, true);
			setObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_HUNGRY);
			
			return false;
		}
		
		CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" has been neglected by owner and the free chance has been used.");
		
		if (currentGameTime >= (lastFed + (updateWeek * 3)))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - It has been at least 2 weeks since the Dianoga: "+sarlacc+" has had food, but less than 3 weeks.");
			currentPhase += 3;
		}
		
		else if (currentGameTime >= (lastFed + (updateWeek * 2)))
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - It has been at least 2 weeks since the Dianoga: "+sarlacc+" has had food, but less than 3 weeks.");
			currentPhase += 2;
		}
		
		if (currentPhase >= house_pet.SARLACC_DEATH)
		{
			
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" was neglected and is currently dead (Starving).");
			currentPhase = house_pet.SARLACC_DEATH;
		}
		
		if (currentPhase >= house_pet.SARLACC_STARVING)
		{
			CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" is about to be set to have data removed because phase is: "+currentPhase);
			
			if (!setSarlaccStarvingOrDead(sarlacc, currentPhase))
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" could not remove data. This is a big error that needs to be reported to SWG Development. Phase is: "+currentPhase);
				return false;
			}
		}
		return true;
	}
	
	
	public boolean getEdibleContents(obj_id player, obj_id sarlacc) throws InterruptedException
	{
		blog("getEdibleContents - Init");
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (!isValidId(sarlacc) || !exists(sarlacc))
		{
			return false;
		}
		
		if (sui.hasPid(player, house_pet.EDIBLES_SUI))
		{
			int pid = sui.getPid(player, house_pet.EDIBLES_SUI);
			forceCloseSUIPage(pid);
		}
		if (sui.hasPid(player, "confirm_sui"))
		{
			int pid = sui.getPid(player, "confirm_sui");
			forceCloseSUIPage(pid);
		}
		
		if (!isGod(player))
		{
			
			if (getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE) < house_pet.SARLACC_HUNGRY)
			{
				return false;
			}
		}
		else
		{
			if (getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE) < house_pet.SARLACC_HUNGRY)
			{
				CustomerServiceLog("sarlacc_minigame: ","updateSarlaccPet - Dianoga: "+sarlacc+" was fed, even though the Dianoga was not hungry because the CS Representative: "+player+" did so intentionally while in godmode");
				sendSystemMessage(player, SID_FEEDING_DUE_TO_GODMODE);
			}
		}
		
		if (getBooleanObjVar(sarlacc, house_pet.SARLACC_REWARD_AVAILABLE))
		{
			sendSystemMessage(player, SID_SIFT_SARLACC_BOX);
			return false;
		}
		blog("getEdibleContents - About to create array of junk");
		
		if (!checkPlayerInventoryForSarlaccFood(player))
		{
			sendSystemMessage(player, SID_SARLACC_NOTHING_FOUND);
			return false;
		}
		
		blog("getEdibleContents - Player has junk");
		
		obj_id[] edibleItems = smuggler.getAllJunkItems(player);
		if (edibleItems == null && edibleItems.length <= 0)
		{
			return false;
		}
		
		String[] edibleMenu = new String[edibleItems.length];
		
		for (int i = 0; i < edibleItems.length; i++)
		{
			testAbortScript();
			
			String name = getAssignedName(edibleItems[i]);
			if (name.equals("") || name == null)
			{
				name = getString(getNameStringId(edibleItems[i]));
			}
			edibleMenu[i] = name;
		}
		
		if (edibleMenu == null && edibleMenu.length <= 0)
		{
			return false;
		}
		
		int pid = sui.listbox(sarlacc, player, FEED_PROMPT, sui.YES_NO, FEED_TITLE, edibleMenu, "handleSarlaccEatSui", false, false);
		
		if (pid > -1)
		{
			
			setSUIProperty(pid, sui.LISTBOX_BTN_OK, sui.PROP_TEXT, BTN_SELL);
			
			showSUIPage(pid);
			
			utils.setBatchScriptVar(player, house_pet.EDIBLES_IDS, edibleItems);
			sui.setPid(player, pid, house_pet.EDIBLES_SUI);
		}
		return true;
	}
	
	
	public boolean eatJunkItem(obj_id self, obj_id player, obj_id item) throws InterruptedException
	{
		blog("eatJunkItem - Init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return false;
		}
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		CustomerServiceLog("sarlacc_minigame: ", "eatJunkItem - Player ( OID: "+ player + ") has fed Dianoga item ("+ item + "). Processing");
		
		if (!isIdValid(item) || !exists(item))
		{
			
			sendSystemMessage(player, SID_ITEM_NOT_VALID);
			return false;
		}
		
		obj_id bioLink = getBioLink(item);
		if (isIdValid(bioLink) && utils.stringToLong(bioLink.toString()) != 1)
		{
			
			sendSystemMessage(player, SID_ITEM_NOT_BIOLINK_FED);
			return false;
		}
		
		if (utils.outOfRange(self, player, 10.0f, true))
		{
			
			sendSystemMessage(player, SID_OUT_OF_RANGE);
			return false;
		}
		
		if (!validateItemForSarlacc(player, item))
		{
			return false;
		}
		
		String playerName = getName(player);
		String itemName = getEncodedName(item);
		CustomerServiceLog("sarlacc_minigame: ", "Player ("+ playerName + " OID: "+ player + ") fed Dianoga item ("+ itemName + " OID: "+ item + ").");
		
		String templateName = getTemplateName(item);
		if (templateName == null || templateName.equals(""))
		{
			return false;
		}
		
		if (!hasObjVar(self, house_pet.SARLACC_FEED_ARRAY))
		{
			CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" has NO previous feeding list. Creating a new one.");
			
			blog("Received Item - Created new feed list");
			String[] feedingList = new String[1];
			feedingList[0] = templateName;
			setObjVar(self, house_pet.SARLACC_FEED_ARRAY, feedingList);
		}
		else
		{
			
			CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" has A previous feeding list. Getting data.");
			
			String[] feedingList = getStringArrayObjVar(self, house_pet.SARLACC_FEED_ARRAY);
			if (feedingList == null || feedingList.length <= 0)
			{
				CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" previous feeding list was bugged or null. Creating a new list.");
				
				blog("Received Item - ERROR -NO PREVIOUS feedingList found, making new.");
				String[] newList = new String[1];
				newList[0] = templateName;
				setObjVar(self, house_pet.SARLACC_FEED_ARRAY, newList);
			}
			else if (feedingList.length > 0 && feedingList.length < 4)
			{
				CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" previous feeding list was "+feedingList.length+" in length. Adding another item to this list.");
				
				blog("Received Item - ARRAY COPY!!!!!!!!!!!!!!!!!!!!!!!!");
				blog("Received Item - feedingList.length "+feedingList.length);
				String[] newList = new String[feedingList.length+1];
				System.arraycopy(feedingList, 0, newList, 0, feedingList.length);
				newList[newList.length-1] = templateName;
				blog("Received Item - newList.length "+newList.length);
				setObjVar(self, house_pet.SARLACC_FEED_ARRAY, newList);
				CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" new feeding list NOW has "+newList.length+" items.");
			}
		}
		
		int currentIteration = getIntObjVar(self, house_pet.SARLACC_FEEDING_ITERATION);
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Current Feeding Iteration: "+currentIteration);
		
		currentIteration++;
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Current Feeding Iteration is now: "+currentIteration);
		
		setObjVar(self, house_pet.SARLACC_FEEDING_ITERATION, currentIteration);
		
		blog("Received Item - setting phase fed and setting time fed");
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Current Phase right after being fed is: "+getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE));
		
		setObjVar(self, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_FED);
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Current Phase now is: "+getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE));
		
		if (!hasObjVar(self, house_pet.NO_SARLACC_SOUND))
		{
			play2dNonLoopingSound(player, SOUND_CUTE);
		}
		
		doAnimationAction(self, ANIMATON_FEED);
		
		decrementCount(item);
		sendSystemMessage(player, SID_SARLACC_FED);
		
		blog("Received Item - end, current phase: "+getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE));
		
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Last Fed WAS: "+getIntObjVar(self, house_pet.SARLACC_LAST_FED));
		
		setObjVar(self, house_pet.SARLACC_LAST_FED, getCalendarTime());
		CustomerServiceLog("sarlacc_minigame: ", "Dianoga: "+self+" Last Fed NOW: "+getIntObjVar(self, house_pet.SARLACC_LAST_FED));
		
		if (hasObjVar(self, house_pet.SARLACC_PENALTY_VAR))
		{
			setObjVar(self, house_pet.SARLACC_FREE_CHANCE, true);
			removeObjVar(self, house_pet.SARLACC_PENALTY_VAR);
		}
		
		dictionary params = new dictionary();
		params.put("player", player);
		
		messageTo(self, "growlAnimation", params, 2, false);
		
		return true;
	}
	
	
	public boolean validateItemForSarlacc(obj_id player, obj_id item) throws InterruptedException
	{
		if (utils.getContainingPlayer(item) != (player))
		{
			CustomerServiceLog("sarlacc_minigame: ", "Player: "+player+" is attempting to give a Dianoga item: "+item+" but that item is not in their inventory");
			
			sendSystemMessage(player, SID_ITEM_NOT_INVENTORY);
			return false;
		}
		
		if (smuggler.hasItemsInContainer(item))
		{
			CustomerServiceLog("sarlacc_minigame: ", "Player: "+player+" is attempting to give a Dianoga item: "+item+" but that item is not in their ROOT inventory. It may be nested in another container.");
			
			sendSystemMessage(player, SID_ITEM_NOT_INVENTORY);
			return false;
		}
		
		if (utils.isEquipped(item))
		{
			
			return false;
		}
		
		if (hasObjVar(item, "quest_item"))
		{
			CustomerServiceLog("sarlacc_minigame: ", "Player: "+player+" is attempting to give a Dianoga item: "+item+" but that item is a quest item.");
			
			sendSystemMessage(player, SID_QUEST_ITEM);
			return false;
		}
		
		return true;
	}
	
	
	public String getSarlaccCollectionColumn(obj_id self) throws InterruptedException
	{
		blog("getSarlaccCollectionColumn - Init");
		if (!isValidId(self) || !exists(self))
		{
			return null;
		}
		
		blog("getSarlaccCollectionColumn - initial validation ok");
		
		if (!hasObjVar(self, house_pet.SARLACC_FEED_ARRAY))
		{
			CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" could not select a reward collection column because there war no list of items fed on the sarlacc.");
			return null;
		}
		
		blog("getSarlaccCollectionColumn - array of feed exists");
		CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" is selecting a reward collection column based on the list of items fed.");
		
		String[] foodList = getStringArrayObjVar(self, house_pet.SARLACC_FEED_ARRAY);
		if (foodList == null || foodList.length < house_pet.SARLACC_PHASE_WEEKS)
		{
			CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" found a corrupt list of items fed and could not continue.");
			blog("getSarlaccCollectionColumn - foodList.length: "+foodList.length);
			return null;
		}
		blog("getSarlaccCollectionColumn - foodList[0]: "+foodList[0]);
		blog("getSarlaccCollectionColumn - foodList[1]: "+foodList[1]);
		blog("getSarlaccCollectionColumn - foodList[2]: "+foodList[2]);
		blog("getSarlaccCollectionColumn - foodList[3]: "+foodList[3]);
		
		String listData = getListOfFoodItems(self);
		if (listData != null && listData.length() > 0)
		{
			CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" has eaten the following: "+listData);
		}
		
		HashSet allEdibles = new HashSet();
		
		for (int i = 0; i < foodList.length-1; i++)
		{
			testAbortScript();
			if (foodList[i] == null || foodList.equals(""))
			{
				continue;
			}
			
			allEdibles.add(foodList[i]);
		}
		
		if (allEdibles.isEmpty())
		{
			CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" could not put all fed items into a hashset and failed out of the Collection Column function!");
			blog("getSarlaccCollectionColumn - severe issue with feed array");
			return null;
		}
		
		blog("getSarlaccCollectionColumn - allEdibles.size(): "+allEdibles.size());
		String[] templateArray = new String[allEdibles.size()];
		allEdibles.toArray(templateArray);
		if (templateArray == null || templateArray.length <= 0)
		{
			CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" failed in the Collection Column function when attempting to make a hashset and array!");
			blog("getSarlaccCollectionColumn - severe issue with hashset conversion of feed array");
			return null;
		}
		blog("getSarlaccCollectionColumn - templateArray.length: "+templateArray.length);
		
		if (templateArray.length == 1)
		{
			int lootRow = dataTableSearchColumnForString(templateArray[0], "item", house_pet.SARLACC_CTS_CRC_TABLE);
			if (lootRow < 0)
			{
				CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" could not find the table row needed for the item fed and returned the default random column.");
				blog("getSarlaccCollectionColumn - lootRow: "+lootRow);
				return "generic_collectible";
			}
			
			String lootColumn = dataTableGetString(house_pet.SARLACC_CTS_CRC_TABLE, lootRow, "collectible_cols");
			blog("getSarlaccCollectionColumn - lootColumn: "+lootColumn);
			if (lootColumn == null || lootColumn.equals(""))
			{
				CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" could not find the collectible_cols row needed for the item fed and returned the default random column.");
				return "generic_collectible";
			}
			else
			{
				String[] possibleCols = split(lootColumn,',');
				if (possibleCols == null || possibleCols.length <= 0)
				{
					CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" had corrupt data and returned default generic_collectible.");
					return "generic_collectible";
				}
				else if (possibleCols.length == 1)
				{
					CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" had length of 1 and is returning: "+possibleCols[0]);
					return possibleCols[0];
				}
				else
				{
					int randNum = rand(0,possibleCols.length-1);
					CustomerServiceLog("sarlacc_minigame: ", "getSarlaccCollectionColumn - Dianoga: "+self+" had length of 1 and is returning a random item: "+possibleCols[randNum]);
					return possibleCols[randNum];
				}
			}
		}
		else
		{
			String randTemplate = templateArray[rand(0, templateArray.length-1)];
			int lootRow = dataTableSearchColumnForString(randTemplate, "item", house_pet.SARLACC_CTS_CRC_TABLE);
			
			String lootColumn = dataTableGetString(house_pet.SARLACC_CTS_CRC_TABLE, lootRow, "collectible_cols");
			blog("getSarlaccCollectionColumn - lootColumn: "+lootColumn);
			if (lootColumn == null || lootColumn.equals(""))
			{
				return "generic_collectible";
			}
			else
			{
				String[] possibleCols = split(lootColumn,',');
				if (possibleCols == null || possibleCols.length <= 0)
				{
					return "generic_collectible";
				}
				else if (possibleCols.length == 1)
				{
					return possibleCols[0];
				}
				else
				{
					return possibleCols[rand(0,possibleCols.length-1)];
				}
			}
		}
	}
	
	
	public boolean getPlayerReward(obj_id owner, obj_id self) throws InterruptedException
	{
		if (!isValidId(owner) || !exists(owner))
		{
			return false;
		}
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		obj_id playerInventory = utils.getInventoryContainer(owner);
		if (!isValidId(playerInventory) || !exists(playerInventory))
		{
			return false;
		}
		
		if (!hasObjVar(self,house_pet.SARLACC_REWARD_COLUMN))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") could not receive reward for feeding sarlacc. Dianoga reward column objvar was not on the Dianoga item. THIS MIGHT BE DUE TO THE PLAYER ATTEMPTING TO GET MULTIPLE REWARDS via quick radial select exploit.");
			
			return false;
		}
		
		String rewardCol = getStringObjVar(self, house_pet.SARLACC_REWARD_COLUMN);
		if (rewardCol == null || rewardCol.equals(""))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") could not receive reward for properly feeding sarlacc. Dianoga reward column objvar was found but invalid.");
			
			return false;
		}
		
		blog("rewardPlayer - getRandomCollectionItem for col: "+rewardCol);
		
		if (!hasObjVar(self, house_pet.NO_SARLACC_SOUND))
		{
			play2dNonLoopingSound(owner, SOUND_SIFT);
		}
		
		CustomerServiceLog("sarlacc_minigame: ",
		"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") is about to receive Dianoga reward. Using collection library for reward. Check the CollectionLootChannel logs for specifics or search for the player OID to get full details.");
		
		if (!collection.getRandomCollectionItemShowLootBox(owner, playerInventory, house_pet.SARLACC_LOOT_TABLE, rewardCol))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") FAILED TO ISSUE PLAYER REWARD. Dianoga: " +self+". This probably failed in the collection library being used to grant the player a reward item.");
			
			blog("rewardPlayer - severe issue with getRandomCollectionItem for col: "+rewardCol);
		}
		
		CustomerServiceLog("sarlacc_minigame: ",
		"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") has received reward. Check the CollectionLootChannel logs for specifics or search for the player OID to get full details.");
		
		removeObjVar(self, house_pet.SARLACC_REWARD_AVAILABLE);
		removeObjVar(self, house_pet.SARLACC_REWARD_COLUMN);
		sendSystemMessage(owner, SID_FOUND_REWARD);
		blog("rewardPlayer - REWARD GIVEN");
		
		return true;
	}
	
	
	public boolean checkPlayerInventoryForSarlaccFood(obj_id player) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		obj_id inventory = utils.getInventoryContainer(player);
		if (!isIdValid(inventory) || !exists(inventory))
		{
			return false;
		}
		
		obj_id[] contents = utils.getContents(inventory, true);
		if (contents == null || contents.length <= 0)
		{
			return false;
		}
		
		for (int x = 0; x < contents.length; x++)
		{
			testAbortScript();
			
			if (!validateItemForSarlacc(player, contents[x]))
			{
				continue;
			}
			
			if (hasObjVar(contents[x], "noTrade") || utils.isEquipped(contents[x]))
			{
				continue;
			}
			
			if (isCrafted(contents[x]))
			{
				continue;
			}
			
			if (static_item.getStaticObjectValue(getStaticItemName(contents[x])) > 0)
			{
				return true;
			}
			
			String itemTemplate = getTemplateName(contents[x]);
			if (dataTableSearchColumnForString(itemTemplate, "items", smuggler.TBL) > -1)
			{
				return true;
			}
			
		}
		return false;
	}
	
	
	public boolean setSarlaccStarvingOrDead(obj_id sarlacc, int currentPhase) throws InterruptedException
	{
		if (!isValidId(sarlacc) || !exists(sarlacc))
		{
			return false;
		}
		
		customerSvcLogVerbose(sarlacc, "updateSarlaccPet - About to Wipe Dianoga data and set feeding iteration to 0. This is the last point in which you will see previous data. Current time: "+ getCalendarTime());
		
		setObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION, 0);
		removeObjVar(sarlacc, house_pet.SARLACC_FEED_ARRAY);
		setObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE, currentPhase);
		return true;
	}
	
	
	public void cleanupSui(obj_id player) throws InterruptedException
	{
		blog("cleanupSui removing vars");
		utils.removeScriptVar(player, house_pet.EDIBLES_SUI);
		utils.removeBatchScriptVar(player, house_pet.EDIBLES_IDS);
		utils.removeScriptVar(player, "junkfood");
	}
	
	
	public boolean customerSvcLogVerbose(obj_id sarlacc, String msg) throws InterruptedException
	{
		if (!isValidId(sarlacc) || !exists(sarlacc))
		{
			return false;
		}
		
		String logData = "";
		if (msg != null && !msg.equals(""))
		{
			logData += msg;
		}
		
		if (isValidId(getOwner(sarlacc)))
		{
			logData += " Owner: "+getOwner(sarlacc) + ".";
		}
		else
		{
			logData += " No valid Dianoga owner.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_BORN))
		{
			int birthDay = getIntObjVar(sarlacc, house_pet.SARLACC_BORN);
			logData += " Cal Birth Date: "+ getCalendarTimeStringLocal(birthDay) + ".";
			logData += " Cal Date Int: "+ birthDay + ".";
		}
		else
		{
			logData += " No birth date for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE))
		{
			int phase = getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE);
			logData += " Phase: "+ phase + ".";
		}
		else
		{
			logData += " No current phase for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_PHASE_START))
		{
			int phaseStart = getIntObjVar(sarlacc, house_pet.SARLACC_PHASE_START);
			logData += " Cal Phase Start: "+ getCalendarTimeStringLocal(phaseStart) + ".";
			logData += " Phase Start Int: "+ phaseStart + ".";
		}
		else
		{
			logData += " No Feeding phase start for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_LAST_FED))
		{
			int lastFed = getIntObjVar(sarlacc, house_pet.SARLACC_LAST_FED);
			logData += " Cal Last Fed: "+ getCalendarTimeStringLocal(lastFed) + ".";
			logData += " Last Fed Int: "+ lastFed + ".";
		}
		else
		{
			logData += " Dianoga has never been fed.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION))
		{
			int currentIteration = getIntObjVar(sarlacc, house_pet.SARLACC_FEEDING_ITERATION);
			logData += " Feeding Iteration: "+ currentIteration + ".";
		}
		else
		{
			logData += " No Feeding iteration for this sarlacc.";
		}
		
		String listData = getListOfFoodItems(sarlacc);
		if (listData != null && listData.length() > 0)
		{
			logData += " "+ listData + " ";
		}
		else
		{
			logData += " No Feeding list for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR))
		{
			int sarlaccVer = getIntObjVar(sarlacc, house_pet.SARLACC_VERSION_VAR);
			logData += " Dianoga Version Number: "+sarlaccVer+".";
		}
		else
		{
			logData += " No Dianoga Version Number for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_FREE_CHANCE))
		{
			boolean freeChanceUsed = getBooleanObjVar(sarlacc, house_pet.SARLACC_FREE_CHANCE);
			logData += " Has Used Free Chance (T/F): "+freeChanceUsed+".";
		}
		else
		{
			logData += " No Dianoga Free Chance Variable for this sarlacc.";
		}
		
		if (hasObjVar(sarlacc, house_pet.SARLACC_PENALTY_VAR))
		{
			logData += " Dianoga has PENALTY VARIABLE.";
		}
		else
		{
			logData += " No Dianoga Penalty Variable.";
		}
		if (logData == null || logData.equals(""))
		{
			return false;
		}
		
		CustomerServiceLog("sarlacc_minigame: ",logData);
		
		return true;
	}
	
	
	public String getListOfFoodItems(obj_id sarlacc) throws InterruptedException
	{
		if (!isValidId(sarlacc))
		{
			return null;
		}
		if (!hasObjVar(sarlacc, house_pet.SARLACC_FEED_ARRAY))
		{
			return null;
		}
		
		String logData = "";
		String[] feedingList = getStringArrayObjVar(sarlacc, house_pet.SARLACC_FEED_ARRAY);
		if (feedingList != null && feedingList.length > 0)
		{
			for (int i = 0; i < feedingList.length; i++)
			{
				testAbortScript();
				logData += " Fed Item "+(i+1)+": "+ feedingList[i] + ". ";
			}
		}
		return logData;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
