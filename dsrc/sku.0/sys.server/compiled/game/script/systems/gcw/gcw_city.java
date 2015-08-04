package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.ai_lib;
import script.library.colors_hex;
import script.library.factions;
import script.library.gcw;
import script.library.planetary_map;
import script.library.trial;
import script.library.utils;


public class gcw_city extends script.base_script
{
	public gcw_city()
	{
	}
	public static final location KEREN_ANNOUNCEMENT_ORIGIN = new location (1616, 24, 2705, "naboo", null);
	public static final location BESTINE_ANNOUNCEMENT_ORIGIN = new location (-1097, 12, -3583, "tatooine", null);
	public static final location DEARIC_ANNOUNCEMENT_ORIGIN = new location (435, 6, -2966, "talus", null);
	
	public static final float KEREN_ANNOUNCEMENT_RADIUS = 700.0f;
	public static final float BESTINE_ANNOUNCEMENT_RADIUS = 450.0f;
	public static final float DEARIC_ANNOUNCEMENT_RADIUS = 650.0f;
	
	public static final String COLOR_REBELS = "\\"+ colors_hex.COLOR_REBELS;
	public static final String COLOR_IMPERIALS = "\\"+ colors_hex.COLOR_IMPERIALS;
	
	public static final int PHASE_CONSTRUCTION = 0;
	public static final int PHASE_COMBAT = 1;
	public static final int PHASE_OCCUPATION = 2;
	
	public static final obj_id CITY_OBJECT_BESTINE = getObjIdWithNull(9835358);
	public static final obj_id CITY_OBJECT_DEARIC = getObjIdWithNull(9805353);
	public static final obj_id CITY_OBJECT_KEREN = getObjIdWithNull(9865353);
	
	
	public void qaInstabuild(obj_id[] kits) throws InterruptedException
	{
		for (int i = 0, j = kits.length; i < j; i++)
		{
			testAbortScript();
			if (!isIdValid(kits[i]) || !exists(kits[i]))
			{
				continue;
			}
			
			int construction = getIntObjVar(kits[i], "gcw.constructionQuestsCompleted");
			int newConstruction = gcw.GCW_CONSTRUCTION_MAXIMUM / 2;
			
			if (construction >= newConstruction)
			{
				newConstruction = gcw.GCW_CONSTRUCTION_MAXIMUM;
			}
			
			setObjVar(kits[i], "gcw.constructionQuestsCompleted", newConstruction);
			
			messageTo(kits[i], "handleUpdateName", null, 1.0f, false);
		}
	}
	
	
	public void qaInstaclear(obj_id[] kits) throws InterruptedException
	{
		for (int i = 0, j = kits.length; i < j; i++)
		{
			testAbortScript();
			if (!isIdValid(kits[i]) || !exists(kits[i]))
			{
				continue;
			}
			
			setObjVar(kits[i], "gcw.constructionQuestsCompleted", 0);
			
			messageTo(kits[i], "handleUpdateName", null, 1.0f, false);
		}
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id objSpeaker, String strText) throws InterruptedException
	{
		if (!isGod(objSpeaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary params = trial.getSessionDict(self);
		
		if (strText.equals("gcwcleanup") || strText.equals("gcwend"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwcleanup command. session: "+params.getInt(trial.MESSAGE_SESSION));
			obj_id planet = getPlanetByName("tatooine");
			if (!isValidId(planet))
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find planet Tatooine.");
				return SCRIPT_CONTINUE;
			}
			
			String city = gcw.getCityFromTable(self);
			if (city == null || city.length() <= 0)
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find city name.");
				return SCRIPT_CONTINUE;
			}
			
			utils.setScriptVar(planet, "gcw.calendar_time."+ city, getCalendarTime() - gcw.GCW_CONSTRUCTION_END_TIMER - gcw.GCW_COMBAT_END_TIMER);
			
			messageTo(self, "cleanupInvasion", params, 1.0f, false);
			
			debugSpeakMsg(self, "GCW Controller: Cleaning up invasion.");
		}
		
		else if (strText.equals("gcwdefensewin"))
		{
			if (!utils.hasScriptVar(self, "gcw.constructionEnded") && !utils.hasScriptVar(self, "gcw.invasionRunning"))
			{
				debugSpeakMsg(self, "Must be used when in construction or combat phase.");
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwcleanup command. session: "+params.getInt(trial.MESSAGE_SESSION));
			obj_id planet = getPlanetByName("tatooine");
			if (!isValidId(planet))
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find planet Tatooine.");
				return SCRIPT_CONTINUE;
			}
			
			String city = gcw.getCityFromTable(self);
			if (city == null || city.length() <= 0)
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find city name.");
				return SCRIPT_CONTINUE;
			}
			
			obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
			if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
			{
				return SCRIPT_CONTINUE;
			}
			
			Vector loserDefenseParticipants = new Vector();
			loserDefenseParticipants.setSize(0);
			Vector winnerOffenseParticipants = new Vector();
			winnerOffenseParticipants.setSize(0);
			int loserFactionFlag = factions.FACTION_FLAG_UNKNOWN;
			int winnerFactionFlag = factions.FACTION_FLAG_UNKNOWN;
			
			if (factions.isImperialorImperialHelper(defendingGeneral))
			{
				loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
				loserFactionFlag = factions.FACTION_FLAG_REBEL;
				
				winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
				winnerFactionFlag = factions.FACTION_FLAG_IMPERIAL;
			}
			else if (factions.isRebelorRebelHelper(defendingGeneral))
			{
				loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
				loserFactionFlag = factions.FACTION_FLAG_IMPERIAL;
				
				winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
				winnerFactionFlag = factions.FACTION_FLAG_REBEL;
			}
			
			if (loserDefenseParticipants != null && loserDefenseParticipants.size() > 0)
			{
				gcw.awardGcwInvasionParticipants(loserDefenseParticipants, loserFactionFlag, gcw.GCW_TOKENS_LOSER_PARTICIPANTS, gcw.GCW_POINTS_LOSER_PARTICIPANTS);
			}
			if (winnerOffenseParticipants != null && winnerOffenseParticipants.size() > 0)
			{
				gcw.awardGcwInvasionParticipants(winnerOffenseParticipants, winnerFactionFlag, gcw.GCW_TOKENS_WINNER_PARTICIPANTS, gcw.GCW_POINTS_WINNER_PARTICIPANTS);
			}
			
			utils.setScriptVar(planet, "gcw.calendar_time."+ city, getCalendarTime() - gcw.GCW_CONSTRUCTION_END_TIMER - gcw.GCW_COMBAT_END_TIMER);
			messageTo(self, "cleanupInvasion", params, 1.0f, false);
			
			debugSpeakMsg(self, "GCW Controller: Defense has won the invasion -- No announcements will be sent.");
		}
		
		else if (strText.equals("gcwbegin") || strText.equals("gcwstart"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwbegin command. session: "+params.getInt(trial.MESSAGE_SESSION));
			
			obj_id planet = getPlanetByName("tatooine");
			if (!isValidId(planet))
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find planet Tatooine.");
				return SCRIPT_CONTINUE;
			}
			
			String city = gcw.getCityFromTable(self);
			if (city == null || city.length() <= 0)
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find city name.");
				return SCRIPT_CONTINUE;
			}
			
			utils.setScriptVar(planet, "gcw.time."+ city, getGameTime());
			utils.setScriptVar(planet, "gcw.object."+ city, self);
			utils.setScriptVar(planet, "gcw.calendar_time."+ city, getCalendarTime());
			
			messageTo(self, "beginInvasion", null, 1.0f, false);
			
			debugSpeakMsg(self, "GCW Controller: Starting construction.");
		}
		
		else if (strText.equals("gcwendbuild") || strText.equals("gcwstartinvasion"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwendbuild command. session: "+params.getInt(trial.MESSAGE_SESSION));
			
			obj_id planet = getPlanetByName("tatooine");
			if (!isValidId(planet))
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find planet Tatooine.");
				return SCRIPT_CONTINUE;
			}
			
			String city = gcw.getCityFromTable(self);
			if (city == null || city.length() <= 0)
			{
				debugSpeakMsg(self, "GCW Controller: Failed to find city name.");
				return SCRIPT_CONTINUE;
			}
			
			utils.setScriptVar(planet, "gcw.calendar_time."+ city, getCalendarTime() - gcw.GCW_CONSTRUCTION_END_TIMER);
			messageTo(self, "endConstruction", params, 1.0f, false);
			
			debugSpeakMsg(self, "GCW Controller: Starting invasion.");
		}
		
		else if (strText.equals("gcwkits"))
		{
			obj_id[] npcs = trial.getObjectsInInstanceBySpawnId(self, "defense_kit");
			
			debugSpeakMsg(self, "count: "+ npcs.length);
		}
		
		else if (strText.equals("gcwquickbuild"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwquickbuild command. session: "+params.getInt(trial.MESSAGE_SESSION));
			
			obj_id[] kits = trial.getObjectsInInstanceBySpawnId(self, "defense_kit");
			qaInstabuild(kits);
			
			int kitsLength = kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "defense_patrol");
			qaInstabuild(kits);
			
			kitsLength += kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "offense_patrol");
			qaInstabuild(kits);
			
			kitsLength += kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "vehicle_patrol");
			qaInstabuild(kits);
			
			kitsLength += kits.length / 2;
			
			debugSpeakMsg(self, "GCW Controller: "+ kitsLength + " Construction kits modified.");
		}
		
		else if (strText.equals("gcwquickbuildclear"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.OnHearSpeech: God Player: "+objSpeaker+" has used the gcwquickbuildclear command. session: "+params.getInt(trial.MESSAGE_SESSION));
			
			obj_id[] kits = trial.getObjectsInInstanceBySpawnId(self, "defense_kit");
			qaInstaclear(kits);
			
			int kitsLength = kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "defense_patrol");
			qaInstaclear(kits);
			
			kitsLength += kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "offense_patrol");
			qaInstaclear(kits);
			
			kitsLength += kits.length / 2;
			
			kits = trial.getObjectsInInstanceBySpawnId(self, "vehicle_patrol");
			qaInstaclear(kits);
			
			kitsLength += kits.length / 2;
			
			debugSpeakMsg(self, "GCW Controller: "+ kitsLength + " Construction kits modified to zero quests completed.");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("gcw_city_invasion", "gcw_city.OnInitialize: The city sequencer object is starting for the fist time.");
		messageTo(self, "checkForInvasion", null, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkForInvasion(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id planet = getPlanetByName("tatooine");
		
		String cityName = gcw.getCityFromTable(self);
		
		if (cityName != null && cityName.length() > 0 && isIdValid(planet))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.checkForInvasion: The city sequencer object is being checked for a running invasion. session: "+params.getInt(trial.MESSAGE_SESSION));
			params = new dictionary();
			params.put("sequencer", self);
			params.put("city", cityName);
			messageTo(planet, "gcwInvasionTracker", params, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int beginInvasion(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "beginInvasion init");
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("dologging", "gcw_city beginInvasion");
		
		obj_id planet = getPlanetByName("tatooine");
		
		if (!isIdValid(planet))
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = gcw.getCityFromTable(self);
		
		if (utils.hasScriptVar(self, "gcw.invasionRunning"))
		{
			CustomerServiceLog("gcw_city_invasion", "gcw_city.beginInvasion: beginInvasion has been called but according to the city object AN INVASION IS CURRENTLY RUNNING. session: "+params.getInt(trial.MESSAGE_SESSION));
			
			if (cityName == null || cityName.length() < 1)
			{
				return SCRIPT_CONTINUE;
			}
			
			int lastPulse = utils.getIntScriptVar(self, "gcw.time."+ cityName);
			int gameTime = getGameTime();
			
			if (gameTime - lastPulse > gcw.GCW_PULSE_TIMER)
			{
				CustomerServiceLog("gcw_city_invasion", "gcw_city.beginInvasion: The city sequencer object had a gcw.invasionRunning var on it. The invasionRunning and constructionEnded vars are being removed because the system thinks we have an invasion that failed to end properly. Removing vars at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
				messageTo(self, "cleanupInvasion", params, 1.0f, false);
				utils.removeScriptVar(self, "gcw.invasionRunning");
				utils.removeScriptVar(planet, "gcw.invasionRunning."+ cityName);
				utils.removeScriptVar(self, "gcw.constructionEnded");
			}
			
			return SCRIPT_CONTINUE;
		}
		
		CustomerServiceLog("gcw_city_invasion", "gcw_city.beginInvasion: beginInvasion has been called. we are starting the construction phase at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		messageTo(self, "beginSpawn", null, 1.0f, false);
		messageTo(self, "startFactionDefense", null, 5.0f, false);
		
		utils.setScriptVar(self, "gcw.invasionRunning", 1);
		
		gcw_score.setGcwPhase(gcw.GCW_CITY_PHASE_CONSTRUCTION);
		
		utils.setScriptVar(planet, "gcw.invasionRunning."+ cityName, 1);
		
		LOG("gcw_announcement", "beginInvasion getting defending general");
		return SCRIPT_CONTINUE;
	}
	
	
	public int beginInvasionNotifyFactions(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "beginInvasionNotifyFactions init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneralList[] = trial.getObjectsInInstanceBySpawnId(self, "defense_camp");
		if (defendingGeneralList == null || defendingGeneralList.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = defendingGeneralList[0];
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, "defendingGeneral", defendingGeneral);
		utils.setScriptVar(self, "currentOccupyFaction", getIntObjVar(defendingGeneral, "factionFlag"));
		
		if ((!utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			
			String planetName = getCurrentSceneName();
			if (planetName == null || planetName.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			String cityName = gcw.getCityFromTable(self);
			if (cityName == null || cityName.length() <= 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (cityName.equals(gcw.CITY_DEARIC))
			{
				utils.setScriptVar(self, "announcementOrigin", DEARIC_ANNOUNCEMENT_ORIGIN);
				utils.setScriptVar(self, "announcementRadius", DEARIC_ANNOUNCEMENT_RADIUS);
				utils.setScriptVar(self, "cityObject", CITY_OBJECT_DEARIC);
			}
			else if (cityName.equals(gcw.CITY_BESTINE))
			{
				utils.setScriptVar(self, "announcementOrigin", BESTINE_ANNOUNCEMENT_ORIGIN);
				utils.setScriptVar(self, "announcementRadius", BESTINE_ANNOUNCEMENT_RADIUS);
				utils.setScriptVar(self, "cityObject", CITY_OBJECT_BESTINE);
				
			}
			else if (cityName.equals(gcw.CITY_KEREN))
			{
				utils.setScriptVar(self, "announcementOrigin", KEREN_ANNOUNCEMENT_ORIGIN);
				utils.setScriptVar(self, "announcementRadius", KEREN_ANNOUNCEMENT_RADIUS);
				utils.setScriptVar(self, "cityObject", CITY_OBJECT_KEREN);
			}
			
			utils.setScriptVar(self, "planetName", planetName);
			utils.setScriptVar(self, "cityName", cityName);
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_announcement", "beginInvasionNotifyFactions planetName: "+planetName+" cityName: "+cityName);
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "beginInvasionNotifyFactions defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_construct_defenses_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_call_to_arms_rebel_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "beginInvasionNotifyFactions defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_call_to_arms_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_construct_defenses_rebel_");
		}
		webster.put("neutral_announcement", "gcw_announcement_construct_defenses_neutral_");
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		CustomerServiceLog("gcw_city_invasion", "gcw_city.beginInvasionNotifyFactions: beginInvasionNotifyFactions has sent all players messages regarding the construction phase starting at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		utils.setScriptVar(self, "phase", PHASE_CONSTRUCTION);
		messageTo(self, "updateGcwMapData", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int constructionEndsSoon(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "constructionEndsSoon init");
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "constructionEndsSoon defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_building_almost_complete_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_rally_for_invasion_rebel_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "constructionEndsSoon defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_rally_for_invasion_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_building_almost_complete_rebel_");
		}
		webster.put("neutral_announcement", "gcw_announcement_rally_for_invasion_neutral_");
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		CustomerServiceLog("gcw_city_invasion", "gcw_city.constructionEndsSoon: constructionEndsSoon has sent all players messages regarding the construction phase ending soon. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		return SCRIPT_CONTINUE;
	}
	
	
	public int invasionEndsSoon(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "invasionEndsSoon init");
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "invasionEndsSoon defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_invasion_almost_defeated_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_invasion_almost_lost_rebel_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "invasionEndsSoon defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_invasion_almost_lost_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_invasion_almost_defeated_rebel_");
		}
		webster.put("neutral_announcement", "gcw_announcement_invasion_almost_defeated_neutral_");
		CustomerServiceLog("gcw_city_invasion", "gcw_city.invasionEndsSoon: invasionEndsSoon has sent all players messages regarding the combat phase ending soon. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int defenseGeneralUnderSustainedAttack(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "defenseGeneralUnderSustainedAttack init");
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "defenseGeneralUnderSustainedAttack defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_general_under_attack_imperial_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "defenseGeneralUnderSustainedAttack defending general is Rebel");
			webster.put("rebel_announcement", "gcw_announcement_general_under_attack_rebel_");
		}
		webster.put("neutral_announcement", "gcw_announcement_general_under_attack_neutral_");
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		CustomerServiceLog("gcw_city_invasion", "gcw_city.defenseGeneralUnderSustainedAttack: defenseGeneralUnderSustainedAttack has sent all players messages regarding the generals status. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int startFactionDefense(obj_id self, dictionary params) throws InterruptedException
	{
		
		gcw_score.clearGcwData();
		gcw_score.setGcwPhase(gcw.GCW_CITY_PHASE_CONSTRUCTION);
		
		int percentage = gcw.getRebelPercentileByRegion(self);
		
		obj_id planet = getPlanetByName("tatooine");
		
		String cityName = gcw.getCityFromTable(self);
		
		if (percentage > 50)
		{
			LOG("dologging", "gcw_city beginInvasion rebel_defense > 50");
			sendSequenceTrigger(self, "rebel_defense");
			CustomerServiceLog("gcw_city_invasion", "gcw_city.startFactionDefense: startFactionDefense is setting the city object "+self+" to be rebel_defense at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
			utils.setScriptVar(planet, "gcw.factionDefending."+ cityName, factions.FACTION_FLAG_REBEL);
		}
		
		else if (percentage < 50)
		{
			LOG("dologging", "gcw_city beginInvasion imperial_defense < 50");
			sendSequenceTrigger(self, "imperial_defense");
			CustomerServiceLog("gcw_city_invasion", "gcw_city.startFactionDefense: startFactionDefense is setting the city object "+self+" to be imperial_defense at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
			utils.setScriptVar(planet, "gcw.factionDefending."+ cityName, factions.FACTION_FLAG_IMPERIAL);
		}
		else if (percentage == 50)
		{
			LOG("dologging", "gcw_city beginInvasion rebel_defense == 50");
			sendSequenceTrigger(self, "rebel_defense");
			CustomerServiceLog("gcw_city_invasion", "gcw_city.startFactionDefense: startFactionDefense is setting the city object "+self+" to be rebel_defense at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
			utils.setScriptVar(planet, "gcw.factionDefending."+ cityName, factions.FACTION_FLAG_REBEL);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int endConstruction(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw", "endConstruction");
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "gcw.constructionEnded"))
		{
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(self, "gcw.constructionEnded", 1);
		gcw_score.setGcwPhase(gcw.GCW_CITY_PHASE_COMBAT);
		
		LOG("gcw", "endConstruction 2");
		
		sendSequenceTrigger(self, "invasion");
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "endConstruction defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_man_defenses_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_attack_city_rebel_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "endConstruction defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_attack_city_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_man_defenses_rebel_");
		}
		
		Vector rebelParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
		Vector imperialParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
		
		gcw.awardGcwInvasionParticipants(rebelParticipants, factions.FACTION_FLAG_REBEL, gcw.GCW_TOKENS_CONSTRUCTION_PHASE, gcw.GCW_POINTS_CONSTRUCTION_PHASE);
		gcw.awardGcwInvasionParticipants(imperialParticipants, factions.FACTION_FLAG_IMPERIAL, gcw.GCW_TOKENS_CONSTRUCTION_PHASE, gcw.GCW_POINTS_CONSTRUCTION_PHASE);
		
		webster.put("neutral_announcement", "gcw_announcement_man_defenses_neutral_");
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		CustomerServiceLog("gcw_city_invasion", "gcw_city.endConstruction: endConstruction has sent all players messages regarding the construction phase has ended. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		utils.setScriptVar(self, "phase", PHASE_COMBAT);
		messageTo(self, "updateGcwMapData", params, 0.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public void sendSequenceTrigger(obj_id self, String trig) throws InterruptedException
	{
		dictionary dict = trial.getSessionDict(self);
		
		LOG("dologging", "sendSequenceTrigger dict.getInt(MESSAGE_SESSION): "+ dict.getInt(trial.MESSAGE_SESSION));
		
		dict.put("triggerType", "triggerId");
		dict.put("triggerName", trig);
		messageTo(self, "triggerFired", dict, 2.0f, false);
	}
	
	
	public int invasionHalfWayPoint(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "invasionHalfWayPoint init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "invasionHalfWayPoint defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_defense_half_done_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_attack_half_done_rebel_");
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "invasionHalfWayPoint defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_attack_half_done_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_defense_half_done_rebel_");
		}
		webster.put("neutral_announcement", "gcw_announcement_defense_half_done_neutral_");
		CustomerServiceLog("gcw_city_invasion", "gcw_city.invasionHalfWayPoint: invasionHalfWayPoint has sent all players messages regarding the combat phase being half over Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int defendersEvacuate(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "defendersEvacuate init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		Vector loserDefenseParticipants = new Vector();
		loserDefenseParticipants.setSize(0);
		Vector winnerOffenseParticipants = new Vector();
		winnerOffenseParticipants.setSize(0);
		int loserFactionFlag = factions.FACTION_FLAG_UNKNOWN;
		int winnerFactionFlag = factions.FACTION_FLAG_UNKNOWN;
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "defendersEvacuate defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_city_lost_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_city_won_rebel_");
			
			loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
			loserFactionFlag = factions.FACTION_FLAG_IMPERIAL;
			
			winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
			winnerFactionFlag = factions.FACTION_FLAG_REBEL;
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "defendersEvacuate defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_city_won_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_city_lost_rebel_");
			
			loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
			loserFactionFlag = factions.FACTION_FLAG_REBEL;
			
			winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
			winnerFactionFlag = factions.FACTION_FLAG_IMPERIAL;
		}
		
		gcw_score.setGcwWinner(winnerFactionFlag);
		
		webster.put("neutral_announcement", "gcw_announcement_city_lost_neutral_");
		CustomerServiceLog("gcw_city_invasion", "gcw_city.defendersEvacuate: defendersEvacuate has sent all players messages regarding the defense losing the battle. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		
		if (loserDefenseParticipants != null && loserDefenseParticipants.size() > 0)
		{
			gcw.awardGcwInvasionParticipants(loserDefenseParticipants, loserFactionFlag, gcw.GCW_TOKENS_LOSER_PARTICIPANTS, gcw.GCW_POINTS_LOSER_PARTICIPANTS);
		}
		if (winnerOffenseParticipants != null && winnerOffenseParticipants.size() > 0)
		{
			gcw.awardGcwInvasionParticipants(winnerOffenseParticipants, winnerFactionFlag, gcw.GCW_TOKENS_WINNER_PARTICIPANTS, gcw.GCW_POINTS_WINNER_PARTICIPANTS);
		}
		
		messageTo(self, "cleanupInvasion", params, 1.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cityAttackUnsuccessful(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_announcement", "cityAttackUnsuccessful init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!utils.hasScriptVar(self, "defendingGeneral") || !utils.hasScriptVar(self, "planetName") || !utils.hasScriptVar(self, "cityName")))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id defendingGeneral = utils.getObjIdScriptVar(self, "defendingGeneral");
		if (!isValidId(defendingGeneral) || !exists(defendingGeneral))
		{
			return SCRIPT_CONTINUE;
		}
		
		String planetName = utils.getStringScriptVar(self, "planetName");
		if (planetName == null || planetName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		float announcementRadius = utils.getFloatScriptVar(self, "announcementRadius");
		if (announcementRadius <= -1)
		{
			return SCRIPT_CONTINUE;
		}
		
		dictionary webster = new dictionary();
		webster.put("string_file", "gcw");
		webster.put("planetName", planetName);
		webster.put("cityName", cityName);
		
		Vector loserDefenseParticipants = new Vector();
		loserDefenseParticipants.setSize(0);
		Vector winnerOffenseParticipants = new Vector();
		winnerOffenseParticipants.setSize(0);
		int loserFactionFlag = factions.FACTION_FLAG_UNKNOWN;
		int winnerFactionFlag = factions.FACTION_FLAG_UNKNOWN;
		
		if (factions.isImperialorImperialHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "cityAttackUnsuccessful defending general is Imperial");
			webster.put("imperial_announcement", "gcw_announcement_city_safe_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_attack_unsuccessful_rebel_");
			
			loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
			loserFactionFlag = factions.FACTION_FLAG_REBEL;
			
			winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
			winnerFactionFlag = factions.FACTION_FLAG_IMPERIAL;
		}
		else if (factions.isRebelorRebelHelper(defendingGeneral))
		{
			LOG("gcw_announcement", "cityAttackUnsuccessful defending general is Rebel");
			webster.put("imperial_announcement", "gcw_announcement_attack_unsuccessful_imperial_");
			webster.put("rebel_announcement", "gcw_announcement_city_safe_rebel_");
			
			loserDefenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_IMPERIAL);
			loserFactionFlag = factions.FACTION_FLAG_IMPERIAL;
			
			winnerOffenseParticipants = trial.getNonInstanceFactionParticipants(self, factions.FACTION_FLAG_REBEL);
			winnerFactionFlag = factions.FACTION_FLAG_REBEL;
			
		}
		
		gcw_score.setGcwWinner(winnerFactionFlag);
		
		webster.put("neutral_announcement", "gcw_announcement_city_safe_neutral_");
		CustomerServiceLog("gcw_city_invasion", "gcw_city.cityAttackUnsuccessful: cityAttackUnsuccessful has sent all players messages regarding the offense losing the battle. Message sent at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		messageToPlayersOnPlanet("notifyPlayerOfGcwCityEventAnnouncement", webster, 0.0f, announcementOrigin, announcementRadius, false);
		if (loserDefenseParticipants != null && loserDefenseParticipants.size() > 0)
		{
			gcw.awardGcwInvasionParticipants(loserDefenseParticipants, loserFactionFlag, gcw.GCW_TOKENS_LOSER_PARTICIPANTS, gcw.GCW_POINTS_LOSER_PARTICIPANTS);
		}
		if (winnerOffenseParticipants != null && winnerOffenseParticipants.size() > 0)
		{
			gcw.awardGcwInvasionParticipants(winnerOffenseParticipants, winnerFactionFlag, gcw.GCW_TOKENS_WINNER_PARTICIPANTS, gcw.GCW_POINTS_WINNER_PARTICIPANTS);
		}
		
		messageTo(self, "cleanupInvasion", params, 1.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanupInvasion(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!trial.verifySession(self, params))
		{
			return SCRIPT_CONTINUE;
		}
		
		trial.clearNonInstanceFactionParticipants(self);
		
		utils.setScriptVar(self, "phase", PHASE_OCCUPATION);
		messageTo(self, "updateGcwMapData", params, 0.0f, false);
		
		utils.removeScriptVar(self, "gcw.constructionEnded");
		utils.removeScriptVar(self, "gcw.invasionRunning");
		gcw_score.setGcwPhase(gcw.GCW_CITY_PHASE_UNKNOWN);
		
		obj_id planet = getPlanetByName("tatooine");
		String cityName = gcw.getCityFromTable(self);
		
		if (isIdValid(planet))
		{
			utils.removeScriptVar(planet, "gcw.invasionRunning."+ cityName);
			utils.removeScriptVar(planet, "gcw.factionDefending."+ cityName);
		}
		
		messageTo(self, "cleanupSpawn", null, 1.0f, false);
		CustomerServiceLog("gcw_city_invasion", "gcw_city.cleanupInvasion: cleanupInvasion is cleaning up all the assets for the battle. This happened at: "+getGameTime()+" session: "+params.getInt(trial.MESSAGE_SESSION));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int updateGcwMapData(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("gcw_map_data", "updateGcwMapData init");
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(self, "phase"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int phase = utils.getIntScriptVar(self, "phase");
		if (phase < PHASE_CONSTRUCTION)
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("gcw_map_data", "updateGcwMapData phase: "+phase);
		
		if (!trial.verifySession(self, params) && phase != PHASE_OCCUPATION)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		location announcementOrigin = utils.getLocationScriptVar(self, "announcementOrigin");
		if (announcementOrigin == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityName = utils.getStringScriptVar(self, "cityName");
		if (cityName == null || cityName.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String cityFirstLetter = cityName.substring(0,1);
		String remainder = cityName.substring(1);
		String cityNameString = cityFirstLetter.toUpperCase() + remainder.toLowerCase();
		
		obj_id cityObject = utils.getObjIdScriptVar(self, "cityObject");
		if (!isValidId(cityObject) || !exists(cityObject))
		{
			return SCRIPT_CONTINUE;
		}
		
		String color = "";
		String defenders = "";
		String invaders = "";
		String stringName = "";
		
		int faction = utils.getIntScriptVar(self, "currentOccupyFaction");
		
		LOG("gcw_map_data", "updateGcwMapData faction: "+faction);
		
		if (faction != factions.FACTION_FLAG_REBEL && faction != factions.FACTION_FLAG_IMPERIAL)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (faction == factions.FACTION_FLAG_REBEL)
		{
			LOG("gcw_map_data", "updateGcwMapData faction is REB");
			
			color = COLOR_REBELS;
			defenders = "Rebel";
			invaders = "Imperial";
		}
		else if (faction == factions.FACTION_FLAG_IMPERIAL)
		{
			LOG("gcw_map_data", "updateGcwMapData faction is IMP");
			
			color = COLOR_IMPERIALS;
			defenders = "Imperial";
			invaders = "Rebel";
		}
		
		obj_id planet = getPlanetByName("tatooine");
		
		int currentTime = getCalendarTime();
		LOG("gcw_map_data", "updateGcwMapData currentTime format: "+getCalendarTimeStringLocal(currentTime));
		int lastPulse = utils.getIntScriptVar(planet, "gcw.calendar_time."+ cityName);
		LOG("gcw_map_data", "updateGcwMapData lastPulse format: "+getCalendarTimeStringLocal(lastPulse));
		
		if (phase == PHASE_CONSTRUCTION)
		{
			int nextPulse = (lastPulse + gcw.GCW_CONSTRUCTION_END_TIMER);
			LOG("gcw_map_data", "updateGcwMapData nextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			String theNextPulse = utils.padTimeHM(nextPulse - currentTime);
			LOG("gcw_map_data", "updateGcwMapData theNextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			if (theNextPulse != null && theNextPulse.length() > 0)
			{
				stringName = color + cityNameString + "\n"+ defenders + "s Building Defenses\n"+ theNextPulse;;
			}
			else
			{
				stringName = color + cityNameString + "\n"+ defenders + "s Building Defenses";
			}
			
		}
		else if (phase == PHASE_COMBAT)
		{
			int nextPulse = (lastPulse + gcw.GCW_CONSTRUCTION_END_TIMER + gcw.GCW_COMBAT_END_TIMER);
			LOG("gcw_map_data_err", "updateGcwMapData nextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			String theNextPulse = utils.padTimeHM(nextPulse - currentTime);
			LOG("gcw_map_data_err", "updateGcwMapData theNextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			if (theNextPulse != null && theNextPulse.length() > 0)
			{
				stringName = color + cityNameString + "\n"+ invaders + "s Invading\n"+ theNextPulse;;
			}
			else
			{
				stringName = color + cityNameString + "\n"+ invaders + "s Invading ";
			}
		}
		else if (phase == PHASE_OCCUPATION)
		{
			int nextPulse = (lastPulse + gcw.GCW_PULSE_TIMER);
			LOG("gcw_map_data", "updateGcwMapData nextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			String theNextPulse = utils.padTimeHM(nextPulse - currentTime);
			LOG("gcw_map_data", "updateGcwMapData theNextPulse format: "+getCalendarTimeStringLocal(nextPulse));
			
			if (theNextPulse != null && theNextPulse.length() > 0)
			{
				stringName = color + cityNameString + "\n"+ defenders + " Occupation\n"+ theNextPulse;
			}
			else
			{
				stringName = color + cityNameString + "\n"+ defenders + " Occupation";
			}
		}
		
		LOG("gcw_map_data", "updateGcwMapData stringName: "+stringName);
		
		removePlanetaryMapLocation(cityObject);
		if (!addPlanetaryMapLocation(cityObject, stringName, (int)announcementOrigin.x, (int)announcementOrigin.z - 10, planetary_map.CITY, null, MLT_STATIC, planetary_map.NO_FLAG))
		{
			LOG("gcw_map_data", "updateGcwMapData addPlanetaryMapLocation FAILED to update planet map");
		}
		else
		{
			LOG("gcw_map_data", "updateGcwMapData addPlanetaryMapLocation successfully updated planet map");
		}
		
		dictionary dict = trial.getSessionDict(cityObject);
		messageTo(self, "updateGcwMapData", params, 60.0f, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean verifyCurrentOccupyFaction(obj_id cityObject) throws InterruptedException
	{
		LOG("gcw_map_data", "switchCurrentOccupyFaction init");
		if (!isValidId(cityObject) || !exists(cityObject))
		{
			return false;
		}
		if (!utils.hasScriptVar(cityObject, "currentOccupyFaction"))
		{
			return false;
		}
		
		int faction = utils.getIntScriptVar(cityObject, "currentOccupyFaction");
		LOG("gcw_map_data", "switchCurrentOccupyFaction faction: "+faction);
		if (faction != factions.FACTION_FLAG_REBEL && faction != factions.FACTION_FLAG_IMPERIAL)
		{
			return false;
		}
		
		int percentage = gcw.getRebelPercentileByRegion(cityObject);
		
		if (percentage > 50 && faction == factions.FACTION_FLAG_IMPERIAL)
		{
			LOG("gcw_map_data", "switchCurrentOccupyFaction switching to REB faction");
			utils.setScriptVar(cityObject, "currentOccupyFaction", factions.FACTION_FLAG_REBEL);
		}
		
		else if (percentage < 50 && faction == factions.FACTION_FLAG_REBEL)
		{
			LOG("gcw_map_data", "switchCurrentOccupyFaction switching to IMP faction");
			utils.setScriptVar(cityObject, "currentOccupyFaction", factions.FACTION_FLAG_IMPERIAL);
		}
		else if (percentage == 50 && faction == factions.FACTION_FLAG_IMPERIAL)
		{
			LOG("gcw_map_data", "switchCurrentOccupyFaction switching to REB faction");
			utils.setScriptVar(cityObject, "currentOccupyFaction", factions.FACTION_FLAG_REBEL);
		}
		
		dictionary dict = trial.getSessionDict(cityObject);
		messageTo(cityObject, "updateGcwMapData", dict, 0.0f, false);
		
		return true;
	}
}
