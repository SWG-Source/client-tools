package script.systems.npc_lair;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.utils;
import script.library.ai_lib;
import script.ai.ai_combat;
import script.library.factions;
import script.library.xp;



public class npc_lair extends script.theme_park.poi.base
{
	public npc_lair()
	{
	}
	public static final String TERMINAL_LOGGING = "lair_attack";
	public static final boolean LOGGING_ON = true;
	
	public static final String CREATURE_TABLE = "datatables/mob/creatures.iff";
	public static final int MAX_ITERATIONS = 5;
	public static final int MIN_NON_DIFFICULTY_CREATURES = 2;
	public static final int MAX_NON_DIFFICULTY_CREATURES = 5;
	public static final int MAX_LAIR_HEALTH = 850000;
	
	public static final float CREATURE_LAIR_EXCHANGE = 0.25f;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		location locTest = getLocation(self);
		float fltHeight = getHeightAtLocation(locTest.x, locTest.z);
		
		if (fltHeight != locTest.y)
		{
			locTest.y = fltHeight;
			setLocation(self, locTest);
			
		}
		
		String strLairType = getStringObjVar(self, "spawning.lairType");
		
		if (strLairType == null)
		{
			LOG("DESIGNER_FATAL", "Object "+ self + " at "+ getLocation(self) + " has no lairType objvar. Please fix ASAP");
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		int intIndex = strLairType.indexOf("theater");
		
		if (intIndex > -1)
		{
			if (!hasObjVar(self, "intTheaterCheck"))
			{
				int intRoll = rand(1, 2);
				
				if (intRoll < 75)
				{
					setObjVar(self, "intNoTheaters", 1);
					setObjVar(self, "spawning.buildingType", "none");
				}
				
				setObjVar(self, "intTheaterCheck", 1);
			}
		}
		
		if (hasObjVar(self, "npc_lair.target"))
		{
			removeObjVar(self, "npc_lair.target");
		}
		
		initializePoi(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		initializePoi(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void initializePoi(obj_id poiBaseObject) throws InterruptedException
	{
		obj_id self = getSelf();
		
		if (hasObjVar(self, "npc_lair.numberOfMobiles"))
		{
			int numRespawned = getIntObjVar(self, "npc_lair.numRespawned");
			int population = getIntObjVar(self, "npc_lair.numberOfMobiles");
			
			if (numRespawned >= (population * 3))
			{
				if (!poiIsCompleted(self))
				{
					poiComplete(self);
				}
				
				destroyObject(self);
				
				return;
			}
		}
		
		String diff = getStringObjVar(poiBaseObject, "spawning.lairDifficulty");
		int lairLevel = getIntObjVar(poiBaseObject, "spawning.intDifficultyLevel");
		int groupSize = getIntObjVar(poiBaseObject, "spawning.groupSize");
		String lairType = getStringObjVar(poiBaseObject, "spawning.lairType");
		String lairDatatable = "datatables/npc_lair/"+ lairType + ".iff";
		
		if (!dataTableOpen(lairDatatable))
		{
			destroyObject(self);
			return;
		}
		
		PROFILER_START("npc_lair.script.initializePoi."+ lairType);
		
		String strFaction = utils.getFactionSubString(lairType);
		
		if (strFaction != null)
		{
			setObjVar(poiBaseObject, factions.FACTION, strFaction);
		}
		
		dictionary dctParams = new dictionary();
		
		dctParams.put("lairType", lairType);
		dctParams.put("diff", diff);
		messageTo(poiBaseObject, "doTheater", dctParams, 1, false);
		
		dctParams = new dictionary();
		dctParams.put("lairType", lairType);
		dctParams.put("lairLevel", lairLevel);
		dctParams.put("groupSize", groupSize);
		
		if (lairType.equals("") || lairLevel < 1)
		{
			LOG("designer_fatal", "Failed spawning lairType "+ lairType + " at level "+ lairLevel);
			destroyObject(self);
			return;
		}
		
		messageTo(poiBaseObject, "makeSomeMobiles", dctParams, 4, false);
		
		messageTo(poiBaseObject, "handleNpcAiManagement", null, 20, false);
		PROFILER_STOP("npc_lair.script.initializePoi."+ lairType);
	}
	
	
	public int doTheater(obj_id self, dictionary params) throws InterruptedException
	{
		String lairType = params.getString("lairType");
		String diff = params.getString("diff");
		
		createTheater(self, lairType, diff);
		return SCRIPT_CONTINUE;
	}
	
	
	public int makeSomeMobiles(obj_id self, dictionary params) throws InterruptedException
	{
		String lairType = params.getString("lairType");
		int lairLevel = params.getInt("lairLevel");
		int groupSize = params.getInt("groupSize");
		
		if (groupSize < 1)
		{
			groupSize = 2;
		}
		spawnNpcLairMobiles(self, lairType, lairLevel, groupSize);
		return SCRIPT_CONTINUE;
	}
	
	
	public void flagCreatureLairs(obj_id poiBaseObject) throws InterruptedException
	{
		PROFILER_START("npc_lair.flagCreatureLairs");
		
		if (hasObjVar(poiBaseObject, "npc_lair.isNotCreatureLair"))
		{
			PROFILER_STOP("npc_lair.flagCreatureLairs");
			return;
		}
		
		obj_id target = getObjIdObjVar(poiBaseObject, "npc_lair.target");
		
		if (isIdValid(target))
		{
			setObjVar(poiBaseObject, "npc_lair.isCreatureLair", true);
			setObjVar(target, "npc_lair.isCreatureLair", true);
			setYaw(target, rand(0.0f, 360.0f));
		}
		
		PROFILER_STOP("npc_lair.flagCreatureLairs");
	}
	
	
	public int handleTargetDestroyed(obj_id self, dictionary params) throws InterruptedException
	{
		poiComplete(POI_SUCCESS);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTheaterComplete(obj_id self, dictionary params) throws InterruptedException
	{
		PROFILER_START("npc_lair.handleTheaterComplete.Start");
		obj_id objTheater = params.getObjId("master");
		
		if (!isIdValid(objTheater))
		{
			
			PROFILER_STOP("npc_lair.handleTheaterComplete.Start");
			return SCRIPT_CONTINUE;
		}
		
		PROFILER_START("npc_lair.handleTheaterComplete.ObjVars");
		setObjVar(self, "theater.objTheater", objTheater);
		PROFILER_STOP("npc_lair.handleTheaterComplete.Start");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void createTheater(obj_id poiBaseObject, String lairType, String diff) throws InterruptedException
	{
		
		PROFILER_START("npc_lair.script.createTheatre");
		
		String buildingToSpawn = null;
		
		if (hasObjVar(poiBaseObject, "spawning.buildingType"))
		{
			buildingToSpawn = getStringObjVar(poiBaseObject, "spawning.buildingType");
		}
		else
		{
			String lairDatatable = "datatables/npc_lair/"+ lairType + ".iff";
			
			String[] buildings = dataTableGetStringColumnNoDefaults(lairDatatable, diff);
			
			if (buildings == null)
			{
				PROFILER_STOP("npc_lair.script.createTheatre");
				return;
			}
			if (buildings.length == 0)
			{
				PROFILER_STOP("npc_lair.script.createTheatre");
				return;
			}
			
			buildingToSpawn = buildings[rand(0, buildings.length - 1)];
			setObjVar(poiBaseObject, "spawning.buildingType", buildingToSpawn);
		}
		
		if (buildingToSpawn.startsWith("object/building/poi"))
		{
			buildingToSpawn = "none";
		}
		if (buildingToSpawn.startsWith("object/static/structure/naboo/gungan_relic_s01.iff"))
		{
			buildingToSpawn = "none";
		}
		
		if (!buildingToSpawn.equals("none"))
		{
			obj_id mainBuilding = poiCreateObject(buildingToSpawn, 0f, 0f);
			int intIndex = lairType.indexOf("theater");
			
			if (intIndex < 0)
			{
				if (isIdValid(mainBuilding))
				{
					poiSetDestroyMessage(mainBuilding, "handleBuildingDestruction", 1);
					setYaw(mainBuilding, rand(0.0f, 360.0f));
					setObjVar(poiBaseObject, "npc_lair.target", mainBuilding);
					setHpAndXpValues(poiBaseObject, mainBuilding);
				}
			}
			else
			{
				setObjVar(poiBaseObject, "theater.objTheater", mainBuilding);
			}
		}
		
		PROFILER_STOP("npc_lair.script.createTheatre");
	}
	
	
	public int handleBuildingDestruction(obj_id self, dictionary params) throws InterruptedException
	{
		setObjVar(self, "npc_lair.buildingType", "none");
		poiComplete(POI_SUCCESS);
		return SCRIPT_CONTINUE;
	}
	
	
	public float getAverageDifficulty(float[] fltDifficulties) throws InterruptedException
	{
		float fltAverage = 0;
		
		for (int intI = 0; intI < fltDifficulties.length; intI++)
		{
			testAbortScript();
			fltAverage = fltAverage + fltDifficulties[intI];
		}
		fltAverage = fltAverage / fltDifficulties.length;
		return fltAverage;
		
	}
	
	
	public float getMaxDifficulty(float[] fltDifficulties) throws InterruptedException
	{
		float fltMaxDifficulty = 0;
		
		for (int intI = 0; intI < fltDifficulties.length; intI++)
		{
			testAbortScript();
			if (fltDifficulties[intI] > fltMaxDifficulty)
			{
				fltMaxDifficulty = fltDifficulties[intI];
			}
		}
		
		return fltMaxDifficulty;
	}
	
	
	public void spawnNpcLairMobiles(obj_id poiBaseObject, String lairType, int lairLevel, int groupSize) throws InterruptedException
	{
		PROFILER_START("npc_lair.spawnNpcLairMobiles.setup");
		
		if (utils.hasScriptVar(poiBaseObject, "npc_lair.mobile.0"))
		{
			respawnAllMobiles(poiBaseObject);
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.setup");
			return;
		}
		
		String lairDatatable = "datatables/npc_lair/"+ lairType + ".iff";
		String[] creatureList = dataTableGetStringColumnNoDefaults(lairDatatable, "mobiles");
		
		if (creatureList == null)
		{
			LOG("DESIGNER_FATAL", "WARNING: "+ lairDatatable + " has no mobiles column.");
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.setup");
			destroyObject(getSelf());
			return;
		}
		
		int[] spawnLimit = getSpawnLimit(lairDatatable, creatureList);
		
		if (spawnLimit == null)
		{
			debugServerConsoleMsg(null, "WARNING: "+ lairDatatable + " has no spawn Limit");
			LOG("DESIGNER_FATAL", "WARNING: "+ lairDatatable + " has no spawn Limit");
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.setup");
			destroyObject(getSelf());
			return;
		}
		
		dictionary[] dctCreatureInfo = new dictionary[creatureList.length];
		
		for (int intI = 0; intI < creatureList.length; intI++)
		{
			testAbortScript();
			dctCreatureInfo[intI] = dataTableGetRow(CREATURE_TABLE, creatureList[intI]);
			
			if (dctCreatureInfo[intI] == null)
			{
				PROFILER_STOP("npc_lair.spawnNpcLairMobiles.setup");
				LOG("DESIGNER_FATAL", "For lairType "+ lairType + " and creature "+ creatureList[intI] + " there is no creatures.tab entry!");
				destroyObject(getSelf());
				return;
			}
		}
		
		int[] numSpawned = new int[creatureList.length];
		
		int mobileNumber = 0;
		int intI = 0;
		
		PROFILER_STOP("npc_lair.spawnNpcLairMobiles.setup");
		
		int creatureListLength = creatureList.length;
		
		if (creatureListLength != spawnLimit.length || creatureListLength != numSpawned.length)
		{
			LOG("DESIGNER_FATAL", "Mis aligned arrays for lair type "+ lairType);
			destroyObject(getSelf());
			return;
		}
		
		PROFILER_START("npc_lair.spawnNpcLairMobiles.makeMobs");
		
		int intArrayLength = creatureList.length - 1;
		
		if (intArrayLength < 0)
		{
			intArrayLength = 0;
		}
		
		int xpValue = 0;
		int hpValue = 1500;
		
		int minSpawn = (int) ((groupSize * 1.5f) + 0.5f);
		int maxSpawn = (int) ((groupSize * 2.5f) + 0.5f);
		int spawnAmount = rand(minSpawn, maxSpawn);
		
		for (int intTotalCount = 0; intTotalCount < spawnAmount; intTotalCount++)
		{
			testAbortScript();
			PROFILER_START("npc_lair.spawnNpcLairMobiles.makeMobs.innerLoop");
			
			if (intArrayLength < 0)
			{
				PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs.innerLoop");
				break;
			}
			
			int intIndex = rand(0, intArrayLength);
			
			PROFILER_START("npc_lair.spawnNpcLairMobiles.makeMobs.getting");
			int creatureXpValue = xp.getLevelBasedXP(lairLevel);
			
			xpValue += (int) (CREATURE_LAIR_EXCHANGE * creatureXpValue);
			utils.setScriptVar(poiBaseObject, "npc_lair.mobile."+ mobileNumber, creatureList[intIndex]);
			
			numSpawned[intIndex]++;
			mobileNumber++;
			boolean boolSpawnLimit = false;
			
			if (spawnLimit[intIndex] > 0)
			{
				if (numSpawned[intIndex] >= spawnLimit[intIndex])
				{
					boolSpawnLimit = true;
				}
				
			}
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs.getting");
			PROFILER_START("npc_lair.spawnNpcLairMobiles.makeMobs.swap");
			
			if (boolSpawnLimit)
			{
				String strSwapString = "";
				int intSwapInt = 0;
				float fltSwapFloat = 0;
				
				dictionary dctSwapThing = new dictionary();
				
				dctSwapThing = dctCreatureInfo[intArrayLength];
				dctCreatureInfo[intIndex] = dctSwapThing;
				
				strSwapString = creatureList[intArrayLength];
				creatureList[intArrayLength] = creatureList[intIndex];
				creatureList[intIndex] = strSwapString;
				
				intSwapInt = spawnLimit[intArrayLength];
				spawnLimit[intArrayLength] = spawnLimit[intIndex];
				spawnLimit[intIndex] = intSwapInt;
				
				intSwapInt = numSpawned[intArrayLength];
				numSpawned[intArrayLength] = numSpawned[intIndex];
				numSpawned[intIndex] = intSwapInt;
				
				intArrayLength = intArrayLength - 1;
			}
			
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs.swap");
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs.innerLoop");
		}
		
		obj_id target = getObjIdObjVar(getSelf(), "npc_lair.target");
		
		if (mobileNumber <= 0)
		{
			PROFILER_START("npc_lair.spawnNpcLairMobiles.makeMobs.badDifficulty");
			int intJ = 0;
			
			while (intJ < 3)
			{
				testAbortScript();
				int intRoll = rand(0, creatureList.length - 1);
				obj_id mobile = null;
				
				mobile = spawnMobile(creatureList[intRoll], target, lairLevel);
				
				if (isIdValid(mobile))
				{
					int creatureXpValue = xp.getLevelBasedXP(lairLevel);
					
					xpValue += (int) (CREATURE_LAIR_EXCHANGE * creatureXpValue);
					utils.setScriptVar(poiBaseObject, "npc_lair.mobile."+ mobileNumber, creatureList[intRoll]);
					numSpawned[intRoll]++;
					mobileNumber++;
				}
				intJ = intJ + 1;
			}
			LOG("DESIGNER_FATAL", "BAD MOJO WARNING: object id: "+ poiBaseObject);
			LOG("DESIGNER_FATAL", "BAD MOJO WARNING: lairType: "+ lairType);
			LOG("DESIGNER_FATAL", "BAD MOJO WARNING: mobileNumber: "+ mobileNumber);
			PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs.badDifficulty");
		}
		
		if (lairLevel < 5)
		{
			lairLevel = 5;
		}
		if (lairLevel > 90)
		{
			lairLevel = 90;
		}
		
		int avgAttrib = dataTableGetInt(create.STAT_BALANCE_TABLE, lairLevel - 1, "HP");
		
		int intMinHP = (int) (avgAttrib * 0.9f);
		int intMaxHP = (int) (avgAttrib * 1.1f);
		
		hpValue += 6 * (rand(intMinHP, intMaxHP));
		
		if (groupSize > 1)
		{
			hpValue = hpValue * groupSize;
		}
		
		if (hpValue > MAX_LAIR_HEALTH)
		{
			hpValue = MAX_LAIR_HEALTH;
		}
		
		setObjVar(poiBaseObject, "npc_lair.targetHps", hpValue);
		setObjVar(poiBaseObject, "npc_lair.targetXps", xpValue);
		setObjVar(poiBaseObject, "npc_lair.lairDifficulty", lairLevel);
		
		if (isIdValid(target))
		{
			setObjVar(target, "intCombatDifficulty", lairLevel);
			setMaxHitpoints(target, hpValue);
			setHitpoints(target, hpValue);
		}
		else
		{
			dictionary params = new dictionary();
			
			params.put("hpValue", hpValue);
			
			params.put("intCombatDifficulty", lairLevel);
			messageTo(poiBaseObject, "handleDelayedTargetSetup", params, 2, false);
		}
		
		setObjVar(poiBaseObject, "npc_lair.numberOfMobiles", mobileNumber);
		respawnAllMobiles(poiBaseObject);
		PROFILER_STOP("npc_lair.spawnNpcLairMobiles.makeMobs");
	}
	
	
	public int handleDelayedTargetSetup(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id target = getObjIdObjVar(self, "npc_lair.target");
		
		if (!isIdValid(target))
		{
			return SCRIPT_CONTINUE;
		}
		
		int hpValue = params.getInt("hpValue");
		int intCombatDifficulty = params.getInt("intCombatDifficulty");
		
		setObjVar(target, "intCombatDifficulty", intCombatDifficulty);
		
		if (hpValue > MAX_LAIR_HEALTH)
		{
			hpValue = MAX_LAIR_HEALTH;
		}
		
		setMaxHitpoints(target, hpValue);
		setHitpoints(target, hpValue);
		return SCRIPT_CONTINUE;
	}
	
	
	public int[] getSpawnLimit(String lairDatatable, String[] creatureList) throws InterruptedException
	{
		if (creatureList == null)
		{
			return null;
		}
		
		int[] entireSpawnLimit = dataTableGetIntColumn(lairDatatable, "spawnLimit");
		
		if (entireSpawnLimit == null)
		{
			debugServerConsoleMsg(null, "WARNING: "+ lairDatatable + " has no spawnLimit column.");
			return null;
		}
		
		if (entireSpawnLimit.length < creatureList.length)
		{
			debugServerConsoleMsg(null, "WARNING: "+ lairDatatable + " has mismatched creature, spawnLimit columns.");
			return null;
		}
		
		int[] spawnLimitList = new int[creatureList.length];
		
		for (int i = 0; i < creatureList.length; i++)
		{
			testAbortScript();
			spawnLimitList[i] = entireSpawnLimit[i];
		}
		
		return spawnLimitList;
	}
	
	
	public float[] getDifficultyList(dictionary[] dctCreatureInfo) throws InterruptedException
	{
		float diffArray[] = new float[dctCreatureInfo.length];
		
		for (int i = 0; i < dctCreatureInfo.length; i++)
		{
			testAbortScript();
			if (dctCreatureInfo[i] != null)
			{
				diffArray[i] = (float) dctCreatureInfo[i].getInt("level");
			}
			else
			{
				diffArray[i] = 0;
			}
		}
		
		return diffArray;
	}
	
	
	public float getLowestLevel(obj_id poiBaseObj, float[] difficultyList, int[] spawnLimit, int[] numSpawned) throws InterruptedException
	{
		float lowestLevel = 999;
		
		for (int i = 0; i < difficultyList.length; i++)
		{
			testAbortScript();
			if (spawnLimit[i] == -1 || numSpawned[i] < spawnLimit[i])
			{
				if ((difficultyList[i] < lowestLevel) && (difficultyList[i] > 0))
				{
					lowestLevel = difficultyList[i];
				}
			}
		}
		
		return lowestLevel;
	}
	
	
	public int spawnCreatures(obj_id self, dictionary params) throws InterruptedException
	{
		int mobSpawnLoopNumber = params.getInt("mobSpawnLoopNumber");
		int storedMobSpawnLoopNumber = utils.getIntScriptVar(self, "mobSpawnLoopNumber");
		
		if (storedMobSpawnLoopNumber != mobSpawnLoopNumber)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id poiBaseObject = self;
		int mobileNumber = params.getInt("mobileNumber");
		obj_id target = null;
		
		if (!hasObjVar(poiBaseObject, "npc_lair.target"))
		{
			String lairType = getStringObjVar(poiBaseObject, "spawning.lairType");
			String strFaction = utils.getFactionSubString(lairType);
			
			if (hasObjVar(poiBaseObject, "spawning.target"))
			{
				location locSpawnLocation = getLocation(poiBaseObject);
				
				if (hasObjVar(poiBaseObject, "theater.objTheater"))
				{
					obj_id objTheater = getObjIdObjVar(poiBaseObject, "theater.objTheater");
					
					obj_id[] objObjectiveSpawners = getObjIdArrayObjVar(objTheater, "theater.objObjectiveSpawners");
					
					if (objObjectiveSpawners != null)
					{
						if (objObjectiveSpawners.length > 0)
						{
							locSpawnLocation = getLocation(objObjectiveSpawners[rand(0, objObjectiveSpawners.length - 1)]);
						}
					}
				}
				
				target = poiCreateObject(poiBaseObject, getStringObjVar(poiBaseObject, "spawning.target"), locSpawnLocation);
				
				if (isIdValid(target))
				{
					if (strFaction != null)
					{
						factions.setFaction(target, strFaction);
					}
					
					setYaw(target, rand(0.0f, 360.0f));
					PROFILER_STOP("npc_lair.setYaw");
					string_id strLairName = new string_id("lair_n", lairType);
					String strTest = getString(strLairName);
					
					if (strTest != null && !strTest.equals(""))
					{
						setName(target, strLairName);
					}
					
					poiSetDestroyMessage(target, "handleTargetDestroyed");
					setObjVar(poiBaseObject, "npc_lair.target", target);
					setHpAndXpValues(poiBaseObject, target);
				}
			}
		}
		else
		{
			target = getObjIdObjVar(poiBaseObject, "npc_lair.target");
		}
		
		int lairLevel = getIntObjVar(poiBaseObject, "spawning.intDifficultyLevel");
		
		boolean isCreatureLair = (hasObjVar(poiBaseObject, "npc_lair.isCreatureLair"));
		
		if (!isIdValid(target))
		{
			isCreatureLair = false;
		}
		
		int xpValue = xp.getLevelBasedXP(lairLevel);
		
		String mobileName = utils.getStringScriptVar(poiBaseObject, "npc_lair.mobile."+ mobileNumber);
		
		if (mobileName != null)
		{
			obj_id mobile = null;
			
			if (lairLevel < 1)
			{
				lairLevel = 1;
			}
			
			if (lairLevel > 90)
			{
				lairLevel = 90;
			}
			
			if (utils.dataTableGetInt(CREATURE_TABLE, mobileName, "difficultyClass") != 2)
			{
				mobile = spawnMobile(mobileName, target, lairLevel);
			}
			else
			{
				int baseLevel = utils.dataTableGetInt(CREATURE_TABLE, mobileName, "BaseLevel");
				
				if (baseLevel < lairLevel)
				{
					mobile = spawnMobile(mobileName, target, lairLevel);
				}
				else
				{
					mobile = spawnMobile(mobileName, target, false);
				}
			}
			
			if (isIdValid(mobile))
			{
				if (!ai_lib.isMonster(mobile))
				{
					setObjVar(self, "npc_lair.isNotCreatureLair", true);
				}
				
				attachNPCLairAiScript(mobile, mobileNumber);
				
				if (isCreatureLair)
				{
					int creatureXpValue = getIntObjVar(mobile, "combat.intCombatXP");
					
					xpValue = xpValue + creatureXpValue;
				}
			}
		}
		
		mobileNumber = mobileNumber + 1;
		
		if (utils.hasScriptVar(self, "npc_lair.mobile." + mobileNumber))
		{
			params.put("mobileNumber", mobileNumber);
			
			messageTo(self, "spawnCreatures", params, .50f, false);
		}
		else
		{
			if (isIdValid(target))
			{
				xpValue = (int) (xpValue * CREATURE_LAIR_EXCHANGE);
				attachScript(target, "systems.combat.credit_for_kills");
				flagCreatureLairs(self);
			}
		}
		
		if (isIdValid(target))
		{
			setObjVar(target, "combat.intCombatXP", xpValue);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void respawnAllMobiles(obj_id poiBaseObject) throws InterruptedException
	{
		obj_id self = getSelf();
		int mobSpawnLoopNumber = utils.getIntScriptVar(self, "mobSpawnLoopNumber");
		
		++mobSpawnLoopNumber;
		utils.setScriptVar(self, "mobSpawnLoopNumber", mobSpawnLoopNumber);
		
		dictionary dctParams = new dictionary();
		
		dctParams.put("mobileNumber", 0);
		dctParams.put("mobSpawnLoopNumber", mobSpawnLoopNumber);
		messageTo(poiBaseObject, "spawnCreatures", dctParams, 4, false);
		return;
	}
	
	
	public int handleSpawnWaveOfDefenders(obj_id self, dictionary params) throws InterruptedException
	{
		params.put("mobileNumber", 0);
		messageTo(self, "handleSpawnNextDefender", params, rand(1, 10), false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpawnNextDefender(obj_id self, dictionary params) throws InterruptedException
	{
		blog("handleSpawnNextDefender - init");
		
		int mobileNumber = params.getInt("mobileNumber");
		obj_id poiBaseObject = self;
		obj_id target = getObjIdObjVar(poiBaseObject, "npc_lair.target");
		
		blog("handleSpawnNextDefender - target: "+target);
		
		int lairLevel = getIntObjVar(poiBaseObject, "spawning.intDifficultyLevel");
		int xpValue = xp.getLevelBasedXP(lairLevel);
		
		boolean isCreatureLair = hasObjVar(poiBaseObject, "npc_lair.isCreatureLair");
		
		if (!isIdValid(target))
		{
			isCreatureLair = false;
		}
		
		String mobileName = utils.getStringScriptVar(poiBaseObject, "npc_lair.mobile."+ mobileNumber);
		
		if (mobileName == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id lair = params.getObjId("spawningFor");
		
		if (!isIdValid(lair))
		{
			return SCRIPT_CONTINUE;
		}
		
		int maxHP = getMaxHitpoints(lair);
		int curHP = getHitpoints(lair);
		
		int numSpawned = getIntObjVar(self, "npc_lair.numberOfMobiles");
		int costPerMobile = maxHP / (numSpawned * 5);
		
		if (costPerMobile > curHP)
		{
			return SCRIPT_CONTINUE;
		}
		
		curHP -= costPerMobile;
		setHitpoints(lair, curHP);
		
		obj_id mobile = null;
		
		mobile = spawnMobile(mobileName, target, lairLevel, false);
		
		if (isIdValid(mobile))
		{
			int creatureXpValue = getIntObjVar(mobile, "combat.intCombatXP");
			
			xpValue += (int) (CREATURE_LAIR_EXCHANGE * creatureXpValue);
			attachNPCLairAiScript(mobile, mobileNumber);
			ai_lib.setDefaultCalmBehavior(mobile, ai_lib.BEHAVIOR_LOITER);
		}
		else
		{
			
			debugServerConsoleMsg(self, "designer WARNING: NPC LAIR handleSpawnNextDefender: I couldn't spawn a defender called "+ mobileName);
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(target, "combat.intCombatXP", xpValue);
		
		mobileNumber++;
		
		if (utils.hasScriptVar(poiBaseObject, "npc_lair.mobile." + mobileNumber) && (mobileNumber < 20))
		{
			params.put("mobileNumber", mobileNumber);
			
			int numAttackers = getIntObjVar(self, "npc_lair.numAttackers");
			int delay = rand(15, 20);
			
			if (numAttackers > 15)
			{
				delay = 1;
			}
			else if (numAttackers > 10)
			{
				delay = rand(5, 10);
			}
			else if (numAttackers > 5)
			{
				delay = rand(10, 15);
			}
			
			messageTo(self, "handleSpawnNextDefender", params, delay, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleSpawnBossMonster(obj_id self, dictionary params) throws InterruptedException
	{
		String lairType = getStringObjVar(self, "spawning.lairType");
		
		if (lairType == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String lairDatatable = "datatables/npc_lair/"+ lairType + ".iff";
		
		if (!dataTableHasColumn(lairDatatable, "bossMonster"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] bossMonsters = dataTableGetStringColumnNoDefaults(lairDatatable, "bossMonster");
		
		if (bossMonsters == null || bossMonsters.length < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		location spawnLoc = getLocation(self);
		
		obj_id target = params.getObjId("npc_lair");
		int lairLevel = getIntObjVar(self, "spawning.intDifficultyLevel");
		
		for (int i = 0; i < bossMonsters.length; i++)
		{
			testAbortScript();
			int baseLevel = utils.dataTableGetInt(CREATURE_TABLE, bossMonsters[i], "BaseLevel");
			
			obj_id mobile = null;
			
			if (baseLevel < lairLevel)
			{
				mobile = create.object(bossMonsters[i], spawnLoc, lairLevel);
			}
			else
			{
				mobile = create.object(bossMonsters[i], spawnLoc);
			}
			
			setObjVar(mobile, "npc_lair.target", target);
			stopFloating(mobile);
			
			obj_id[] enemies = getPlayerCreaturesInRange(self, 65.0f);
			
			if (enemies != null && enemies.length > 0)
			{
				obj_id victim = enemies[rand(0, (enemies.length - 1))];
				
				startCombat(mobile, victim);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCreatureDeath(obj_id self, dictionary params) throws InterruptedException
	{
		int numRespawned = getIntObjVar(self, "npc_lair.numRespawned");
		
		numRespawned++;
		int population = getIntObjVar(self, "npc_lair.numberOfMobiles");
		int maxPopulation = population;
		
		if (hasObjVar(self, "npc_lair.isCreatureLair"))
		{
			maxPopulation *= 3;
		}
		
		if (numRespawned >= maxPopulation)
		{
			if (!hasObjVar(self, "npc_lair.target"))
			{
				poiComplete(self, POI_SUCCESS);
			}
			else
			{
				obj_id objTarget = getObjIdObjVar(self, "npc_lair.target");
				
				if (isIdValid(objTarget))
				{
					setObjVar(objTarget, "fltDamageModifier", 5.0f);
					
				}
			}
			messageTo(self, "handlePoiTimeOutDestruction", null, 14400f, false);
			return SCRIPT_CONTINUE;
		}
		else
		{
			setObjVar(self, "npc_lair.numRespawned", numRespawned);
		}
		
		if (hasObjVar(self, "objMission"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String name = params.getString("creatureName");
		
		if (name == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "handleRespawnCreature", params, rand(120, 240), false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleRespawnCreature(obj_id self, dictionary params) throws InterruptedException
	{
		if (poiIsCompleted(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int lairLevel = getIntObjVar(self, "spawning.intDifficultyLevel");
		
		obj_id target = getObjIdObjVar(self, "npc_lair.target");
		String name = params.getString("name");
		
		if (name != null)
		{
			spawnMobile(name, target, lairLevel);
		}
		else
		{
			name = params.getString("creatureName");
			
			if (name != null)
			{
				spawnMobile(name, target, lairLevel);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id spawnMobile(String name, obj_id target) throws InterruptedException
	{
		return spawnMobile(name, target, -1);
	}
	
	
	public obj_id spawnMobile(String name, obj_id target, int level) throws InterruptedException
	{
		return spawnMobile(name, target, level, true);
	}
	
	
	public obj_id spawnMobile(String name, obj_id target, boolean randomLoc) throws InterruptedException
	{
		return spawnMobile(name, target, -1, randomLoc);
	}
	
	
	public obj_id spawnMobile(String name, obj_id target, int level, boolean randomLoc) throws InterruptedException
	{
		boolean boolMobSpawners = false;
		
		obj_id mobile = null;
		obj_id objBase = poiGetBaseObject(target);
		
		if (isIdValid(objBase))
		{
			obj_id objTheater = getObjIdObjVar(objBase, "theater.objTheater");
			
			if (isIdValid(objTheater))
			{
				obj_id[] objMobSpawners = getObjIdArrayObjVar(objTheater, "theater.objMobSpawners");
				
				if (objMobSpawners != null && objMobSpawners.length > 0)
				{
					location locSpawnLocation = getLocation(objMobSpawners[rand(0, objMobSpawners.length - 1)]);
					
					if (locSpawnLocation != null)
					{
						boolMobSpawners = true;
						mobile = poiCreateObject(objBase, name, locSpawnLocation, level);
						stopFloating(mobile);
					}
				}
			}
		}
		
		if (!boolMobSpawners)
		{
			float x = 0.0f;
			float y = 0.0f;
			
			if (randomLoc)
			{
				x = rand(-25f, 25f);
				y = rand(-25f, 25f);
			}
			else
			{
				x = rand(-15f, 15f);
				y = rand(-15f, 15f);
			}
			
			mobile = poiCreateObject(name, x, y, level);
			stopFloating(mobile);
		}
		
		boolean tamable = (utils.dataTableGetFloat(CREATURE_TABLE, name, "canTame") > 0.0f);
		boolean npcLair = hasObjVar(getSelf(), "npc_lair.isNotCreatureLair");
		
		if (hasObjVar(getSelf(), "npc_lair.isCreatureLair"))
		{
			npcLair = false;
		}
		
		if (hasObjVar(objBase, "objMission"))
		{
			tamable = false;
		}
		
		if (isIdValid(mobile))
		{
			if ((!randomLoc) && (!boolMobSpawners))
			{
				location homeLoc = getLocation(mobile);
				
				homeLoc.x += rand(-10, 10);
				homeLoc.z += rand(-10, 10);
				setHomeLocation(mobile, homeLoc);
			}
			
			String faction = factions.getFaction(mobile);
			
			if (faction != null)
			{
				
				if (!factions.isAggro(mobile))
				{
					if (!isPlayer(mobile))
					{
						pvpMakeNeutral(mobile);
					}
					removeObjVar(mobile, "ai.faction.nonAggro");
				}
			}
			
			poiSetDestroyMessage(mobile, "handleCreatureDeath", 1);
			
			if (isIdValid(target))
			{
				setObjVar(mobile, "npc_lair.target", target);
			}
			
			if (!ai_lib.isMonster(mobile))
			{
				tamable = false;
			}
			
			if (!ai_lib.isMonster(mobile))
			{
				setObjVar(getSelf(), "npc_lair.isNotCreatureLair", true);
				npcLair = true;
				
			}
			
			return mobile;
		}
		
		return null;
	}
	
	
	public int handleNpcLairDecay(obj_id self, dictionary params) throws InterruptedException
	{
		poiComplete(POI_INCOMPLETE);
		messageTo(self, "handlePoiTimeOutDestruction", null, 14400f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handlePoiTimeOutDestruction(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void makeBaby(obj_id mobile) throws InterruptedException
	{
		obj_id baseObj = poiGetBaseObject();
		
		String name = ai_lib.getCreatureName(mobile);
		
		if (name == null || name.equals(""))
		{
			return;
		}
		
		float tameChance = utils.dataTableGetFloat(CREATURE_TABLE, name, "canTame");
		
		if (rand(0.0f, 1.0f) > tameChance)
		{
			return;
		}
		
		int numBabiesSpawned = utils.getIntScriptVar(baseObj, "npc_lair.numbabies");
		
		if (numBabiesSpawned > 2 && (rand(1, 100) > 25))
		{
			return;
		}
		
		numBabiesSpawned++;
		utils.setScriptVar(baseObj, "npc_lair.numbabies", numBabiesSpawned);
		attachScript(mobile, "ai.pet_advance");
	}
	
	
	public int handleNpcAiManagement(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar(self, "npc_lair.isCreatureLair"))
		{
			doCreatureLairAiManagement(self);
		}
		else
		{
			doNPCLairAiManagement(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void attachNPCLairAiScript(obj_id mobile, int mobileNumber) throws InterruptedException
	{
		if (hasScript(mobile, "ai.pet_advance"))
		{
			return;
		}
		else if (hasScript(mobile, "ai.tusken_raider"))
		{
			return;
		}
		else if (hasScript(mobile, "ai.soldier"))
		{
			return;
		}
		
		setObjVar(mobile, "npc_lair.mobileNumber", mobileNumber);
		attachScript(mobile, "systems.npc_lair.npc_lair_ai");
	}
	
	public static final int JOB_NONE = 0;
	public static final int JOB_LOITER = 1;
	public static final int JOB_SCOUT = 2;
	public static final int JOB_REST = 3;
	public static final int JOB_DEFEND = 4;
	public static final int JOB_HEAL = 5;
	public static final int JOB_CONVERSE = 6;
	public static final int JOB_DANCE = 7;
	
	
	public int handleScoutAlarm(obj_id self, dictionary params) throws InterruptedException
	{
		
		params.put("job", JOB_DEFEND);
		broadcastMessage("handleNpcLairCustomAi", params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCallForHealing(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!hasObjVar(self, "npc_lair.isCreatureLair"))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		params.put("job", JOB_HEAL);
		broadcastMessage("handleNpcLairCustomAi", params);
		return SCRIPT_CONTINUE;
	}
	
	
	public void doCreatureLairAiManagement(obj_id poiBaseObject) throws InterruptedException
	{
		blog("npc_lair.doCreatureLairAiManagement - init");
		
		int numMobiles = getIntObjVar(poiBaseObject, "npc_lair.numberOfMobiles");
		blog("npc_lair.doCreatureLairAiManagement - numMobiles: "+numMobiles);
		
		if (numMobiles < 2 || numMobiles > 20)
		{
			return;
		}
		
		blog("npc_lair.doCreatureLairAiManagement - numMobiles is greater than 2 and less than 20: "+numMobiles);
		
		int[] jobAssignment = new int[numMobiles];
		
		for (int currentMob = 0; currentMob < numMobiles; currentMob++)
		{
			testAbortScript();
			blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+currentMob);
			blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+numMobiles);
			
			if (currentMob < (numMobiles / 5) * 2)
			{
				blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+currentMob+" has JOB_LOITER");
				jobAssignment[currentMob] = JOB_LOITER;
			}
			else if (currentMob < ((numMobiles / 5) * 3))
			{
				blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+currentMob+" has JOB_SCOUT");
				jobAssignment[currentMob] = JOB_SCOUT;
			}
			else if (currentMob < ((numMobiles / 5) * 4))
			{
				blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+currentMob+" has JOB_REST");
				jobAssignment[currentMob] = JOB_REST;
			}
			else
			{
				blog("npc_lair.doCreatureLairAiManagement - currentMob #: "+currentMob+" has JOB_LOITER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				jobAssignment[currentMob] = JOB_LOITER;
			}
		}
		
		dictionary params = new dictionary();
		
		params.put("jobAssignments", jobAssignment);
		params.put("baseObj", poiBaseObject);
		
		broadcastMessage("handleNpcLairCustomAi", params);
		
		return;
	}
	
	
	public void doNPCLairAiManagement(obj_id poiBaseObject) throws InterruptedException
	{
		int numMobiles = getIntObjVar(poiBaseObject, "npc_lair.numberOfMobiles");
		
		if (numMobiles < 2 || numMobiles > 20)
		{
			return;
		}
		
		location danceLoc = new location(getLocation(poiBaseObject));
		
		switch (rand(1, 4))
		{
			case 1:
			danceLoc.x += rand(7, 10);
			danceLoc.z += rand(7, 10);
			break;
			case 2:
			danceLoc.x -= rand(7, 10);
			danceLoc.z += rand(7, 10);
			break;
			case 3:
			danceLoc.x += rand(7, 10);
			danceLoc.z -= rand(7, 10);
			break;
			case 4:
			danceLoc.x -= rand(7, 10);
			danceLoc.z -= rand(7, 10);
			break;
		}
		setObjVar(poiBaseObject, "npc_lair.danceLoc", danceLoc);
		
		int[] jobAssignment = new int[numMobiles];
		
		for (int currentMob = 0; currentMob < numMobiles; currentMob++)
		{
			testAbortScript();
			switch (currentMob)
			{
				case 0:
				case 1:
				jobAssignment[currentMob] = JOB_CONVERSE;
				break;
				case 2:
				jobAssignment[currentMob] = JOB_DANCE;
				break;
				case 3:
				case 4:
				jobAssignment[currentMob] = JOB_REST;
				break;
				default:
				jobAssignment[currentMob] = JOB_NONE;
				break;
			}
		}
		
		dictionary params = new dictionary();
		
		params.put("jobAssignments", jobAssignment);
		params.put("baseObj", poiBaseObject);
		
		broadcastMessage("handleNpcLairCustomAi", params);
		
		return;
	}
	
	
	public void setHpAndXpValues(obj_id poiBaseObject, obj_id target) throws InterruptedException
	{
		if (hasObjVar(poiBaseObject, "npc_lair.targetHps"))
		{
			int hpValue = getIntObjVar(poiBaseObject, "npc_lair.targetHps");
			
			setMaxHitpoints(target, hpValue);
			setHitpoints(target, hpValue);
		}
		if (hasObjVar(poiBaseObject, "npc_lair.targetXps"))
		{
			int xpValue = getIntObjVar(poiBaseObject, "npc_lair.targetXps");
			
			setObjVar(target, "combat.intCombatXP", xpValue);
		}
		if (hasObjVar(poiBaseObject, "npc_lair.lairDifficulty"))
		{
			int diff = getIntObjVar(poiBaseObject, "npc_lair.lairDifficulty");
			
			if (diff < 4)
			{
				diff = 4;
			}
			setObjVar(target, "intCombatDifficulty", diff);
		}
		
	}
	
	
	public boolean canOfferMission(obj_id mobile) throws InterruptedException
	{
		if (rand(1, 10) > 5)
		{
			return false;
		}
		
		String creatureName = ai_lib.getCreatureName(mobile);
		
		if (creatureName == null)
		{
			return false;
		}
		
		return (dataTableGetInt(CREATURE_TABLE, creatureName, "canOfferMission") == 1);
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (msg == null || msg.equals(""))
		{
			return false;
		}
		if (LOGGING_ON)
		{
			LOG(TERMINAL_LOGGING, msg);
		}
		
		return true;
	}
}
