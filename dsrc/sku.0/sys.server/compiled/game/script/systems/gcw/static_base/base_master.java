package script.systems.gcw.static_base;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.utils;


public class base_master extends script.base_script
{
	public base_master()
	{
	}
	public static final int NO_CONTROL = 0;
	public static final int IMPERIAL_CONTROL = 1;
	public static final int REBEL_CONTROL = 2;
	
	public static final String VAR_TERMINAL_IDS = "gcw.static_base.terminal_ids";
	public static final String VAR_TERMINAL_STATUS = "gcw.static_base.terminal_status";
	public static final String VAR_MASTER = "gcw.static_base.master";
	public static final String VAR_BASE_STATUS = "gcw.static_base.base_status";
	public static final String VAR_BASE_LAST_CAPTURE = "gcw.static_base.last_capture";
	public static final String VAR_ACCESS_DELAY = "gcw.static_base.access_delay";
	
	public static final String SCRIPT_VAR_VALIDATION = "gcw.static_base.validation";
	
	public static final String TABLE_TERMINAL_SPAWN = "datatables/gcw/static_base/terminal_spawn.iff";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		obj_id master = findMasterObject(self);
		
		if (!isIdValid(master))
		{
			dictionary d = new dictionary();
			d.put("tries", 0);
			
			messageTo(self, "handleBaseInitializationRetry", d, 30.0f, false);
		}
		else
		{
			initializeBase(self, master);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		obj_id master = findMasterObject(self);
		
		if (!isIdValid(master))
		{
			dictionary d = new dictionary();
			d.put("tries", 0);
			
			messageTo(self, "handleBaseInitializationRetry", d, 30.0f, false);
		}
		else
		{
			initializeBase(self, master);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public obj_id findMasterObject(obj_id self) throws InterruptedException
	{
		LOG("gcw_testing","Attempting to find Master Object: ");
		obj_id[] objects = getObjectsInRange(getLocation(self), 100f);
		
		if (objects == null || objects.length == 0)
		{
			return obj_id.NULL_ID;
		}
		
		for (int i = 0; i < objects.length; i++)
		{
			testAbortScript();
			if (isIdValid(objects[i]))
			{
				if (hasScript(objects[i], "systems.gcw.static_base.master"))
				{
					LOG("gcw_testing","MASTER OID: "+objects[i]);
					return objects[i];
				}
			}
		}
		
		return obj_id.NULL_ID;
	}
	
	
	public void initializeBase(obj_id self, obj_id master) throws InterruptedException
	{
		if (isIdValid(master))
		{
			setObjVar(self, VAR_MASTER, master);
			
			if (hasObjVar(master, VAR_BASE_STATUS))
			{
				int status = getIntObjVar(master, VAR_BASE_STATUS);
				setObjVar(self, VAR_BASE_STATUS, status);
			}
			
			if (hasObjVar(master, VAR_TERMINAL_IDS))
			{
				obj_id[] terminals = getObjIdArrayObjVar(master, VAR_TERMINAL_IDS);
				setObjVar(self, VAR_TERMINAL_IDS, terminals);
			}
		}
		
		if (!hasScript(self, "systems.gcw.static_base.base_spawner"))
		{
			attachScript(self, "systems.gcw.static_base.base_spawner");
		}
		
		messageTo(self, "handleTerminalSetup", null, 1.0f, false);
	}
	
	
	public void beginTerminalSpawnSequence(obj_id self) throws InterruptedException
	{
		float spawnTime = 10.0f;
		
		if (hasObjVar(self, VAR_TERMINAL_IDS))
		{
			destroyTerminals(self);
			spawnTime = 60.0f;
		}
		
		messageTo(self, "handleTerminalSpawnRequest", null, spawnTime, false);
	}
	
	
	public void validateTerminals(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, VAR_TERMINAL_IDS))
		{
			return;
		}
		
		obj_id[] terminals = getObjIdArrayObjVar(self, VAR_TERMINAL_IDS);
		
		if (terminals == null || terminals.length == 0)
		{
			return;
		}
		
		int[] validate = new int[terminals.length];
		
		dictionary d = new dictionary();
		d.put("master", self);
		
		for (int i = 0; i < terminals.length; i++)
		{
			testAbortScript();
			if (isIdValid(terminals[i]))
			{
				messageTo(terminals[i], "handleTerminalValidationRequest", d, 20.0f, false);
			}
			
			validate[i] = 0;
		}
		
		utils.setScriptVar(self, SCRIPT_VAR_VALIDATION, validate);
		
		messageTo(self, "handleTerminalValidation", null, 60.0f, false);
	}
	
	
	public void destroyTerminals(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, VAR_TERMINAL_IDS))
		{
			return;
		}
		
		obj_id[] terminals = getObjIdArrayObjVar(self, VAR_TERMINAL_IDS);
		
		if (terminals == null || terminals.length == 0)
		{
			return;
		}
		
		for (int i = 0; i < terminals.length; i++)
		{
			testAbortScript();
			if (isIdValid(terminals[i]))
			{
				messageTo(terminals[i], "handleTerminalDestructionRequest", null, 20.0f, false);
			}
		}
		
		if (hasObjVar(self, VAR_TERMINAL_STATUS))
		{
			removeObjVar(self, VAR_TERMINAL_STATUS);
		}
	}
	
	
	public void testBaseStatusChange(obj_id self) throws InterruptedException
	{
		int status = NO_CONTROL;
		
		if (hasObjVar(self, VAR_BASE_STATUS))
		{
			status = getIntObjVar(self, VAR_BASE_STATUS);
		}
		
		int[] terminalStatus = null;
		
		if (!hasObjVar(self, VAR_TERMINAL_STATUS))
		{
			return;
		}
		
		terminalStatus = getIntArrayObjVar(self, VAR_TERMINAL_STATUS);
		
		if (terminalStatus == null || terminalStatus.length == 0)
		{
			return;
		}
		
		int testCase = terminalStatus[0];
		
		for (int i = 0; i < terminalStatus.length; i++)
		{
			testAbortScript();
			if (testCase != terminalStatus[i])
			{
				return;
			}
		}
		
		if (status != testCase)
		{
			beginBaseFactionSwitcheroo(self, testCase);
		}
	}
	
	
	public void beginBaseFactionSwitcheroo(obj_id self, int status) throws InterruptedException
	{
		LOG("gcw_testing", "beginBaseFactionSwitcheroo setting: "+VAR_BASE_STATUS+ " to "+status);
		setObjVar(self, VAR_BASE_STATUS, status);
		
		String faction = "imperial_";
		
		if (status == REBEL_CONTROL)
		{
			faction = "rebel_";
		}
		
		location loc = getLocation(self);
		
		string_id message = new string_id("gcw", "static_base_capture_"+ faction + loc.area);
		
		sendFactionalSystemMessagePlanet(message, null, -1.0f, true, true);
		
		obj_id master = getObjIdObjVar(self, VAR_MASTER);
		LOG("gcw_testing", "beginBaseFactionSwitcheroo master: "+master);
		
		if (isIdValid(master))
		{
			setObjVar(master, VAR_BASE_LAST_CAPTURE, getGameTime());
			setObjVar(master, VAR_BASE_STATUS, status);
		}
		
		if (hasObjVar(self, VAR_TERMINAL_IDS))
		{
			obj_id[] terminals = getObjIdArrayObjVar(self, VAR_TERMINAL_IDS);
			
			if (terminals != null && terminals.length > 0)
			{
				for (int i = 0; i < terminals.length; i++)
				{
					testAbortScript();
					if (isIdValid(terminals[i]))
					{
						setObjVar(terminals[i], VAR_ACCESS_DELAY, (getGameTime()+ 30));
					}
				}
			}
		}
		
		messageTo(self, "handleBaseCleanupRequest", null, 0.0f, false);
		messageTo(self, "handleBeginSpawnRequest", null, 2.0f, false);
		
	}
	
	
	public int handleBaseInitializationRetry(obj_id self, dictionary params) throws InterruptedException
	{
		int tries = params.getInt("tries");
		obj_id master = findMasterObject(self);
		
		if (!isIdValid(master) && tries < 5)
		{
			tries++;
			
			dictionary d = new dictionary();
			d.put("tries", tries);
			
			messageTo(self, "handleBaseInitializationRetry", d, 30.0f, false);
		}
		else
		{
			initializeBase(self, master);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTerminalSetup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, VAR_TERMINAL_IDS))
		{
			beginTerminalSpawnSequence(self);
		}
		else
		{
			validateTerminals(self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTerminalSpawnRequest(obj_id self, dictionary params) throws InterruptedException
	{
		int status = NO_CONTROL;
		
		if (hasObjVar(self, VAR_BASE_STATUS))
		{
			status = getIntObjVar(self, VAR_BASE_STATUS);
		}
		
		String planet = getLocation(self).area;
		
		if (planet == null || planet.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		String[] planetList = dataTableGetStringColumn(TABLE_TERMINAL_SPAWN, "planet");
		
		if (planetList == null || planetList.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int counter = 0;
		Vector v = new Vector();
		
		for (int i = 0; i < planetList.length; i++)
		{
			testAbortScript();
			if (planet.equals(planetList[i]))
			{
				dictionary spawnData = dataTableGetRow(TABLE_TERMINAL_SPAWN, i);
				float x = spawnData.getFloat("loc_x");
				float y = spawnData.getFloat("loc_y");
				float z = spawnData.getFloat("loc_z");
				float yaw = spawnData.getFloat("yaw");
				String cellName = spawnData.getString("cell");
				String template = spawnData.getString("template");
				
				obj_id cell = getCellId(self, cellName);
				location loc = new location(x, y, z, planet, cell);
				
				if (template == null || template.equals(""))
				{
					continue;
				}
				
				obj_id terminal = create.object(template, loc);
				
				setYaw(terminal, yaw);
				
				if (isIdValid(terminal))
				{
					dictionary d = new dictionary();
					d.put("master", self);
					d.put("id", counter);
					d.put("control", status);
					
					messageTo(terminal, "handleTerminalInitialization", d, 1.0f, false);
					
					v.addElement(terminal);
					++counter;
				}
			}
		}
		
		setObjVar(self, VAR_TERMINAL_IDS, v);
		
		obj_id master = getObjIdObjVar(self, VAR_MASTER);
		if (isIdValid(master))
		{
			setObjVar(master, VAR_TERMINAL_IDS, v);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTerminalValidationResponse(obj_id self, dictionary params) throws InterruptedException
	{
		int response = params.getInt("response");
		int id = params.getInt("id");
		
		int[] validate = utils.getIntArrayScriptVar(self, SCRIPT_VAR_VALIDATION);
		
		if (validate == null || validate.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (id < 0 || id >= validate.length)
		{
			return SCRIPT_CONTINUE;
		}
		
		validate[id] = response;
		
		utils.setScriptVar(self, SCRIPT_VAR_VALIDATION, validate);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleTerminalValidation(obj_id self, dictionary params) throws InterruptedException
	{
		int[] validate = utils.getIntArrayScriptVar(self, SCRIPT_VAR_VALIDATION);
		
		if (validate == null || validate.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < validate.length; i++)
		{
			testAbortScript();
			if (validate[i] < 1)
			{
				beginTerminalSpawnSequence(self);
				break;
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleControlUpdate(obj_id self, dictionary params) throws InterruptedException
	{
		int id = params.getInt("id");
		int control = params.getInt("control");
		
		int[] terminalStatus = null;
		
		if (hasObjVar(self, VAR_TERMINAL_STATUS))
		{
			terminalStatus = getIntArrayObjVar(self, VAR_TERMINAL_STATUS);
		}
		else
		{
			if (hasObjVar(self, VAR_TERMINAL_IDS))
			{
				obj_id[] temp = getObjIdArrayObjVar(self, VAR_TERMINAL_IDS);
				terminalStatus = new int[temp.length];
				
				for (int i = 0; i < terminalStatus.length; i++)
				{
					testAbortScript();
					terminalStatus[i] = 0;
				}
			}
			else
			{
				return SCRIPT_CONTINUE;
			}
		}
		
		if (terminalStatus == null || terminalStatus.length == 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (id < 0 || id >= terminalStatus.length)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (control != NO_CONTROL && control != REBEL_CONTROL && control != IMPERIAL_CONTROL)
		{
			return SCRIPT_CONTINUE;
		}
		
		terminalStatus[id] = control;
		
		setObjVar(self, VAR_TERMINAL_STATUS, terminalStatus);
		
		testBaseStatusChange(self);
		
		return SCRIPT_CONTINUE;
	}
}
