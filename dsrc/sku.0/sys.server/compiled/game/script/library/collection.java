package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.HashSet;
import script.library.badge;
import script.library.buff;
import script.library.gcw;
import script.library.groundquests;
import script.library.prose;
import script.library.skill;
import script.library.static_item;
import script.library.stealth;
import script.library.sui;
import script.library.utils;
import script.library.xp;


public class collection extends script.base_script
{
	public collection()
	{
	}
	public static final String COLLECTION_REWARD_TABLE = "datatables/collection/rewards.iff";
	public static final String COLLECTION_TABLE = "datatables/collection/collection.iff";
	public static final String COLLECTION_NPC_TABLE = "datatables/collection/collection_npc.iff";
	public static final String QUEST__COMPLETED_COLLECTIONS = "datatables/collection/quest_completion_collections.iff";
	public static final String COL_NPC_PID = "collection_npc";
	public static final String CONSUME_PID_NAME = "collectionConsume";
	public static final String REWARD_ON_UPDATE_CATEGORY = "rewardOnUpdate";
	public static final String REWARD_ON_COMPLETE_CATEGORY = "rewardOnComplete";
	public static final String CLEAR_ON_COMPLETE = "clearOnComplete";
	public static final String CATEGORY_PREREQ_MET = "grantIfPreReqMet";
	public static final String CATEGORY_UPDATE_ON_COUNT = "updateOnCount";
	public static final String NO_MESSAGE_CATEGORY = "noMessage";
	public static final String CRAFTING_TEMPLATE = "crafting_template";
	public static final String OBJVAR_SLOT_NAME = "collection.slotName";
	public static final String OBJVAR_PAGE_NAME = "collection.pageName";
	public static final String ENT_BUFF_COLLECTION_01 = "col_entertainer_01";
	public static final String COL_BUFF_TRACKER = "collection.buffTracker";
	public static final String COL_HEROIC_SD_TAXI_SERVICE = "heroic_sd_taxi_service_01";
	public static final String COL_HEROIC_SD_SMUGGLE_OBJVAR = "collection.heroic_sd_smuggle_timer";
	public static final String COL_SLOT_SMUGGLE_GROUPS = "smuggled_groups";
	public static final String COL_BUFF_ARRAY = "collection.buffTracker";
	public static final String AUTO_STACK_SCRIPT = "object.autostack";
	public static final String CATEGORY_NUM_ALT_TITLES = "numAltTitles";
	
	public static final boolean LOGGING_ENABLED = true;
	
	public static final float MAX_HATE_RADIUS_ON_CLICK_COL = 15.0f;
	
	public static final int MAXLOOP = 7;
	public static final int REQUIRED_TIME_LAPSE = 21600;
	
	public static final int CONST_ROLL_CHANCE = 50;
	
	public static final int CONST_RESOURCE_ROLL = 4;
	public static final String PRISTINE_MEAT = "col_pristine_meat_02_01";
	public static final String PRISTINE_HIDE = "col_pristine_hide_02_01";
	public static final String PRISTINE_BONE = "col_pristine_bone_02_01";
	public static final String PRISTINE_EGG = "col_pristine_egg_02_01";
	public static final String PRISTINE_MILK = "col_pristine_milk_02_01";
	
	public static final string_id SID_REWARD_XP = new string_id("collection","reward_xp_amount");
	public static final string_id SID_REWARD_ITEM = new string_id("collection","reward_item");
	public static final string_id SID_REWARD_SKILL_MOD = new string_id("collection","reward_skill_mod");
	public static final string_id SID_SLOT_INCREMENT = new string_id("collection","player_slot_increment");
	public static final string_id SID_SYS_NO_COMBAT = new string_id("collection","click_not_combat");
	public static final string_id SID_SYS_NO_MOUNT = new string_id("collection","click_not_mounted");
	public static final string_id SID_SYS_NO_DEAD_INCAP = new string_id("collection","click_not_dead_incap");
	public static final string_id SID_NEED_TO_ACTIVATE_COLLECTION = new string_id("collection", "need_to_activate_collection");
	public static final string_id SID_MAX_ACTIVE = new string_id("collection", "max_active_collections");
	public static final string_id COMPLETE_COLLECTION_CAN_NOT_CLEAR = new string_id("collection", "collection_complete_can_not_clear");
	public static final string_id COLLECTION_CLEARED = new string_id("collection", "collection_cleared");
	public static final string_id SID_SYS_QUEST_INCORRECT = new string_id("collection", "collection_quest_wrong");
	public static final String SUI_DELETE_TITLE = "@collection:collection_sui_delete_title";
	public static final String SUI_DELETE_PROMPT = "@collection:collection_sui_delete_prompt";
	public static final String COL_NPC_PROMPT = "@collection:col_npc_prompt";
	public static final String COL_NPC_TITLE = "@collection:col_npc_title";
	public static final string_id SID_COLLECTION_RESET = new string_id("collection","player_collection_reset");
	public static final string_id UPDATED_ENTERTAINER_COLLECTION = new string_id("collection", "update_entertainer");
	public static final string_id SUCCESSFUL_SMUGGLE_ATTEMPT = new string_id("collection", "successful_smuggle_attempt");
	public static final string_id SMUGGLE_ATTEMPT_TIME_ISSUE = new string_id("collection", "smuggle_attempt_time_issue");
	public static final string_id DRIVING_SOROSUUB = new string_id("collection", "driving_sorosuub");
	public static final string_id STILL_HAS_TAXI_BUFF = new string_id("collection", "still_has_taxi_buff");
	public static final string_id SID_ALREADY_FINISHED_COLLECTION = new string_id("collection", "already_finished_collection");
	public static final string_id SID_NOT_CLOSE_ENOUGH = new string_id("collection", "not_close_enough");
	public static final string_id SID_ALREADY_HAVE_SLOT = new string_id("collection", "already_have_slot");
	public static final string_id SID_REPORT_CONSUME_ITEM_FAIL = new string_id("collection", "report_consume_item_fail");
	
	public static final String[] ICE_CREAM_BUFF_ARRAY = 
	{
		"item_ice_cream_buff_debuff_01_01",
		"item_ice_cream_buff_forage_buff_01_01",
		"item_ice_cream_buff_health_01_01",
		"item_ice_cream_buff_action_01_01",
		"item_ice_cream_buff_xp_bonus_01_01",
		"item_ice_cream_buff_remove_weakened_01_01",
		"item_ice_cream_buff_remove_debuff_01_01",
		"item_ice_cream_buff_action_reduction_01_01"
	};
	
	public static final String FRYER_BUFF_TYPE = "ice_cream_category";
	public static final int ICE_CREAM_ALL_PLAYERS = 0;
	public static final int ICE_CREAM_DOMESTICS_ONLY = 1;
	public static final int MAX_RANGE_TO_COLLECT = 5;
	
	
	public static void blog(String identifier, String text) throws InterruptedException
	{
		if (LOGGING_ENABLED)
		{
			LOG(identifier, text);
		}
	}
	
	
	public static boolean grantCollectionReward(obj_id player, String collectionName, boolean canResetCollection) throws InterruptedException
	{
		LOG("debug_collection","grantCollectionReward for player: "+player+" collectionName: "+collectionName);
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		int row = dataTableSearchColumnForString(collectionName, "collection_name", COLLECTION_REWARD_TABLE);
		
		if (row < 0)
		{
			
			blog("COLLECTIONS", "collection with no entry in rewards table. Collection name is "+ collectionName);
			
			return false;
		}
		
		dictionary dict = dataTableGetRow(COLLECTION_REWARD_TABLE, row);
		
		if (dict == null || dict.isEmpty())
		{
			return false;
		}
		
		LOG("debug_collection","grantCollectionReward for dict: "+dict);
		
		String slotName = dict.getString("slot_name");
		String quest = dict.getString("quest");
		String item = dict.getString("item");
		String command = dict.getString("command");
		String skillMod = dict.getString("skill_mod");
		String questSignal = dict.getString("quest_signal");
		int grantRandom = dict.getInt("grantRandomItem");
		int grantWeightedRandom = dict.getInt("grantWeightedRandom");
		int itemIncrement = dict.getInt("stackAmount");
		
		prose_package pp = new prose_package();
		
		LOG("debug_collection","grantCollectionReward for player: "+player+" collectionName: "+collectionName+" THE ITEM: "+item);
		
		if (slotName != null && !slotName.equals(""))
		{
			
			String[] badges = split(slotName, ',');
			blog("COLLECTIONS", "badges.length: "+ badges.length + " badges[0]: "+ badges[0]);
			for (int i = 0; i < badges.length; ++i)
			{
				testAbortScript();
				String[] info = getCollectionSlotInfo(badges[i]);
				if ((info != null) || (info.length == COLLECTION_INFO_ARRAY_SIZE) || (info[COLLECTION_INFO_INDEX_BOOK] != null))
				{
					if (info[COLLECTION_INFO_INDEX_BOOK].equals("badge_book"))
					{
						badge.grantBadge(player, badges[i]);
					}
					else
					{
						modifyCollectionSlotValue(player, badges[i], 1);
					}
				}
				else
				{
					
				}
			}
			CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted Slot"+ slotName + ".");
		}
		
		LOG("debug_collection","grantCollectionReward for player: "+player+" collectionName: "+collectionName+" made it to xp");
		
		int xpAmount = 0;
		boolean spaceType = false;
		
		int isSpaceXP = dataTableGetInt(collection.COLLECTION_REWARD_TABLE, collectionName, "is_space_xp");
		
		if (isSpaceXP == 1)
		{
			spaceType = true;
			xpAmount = xp.grantCollectionSpaceXP(player, collectionName);
		}
		else
		{
			
			xpAmount = xp.grantCollectionXP(player, collectionName);
		}
		
		if (xpAmount > 0)
		{
			
			prose.setStringId(pp, SID_REWARD_XP);
			prose.setDI(pp, xpAmount);
			sendSystemMessageProse(player, pp);
			
			if (spaceType)
			{
				xp.grant(player, xp.SPACE_COMBAT_GENERAL, xpAmount, true);
			}
			else
			{
				xp.grantXpByTemplate(player, xpAmount);
			}
			CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted XP: "+ xpAmount + ".");
		}
		
		if (quest != null && !quest.equals(""))
		{
			
			String[] quests = split(quest, ',');
			for (int i = 0; i < quests.length; ++i)
			{
				testAbortScript();
				int result = groundquests.grantQuestNoAcceptUI(player, quests[i], false);
				if (result > 0)
				{
					logQuestError(player, quest, result);
				}
			}
		}
		LOG("debug_collection","grantCollectionReward for player: "+player+" collectionName: "+collectionName+" made it to item");
		
		if (item != null && !item.equals(""))
		{
			LOG("debug_collection","grantCollectionReward collectionName: "+collectionName+" item: "+item);
			
			Vector allNewObjectsResizable = new Vector();
			allNewObjectsResizable.setSize(0);
			obj_id itemId = null;
			String[] items = split(item, ',');
			
			if (grantRandom == 1)
			{
				itemId = getRandomItem(player, items, collectionName, pp);
				if (!isValidId(itemId))
				{
					return false;
				}
				utils.addElement(allNewObjectsResizable, itemId);
				if (itemIncrement > 1 && hasScript(itemId, AUTO_STACK_SCRIPT))
				{
					setCount(itemId, itemIncrement);
				}
			}
			else if (grantWeightedRandom == 1)
			{
				itemId = getWeightedRandomItem(player, items, collectionName, pp);
				if (!isValidId(itemId))
				{
					return false;
				}
				
				utils.addElement(allNewObjectsResizable, itemId);
				if (itemIncrement > 1 && hasScript(itemId, AUTO_STACK_SCRIPT))
				{
					setCount(itemId, itemIncrement);
				}
			}
			else
			{
				LOG("debug_collection","grantCollectionReward collectionName reward item not random or weighted: "+item);
				
				for (int i = 0; i < items.length; ++i)
				{
					testAbortScript();
					if (static_item.isStaticItem(items[i]))
					{
						LOG("debug_collection","grantCollectionReward collectionName reward item is static item: "+item);
						
						itemId = createStaticItemWithMessage(player, items[i], collectionName, pp);
						if (!isValidId(itemId))
						{
							return false;
						}
						
						LOG("debug_collection","grantCollectionReward collectionName reward item awarded to player");
						
						utils.addElement(allNewObjectsResizable, itemId);
					}
					else
					{
						itemId = createTemplateItemWithMessage(player, items[i], collectionName, pp);
						if (!isValidId(itemId))
						{
							return false;
						}
						
						utils.addElement(allNewObjectsResizable, itemId);
					}
					if (itemIncrement > 1 && hasScript(itemId, AUTO_STACK_SCRIPT))
					{
						setCount(itemId, itemIncrement);
					}
				}
			}
			obj_id[] finalLootList = new obj_id[allNewObjectsResizable.size()];
			allNewObjectsResizable.toArray(finalLootList);
			showLootBox(player, finalLootList);
		}
		
		if (command != null && !command.equals(""))
		{
			String[] commands = split(command, ',');
			for (int i = 0; i < commands.length; ++i)
			{
				testAbortScript();
				grantCommand(player, commands[i]);
				CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted Command: "+ command + ".");
			}
		}
		
		if (skillMod != null && !skillMod.equals(""))
		{
			int skillModAmount = dict.getInt("skill_mod_amount");
			int skillModMax = dict.getInt("skill_mod_max");
			
			String[] skillMods = split(skillMod, ',');
			for (int i = 0; i < skillMods.length; ++i)
			{
				testAbortScript();
				int currentSkillModAmount = getSkillStatMod(player, skillMods[i]);
				
				if (currentSkillModAmount < skillModMax || skillModMax == -1)
				{
					prose.setStringId(pp, SID_REWARD_SKILL_MOD);
					prose.setDI(pp, skillModAmount);
					prose.setTT(pp, new string_id("stat_n", skillMods[i]));
					applySkillStatisticModifier(player, skillMods[i], skillModAmount);
					sendSystemMessageProse(player, pp);
					CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted SkillMod: "+ skillMods[i] + ".");
				}
			}
		}
		
		if (questSignal != null && !questSignal.equals(""))
		{
			groundquests.sendSignal(player, questSignal);
			CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted QuestSignal "+ questSignal + ".");
		}
		
		if (canResetCollection == true)
		{
			removeCompletedCollection(player, collectionName);
		}
		
		return true;
	}
	
	
	public static boolean updateCraftingSlot(obj_id player, String template) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (template == null || template.equals(""))
		{
			return false;
		}
		
		String[] collectionTemplateCol = dataTableGetStringColumn(COLLECTION_REWARD_TABLE, CRAFTING_TEMPLATE );
		
		if (collectionTemplateCol == null || collectionTemplateCol.equals(""))
		{
			return false;
		}
		
		int row = dataTableSearchColumnForString(template, CRAFTING_TEMPLATE, COLLECTION_REWARD_TABLE);
		
		if (row < 0)
		{
			blog("COLLECTIONS", "Collection Reward Table did not have template data to increment slot for crafting collection for "+ template + ".");
			return false;
		}
		
		String category = dataTableGetString(COLLECTION_REWARD_TABLE, row, "category");
		if (category == null || category.equals(""))
		{
			blog("COLLECTIONS", "Collection Reward Table had a template but no category for "+ template + ".");
			return false;
		}
		
		String[] slotNames = getAllCollectionSlotsInCategory(category);
		
		if (slotNames == null || slotNames.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < slotNames.length; ++i)
		{
			testAbortScript();
			if (hasCompletedCollectionSlot(player, slotNames[i]))
			{
				continue;
			}
			
			String[] collectionInfo = getCollectionSlotInfo(slotNames[i]);
			String collectionName = collectionInfo[COLLECTION_INFO_INDEX_COLLECTION];
			
			modifyCollectionSlotValue(player, slotNames[i], 1);
			
			blog("COLLECTIONS", "Updating allslotName "+ slotNames[i]);
			
			if (hasCompletedCollectionSlot(player, slotNames[i]))
			{
				continue;
			}
			
			prose_package pp = new prose_package();
			prose.setStringId(pp, SID_SLOT_INCREMENT);
			prose.setTU(pp, new string_id("collection_n", slotNames[i]));
			prose.setTO(pp, new string_id("collection_n", collectionName));
			sendSystemMessageProse(player, pp);
		}
		return true;
	}
	
	
	public static boolean checkState(obj_id player) throws InterruptedException
	{
		if (getState(player, STATE_COMBAT) == 1)
		{
			sendSystemMessage(player, SID_SYS_NO_COMBAT);
			return false;
		}
		if (getState(player, STATE_RIDING_MOUNT) == 1)
		{
			sendSystemMessage(player, SID_SYS_NO_MOUNT);
			return false;
		}
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, SID_SYS_NO_DEAD_INCAP);
			return false;
		}
		return true;
	}
	
	
	public static boolean checkDistance(obj_id clickObj, obj_id player, int maxDist) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		location objLoc = getLocation(clickObj);
		if (objLoc == null || objLoc.equals(""))
		{
			return false;
		}
		
		location playerLoc = getLocation(player);
		if (playerLoc == null || playerLoc.equals(""))
		{
			return false;
		}
		
		float dist = utils.getDistance2D(objLoc, playerLoc);
		if (dist > maxDist || dist < 0)
		{
			return false;
		}
		
		return true;
	}
	
	
	public static boolean giveAreaMobsHate(obj_id collectible, obj_id player) throws InterruptedException
	{
		if (hasObjVar(collectible, "collection.aggro"))
		{
			boolean hasAggro = getBooleanObjVar(collectible, "collection.aggro");
			if (!hasAggro)
			{
				return false;
			}
		}
		
		if (!isIdValid(collectible) || !exists(collectible))
		{
			return false;
		}
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		location collectibleLoc = getLocation(collectible);
		if (collectibleLoc == null)
		{
			return false;
		}
		
		obj_id[] allMobsCombat = getAllNpcs(collectibleLoc, MAX_HATE_RADIUS_ON_CLICK_COL);
		if (allMobsCombat == null || allMobsCombat.length < 1)
		{
			return false;
		}
		
		blog("collection","length of mobs: "+allMobsCombat.length);
		
		for (int i = 0; i < allMobsCombat.length; i++)
		{
			testAbortScript();
			
			if (allMobsCombat[i] == player)
			{
				continue;
			}
			if (hasObjVar(allMobsCombat[i], "ignoreCollector"))
			{
				continue;
			}
			if (isInvulnerable(allMobsCombat[i]))
			{
				continue;
			}
			if (pvpCanAttack(allMobsCombat[i], player))
			{
				startCombat(allMobsCombat[i], player);
			}
			
		}
		return true;
	}
	
	
	public static void showNpcCollections(obj_id player, obj_id npc, String columnName) throws InterruptedException
	{
		if (sui.hasPid(player, COL_NPC_PID))
		{
			int pid = sui.getPid(player, COL_NPC_PID);
			forceCloseSUIPage(pid);
		}
		
		HashSet availableCollections = new HashSet();
		HashSet active_collection = new HashSet();
		
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, columnName);
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, columnName);
		String[] prereqColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		for (int i = 0; i < allCollections.length; i++)
		{
			testAbortScript();
			if (!hasCompletedCollectionSlot(player, prereqColumn[i]))
			{
				availableCollections.add(allCollections[i]);
			}
			else if (hasCompletedCollectionSlot(player, prereqColumn[i]) && !hasCompletedCollection(player, allCollections[i]))
			{
				active_collection.add(allCollections[i]);
			}
		}
		
		String[] strArrayAvailableCollections = new String[availableCollections.size()];
		availableCollections.toArray(strArrayAvailableCollections);
		
		String[] activeCollections = new String[active_collection.size()];
		active_collection.toArray(activeCollections);
		
		if (activeCollections.length >= 2)
		{
			sendSystemMessage(player, SID_MAX_ACTIVE);
			return;
		}
		
		String[] SUIcollectionNames = new String[strArrayAvailableCollections.length];
		
		for (int q = 0; q < strArrayAvailableCollections.length; q++)
		{
			testAbortScript();
			SUIcollectionNames[q] = "@collection_n:" + strArrayAvailableCollections[q];
		}
		
		utils.setScriptVar(player, "collection.allCollections", strArrayAvailableCollections);
		int pid = sui.listbox( npc, player, COL_NPC_PROMPT, sui.OK_CANCEL, COL_NPC_TITLE, SUIcollectionNames, "handleCollectionNpc", true );
		sui.setPid(player, pid, COL_NPC_PID);
	}
	
	
	public static void findAndGrantSlot(obj_id player, obj_id npc, String selectedCollection) throws InterruptedException
	{
		String collectionColumn = "";
		String slotToGrant = "";
		
		if (utils.hasObjVar(npc, "collection.columnName"))
		{
			collectionColumn = getStringObjVar(npc, "collection.columnName");
		}
		else
		{
			return;
		}
		
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, collectionColumn);
		
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, collectionColumn);
		String[] grantColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		for (int r = 0; r < allCollections.length; r++)
		{
			testAbortScript();
			if (allCollections[r].equals(selectedCollection))
			{
				modifyCollectionSlotValue(player, grantColumn[r], 1);
			}
		}
	}
	
	
	public static boolean checkMaxActive(obj_id player, obj_id npc, String columnName) throws InterruptedException
	{
		HashSet active_collection = new HashSet();
		
		if (!hasObjVar(npc, "collection.columnName"))
		{
			return true;
		}
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, columnName);
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, columnName);
		String[] prereqColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		if (allCollections == null || allCollections.length <= 0)
		{
			return false;
		}
		if (prereqColumn == null || prereqColumn.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < allCollections.length; i++)
		{
			testAbortScript();
			if (hasCompletedCollectionSlot(player, prereqColumn[i]) && !hasCompletedCollection(player, allCollections[i]))
			{
				active_collection.add(allCollections[i]);
			}
		}
		
		String[] activeCollections = new String[active_collection.size()];
		active_collection.toArray(activeCollections);
		
		if (activeCollections.length >= 2)
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean npcHasMoreCollections(obj_id player, obj_id npc, String columnName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (!isIdValid(npc) || !exists(npc))
		{
			return false;
		}
		if (columnName == null || columnName.equals(""))
		{
			return false;
		}
		
		HashSet playerCollections = new HashSet();
		
		if (!hasObjVar(npc, "collection.columnName"))
		{
			return false;
		}
		
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, columnName);
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, columnName);
		String[] prereqColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		if (allCollections == null || allCollections.length <= 0)
		{
			return false;
		}
		
		if (prereqColumn == null || prereqColumn.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < allCollections.length; i++)
		{
			testAbortScript();
			if (hasCompletedCollectionSlot(player, prereqColumn[i]))
			{
				playerCollections.add(allCollections[i]);
				
			}
		}
		
		String[] activeOrComplete = new String[playerCollections.size()];
		playerCollections.toArray(activeOrComplete);
		
		if (activeOrComplete.length == allCollections.length)
		{
			return false;
		}
		return true;
	}
	
	
	public static String[] getAllCollectionsForItem(obj_id collectible) throws InterruptedException
	{
		if (!isIdValid(collectible) || !exists(collectible))
		{
			return null;
		}
		
		String[] baseSlotNames = split(getStringObjVar(collectible, OBJVAR_SLOT_NAME), '|');
		
		Vector collectionNames = new Vector();
		
		for (int i = 0; i < baseSlotNames.length; ++i)
		{
			testAbortScript();
			
			String[] splitSlotNames = split(baseSlotNames[i], ':');
			collection.blog("COLLECTIONS", "splitSlotNames.length "+ splitSlotNames.length);
			
			for (int j = 0; j < splitSlotNames.length; j += 2)
			{
				testAbortScript();
				collectionNames.add(splitSlotNames[j]);
			}
			
		}
		
		String[] availableCollections = new String[collectionNames.size()];
		collectionNames.toArray(availableCollections);
		return availableCollections;
	}
	
	
	public static String[] getAllSlotsForItem(obj_id collectible) throws InterruptedException
	{
		if (!isIdValid(collectible) || !exists(collectible))
		{
			return null;
		}
		
		String[] baseSlotNames = split(getStringObjVar(collectible, OBJVAR_SLOT_NAME), '|');
		
		Vector slotNames = new Vector();
		
		for (int i = 0; i < baseSlotNames.length; ++i)
		{
			testAbortScript();
			
			String[] splitSlotNames = split(baseSlotNames[i], ':');
			
			collection.blog("COLLECTIONS", "splitSlotNames.length "+ splitSlotNames.length);
			
			for (int j = 0; j < splitSlotNames.length; j += 2)
			{
				testAbortScript();
				slotNames.add(splitSlotNames[j + 1]);
			}
			
		}
		
		String[] availableSlots = new String[slotNames.size()];
		slotNames.toArray(availableSlots);
		return availableSlots;
	}
	
	
	public static String[] getAllAvailableCollectionsForItem(obj_id player, obj_id collectible) throws InterruptedException
	{
		if (!isIdValid(collectible) || !exists(collectible))
		{
			return null;
		}
		
		String[] baseSlotNames = split(getStringObjVar(collectible, OBJVAR_SLOT_NAME), '|');
		
		Vector collectionNames = new Vector();
		
		for (int i = 0; i < baseSlotNames.length; ++i)
		{
			testAbortScript();
			
			String[] splitSlotNames = split(baseSlotNames[i], ':');
			collection.blog("COLLECTIONS", "splitSlotNames.length "+ splitSlotNames.length);
			
			for (int j = 0; j < splitSlotNames.length; j += 2)
			{
				testAbortScript();
				collection.blog("COLLECTIONS", "splitSlotNames["+ j + "] "+ splitSlotNames[j]);
				
				if (!hasCompletedCollection(player, splitSlotNames[j]))
				{
					
					if (!hasCompletedCollectionSlot(player, splitSlotNames[j + 1]))
					{
						
						if (hasCompletedCollectionSlotPrereq(player, splitSlotNames[j + 1]))
						{
							
							collectionNames.add(splitSlotNames[j]);
						}
					}
				}
			}
			
		}
		
		String[] availableCollections = new String[collectionNames.size()];
		collectionNames.toArray(availableCollections);
		return availableCollections;
	}
	
	
	public static String[] getAllAvailableSlotsForItem(obj_id player, obj_id collectible) throws InterruptedException
	{
		if (!isIdValid(collectible) || !exists(collectible))
		{
			return null;
		}
		
		String[] baseSlotNames = split(getStringObjVar(collectible, OBJVAR_SLOT_NAME), '|');
		
		Vector slotNames = new Vector();
		
		for (int i = 0; i < baseSlotNames.length; ++i)
		{
			testAbortScript();
			
			String[] splitSlotNames = split(baseSlotNames[i], ':');
			collection.blog("COLLECTIONS", "splitSlotNames.length "+ splitSlotNames.length);
			
			for (int j = 0; j < splitSlotNames.length; j += 2)
			{
				testAbortScript();
				collection.blog("COLLECTIONS", "splitSlotNames["+ j + "] "+ splitSlotNames[j]);
				
				if (!hasCompletedCollection(player, splitSlotNames[j]))
				{
					
					if (!hasCompletedCollectionSlot(player, splitSlotNames[j + 1]))
					{
						
						if (hasCompletedCollectionSlotPrereq(player, splitSlotNames[j + 1]))
						{
							
							slotNames.add(splitSlotNames[j + 1]);
						}
					}
				}
			}
			
		}
		
		String[] availableSlots = new String[slotNames.size()];
		slotNames.toArray(availableSlots);
		return availableSlots;
	}
	
	
	public static boolean removeCollectionForRealsies(obj_id player, String collectionName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		String[] slotsInCollection = getAllCollectionSlotsInCollection(collectionName);
		
		for (int i = 0; i < slotsInCollection.length; i++)
		{
			testAbortScript();
			
			long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
			
			modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotValue);
		}
		sendSystemMessage(player, COLLECTION_CLEARED);
		return true;
	}
	
	
	public static boolean revokeThenGrantCollection(obj_id player, String collectionName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		String[] slotsInCollection = getAllCollectionSlotsInCollection(collectionName);
		
		for (int i = 0; i < slotsInCollection.length; i++)
		{
			testAbortScript();
			
			long collectionSlotNegativeValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
			long collectionSlotPostiveValue = getCollectionSlotValue(player, slotsInCollection[i]);
			
			modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotNegativeValue);
			modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotPostiveValue);
		}
		sendSystemMessageTestingOnly(player, "Collection Revoked and Regranted.");
		return true;
	}
	
	
	public static boolean removeCollection(obj_id player, String collectionName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		if (hasCompletedCollection(player, collectionName))
		{
			sendSystemMessage(player, COMPLETE_COLLECTION_CAN_NOT_CLEAR);
			return false;
		}
		
		String[] slotsInCollection = getAllCollectionSlotsInCollection(collectionName);
		
		for (int i = 0; i < slotsInCollection.length; i++)
		{
			testAbortScript();
			
			long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
			
			modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotValue);
		}
		sendSystemMessage(player, COLLECTION_CLEARED);
		CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") has voluntarily reset their collection: "+ collectionName);
		return true;
	}
	
	
	public static boolean removeCompletedCollection(obj_id player, String collectionName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		if (!hasCompletedCollection(player, collectionName))
		{
			return false;
		}
		
		String[] slotsInCollection = getAllCollectionSlotsInCollection(collectionName);
		
		for (int i = 0; i < slotsInCollection.length; i++)
		{
			testAbortScript();
			
			long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]) * -1;
			
			modifyCollectionSlotValue(player, slotsInCollection[i], collectionSlotValue);
		}
		
		prose_package pp = new prose_package();
		prose.setTO(pp, new string_id("collection_n", collectionName));
		prose.setStringId(pp, SID_COLLECTION_RESET);
		sendSystemMessageProse(player, pp);
		CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") has voluntarily reset their collection: "+ collectionName);
		return true;
	}
	
	
	public static void showNpcCollectionsRemoval(obj_id player, obj_id npc, String columnName) throws InterruptedException
	{
		if (sui.hasPid(player, COL_NPC_PID))
		{
			int pid = sui.getPid(player, COL_NPC_PID);
			forceCloseSUIPage(pid);
		}
		
		HashSet active_collection = new HashSet();
		
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, columnName);
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, columnName);
		String[] prereqColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		for (int i = 0; i < allCollections.length; i++)
		{
			testAbortScript();
			if (hasCompletedCollectionSlot(player, prereqColumn[i]) && !hasCompletedCollection(player, allCollections[i]))
			{
				active_collection.add(allCollections[i]);
			}
		}
		
		String[] activeCollections = new String[active_collection.size()];
		active_collection.toArray(activeCollections);
		
		String[] SUIcollectionNames = new String[activeCollections.length];
		
		for (int q = 0; q < activeCollections.length; q++)
		{
			testAbortScript();
			SUIcollectionNames[q] = "@collection_n:" + activeCollections[q] + "_finished";
		}
		
		utils.setScriptVar(player, "collection.allCollections", activeCollections);
		int pid = sui.listbox( npc, player, SUI_DELETE_PROMPT, sui.OK_CANCEL, SUI_DELETE_TITLE, SUIcollectionNames, "handleCollectionRemoval", true );
		sui.setPid(player, pid, COL_NPC_PID);
	}
	
	
	public static boolean npcHasCollectionsToRemove(obj_id player, obj_id npc, String columnName) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		if (!isIdValid(npc) || !exists(npc))
		{
			return false;
		}
		if (columnName == null || columnName.equals(""))
		{
			return false;
		}
		
		HashSet active_collection = new HashSet();
		
		String[] allCollections = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, columnName);
		int columnNumber = dataTableFindColumnNumber(COLLECTION_NPC_TABLE, columnName);
		String[] prereqColumn = dataTableGetStringColumnNoDefaults(COLLECTION_NPC_TABLE, (columnNumber + 1));
		
		if (allCollections == null || allCollections.length <= 0)
		{
			return false;
		}
		if (prereqColumn == null || prereqColumn.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < allCollections.length; i++)
		{
			testAbortScript();
			if (hasCompletedCollectionSlot(player, prereqColumn[i]) && !hasCompletedCollection(player, allCollections[i]))
			{
				active_collection.add(allCollections[i]);
			}
		}
		
		String[] activeCollections = new String[active_collection.size()];
		active_collection.toArray(activeCollections);
		
		if (activeCollections.length > 0)
		{
			return true;
		}
		return false;
	}
	
	
	public static boolean spaceGetCreditForKills(obj_id player, String[] slotNames) throws InterruptedException
	{
		if (slotNames == null || slotNames.length <= 0)
		{
			return false;
		}
		
		for (int i = 0; i < slotNames.length; ++i)
		{
			testAbortScript();
			if (!hasCompletedCollectionSlot(player, slotNames[i]))
			{
				modifyCollectionSlotValue(player, slotNames[i], 1);
			}
		}
		return true;
	}
	
	
	public static boolean logQuestError(obj_id player, String quest, int result) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player))
		{
			return false;
		}
		
		String s = "";
		
		if (result == 1)
		{
			s = "Error: Quest already active.";
		}
		else if (result == 2)
		{
			s = "Error: No such quest.";
		}
		else if (result == 3)
		{
			s = "Error: No such task.";
		}
		else if (result == 4)
		{
			s = "Error: Quest already completed, and not repeatable.";
		}
		else if (result == 5)
		{
			s = "Error: Failed prerequisites.";
		}
		else if (result == 6)
		{
			s = "Error: Failed exclusions.";
		}
		else if (result == 7)
		{
			s = "Error: Quest Not active.";
		}
		else if (result == 8)
		{
			s = "Error: Task not active.";
		}
		else if (result == 9)
		{
			s = "Error: No such player.";
		}
		
		if (s.equals("") || s.equals(""))
		{
			return false;
		}
		
		CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has failed to receive quest: "+ quest + " becasue of the errror: "+ s);
		blog("collection","Quest Error: "+ s);
		
		return true;
	}
	
	
	public static boolean entertainerBuffCollection(obj_id buffedPlayerId, obj_id bufferId, float duration) throws InterruptedException
	{
		
		if (!isIdValid(buffedPlayerId) || !isIdValid(bufferId))
		{
			return false;
		}
		
		if (hasCompletedCollection(bufferId, ENT_BUFF_COLLECTION_01))
		{
			return false;
		}
		
		String buffedPlayerTemplate = getSkillTemplate(buffedPlayerId);
		
		if (buffedPlayerTemplate.startsWith("trader"))
		{
			buffedPlayerTemplate = "prof_trader";
		}
		
		if (buffedPlayerTemplate.startsWith("entertainer"))
		{
			return false;
		}
		
		if (hasCompletedCollectionSlot(bufferId, buffedPlayerTemplate))
		{
			
			return false;
		}
		
		int collectionRollChance = rand(1, 100);
		if (collectionRollChance > CONST_ROLL_CHANCE)
		{
			
			buff.applyBuff(buffedPlayerId, "col_ent_invis_buff_tracker");
			return false;
		}
		
		if (duration >= 7200.00)
		{
			
			buff.applyBuff(buffedPlayerId, "col_ent_invis_buff_tracker");
			
			modifyCollectionSlotValue(bufferId, buffedPlayerTemplate, 1);
			
			sendSystemMessage(bufferId, new string_id("collection", "update_entertainer"));
			
			return true;
		}
		else
		{
			sendSystemMessage(bufferId, new string_id("collection", "time_too_short"));
			return false;
		}
	}
	
	
	public static boolean pilotSmuggleTimeCheck(obj_id pilot, obj_id groupMember, obj_id ship, String strChassisType) throws InterruptedException
	{
		
		if (buff.hasBuff(groupMember, "col_sd_invis_buff_tracker"))
		{
			buff.removeBuff(groupMember, "col_sd_invis_buff_tracker");
			CustomerServiceLog("Collection: ", "DePalma: Buff was removed from player: ("+ groupMember + ") we are now tracking by objvar");
		}
		
		if (!isIdValid(pilot) || !isIdValid(groupMember) || !isIdValid(ship))
		{
			CustomerServiceLog("Collection: ", "DePalma: validation of pilot/groupMember/ship null value: Pilot: "+ pilot + "groupMember: "+ groupMember + "Ship: "+ ship);
			return false;
		}
		
		if (hasCompletedCollection(groupMember, COL_HEROIC_SD_TAXI_SERVICE))
		{
			if (hasObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR))
			{
				
				removeObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR);
				CustomerServiceLog("Collection: ", "DePalma: Player completed collection - objvar removed: "+ groupMember + ".");
			}
			return false;
		}
		if (strChassisType == null || strChassisType.equals(""))
		{
			CustomerServiceLog("Collection: ", "DePalma: invalid Chassis(checks everyone): blank or null: ShipId: "+ ship + "("+ strChassisType + ")");
			
			return false;
		}
		if (strChassisType.equals("player_sorosuub_space_yacht") && groupMember == pilot && utils.isOwner(ship, groupMember))
		{
			CustomerServiceLog("Collection: ", "DePalma: Sorosuub Chassis (Pilot check): Chassis "+ strChassisType + "Pilot Id ("+ pilot + ")"+ "groupMember(should be same as pilot)"+ groupMember);
			
			sendSystemMessage(pilot, DRIVING_SOROSUUB);
			return false;
		}
		
		int currentTime = getCalendarTime();
		int nextTimeEligible = 0;
		
		if (hasObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR))
		{
			
			int timeStampCheck = getIntObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR);
			
			if (currentTime < timeStampCheck)
			{
				if (groupMember.equals(pilot) && utils.isOwner(ship, groupMember))
				{
					CustomerServiceLog("Collection: ", "DePalma: Timer not expired - currentTime: "+ currentTime + ". from Player: "+ groupMember + ".");
					
					sendSystemMessage(pilot, SMUGGLE_ATTEMPT_TIME_ISSUE);
				}
				return false;
			}
			else
			{
				
				nextTimeEligible = currentTime + REQUIRED_TIME_LAPSE;
				
				setObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR, nextTimeEligible);
				
				if (groupMember.equals(pilot) && utils.isOwner(ship, groupMember))
				{
					CustomerServiceLog("Collection: ", "DePalma: Successful Run: Pilot ("+ groupMember + ") received update. This was his first run - did not have objvar...Must wait until "+ nextTimeEligible + " for an update.");
					
					modifyCollectionSlotValue(groupMember, COL_SLOT_SMUGGLE_GROUPS, 1);
					
					sendSystemMessage(pilot, SUCCESSFUL_SMUGGLE_ATTEMPT);
					return true;
				}
			}
		}
		
		else
		{
			nextTimeEligible = currentTime + REQUIRED_TIME_LAPSE;
			
			setObjVar(groupMember, COL_HEROIC_SD_SMUGGLE_OBJVAR, nextTimeEligible);
			
			if (groupMember.equals(pilot) && utils.isOwner(ship, groupMember))
			{
				CustomerServiceLog("Collection: ", "DePalma: Successful Run: Pilot ("+ groupMember + ") received update. Must wait until "+ nextTimeEligible + " for an update.");
				
				modifyCollectionSlotValue(groupMember, COL_SLOT_SMUGGLE_GROUPS, 1);
				
				sendSystemMessage(pilot, SUCCESSFUL_SMUGGLE_ATTEMPT);
				return true;
			}
		}
		return false;
	}
	
	
	public static boolean collectionResource(obj_id player, String restype) throws InterruptedException
	{
		return collectionResource(player, restype, 0);
	}
	
	
	public static boolean collectionResource(obj_id player, String restype, int modifier) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (restype == null || restype.equals(""))
		{
			return false;
		}
		if (modifier < 0)
		{
			CustomerServiceLog("Collection: ", "DESIGNER BUG "+modifier+" is invalid. Int should be higher than zero.");
			modifier = 0;
		}
		
		CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") is rolling for resource: "+restype+" with a modifier of "+modifier+".");
		int randomChoice = rand(1, 100);
		CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") is rolling for resource: "+restype+" and has received a roll of "+randomChoice+".");
		
		if (modifier > 0)
		{
			CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") is rolling for resource: "+restype+" has a modifier of "+modifier+" so this will be SUBTRACTED FROM: "+randomChoice+".");
			randomChoice -= modifier;
			if (randomChoice < 0)
			{
				randomChoice = 0;
			}
			
			CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") is rolling for resource: "+restype+" the roll has been modified to be: "+randomChoice+".");
		}
		
		if (utils.hasScriptVar(player, "qa.resource_roll_bypass"))
		{
			randomChoice = 1;
			sendSystemMessageTestingOnly(player, "Collection Harvest Percentage bypassed for testing(100% success).");
		}
		
		if (randomChoice > CONST_RESOURCE_ROLL)
		{
			CustomerServiceLog("Collection: ", "Player "+ getFirstName(player) + "("+ player + ") was rolling for resource: "+restype+" but rolled "+randomChoice+" which was higher than the cap of "+CONST_RESOURCE_ROLL+".");
			return false;
		}
		
		String staticItem = "";
		String resourceMessage = "";
		if (restype.startsWith("meat"))
		{
			staticItem = PRISTINE_MEAT;
			resourceMessage = "resource_meat";
		}
		else if (restype.startsWith("hide"))
		{
			staticItem = PRISTINE_HIDE;
			resourceMessage = "resource_hide";
		}
		else if (restype.startsWith("bone"))
		{
			staticItem = PRISTINE_BONE;
			resourceMessage = "resource_bone";
		}
		else if (restype.startsWith("egg"))
		{
			staticItem = PRISTINE_EGG;
			resourceMessage = "resource_egg";
		}
		else if (restype.startsWith("milk"))
		{
			staticItem = PRISTINE_MILK;
			resourceMessage = "resource_milk";
		}
		if (staticItem == null || staticItem.equals(""))
		{
			CustomerServiceLog("Collection: ", "DESIGNER BUG - static item reward came back invalid for Player "+ getFirstName(player) + "("+ player + ").");
			return false;
		}
		if (resourceMessage == null || resourceMessage.equals(""))
		{
			CustomerServiceLog("Collection: ", "DESIGNER BUG - reward message came back invalid for Player "+ getFirstName(player) + "("+ player + ").");
			return false;
		}
		
		obj_id newResourceItem = static_item.createNewItemFunction(staticItem, player);
		if (!isValidId(newResourceItem) || !exists(newResourceItem))
		{
			
			CustomerServiceLog("CollectionLootChannel: ", "collectionResource - BrokenLoot: inValid resource failed to be rewarded to Player: "+ player + " Name: "+getName(player));
			return false;
		}
		CustomerServiceLog("CollectionLootChannel", "collectionResource - Player: "+player+" Name: "+getName(player)+ " Has triggered the creation of a static item object within the collection system. The object: "+newResourceItem+" Name: "+getName(newResourceItem));
		
		sendSystemMessage(player, new string_id("collection", resourceMessage));
		return true;
	}
	
	
	public static boolean getRandomCollectionItemShowLootBox(obj_id player, obj_id container, String loot_table, String loot_column) throws InterruptedException
	{
		
		if (!isIdValid(container) || !isIdValid(player))
		{
			return false;
		}
		
		String[] collectionLootArray = dataTableGetStringColumnNoDefaults(loot_table, loot_column);
		
		if (collectionLootArray == null || collectionLootArray.length <= 0)
		{
			return false;
		}
		int random = rand(0, collectionLootArray.length-1);
		
		obj_id collectionItem = static_item.createNewItemFunction(collectionLootArray[random], container);
		
		if (!isValidId(collectionItem) || !exists(collectionItem))
		{
			
			CustomerServiceLog("CollectionLootChannel: ", "getRandomCollectionItem - BrokenLoot: inValid object was created for player "+ player + " Name: "+getName(player)+" for container: "+ container);
			return false;
		}
		CustomerServiceLog("CollectionLootChannel", "getRandomCollectionItem - Player: "+player+" Name: "+getName(player)+ " Has triggered the creation of a collection object from the collection system. The object: "+collectionItem+" Name: "+getName(collectionItem)+" has been placed in the container: "+container);
		obj_id objArrray[] = new obj_id[1];
		objArrray[0] = collectionItem;
		showLootBox(player, objArrray);
		return true;
	}
	
	
	public static boolean getRandomCollectionItem(obj_id player, obj_id container, String loot_table, String loot_column) throws InterruptedException
	{
		
		if (!isIdValid(container) || !isIdValid(player))
		{
			return false;
		}
		
		String[] collectionLootArray = dataTableGetStringColumnNoDefaults(loot_table, loot_column);
		
		if (collectionLootArray == null || collectionLootArray.length <= 0)
		{
			return false;
		}
		int random = rand(0, collectionLootArray.length-1);
		
		obj_id collectionItem = static_item.createNewItemFunction(collectionLootArray[random], container);
		
		if (!isValidId(collectionItem) || !exists(collectionItem))
		{
			
			CustomerServiceLog("CollectionLootChannel: ", "getRandomCollectionItem - BrokenLoot: inValid object was created for player "+ player + " Name: "+getName(player)+" for container: "+ container);
			return false;
		}
		CustomerServiceLog("CollectionLootChannel", "getRandomCollectionItem - Player: "+player+" Name: "+getName(player)+ " Has triggered the creation of a collection object from the collection system. The object: "+collectionItem+" Name: "+getName(collectionItem)+" has been placed in the container: "+container);
		
		return true;
	}
	
	
	public static obj_id grantRandomCollectionItem(obj_id player, String loot_table, String loot_column) throws InterruptedException
	{
		if (!isIdValid(player) || !isPlayer(player))
		{
			return obj_id.NULL_ID;
		}
		obj_id playerInv = getObjectInSlot(player, "inventory");
		if (!isIdValid(playerInv))
		{
			return obj_id.NULL_ID;
		}
		
		String[] collectionLootArray = dataTableGetStringColumnNoDefaults(loot_table, loot_column);
		if (collectionLootArray == null || collectionLootArray.length <= 0)
		{
			return obj_id.NULL_ID;
		}
		
		int random = rand(0, collectionLootArray.length-1);
		obj_id collectionItem = static_item.createNewItemFunction(collectionLootArray[random], playerInv);
		if (!isValidId(collectionItem) || !exists(collectionItem))
		{
			
			CustomerServiceLog("CollectionLootChannel: ", "grantRandomCollectionItem - BrokenLoot: inValid object was created for player "+ player + " Name: "+getName(player));
			return obj_id.NULL_ID;
		}
		else
		{
			CustomerServiceLog("CollectionLootChannel", "grantRandomCollectionItem - Player: "+player+" Name: "+getName(player)+ " Has received a random collection object from the collection system. The object: "+collectionItem+" Name: "+getName(collectionItem)+" has been placed in the player inventory: "+playerInv);
			prose_package pp = new prose_package();
			prose.setStringId(pp, new string_id("collection","reward_item"));
			prose.setTT(pp, new string_id("static_item_n", getStaticItemName(collectionItem)));
			sendSystemMessageProse(player, pp);
		}
		
		return collectionItem;
	}
	
	
	public static boolean gcwBaseControlCheck(obj_id factionObject, obj_id player) throws InterruptedException
	{
		
		String playerFaction = factions.getFaction(player);
		
		obj_id subject = gcw.getPub30StaticBaseControllerId(factionObject);
		int baseFaction = gcw.getPub30StaticBaseControllingFaction(subject);
		
		if (baseFaction == gcw.NO_CONTROL)
		{
			return false;
		}
		
		String planet = getCurrentSceneName();
		
		if (baseFaction == gcw.REBEL_CONTROL)
		{
			
			if (toLower(playerFaction).equals("imperial"))
			{
				
				utils.setScriptVar(factionObject, "collection.gcwSlotName", "col_gcw_insurgency_"+ planet + "_imperial");
				return true;
			}
			else
			{
				return false;
			}
		}
		
		else if (baseFaction == gcw.IMPERIAL_CONTROL)
		{
			if (toLower(playerFaction).equals("rebel"))
			{
				
				utils.setScriptVar(factionObject, "collection.gcwSlotName", "col_gcw_insurgency_"+ planet + "_rebel");
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
	
	
	public static boolean canCollectCollectible(obj_id player, obj_id collectible) throws InterruptedException
	{
		if (!isValidId(collectible) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectible) || !exists(player))
		{
			return false;
		}
		
		if (!checkState(player))
		{
			return false;
		}
		
		String baseSlotName = getStringObjVar(collectible, OBJVAR_SLOT_NAME);
		if (baseSlotName == null || baseSlotName.equals(""))
		{
			return false;
		}
		
		String[] splitSlotNames = split(baseSlotName, ':');
		if (splitSlotNames == null || splitSlotNames.length < 0)
		{
			return false;
		}
		
		String slotName = splitSlotNames[1];
		String collectionName = splitSlotNames[0];
		
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		boolean isCloseEnough = checkDistance(collectible, player, MAX_RANGE_TO_COLLECT);
		if (!isCloseEnough)
		{
			sendSystemMessage(player, SID_NOT_CLOSE_ENOUGH);
			return false;
		}
		
		else if (!hasCompletedCollectionSlotPrereq(player, slotName))
		{
			
			sendSystemMessage(player, SID_NEED_TO_ACTIVATE_COLLECTION);
			return false;
		}
		
		else if (hasCompletedCollection(player, collectionName))
		{
			
			sendSystemMessage(player, SID_ALREADY_FINISHED_COLLECTION);
			return false;
		}
		
		else if ((slotName == null || slotName.equals("")) || hasCompletedCollectionSlot(player, slotName))
		{
			
			sendSystemMessage(player, SID_ALREADY_HAVE_SLOT);
			return false;
		}
		
		return true;
	}
	
	
	public static boolean rewardPlayerCollectionSlot(obj_id player, obj_id collectible) throws InterruptedException
	{
		if (!isValidId(collectible) || !isValidId(player))
		{
			return false;
		}
		else if (!exists(collectible) || !exists(player))
		{
			return false;
		}
		
		String baseSlotName = getStringObjVar(collectible, OBJVAR_SLOT_NAME);
		if (baseSlotName == null || baseSlotName.equals(""))
		{
			return false;
		}
		
		String[] splitSlotNames = split(baseSlotName, ':');
		if (splitSlotNames == null || splitSlotNames.length < 0)
		{
			return false;
		}
		
		String slotName = splitSlotNames[1];
		String collectionName = splitSlotNames[0];
		if (collectionName == null || collectionName.equals(""))
		{
			return false;
		}
		
		if (!hasCompletedCollectionSlotPrereq(player, slotName))
		{
			
			sendSystemMessage(player, SID_NEED_TO_ACTIVATE_COLLECTION);
			return false;
		}
		
		else if (hasCompletedCollection(player, collectionName))
		{
			
			sendSystemMessage(player, SID_ALREADY_FINISHED_COLLECTION);
			return false;
		}
		
		else if ((slotName == null || slotName.equals("")) || hasCompletedCollectionSlot(player, slotName))
		{
			
			sendSystemMessage(player, SID_ALREADY_HAVE_SLOT);
			return false;
		}
		
		if (!checkState(player))
		{
			return false;
		}
		
		boolean isCloseEnough = collection.checkDistance(collectible, player, MAX_RANGE_TO_COLLECT);
		if (!isCloseEnough)
		{
			sendSystemMessage(player, SID_NOT_CLOSE_ENOUGH);
			return false;
		}
		
		stealth.testInvisNonCombatAction(player, collectible);
		
		giveAreaMobsHate(collectible, player);
		
		if (modifyCollectionSlotValue(player, slotName, 1))
		{
			CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectible + ")"+ " was consumed into a collection, for player "+ getFirstName(player) + "("+ player + ").");
		}
		else
		{
			
			CustomerServiceLog("CollectionConsume: ", "collectionItem ("+ collectible + ")"+ " was NOT consumed into a collection, for player "+ getFirstName(player) + "("+ player + ").");
			
			sendSystemMessage(player, SID_REPORT_CONSUME_ITEM_FAIL);
			return false;
		}
		
		return true;
	}
	
	
	public static obj_id getRandomItem(obj_id player, String[] items, String collectionName, prose_package pp) throws InterruptedException
	{
		int randomChoice = rand(0, items.length -1);
		String itemToGrant = items[randomChoice];
		
		if (static_item.isStaticItem(itemToGrant))
		{
			return createStaticItemWithMessage(player, itemToGrant, collectionName, pp);
		}
		
		return createTemplateItemWithMessage(player, itemToGrant, collectionName, pp);
	}
	
	
	public static obj_id getWeightedRandomItem(obj_id player, String[] items, String collectionName, prose_package pp) throws InterruptedException
	{
		
		int randomChoice = 0;
		
		for (int k = 0; k < MAXLOOP; k++)
		{
			testAbortScript();
			
			randomChoice = rand(0, items.length -1);
			
			dictionary itemDict = dataTableGetRow(static_item.ITEM_STAT_BALANCE_TABLE, items[randomChoice]);
			
			String slotObjvar = itemDict.getString("objvars");
			String[] objvarArray = split(slotObjvar, ':');
			slotObjvar = objvarArray[objvarArray.length -1];
			
			if (hasCompletedCollectionSlot(player, slotObjvar))
			{
				
				continue;
			}
			else
			{
				
				break;
			}
		}
		if (static_item.isStaticItem(items[randomChoice]))
		{
			return createStaticItemWithMessage(player, items[randomChoice], collectionName, pp);
		}
		
		return createTemplateItemWithMessage(player, items[randomChoice], collectionName, pp);
	}
	
	
	public static obj_id createStaticItemWithMessage(obj_id player, String staticItem, String collectionName, prose_package pp) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player) || staticItem == null || staticItem.equals(""))
		{
			return null;
		}
		
		obj_id itemId = static_item.createNewItemFunction(staticItem, player);
		if (!isValidId(itemId) || !exists(itemId))
		{
			CustomerServiceLog("CollectionComplete: ", "createStaticItemWithMessage - Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " Item: "+ staticItem + " was not created due to invalid itemID. Attempted to create as static item.");
			return null;
		}
		CustomerServiceLog("CollectionComplete: ", "createStaticItemWithMessage - Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " Item: "+ staticItem + " OID: "+itemId+" was rewarded.");
		
		prose.setStringId(pp, SID_REWARD_ITEM);
		prose.setTT(pp, new string_id("static_item_n", getStaticItemName(itemId)));
		sendSystemMessageProse(player, pp);
		CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " and was granted Item: "+ staticItem + "("+ itemId + ")"+ ".");
		return itemId;
	}
	
	
	public static obj_id createTemplateItemWithMessage(obj_id player, String templateItem, String collectionName, prose_package pp) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player) || templateItem == null || templateItem.equals(""))
		{
			return null;
		}
		
		obj_id pInv = utils.getInventoryContainer(player);
		if (!isValidId(pInv) || !exists(pInv))
		{
			CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " Item: "+ templateItem + "was not created due to invalid Player Inventory.");
			return null;
		}
		obj_id itemId = createObjectOverloaded(templateItem, pInv);
		if (!isValidId(itemId) || !exists(itemId))
		{
			CustomerServiceLog("CollectionComplete: ", "Player "+ getFirstName(player) + "("+ player + ") has completed "+ collectionName + " Item: "+ templateItem + "was not created due to invalid itemID. Attempted to create as object template.");
			return null;
		}
		prose.setStringId(pp, SID_REWARD_ITEM);
		prose.setTT(pp, "@"+ getName(itemId));
		sendSystemMessageProse(player, pp);
		return itemId;
	}
	
	
	public static void giveNewbieCommWindow(obj_id player, obj_id object) throws InterruptedException
	{
		if (!isIdValid(player) || !isIdValid(object))
		{
			return;
		}
		
		if (getLevel(player) >= 10)
		{
			return;
		}
		
		if (utils.hasScriptVar(player, "newbie_comm_series"))
		{
			int number = utils.getIntScriptVar(player, "newbie_comm_series");
			if (number > 3)
			{
				
				return;
			}
			prose_package pp = new prose_package();
			
			switch(number)
			{
				case 1:
				prose.setStringId(pp, new string_id("collection", "newbie_comm_message_2"));
				commPlayers(object, "object/mobile/r2.iff", "sound/dro_r2_3_danger.snd", 10f, player, pp);
				number++;
				utils.setScriptVar(player, "newbie_comm_series", number);
				break;
				case 2:
				prose.setStringId(pp, new string_id("collection", "newbie_comm_message_3"));
				commPlayers(object, "object/mobile/r2.iff", "sound/dro_r2_3_danger.snd", 15f, player, pp);
				number++;
				utils.setScriptVar(player, "newbie_comm_series", number);
				break;
				case 3:
				prose.setStringId(pp, new string_id("collection", "newbie_comm_message_4"));
				commPlayers(object, "object/mobile/r2.iff", "sound/dro_r2_3_danger.snd", 12f, player, pp);
				number++;
				utils.setScriptVar(player, "newbie_comm_series", number);
				break;
				default:
				break;
			}
		}
		else
		{
			prose_package pp = new prose_package();
			prose.setStringId(pp, new string_id("collection", "newbie_comm_message"));
			commPlayers(object, "object/mobile/r2.iff", "sound/dro_r2_3_danger.snd", 10f, player, pp);
			utils.setScriptVar(player, "newbie_comm_series", 1);
		}
		return;
	}
	
	
	public static void grantQuestBasedCollections(String questString, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player) || !exists(player) || !isPlayer(player))
		{
			return;
		}
		
		String collectionFaction = null;
		String collectionName = null;
		
		if (factions.isRebelorRebelHelper(player))
		{
			collectionFaction = "rebelCollection";
		}
		
		else if (factions.isImperialorImperialHelper(player))
		{
			collectionFaction = "imperialCollection";
		}
		
		else
		{
			collectionFaction = "neutralCollection";
		}
		
		int tableRow = dataTableSearchColumnForString(questString, "questName", QUEST__COMPLETED_COLLECTIONS);
		
		if (tableRow > -1)
		{
			collectionName = dataTableGetString(QUEST__COMPLETED_COLLECTIONS, tableRow, collectionFaction);
		}
		
		if (collectionName == null)
		{
			return;
		}
		
		if (!hasCompletedCollectionSlot(player, collectionName))
		{
			modifyCollectionSlotValue(player, collectionName, 1);
		}
		
		return;
	}
}
