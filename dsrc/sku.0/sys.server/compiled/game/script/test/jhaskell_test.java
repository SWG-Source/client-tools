package script.test;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.city;
import script.library.qa;
import script.library.utils;


public class jhaskell_test extends script.base_script
{
	public jhaskell_test()
	{
	}
	public static final int SF_COST_CITY_LOW = 16;
	public static final int SF_SKILL_TRAINER = 64;
	
	public static final String CITY_SKILL_TRAINERS = "datatables/city/skill_trainers.iff";
	
	
	public int OnSpeaking(obj_id self, String text) throws InterruptedException
	{
		if (isGod(self))
		{
			debugConsoleMsg( self, "jhaskell_test OnSpeaking: "+ text );
			
			java.util.StringTokenizer tok = new java.util.StringTokenizer (text);
			
			if (tok.hasMoreTokens ())
			{
				String priCommand = tok.nextToken ();
				String secCommand = "";
				if (tok.hasMoreTokens ())
				{
					secCommand = tok.nextToken ();
				}
				
				debugConsoleMsg( self, "command is: "+ priCommand);
				
				if (priCommand.equalsIgnoreCase ("what_scene") && secCommand.equals(""))
				{
					sendSystemMessage(self, "You are in scene: "+ getCurrentSceneName(), null);
				}
				else if (priCommand.equalsIgnoreCase ("is_tutorial") && secCommand.equals(""))
				{
					sendSystemMessage(self, "Is in Tutorial: "+ isInTutorialArea(self), null);
				}
				else if (priCommand.equalsIgnoreCase ("city") && secCommand.equalsIgnoreCase ("set_trainers"))
				{
					
					obj_id structure = qa.findTarget(self);
					if (!isValidId(structure) || !exists(structure))
					{
						
						sendSystemMessage(self, "You must target object", null);
					}
					int city_id = findCityByCityHall( structure );
					if (city_id <= 0)
					{
						
						sendSystemMessage(self, "You must target city hall", null);
					}
					if (!cityExists( city_id ))
					{
						
						sendSystemMessage(self, "Structure does't exist", null);
						return SCRIPT_CONTINUE;
					}
					
					boolean successSkillTrainers = setCitySkillTrainers(self, city_id);
					if (successSkillTrainers)
					{
						sendSystemMessage(self, "Legacy Trainer Spawners placed", null);
					}
					else
					{
						sendSystemMessage(self, "The Trainer Routine Failed", null);
					}
					
				}
				else if (priCommand.equalsIgnoreCase ("city") && secCommand.equalsIgnoreCase ("destroy_trainers"))
				{
					
					obj_id structure = qa.findTarget(self);
					if (!isValidId(structure) || !exists(structure))
					{
						
						sendSystemMessage(self, "You must target object", null);
					}
					int city_id = findCityByCityHall( structure );
					if (city_id <= 0)
					{
						
						sendSystemMessage(self, "You must target city hall", null);
					}
					if (!cityExists( city_id ))
					{
						
						sendSystemMessage(self, "Structure does't exist", null);
						return SCRIPT_CONTINUE;
					}
					
					boolean destroySuccess = destroyCitySkillTrainers(self, city_id);
				}
				else if (priCommand.equalsIgnoreCase ("city") && secCommand.equalsIgnoreCase ("get_structures"))
				{
					
					obj_id structure = qa.findTarget(self);
					if (!isValidId(structure) || !exists(structure))
					{
						
						sendSystemMessage(self, "You must target object", null);
					}
					int city_id = findCityByCityHall( structure );
					if (city_id <= 0)
					{
						
						sendSystemMessage(self, "You must target city hall", null);
					}
					if (!cityExists( city_id ))
					{
						
						sendSystemMessage(self, "Structure does't exist", null);
						return SCRIPT_CONTINUE;
					}
					
					obj_id[] structures = cityGetStructureIds( city_id );
					for (int i = 0; i < structures.length; i++)
					{
						testAbortScript();
						sendSystemMessage(self, "Found: "+getName(structures[i]), null);
					}
					sendSystemMessage(self, "getTrainerCount: "+city.getTrainerCount( city_id ), null);
					sendSystemMessage(self, "getMaxTrainerCount: "+city.getMaxTrainerCount( city_id ), null);
				}
				else if (priCommand.equalsIgnoreCase ("get_name") && secCommand.equals(""))
				{
					
					obj_id target = qa.findTarget(self);
					if (!isValidId(target) || !exists(target))
					{
						
						sendSystemMessage(self, "You must target object", null);
					}
					sendSystemMessage(self, "Name: "+localize( getNameStringId(target)), null);
					
				}
				else if (priCommand.equalsIgnoreCase ("get_loco") && secCommand.equals(""))
				{
					sendSystemMessage(self, "Locomotion:"+getLocomotion(self), null);
				}
				else if (priCommand.equalsIgnoreCase ("squad") && !secCommand.equals(""))
				{
					
					sendSystemMessage(self, "Ship: "+secCommand, null);
					obj_id secCmd = utils.stringToObjId(secCommand);
					if (!isValidId(secCmd))
					{
						sendSystemMessage(self, "Ship OID not valid ", null);
					}
					
					sendSystemMessage(self, "Ship OID valid", null);
					sendSystemMessage(self, "Ship Squad: "+_spaceUnitGetSquadId(secCmd), null);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean setCitySkillTrainers(obj_id self, int city_id) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		if (!cityExists( city_id ))
		{
			return false;
		}
		
		location poiStartLocation = getLocation(self);
		float yaw = getYaw( self );
		String[] arrayOfAllTrianerTemplates = dataTableGetStringColumn(CITY_SKILL_TRAINERS, "TEMPLATE");
		String[] arrayOfAllTrianerStrings = dataTableGetStringColumn(CITY_SKILL_TRAINERS, "STRING");
		
		sendSystemMessage(self, "Number of Trainers Being Placed: "+ arrayOfAllTrianerTemplates.length, null);
		if (arrayOfAllTrianerTemplates.length != arrayOfAllTrianerStrings.length)
		{
			return false;
		}
		
		for (int i = 0; i < arrayOfAllTrianerTemplates.length; i++)
		{
			testAbortScript();
			location poiLocation = utils.getRandomAwayLocation(poiStartLocation, 5.f, 10.f);
			obj_id poi = createObject(arrayOfAllTrianerTemplates[i], poiLocation);
			
			if (isValidId(poi) && exists(poi))
			{
				setYaw( poi, yaw );
				setObjVar( poi, "creator", self );
				setName(poi, arrayOfAllTrianerStrings[i]);
				int flags = SF_COST_CITY_LOW | SF_SKILL_TRAINER;
				citySetStructureInfo( city_id, poi, flags, true );
				setObjVar( poi, "city_id", city_id );
			}
			sendSystemMessage(self, arrayOfAllTrianerStrings[i] + " placed.", null);
			
		}
		return true;
	}
	
	
	public boolean destroyCitySkillTrainers(obj_id self, int city_id) throws InterruptedException
	{
		obj_id[] structures = cityGetStructureIds( city_id );
		for (int i=0; i<structures.length; i++)
		{
			testAbortScript();
			if (structures[i].isLoaded())
			{
				if (!city.isNormalStructure( city_id, structures[i] ))
				{
					destroyObject(structures[i]);
				}
			}
		}
		return true;
	}
}
