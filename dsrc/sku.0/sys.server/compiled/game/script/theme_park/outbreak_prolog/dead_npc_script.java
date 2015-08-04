package script.theme_park.outbreak_prolog;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.attrib;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.utils;


public class dead_npc_script extends script.base_script
{
	public dead_npc_script()
	{
	}
	public static final String MENU_STRING_FILE = "theme_park/outbreak/outbreak";
	public static final String MENU_OBJ_VAR = "menu_string";
	public static final String NO_MENU_OBJ_VAR = "none";
	public static final String KEY_QUEST_INT_OBJVAR = "quest";
	public static final String SIGNAL_INT_OBJVAR = "signal";
	public static final String IMPERIAL_KEY_QUEST = "outbreak_quest_administrative_building_imperial_0";
	public static final String REBEL_KEY_QUEST = "outbreak_quest_administrative_building_rebel_0";
	public static final String NEUTRAL_KEY_QUEST = "outbreak_quest_administrative_building_neutral_0";
	public static final String NEUTRAL_SEARCH_QUEST = "outbreak_quest_facility_02_neutral";
	public static final String IMPERIAL_SEARCH_QUEST = "outbreak_quest_facility_02_imperial";
	public static final String REBEL_SEARCH_QUEST = "outbreak_quest_facility_02_rebel";
	public static final String FOUND_KEY_SIGNAL = "hasFoundKey";
	public static final String OUTBREAK_KEY_SEARCH = "search_body_outbreak_key";
	public static final String OUTBREAK_VIRUS_SEARCH = "scientist_body_searched";
	
	public static final string_id SID_YOU_FIND_NOTHING = new string_id(MENU_STRING_FILE, "you_find_nothing");
	public static final string_id SID_THIS_NOT_FOR_YOU = new string_id(MENU_STRING_FILE, "this_isnt_for_you");
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "knockDown", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		messageTo(self, "knockDown", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, MENU_OBJ_VAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		String menuObjVar = getStringObjVar(self, MENU_OBJ_VAR);
		if (menuObjVar == null || menuObjVar.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (menuObjVar.equals(NO_MENU_OBJ_VAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data data = mi.getMenuItemByType (menu_info_types.SERVER_MENU2);
		mi.addRootMenu(menu_info_types.SERVER_MENU2, new string_id(MENU_STRING_FILE, menuObjVar));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (isDead(player) || isIncapacitated(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, MENU_OBJ_VAR))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.SERVER_MENU2)
		{
			return SCRIPT_CONTINUE;
		}
		
		String menuObjVar = getStringObjVar(self, MENU_OBJ_VAR);
		if (menuObjVar == null || menuObjVar.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() menuObjVar: "+menuObjVar);
		
		if (menuObjVar.equals("content_biologist_01"))
		{
			if (!groundquests.isTaskActive(player, "quest_03_investigate_biolab", "getDataDisk"))
			{
				sendSystemMessage(player, SID_THIS_NOT_FOR_YOU);
				return SCRIPT_CONTINUE;
			}
			
			groundquests.sendSignal(player, "playerFoundDataDisk");
			return SCRIPT_CONTINUE;
		}
		else if (menuObjVar.equals(OUTBREAK_KEY_SEARCH))
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Key menu selected by Player: "+player);
			
			int questNumber = getIntObjVar(self, KEY_QUEST_INT_OBJVAR);
			if (questNumber < 1)
			{
				
				CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Quest Integer not found. Designer error for player: "+player);
				sendSystemMessage(player, SID_YOU_FIND_NOTHING);
				return SCRIPT_CONTINUE;
			}
			else if ((!groundquests.isQuestActive(player, IMPERIAL_KEY_QUEST+questNumber) && !groundquests.isQuestActive(player, REBEL_KEY_QUEST+questNumber) && !groundquests.isQuestActive(player, NEUTRAL_KEY_QUEST+questNumber)))
			{
				
				CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Quest Integer found: "+questNumber+ ". Player: "+player+" didn't have proper quest.");
				sendSystemMessage(player, SID_YOU_FIND_NOTHING);
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Quest Integer found: "+questNumber+ ". Player: "+player+" has quest and the quest is being updated with signal to player.");
			groundquests.sendSignal(player, FOUND_KEY_SIGNAL);
			spawnEnemy(self, player, getLocation(self));
			return SCRIPT_CONTINUE;
			
		}
		else if (menuObjVar.equals(OUTBREAK_VIRUS_SEARCH))
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() virus menu selected by Player: "+player);
			
			int signalNumber = getIntObjVar(self, SIGNAL_INT_OBJVAR);
			if (signalNumber < 1)
			{
				
				CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Signal Integer not found. Designer error for player: "+player);
				sendSystemMessage(player, SID_YOU_FIND_NOTHING);
				return SCRIPT_CONTINUE;
			}
			else if ((!groundquests.isTaskActive(player, NEUTRAL_SEARCH_QUEST, "waitForTasks") && !groundquests.isTaskActive(player, IMPERIAL_SEARCH_QUEST, "waitForTasks") && !groundquests.isTaskActive(player, REBEL_SEARCH_QUEST, "waitForTasks")))
			{
				
				CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Quest not found for Player: "+player);
				sendSystemMessage(player, SID_YOU_FIND_NOTHING);
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.OnObjectMenuSelect() Quest task found for Player: "+player);
			groundquests.sendSignal(player, OUTBREAK_VIRUS_SEARCH+"_"+signalNumber);
			return SCRIPT_CONTINUE;
			
		}
		sendSystemMessage(player, SID_YOU_FIND_NOTHING);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int knockDown(obj_id self, dictionary params) throws InterruptedException
	{
		ai_lib.aiSetPosture(self, POSTURE_KNOCKED_DOWN);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean spawnEnemy(obj_id self, obj_id player, location spawnLoc) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Initialized!");
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		if (spawnLoc == null)
		{
			return false;
		}
		
		if (!hasObjVar(self, "creature_type"))
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() No creature name var on object: "+self);
			return false;
		}
		
		String creatureName = getStringObjVar(self,"creature_type");
		if (creatureName == null || creatureName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Creature var INVALID on object: "+self);
			return false;
		}
		
		int spawnCount = 1;
		if (hasObjVar(self, "spawnCount"))
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() creature spawn count var is overwriting default value of 1");
			spawnCount = getIntObjVar(self,"spawnCount");
			return false;
		}
		
		int combatLevel = getLevel(player);
		if (combatLevel < 0 || combatLevel > 90)
		{
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Player "+player+" has a level that is invalid. Player level is: "+combatLevel+". Aborting the camp defense.");
			return false;
		}
		
		for (int i = 0; i < spawnCount; i++)
		{
			testAbortScript();
			
			obj_id mob = create.object(creatureName + "_undead", spawnLoc);
			if (!isValidId(mob) || !exists(mob))
			{
				CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Mob: "+mob+" could not be created!");
				return false;
			}
			
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Mob level is set to match player level");
			
			setObjVar(mob, create.INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR, 1);
			
			dictionary creatureDict = utils.dataTableGetRow(CREATURE_TABLE, creatureName);
			if (creatureDict != null)
			{
				create.initializeCreature (mob, creatureName, creatureDict, combatLevel);
			}
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() Mob level is set to match player level.");
			
			setInvulnerable(mob, false);
			clearCondition(mob, CONDITION_CONVERSABLE);
			setMovementRun(mob);
			setBaseRunSpeed(mob, (getBaseRunSpeed(self)-8));
			setAttributeAttained(mob, attrib.OUTBREAK_AFFLICTED);
			setObjVar(mob, "owner", player);
			
			attachScript(mob, "theme_park.outbreak.dynamic_enemy");
			startCombat(mob, player);
			CustomerServiceLog("outbreak_themepark", "dead_npc_script.spawnEnemy() the NPC is attacking player: "+player);
		}
		
		return true;
	}
}
