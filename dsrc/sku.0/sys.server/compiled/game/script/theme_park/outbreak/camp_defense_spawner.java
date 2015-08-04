package script.theme_park.outbreak;

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
import script.library.groundquests;
import script.library.stealth;
import script.library.utils;


public class camp_defense_spawner extends script.base_script
{
	public camp_defense_spawner()
	{
	}
	public static final boolean LOGGING_ON = true;
	
	public static final String SCRIPT_LOG = "outbreak_trigger";
	public static final String CLIENT_EFFECT = "appearance/pt_smoke_puff.prt";
	public static final String SPAWNER_OBJVAR = "spawnerObject";
	public static final String PATHNODE_OBJVAR = "pathObject";
	public static final String PATHNODE_PRIORITY = "pathPriority";
	public static final String SURVIVAL_TASK = "surviveTimer";
	public static final String MOB_TO_SPAWN = "outbreak_afflicted_defense";
	public static final String NUMBER_ENEMIES_VAR = "numberOfEnemies";
	public static final String LEVEL_ENEMIES_VAR = "maxLevelOfEnemies";
	public static final String ENEMY_LIST = "enemyList";
	
	public static final float SEARCH_RADIUS = 300f;
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() initializing a spawner for camp defense.");
		
		if (!hasObjVar(self, SPAWNER_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "findPathNodes", null, 3, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int startSpawning(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Initialized!");
		
		if ((params == null) || (params.isEmpty()))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Params invalid.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("player"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() player param not found.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("questName"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() questName param not found.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("combatLevel"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() questName param not found.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("ourSpawner"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() ourSpawner param not found.");
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, NUMBER_ENEMIES_VAR))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() number of enemies objvar not found.");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() validation completed.");
		
		int numberOfEnemies = getIntObjVar(self,NUMBER_ENEMIES_VAR);
		if (numberOfEnemies <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() number of enemies objvar not valid.");
			return SCRIPT_CONTINUE;
		}
		
		int maxLevelOfEnemies = getIntObjVar(self,LEVEL_ENEMIES_VAR);
		if (maxLevelOfEnemies <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() number of enemies objvar not valid.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		if (!isValidId(player) || !exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() player OID invalid.");
			return SCRIPT_CONTINUE;
		}
		String questName = params.getString("questName");
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() player "+player+" has received an invalid quest name.");
			return SCRIPT_CONTINUE;
		}
		int combatLevel = params.getInt("combatLevel");
		if (combatLevel < 0 || combatLevel > 90)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Player "+player+" has a level that is invalid. Player level is: "+combatLevel+". Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		
		if (combatLevel > maxLevelOfEnemies)
		{
			params.put("creatureLevel", maxLevelOfEnemies);
		}
		else
		{
			params.put("creatureLevel", combatLevel);
		}
		
		if (!groundquests.isQuestActive(player, questName))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Player "+player+" did not have the appropriate quest active. Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, questName, SURVIVAL_TASK))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Player "+player+" did not have the appropriate quest TASK active. Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.startSpawning() Spawn Loop starting.");
		
		int delay = 0;
		for (int i = 0; i < numberOfEnemies; i++)
		{
			testAbortScript();
			messageTo(self, "spawnDynamicEnemy", params, delay, false);
			delay += 5;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnDynamicEnemy(obj_id self, dictionary params) throws InterruptedException
	{
		if ((params == null) || (params.isEmpty()))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Params invalid.");
			return SCRIPT_CONTINUE;
			
		}
		if (!params.containsKey("player"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() player param not found.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("questName"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() questName param not found.");
			return SCRIPT_CONTINUE;
		}
		if (!params.containsKey("creatureLevel"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() creatureLevel param not found.");
			return SCRIPT_CONTINUE;
		}
		obj_id player = params.getObjId("player");
		if (!isValidId(player) || !exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() player OID invalid.");
			return SCRIPT_CONTINUE;
		}
		String questName = params.getString("questName");
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() player "+player+" has received an invalid quest name.");
			return SCRIPT_CONTINUE;
		}
		if (!groundquests.isQuestActive(player, questName))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Player "+player+" did not have the appropriate quest active. Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		if (!groundquests.isTaskActive(player, questName, SURVIVAL_TASK))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Player "+player+" did not have the appropriate quest TASK active. Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		int combatLevel = params.getInt("creatureLevel");
		if (combatLevel <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Player "+player+" did not have the appropriate combat level for spawner. Aborting.");
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "patrolList"))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() spawner, "+self+", did not have the path node script var. Aborting.");
			return SCRIPT_CONTINUE;
		}
		
		location patrolList[] = utils.getLocationArrayScriptVar(self, "patrolList");
		if (patrolList == null || patrolList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() spawner, "+self+", did not have valid path nodes. Aborting.");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() patrolList[0]: "+patrolList[0]);
		
		obj_id mob = create.object(MOB_TO_SPAWN, getLocation(self));
		if (!isValidId(mob) || !exists(mob))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Mob: "+MOB_TO_SPAWN+" could not be created!");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Mob level is set to match player level");
		
		setObjVar(mob, create.INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR, 1);
		
		String creatureName = getStringObjVar(mob, "creature_type");
		if (creatureName != null && creatureName.length() > 0)
		{
			dictionary creatureDict = utils.dataTableGetRow(CREATURE_TABLE, creatureName);
			if (creatureDict != null)
			{
				create.initializeCreature (mob, creatureName, creatureDict, combatLevel);
			}
		}
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() Mob level is set to match player level.");
		
		setInvulnerable(mob, false);
		clearCondition(mob, CONDITION_CONVERSABLE);
		setMovementRun(mob);
		setBaseRunSpeed(mob, (getBaseRunSpeed(self)-8));
		setAttributeAttained(mob, attrib.OUTBREAK_AFFLICTED);
		
		setObjVar(mob, "owner", player);
		setObjVar(mob, "questName", questName);
		setObjVar(mob, "survivalTaskName", SURVIVAL_TASK);
		setObjVar(mob, "cleanUpTaskName", "waitToCleanUpAll");
		
		attachScript(mob, "theme_park.outbreak.outbreak_defense_enemy");
		utils.setScriptVar(mob, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, patrolList);
		utils.setScriptVar(mob, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
		patrolOnce(mob, patrolList, 0);
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() the NPC is successfully pathing.");
		
		if (!utils.hasScriptVar(player, ENEMY_LIST))
		{
			Vector enemies = null;
			enemies = utils.addElement(enemies, mob);
			utils.setScriptVar(player, ENEMY_LIST, enemies);
		}
		else
		{
			Vector enemyVector = null;
			obj_id[] enemyArray = utils.getObjIdArrayScriptVar(player, ENEMY_LIST);
			Vector enemies = utils.concatArrays(enemyVector, enemyArray);
			if (enemies == null || enemies.size() <= 0)
			{
				CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.spawnDynamicEnemy() could not retrieve data from the current enemy list on the spawner: "+self);
				return SCRIPT_CONTINUE;
			}
			enemies = utils.addElement(enemies, mob);
			utils.setScriptVar(player, ENEMY_LIST, enemies);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int findPathNodes(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.findPathNodes() handler init.");
		if (!hasObjVar(self, SPAWNER_OBJVAR))
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() initializing failed. The spawner, "+self+" had no objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		String questName = getStringObjVar(self, SPAWNER_OBJVAR);
		if (questName == null || questName.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() initializing failed. The spawner, "+self+" didnt have a valid questName objvar. Location: "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] pathNodeList = getAllObjectsWithObjVar(getLocation(self), SEARCH_RADIUS, PATHNODE_OBJVAR);
		if (pathNodeList == null || pathNodeList.length <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() the spawner, "+self+" failed to find any valid path node objects within "+SEARCH_RADIUS+" of "+getLocation(self));
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.findPathNodes() pathNodeList.length: "+pathNodeList.length);
		
		Vector nodePtLocs = new Vector();
		nodePtLocs.setSize(0);
		
		for (int i = 0; i < pathNodeList.length; i++)
		{
			testAbortScript();
			if (!getStringObjVar(pathNodeList[i], PATHNODE_OBJVAR).equals(questName))
			{
				continue;
			}
			
			utils.addElement(nodePtLocs, getLocation(pathNodeList[i]));
		}
		
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.findPathNodes() nodePtLocs.length: "+nodePtLocs.size());
		if (nodePtLocs == null || nodePtLocs.size() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.OnInitialize() the spawner, "+self+" failed to get a list of nodes for all spawns to follow.");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("outbreak_themepark", "camp_defense_spawner.findPathNodes() nodePtLocs[0] "+((location)(nodePtLocs.get(0))));
		
		utils.setScriptVar(self, "patrolList", nodePtLocs);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(SCRIPT_LOG,msg);
		}
		return true;
	}
}
