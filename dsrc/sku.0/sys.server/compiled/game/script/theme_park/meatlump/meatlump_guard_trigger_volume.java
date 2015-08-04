package script.theme_park.meatlump;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.attrib;
import script.library.create;
import script.library.group;
import script.library.stealth;
import script.library.utils;


public class meatlump_guard_trigger_volume extends script.base_script
{
	public meatlump_guard_trigger_volume()
	{
	}
	public static final boolean LOGGING_ON = false;
	
	public static final String MEATLUMP_TRIGGER_NAME_PREFIX = "meatlump_guardian_trigger_";
	public static final String MEATLUMP_TRIGGER = "meatlump_trigger";
	public static final String MEATLUMP_MOB = "meatlump_outpost_guard_spawned";
	public static final String MEATLUMP_ENEMY_SCRIPT = "creature.dynamic_enemy";
	public static final String PHRASE_STRING_FILE = "phrase_string_file";
	public static final String SPAM_STRING_FILE = "spam";
	public static final String MEATLUMP_LOG = "meatlump_trigger";
	public static final float MEATLUMP_TRIGGER_RADIUS = 8f;
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		createTriggerVolume(MEATLUMP_TRIGGER_NAME_PREFIX + self, MEATLUMP_TRIGGER_RADIUS, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeEntered(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		if (!isPlayer(whoTriggeredMe))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("Entered trigger volume: "+whoTriggeredMe);
		
		if (utils.hasScriptVar(whoTriggeredMe, MEATLUMP_TRIGGER) && utils.getStringScriptVar(whoTriggeredMe, MEATLUMP_TRIGGER).equals(MEATLUMP_TRIGGER_NAME_PREFIX+self))
		{
			blog("Already triggered volume: "+whoTriggeredMe);
			return SCRIPT_CONTINUE;
		}
		
		utils.setScriptVar(whoTriggeredMe, MEATLUMP_TRIGGER, MEATLUMP_TRIGGER_NAME_PREFIX+self);
		
		String invis = stealth.getInvisBuff(whoTriggeredMe);
		if (invis != null)
		{
			stealth.checkForAndMakeVisibleNoRecourse(self);
		}
		
		int mobLevel = getLevel(whoTriggeredMe);
		
		location curLoc = getLocation(self);
		
		obj_id mob = create.object(MEATLUMP_MOB, curLoc, mobLevel);
		
		setObjVar(mob, "player", whoTriggeredMe);
		
		addHate(mob, whoTriggeredMe, 1);
		
		setObjVar(mob, PHRASE_STRING_FILE, SPAM_STRING_FILE);
		
		getRandomPhrases(mob);
		
		attachScript(mob, MEATLUMP_ENEMY_SCRIPT);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTriggerVolumeExited(obj_id self, String volumeName, obj_id whoTriggeredMe) throws InterruptedException
	{
		blog("Exited trigger volume: "+whoTriggeredMe);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getRandomPhrases(obj_id mob) throws InterruptedException
	{
		if (!isValidId(mob) || !exists(mob))
		{
			return false;
		}
		
		int attackPhraseInt = rand(1,10);
		int defeatPhraseInt = rand(1,10);
		
		String attackPhrase = "enemy_attack_you_thief";
		String defeatPhrase = "enemy_defeat_tell_everyone";
		
		switch(attackPhraseInt)
		{
			case 1:
			attackPhrase = "enemy_attack_rest";
			break;
			case 2:
			attackPhrase = "enemy_attack_oh_look";
			break;
			case 3:
			attackPhrase = "enemy_attack_surprise";
			break;
			case 4:
			attackPhrase = "enemy_attack_now_got";
			break;
			case 5:
			attackPhrase = "enemy_attack_what_did";
			break;
			case 6:
			attackPhrase = "enemy_attack_sneaky";
			break;
			case 7:
			attackPhrase = "enemy_attack_youre_not";
			break;
			case 8:
			attackPhrase = "enemy_attack_lotta_nerve";
			break;
			case 9:
			attackPhrase = "enemy_attack_stop";
			break;
			case 10:
			attackPhrase = "enemy_attack_you_thief";
			break;
			default :
			break;
		}
		
		switch(defeatPhraseInt)
		{
			case 1:
			defeatPhrase = "enemy_defeat_oops";
			break;
			case 2:
			defeatPhrase = "enemy_defeat_challenging";
			break;
			case 3:
			defeatPhrase = "enemy_defeat_boo_hoo";
			break;
			case 4:
			defeatPhrase = "enemy_defeat_take_that";
			break;
			case 5:
			defeatPhrase = "enemy_defeat_entertainer";
			break;
			case 6:
			defeatPhrase = "enemy_defeat_trader";
			break;
			case 7:
			defeatPhrase = "enemy_defeat_sweat";
			break;
			case 8:
			defeatPhrase = "enemy_defeat_training";
			break;
			case 9:
			defeatPhrase = "enemy_defeat_lunch";
			break;
			case 10:
			defeatPhrase = "enemy_defeat_tell_everyone";
			break;
			default :
			break;
		}
		
		setObjVar(mob, "attack_phrase", attackPhrase);
		setObjVar(mob, "defeat_phrase", defeatPhrase);
		return true;
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
