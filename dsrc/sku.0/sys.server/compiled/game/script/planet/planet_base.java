package script.planet;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.cloninglib;
import script.library.gcw;
import script.library.regions;
import script.library.scheduled_drop;
import script.library.utils;


public class planet_base extends script.base_script
{
	public planet_base()
	{
	}
	
	public int OnUniverseComplete(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "planet_base.OnUniverseComplete: trigger initialized.");
		
		dictionary dctParams = new dictionary();
		messageTo(self, "doSpawnSetup", dctParams, 60, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int doSpawnSetup(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: messageHandler initialized.");
		
		String strPlanet = getNameForPlanetObject(self);
		
		if (strPlanet == null)
		{
			CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Planet name for this object is NULL. Notify SWG Development.");
			
			debugServerConsoleMsg(self, "Man you're so fucked. Your scenename is null. Go get justin!");
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Planet name for this object is: "+strPlanet);
		
		debugServerConsoleMsg(self, "************************************BASE PLANET INITIALIZATION***************************************");
		debugServerConsoleMsg(self, "onInitialize, my current scene is "+strPlanet);
		debugServerConsoleMsg(self, "************************************BASE PLANET INITIALIZATION***************************************");
		
		if (!strPlanet.equals("tutorial") && !hasScript(self, "systems.spawning.spawn_master"))
		{
			CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Non-tutorial Planet detected. Attaching systems.spawning.spawn_master");
			
			debugServerConsoleMsg(self, "************************************SPAWN ERROR****************************************************");
			debugServerConsoleMsg(self, "No spawnmaster script found! Attaching one!");
			debugServerConsoleMsg(self, "************************************SPAWN ERROR****************************************************");
			attachScript(self, "systems.spawning.spawn_master");
		}
		
		CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: doSpawnSetup complete.");
		obj_id tatooinePlanet = getPlanetByName("tatooine");
		if (isIdValid(tatooinePlanet) && exists(tatooinePlanet))
		{
			CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Tatooine Planet detected obj_id: "+tatooinePlanet);
			if (!hasScript(tatooinePlanet, "event.planet_event_handler"))
			{
				CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Tatooine Planet detected. Script Not Attached. Attaching event.planet_event_handler");
				attachScript (tatooinePlanet, "event.planet_event_handler");
			}
			scheduled_drop.instantiatePromotionsOnCluster();
		}
		else
		{
			CustomerServiceLog("holidayEvent", "planet_base.doSpawnSetup: Tatooine Planet not found!! Notify development.");
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDetach(obj_id self) throws InterruptedException
	{
		
		String strPlanet = getCurrentSceneName();
		if (strPlanet == null)
		{
			
			debugServerConsoleMsg(self, "Man you're so fucked. Your scenename is null. Go get justin!");
			return SCRIPT_CONTINUE;
		}
		
		String strRegionScript;
		
		strRegionScript = "systems.spawning.spawn_regions.";
		
		debugServerConsoleMsg(self, "region script name is "+strRegionScript+"regions_"+strPlanet);
		
		detachScript(self, strRegionScript+"regions_"+strPlanet);
		
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int registerCloningFacility(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id facilityId = params.getObjId("id");
		String cloneName = params.getString("name");
		String areaName = params.getString("buildout");
		obj_id areaId = params.getObjId("areaId");
		location facilityLoc = params.getLocation("loc");
		location facilityRespawn = params.getLocation("respawn");
		int cloneType = params.getInt("type");
		
		Vector idList = utils.getResizeableObjIdArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_ID);
		Vector nameList = utils.getResizeableStringArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_NAME);
		Vector areaList = utils.getResizeableStringArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA);
		Vector areaIdList = utils.getResizeableObjIdArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA_ID);
		Vector locList = utils.getResizeableLocationArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_LOC);
		Vector respawnList = utils.getResizeableLocationArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_RESPAWN);
		Vector cloneTypeList = utils.getResizeableIntArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_TYPE);
		
		if (idList == null || areaList == null || areaIdList == null || locList == null || respawnList == null || cloneTypeList == null)
		{
			idList = utils.addElement(null, facilityId);
			nameList = utils.addElement(null, cloneName);
			areaList = utils.addElement(null, areaName);
			areaIdList = utils.addElement(null, areaId);
			locList = utils.addElement(null, facilityLoc);
			respawnList = utils.addElement(null, facilityRespawn);
			cloneTypeList = utils.addElement(null, cloneType);
		}
		else
		{
			int pos = utils.getElementPositionInArray(idList, facilityId);
			if (pos >= 0)
			{
				nameList.set(pos, cloneName);
				areaList.set(pos, areaName);
				areaIdList.set(pos, areaId);
				locList.set(pos, facilityLoc);
				respawnList.set(pos, facilityRespawn);
				cloneTypeList.set(pos, new Integer(cloneType));
			}
			else
			{
				idList = utils.addElement(idList, facilityId);
				nameList = utils.addElement(nameList, cloneName);
				areaList = utils.addElement(areaList, areaName);
				areaIdList = utils.addElement(areaIdList, areaId);
				locList = utils.addElement(locList, facilityLoc);
				respawnList = utils.addElement(respawnList, facilityRespawn);
				cloneTypeList = utils.addElement(cloneTypeList, cloneType);
			}
		}
		
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_ID, idList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_NAME, nameList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA, areaList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA_ID, areaIdList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_LOC, locList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_RESPAWN, respawnList);
		utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_TYPE, cloneTypeList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int unregisterCloningFacility(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id facilityId = params.getObjId("id");
		
		Vector idList = utils.getResizeableObjIdArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_ID);
		Vector nameList = utils.getResizeableStringArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_NAME);
		Vector areaList = utils.getResizeableStringArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA);
		Vector areaIdList = utils.getResizeableObjIdArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA_ID);
		Vector locList = utils.getResizeableLocationArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_LOC);
		Vector respawnList = utils.getResizeableLocationArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_RESPAWN);
		Vector cloneTypeList = utils.getResizeableIntArrayScriptVar(self, cloninglib.VAR_PLANET_CLONE_TYPE);
		
		if (idList != null && nameList != null && areaList != null && areaIdList != null && locList != null && respawnList != null && cloneTypeList != null)
		{
			int pos = utils.getElementPositionInArray(idList, facilityId);
			if (pos >= 0)
			{
				idList = utils.removeElementAt(idList, pos);
				nameList = utils.removeElementAt(nameList, pos);
				areaList = utils.removeElementAt(areaList, pos);
				areaIdList = utils.removeElementAt(areaIdList, pos);
				locList = utils.removeElementAt(locList, pos);
				respawnList = utils.removeElementAt(respawnList, pos);
				cloneTypeList = utils.removeElementAt(cloneTypeList, pos);
				
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_ID, idList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_NAME, nameList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA, areaList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_AREA_ID, areaIdList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_LOC, locList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_RESPAWN, respawnList);
				utils.setScriptVar(self, cloninglib.VAR_PLANET_CLONE_TYPE, cloneTypeList);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwInvasionTracker(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("dologging", "planet_base gcwInvasionTracker");
		
		String city = params.getString("city");
		int lastPulse = utils.getIntScriptVar(self, "gcw.time."+ city);
		int gameTime = getGameTime();
		
		if (gameTime - lastPulse > gcw.GCW_PULSE_TIMER)
		{
			obj_id citySequencer = params.getObjId("sequencer");
			
			if (isIdValid(citySequencer))
			{
				LOG("dologging", "planet_base gcwInvasionTracker citySequencer: "+ citySequencer);
				
				utils.setScriptVar(self, "gcw.time."+ city, gameTime);
				utils.setScriptVar(self, "gcw.object."+ city, citySequencer);
				utils.setScriptVar(self, "gcw.calendar_time."+ city, getCalendarTime());
				messageTo(citySequencer, "beginInvasion", null, 1.0f, false);
			}
		}
		
		messageTo(self, "gcwInvasionTracker", params, 300.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int gcwGetInvasionObject(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary newParams = new dictionary();
		
		String cityName = params.getString("city");
		obj_id whoToMessage = params.getObjId("object");
		String message = params.getString("messageHandler");
		
		if (utils.hasScriptVar(self, "gcw.object." + cityName))
		{
			obj_id invasionObject = utils.getObjIdScriptVar(self, "gcw.object."+ cityName);
			
			newParams.put("invasionObject", invasionObject);
			
			messageTo(whoToMessage, message, newParams, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
}
