package script.theme_park.outbreak;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.factions;
import script.library.groundquests;
import script.library.trial;
import script.library.utils;


public class blackwing_canister_destroy extends script.base_script
{
	public blackwing_canister_destroy()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "blackwing_canister_destroy.OnAttach() Canister ("+self+") has the proper script");
		messageTo(self, "setUp", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int setUp(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "blackwing_canister_destroy.setUp() Canister ("+self+") is getting the player owner OID.");
		
		obj_id player = getObjIdObjVar(self, "owner");
		if (isValidId(player) && exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_destroy.OnAttach() Canister ("+self+") has the player OID: "+player);
			playClientEffectObj(player, "sound/amb_beeping_timer.snd", self, "");
		}
		messageTo(self, "blowUp", null, 5, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int blowUp(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "blackwing_canister_destroy.blowUp() blowUp ("+self+") is blowing up. the player owner OID.");
		obj_id player = getObjIdObjVar(self, "owner");
		if (!isValidId(player) || !exists(player))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Could not find the player owner for this object: "+self);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		if (hasCompletedCollection(player, "deathtrooper_virus_canister_demo"))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Player completed collection already. Aborting credit for Player: "+player);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		String quest = getStringObjVar(self, "quest");
		if (quest == null || quest.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Could not find the parent spawner quest for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, quest, "canisterWaitingForSignal"))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Player: "+player+" did not have the quest "+quest+" needed for this collection. canister: ("+self);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		String questSignal = getStringObjVar(self, "questSignal");
		if (questSignal == null || questSignal.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Could not find the parent spawner quest SIGNAL for participating in the canister quest for canister: ("+self+"). The operation was aborted for Player: "+player);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		String collectionSlot = getStringObjVar(self, "collectionSlot");
		if (collectionSlot == null || collectionSlot.length() <= 0)
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Could not find canister: "+self+" collection slot: "+collectionSlot);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasCompletedCollectionSlotPrereq(player, collectionSlot))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() The player: "+player+" somehow managed to destroy the canister even though they do not have the collection prereq slot. object: "+self);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		if (hasCompletedCollectionSlot(player, collectionSlot))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Player completed collection SLOT. Aborting credit for Player: "+player);
			playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
			destroyObject(self);
			return SCRIPT_CONTINUE;
		}
		
		playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0);
		
		if (modifyCollectionSlotValue(player, collectionSlot, 1))
		{
			CustomerServiceLog("outbreak_themepark", "blackwing_canister_collection.blowUp() Player: "+player+" has received collection credit for the collection slot: "+collectionSlot+"completed collection SLOT. Aborting credit for Player: "+player);
			
			if (hasCompletedCollectionSlot(player, collectionSlot))
			{
				groundquests.sendSignal(player, questSignal);
			}
		}
		obj_id[] playerTargets = trial.getValidPlayersInRadius(self, 7);
		obj_id[] npcTargets = getNPCsInRange(getLocation(self), 10);
		
		playClientEffectLoc(self, "clienteffect/combat_explosion_lair_large.cef", getLocation(self), 0.4f);
		
		if (playerTargets != null && playerTargets.length > 0)
		{
			CustomerServiceLog("outbreak_themepark", "beast_pathing.blowUp() Valid player in radius found.");
			
			for (int i=0; i<playerTargets.length; i++)
			{
				testAbortScript();
				CustomerServiceLog("outbreak_themepark", "beast_pathing.blowUp() Valid player #"+i+" is about to be incapped.");
				setPosture(playerTargets[i], POSTURE_INCAPACITATED);
				int damageAmount = getAttrib(playerTargets[i], HEALTH) + 1000;
				damage(playerTargets[i], DAMAGE_KINETIC, HIT_LOCATION_BODY, damageAmount);
			}
		}
		
		if (npcTargets != null && npcTargets.length > 0)
		{
			CustomerServiceLog("outbreak_themepark", "beast_pathing.blowUp() Valid NPCs in radius.");
			
			for (int a=0; a<npcTargets.length; a++)
			{
				testAbortScript();
				if (!isValidId(npcTargets[a]) || !exists(npcTargets[a]))
				{
					continue;
				}
				if (!hasObjVar(npcTargets[a], factions.FACTION))
				{
					continue;
				}
				
				if (factions.getFaction(npcTargets[a]).equals("afflicted"))
				{
					setPosture(npcTargets[a], POSTURE_INCAPACITATED);
				}
			}
		}
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
}
