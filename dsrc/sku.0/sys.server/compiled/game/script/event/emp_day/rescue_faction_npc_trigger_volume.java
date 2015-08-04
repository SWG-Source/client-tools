package script.event.emp_day;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.chat;
import script.library.create;
import script.library.factions;
import script.library.groundquests;
import script.library.holiday;
import script.library.prose;
import script.library.stealth;
import script.library.utils;


public class rescue_faction_npc_trigger_volume extends script.base_script
{
	public rescue_faction_npc_trigger_volume()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String MEATLUMP_LOG = "empire_day_trigger";
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		createTriggerVolume(holiday.VOL_TRIGGER_NAME_PREFIX + self, holiday.VOL_TRIGGER_RADIUS, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		blog("OnTriggerVolumeEntered INIT: "+whoTriggeredMe);
		
		if (!isPlayer(whoTriggeredMe))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("Spawner: "+getIntObjVar(self, "spawner"));
		
		if (!hasObjVar(self, "spawn_creature"))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("spawn_creature received!");
		
		if (!hasObjVar(self, "active_quest"))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("active_quest received!");
		
		if (!hasObjVar(self, "active_task"))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("active_task received!");
		
		if (!hasObjVar(self, "signal_to_send"))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("signal_to_send received!");
		
		String creatureSpawn = getStringObjVar(self, "spawn_creature");
		if (creatureSpawn == null || creatureSpawn.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String activeQuest = getStringObjVar(self, "active_quest");
		if (activeQuest == null || activeQuest.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String activeTask = getStringObjVar(self, "active_task");
		if (activeTask == null || activeTask.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String sendSignal = getStringObjVar(self, "signal_to_send");
		if (sendSignal == null || sendSignal.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isQuestActive(whoTriggeredMe, activeQuest))
		{
			return SCRIPT_CONTINUE;
		}
		blog("Ground Quest Found!");
		
		if (!groundquests.isTaskActive(whoTriggeredMe, activeQuest, activeTask))
		{
			return SCRIPT_CONTINUE;
		}
		blog("Ground Task Found!");
		
		if (factions.isOnLeave(whoTriggeredMe) && (factions.isRebel(whoTriggeredMe) || factions.isImperial(whoTriggeredMe)))
		{
			if (!utils.hasScriptVar(whoTriggeredMe, "commPlayerOnLeave"))
			{
				String npc = "object/mobile/mara_jade_empire_day.iff";
				String sound = "sound/sys_comm_rebel_female.snd";
				
				if (factions.isRebel(whoTriggeredMe))
				{
					npc = "object/mobile/wedge_antilles_remembrance_day.iff";
					sound = "sound/sys_comm_rebel_male.snd";
				}
				prose_package pp = new prose_package();
				prose.setStringId(pp, new string_id("event/empire_day", "rescue_onleave_notgood"));
				commPlayers(whoTriggeredMe, npc, sound, 10f, whoTriggeredMe, pp);
				
				utils.setScriptVar(whoTriggeredMe, "commPlayerOnLeave", true);
			}
			return SCRIPT_CONTINUE;
		}
		
		String invis = stealth.getInvisBuff(whoTriggeredMe);
		if (invis != null)
		{
			stealth.checkForAndMakeVisibleNoRecourse(self);
		}
		
		blog("Sending Signal!");
		
		groundquests.sendSignal(whoTriggeredMe, sendSignal);
		
		blog("Signal Sent!");
		location curLoc = getLocation(self);
		
		blog("Creating Mob");
		obj_id mob = create.object(creatureSpawn, curLoc, 100);
		if (!isValidId(mob) || !exists(mob))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("Mob created Sent!");
		
		setObjVar(mob, "rescuerQuest", activeQuest);
		setObjVar(mob, "rescuer", whoTriggeredMe);
		blog("I set: "+whoTriggeredMe+" rescuer. Getting player location");
		
		location playerLoc = getLocation(whoTriggeredMe);
		if (playerLoc != null)
		{
			blog("player location found");
			pathTo(mob, playerLoc);
		}
		
		blog("saying something");
		
		prose_package pp = prose.getPackage(new string_id("event/empire_day", "rescue_me_breacher"), whoTriggeredMe);
		chat.publicChat(mob, whoTriggeredMe, null, null, pp);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		blog("Exited trigger volume: "+whoTriggeredMe);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG(MEATLUMP_LOG,msg);
		}
		return true;
	}
}
