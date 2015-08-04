package script.theme_park.lok;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.ai.ai;
import script.library.ai_lib;
import script.library.create;
import script.ai.ai_combat;


public class cantina extends script.base_script
{
	public cantina()
	{
	}
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		spawnEveryone(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public void spawnEveryone(obj_id self) throws InterruptedException
	{
		debugSpeakMsg (self, "Spawning Everyone");
		spawnCelebs(self);
		return;
	}
	
	
	public void spawnCelebs(obj_id self) throws InterruptedException
	{
		spawnHan(self);
		spawnChewie(self);
		spawnNien(self);
		spawnPadawanTrialsNpc(self);
		
		spawnGamblor(self);
		spawnRetired(self);
		spawnEngineer(self);
		
	}
	
	
	public obj_id spawnHan(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "private_room2");
		location hanLoc = new location (-30.20f, -0.57f, 7.24f, "lok", room);
		obj_id han = create.staticObject ("han_solo", hanLoc );
		setObjVar (self, "cantinaInhabitants.han", han);
		setYaw (han, 45f);
		setObjVar (han, "cantina", self);
		return han;
	}
	
	
	public obj_id spawnChewie(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "private_room2");
		location chewieLoc = new location (-28.48f, -0.57f, 5.87f, "lok", room);
		obj_id chewie = create.staticObject ("chewbacca", chewieLoc );
		setObjVar (self, "cantinaInhabitants.chewie", chewie);
		setYaw (chewie, 12f);
		setObjVar (chewie, "cantina", self);
		return chewie;
	}
	
	
	public obj_id spawnPadawanTrialsNpc(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "stage");
		location npcLoc = new location (-13.19f, -0.89f, -20.28f, "lok", room);
		obj_id npc = create.object("noren_krast", npcLoc );
		ai_lib.setDefaultCalmMood (npc, "npc_sitting_chair");
		setYaw (npc, 12f);
		setInvulnerable(npc, true);
		setCreatureStatic(npc, true);
		setName(npc, "Vixur Webb (a musician)");
		
		attachScript(npc, "conversation.padawan_old_musician_03");
		
		setObjVar (self, "cantinaInhabitants.padawanTrialsNpc", npc);
		setObjVar (npc, "cantina", self);
		return npc;
	}
	
	
	public obj_id spawnNien(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "alcove5");
		location nienLoc = new location (-12.12f, -0.89f, 23.08f, "lok", room);
		obj_id nien = create.staticObject ("nien_nunb", nienLoc );
		setObjVar (self, "cantinaInhabitants.nien", nien);
		setAnimationMood (nien, "npc_sitting_chair");
		setYaw (nien, 80f);
		setObjVar (nien, "cantina", self);
		return nien;
	}
	
	
	public obj_id spawnGamblor(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "alcove2");
		location gamblorLoc = new location (12.82f, -0.89f, 21.49f, "lok", room);
		obj_id gamblor = create.staticObject ("commoner", gamblorLoc );
		attachScript (gamblor, "theme_park.nym.nym_contact");
		setObjVar (self, "cantinaInhabitants.gamblor", gamblor);
		setAnimationMood (gamblor, "npc_sitting_chair");
		setYaw (gamblor, 89f);
		setObjVar (gamblor, "cantina", self);
		return gamblor;
	}
	
	
	public obj_id spawnEngineer(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "alcove3");
		location engineerLoc = new location (11.63f, -0.89f, -16.56f, "lok", room);
		obj_id engineer = create.staticObject ("nym_themepark_choster_cantina", engineerLoc );
		
		setObjVar (self, "cantinaInhabitants.engineer", engineer);
		setYaw (engineer, -83f);
		setObjVar (engineer, "cantina", self);
		return engineer;
	}
	
	
	public obj_id spawnRetired(obj_id self) throws InterruptedException
	{
		obj_id room = getCellId (self, "back_hallway");
		location initiateLoc = new location (-36.21f, 0.11f, -13.93f, "lok", room);
		obj_id retired = create.staticObject ("nym_themepark_sergeant_moore_cantina", initiateLoc );
		
		setObjVar (self, "cantinaInhabitants.retired", retired);
		setYaw (retired, 87f);
		setObjVar (retired, "cantina", self);
		return retired;
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id speaker, String text) throws InterruptedException
	{
		if (!hasObjVar (speaker, "gm"))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (text.equals("celebs"))
		{
			spawnCelebs(self);
		}
		
		if (text.equals("kill celebs"))
		{
			killCelebs(self);
		}
		
		if (text.equals("kill all"))
		{
			killAll(self);
		}
		
		if (text.equals("padawanTrials"))
		{
			spawnPadawanTrialsNpc(self);
		}
		
		if (text.equals("killPadawan"))
		{
			obj_id npc = getObjIdObjVar(self, "cantinaInhabitants.padawanTrialsNpc");
			if (isIdValid(npc))
			{
				destroyObject(npc);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void killAll(obj_id self) throws InterruptedException
	{
		killCelebs(self);
		return;
	}
	
	
	public void killCelebs(obj_id self) throws InterruptedException
	{
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.han"));
		removeObjVar (self, "cantinaInhabitants.han");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.chewie"));
		removeObjVar (self, "cantinaInhabitants.chewie");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.nien"));
		removeObjVar (self, "cantinaInhabitants.nien");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.gamblor"));
		removeObjVar (self, "cantinaInhabitants.gamblor");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.engineer"));
		removeObjVar (self, "cantinaInhabitants.engineer");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.retired"));
		removeObjVar (self, "cantinaInhabitants.retired");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.protector"));
		removeObjVar (self, "cantinaInhabitants.protector");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.rancortamer"));
		removeObjVar (self, "cantinaInhabitants.rancortamer");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.ranger"));
		removeObjVar (self, "cantinaInhabitants.ranger");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.sentinel"));
		removeObjVar (self, "cantinaInhabitants.sentinel");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.sentry"));
		removeObjVar (self, "cantinaInhabitants.sentry");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.spellweaver"));
		removeObjVar (self, "cantinaInhabitants.spellweaver");
		destroyObject(getObjIdObjVar( self, "cantinaInhabitants.stalker"));
		removeObjVar (self, "cantinaInhabitants.stalker");
		return;
	}
	
}
