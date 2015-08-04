package script.city;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.create;
import script.library.utils;
import script.library.locations;
import script.library.gcw;


public class guard_spawner extends script.base_script
{
	public guard_spawner()
	{
	}
	public static final String guardTable = "datatables/npc/guard_spawner/guard.iff";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo( self, "checkForStart", null, 3, false );
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		location here = getLocation( self );
		String city = locations.getCityName( here );
		if (city != null)
		{
			if (city.equals("bestine"))
			{
				if (!hasScript( self, "city.bestine.politician_event_npc" ))
				{
					attachScript( self, "city.bestine.politician_event_npc");
				}
			}
			else if (city.equals("theed"))
			{
				String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
				if (empiredayRunning != null && (empiredayRunning.equals("true") || empiredayRunning.equals("1")))
				{
					utils.setScriptVar(self, "OFF", "DUE TO EMPIRE DAY");
					
					return SCRIPT_CONTINUE;
					
				}
			}
			
		}
		
		if (hasObjVar (self, "current"))
		{
			killAll( self );
			messageTo( self, "checkForStart", null, 30, false );
		}
		else
		{
			messageTo( self, "checkForStart", null, 30, false );
		}
		
		if (!hasScript( self, "systems.gcw.gcw_data_updater" ))
		{
			attachScript( self, "systems.gcw.gcw_data_updater");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!hasObjVar (speaker, "gm"))
		{
			return SCRIPT_OVERRIDE;
		}
		int stringCheck = text.indexOf("max");
		String population = text.substring( text.indexOf(" ")+1, text.length() );
		int maxPop = utils.stringToInt( population );
		if (stringCheck > -1)
		{
			setObjVar (self, "pop", maxPop);
		}
		if (text.equals("killall"))
		{
			killAll( self );
		}
		else if (text.equals("restart"))
		{
			messageTo( self, "startSpawning", null, 1, false );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkForStart(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar (self, "pop"))
		{
			messageTo( self, "startSpawning", null, 10, false );
			return SCRIPT_CONTINUE;
		}
		else
		{
			setObjVar( self, "pop", 5 );
			messageTo( self, "checkForStart", null, 3, false );
			return SCRIPT_CONTINUE;
		}
	}
	
	
	public int startSpawning(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!utils.hasScriptVar( self, "police" ))
		{
			if (rand() < 0.25f)
			{
				utils.setScriptVar( self, "police", 1 );
			}
			else
			{
				utils.setScriptVar( self, "police", 0 );
			}
		}
		
		int maxPop = getIntObjVar( self, "pop");
		
		if (maxPop <= 5)
		{
			location here = getLocation( self );
			String city = locations.getCityName( here );
			if (city != null)
			{
				if (city.equals("bestine") && hasObjVar(self, "bestine.electionWinner"))
				{
					String electionWinner = getStringObjVar( self, "bestine.electionWinner");
					if (electionWinner.equals("victor") || electionWinner.equals("Victor"))
					{
						maxPop = (int)(maxPop*2.5);
					}
					else
					{
						maxPop = (int)(maxPop*1.5);
					}
				}
			}
		}
		
		if (!hasObjVar (self, "current"))
		{
			setObjVar( self, "current", 0 );
			doSpawn( self, false );
		}
		else
		{
			int currentPop = getIntObjVar( self, "current");
			if (currentPop < maxPop)
			{
				doSpawn( self, false );
			}
			else
			{
				utils.removeScriptVar( self, "police");
				return SCRIPT_OVERRIDE;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int guardSpawnerRespawnNpc(obj_id self, dictionary params) throws InterruptedException
	{
		doSpawn( self, true );
		return SCRIPT_CONTINUE;
	}
	
	
	public void doSpawn(obj_id self, boolean onlySpawnOne) throws InterruptedException
	{
		String type = getGuardArea( self );
		
		location me = getLocation(self);
		location here = locations.getGoodLocationAroundLocation(me, 1f, 1f, 1.5f, 1.5f);
		if (here == null)
		{
			debugServerConsoleMsg (self, "guard_spawner couldn't find a good location");
			return;
		}
		
		String spawn = npcToSpawn( type );
		obj_id npc = create.object( spawn, here );
		if (!isIdValid(npc))
		{
			logScriptDataError("unable to spawn "+ spawn + " + at loc "+ here );
			return;
		}
		attachScript (npc, "city.guard_wander");
		attachScript (npc, "city.city_mob_death_msg");
		
		setObjVar(npc, "cityMobSpawner", self);
		
		if (!hasObjVar (self, "myCreations"))
		{
			Vector myCreations = new Vector();
			myCreations.setSize(0);
			utils.addElement (myCreations, npc);
			setObjVar (self, "myCreations", myCreations);
		}
		else
		{
			Vector theList = getResizeableObjIdArrayObjVar (self, "myCreations");
			int maxPop = getIntObjVar (self, "pop");
			
			if (theList.size() >= maxPop)
			{
				CustomerServiceLog ("SPAWNER_OVERLOAD", "Tried to spawn something even though the list was full.");
				return;
			}
			else
			{
				theList.add (npc);
				setObjVar (self, "myCreations", theList);
			}
		}
		
		int currentPop = getIntObjVar (self, "current");
		currentPop = currentPop + 1;
		
		if (currentPop == 1)
		{
			setObjVar (self, "leader", npc);
		}
		else
		{
			obj_id leader = getObjIdObjVar (self, "leader");
			ai_lib.followInFormation( npc, leader , ai_lib.FORMATION_COLUMN, currentPop-1);
			debugSpeakMsg (self, "Formation # "+ (currentPop-1));
		}
		
		setObjVar (self, "current", currentPop);
		
		if (!onlySpawnOne)
		{
			messageTo (self, "startSpawning", null, 10, false);
		}
		
		return;
	}
	
	
	public String npcToSpawn(String type) throws InterruptedException
	{
		String[] guardList = dataTableGetStringColumnNoDefaults( guardTable, type );
		int guardNum = rand( 0, guardList.length -1 );
		String guard = guardList[guardNum];
		return guard;
	}
	
	
	public int cityMobKilled(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id deadNpc = params.getObjId ("deadNpc");
		
		if (!hasObjVar (self, "myCreations"))
		{
			CustomerServiceLog ("SPAWNER_OVERLOAD", "Spawner "+ self + " tried to spawn a replacement for "+ deadNpc + " but I don't have a list!");
			return SCRIPT_CONTINUE;
		}
		
		Vector spawnedList = getResizeableObjIdArrayObjVar (self, "myCreations");
		
		if (spawnedList.contains(deadNpc))
		{
			int currentPop = getIntObjVar (self, "current");
			currentPop = currentPop - 1;
			
			if (currentPop < 0)
			{
				currentPop = 0;
				CustomerServiceLog ("SPAWNER_OVERLOAD", "Count went below 0 on "+ self + " in Guard_spawner script.");
			}
			
			setObjVar(self, "current", currentPop);
			
			spawnedList.remove(deadNpc);
			
			if (spawnedList.size() > 0)
			{
				setObjVar (self, "myCreations", spawnedList);
			}
			else
			{
				removeObjVar (self, "myCreations");
			}
			
			messageTo (self, "guardSpawnerRespawnNpc", null, 10, false);
		}
		else
		{
			CustomerServiceLog ("SPAWNER_OVERLOAD", "Spawner "+ self + " tried to spawn a replacement for "+ deadNpc + " but he's not on my list!");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void killAll(obj_id self) throws InterruptedException
	{
		
		if (hasObjVar (self, "spawn"))
		{
			int increment = 1;
			int totalBodies = getIntObjVar (self, "pop");
			
			while (increment <= totalBodies)
			{
				testAbortScript();
				obj_id npcToKill = getObjIdObjVar( self, "spawn."+ increment);
				
				if (npcToKill != null)
				{
					destroyObject (npcToKill);
					removeObjVar (self, "spawn."+ increment);
				}
				increment = increment + 1;
			}
			
			removeObjVar( self, "spawn");
		}
		
		if (hasObjVar (self, "myCreations"))
		{
			Vector theList = getResizeableObjIdArrayObjVar (self, "myCreations");
			
			for (int i = 0; i < theList.size(); ++i)
			{
				testAbortScript();
				destroyObject( ((obj_id)(theList.get(i))) );
			}
			
			removeObjVar( self, "myCreations");
		}
		
		removeObjVar( self, "current");
		return;
	}
	
	
	public String getGuardArea(obj_id self) throws InterruptedException
	{
		String guardAreaName = "tatooine";
		
		location here = getLocation( getSelf() );
		String planet = here.area;
		String city = locations.getCityName( here );
		
		if (city == null)
		{
			city = locations.getGuardSpawnerRegionName( here );
		}
		
		if ((city != null) && (planet.equals("tatooine") || planet.equals("naboo") || planet.equals("corellia")))
		{
			
			float imp_r = gcw.getImperialRatio( self );
			float reb_r = gcw.getRebelRatio( self );
			
			String test_city = city;
			if (imp_r >= reb_r)
			{
				float delta = imp_r - reb_r;
				if (delta > 2.f)
				{
					test_city = test_city + "_imperial_hard";
				}
				else
				{
					test_city = test_city + "_imperial";
				}
			}
			else
			{
				float delta = reb_r - imp_r;
				if (delta > 2.f)
				{
					test_city = test_city + "_rebel_hard";
				}
				else
				{
					test_city = test_city + "_rebel";
				}
			}
			
			if (utils.getIntScriptVar( self, "police" ) == 1)
			{
				LOG( "spawn_guard", "Trying police! "+ city);
				if (dataTableHasColumn( guardTable, city + "_police" ))
				{
					test_city = city + "_police";
				}
			}
			
			if (dataTableHasColumn( guardTable, test_city ))
			{
				city = test_city;
			}
			
			LOG( "spawn_guard", "Spawning a guard from table "+ city + " (tried "+ test_city + ")");
		}
		
		if (city == null)
		{
			guardAreaName = planet;
		}
		else
		{
			
			if (dataTableHasColumn(guardTable, city))
			{
				guardAreaName = city;
			}
			else
			{
				guardAreaName = planet;
			}
		}
		
		setObjVar( getSelf(), "guardArea", guardAreaName );
		return guardAreaName;
	}
	
}
