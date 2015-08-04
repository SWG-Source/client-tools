package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.factions;
import script.library.gcw;
import script.library.trial;
import script.library.utils;


public class gcw_city_kit extends script.base_script
{
	public gcw_city_kit()
	{
	}
	public static final int CONSTRUCTION_HP_BASE = 5000;
	
	
	public void destroyKitObj(obj_id self) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return;
		}
		
		obj_id kitObj = getObjIdObjVar(self, "gcw.kitObj");
		
		if (!isIdValid(kitObj) || !exists(kitObj))
		{
			return;
		}
		
		trial.cleanupObject(kitObj);
		
		removeObjVar(self, "gcw.kitObj");
		utils.removeScriptVar(self, "iconMessageTime");
	}
	
	
	public int OnHearSpeech(obj_id self, obj_id objSpeaker, String strText) throws InterruptedException
	{
		if (!isGod(objSpeaker))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (strText.equals("gcwcleanup"))
		{
			destroyKitObj(self);
		}
		
		if (strText.equals("gcwbuildi"))
		{
			
			dictionary params = new dictionary();
			
			params.put("faction", factions.FACTION_FLAG_IMPERIAL);
			
			messageTo(self, "beginConstruction", params, 1.0f, false);
		}
		
		if (strText.equals("gcwbuildr"))
		{
			
			dictionary params = new dictionary();
			
			params.put("faction", factions.FACTION_FLAG_REBEL);
			
			messageTo(self, "beginConstruction", params, 1.0f, false);
		}
		
		if (strText.equals("gcwinvadei"))
		{
			
			dictionary params = new dictionary();
			
			obj_id pylon = getObjIdObjVar(self, "gcw.kitObj");
			
			if (!isIdValid(pylon) || !exists(pylon))
			{
				return SCRIPT_CONTINUE;
			}
			
			setObjVar(pylon, "gcw.constructionQuestsCompleted", 40);
			
			params.put("faction", factions.FACTION_FLAG_IMPERIAL);
			
			messageTo(self, "beginInvasion", params, 1.0f, false);
		}
		
		if (strText.equals("gcwinvader"))
		{
			
			dictionary params = new dictionary();
			
			obj_id pylon = getObjIdObjVar(self, "gcw.kitObj");
			
			if (!isIdValid(pylon) || !exists(pylon))
			{
				return SCRIPT_CONTINUE;
			}
			
			setObjVar(pylon, "gcw.constructionQuestsCompleted", 40);
			
			params.put("faction", factions.FACTION_FLAG_REBEL);
			
			messageTo(self, "beginInvasion", params, 1.0f, false);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		destroyKitObj(self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int getConstructionQuestsCompleted(obj_id pylon) throws InterruptedException
	{
		int completed = 0;
		
		if (!isIdValid(pylon) || !exists(pylon))
		{
			return 0;
		}
		
		if (hasObjVar(pylon, "gcw.constructionQuestsCompleted"))
		{
			completed = getIntObjVar(pylon, "gcw.constructionQuestsCompleted");
		}
		
		return completed;
	}
	
	
	public boolean hasConstructionQuests() throws InterruptedException
	{
		return true;
	}
	
	
	public void setupConstructionQuests(obj_id self, obj_id pylon) throws InterruptedException
	{
	}
	
	
	public int beginConstruction(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyKitObj(self);
		
		int faction = params.getInt("faction");
		
		if (hasObjVar(self, "factionFlag"))
		{
			faction = getIntObjVar(self, "factionFlag");
		}
		
		if (faction != factions.FACTION_FLAG_REBEL && faction != factions.FACTION_FLAG_IMPERIAL)
		{
			return SCRIPT_CONTINUE;
		}
		
		location loc = getLocation(self);
		
		if (hasConstructionQuests())
		{
			
			obj_id pylon = createObject("object/tangible/destructible/gcw_city_construction_beacon.iff", loc);
			
			if (isIdValid(pylon) && exists(pylon))
			{
				setObjVar(self, "gcw.kitObj", pylon);
				utils.setObjVar(self, "factionFlag", faction);
				utils.setScriptVar(pylon, "faction", faction);
				setObjVar(pylon, "gcw.constructionQuestsCompleted", getObjVar(self, "gcw.constructionQuestsCompleted"));
				
				String spawnID = getStringObjVar(self, "spawn_id");
				
				setObjVar(pylon, "spawn_id", spawnID);
				
				obj_id kitParent = trial.getParent(self);
				
				if (isIdValid(kitParent) && exists(kitParent))
				{
					trial.setParent(kitParent, pylon, true);
					trial.storeSpawnedChild(kitParent, pylon, spawnID);
				}
				
				trial.markAsTempObject(pylon, true);
				trial.setInterest(pylon);
				
				attachScript(pylon, "systems.dungeon_sequencer.ai_controller");
				
				setupConstructionQuests(self, pylon);
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void setupInvasionQuests(obj_id kit) throws InterruptedException
	{
	}
	
	
	public obj_id createFactionKit(int faction, location loc) throws InterruptedException
	{
		return null;
	}
	
	
	public int getConstructionHP() throws InterruptedException
	{
		return CONSTRUCTION_HP_BASE;
	}
	
	
	public int beginInvasion(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isIdValid(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id pylon = getObjIdObjVar(self, "gcw.kitObj");
		
		int construction = getConstructionQuestsCompleted(pylon);
		int faction = getIntObjVar(self, "factionFlag");
		
		destroyKitObj(self);
		
		if (construction < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "gcw.constructionQuestsCompleted", construction);
		
		location loc = getLocation(self);
		
		obj_id kit = createFactionKit(faction, loc);
		
		if (isIdValid(kit) && exists(kit))
		{
			setYaw(kit, getYaw(self));
			
			setObjVar(self, "gcw.kitObj", kit);
			setObjVar(self, "factionFlag", faction);
			utils.setScriptVar(kit, "faction", faction);
			
			if (faction == factions.FACTION_FLAG_REBEL)
			{
				factions.setFaction(kit, "Rebel");
			}
			else if (faction == factions.FACTION_FLAG_IMPERIAL)
			{
				factions.setFaction(kit, "Imperial");
			}
			
			utils.setScriptVar(kit, "creator", self);
			
			obj_id kitParent = trial.getParent(self);
			
			String spawnId = getStringObjVar(self, "spawn_id");
			
			if (isIdValid(kitParent) && exists(kitParent))
			{
				trial.setParent(kitParent, kit, true);
				if (spawnId.equals("offense_patrol") || spawnId.equals("vehicle_patrol"))
				{
					trial.storeSpawnedChild(kitParent, kit, "offense_quest_asset");
					setObjVar(kit, "spawn_id", "offense_quest_asset");
				}
				else if (spawnId.equals("defense_patrol"))
				{
					trial.storeSpawnedChild(kitParent, kit, "defense_quest_asset");
					setObjVar(kit, "spawn_id", "defense_quest_asset");
				}
				else
				{
					trial.storeSpawnedChild(kitParent, kit, spawnId);
					setObjVar(kit, "spawn_id", spawnId);
				}
			}
			
			trial.markAsTempObject(kit, true);
			trial.setInterest(kit);
			trial.setHp(kit, construction * getConstructionHP());
			
			if (!isMob(kit))
			{
				setObjVar(kit, "pvpCanAttack", 1);
				setObjVar(kit, "ai_combat.attackable", 1);
			}
			
			attachScript(kit, "systems.dungeon_sequencer.ai_controller");
		}
		
		setupInvasionQuests(kit);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanup(obj_id self, dictionary params) throws InterruptedException
	{
		destroyKitObj(self);
		
		return SCRIPT_CONTINUE;
	}
}
