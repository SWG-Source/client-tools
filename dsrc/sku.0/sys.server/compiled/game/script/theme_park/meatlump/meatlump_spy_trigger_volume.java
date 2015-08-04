package script.theme_park.meatlump;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.create;
import script.library.groundquests;
import script.library.stealth;
import script.library.utils;


public class meatlump_spy_trigger_volume extends script.base_script
{
	public meatlump_spy_trigger_volume()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String TRIGGER_NAME_PREFIX = "meatlump_spy_trigger_";
	public static final String TRIGGER = "meatlump_spy_trigger";
	public static final String ENEMY_MOB = "mtp_hideout_infiltrator";
	public static final String MEATLUMP_ENEMY_SCRIPT = "creature.dynamic_enemy";
	public static final String PHRASE_STRING_FILE = "phrase_string_file";
	public static final String SPAM_STRING_FILE = "spam";
	public static final String MEATLUMP_LOG = "meatlump_trigger";
	public static final String CLIENT_EFFECT = "appearance/pt_smoke_puff.prt";
	public static final String QUEST_STRING = "quest/mtp_find_infiltrator_";
	public static final String QUEST_TASK_NAME = "findInfiltrator";
	public static final String QUEST_SIGNAL = "findTheInfiltrator";
	public static final float TRIGGER_RADIUS = 2f;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		createTriggerVolume(TRIGGER_NAME_PREFIX + self, TRIGGER_RADIUS, true);
		
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
		
		if (!hasObjVar(self, "spawner"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int spawner = getIntObjVar(self, "spawner");
		if (!groundquests.isQuestActive(whoTriggeredMe, QUEST_STRING+spawner))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("Ground Quest Found!");
		
		if (!groundquests.isTaskActive(whoTriggeredMe, QUEST_STRING+spawner, QUEST_TASK_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		blog("Ground Task Found!");
		
		String invis = stealth.getInvisBuff(whoTriggeredMe);
		if (invis != null)
		{
			stealth.checkForAndMakeVisibleNoRecourse(self);
		}
		
		blog("Sending Signal!");
		
		groundquests.sendSignal(whoTriggeredMe, QUEST_SIGNAL);
		
		blog("Signal Sent!");
		
		int mobLevel = getLevel(whoTriggeredMe);
		
		location curLoc = getLocation(self);
		
		blog("Creating Mob");
		obj_id mob = create.object(ENEMY_MOB, curLoc, mobLevel);
		if (!isValidId(mob) || !exists(mob))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(mob, "player", whoTriggeredMe);
		
		addHate(mob, whoTriggeredMe, 1);
		
		setObjVar(mob, PHRASE_STRING_FILE, SPAM_STRING_FILE);
		
		setObjVar(mob, "attack_phrase", "infiltrator_attack_oops");
		setObjVar(mob, "defeat_phrase", "infiltrator_defeat_tell");
		
		playClientEffectObj(mob, CLIENT_EFFECT, mob, "");
		setObjVar(mob, "clientEffect", CLIENT_EFFECT);
		attachScript(mob, MEATLUMP_ENEMY_SCRIPT);
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
