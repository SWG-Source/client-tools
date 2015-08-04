package script.event.gcwraids;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.utils;
import script.library.create;
import script.library.locations;
import script.library.chat;
import java.util.StringTokenizer;
import script.library.gcw;
import script.library.ai_lib;


public class cheerleader extends script.base_script
{
	public cheerleader()
	{
	}
	public static final int NUMSPEECHES = 1;
	public static final String STF_FILE = "event/gcw_raids";
	public static final float MY_VISIT_TIME = 60 * 15;
	public static final String DATATABLE = "datatables/event/gcwraid/city_data.iff";
	
	public static final String[] SHUTTLETYPE =
	{
		"object/creature/npc/theme_park/lambda_shuttle.iff",
		"object/creature/npc/theme_park/player_shuttle.iff"
	};
	public static final String[] CELEB =
	{
		"darth_vader", "luke_skywalker"
	};
	public static final String[] ESCORT =
	{
		"fbase_dark_trooper_extreme", "rebel_commando"
	};
	
	
	public int startCheerleaderEvent(obj_id self, dictionary params) throws InterruptedException
	{
		
		float imp_r = gcw.getImperialRatio(self);
		float reb_r = gcw.getRebelRatio(self);
		
		if (reb_r > imp_r)
		{
			setObjVar(self, "event.gcwraids.cheerleader_type", 1);
		}
		else
		{
			setObjVar(self, "event.gcwraids.cheerleader_type", 0);
		}
		
		location here = getLocation(self);
		String myCity = locations.getGuardSpawnerRegionName(here);
		
		if (myCity.equals("@tatooine_region_names:bestine"))
		{
			setObjVar(self, "event.gcwraids.cheerleader_type", 0);
		}
		
		if (myCity.equals("@tatooine_region_names:anchorhead"))
		{
			setObjVar(self, "event.gcwraids.cheerleader_type", 1);
		}
		
		messageTo(self, "createShuttle", null, 1, false);
		removeObjVar(self, "auto_invasion.next_invasion_time");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int createShuttle(obj_id self, dictionary params) throws InterruptedException
	{
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		location here = getLocation(self);
		obj_id shuttle = create.object(SHUTTLETYPE[type], here);
		
		setObjVar(self, "event.gcwraids.shuttle", shuttle);
		
		setYaw(shuttle, 178);
		detachScript(shuttle, "ai.ai");
		detachScript(shuttle, "ai.creature_combat");
		detachScript(shuttle, "skeleton.humanoid");
		detachScript(shuttle, "systems.combat.combat_actions");
		detachScript(shuttle, "systems.combat.credit_for_kills");
		stop(shuttle);
		
		if (type == 1)
		{
			setPosture(shuttle, POSTURE_PRONE);
		}
		
		messageTo(self, "landShuttle", null, 6, false);
		
		obj_id[] objPlayers = getPlayerCreaturesInRange(self, 256.0f);
		
		if (objPlayers != null && objPlayers.length > 0)
		{
			for (int i = 0; i < objPlayers.length; i++)
			{
				testAbortScript();
				sendSystemMessage(objPlayers[i], new string_id(STF_FILE, "areabroadcast_"+ type) );
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int landShuttle(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id shuttle = getObjIdObjVar(self, "event.gcwraids.shuttle");
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		
		if (type == 1)
		{
			queueCommand(shuttle, (-1465754503), self, "", COMMAND_PRIORITY_FRONT);
			setPosture(shuttle, POSTURE_UPRIGHT);
		}
		
		if (type == 0)
		{
			queueCommand(shuttle, (-1114832209), self, "", COMMAND_PRIORITY_FRONT);
			setPosture(shuttle, POSTURE_PRONE);
		}
		
		messageTo(self, "spawnEscortsAndCeleb", null, 30, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnEscortsAndCeleb(obj_id self, dictionary params) throws InterruptedException
	{
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		location here = getLocation(self);
		location celebSpot = (location)here.clone();
		celebSpot.z -= 12;
		
		obj_id celeb = create.object(CELEB[type], celebSpot);
		setObjVar(celeb, "event.gcwraids.type", type);
		ai_lib.setDefaultCalmBehavior(celeb, ai_lib.BEHAVIOR_SENTINEL );
		stop(celeb);
		setInvulnerable(celeb, true);
		attachScript(celeb, "event.gcwraids.celeb_respect");
		setObjVar(self, "event.gcwraids.celeb", celeb);
		
		if (type == 0)
		{
			detachScript(celeb, "npc.celebrity.darth_vader");
			detachScript(celeb, "theme_park.imperial.quest_convo");
			detachScript(celeb, "npc.converse.npc_converse_menu");
		}
		
		if (type == 1)
		{
			detachScript(celeb, "npc.celebrity.luke");
			detachScript(celeb, "theme_park.rebel.quest_convo");
			detachScript(celeb, "npc.converse.npc_converse_menu");
		}
		
		clearCondition(celeb, CONDITION_CONVERSABLE);
		
		obj_id[] objPlayers = getPlayerCreaturesInRange(self, 100.0f);
		
		if (objPlayers != null && objPlayers.length > 0)
		{
			for (int i = 0; i < objPlayers.length; i++)
			{
				testAbortScript();
				if (type == 0)
				{
					playMusic(objPlayers[i], "sound/music_emperor_theme_stereo.snd");
				}
				
				if (type == 1)
				{
					playMusic(objPlayers[i], "sound/music_ambience_desert_stereo.snd");
				}
			}
		}
		
		for (int i = 0; i < 12; i++)
		{
			testAbortScript();
			obj_id escort = create.object(ESCORT[type], here);
			setObjVar(self, "event.gcwraids.escort"+ i, escort);
			setObjVar(escort, "event.gcwraids.myId", i);
			setObjVar(escort, "event.gcwraids.mom", self);
			attachScript(escort, "event.gcwraids.cheerleader_escort");
			ai_lib.setDefaultCalmBehavior(celeb, ai_lib.BEHAVIOR_WANDER);
		}
		
		messageTo(self, "walkForwardCeleb", null, 3, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int walkForwardCeleb(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		location there = getLocation(self);
		there.z += -25;
		
		String celebType = getCreatureName(celeb);
		if (celebType.equals("darth_vader"))
		{
			ai_lib.aiPathTo(celeb, there);
			setMovementWalk(celeb);
		}
		else
		{
			ai_lib.aiPathTo(celeb, there);
			setMovementRun(celeb);
		}
		
		int dialogueSet = rand(1, NUMSPEECHES);
		utils.setScriptVar(self, "dialogue_set", dialogueSet);
		utils.setScriptVar(self, "dialogue_step", 0);
		messageTo(self, "engageInJingoisticRhetoric", null, 8, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int engageInJingoisticRhetoric(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		int dialogueStep = utils.getIntScriptVar(self, "dialogue_step");
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		int dialogueSet = utils.getIntScriptVar(self, "dialogue_set");
		
		if (dialogueStep > 10)
		{
			messageTo(self, "celebStartGivingQuests", null, 10, false);
			return SCRIPT_CONTINUE;
		}
		
		string_id myLine = new string_id(STF_FILE, "cheerleader_"+ type + "_"+ dialogueSet + "_"+ dialogueStep);
		
		chat.chat(celeb, chat.CHAT_SAY, chat.MOOD_NONE, myLine);
		
		if (dialogueStep == 3 && type == 0)
		{
			doAnimationAction(celeb, "pound_fist_palm");
		}
		
		if (dialogueStep == 5 && type == 0)
		{
			doAnimationAction(celeb, "force_push");
		}
		
		if (dialogueStep == 7 && type == 0)
		{
			doAnimationAction(celeb, "force_choke");
		}
		
		if (dialogueStep == 10 && type == 0)
		{
			doAnimationAction(celeb, "point_forward");
		}
		
		if (dialogueStep == 3 && type == 1)
		{
			doAnimationAction(celeb, "point_right");
		}
		
		if (dialogueStep == 5 && type == 1)
		{
			doAnimationAction(celeb, "pound_fist_palm");
		}
		
		if (dialogueStep == 7 && type == 1)
		{
			doAnimationAction(celeb, "nod_head_once");
		}
		
		if (dialogueStep == 10 && type == 1)
		{
			doAnimationAction(celeb, "point_forward");
		}
		
		dialogueStep++;
		utils.setScriptVar(self, "dialogue_step", dialogueStep);
		
		messageTo(self, "engageInJingoisticRhetoric", null, 10, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int celebStartGivingQuests(obj_id self, dictionary params) throws InterruptedException
	{
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		setCondition(celeb, CONDITION_CONVERSABLE);
		
		if (type == 0)
		{
			attachScript(celeb, "conversation.event_cheerleader_vader");
		}
		
		if (type == 1)
		{
			attachScript(celeb, "conversation.event_cheerleader_luke");
		}
		
		messageTo(self, "everyoneWalkBack", null, MY_VISIT_TIME, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int everyoneWalkBack(obj_id self, dictionary params) throws InterruptedException
	{
		location here = getLocation(self);
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		setObjVar(self, "auto_invasion.invasion_active", 0);
		
		if (isIdValid(celeb))
		{
			String celebType = getCreatureName(celeb);
			if (celebType.equals("darth_vader"))
			{
				ai_lib.aiPathTo(celeb, here);
				setMovementWalk(celeb);
			}
			else
			{
				ai_lib.aiPathTo(celeb, here);
				setMovementRun(celeb);
			}
		}
		
		for (int i = 0; i < 12; i++)
		{
			testAbortScript();
			obj_id escort = getObjIdObjVar(self, "event.gcwraids.escort"+ i);
			
			if (isIdValid(escort))
			{
				ai_lib.aiPathTo(escort, here);
			}
		}
		
		messageTo(self, "endEvent", null, 10, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int endEvent(obj_id self, dictionary params) throws InterruptedException
	{
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		obj_id shuttle = getObjIdObjVar(self, "event.gcwraids.shuttle");
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		destroyObject(celeb);
		
		for (int i = 0; i < 12; i++)
		{
			testAbortScript();
			obj_id escort = getObjIdObjVar(self, "event.gcwraids.escort"+ i);
			
			if (isIdValid(escort))
			{
				destroyObject(escort);
			}
		}
		
		if (type > 0)
		{
			queueCommand(shuttle, (-1114832209), self, "", COMMAND_PRIORITY_FRONT);
			setPosture(shuttle, POSTURE_PRONE);
		}
		
		if (type == 0)
		{
			queueCommand(shuttle, (-1465754503), self, "", COMMAND_PRIORITY_FRONT);
			setPosture(shuttle, POSTURE_UPRIGHT);
		}
		
		messageTo(self, "cleanupShuttleAndStuff", null, 20, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanupShuttleAndStuff(obj_id self, dictionary params) throws InterruptedException
	{
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		obj_id shuttle = getObjIdObjVar(self, "event.gcwraids.shuttle");
		destroyObject(shuttle);
		removeObjVar(self, "event.gcwraids.celeb");
		removeObjVar(self, "event.gcwraids.shuttle");
		
		for (int i = 0; i < 12; i++)
		{
			testAbortScript();
			removeObjVar(self, "event.gcwraids.escort"+ i);
		}
		
		obj_id[] objPlayers = getPlayerCreaturesInRange(self, 256.0f);
		
		if (objPlayers != null && objPlayers.length > 0)
		{
			for (int i = 0; i < objPlayers.length; i++)
			{
				testAbortScript();
				sendSystemMessage(objPlayers[i], new string_id(STF_FILE, "closingbroadcast_"+ type) );
			}
		}
		
		int referenceNumber = getIntObjVar(self, "auto_invasion.reference_number");
		float minInvasionTime = dataTableGetFloat(DATATABLE, referenceNumber, "MINTIME");
		float timeChunkSize = dataTableGetFloat(DATATABLE, referenceNumber, "TIMECHUNKSIZE");
		int numTimeChunk = dataTableGetInt(DATATABLE, referenceNumber, "NUMTIMECHUNK");
		float rightNow = getGameTime();
		
		if (!hasObjVar(self, "auto_invasion.next_invasion_time"))
		{
			float nextInvasionTime = ( rand(1, numTimeChunk) * timeChunkSize ) + minInvasionTime + rightNow;
			setObjVar(self, "auto_invasion.next_invasion_time", nextInvasionTime);
		}
		
		setObjVar(self, "auto_invasion.testing_multiplier", 1);
		messageTo(self, "invasionTimerPing", null, 2700, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int escortDied(obj_id self, dictionary params) throws InterruptedException
	{
		int invasionActive = getIntObjVar(self, "auto_invasion.invasion_active");
		
		if (invasionActive != 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		location here = getLocation(self);
		int type = getIntObjVar(self, "event.gcwraids.cheerleader_type");
		obj_id deadEscort = params.getObjId("escort");
		int myId = params.getInt("myId");
		obj_id celeb = getObjIdObjVar(self, "event.gcwraids.celeb");
		obj_id verifyEscort = getObjIdObjVar(self, "event.gcwraids.escort"+ myId);
		
		if (deadEscort == verifyEscort)
		{
			removeObjVar(self, "event.gcwraids.escort"+ myId);
			obj_id escort = create.object(ESCORT[type], here);
			setObjVar(self, "event.gcwraids.escort"+ myId, escort);
			setObjVar(escort, "event.gcwraids.myId", myId);
			setObjVar(escort, "event.gcwraids.mom", self);
			attachScript(escort, "event.gcwraids.cheerleader_escort");
		}
		
		return SCRIPT_CONTINUE;
	}
}
