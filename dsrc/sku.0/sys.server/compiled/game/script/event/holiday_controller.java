package script.event;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.holiday;


public class holiday_controller extends script.base_script
{
	public holiday_controller()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday_controller.OnInitialize planet initialized, holiday controller called.");
		
		messageTo(self, "halloweenServerStart", null, 600.0f, false);
		
		messageTo(self, "lifedayServerStart", null, 610.0f, false);
		
		messageTo(self, "lovedayServerStart", null, 615.0f, false);
		
		CustomerServiceLog("holidayEvent", "holiday_controller.OnInitialize seconds to start empireday: "+holiday.EMPIRE_DAY_EVENT_START_DELAY);
		
		messageTo(self, "empiredayServerStart", null, holiday.EMPIRE_DAY_EVENT_START_DELAY, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "halloweenServerStart", null, 720.0f, false);
		
		messageTo(self, "lifedayServerStart", null, 730.0f, false);
		
		messageTo(self, "lovedayServerStart", null, 735.0f, false);
		
		messageTo(self, "empiredayServerStart", null, (holiday.EMPIRE_DAY_EVENT_START_DELAY + 100), false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		if (!isGod(player))
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		String halloweenRunning = getConfigSetting("GameServer", "halloween");
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		String lovedayRunning = getConfigSetting("GameServer", "loveday");
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		
		if (halloweenRunning != null && (halloweenRunning.equals("true") || halloweenRunning.equals("1")))
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Halloween Event Running = True";
			idx++;
		}
		else
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Halloween Event Running = False";
			idx++;
		}
		if (lifedayRunning != null && (lifedayRunning.equals("true") || lifedayRunning.equals("1")))
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Life Day Event Running = True";
			idx++;
		}
		else
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Life Day Event Running = False";
			idx++;
		}
		if (lifedayRunning != null && (lovedayRunning.equals("true") || lovedayRunning.equals("1")))
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Love Day Event Running = True";
			idx++;
		}
		else
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Love Day Event Running = False";
			idx++;
		}
		if (empiredayRunning != null && (empiredayRunning.equals("true") || empiredayRunning.equals("1")))
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Empire Day Event Running = True";
			idx++;
		}
		else
		{
			names[idx] = "holiday_event";
			attribs[idx] = "Empire Day Event Running = False";
			idx++;
		}
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!isGod(speaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		String halloweenString = getCurrentUniverseWideEvents();
		int halloween = halloweenString.indexOf("halloween");
		String halloweenRunning = getConfigSetting("GameServer", "halloween");
		if (text.equals("halloweenStart"))
		{
			startHolidayEvent(speaker, "halloween", halloweenRunning, halloween);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("halloweenStop"))
		{
			stopHolidayEvent(speaker, "halloween", halloweenRunning, halloween);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("halloweenStartForReals"))
		{
			startHolidayEventForReals(speaker, "halloween", halloweenRunning);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("halloweenStopForReals"))
		{
			stopHolidayEventForReals(speaker, "halloween");
			return SCRIPT_OVERRIDE;
		}
		
		String lifedayString = getCurrentUniverseWideEvents();
		int lifeday = lifedayString.indexOf("lifeday");
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		if (text.equals("lifedayStart"))
		{
			startHolidayEvent(speaker, "lifeday", lifedayRunning, lifeday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lifedayStop"))
		{
			stopHolidayEvent(speaker, "lifeday", lifedayRunning, lifeday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lifedayStartForReals"))
		{
			startHolidayEventForReals(speaker, "lifeday", lifedayRunning);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lifedayStopForReals"))
		{
			stopHolidayEventForReals(speaker, "lifeday");
			return SCRIPT_OVERRIDE;
		}
		
		String lovedayString = getCurrentUniverseWideEvents();
		int loveday = lifedayString.indexOf("loveday");
		String lovedayRunning = getConfigSetting("GameServer", "loveday");
		if (text.equals("lovedayStart"))
		{
			startHolidayEvent(speaker, "loveday", lovedayRunning, loveday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lovedayStop"))
		{
			stopHolidayEvent(speaker, "loveday", lovedayRunning, loveday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lovedayStartForReals"))
		{
			startHolidayEventForReals(speaker, "loveday", lovedayRunning);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("lovedayStopForReals"))
		{
			stopHolidayEventForReals(speaker, "loveday");
			return SCRIPT_OVERRIDE;
		}
		
		String empiredayString = getCurrentUniverseWideEvents();
		int empireday = empiredayString.indexOf("empireday_ceremony");
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		if (text.equals("empiredayStart"))
		{
			startHolidayEvent(speaker, "empireday_ceremony", empiredayRunning, empireday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("empiredayStop"))
		{
			stopHolidayEvent(speaker, "empireday_ceremony", empiredayRunning, empireday);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("empiredayStartForReals"))
		{
			startHolidayEventForReals(speaker, "empireday_ceremony", empiredayRunning);
			return SCRIPT_OVERRIDE;
		}
		
		if (text.equals("empiredayStopForReals"))
		{
			stopHolidayEventForReals(speaker, "empireday_ceremony");
			return SCRIPT_OVERRIDE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void startHolidayEvent(obj_id speaker, String holidayName, String holidayRunning, int holidayStatus) throws InterruptedException
	{
		if (holidayRunning == null)
		{
			sendSystemMessageTestingOnly(speaker, "Server config is not marked as "+holidayName+ " running");
			return;
		}
		
		if (holidayRunning.equals("true") || holidayRunning.equals("1"))
		{
			if (holidayStatus > -1)
			{
				sendSystemMessageTestingOnly(speaker, "Server says that "+holidayName+ " is already running. If you are sure that it's not, say "+holidayName+"StartForReals");
			}
			
			if (holidayStatus < 0)
			{
				sendSystemMessageTestingOnly(speaker, holidayName + " started.");
				startUniverseWideEvent(holidayName);
			}
			return;
		}
		
		return;
	}
	
	
	public void startHolidayEventForReals(obj_id speaker, String holidayName, String holidayRunning) throws InterruptedException
	{
		if (holidayRunning == null)
		{
			sendSystemMessageTestingOnly(speaker, "Server config is not marked as "+holidayName+" running");
			return;
		}
		
		if (holidayRunning.equals("true") || holidayRunning.equals("1"))
		{
			sendSystemMessageTestingOnly(speaker, holidayName+" started.");
			startUniverseWideEvent(holidayName);
			return;
		}
		return;
	}
	
	
	public void stopHolidayEvent(obj_id speaker, String holidayName, String holidayRunning, int holidayStatus) throws InterruptedException
	{
		if (holidayRunning.equals("true") || holidayRunning.equals("1"))
		{
			sendSystemMessageTestingOnly(speaker, "Server config is marked as "+holidayName+" running. If you are sure that it should not be running anyway, say "+holidayName+"StopForReals");
			return;
		}
		
		if (holidayRunning == null)
		{
			if (holidayStatus < 0)
			{
				sendSystemMessageTestingOnly(speaker, "Server says that "+holidayName+" is already stopped. If you are sure that it's not, say "+holidayName+"StartForReal");
			}
			
			if (holidayStatus > -1)
			{
				sendSystemMessageTestingOnly(speaker, holidayName+" stopped.");
				stopUniverseWideEvent(holidayName);
			}
			return;
		}
		
		return;
	}
	
	
	public void stopHolidayEventForReals(obj_id speaker, String holidayName) throws InterruptedException
	{
		sendSystemMessageTestingOnly(speaker, holidayName+" stopped.");
		stopUniverseWideEvent(holidayName);
		return;
	}
	
	
	public int halloweenServerStart(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday_controller.halloweenServerStart Halloween event handler called.");
		
		String halloweenString = getCurrentUniverseWideEvents();
		int halloween = halloweenString.indexOf("halloween");
		String halloweenRunning = getConfigSetting("GameServer", "halloween");
		
		CustomerServiceLog("holidayEvent", "holiday_controller.halloweenServerStart halloweenString: "+halloweenString+" halloween: "+halloween+" halloweenRunning: "+halloweenRunning);
		
		if (halloweenRunning == null)
		{
			CustomerServiceLog("holidayEvent", "holiday_controller.halloweenServerStart Halloween event is either not running or not in the server configs.");
			
			if (halloween > -1)
			{
				stopUniverseWideEvent("halloween");
			}
		}
		
		else if (halloweenRunning.equals("true") || halloweenRunning.equals("1"))
		{
			if (halloween < 0)
			{
				if (!startUniverseWideEvent("halloween"))
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.halloweenServerStart startUniverseWideEvent reports FAILURE to start universe wide event.");
				}
				else
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.halloweenServerStart startUniverseWideEvent reports SUCCESS starting universe wide event.");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int lifedayServerStart(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday_controller.lifedayServerStart Life Day event handler called.");
		
		String lifedayString = getCurrentUniverseWideEvents();
		int lifeday = lifedayString.indexOf("lifeday");
		String lifedayRunning = getConfigSetting("GameServer", "lifeday");
		CustomerServiceLog("holidayEvent", "holiday_controller.lifedayServerStart lifedayString: "+lifedayString+" lifeday: "+lifeday+" lifedayRunning: "+lifedayRunning);
		
		if (lifedayRunning == null)
		{
			CustomerServiceLog("holidayEvent", "holiday_controller.lifedayServerStart event is either not running or not in the server configs.");
			
			if (lifeday > -1)
			{
				stopUniverseWideEvent("lifeday");
			}
		}
		
		else if (lifedayRunning.equals("true") || lifedayRunning.equals("1"))
		{
			if (lifeday < 0)
			{
				if (!startUniverseWideEvent("lifeday"))
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.lifedayServerStart startUniverseWideEvent reports FAILURE to start universe wide event.");
				}
				else
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.lifedayServerStart startUniverseWideEvent reports SUCCESS starting universe wide event.");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int lovedayServerStart(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday_controller.lovedayServerStart Love Day event handler called.");
		
		String lovedayString = getCurrentUniverseWideEvents();
		int loveday = lovedayString.indexOf("loveday");
		String lovedayRunning = getConfigSetting("GameServer", "loveday");
		CustomerServiceLog("holidayEvent", "holiday_controller.lovedayServerStart lovedayString: "+lovedayString+" loveday: "+loveday+" lovedayRunning: "+lovedayRunning);
		
		if (lovedayRunning == null)
		{
			CustomerServiceLog("holidayEvent", "holiday_controller.lovedayServerStart event is either not running or not in the server configs.");
			
			if (loveday > -1)
			{
				stopUniverseWideEvent("loveday");
			}
		}
		
		else if (lovedayRunning.equals("true") || lovedayRunning.equals("1"))
		{
			if (loveday < 0)
			{
				if (!startUniverseWideEvent("loveday"))
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.lovedayServerStart startUniverseWideEvent reports FAILURE to start universe wide event.");
				}
				else
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.lovedayServerStart startUniverseWideEvent reports SUCCESS starting universe wide event.");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int empiredayServerStart(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart Empire Day event handler called.");
		String empiredayString = getCurrentUniverseWideEvents();
		int empireday = empiredayString.indexOf("empireday_ceremony");
		String empiredayRunning = getConfigSetting("GameServer", "empireday_ceremony");
		CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart empiredayString: "+empiredayString+" empireday: "+empireday+" empiredayRunning: "+empiredayRunning);
		
		if (empiredayRunning == null)
		{
			CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart event is either not running or not in the server configs.");
			if (empireday > -1)
			{
				stopUniverseWideEvent("empireday_ceremony");
			}
		}
		
		else if (empiredayRunning.equals("true") || empiredayRunning.equals("1"))
		{
			CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart event is starting.");
			
			if (empireday < 0)
			{
				if (!startUniverseWideEvent("empireday_ceremony"))
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart startUniverseWideEvent reports FAILURE to start universe wide event.");
				}
				else
				{
					CustomerServiceLog("holidayEvent", "holiday_controller.empiredayServerStart startUniverseWideEvent reports SUCCESS starting universe wide event.");
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
}
