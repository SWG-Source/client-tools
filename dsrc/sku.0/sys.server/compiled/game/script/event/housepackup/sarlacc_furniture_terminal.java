package script.event.housepackup;

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
import script.library.sui;
import script.library.utils;


public class sarlacc_furniture_terminal extends script.base_script
{
	public sarlacc_furniture_terminal()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String LOGGING_CATEGORY = "sarlacc";
	
	public static final string_id SID_MENU_FEED = new string_id("sarlacc_minigame", "mnu_feed");
	public static final string_id SID_MENU_REWARD = new string_id("sarlacc_minigame", "mnu_reward");
	public static final string_id SID_SIFT_SARLACC_BOX = new string_id("sarlacc_minigame", "must_sift_sarlacc_box");
	public static final string_id SID_SARLACC_FED = new string_id("sarlacc_minigame", "sarlacc_fed");
	public static final string_id SID_FOUND_REWARD = new string_id("sarlacc_minigame", "sifted_reward");
	public static final string_id SID_SARLACC_NOTHING_FOUND = new string_id("sarlacc_minigame", "no_junk_inventory");
	public static final string_id SID_VERIFICATION_MSG = new string_id("sarlacc_minigame", "verify_consumption");
	public static final string_id SID_ITEM_NOT_FED = new string_id("sarlacc_minigame", "item_not_fed");
	public static final string_id SID_ITEM_NOT_BIOLINK_FED = new string_id("sarlacc_minigame", "item_not_biolink_fed");
	public static final string_id SID_ITEM_NOT_INVENTORY = new string_id("sarlacc_minigame", "item_not_inventory");
	public static final string_id SID_ITEM_EQUIPPED = new string_id("sarlacc_minigame", "item_not_equipped");
	public static final string_id SID_ITEM_NOT_VALID = new string_id("sarlacc_minigame", "item_not_valid");
	public static final string_id SID_OUT_OF_RANGE = new string_id("sarlacc_minigame", "out_of_range");
	public static final string_id SID_QUEST_ITEM = new string_id("sarlacc_minigame", "item_quest_item");
	
	public static final String FEED_TITLE = "@sarlacc_minigame:edible_title";
	public static final String FEED_PROMPT = "@sarlacc_minigame:edible_prompt";
	public static final String BTN_SELL = "@sarlacc_minigame:btn_feed";
	
	public static final String SOUND_BURP = "sound/sarlacc_burp.snd";
	public static final String SOUND_EAT = "sound/sarlacc_eating.snd";
	public static final String SOUND_SIFT = "sound/sarlacc_drawer_open.snd";
	public static final String SOUND_CUTE = "sound/sarlacc_cute.snd";
	public static final String SOUND_BREATHING = "sound/sarlacc_breathing.snd";
	
	public static final String ANIMATON_GROWL = "vocalize";
	public static final String ANIMATON_FEED = "eat";
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		house_pet.stopPlayingMusic(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
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
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isInHouseCellSpace(self))
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
		
		if (player != getOwner(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, house_pet.SARLACC_REWARD_AVAILABLE))
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
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return false;
		}
		
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
		
		return true;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isNestedWithin(self, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (player != getOwner(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			if (getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE) < house_pet.SARLACC_HUNGRY)
			{
				return SCRIPT_CONTINUE;
			}
			
			getEdibleContents(player, self);
		}
		else if (item == menu_info_types.SERVER_MENU2)
		{
			
			getPlayerReward(player, self);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
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
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
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
			messageTo(self, "playCute", params, 1, false);
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
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
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
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		if (bp == sui.BP_CANCEL)
		{
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		else
		{
			
			obj_id[] junkFood = utils.getObjIdBatchScriptVar(player, house_pet.EDIBLES_IDS);
			
			if ((junkFood == null) || (junkFood.length == 0))
			{
				cleanupSui(player);
				return SCRIPT_CONTINUE;
			}
			
			if (idx > junkFood.length - 1)
			{
				cleanupSui(player);
				return SCRIPT_CONTINUE;
			}
			blog("verifySarlaccEatSui: Setting junk item script var: "+junkFood[idx]);
			
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
			blog("verifySarlaccEatSui: Selected Cancel or No.");
			return SCRIPT_CONTINUE;
		}
		
		blog("verifySarlaccEatSui: Getting junk item from script var.");
		if (!utils.hasScriptVar(player, "junkfood.junk_item"))
		{
			blog("verifySarlaccEatSui: No Junk item Found");
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		obj_id junkFood = utils.getObjIdScriptVar(player, "junkfood.junk_item");
		blog("verifySarlaccEatSui: Junk item: "+junkFood);
		
		if (!isValidId(junkFood) || !exists(junkFood))
		{
			blog("verifySarlaccEatSui: Junk item Invalid..bailing out");
			
			sendSystemMessage(player, SID_ITEM_NOT_VALID);
			cleanupSui(player);
			return SCRIPT_CONTINUE;
		}
		
		blog("verifySarlaccEatSui: Junk Confirmed, eating.");
		
		if (!eatJunkItem(self, player, junkFood))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(player) + " OID: "+ player + ") FAILED TO FEED sarlacc: " +self+" item ("+ getName(junkFood) + " OID: "+ junkFood + ").");
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
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
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
	
	
	public int playBurp(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
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
		
		play2dNonLoopingSound(player, SOUND_BURP);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playCute(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
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
		
		play2dNonLoopingSound(player, SOUND_CUTE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkTriggerVolume(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
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
	
	
	public int updateSarlaccPet(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		blog("updateSarlaccPet - RECEIVED MESSAGE TO - Current phase: "+getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE)+" time: "+ getCalendarTime());
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, house_pet.SARLACC_CURRENT_PHASE))
		{
			blog("updateSarlaccPet - no phase for sarlacc");
			return SCRIPT_CONTINUE;
		}
		if (!hasObjVar(self, house_pet.SARLACC_FEEDING_ITERATION))
		{
			blog("updateSarlaccPet - no feeding iteration for sarlacc");
			return SCRIPT_CONTINUE;
		}
		
		blog("updateSarlaccPet - Is in structure");
		
		int currentGameTime = getCalendarTime();
		int updateWeek = house_pet.getUpdateWeekly(self);
		int updateDay = house_pet.getUpdateDaily(self);
		
		int lastFed = getIntObjVar(self, house_pet.SARLACC_LAST_FED);
		
		blog("updateSarlaccPet - lastFed: "+lastFed);
		if (currentGameTime < (lastFed + updateWeek))
		{
			blog("updateSarlaccPet - Last Fed Timer: "+(lastFed + updateWeek)+" currentGameTime: "+currentGameTime);
			
			if (hasMessageTo(self, "updateSarlaccPet"))
			{
				return SCRIPT_CONTINUE;
			}
			
			int later = ((lastFed + updateWeek) - currentGameTime);
			blog("updateSarlaccPet - messageTo in : "+later);
			if (later > 1)
			{
				blog("updateSarlaccPet - TOO SOOOOON messageTo in : "+later);
				messageTo(self, "updateSarlaccPet", null, later, false);
			}
			else
			{
				blog("updateSarlaccPet - WRONG messageTo, Resending in : "+updateDay);
				messageTo(self, "updateSarlaccPet", null, updateDay, false);
			}
			return SCRIPT_CONTINUE;
		}
		
		if (hasMessageTo(self, "updateSarlaccPet"))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("updateSarlaccPet - Is in structure");
		
		int currentIteration = getIntObjVar(self, house_pet.SARLACC_FEEDING_ITERATION);
		int currentPhase = getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE);
		
		if (currentPhase < house_pet.SARLACC_FED)
		{
			blog("updateSarlaccPet - currentPhase WAS -1");
			currentPhase = house_pet.SARLACC_FED;
		}
		
		currentPhase++;
		
		if (currentPhase >= house_pet.SARLACC_DEATH)
		{
			setObjVar(self, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_DEATH);
		}
		else
		{
			setObjVar(self, house_pet.SARLACC_CURRENT_PHASE, currentPhase);
		}
		
		if (currentPhase == house_pet.SARLACC_DEATH)
		{
			blog("updateSarlaccPet - SARLACC DEAD!!!!!!!!");
			if (!hasMessageTo(self, "updateSarlaccPet"))
			{
				blog("updateSarlaccPet - SARLACC_DEATH messageTo");
				messageTo(self, "updateSarlaccPet", null, updateWeek, false);
			}
			return SCRIPT_CONTINUE;
		}
		else if (currentPhase == house_pet.SARLACC_STARVING)
		{
			blog("updateSarlaccPet - SARLACC STARVING!!!!!!!!");
			setObjVar(self, house_pet.SARLACC_FEEDING_ITERATION, 0);
			removeObjVar(self, house_pet.SARLACC_FEED_ARRAY);
			if (!hasMessageTo(self, "updateSarlaccPet"))
			{
				blog("updateSarlaccPet - SARLACC_STARVING messageTo");
				messageTo(self, "updateSarlaccPet", null, updateWeek, false);
			}
			return SCRIPT_CONTINUE;
		}
		
		else if (getIntObjVar(self, house_pet.SARLACC_FEEDING_ITERATION) >= house_pet.SARLACC_PHASE_WEEKS)
		{
			blog("updateSarlaccPet - ITERATION DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			String collectionCol = getSarlaccCollectionColumn(self);
			if (collectionCol == null || collectionCol.equals(""))
			{
				
				blog("updateSarlaccPet - getSarlaccCollectionColumn FAILED!");
				collectionCol = "generic_collectible";
			}
			
			blog("updateSarlaccPet - getSarlaccCollectionColumn: "+collectionCol);
			
			setObjVar(self, house_pet.SARLACC_FEEDING_ITERATION, 0);
			
			removeObjVar(self, house_pet.SARLACC_FEED_ARRAY);
			
			setObjVar(self, house_pet.SARLACC_REWARD_AVAILABLE, true);
			
			setObjVar(self, house_pet.SARLACC_REWARD_COLUMN, collectionCol);
		}
		
		if (!hasMessageTo(self, "updateSarlaccPet"))
		{
			messageTo(self, "updateSarlaccPet", null, updateWeek, false);
			blog("updateSarlaccPet - messageTo end in one week");
		}
		
		return SCRIPT_CONTINUE;
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
		
		if (getTemplateName(sarlacc).indexOf("mini_s01.iff") > 0)
		{
			return false;
		}
		
		if (getIntObjVar(sarlacc, house_pet.SARLACC_CURRENT_PHASE) < house_pet.SARLACC_HUNGRY)
		{
			return false;
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
			
			utils.setScriptVar(player, house_pet.EDIBLES_SUI, pid);
			utils.setBatchScriptVar(player, house_pet.EDIBLES_IDS, edibleItems);
		}
		return true;
	}
	
	
	public boolean eatJunkItem(obj_id self, obj_id player, obj_id item) throws InterruptedException
	{
		blog("eatJunkItem - Init");
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return false;
		}
		
		if (!isIdValid(self) || !exists(self))
		{
			return false;
		}
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
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
		CustomerServiceLog("sarlacc_minigame: ", "Player ("+ playerName + " OID: "+ player + ") fed sarlacc item ("+ itemName + " OID: "+ item + ").");
		
		String templateName = getTemplateName(item);
		if (templateName == null || templateName.equals(""))
		{
			return false;
		}
		
		if (!hasObjVar(self, house_pet.SARLACC_FEED_ARRAY))
		{
			blog("Received Item - Created new feed list");
			String[] feedingList = new String[1];
			feedingList[0] = templateName;
			setObjVar(self, house_pet.SARLACC_FEED_ARRAY, feedingList);
		}
		else
		{
			String[] feedingList = getStringArrayObjVar(self, house_pet.SARLACC_FEED_ARRAY);
			if (feedingList == null || feedingList.length <= 0)
			{
				blog("Received Item - ERROR -NO PREVIOUS feedingList found, making new.");
				String[] newList = new String[1];
				newList[0] = templateName;
				setObjVar(self, house_pet.SARLACC_FEED_ARRAY, newList);
			}
			else if (feedingList.length > 0 && feedingList.length < 4)
			{
				blog("Received Item - ARRAY COPY!!!!!!!!!!!!!!!!!!!!!!!!");
				blog("Received Item - feedingList.length "+feedingList.length);
				String[] newList = new String[feedingList.length+1];
				System.arraycopy(feedingList, 0, newList, 0, feedingList.length);
				newList[newList.length-1] = templateName;
				blog("Received Item - newList.length "+newList.length);
				setObjVar(self, house_pet.SARLACC_FEED_ARRAY, newList);
			}
		}
		
		int currentIteration = getIntObjVar(self, house_pet.SARLACC_FEEDING_ITERATION);
		currentIteration++;
		setObjVar(self, house_pet.SARLACC_FEEDING_ITERATION, currentIteration);
		
		blog("Received Item - setting phase fed and setting time fed");
		setObjVar(self, house_pet.SARLACC_CURRENT_PHASE, house_pet.SARLACC_FED);
		
		play2dNonLoopingSound(player, SOUND_EAT);
		
		doAnimationAction(self, ANIMATON_FEED);
		
		decrementCount(item);
		sendSystemMessage(player, SID_SARLACC_FED);
		
		blog("Received Item - end, current phase: "+getIntObjVar(self, house_pet.SARLACC_CURRENT_PHASE));
		setObjVar(self, house_pet.SARLACC_LAST_FED, getCalendarTime());
		
		if (!hasMessageTo(self, "updateSarlaccPet"))
		{
			blog("Received Item - first feeding kicks off the messageTo");
			messageTo(self, "updateSarlaccPet", null, house_pet.getUpdateWeekly(self), false);
		}
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo(self, "playBurp", params, 4, false);
		messageTo(self, "growlAnimation", params, 2, false);
		
		return true;
	}
	
	
	public boolean validateItemForSarlacc(obj_id player, obj_id item) throws InterruptedException
	{
		if (utils.getContainingPlayer(item) != (player))
		{
			
			sendSystemMessage(player, SID_ITEM_NOT_INVENTORY);
			return false;
		}
		
		if (smuggler.hasItemsInContainer(item))
		{
			
			sendSystemMessage(player, SID_ITEM_NOT_INVENTORY);
			return false;
		}
		
		if (utils.isEquipped(item))
		{
			
			sendSystemMessage(player, SID_ITEM_EQUIPPED);
			return false;
		}
		
		if (hasObjVar(item, "quest_item"))
		{
			
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
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
		{
			return null;
		}
		
		blog("getSarlaccCollectionColumn - initial validation ok");
		
		if (!hasObjVar(self, house_pet.SARLACC_FEED_ARRAY))
		{
			return null;
		}
		
		blog("getSarlaccCollectionColumn - array of feed exists");
		
		String[] foodList = getStringArrayObjVar(self, house_pet.SARLACC_FEED_ARRAY);
		if (foodList == null || foodList.length < house_pet.SARLACC_PHASE_WEEKS)
		{
			blog("getSarlaccCollectionColumn - foodList.length: "+foodList.length);
			return null;
		}
		blog("getSarlaccCollectionColumn - foodList[0]: "+foodList[0]);
		blog("getSarlaccCollectionColumn - foodList[1]: "+foodList[1]);
		blog("getSarlaccCollectionColumn - foodList[2]: "+foodList[2]);
		blog("getSarlaccCollectionColumn - foodList[3]: "+foodList[3]);
		
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
			blog("getSarlaccCollectionColumn - severe issue with feed array");
			return null;
		}
		
		blog("getSarlaccCollectionColumn - allEdibles.size(): "+allEdibles.size());
		String[] templateArray = new String[allEdibles.size()];
		allEdibles.toArray(templateArray);
		if (templateArray == null || templateArray.length <= 0)
		{
			blog("getSarlaccCollectionColumn - severe issue with hashset conversion of feed array");
			return null;
		}
		blog("getSarlaccCollectionColumn - templateArray.length: "+templateArray.length);
		
		if (templateArray.length == 1)
		{
			int lootRow = dataTableSearchColumnForString(templateArray[0], "item", house_pet.SARLACC_CTS_CRC_TABLE);
			if (lootRow < 0)
			{
				blog("getSarlaccCollectionColumn - lootRow: "+lootRow);
				return "generic_collectible";
			}
			
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
		
		if (getTemplateName(self).indexOf("mini_s01.iff") > 0)
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
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") could not receive reward for properly feeding sarlacc. Sarlacc reward column objvar was not on the sarlacc item.");
			
			return false;
		}
		
		String rewardCol = getStringObjVar(self, house_pet.SARLACC_REWARD_COLUMN);
		if (rewardCol == null || rewardCol.equals(""))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") could not receive reward for properly feeding sarlacc. Sarlacc reward column objvar was found but invalid.");
			
			return false;
		}
		
		blog("rewardPlayer - getRandomCollectionItem for col: "+rewardCol);
		
		play2dNonLoopingSound(owner, SOUND_SIFT);
		
		CustomerServiceLog("sarlacc_minigame: ",
		"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") is about to receive Sarlacc reward. Using collection library for reward. Check the CollectionLootChannel logs for specifics or search for the player OID to get full details.");
		
		if (!collection.getRandomCollectionItem(owner, playerInventory, house_pet.SARLACC_LOOT_TABLE, rewardCol))
		{
			CustomerServiceLog("sarlacc_minigame: ",
			"Player ("+ getPlayerName(owner) + " OID: "+ owner + ") FAILED TO ISSUE PLAYER REWARD. Sarlacc: " +self+". This probably failed in the collection library being used to grant the player a reward item.");
			
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
	
	
	public void cleanupSui(obj_id player) throws InterruptedException
	{
		blog("cleanupSui removing vars");
		utils.removeScriptVar(player, house_pet.EDIBLES_SUI);
		utils.removeBatchScriptVar(player, house_pet.EDIBLES_IDS);
		utils.removeScriptVar(player, "junkfood");
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
