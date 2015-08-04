package script.systems.treasure_map.base;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.buff;
import script.library.combat;
import script.library.groundquests;
import script.library.locations;
import script.library.stealth;
import script.library.sui;
import script.library.utils;


public class treasure_map extends script.base_script
{
	public treasure_map()
	{
	}
	public static final string_id SID_USE = new string_id("treasure_map/treasure_map", "use");
	public static final string_id SID_SEARCH_AREA = new string_id("treasure_map/treasure_map", "search_area");
	public static final string_id SID_EXTRACT_TREASURE = new string_id("treasure_map/treasure_map", "extract_treasure");
	public static final string_id SID_SYS_NO_REGIONS = new string_id("treasure_map/treasure_map", "sys_no_regions");
	public static final string_id SID_SYS_STORE_WAYPOINT = new string_id("treasure_map/treasure_map", "sys_store_waypoint");
	public static final string_id SID_SYS_WAYPOINT_EXISTS = new string_id("treasure_map/treasure_map", "sys_waypoint_exists");
	public static final string_id SID_SYS_DIST_HERE = new string_id("treasure_map/treasure_map", "sys_dist_here");
	public static final string_id SID_SYS_DIST_NEAR = new string_id("treasure_map/treasure_map", "sys_dist_near");
	public static final string_id SID_SYS_DIST_FAR = new string_id("treasure_map/treasure_map", "sys_dist_far");
	public static final string_id SID_SYS_NOT_IN_INV = new string_id("treasure_map/treasure_map", "sys_not_in_inv");
	public static final string_id SID_SYS_NO_WAYPOINT = new string_id("treasure_map/treasure_map", "sys_no_waypoint");
	public static final string_id SID_SYS_START_PINPOINT = new string_id("treasure_map/treasure_map", "sys_start_pinpoint");
	public static final string_id SID_SYS_CANT_PINPOINT = new string_id("treasure_map/treasure_map", "sys_cant_pinpoint");
	public static final string_id SID_SYS_PINPOINT = new string_id("treasure_map/treasure_map", "sys_pinpoint");
	public static final string_id SID_SYS_FOUND = new string_id("treasure_map/treasure_map", "sys_found");
	public static final string_id SID_SYS_EXTRACTING = new string_id("treasure_map/treasure_map", "sys_extracting");
	public static final string_id SID_SYS_NO_COMBAT = new string_id("treasure_map/treasure_map", "sys_no_combat");
	public static final string_id SID_SYS_NO_MOUNT = new string_id("treasure_map/treasure_map", "sys_no_mount");
	public static final string_id SID_SYS_NO_DEAD_INCAP = new string_id("treasure_map/treasure_map", "sys_no_dead_incap");
	public static final string_id SID_SYS_TREASURE_TIME_LIMIT = new string_id("treasure_map/treasure_map", "sys_time_limit");
	
	public static final String SID_STORE_WAYPOINT = "@treasure_map/treasure_map:store_waypoint";
	public static final String SID_CLOSE = "@treasure_map/treasure_map:close";
	public static final String SCRIPT_DUMMY_CHEST = "systems.treasure_map.base.dummy_treasure_drum";
	public static final String TREASURE_TABLE = "datatables/treasure_map/treasure_map.iff";
	public static final String DUMMY_CHEST = "object/tangible/container/drum/nonopening_treasure_drum.iff";
	public static final String LOOT_TABLE_81_90 = "treasure/treasure_81_90";
	
	public static final int EXPLOITER_RANGE = 64;
	public static final int MIN_DISTANCE_TO_MAP_OWNER = 300;
	public static final int MAX_DISTANCE_FOR_SEARCH = 16;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		obj_id map = self;
		messageTo(map, "getMapTreasurePlanet", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		
		obj_id map = self;
		obj_id player = getObjIdObjVar(map, "owner");
		if (!isIdValid(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id chest = getObjIdObjVar(map, "chest");
		if (!isIdValid(chest) || !exists(chest))
		{
			obj_id mapWaypoint = getObjIdObjVar(map, "treasureMapWaypoint");
			if (isIdValid(mapWaypoint))
			{
				
				destroyWaypointInDatapad(mapWaypoint, player);
				removeObjVar(chest, "treasureMapWaypoint");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to retrieve a menu for player "+ player + ". [ OnObjectMenuRequest() ]");
			sendSystemMessage(player, "The treasure map failed to retrieve the menu correctly.", null);
			return SCRIPT_CONTINUE;
		}
		
		obj_id map = self;
		if (!hasObjVar( map, "pinpoint"))
		{
			
			if (!isValidId(getWaypoint(map, player)))
			{
				mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE);
			}
			else
			{
				mi.addRootMenu (menu_info_types.ITEM_USE, SID_SEARCH_AREA);
				setObjVar( map, "searchArea", true );
			}
		}
		else
		{
			if (!isValidId(getWaypoint(map, player)))
			{
				mi.addRootMenu (menu_info_types.ITEM_USE, SID_USE);
			}
			else
			{
				mi.addRootMenu (menu_info_types.ITEM_USE, SID_EXTRACT_TREASURE);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isIdValid(player) && item == -1)
		{
			CustomerServiceLog("treasureMap", "a treasure map trigger failed for player "+ player + ". [ OnObjectMenuSelect() ]");
			sendSystemMessage(player, "The treasure map failed to select the menu correctly.", null);
			return SCRIPT_CONTINUE;
		}
		
		obj_id map = self;
		sendDirtyObjectMenuNotification(map);
		if (item == menu_info_types.ITEM_USE)
		{
			if (hasObjVar(map, "pinpoint") && hasObjVar(map, "searchArea"))
			{
				doDebugLogging("treasureMap", "Player has pinpoint and searchArea");
				
				boolean statePass = checkState(player);
				if (!statePass)
				{
					return SCRIPT_CONTINUE;
				}
				
				boolean planetCheck = checkScene(player, map);
				if (!planetCheck)
				{
					return SCRIPT_CONTINUE;
				}
				
				extractTreasure(map, player);
				
				return SCRIPT_CONTINUE;
			}
			else if (!hasObjVar(map, "pinpoint") && hasObjVar(map, "searchArea") && isValidId(getWaypoint(map, player)))
			{
				doDebugLogging("treasureMap", "Player has just searchArea");
				
				boolean statePass = checkState(player);
				if (!statePass)
				{
					return SCRIPT_CONTINUE;
				}
				
				boolean planetCheck = checkScene(player, map);
				if (!planetCheck)
				{
					return SCRIPT_CONTINUE;
				}
				
				searchArea(map, player);
				
				return SCRIPT_CONTINUE;
			}
			else
			{
				doDebugLogging("treasureMap", "player has no objvars or has deleted waypoint");
				
				displayDialog( map, player );
				
				return SCRIPT_CONTINUE;
			}
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id getWaypoint(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not retrieve a waypoint for player because the map OID was not passed properly. [ getWaypoint() ]");
			sendSystemMessage(player, "The treasure map failed to get a waypoint.", null);
			return null;
		}
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not retrieve a waypoint for player because the player OID was not passed properly. [ getWaypoint() ]");
			return null;
		}
		
		location treasureLoc = getLocationObjVar(map, "treasureLoc");
		
		obj_id[] data = getWaypointsInDatapad(player);
		if (data != null)
		{
			for (int i=0; i<data.length; i++)
			{
				testAbortScript();
				if (isIdNull(data[i]))
				{
					continue;
				}
				location waypointLoc = getWaypointLocation(data[i]);
				if ((waypointLoc != null) && (waypointLoc.equals(treasureLoc)))
				{
					
					doDebugLogging("treasureMap", "Waypoint found ");
					return data[i];
					
				}
			}
		}
		return null;
	}
	
	
	public void displayDialog(obj_id map, obj_id player) throws InterruptedException
	{
		if (utils.hasScriptVar(map,"suiOpen"))
		{
			return;
		}
		
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to view dialog for player because the map OID was not passed properly. [ displayDialog() ]");
			sendSystemMessage(player, "The treasure map failed to display dialog properly.", null);
			return;
		}
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to view dialog for player because the player OID was not passed properly. [ displayDialog() ]");
			return;
		}
		
		obj_id inventory = getObjectInSlot( player, "inventory");
		if (!contains( inventory, map ))
		{
			sendSystemMessage( player, SID_SYS_NOT_IN_INV );
			return;
		}
		
		if (!dataTableOpen( TREASURE_TABLE ))
		{
			return;
		}
		
		String entryName = getStringObjVar(map, "mob");
		
		String text = "@treasure_map/treasure_map:text_"+ entryName;
		String title = "@treasure_map/treasure_map:title_"+ entryName;
		
		if (hasObjVar( map, "map_text" ))
		{
			text = "@treasure_map/treasure_map:"+ getStringObjVar( map, "map_text");
		}
		if (hasObjVar( map, "map_title" ))
		{
			title = "@treasure_map/treasure_map:"+ getStringObjVar( map, "map_title");
		}
		
		createDialog( map, player, text, title );
	}
	
	
	public int createDialog(obj_id map, obj_id player, String text, String title) throws InterruptedException
	{
		
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create map dialog because no player OID was passed. [ createDialog() ]");
			return -1;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create map dialog because no map OID was passed. [ createDialog() ]");
			sendSystemMessage(player, "The treasure map failed to create dialog properly.", null);
			return -1;
		}
		if (text == null || text.equals(""))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create dialog because no map text was passed. [ createDialog() ]");
			sendSystemMessage(player, "The treasure map failed to attain the dialog text properly.", null);
			return -1;
		}
		if (title == null || title.equals(""))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create a dialog title because no title text was passed. [ createDialog() ]");
			sendSystemMessage(player, "The treasure map failed to attain the dialog title properly.", null);
			return -1;
		}
		
		utils.setScriptVar(map, "suiOpen", true);
		
		int pid = sui.createSUIPage( sui.SUI_MSGBOX, map, player, "handleDialogInput");
		
		sui.setSUIProperty( pid, sui.MSGBOX_PROMPT, sui.PROP_TEXT, text );
		sui.setSUIProperty (pid, sui.MSGBOX_TITLE, sui.PROP_TEXT, title );
		
		sui.msgboxButtonSetup( pid, sui.OK_CANCEL );
		sui.setSUIProperty( pid, sui.MSGBOX_BTN_CANCEL, sui.PROP_TEXT, SID_CLOSE);
		sui.setSUIProperty( pid, sui.MSGBOX_BTN_OK, sui.PROP_TEXT, SID_STORE_WAYPOINT);
		
		sui.showSUIPage( pid );
		return pid;
	}
	
	
	public int handleDialogInput(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id map = self;
		if ((params == null) || (params.isEmpty()))
		{
			
			removeObjVar(map, "pinpoint");
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId( params );
		int bp = sui.getIntButtonPressed( params );
		switch ( bp )
		{
			
			case sui.BP_OK: 	
			boolean waypointStored = storeWaypoint(map, player);
			if (waypointStored)
			{
				setObjVar( map, "searchArea", true );
			}
			
			utils.removeScriptVar(map, "suiOpen");
			return SCRIPT_CONTINUE;
			
			case sui.BP_CANCEL: 
			utils.removeScriptVar(map, "suiOpen");
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean storeWaypoint(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to store a waypoint because no player OID was passed. [ storeWaypoint() ]");
			return false;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to store a waypoint because no waypoint OID was passed. [ storeWaypoint() ]");
			sendSystemMessage(player, "The treasure map failed to create a waypoint.", null);
			return false;
		}
		
		String req_planet = getStringObjVar(map, "planet");
		
		if (req_planet == null || req_planet.equals(""))
		{
			
			CustomerServiceLog("treasureMap", "a treasure map failed to find the planet variable for player "+ player + " "+ getName(player)+". It is probably an old legacy treasure map. [ storeWaypoint() ]");
			sendSystemMessage(player, "This map is so old that when you attempt to get the waypoint the disk fails. The disk doesn't seem to function properly.", null);
			return false;
		}
		
		String scene = getCurrentSceneName();
		
		if (scene == null || scene.equals(""))
		{
			scene = "tatooine";
			CustomerServiceLog("treasureMap", "a treasure map failed to find the current scene for player "+ player + ". [ storeWaypoint() ]");
			sendSystemMessage(player, "The treasure map failed to attain the correct planet scene, Tatooine is now the current scene.", null);
		}
		
		if (!scene.equals(req_planet) && !(isGod(player)))
		{
			
			String requiredPlanet = localize(new string_id("planet_n", req_planet));
			sendSystemMessage(player, "You need to travel to "+ requiredPlanet + " before receiving the waypoint.", null);
			return false;
		}
		
		location locationMapLocation = getMapLocation(map, player);
		if (locationMapLocation != null && !locationMapLocation.equals(""))
		{
			
			if (isValidId(getWaypoint(map, player)) && exists(getWaypoint(map, player)))
			{
				sendSystemMessage( player, SID_SYS_WAYPOINT_EXISTS );
				return true;
			}
			else if (isGod(player))
			{
				sendSystemMessage(player, "Since you are in GODMODE, the treasure map will be on this planet. Test the treaure planet variable by getting the waypoint while not in GODMODE.", null);
				
				CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
				+ " is godplayer using map: "+ map
				+ " to make the map waypoint on their current planet location as opposed to: "+ locationMapLocation
				+". [ storeWaypoint() ]");
			}
			
			boolean boolSuccessWaypointCreation = createMapWaypointAtLocation(map, player, locationMapLocation);
			return true;
		}
		else
		{
			CustomerServiceLog("treasureMap", "a treasure map "+ map + "failed to attain a valid location for player "+ player + ". [ storeWaypoint() ]");
			sendSystemMessage(player, "The treasure map failed to attain a valid location.", null);
		}
		
		return false;
	}
	
	
	public void searchArea(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to search the waypoint because no player OID was passed. [ searchArea() ]");
			return;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to search the waypoint for exact treasure location with player "+ player + ". [ searchArea() ]");
			sendSystemMessage(player, "The treasure map failed. You cannot search at this time.", null);
			return;
		}
		
		boolean inPlayerInventory = utils.isNestedWithin(map, player);
		if (!inPlayerInventory)
		{
			sendSystemMessage( player, SID_SYS_NOT_IN_INV );
			return;
		}
		
		if (!hasObjVar( map, "treasureLoc" ) || (!isValidId(getWaypoint(map, player))))
		{
			sendSystemMessage( player, SID_SYS_NO_WAYPOINT );
			return;
		}
		
		sendSystemMessage( player, SID_SYS_START_PINPOINT );
		dictionary params = new dictionary();
		params.put("player", player);
		messageTo( map, "finishSearchArea", params, 5, false );
		
	}
	
	
	public int finishSearchArea(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id map = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		obj_id player = params.getObjId("player");
		
		obj_id inventory = getObjectInSlot(player, "inventory");
		if (!contains( inventory, map))
		{
			sendSystemMessage(player, SID_SYS_NOT_IN_INV);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(map, "treasureLoc") || (!isValidId(getWaypoint(map, player))))
		{
			sendSystemMessage( player, SID_SYS_NO_WAYPOINT );
			return SCRIPT_CONTINUE;
		}
		
		location treasureLoc = getLocationObjVar( map, "treasureLoc");
		location curLoc = getLocation( player );
		float dist = utils.getDistance2D(treasureLoc, curLoc);
		if (dist <= MAX_DISTANCE_FOR_SEARCH && dist >= 0)
		{
			
			obj_id waypoint = getWaypoint( map, player );
			
			treasureLoc = groundquests.getRandom2DLocationAroundLocation(player, 1, 1, 10, 40);
			sendSystemMessage( player, SID_SYS_PINPOINT );
			setWaypointLocation( waypoint, treasureLoc );
			setObjVar( map, "treasureLoc", treasureLoc );
			
			setObjVar( map, "pinpoint", true );
		}
		else
		{
			
			sendSystemMessage( player, SID_SYS_CANT_PINPOINT );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void extractTreasure(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to extract the treasure because no player OID was passed. [ extractTreasure() ]");
			return;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map would not allow the player to extract the treasure for player "+ player + ". [ extractTreasure() ]");
			sendSystemMessage(player, "The treasure map failed. You cannot extract the treasure.", null);
			return;
		}
		
		obj_id inventory = getObjectInSlot( player, "inventory");
		if (!contains( inventory, map ))
		{
			sendSystemMessage( player, SID_SYS_NOT_IN_INV );
			return;
		}
		
		if (!hasObjVar( map, "treasureLoc" ) || (getWaypoint( map, player ) == null))
		{
			sendSystemMessage( player, SID_SYS_NO_WAYPOINT );
			return;
		}
		
		if (hasObjVar( map, "extracting" ))
		{
			sendSystemMessage( player, SID_SYS_EXTRACTING );
			return;
		}
		
		location treasureLoc = getLocationObjVar( map, "treasureLoc");
		location curLoc = getLocation(player);
		float dist = utils.getDistance2D( treasureLoc, curLoc );
		if (dist <= 8 && dist >= 0)
		{
			
			sendSystemMessage( player, SID_SYS_DIST_HERE );
			
			CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
			+ " is extracting a chest using: "+ map
			+ " at location "+ getLocation(player) + ". [ extractTreasure() ]");
			
			dictionary params = new dictionary();
			params.put( "player", player );
			messageTo( map, "spawnTreasure", params, 2, false );
			
			setObjVar( map, "extracting", true );
			
		}
		else if (dist <= 30)
		{
			
			sendSystemMessage( player, SID_SYS_DIST_NEAR );
		}
		else
		{
			
			sendSystemMessage( player, SID_SYS_DIST_FAR );
		}
	}
	
	
	public int spawnTreasure(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id map = self;
		if ((params == null) || (params.isEmpty()))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId( "player");
		boolean paramsReceived = getTreasureParams(map, player);
		
		if (!paramsReceived)
		{
			CustomerServiceLog("treasureMap", "TREASURE MAP ERROR: Player: "+ player + " "+ getName(player)
			+ " could not spawn a treasure chest at location: "+ getLocation(player)
			+ ". Please notify development. [ spawnTreasure() ]");
			
			doDebugLogging("treasureMap", "a treasure map failed to spawn for player: "+player+". [ spawnTreasure() ]");
			sendSystemMessage(player, "You could not extract the chest. Notify Customer Service.", null);
			
			return SCRIPT_CONTINUE;
		}
		
		int treasureLevel = getIntObjVar(map, "max");
		
		removeObjVar(map, "extracting");
		
		sendSystemMessage( player, SID_SYS_FOUND );
		sendSystemMessage(player, SID_SYS_TREASURE_TIME_LIMIT);
		
		obj_id waypoint = getWaypoint( map, player );
		if (isIdNull(waypoint))
		{
			destroyObject( waypoint );
		}
		
		location playerLocation = getLocation(player);
		playerLocation.y = getHeightAtLocation( playerLocation.x, playerLocation.z );
		
		obj_id treasureChest = createObject(DUMMY_CHEST, playerLocation);
		if (!isValidId(treasureChest))
		{
			CustomerServiceLog("treasureMap", "TREASURE MAP ERROR: Player: "+ player + " "+ getName(player)
			+ " could not spawn a treasure chest at location: "+ playerLocation
			+ ". Please notify development. [ spawnTreasure() ]");
			
			doDebugLogging("treasureMap", "a treasure map failed to spawn for player: "+player+". [ spawnTreasure() ]");
			sendSystemMessage(player, "You could not extract the chest. Notify Customer Service.", null);
			
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(treasureChest, "owner", player);
		setOwner(treasureChest, player);
		
		String invis = stealth.getInvisBuff(player);
		if (invis != null)
		{
			stealth.checkForAndMakeVisibleNoRecourse(player);
		}
		
		attachScript(treasureChest, SCRIPT_DUMMY_CHEST);
		
		setObjVar(map, "chest", treasureChest);
		
		setObjVar(treasureChest, "treasureLevel", treasureLevel);
		
		if (hasObjVar(map, "treasureMapWaypoint"))
		{
			obj_id treasureMapWaypoint = getObjIdObjVar(map, "treasureMapWaypoint");
			setObjVar(treasureChest, "treasureMapWaypoint", treasureMapWaypoint);
		}
		
		String loot_table = getStringObjVar(map, "loot_table");
		setObjVar(treasureChest, "loot_table", loot_table);
		String type = getStringObjVar(map, "mob");
		int mobLevel = getIntObjVar(map, "mobLevel");
		int groupModifier = getIntObjVar(map, "groupModifier");
		int count = getIntObjVar(map, "count");
		int dataTableIdx = getIntObjVar(map, "dataTableIdx");
		
		boolean verifiedAllVars = verifyVariablesNotNull(loot_table, type, mobLevel, groupModifier, count, dataTableIdx);
		if (!verifiedAllVars)
		{
			CustomerServiceLog("treasureMap", "TREASURE MAP ERROR: Player: "+ player + " "+ getName(player)
			+ " could not spawn guards correctly because data taken off the map: "+ map
			+ " was incomplete, corrupted or invalid. [ spawnTreasure() ]");
			
			doDebugLogging("treasureMap", "a treasure map failed to retrieve all data necessary to spawn treasure for player: "+player+". [ spawnTreasure() ]");
			sendSystemMessage(player, "The treasure map failed to attain treasure guard data. Notify Customer Service.", null);
		}
		
		String boss_mob = checkForBossMobSpawn(player, mobLevel, groupModifier, dataTableIdx, loot_table);
		
		dictionary outparams = new dictionary();
		outparams.put("loot_table", loot_table);
		outparams.put("count", count);
		outparams.put("type", type);
		outparams.put("mobLevel", mobLevel);
		outparams.put("location", playerLocation);
		outparams.put("player", player);
		outparams.put("boss_mob", boss_mob);
		outparams.put("groupModifier", groupModifier);
		
		doDebugLogging("treasureMap", "loot_table: "+ loot_table 
		+ " count: "+ count 
		+ " type: "+ type
		+ "player: "+ player
		+ "treasureLoc: "+ playerLocation
		+ "boss_mob: "+ boss_mob 	
		);
		
		CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player) 
		+ " spawned treasure chest: "+ treasureChest 
		+ " at location: "+ getLocation(treasureChest)
		+ " with "+ count 
		+ " mobs of type: "+ type
		+ " and their level being: "+ mobLevel 
		+". [ spawnTreasure() ]");
		
		messageTo(treasureChest, "spawnTreasureGuards", outparams, 0, false);
		
		CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
		+ " extracted the treasure chest at location: "+ playerLocation
		+ " and the map:"+ map 
		+ " was destroyed. [ spawnTreasure() ]");
		
		doDebugLogging("treasureMap", "map: "+ map + " destroyed. [ spawnTreasure() ]");
		
		destroyObject(map);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getTreasureParams(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the treasure parameters because the player OID was not passed. [ getTreasureParams() ]");
			return false;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the treasure parameters because the map OID was not passed. [ getTreasureParams() ]");
			sendSystemMessage(player, "The treasure map failed to get the loot table and group modifier.", null);
			return false;
		}
		
		if (!hasObjVar(map, "max"))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the treasure parameters because the max variable was not found on the map. [ getTreasureParams() ]");
			sendSystemMessage(player, "The treasure map failed to find the map max level.", null);
			return false;
		}
		
		boolean successPlayerLvl = setPlayerGroupLevel(map, player);
		if (!successPlayerLvl)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to retrieve the player combat level. [ getTreasureParams() ]");
			sendSystemMessage(player, "The treasure map failed to retrieve your combat level.", null);
			return false;
		}
		
		int intMaxLevel = getIntObjVar(map, "max");
		int intMinLevel = getIntObjVar(map, "min");
		int dataTableIdx = getIntObjVar(map, "dataTableIdx");
		
		if (dataTableIdx == -1)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to retrieve the datatable index. This is possibly due to the datatable being changed or corrupted . [ getTreasureParams() ]");
			sendSystemMessage(player, "The treasure map failed to retrieve a data index.", null);
			return false;
		}
		
		int count = 0;
		
		String type = null;
		dictionary params = dataTableGetRow(TREASURE_TABLE, dataTableIdx);
		if (params.isEmpty() || params == null)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the treasure loot table and group modifier for player "+ player + ". [ getTreasureParams() ]");
			sendSystemMessage(player, "The treasure map failed to get the loot table and group modifier.", null);
			return false;
		}
		
		setObjVar(map, "loot_table", params.getString("loot_table"));
		
		if (!hasObjVar(map, "count"))
		{
			count = rand(params.getInt("mob_count_min"), params.getInt("mob_count_max"));
			if (hasObjVar(map, "playerLevel"))
			{
				int playerLevel = getIntObjVar(map, "playerLevel");
				
				int mobLevelModifier = params.getInt("mob_level_modifier");
				
				int mobLevel = getMobLevel(map, player, playerLevel, intMaxLevel, intMinLevel, mobLevelModifier);
				setObjVar(map, "mobLevel", mobLevel);
				if (hasObjVar(map, "groupModifier"))
				{
					count = getEnemyReCount(player, map, count);
				}
			}
			setObjVar(map, "count", count);
		}
		return true;
	}
	
	
	public boolean setPlayerGroupLevel(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to set the group level because the player OID was not passed. [ setPlayerGroupLevel() ]");
			return false;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to set the group level because the map OID was not passed. [ setPlayerGroupLevel() ]");
			sendSystemMessage(player, "The treasure map failed to set the group level.", null);
			return false;
		}
		
		int intPlayerLevel = getLevel(player);
		if (intPlayerLevel == -1|| intPlayerLevel == 0)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to use the player's correct combat level. [ setPlayerGroupLevel() ]");
			sendSystemMessage(player, "The treasure map failed to retrieve your current level.", null);
			
			intPlayerLevel = 5;
			return false;
		}
		
		location ownerLocation = getLocation(player);
		String ownerArea = ownerLocation.area;
		obj_id groupId = getGroupObject(player);
		if (isValidId(groupId))
		{
			obj_id[] groupOids = getGroupMemberIds(groupId);
			
			int finalCount = 1;
			
			for (int i = 0; i < groupOids.length; i++)
			{
				testAbortScript();
				if (!isValidId(groupOids[i]) || !exists(groupOids[i]))
				{
					continue;
				}
				
				if (groupOids[i] == player)
				{
					continue;
				}
				
				location groupMemberLocation = getLocation(groupOids[i]);
				if (groupMemberLocation == null)
				{
					continue;
				}
				
				String groupMemberArea = groupMemberLocation.area;
				if (!ownerArea.equals(groupMemberArea))
				{
					continue;
				}
				
				float dist = utils.getDistance2D(ownerLocation, groupMemberLocation);
				if (dist > MIN_DISTANCE_TO_MAP_OWNER)
				{
					continue;
				}
				
				int groupMemberLevel = getLevel(groupOids[i]);
				
				if (groupMemberLevel > intPlayerLevel)
				{
					intPlayerLevel = groupMemberLevel;
				}
				
				finalCount++;
			}
			
			if (finalCount > 0 && finalCount <= 8)
			{
				setObjVar(map, "groupModifier", finalCount);
			}
			else if (finalCount > 8)
			{
				
				setObjVar(map, "groupModifier", 8);
			}
		}
		
		else
		{
			removeObjVar(map, "groupModifier");
		}
		
		setObjVar(map, "playerLevel", intPlayerLevel);
		return true;
	}
	
	
	public int findAmbushNearBy(obj_id map, obj_id player, int intPlayerLevel, location ownerLocation) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			return 0;
		}
		if (intPlayerLevel < 0)
		{
			return 0;
		}
		if (ownerLocation == null)
		{
			return 0;
		}
		
		int exploiterLevel = intPlayerLevel;
		int exploiterGroup = 0;
		
		obj_id[] playersNear = getAllObjectsWithScript(ownerLocation, EXPLOITER_RANGE, "player.base.base_player");
		if (playersNear == null || playersNear.length == 0)
		{
			return 0;
		}
		
		for (int i = 0; i < playersNear.length; i++)
		{
			testAbortScript();
			if (!isValidId(playersNear[i]) || !exists(playersNear[i]))
			{
				continue;
			}
			
			if (!isPlayer(playersNear[i]))
			{
				continue;
			}
			
			if (playersNear[i] == player)
			{
				continue;
			}
			
			if (getLocomotion(playersNear[i]) == 3)
			{
				continue;
			}
			
			if (i == 14)
			{
				break;
			}
			
			int playerNearLevel = getLevel(playersNear[i]);
			if (playerNearLevel > intPlayerLevel)
			{
				exploiterLevel = playerNearLevel;
				exploiterGroup =+ 1;
			}
		}
		
		int groupModifier = getIntObjVar(map, "groupModifier");
		if (groupModifier > 0)
		{
			exploiterGroup =+ groupModifier;
		}
		
		if (exploiterGroup > 8)
		{
			exploiterGroup = 8;
		}
		
		sendSystemMessage(player, "exploiterGroup:"+exploiterGroup, null);
		sendSystemMessage(player, "groupModifier:"+groupModifier, null);
		
		if (exploiterGroup > 0 && exploiterGroup > groupModifier)
		{
			setObjVar(map, "groupModifier", exploiterGroup);
		}
		
		return exploiterLevel;
	}
	
	
	public int getEnemyReCount(obj_id player, obj_id map, int intCurrentCount) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to recount the enemies before treasure extraction because the player OID was not received. [ getEnemyReCount() ]");
			return -1;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to recount the enemies before treasure extraction because the map OID was not received. [ getEnemyReCount() ]");
			sendSystemMessage(player, "The treasure map failed to recount enemies.", null);
			return -1;
		}
		if (intCurrentCount == -1)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to recount the enemies before treasure extraction because the enemy count was -1. [ getEnemyReCount() ]");
			sendSystemMessage(player, "The treasure map failed to recount enemies due to incorrect enemy count variable.", null);
			return -1;
		}
		
		int groupRecount = 0;
		int finalCount = 0;
		
		if (hasObjVar(map, "groupModifier"))
		{
			
			int modifier = getIntObjVar(map, "groupModifier");
			if (modifier >= 2)
			{
				if (modifier <= 4)
				{
					groupRecount = modifier+2;
				}
				else
				{
					groupRecount = rand(modifier, modifier+1);
				}
			}
		}
		if (groupRecount > intCurrentCount)
		{
			return groupRecount;
		}
		
		return intCurrentCount;
	}
	
	
	public location getMapLocation(obj_id map, obj_id player) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to recount the enemies before treasure extraction because the player OID was not received. [ getEnemyReCount() ]");
			return null;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to recount the enemies before treasure extraction because the map OID was not received. [ getEnemyReCount() ]");
			sendSystemMessage(player, "The treasure map failed to recount enemies.", null);
			return null;
		}
		
		location treasureLoc = null;
		location storedLoc = getLocationObjVar( map, "treasureLoc");
		
		if (storedLoc == null)
		{
			setObjVar(map, "owner", player);
			setObjVar(map, "noTrade", true);
			
			region[] nearRegions = getRegionsAtPoint( getLocation( player ) );
			if (nearRegions == null)
			{
				
				sendSystemMessage( player, SID_SYS_NO_REGIONS );
				destroyObject( map );
				return null;
			}
			
			region closestCity = locations.getClosestCityRegion( nearRegions[0] );
			boolean goodLoc = false;
			for (int i=0; (i < 30)&&!goodLoc; i++)
			{
				testAbortScript();
				
				treasureLoc = locations.getGoodLocationOutsideOfRegion(closestCity, 1, 1, 8000);
				
				if (!isValidLocation(treasureLoc, 10.f) || treasureLoc == null || locations.isInCity(treasureLoc))
				{
					continue;
				}
				goodLoc = true;
			}
			
			if (treasureLoc == null)
			{
				return null;
			}
			
			setObjVar( map, "treasureLoc", treasureLoc );
		}
		else
		{
			treasureLoc = (location)storedLoc.clone();
		}
		return treasureLoc;
	}
	
	
	public boolean createMapWaypointAtLocation(obj_id map, obj_id player, location locationMapLocation) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create a waypoint because the player OID was not passed. [ getMobLevel() ]");
			return false;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create a waypoint for player "+ player + " because the map object was not passed. [ createMapWaypointAtLocation() ]");
			sendSystemMessage(player, "The treasure map failed to attain a waypoint because there was no map found.", null);
			return false;
		}
		if (locationMapLocation == null)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create a waypoint for player "+ player + " because the map object was not passed. [ createMapWaypointAtLocation() ]");
			sendSystemMessage(player, "The treasure map failed to attain a waypoint because there was no map found.", null);
			return false;
		}
		
		obj_id waypoint = createWaypointInDatapad(player, locationMapLocation);
		if (!isIdValid(waypoint))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to create a waypoint for player "+ player + " because the waypoint couldn't be created in the player datapad. [ createMapWaypointAtLocation() ]");
			sendSystemMessage(player, "The treasure map failed to create a waypoint in your datapad.", null);
			return false;
		}
		
		setWaypointVisible(waypoint, true);
		setWaypointActive(waypoint, true);
		setWaypointName(waypoint, "Treasure Location");
		
		setObjVar(map, "treasureMapWaypoint", waypoint);
		
		sendSystemMessage( player, SID_SYS_STORE_WAYPOINT );
		return true;
	}
	
	
	public int getMobLevel(obj_id map, obj_id player, int playerLevel, int intMaxLevel, int intMinLevel, int mobLevelModifier) throws InterruptedException
	{
		if (!isIdValid(player))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the mob enemy level because the player OID was not passed. [ getMobLevel() ]");
			return -1;
		}
		if (!isIdValid(map))
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get the mob enemy level for "+ player + " because the map OID was not passed. [ getMobLevel() ]");
			sendSystemMessage(player, "The treasure map failed to attain an accurate enemy mob level.", null);
			return -1;
		}
		if (playerLevel < 5)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get a useabel player level for "+ player + ". The player level is less than 5. [ getMobLevel() ]");
			sendSystemMessage(player, "The treasure map failed to attain your player level accurately.", null);
			return -1;
		}
		if (intMaxLevel < 10 || intMinLevel > 90)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get a mob level for "+ player + " because the map maximum level was out of range [ getMobLevel() ]");
			sendSystemMessage(player, "The treasure map failed to attain your player level accurately.", null);
			return -1;
		}
		if (intMinLevel < 1 || intMinLevel > 81)
		{
			CustomerServiceLog("treasureMap", "a treasure map failed to get a mob level for "+ player + " because the map minimum level was out of range [ getMobLevel() ]");
			sendSystemMessage(player, "The treasure map failed to attain your player level accurately.", null);
			return -1;
		}
		
		if (playerLevel > intMaxLevel)
		{
			int newMobLevel = intMaxLevel + mobLevelModifier;
			if (newMobLevel > 90)
			{
				return 90;
			}
			return newMobLevel;
		}
		if (playerLevel < intMinLevel)
		{
			return intMinLevel;
		}
		
		int newMobLevel = playerLevel + mobLevelModifier;
		if (newMobLevel > 90)
		{
			return 90;
		}
		return newMobLevel;
	}
	
	
	public int getMapTreasurePlanet(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id map = self;
		int intMaxLevel = getIntObjVar(map, "max");
		
		if (intMaxLevel <= 0)
		{
			
			setObjVar(map, "planet", "tatooine");
			CustomerServiceLog("treasureMap", "a treasure map failed to get the correct planet, setting map to Tatooine instead. [ getMapTreasurePlanet() ]");
		}
		
		int dataTableIdx = dataTableSearchColumnForInt(intMaxLevel, "map_level_max", TREASURE_TABLE);
		if (dataTableIdx == -1)
		{
			
			setObjVar(map, "planet", "tatooine");
			CustomerServiceLog( "treasureMap", "a treasure map failed to get the correct planet because the treasure map data table failed to return a row number. [ getMapTreasurePlanet() ]");
		}
		
		setObjVar(map, "dataTableIdx", dataTableIdx);
		
		String allPlanets = dataTableGetString(TREASURE_TABLE, dataTableIdx, "planets");
		String chosenPlanet = "";
		
		String[] planetList = split(allPlanets, ',');
		if (planetList.length == 1)
		{
			chosenPlanet = planetList[0];
		}
		else
		{
			int planetIndex = rand(0,planetList.length-1);
			chosenPlanet = planetList[planetIndex];
		}
		if (chosenPlanet != null && !chosenPlanet.equals(""))
		{
			
			setObjVar(map, "planet", chosenPlanet);
		}
		else
		{
			
			setObjVar(map, "planet", "tatooine");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public String checkForBossMobSpawn(obj_id player, int mobLevel, int groupModifier, int dataTableIdx, String loot_table) throws InterruptedException
	{
		if (!loot_table.equals(LOOT_TABLE_81_90))
		{
			return "none";
		}
		
		if (isGod(player) && hasObjVar(player, "boss_mob"))
		{
			return dataTableGetString(TREASURE_TABLE, dataTableIdx, "boss_mob");
		}
		if (mobLevel < 80)
		{
			return "none";
		}
		if (groupModifier < 8)
		{
			return "none";
		}
		
		return dataTableGetString(TREASURE_TABLE, dataTableIdx, "boss_mob");
	}
	
	
	public boolean verifyVariablesNotNull(String loot_table, String type, int mobLevel, int groupModifier, int count, int datTableIdx) throws InterruptedException
	{
		if (loot_table.equals("") || loot_table.equals(""))
		{
			return false;
		}
		if (type.equals("") || type.equals(""))
		{
			return false;
		}
		if (mobLevel <= 0)
		{
			return false;
		}
		if (groupModifier < 0)
		{
			return false;
		}
		if (count < 2)
		{
			return false;
		}
		if (datTableIdx < 0)
		{
			return false;
		}
		
		return true;
	}
	
	
	public boolean checkState(obj_id player) throws InterruptedException
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
	
	
	public boolean checkScene(obj_id player, obj_id map) throws InterruptedException
	{
		
		String req_planet = getStringObjVar(map, "planet");
		String scene = getCurrentSceneName();
		doDebugLogging("treasureMap", "planet: "+req_planet);
		if (scene.equals(req_planet))
		{
			return true;
		}
		
		String requiredPlanet = localize(new string_id("planet_n", req_planet));
		sendSystemMessage(player, "You need to travel to "+ requiredPlanet + " before using the map.", null);
		return false;
	}
	
	
	public void doDebugLogging(String section, String message) throws InterruptedException
	{
		LOG(section, message);
	}
}
