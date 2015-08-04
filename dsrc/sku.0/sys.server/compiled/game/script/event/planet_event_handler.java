package script.event;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.holiday;
import script.library.utils;


public class planet_event_handler extends script.base_script
{
	public planet_event_handler()
	{
	}
	public static final String EVENT_TIMESTAMP = "lifeday.time_stamp";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		
		if (lifedayRunning != null && !lifedayRunning.equals("false"))
		{
			checkLifeDayData(self);
		}
		
		else if (hasObjVar(self, "lifeday"))
		{
			removeObjVar(self, "lifeday");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "planet_event_handler.OnInitialize: trigger initialized.");
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		
		if (lifedayRunning != null && !lifedayRunning.equals("false"))
		{
			checkLifeDayData(self);
		}
		
		else if (hasObjVar(self, "lifeday"))
		{
			removeObjVar(self, "lifeday");
		}
		
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		if (empiredayRunning != null && (empiredayRunning.equals("true") || empiredayRunning.equals("1")))
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.OnInitialize: EMPIRE DAY CONFIG FOUND.");
			dictionary params = new dictionary();
			params.put("eventVar", holiday.PLANET_VAR_EMPIRE_DAY);
			params.put("eventConfig", "empireday_ceremony");
			messageTo (self, "setUpEventLeaderBoard", params, 3.0f, false);
		}
		else
		{
			
			if (hasObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY))
			{
				CustomerServiceLog("holidayEvent", "planet_event_handler.OnInitialize: Var: "+holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY+" removed because the event was not running.");
				removeObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public void checkLifeDayData(obj_id self) throws InterruptedException
	{
		if (!hasObjVar(self, EVENT_TIMESTAMP))
		{
			newTimeStamp(self);
		}
		
		if (!hasObjVar(self, "lifeday.emptyScoreBoard"))
		{
			setObjVar(self, "lifeday.emptyScoreBoard", "true");
		}
		
		if (hasObjVar(self, "lifeday.positionOne.playerScore"))
		{
			if (!newDay(self))
			{
				setObjVar(self, "lifeday.emptyScoreBoard", "false");
			}
		}
		
		if (newDay(self))
		{
			removeObjVar(self, "lifeday");
			setObjVar(self, "lifeday.emptyScoreBoard", "true");
			newTimeStamp(self);
			createDailyAlarmClock(self, "lifeDayDailyAlarm", null, 10, 0, 0);
		}
	}
	
	
	public boolean newDay(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, EVENT_TIMESTAMP))
		{
			int now = getCalendarTime();
			int then = getIntObjVar(self, EVENT_TIMESTAMP);
			
			if (now > then)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	
	
	public void newTimeStamp(obj_id self) throws InterruptedException
	{
		int now = getCalendarTime();
		int secondsUntil = secondsUntilNextDailyTime(10, 0, 0);
		int then = now + secondsUntil;
		setObjVar(self, EVENT_TIMESTAMP, then);
		createDailyAlarmClock(self, "lifeDayDailyAlarm", null, 10, 0, 0);
	}
	
	
	public int setUpEventLeaderBoard(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "planet_event_handler.setUpEventLeaderBoard: Event Leader Board Data being initialized.");
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		String eventVar = params.getString("eventVar");
		if (eventVar == null || eventVar.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + eventVar))
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.OnInitialize: Event data found: "+eventVar+", removing data so we can start new.");
			removeObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + eventVar);
		}
		
		if (eventVar.equals(holiday.PLANET_VAR_EMPIRE_DAY))
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.setUpEventLeaderBoard: Setting up Empire Day data for the first time.");
			if (!hasObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE_TIMESTAMP))
			{
				CustomerServiceLog("holidayEvent", "planet_event_handler.setUpEventLeaderBoard: Setting up Empire Day score time stamp.");
				
				int now = getCalendarTime();
				int secondsUntil = holiday.getEmpireDayLockoutDelay();
				int then = now + secondsUntil;
				setObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE_TIMESTAMP, then);
				
				messageTo(self, "resetEventDataAfterDelay", params, secondsUntil, false);
				
				setupInitialEventScores(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY + holiday.PLANET_VAR_SCORE, 5, true);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int resetEventDataAfterDelay(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.resetEventDataAfterDelay: Could not find params for reset.");
			return SCRIPT_CONTINUE;
		}
		
		String eventVar = params.getString("eventVar");
		if (eventVar == null || eventVar.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.resetEventDataAfterDelay: Could not find eventVar params for reset.");
			return SCRIPT_CONTINUE;
		}
		
		String eventConfig = params.getString("eventConfig");
		if (eventConfig == null || eventConfig.length() <= 0)
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.resetEventDataAfterDelay: Could not find eventConfig params for reset.");
			return SCRIPT_CONTINUE;
		}
		
		String eventRunning = getConfigSetting("GameServer", eventConfig);
		if (eventRunning != null && !eventRunning.equals("false"))
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.resetEventDataAfterDelay: Message received with cargo. eventVar: "+eventVar+" with eventConfig: "+eventConfig+". The data is being reset because the delay time has expired as planned.");
			messageTo (self, "setUpEventLeaderBoard", params, 3.0f, false);
		}
		else
		{
			CustomerServiceLog("holidayEvent", "planet_event_handler.resetEventDataAfterDelay: Message received with cargo. eventVar: "+eventVar+" with eventConfig: "+eventConfig+". The data WAS NOT reset because the event was not running.");
			
			if (hasObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY))
			{
				removeObjVar(self, holiday.PLANET_VAR_EVENT_PREFIX + holiday.PLANET_VAR_EMPIRE_DAY);
			}
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int lifeDayDailyAlarm(obj_id self, dictionary params) throws InterruptedException
	{
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		
		if (lifedayRunning != null && !lifedayRunning.equals("false"))
		{
			checkLifeDayData(self);
		}
		
		else if (hasObjVar(self, "lifeday"))
		{
			removeObjVar(self, "lifeday");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int lifeDayScoreBoardUpdate(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = params.getObjId("player");
		checkLifeDayData(self);
		
		String scoreBoardEmpty = "true";
		int thirdPlaceScore = 0;
		int secondPlaceScore = 0;
		int firstPlaceScore = 0;
		
		if (hasObjVar(self, "lifeday.emptyScoreBoard"))
		{
			scoreBoardEmpty = getStringObjVar(self, "lifeday.emptyScoreBoard");
		}
		if (hasObjVar(self, "lifeday.positionThree.playerScore"))
		{
			thirdPlaceScore = getIntObjVar (self, "lifeday.positionThree.playerScore");
		}
		if (hasObjVar(self, "lifeday.positionTwo.playerScore"))
		{
			secondPlaceScore = getIntObjVar (self, "lifeday.positionTwo.playerScore");
		}
		if (hasObjVar(self, "lifeday.positionOne.playerScore"))
		{
			firstPlaceScore = getIntObjVar (self, "lifeday.positionOne.playerScore");
		}
		
		dictionary newParams = new dictionary();
		newParams.put("scoreBoardEmpty", scoreBoardEmpty);
		newParams.put("thirdPlaceScore", thirdPlaceScore);
		newParams.put("secondPlaceScore", secondPlaceScore);
		newParams.put("firstPlaceScore", firstPlaceScore);
		messageTo (player, "scoreBoardCheck", newParams, 3.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean setupInitialEventScores(obj_id planet, String scoreObjVar, int modifier, boolean faction) throws InterruptedException
	{
		if (!isValidId(planet) || !exists(planet))
		{
			return false;
		}
		if (scoreObjVar == null || scoreObjVar.length() <= 0)
		{
			return false;
		}
		if (modifier < 0)
		{
			return false;
		}
		
		if (!faction)
		{
			for (int i = 1, score = modifier; i <= holiday.MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				int randomPosition = rand(0,holiday.DEFAULT_HIGH_SCORE_LIST.length-1);
				String name = holiday.DEFAULT_HIGH_SCORE_LIST[randomPosition];
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_NAME, name);
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_SCORE, score);
				score --;
			}
		}
		else
		{
			for (int i = 1, score = modifier; i <= holiday.MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				int randomPosition = rand(0,holiday.DEFAULT_HIGH_SCORE_LIST.length-1);
				String name = holiday.DEFAULT_HIGH_SCORE_LIST[randomPosition];
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_NAME, name);
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_FACTION_REB + holiday.PLANET_VAR_PLAYER_SCORE, score);
				score --;
			}
			
			for (int i = 1, score = modifier; i <= holiday.MAX_NUMBER_OF_PLANET_HIGH_SCORES; i++)
			{
				testAbortScript();
				int randomPosition = rand(0,holiday.DEFAULT_HIGH_SCORE_LIST.length-1);
				String name = holiday.DEFAULT_HIGH_SCORE_LIST[randomPosition];
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_NAME, name);
				setObjVar(planet, scoreObjVar+".slot_"+i+holiday.PLANET_VAR_PLAYER_FACTION_IMP + holiday.PLANET_VAR_PLAYER_SCORE, score);
				score --;
			}
		}
		return true;
	}
	
}
