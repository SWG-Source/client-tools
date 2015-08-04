package script.library;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.theater;
import script.library.structure;
import script.library.player_structure;
import script.library.planetary_map;
import script.library.trial;


public class hq extends script.base_script
{
	public hq()
	{
	}
	public static final float VULNERABILITY_CYCLE = 172800f;
	
	public static final float VULNERABILITY_LENGTH = 10800f;
	
	public static final String SCRIPT_CLONING_OVERRIDE = "faction_perk.hq.terminal_cloning_override";
	public static final String SCRIPT_INSURANCE_OVERRIDE = "faction_perk.hq.terminal_insurance_override";
	
	public static final String SCRIPT_TERMINAL_DISABLE = "faction_perk.hq.terminal_disable";
	
	public static final String SCRIPT_DEFENSE_OBJECT = "faction_perk.hq.defense_object";
	
	public static final String VAR_HQ_BASE = "hq";
	
	public static final String SCRIPTVAR_FLATTENER = "hq.flattener";
	
	public static final String SCRIPTVAR_THEATER_OBJECTS = "hq.theaterObjects";
	public static final String SCRIPTVAR_TERMINALS = "hq.terminals";
	public static final String VAR_DEFENSE_TURTERMINALS = "hq.turterminals.terminals";
	
	public static final String VAR_SPAWN_BASE = VAR_HQ_BASE + ".spawn";
	public static final String SCRIPTVAR_SPAWN_INTERIOR = VAR_SPAWN_BASE + ".interior";
	public static final String SCRIPTVAR_SPAWN_EXTERIOR = VAR_SPAWN_BASE + ".exterior";
	
	public static final String SCRIPTVAR_SPAWN_REINFORCEMENT = VAR_SPAWN_BASE + ".reinforcement";
	
	public static final String VAR_HQ_RESOURCE_CNT = VAR_HQ_BASE + ".resource.cnt";
	
	public static final String VAR_DEFENSE_BASE = VAR_HQ_BASE + ".defense";
	
	public static final String VAR_DEFENSE_PARENT = VAR_DEFENSE_BASE + ".parent";
	
	public static final String VAR_OBJECTIVE_BASE = VAR_HQ_BASE + ".objective";
	public static final String VAR_OBJECTIVE_STAMP = VAR_OBJECTIVE_BASE + ".stamp";
	
	public static final String VAR_OBJECTIVE_TRACKING = VAR_OBJECTIVE_BASE + ".tracking";
	public static final String VAR_OBJECTIVE_ID = VAR_OBJECTIVE_TRACKING + ".id";
	public static final String VAR_OBJECTIVE_DISABLED = VAR_OBJECTIVE_TRACKING + ".disabled";
	
	public static final String VAR_VULNERABILITY_RESET = VAR_HQ_BASE + ".vulnerabilityReset";
	public static final int RESET_MAX = 1;
	
	public static final String VAR_IS_DISABLED = VAR_OBJECTIVE_BASE + ".isDisabled";
	
	public static final String[] OBJECTIVE_TEMPLATE =
	{
		"object/tangible/hq_destructible/uplink_terminal.iff",
		"object/tangible/hq_destructible/security_terminal.iff",
		"object/tangible/hq_destructible/override_terminal.iff",
		"object/tangible/hq_destructible/power_regulator.iff"
	};
	
	public static final String VAR_TERMINAL_DISABLE = VAR_HQ_BASE + ".terminal.disable";
	
	public static final String TBL_TERMINAL_PATH = "datatables/faction_perk/hq/terminal/";
	public static final String TBL_TURRET_TERMINAL_PATH = "datatables/faction_perk/hq/turret_terminal/";
	
	public static final String TBL_SPAWN_PATH = "datatables/faction_perk/hq/spawn/";
	public static final String TBL_SPAWN_INTERIOR_PATH = TBL_SPAWN_PATH + "interior/";
	public static final String TBL_SPAWN_EXTERIOR_PATH = TBL_SPAWN_PATH + "exterior/";
	public static final String TBL_SPAWN_EGG_PATH = TBL_SPAWN_PATH + "egg/";
	public static final String TBL_SPAWN_FORMATION_PATH = TBL_SPAWN_PATH + "formation/";
	
	public static final String TBL_DEFENSE_PATH = "datatables/faction_perk/hq/defense/";
	
	public static final String TBL_OBJECTIVE_PATH = "datatables/faction_perk/hq/objective/";
	
	public static final string_id PROSE_DISABLE_OTHER = new string_id("hq", "prose_disable_other");
	
	public static final String HANDLER_LOAD_VULNERABILITY = "handleLoadVulnerability";
	public static final String HANDLER_UNLOAD_VULNERABILITY = "handleUnloadVulnerability";
	
	public static final String TEMPLATE_SPAWN_EGG = "object/tangible/hq_egg/hq_spawner.iff";
	
	public static final String VAR_SPAWN_TYPE = VAR_HQ_BASE + ".spawnType";
	
	public static final int ST_NONE = 0;
	public static final int ST_SMALL = 1;
	public static final int ST_MEDIUM = 2;
	public static final int ST_LARGE = 3;
	
	public static final String VAR_SPAWN_CHILDREN = VAR_SPAWN_BASE + ".children";
	public static final String VAR_SPAWN_PARENT = VAR_SPAWN_BASE + ".parent";
	
	public static final String VAR_SPAWN_TEMPLATE = VAR_SPAWN_BASE + ".template";
	public static final String VAR_SPAWN_YAW = VAR_SPAWN_BASE + ".yaw";
	public static final String VAR_SPAWN_COUNT = VAR_SPAWN_BASE + ".cnt";
	
	public static final String SCRIPT_SPAWN_CHILD = "faction_perk.hq.spawn_child";
	
	public static final String VAR_SPAWN_LEADER = VAR_SPAWN_BASE + ".leader";
	
	public static final float FACTION_BASE_REFUND_AMOUNT = 60000f;
	
	public static final int MAX_MINE_TYPES = 3;
	
	
	public static void loadHqTheater(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadHqTheater entered...");
		LOG("hq","loadHqTheater: hq yaw = "+ getYaw(hq));
		obj_id[] children = theater.spawnDatatableOffset(hq);
		utils.removeBatchScriptVar(hq, SCRIPTVAR_THEATER_OBJECTS);
		if (children != null && children.length > 0)
		{
			utils.setBatchScriptVar(hq, SCRIPTVAR_THEATER_OBJECTS, children);
			messageTo(hq, "handleTheaterComplete", null, 5f, false);
		}
		
	}
	
	
	public static void cleanupHqTheater(obj_id hq) throws InterruptedException
	{
		LOG("hq","cleanupHqTheater entered...");
		obj_id[] children = utils.getObjIdBatchScriptVar(hq, SCRIPTVAR_THEATER_OBJECTS);
		if (children != null && children.length > 0)
		{
			utils.destroyObjects(children);
		}
	}
	
	
	public static void loadHqTerminals(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadHqTerminals entered...");
		String faction = factions.getFaction(hq);
		if (faction != null && !faction.equals(""))
		{
			obj_id[] terminals = structure.createStructureTerminals(hq, TBL_TERMINAL_PATH + toLower(faction) + ".iff");
			if (terminals != null && terminals.length > 0)
			{
				utils.setBatchScriptVar(hq, SCRIPTVAR_TERMINALS, terminals);
				
				for (int i = 0; i < terminals.length; i++)
				{
					testAbortScript();
					int tGot = getGameObjectType(terminals[i]);
					if (tGot == GOT_terminal_cloning)
					{
						attachScript(terminals[i], SCRIPT_CLONING_OVERRIDE);
					}
					else if (tGot == GOT_terminal_insurance)
					{
						attachScript(terminals[i], SCRIPT_INSURANCE_OVERRIDE);
					}
				}
			}
			
		}
		else
		{
			destroyObject(hq);
		}
	}
	
	
	public static void loadTurretTerminals(obj_id hq) throws InterruptedException
	{
		LOG("***hq***", "loadTurretTerminals entered...");
		String faction = factions.getFaction(hq);
		if (faction != null && !faction.equals(""))
		{
			obj_id[] turTerminals = structure.createStructureTerminals(hq, TBL_TURRET_TERMINAL_PATH + toLower(faction) + ".iff");
			if (turTerminals != null && turTerminals.length > 0)
			{
				utils.setBatchObjVar(hq, VAR_DEFENSE_TURTERMINALS, turTerminals);
			}
		}
	}
	
	
	public static void loadHqSpawns(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadHqSpawns entered...");
		loadInteriorHqSpawns(hq);
		loadExteriorHqSpawns(hq);
	}
	
	
	public static void loadInteriorHqSpawns(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadInteriorHqSpawns entered...");
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			obj_id[] interiorSpawns = spawnInterior(hq, TBL_SPAWN_INTERIOR_PATH + filename);
			if (interiorSpawns != null && interiorSpawns.length > 0)
			{
				utils.setBatchScriptVar(hq, SCRIPTVAR_SPAWN_INTERIOR, interiorSpawns);
			}
		}
	}
	
	
	public static obj_id[] spawnInterior(obj_id hq, String tbl) throws InterruptedException
	{
		LOG("hq","spawnInterior entered...");
		if (!isIdValid(hq) || tbl == null || tbl.equals(""))
		{
			return null;
		}
		
		int numRow = dataTableGetNumRows(tbl);
		if (numRow > 0)
		{
			Vector spawns = new Vector();
			spawns.setSize(0);
			
			location here = getLocation(hq);
			
			String myFac = factions.getFaction(hq);
			if (myFac == null || myFac.equals(""))
			{
				return null;
			}
			
			LOG("hq","spawnInterior: numRows = "+ numRow);
			for (int i = 0; i < numRow; i++)
			{
				testAbortScript();
				dictionary params = dataTableGetRow(tbl, i);
				
				String spawnType = params.getString("SPAWN");
				LOG("hq","spawnInterior: ("+ i + ") spawnType = "+ spawnType);
				if (spawnType != null && !spawnType.equals(""))
				{
					float x = params.getFloat("X");
					float y = params.getFloat("Y");
					float z = params.getFloat("Z");
					
					float yaw = params.getFloat("YAW");
					String cellname = params.getString("CELL");
					
					location there = null;
					if (cellname != null && !cellname.equals(""))
					{
						if (cellname.equals("WORLD_DELTA"))
						{
							there = new location(here.x + x, here.y + y, here.z + z);
						}
						else
						{
							obj_id cellId = getCellId(hq, cellname);
							if (isIdValid(cellId))
							{
								there = new location(x, y, z, here.area, cellId);
							}
							else
							{
								LOG("hq","spawnInterior: unable to determine id for hq = "+ hq + " cell = "+ cellname);
							}
						}
					}
					
					if (there != null)
					{
						obj_id spawn = null;
						LOG("hq","spawnInterior: attempting to load interior spawn with type: "+ spawnType);
						if (spawnType.startsWith("egg"))
						{
							spawn = createInteriorEgg(hq, there, myFac, yaw);
							
							if (spawnType.indexOf(":") > -1)
							{
								String[] tmpSplit = split(spawnType, ':');
								if (tmpSplit.length > 1)
								{
									setObjVar(spawn, VAR_SPAWN_TEMPLATE, tmpSplit[1]);
								}
							}
						}
						else
						{
							spawn = create.object(spawnType, there);
							if (isIdValid(spawn))
							{
								setYaw(spawn, yaw);
							}
						}
						
						if (isIdValid(spawn))
						{
							spawns = utils.addElement(spawns, spawn);
						}
					}
					else
					{
						LOG("hq","spawnInterior: unable to load valid location for #"+ i + " -> spawnType = "+ spawnType);
					}
				}
			}
			
			if (spawns != null && spawns.size() > 0)
			{
				obj_id[] _spawns = new obj_id[0];
				if (spawns != null)
				{
					_spawns = new obj_id[spawns.size()];
					spawns.toArray(_spawns);
				}
				return _spawns;
			}
		}
		
		return null;
	}
	
	
	public static obj_id createInteriorEgg(obj_id hq, location there, String myFac, float yaw) throws InterruptedException
	{
		LOG("hq","spawnInterior: loading interior spawn egg...");
		obj_id spawn = create.object(TEMPLATE_SPAWN_EGG, there);
		if (isIdValid(spawn))
		{
			LOG("hq","spawnInterior: spawn egg loaded...");
			setObjVar(spawn, VAR_SPAWN_TYPE, ST_SMALL);
			setObjVar(spawn, VAR_SPAWN_PARENT, hq);
			
			setObjVar(spawn, VAR_SPAWN_YAW, yaw);
			
			factions.setFaction(spawn, myFac);
			
			messageTo(spawn, "handleSpawnRequest", null, rand(1,5), false);
			return spawn;
		}
		else
		{
			LOG("hq","spawnInterior: UNABLE TO LOAD interior spawn egg...");
		}
		
		return null;
	}
	
	
	public static void loadExteriorHqSpawns(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadExteriorHqSpawns entered...");
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			if (filename.indexOf( "_pvp" ) == -1)
			{
				obj_id[] exteriorSpawns = spawnExterior(hq, TBL_SPAWN_EXTERIOR_PATH + filename);
				if (exteriorSpawns != null && exteriorSpawns.length > 0)
				{
					utils.setBatchScriptVar(hq, SCRIPTVAR_SPAWN_EXTERIOR, exteriorSpawns);
				}
			}
		}
	}
	
	
	public static obj_id[] spawnExterior(obj_id hq, String tbl) throws InterruptedException
	{
		LOG("hq","spawnExterior entered...");
		if (!isIdValid(hq) || tbl == null || tbl.equals(""))
		{
			return null;
		}
		
		Vector spawns = new Vector();
		spawns.setSize(0);
		
		location here = getLocation(hq);
		float yaw = getYaw(hq);
		
		String myFac = factions.getFaction(hq);
		if (myFac == null || myFac.equals(""))
		{
			return null;
		}
		
		int numCols = dataTableGetNumColumns(tbl);
		for (int i = 0; i < numCols; i++)
		{
			testAbortScript();
			String colName = dataTableGetColumnName(tbl, i);
			
			String[] locs = dataTableGetStringColumnNoDefaults(tbl, i);
			if (locs != null && !locs.equals(""))
			{
				for (int n = 0; n < locs.length; n++)
				{
					testAbortScript();
					
					java.util.StringTokenizer st = new java.util.StringTokenizer(locs[n], ",");
					if (st.countTokens() == 2)
					{
						float dx = utils.stringToFloat(st.nextToken());
						float dz = utils.stringToFloat(st.nextToken());
						if (dx != Float.NEGATIVE_INFINITY && dz != Float.NEGATIVE_INFINITY)
						{
							location spawnLoc = player_structure.transformDeltaWorldCoord(here, dx, dz, yaw);
							if (spawnLoc != null)
							{
								obj_id spawn = create.object(TEMPLATE_SPAWN_EGG, spawnLoc);
								LOG("hq","spawnExterior: spawn egg id = "+ spawn);
								if (isIdValid(spawn))
								{
									if (colName.equalsIgnoreCase("small"))
									{
										setObjVar(spawn, VAR_SPAWN_TYPE, ST_SMALL);
									}
									else if (colName.equalsIgnoreCase("med"))
									{
										setObjVar(spawn, VAR_SPAWN_TYPE, ST_MEDIUM);
									}
									else if (colName.equalsIgnoreCase("large"))
									{
										setObjVar(spawn, VAR_SPAWN_TYPE, ST_LARGE);
									}
									
									setObjVar(spawn, VAR_SPAWN_PARENT, hq);
									
									factions.setFaction(spawn, myFac);
									
									messageTo(spawn, "handleSpawnRequest", null, rand(1,5), false);
									
									spawns = utils.addElement(spawns, spawn);
								}
							}
						}
					}
				}
			}
		}
		
		if (spawns != null && spawns.size() > 0)
		{
			obj_id[] _spawns = new obj_id[0];
			if (spawns != null)
			{
				_spawns = new obj_id[spawns.size()];
				spawns.toArray(_spawns);
			}
			return _spawns;
		}
		
		return null;
	}
	
	
	public static location parseExteriorLocation(location home, String delta) throws InterruptedException
	{
		if (home == null || delta == null || delta.equals(""))
		{
			return null;
		}
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(delta, ",");
		if (st.countTokens() != 2)
		{
			return null;
		}
		
		float dx = utils.stringToFloat(st.nextToken());
		float dz = utils.stringToFloat(st.nextToken());
		if (dx == Float.NEGATIVE_INFINITY || dz == Float.NEGATIVE_INFINITY)
		{
			return null;
		}
		
		return new location(home.x+dx, home.y, home.z+dz, home.area);
	}
	
	
	public static void cleanupHqSpawns(obj_id hq) throws InterruptedException
	{
		LOG("hq","cleanupHqSpawns entered...");
		obj_id[] intSpawn = utils.getObjIdBatchScriptVar(hq, SCRIPTVAR_SPAWN_INTERIOR);
		if (intSpawn != null && intSpawn.length > 0)
		{
			utils.destroyObjects(intSpawn);
		}
		
		obj_id[] extSpawn = utils.getObjIdBatchScriptVar(hq, SCRIPTVAR_SPAWN_EXTERIOR);
		if (extSpawn != null && extSpawn.length > 0)
		{
			utils.destroyObjects(extSpawn);
		}
		
		if (utils.hasScriptVar(hq, SCRIPTVAR_SPAWN_REINFORCEMENT))
		{
			obj_id reinforcementSpawner = utils.getObjIdScriptVar(hq, SCRIPTVAR_SPAWN_REINFORCEMENT);
			messageTo(reinforcementSpawner, "handleParentCleanup", null, 300, false);
		}
	}
	
	
	public static void prepareHqDefenses(obj_id hq) throws InterruptedException
	{
		if (hasObjVar(hq, VAR_DEFENSE_BASE))
		{
			return;
		}
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			String tbl = TBL_DEFENSE_PATH + filename;
			int numCol = dataTableGetNumColumns(tbl);
			for (int i = 0; i < numCol; i++)
			{
				testAbortScript();
				String colName = toLower(dataTableGetColumnName(tbl, i));
				String[] data = dataTableGetStringColumnNoDefaults(tbl, i);
				if (data != null && data.length > 0)
				{
					obj_id[] tmp = new obj_id[data.length];
					for (int j = 0; j < tmp.length; ++j)
					{
						testAbortScript();
						tmp[j] = obj_id.NULL_ID;
					}
					setObjVar(hq, VAR_DEFENSE_BASE + "."+ colName, tmp);
				}
			}
			
			messageTo(hq, "handleCreateMinefield", null, 5f, false);
			
			int[] mines = new int[MAX_MINE_TYPES];
			setObjVar(hq, "mines", mines);
		}
	}
	
	
	public static void cleanupHqDefenses(obj_id hq) throws InterruptedException
	{
		if (!hasObjVar(hq, VAR_DEFENSE_BASE))
		{
			return;
		}
		
		obj_var_list ovl = getObjVarList(hq, VAR_DEFENSE_BASE);
		if (ovl == null)
		{
			return;
		}
		
		int numItems = ovl.getNumItems();
		for (int i = 0; i < numItems; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			if (ov != null)
			{
				obj_id[] defenses = ov.getObjIdArrayData();
				if (defenses != null && defenses.length > 0)
				{
					for (int x = 0; x < defenses.length; x++)
					{
						testAbortScript();
						if (isIdValid(defenses[x]))
						{
							destroyObject(defenses[x]);
						}
					}
				}
			}
		}
	}
	
	
	public static void disableHqTerminals(obj_id hq) throws InterruptedException
	{
		setObjVar(hq, VAR_TERMINAL_DISABLE, true);
		
		obj_id[] terminals = utils.getObjIdBatchScriptVar(hq, SCRIPTVAR_TERMINALS);
		if (terminals != null && terminals.length > 0)
		{
			for (int i = 0; i < terminals.length; i++)
			{
				testAbortScript();
				if (!hasScript(terminals[i], SCRIPT_TERMINAL_DISABLE))
				{
					attachScript(terminals[i], SCRIPT_TERMINAL_DISABLE);
				}
			}
		}
	}
	
	
	public static void enableHqTerminals(obj_id hq) throws InterruptedException
	{
		setObjVar(hq, VAR_TERMINAL_DISABLE, false);
		removeObjVar(hq, VAR_OBJECTIVE_DISABLED);
		obj_id[] objects = trial.getAllObjectsInDungeon(hq);
		if (objects == null || objects.length == 0)
		{
			return;
		}
		
		for (int i=0; i<objects.length; i++)
		{
			testAbortScript();
			if (hasScript(objects[i], SCRIPT_TERMINAL_DISABLE))
			{
				detachScript(objects[i], SCRIPT_TERMINAL_DISABLE);
			}
			if (hasObjVar(objects[i], VAR_IS_DISABLED))
			{
				removeObjVar(objects[i], VAR_IS_DISABLED);
			}
			
		}
	}
	
	
	public static void loadVulnerability(obj_id hq) throws InterruptedException
	{
		setInvulnerable(hq, true);
		
		if (hasObjVar(hq, VAR_OBJECTIVE_ID))
		{
			unloadVulnerabilityForObjectReInit(hq);
		}
		
		int currentTime = getCurrentTimeInSecsNormalized();
		
		if (!hasObjVar(hq, VAR_OBJECTIVE_STAMP))
		{
			setObjVar(hq, VAR_OBJECTIVE_STAMP, currentTime);
			
		}
		
		int stamp = getIntObjVar(hq, VAR_OBJECTIVE_STAMP);
		
		int diff = stamp - currentTime;
		
		int checkfutureclose = (int)VULNERABILITY_LENGTH/2;
		int checkfuturefar = (int)VULNERABILITY_CYCLE;
		int checkpastclose = checkfutureclose*-1;
		int checkpastfar = checkfuturefar*-1;
		
		if (diff <= checkfutureclose && diff >= checkpastclose)
		{
			
			obj_id[] objectives = loadObjectives(hq);
			if (objectives != null && objectives.length > 0)
			{
				LOG("hq", "loadVulnerability ********* OBJECTIVES LOADED...");
				setObjVar(hq, VAR_OBJECTIVE_ID, objectives);
				
				int stampNew = stamp + ((int)VULNERABILITY_CYCLE);
				
				if (stampNew < 0)
				{
					stampNew = overdueStampReplacer (stamp, currentTime);
					
				}
				
				dictionary mapParams = new dictionary();
				int newStatus = 1;
				mapParams.put( "status", newStatus );
				messageTo(hq, "handleFlagVulnerabilityChange", mapParams, 10, false);
				LOG("hq", "loadVulnerability \\\\\\\\\\\\\\\\\\\\\\\\ fired message to 'handleFlagVulnerabilityChange' with status of : "+newStatus);
				
				setObjVar(hq, VAR_OBJECTIVE_STAMP, stampNew);
				
				messageTo(hq, HANDLER_UNLOAD_VULNERABILITY, null, VULNERABILITY_LENGTH, false);
				
				return;
			}
			else
			{
				
			}
			
			messageTo(hq, HANDLER_LOAD_VULNERABILITY, null, VULNERABILITY_CYCLE + diff, false);
			return;
		}
		else if (diff <= checkpastclose)
		{
			int stampNew = overdueStampReplacer (stamp, currentTime);
			
			setObjVar(hq, VAR_OBJECTIVE_STAMP, stampNew);
			int vulnerabilityDelay = stampNew-currentTime;
			messageTo(hq, HANDLER_LOAD_VULNERABILITY, null, vulnerabilityDelay, false);
			
			return;
		}
		else if (diff > checkfutureclose && diff < checkfuturefar)
		{
			
			messageTo(hq, HANDLER_LOAD_VULNERABILITY, null, diff, false);
			return;
		}
		else if (diff >= checkfuturefar)
		{
			int stampNew = farFutureStampReplacer (stamp, currentTime);
			setObjVar(hq, VAR_OBJECTIVE_STAMP, stampNew);
			int vulnerabilityDelay = stampNew-currentTime;
			
			messageTo(hq, HANDLER_LOAD_VULNERABILITY, null, vulnerabilityDelay, false);
		}
	}
	
	
	public static obj_id[] loadObjectives(obj_id hq) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename == null || filename.equals(""))
		{
			return null;
		}
		
		String tbl = TBL_OBJECTIVE_PATH + filename;
		
		int numRows = dataTableGetNumRows(tbl);
		Vector opt = new Vector();
		opt.setSize(0);
		
		for (int i = 0; i < numRows; i++)
		{
			testAbortScript();
			opt = utils.addElement(opt, i);
		}
		
		location here = getLocation(hq);
		float yaw = getYaw(hq);
		
		Vector objectives = new Vector();
		objectives.setSize(0);
		for (int i = 0; i < OBJECTIVE_TEMPLATE.length; i++)
		{
			testAbortScript();
			int idx = rand(0, opt.size() - 1);
			int rowIdx = ((Integer)(opt.get(idx))).intValue();
			
			opt = utils.removeElementAt(opt, idx);
			
			dictionary row = dataTableGetRow(tbl, rowIdx);
			
			String cellName = row.getString("CELL");
			float dx = row.getFloat("X");
			float dy = row.getFloat("Y");
			float dz = row.getFloat("Z");
			float dyaw = row.getFloat("YAW");
			
			obj_id objective = null;
			if (cellName == null || cellName.equals("") || cellName.equals("WORLD_DELTA"))
			{
				location there = player_structure.transformDeltaWorldCoord(here, dx, dz, yaw);
				objective = createObject(OBJECTIVE_TEMPLATE[i], there);
				if (isIdValid(objective))
				{
					setYaw(objective, yaw+dyaw);
					utils.addElement(objectives, objective);
				}
			}
			else
			{
				obj_id cellId = getCellId(hq, cellName);
				if (isIdValid(cellId))
				{
					location iSpot = new location(dx, dy, dz, here.area, cellId);
					objective = createObjectInCell(OBJECTIVE_TEMPLATE[i], hq, cellName, iSpot);
					if (isIdValid(objective))
					{
						setYaw(objective, dyaw);
						utils.addElement(objectives, objective);
					}
				}
			}
		}
		
		obj_id[] _objectives = new obj_id[0];
		if (objectives != null)
		{
			_objectives = new obj_id[objectives.size()];
			objectives.toArray(_objectives);
		}
		return _objectives;
	}
	
	
	public static int overdueStampReplacer(int stamp, int currentTime) throws InterruptedException
	{
		int timeOverdue = currentTime - stamp;
		int timeOverdueMagnitude = timeOverdue / (int)(VULNERABILITY_CYCLE);
		int stampNew = ((timeOverdueMagnitude+1)*(int)(VULNERABILITY_CYCLE)) + stamp;
		return stampNew;
	}
	
	
	public static int farFutureStampReplacer(int stamp, int currentTime) throws InterruptedException
	{
		int futureDiff = stamp - currentTime;
		int futureMagnitude = futureDiff / (int)(VULNERABILITY_CYCLE);
		int stampNew = stamp - (futureMagnitude+(int)(VULNERABILITY_CYCLE));
		return stampNew;
	}
	
	
	public static void unloadVulnerabilityForObjectReInit(obj_id hq) throws InterruptedException
	{
		obj_id[] objectives = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_ID);
		
		setObjVar(hq, "planetMap.flags", MLF_INACTIVE);
		
		dictionary mapParams = new dictionary();
		int newStatus = 0;
		mapParams.put( "status", newStatus );
		messageTo(hq, "handleFlagVulnerabilityChange", mapParams, 10, false);
		LOG("hq", "unloadVulnerabilityForObjectReInit \\\\\\\\\\\\\\\\\\\\\\\\ fired message to 'handleFlagVulnerabilityChange' with status of : "+newStatus);
		
		removeObjVar(hq, VAR_OBJECTIVE_TRACKING);
		
		if (objectives != null && objectives.length > 0)
		{
			for (int i = 0; i < objectives.length; i++)
			{
				testAbortScript();
				destroyObject(objectives[i]);
			}
		}
		activateHackAlarms(hq, false);
		cleanupHqSecurityTeam(hq);
	}
	
	
	public static void unloadVulnerability(obj_id hq) throws InterruptedException
	{
		
		obj_id[] objectives = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_ID);
		
		dictionary mapParams = new dictionary();
		int newStatus = 0;
		mapParams.put( "status", newStatus );
		messageTo(hq, "handleFlagVulnerabilityChange", mapParams, 10, false);
		LOG("hq", "unloadVulnerability \\\\\\\\\\\\\\\\\\\\\\\\ fired message to 'handleFlagVulnerabilityChange' with status of : "+newStatus);
		
		removeObjVar(hq, VAR_OBJECTIVE_TRACKING);
		
		if (objectives != null && objectives.length > 0)
		{
			for (int i = 0; i < objectives.length; i++)
			{
				testAbortScript();
				destroyObject(objectives[i]);
			}
		}
		
		int currentTime = getCurrentTimeInSecsNormalized();
		
		int reVulnerableDelay = (int) (VULNERABILITY_CYCLE - VULNERABILITY_LENGTH);
		
		if (hasObjVar(hq, VAR_OBJECTIVE_STAMP))
		{
			int stamp = getIntObjVar(hq, VAR_OBJECTIVE_STAMP);
			
			if (stamp < 0)
			{
				stamp = overdueStampReplacer (stamp, currentTime);
				
			}
			else if (stamp > (stamp + (int)(VULNERABILITY_CYCLE*2)))
			{
				stamp = farFutureStampReplacer (stamp, currentTime);
				
			}
			
			reVulnerableDelay = stamp - currentTime;
			
			setObjVar(hq, VAR_OBJECTIVE_STAMP, stamp);
		}
		
		messageTo(hq, "handleDefenderRespawn", null, 5, false);
		activateHackAlarms(hq, false);
		cleanupHqSecurityTeam(hq);
		messageTo(hq, HANDLER_LOAD_VULNERABILITY, null, reVulnerableDelay, false);
	}
	
	
	public static int getCurrentTimeInSecsNormalized() throws InterruptedException
	{
		long now = System.currentTimeMillis();
		long curTime = ((now / 1000) - 1072224000);
		int currentTime = (int) curTime;
		return currentTime;
	}
	
	
	public static boolean canDisableObjective(obj_id hq, obj_id objective) throws InterruptedException
	{
		
		LOG("gcw", "checking canDisable Objective for "+hq+" and "+objective);
		if (!isIdValid(hq) || !isIdValid(objective))
		{
			return false;
		}
		
		if (hasObjVar(objective, VAR_IS_DISABLED))
		{
			return false;
		}
		LOG("gcw", "getNextObjective is "+getNextObjective(hq));
		if (objective == getNextObjective(hq))
		{
			return true;
		}
		
		return false;
	}
	
	
	public static obj_id getNextObjective(obj_id hq) throws InterruptedException
	{
		
		obj_id[] objectives = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_ID);
		if (objectives == null || objectives.length == 0)
		{
			return null;
		}
		
		obj_id[] disabled = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_DISABLED);
		if (disabled == null || disabled.length == 0)
		{
			return objectives[0];
		}
		else
		{
			if (disabled.length < objectives.length)
			{
				return objectives[disabled.length];
			}
		}
		
		return null;
	}
	
	
	public static obj_id getPriorObjective(obj_id hq, obj_id objective) throws InterruptedException
	{
		
		obj_id[] objectives = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_ID);
		if (objectives == null || objectives.length == 0)
		{
			return null;
		}
		
		int idx = utils.getElementPositionInArray(objectives, objective);
		if (idx > 0 && idx < objectives.length)
		{
			return objectives[idx - 1];
		}
		
		return null;
	}
	
	
	public static void disableObjective(obj_id hq, obj_id objective) throws InterruptedException
	{
		
		if (canDisableObjective(hq, objective))
		{
			
			Vector disabled = getResizeableObjIdArrayObjVar(hq, VAR_OBJECTIVE_DISABLED);
			disabled = utils.addElement(disabled, objective);
			setObjVar(hq, VAR_OBJECTIVE_DISABLED, disabled, resizeableArrayTypeobj_id);
			
			setObjVar(objective, VAR_IS_DISABLED, true);
			
			messageTo(objective, "handleObjectiveDisabled", null, 1, false);
			activateHackAlarms(hq, true);
			
		}
	}
	
	
	public static void disableObjective(obj_id objective) throws InterruptedException
	{
		
		obj_id hq = null;
		if (hasObjVar(objective, "objParent"))
		{
			hq = getObjIdObjVar(objective, "objParent");
			
		}
		else
		{
			hq = getTopMostContainer(objective);
		}
		if (isIdValid(hq))
		{
			disableObjective(hq, objective);
		}
		else
		{
			LOG("hq", "library.hq::disableObjective(obj_id objective) ********* hq obj_id came in as invalid");
		}
	}
	
	
	public static void detonateHq(obj_id hq) throws InterruptedException
	{
		float maxDist = getFloatObjVar(hq, "poi.fltSize") - 20f;
		
		location here = getLocation(hq);
		
		obj_id[] players = player_structure.getPlayersInBuilding(hq);
		if (players != null && players.length > 0)
		{
			location ejectLoc = getBuildingEjectLocation(hq);
			location worldLoc = getWorldLocation(hq);
			
			for (int i = 0; i < players.length; i++)
			{
				testAbortScript();
				pclib.sendToAnyLocation(players[i], ejectLoc, worldLoc);
			}
		}
		
		location[] locs = new location[rand(5,9)];
		for (int i = 0; i < locs.length; i++)
		{
			testAbortScript();
			location tmpLoc = utils.getRandomLocationInRing(here, 5f, maxDist);
			if (tmpLoc != null)
			{
				tmpLoc.y = getHeightAtLocation(tmpLoc.x, tmpLoc.z) + rand(0f, 5f);
				locs[i] = tmpLoc;
			}
		}
		
		dictionary d = new dictionary();
		d.put("locs", locs);
		
		messageTo(hq, "handleHqDetonation", d, rand(0.1f, 1f), true);
	}
	
	
	public static void validateDefenseTracking(obj_id hq) throws InterruptedException
	{
		if (!isIdValid(hq))
		{
			return;
		}
		
		if (!hasObjVar(hq, VAR_DEFENSE_BASE))
		{
			return;
		}
		
		obj_var_list ovl = getObjVarList(hq, VAR_DEFENSE_BASE);
		if (ovl == null)
		{
			return;
		}
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			String tbl = TBL_DEFENSE_PATH + filename;
			String[] data = dataTableGetStringColumnNoDefaults(tbl, "TURRET");
			
			obj_id[] properTurretData = new obj_id[data.length];
			obj_id[] currentTurretData = getObjIdArrayObjVar(hq, VAR_DEFENSE_BASE + ".turret");
			
			if ((currentTurretData != null && properTurretData != null) && currentTurretData.length != properTurretData.length)
			{
				for (int i = 0; i < properTurretData.length; i++)
				{
					testAbortScript();
					if (i < currentTurretData.length)
					{
						properTurretData[i] = currentTurretData[i];
					}
					else
					{
						properTurretData[i] = obj_id.NULL_ID;
					}
				}
				
				setObjVar(hq, VAR_DEFENSE_BASE + ".turret", properTurretData);
			}
		}
		
		int numType = ovl.getNumItems();
		for (int i = 0; i < numType; i++)
		{
			testAbortScript();
			obj_var ov = ovl.getObjVar(i);
			obj_id[] defenses = ov.getObjIdArrayData();
			if (defenses != null && defenses.length > 0)
			{
				for (int x = 0; x < defenses.length; x++)
				{
					testAbortScript();
					if (isIdValid(defenses[x]))
					{
						if (!exists(defenses[x]) || (getHitpoints(defenses[x]) < 1))
						{
							dictionary d = new dictionary();
							d.put("sender", defenses[x]);
							messageTo(hq, "handleRemoveDefense", d, 1f, false);
						}
						else if (hasScript(defenses[x], "systems.turret.turret_ai"))
						{
							dictionary d = new dictionary();
							d.put("sender", defenses[x]);
							messageTo(hq, "handleResetTurretControl", d, 1f, false);
						}
					}
				}
			}
		}
		obj_id[] numTur = getObjIdArrayObjVar(hq, "hq.defense.turret");
		if (numTur == null || numTur.length == 0)
		{
			detachScript(hq, "faction_perk.hq.base_block");
		}
		int pos = utils.getFirstValidIdIndex(numTur);
		if (pos > -1)
		{
			attachScript(hq, "faction_perk.hq.base_block");
		}
		
		if (hasObjVar(hq, "mines"))
		{
			int[] mines = getIntArrayObjVar(hq, "mines");
			
			if (mines.length < MAX_MINE_TYPES)
			{
				messageTo(hq, "handleMinefieldValidation", null, 10f, false);
			}
		}
		else
		{
			messageTo(hq, "handleMinefieldValidation", null, 10f, false);
		}
	}
	
	
	public static void sealEntrance(obj_id hq) throws InterruptedException
	{
		if (!isIdValid(hq))
		{
			return;
		}
		
		if (!hasObjVar(hq, VAR_DEFENSE_BASE))
		{
			return;
		}
		
		attachScript(hq, "faction_perk.hq.base_block");
	}
	
	
	public static void ejectEnemyFactionOnLogin(obj_id player) throws InterruptedException
	{
		obj_id structure = getTopMostContainer( player );
		if (!isIdValid(structure))
		{
			return;
		}
		
		int myFac = pvpGetAlignedFaction(structure);
		int tFac = pvpGetAlignedFaction(player);
		if (isPlayer( player) && pvpGetType( player ) == PVPTYPE_NEUTRAL)
		{
			tFac = 0;
		}
		
		if (pvpAreFactionsOpposed(myFac, tFac))
		{
			
			if (isIdValid(structure))
			{
				string_id warning = new string_id ("faction_perk","destroy_turrets");
				sendSystemMessage (player, warning);
				{
					location there = getBuildingEjectLocation(structure);
					
					if (there == null)
					{
						return;
					}
					
					dictionary params = new dictionary();
					params.put("player", player);
					params.put("loc", there);
					messageTo(structure, "ejectPlayer", params, 1f, false);
				}
			}
			return;
		}
	}
	
	
	public static void loadFullTimeVulnerability(obj_id hq) throws InterruptedException
	{
		if (hasObjVar(hq,VAR_OBJECTIVE_ID))
		{
			obj_id[] oldObjectives = getObjIdArrayObjVar(hq, VAR_OBJECTIVE_ID);
			removeObjVar(hq, VAR_OBJECTIVE_TRACKING);
			if (oldObjectives != null && oldObjectives.length > 0)
			{
				for (int i = 0; i < oldObjectives.length; i++)
				{
					testAbortScript();
					if (isIdValid(oldObjectives[i]))
					{
						destroyObject(oldObjectives[i]);
					}
				}
			}
		}
		
		obj_id[] objectives = loadObjectives(hq);
		if (objectives != null && objectives.length > 0)
		{
			setObjVar(hq, VAR_OBJECTIVE_ID, objectives);
			
		}
		
		return;
	}
	
	
	public static void planetMapVulnerabilityStatusChange(obj_id hq, int newStatus) throws InterruptedException
	{
		
		map_location maploc = getPlanetaryMapLocation(hq);
		
		byte statusFlag = MLF_INACTIVE;
		if (newStatus == 1)
		{
			statusFlag = MLF_ACTIVE;
		}
		
		addPlanetaryMapLocation(hq, maploc.getLocationName(), (int)maploc.getX(), (int)maploc.getY(), maploc.getCategory(), maploc.getSubCategory(), MLT_STATIC, statusFlag);
		
		return;
	}
	
	
	public static void refundFactionBaseLots(obj_id base) throws InterruptedException
	{
		if (hasObjVar(base, "lotsRefunded2" ))
		{
			return;
		}
		
		obj_id baseOwner = getOwner(base);
		
		String base_template = getTemplateName(base);
		String base_fp_template = player_structure.getFootprintTemplate( base_template );
		if (base_fp_template == null || base_fp_template.equals(""))
		{
			return;
		}
		
		int base_lot_cost = (getNumberOfLots(base_fp_template) / 4);
		if (base_lot_cost < 1)
		{
			base_lot_cost = 1;
		}
		
		int base_lot_refund = base_lot_cost;
		
		dictionary outparams = new dictionary();
		outparams.put( "baseLotRefund", base_lot_refund );
		
		float faction_point_refund = 60000f;
		if (hasObjVar(base, "lotsRefunded" ))
		{
			faction_point_refund = 0f;
		}
		outparams.put( "baseFactionRefund", faction_point_refund );
		int baseFaction = pvpGetAlignedFaction(base);
		outparams.put( "baseFaction", baseFaction );
		
		gcw.incrementGCWScore (base);
		setObjVar(base, "lotsRefunded2", 1);
		
		messageTo(baseOwner, "factionBaseLotRefund", outparams, 1f, true);
		
		return;
	}
	
	
	public static void refundBaseUnit(obj_id base) throws InterruptedException
	{
		
		obj_id baseOwner = getOwner(base);
		messageTo(baseOwner, "factionBaseUnitRefund", null, 1f, true);
		return;
	}
	
	
	public static boolean isPvpFactionBase(obj_id base) throws InterruptedException
	{
		if (hasObjVar(base,"isPvpBase" ))
		{
			return true;
		}
		return false;
	}
	
	
	public static void removeCovertDetector(obj_id base) throws InterruptedException
	{
		if (hasObjVar(base,"hq.defense.covert_detector" ))
		{
			removeObjVar(base,"hq.defense.covert_detector");
		}
		return;
	}
	
	
	public static void loadAlarmUnits(obj_id hq) throws InterruptedException
	{
		LOG("hq","loadAlarmUnits entered...");
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			spawnAlarmUnits(hq);
		}
	}
	
	
	public static void spawnAlarmUnits(obj_id hq) throws InterruptedException
	{
		
		cleanupBaseAlarmUnits(hq);
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename == null || filename.equals(""))
		{
			return;
		}
		
		if (filename.indexOf ("hq_s01") != -1)
		{
			filename = "hq_s01.iff";
		}
		else if (filename.indexOf ("hq_s02") != -1)
		{
			filename = "hq_s02.iff";
		}
		else if (filename.indexOf ("hq_s03") != -1)
		{
			filename = "hq_s03.iff";
		}
		else if (filename.indexOf ("hq_s04") != -1)
		{
			filename = "hq_s04.iff";
		}
		
		String tbl = "datatables/faction_perk/hq/alarm/"+filename;
		
		int numRows = dataTableGetNumRows(tbl);
		
		location here = getLocation(hq);
		
		Vector hackAlarms = new Vector();
		hackAlarms.setSize(0);
		Vector destructAlarms = new Vector();
		destructAlarms.setSize(0);
		for (int i = 0; i < numRows; i++)
		{
			testAbortScript();
			dictionary row = dataTableGetRow(tbl, i);
			
			String alarmType = row.getString("ALARM");
			LOG("hq","hq::spawnAlarmUnits: ("+ i + ") alarmType = "+ alarmType);
			
			if (alarmType != null && !alarmType.equals(""))
			{
				float x = row.getFloat("X");
				float y = row.getFloat("Y");
				float z = row.getFloat("Z");
				float yaw = row.getFloat("YAW");
				String cellName = row.getString("CELL");
				
				obj_id alarmUnit = null;
				if (cellName != null && !cellName.equals(""))
				{
					obj_id cellId = getCellId(hq, cellName);
					if (isIdValid(cellId))
					{
						location loc = new location(x, y, z, here.area, cellId);
						alarmUnit = createObjectInCell(alarmType, hq, cellName, loc);
						
						setLocation(alarmUnit, loc);
						setYaw(alarmUnit, yaw);
						
						transform alarmTransform = getTransform_o2p(alarmUnit);
						if (alarmType.indexOf ("alarm_hack") != -1)
						{
							alarmTransform = alarmTransform.pitch_l((float)((Math.PI)/2));
						}
						else
						{
							alarmTransform = alarmTransform.pitch_l((float)Math.PI);
						}
						setTransform_o2p(alarmUnit, alarmTransform);
						
						if (isIdValid(alarmUnit))
						{
							if (alarmType.indexOf ("hack") != -1)
							{
								utils.addElement(hackAlarms, alarmUnit);
							}
							else if (alarmType.indexOf ("destruct") != -1)
							{
								utils.addElement(destructAlarms, alarmUnit);
							}
							else
							{
								destroyObject(alarmUnit);
							}
						}
					}
				}
			}
		}
		if (hackAlarms.size() > 0)
		{
			setObjVar(hq, "hq.alarm.hack", hackAlarms);
		}
		if (destructAlarms.size() > 0)
		{
			setObjVar(hq, "hq.alarm.destruct", destructAlarms);
		}
		
		return;
	}
	
	
	public static void activateHackAlarms(obj_id hq, boolean activate) throws InterruptedException
	{
		if (hasObjVar(hq, "hq.alarm.hack" ))
		{
			Vector hackAlarmList = getResizeableObjIdArrayObjVar(hq, "hq.alarm.hack");
			for (int i = 0; i< hackAlarmList.size(); i++)
			{
				testAbortScript();
				if (activate == true)
				{
					setCondition(((obj_id)(hackAlarmList.get(i))), CONDITION_ON);
					messageTo(hq, "handleAlarmMute", null, 600, false);
				}
				else
				{
					clearCondition(((obj_id)(hackAlarmList.get(i))), CONDITION_ON);
				}
			}
		}
		return;
	}
	
	
	public static void activateDestructAlarms(obj_id hq, boolean activate) throws InterruptedException
	{
		if (hasObjVar(hq, "hq.alarm.destruct" ))
		{
			Vector destructAlarmList = getResizeableObjIdArrayObjVar(hq, "hq.alarm.destruct");
			for (int i = 0; i< destructAlarmList.size(); i++)
			{
				testAbortScript();
				if (activate == true)
				{
					setCondition(((obj_id)(destructAlarmList.get(i))), CONDITION_ON);
				}
				else
				{
					clearCondition(((obj_id)(destructAlarmList.get(i))), CONDITION_ON);
				}
			}
		}
		return;
	}
	
	
	public static void cleanupBaseAlarmUnits(obj_id hq) throws InterruptedException
	{
		if (hasObjVar(hq, "hq.alarm.hack" ))
		{
			Vector hackAlarms = getResizeableObjIdArrayObjVar(hq, "hq.alarm.hack");
			if (hackAlarms.size () > 0)
			{
				obj_id[] hackAlarmList = new obj_id[hackAlarms.size()];
				hackAlarms.toArray (hackAlarmList);
				utils.destroyObjects(hackAlarmList);
				removeObjVar(hq, "hq.alarm.hack");
			}
		}
		if (hasObjVar(hq, "hq.alarm.destruct" ))
		{
			Vector destructAlarms = getResizeableObjIdArrayObjVar(hq, "hq.alarm.destruct");
			if (destructAlarms.size () > 0)
			{
				obj_id[] destructAlarmList = new obj_id[destructAlarms.size()];
				destructAlarms.toArray (destructAlarmList);
				utils.destroyObjects(destructAlarmList);
				removeObjVar(hq, "hq.alarm.destruct");
			}
		}
	}
	
	
	public static void spawnRovingSecurityTeam(obj_id hq) throws InterruptedException
	{
		String hqTemplate = utils.getTemplateFilenameNoPath(hq);
		int sizeOfBase = 1;
		
		if (hqTemplate.indexOf ("hq_s02") != -1)
		{
			sizeOfBase = 2;
		}
		else if (hqTemplate.indexOf ("hq_s03") != -1)
		{
			sizeOfBase = 3;
		}
		else if (hqTemplate.indexOf ("hq_s04") != -1)
		{
			sizeOfBase = 4;
		}
		
		location start = getGoodLocation(hq, "entry");
		String[] base_cells = getCellNames(hq);
		location[] locs = new location[base_cells.length];
		for (int i = 0; i< base_cells.length; i++)
		{
			testAbortScript();
			locs[i] = getGoodLocation(hq, base_cells[i]);
		}
		
		int numRespondersCheck = rand(0,5);
		int numResponders = 0;
		switch (numRespondersCheck)
		{
			case 5:
			numResponders = 3*sizeOfBase;
			break;
			
			case 4:
			numResponders = 2*sizeOfBase;
			break;
			
			case 3:
			numResponders = sizeOfBase;
			break;
		}
		
		if (numResponders == 0)
		{
			return;
		}
		
		loadReinforcementSpawn(hq);
		
		dictionary params = new dictionary();
		params.put( "locs", locs );
		params.put( "start", start );
		
		for (int i = 0; i < numResponders; i++)
		{
			testAbortScript();
			String guard = guardKeyPicker(hq);
			params.put( "guard", guard );
			messageTo(hq, "handleSpawnSecurityRover", params, rand(i,i+5), false);
		}
		
		return;
	}
	
	
	public static String guardKeyPicker(obj_id hq) throws InterruptedException
	{
		int spawnType = ST_SMALL;
		
		String template = utils.getTemplateFilenameNoPath(hq);
		LOG("hq","hq::guardKeyPicker -- parent template = "+ template);
		if (template == null || template.equals(""))
		{
			return null;
		}
		
		String tbl = TBL_SPAWN_EGG_PATH + template;
		LOG("hq","hq::guardKeyPicker -- tbl = "+ tbl);
		
		String[] spawnList = null;
		spawnList = dataTableGetStringColumnNoDefaults(tbl, "small");
		
		if (spawnList == null || spawnList.length == 0)
		{
			LOG("hq","hq::guardKeyPicker -- spawnList returned from datatable with invalid data...");
			return null;
		}
		
		String spawnKey = spawnList[rand(0,spawnList.length - 1)];
		LOG("hq","hq::guardKeyPicker -- spawnKey = "+ spawnKey);
		if (spawnKey == null || spawnKey.equals(""))
		{
			return null;
		}
		
		return spawnKey;
	}
	
	
	public static void cleanupHqSecurityTeam(obj_id hq) throws InterruptedException
	{
		if (utils.hasScriptVar(hq, "hq.spawn.security" ))
		{
			Vector securityTeam = utils.getResizeableObjIdArrayScriptVar(hq, "hq.spawn.security");
			if (securityTeam.size () > 0)
			{
				obj_id[] securityDetail = new obj_id[securityTeam.size()];
				securityTeam.toArray (securityDetail);
				utils.destroyObjects(securityDetail);
			}
		}
	}
	
	
	public static void loadReinforcementSpawn(obj_id hq) throws InterruptedException
	{
		
		if (utils.hasScriptVar(hq, SCRIPTVAR_SPAWN_REINFORCEMENT))
		{
			return;
		}
		
		String filename = utils.getTemplateFilenameNoPath(hq);
		if (filename != null && !filename.equals(""))
		{
			if (filename.indexOf( "_pvp" ) != -1)
			{
				
				int idx = filename.indexOf("_pvp");
				if (idx >= 0)
				{
					filename = filename.substring(idx-1);
				}
			}
			obj_id reinforcementSpawner = spawnReinforcements(hq, TBL_SPAWN_EXTERIOR_PATH + filename);
			if (isIdValid(reinforcementSpawner))
			{
				utils.setScriptVar(hq, SCRIPTVAR_SPAWN_REINFORCEMENT, reinforcementSpawner);
			}
		}
	}
	
	
	public static obj_id spawnReinforcements(obj_id hq, String tbl) throws InterruptedException
	{
		
		if (!isIdValid(hq) || tbl == null || tbl.equals(""))
		{
			return null;
		}
		
		String myFac = factions.getFaction(hq);
		if (myFac == null || myFac.equals(""))
		{
			return null;
		}
		
		location reinforcementSpawnLoc = utils.findLocInFrontOfTarget(hq, (float)(rand(30.0f,50.0f)));
		
		if (reinforcementSpawnLoc != null)
		{
			obj_id spawn = create.object(TEMPLATE_SPAWN_EGG, reinforcementSpawnLoc);
			
			if (isIdValid(spawn))
			{
				setObjVar(spawn, VAR_SPAWN_TYPE, ST_LARGE);
				setObjVar(spawn, VAR_SPAWN_PARENT, hq);
				setObjVar(spawn, "hq.spawn.reinforcements", 1);
				factions.setFaction(spawn, myFac);
				messageTo(spawn, "handleSpawnRequest", null, rand(1,5), false);
				return spawn;
			}
		}
		
		return null;
	}
	
	
	public static void createMinefield(obj_id hq) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(hq);
		String table = TBL_DEFENSE_PATH + "minefield.iff";
		
		float dx = dataTableGetFloat(table, filename, "x");
		float dz = dataTableGetFloat(table, filename, "z");
		float radius = dataTableGetFloat(table, filename, "radius");
		
		location here = getLocation(hq);
		float yaw = getYaw(hq);
		
		location there = player_structure.transformDeltaWorldCoord(here, dx, dz, yaw);
		there.y = here.y;
		
		obj_id minefield = createObject("object/tangible/faction_perk/faction_base_item/minefield.iff", there);
		
		if (isIdValid(minefield))
		{
			attachScript(minefield, "faction_perk.minefield.advanced_minefield");
			attachScript(minefield, SCRIPT_DEFENSE_OBJECT);
			
			factions.setFaction(minefield, factions.getFaction(hq));
			
			setObjVar(minefield, "pvpCanAttack", 1);
			setInvulnerable(minefield, false);
			persistObject(minefield);
			
			setObjVar(minefield, VAR_DEFENSE_PARENT, hq);
			setOwner(minefield, hq);
			
			obj_id[] data = new obj_id[1];
			data[0] = minefield;
			setObjVar(hq, VAR_DEFENSE_BASE + ".minefield", data);
		}
	}
	
	
	public static int getTotalMines(obj_id hq) throws InterruptedException
	{
		if (!hasObjVar(hq, "mines"))
		{
			return 0;
		}
		
		int[] mines = getIntArrayObjVar(hq, "mines");
		int total = 0;
		
		if (mines == null || mines.length == 0)
		{
			return 0;
		}
		
		for (int i = 0; i < mines.length; i++)
		{
			testAbortScript();
			total += mines[i];
		}
		
		return total;
	}
	
	
	public static float getMinefieldRadius(obj_id hq) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(hq);
		String table = TBL_DEFENSE_PATH + "minefield.iff";
		
		return dataTableGetFloat(table, filename, "radius");
	}
	
	
	public static int getMaxMines(obj_id hq) throws InterruptedException
	{
		String filename = utils.getTemplateFilenameNoPath(hq);
		String table = TBL_DEFENSE_PATH + "minefield.iff";
		
		return dataTableGetInt(table, filename, "max");
	}
	
	
	public static int getRandomMineType(obj_id hq) throws InterruptedException
	{
		if (!hasObjVar(hq, "mines"))
		{
			return -1;
		}
		
		int[] mines = getIntArrayObjVar(hq, "mines");
		if (mines == null || mines.length == 0)
		{
			return -1;
		}
		
		int max = getTotalMines(hq);
		if (max == 0)
		{
			return -1;
		}
		
		int roll = rand(1, 100);
		int base_chance = 0;
		int type_counter = 0;
		int type = -1;
		
		while (type < 0 && type_counter < mines.length)
		{
			testAbortScript();
			if (type_counter == (mines.length - 1))
			{
				type = (mines.length - 1);
			}
			else
			{
				int chance = (int)(((float)mines[type_counter] / (float)max) * 100f);
				base_chance += chance;
				
				if (roll <= base_chance)
				{
					type = type_counter;
				}
			}
			
			type_counter++;
		}
		
		mines[type]--;
		if (mines[type] < 0)
		{
			mines[type] = 0;
		}
		
		setObjVar(hq, "mines", mines);
		
		return type;
	}
	
	
	public static void clearMinefield(obj_id hq) throws InterruptedException
	{
		if (!hasObjVar(hq, "mines"))
		{
			return;
		}
		
		int[] mines = getIntArrayObjVar(hq, "mines");
		if (mines == null || mines.length == 0)
		{
			return;
		}
		
		for (int i = 0; i < mines.length; i++)
		{
			testAbortScript();
			mines[i] = 0;
		}
		
		setObjVar(hq, "mines", mines);
	}
}
