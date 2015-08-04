package script.systems.treasure_map.base;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.Vector;
import script.library.ai_lib;
import script.library.create;
import script.library.groundquests;
import script.library.loot;
import script.library.utils;


public class dummy_treasure_drum extends script.base_script
{
	public dummy_treasure_drum()
	{
	}
	public static final String SCRIPT_TREASUREGUARD = "systems.treasure_map.base.treasure_guard";
	public static final String REAL_CHEST = "object/tangible/container/drum/treasure_drum.iff";
	public static final String SCRIPT_REAL_CHEST = "systems.treasure_map.base.treasure_drum";
	public static final String BONUS_LOOT_TABLE_81_90 = "treasure/treasure_bonus_81_90";
	public static final String RARE_BONUS_LOOT_TABLE_81_90 = "treasure/treasure_rare_bonus_81_90";
	public static final String LOOT_TABLE_81_90 = "treasure/treasure_81_90";
	
	public static final String SID_WRONG_PLAYER = "@treasure_map/treasure_map:wrong_player";
	public static final String SID_KILL_GUARDS = "@treasure_map/treasure_map:kill_guards_message";
	public static final String SID_UNLOCK_CHEST = "@treasure_map/treasure_map:unlock_chest";
	
	public static final int TREASURE_LOOT_ITEMS = 6;
	public static final int NUMBER_OF_BONUS_LOOT_ITEMS = 1;
	public static final int EIGHT_MEMBER_GROUP_BONUS = 2;
	public static final int EIGHT_GROUP_PERCENT_CHANCE_FOR_SUPER_RARE = 5;
	public static final int EIGHT_GROUP_PERCENT_CHANCE_FOR_RARE = 20;
	public static final int FOUR_GROUP_PERCENT_CHANCE_FOR_RARE = 25;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "cleanUpFakeChest", null, 1000, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info item) throws InterruptedException
	{
		obj_id fake_chest = self;
		if (!exists(fake_chest))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = getObjIdObjVar(fake_chest, "owner");
		if (!isValidId(owner) || !exists(owner))
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(fake_chest, "recalculateGuardArray", null, 0, false);
		
		if (owner != player)
		{
			sendSystemMessage(player, SID_WRONG_PLAYER, null);
			return SCRIPT_CONTINUE;
		}
		
		obj_id[] guards = getObjIdArrayObjVar(fake_chest, "guards");
		if (guards == null)
		{
			sendSystemMessage(player, SID_UNLOCK_CHEST, null);
			messageTo(fake_chest, "setupChest", null, 1, false);
		}
		else
		{
			sendSystemMessage(player, SID_KILL_GUARDS, null);
			
			for (int i=0; i<guards.length; i++)
			{
				testAbortScript();
				if ((isIdNull(guards[i])) || !exists(guards[i]) || isDead(guards[i]))
				{
					continue;
				}
				
				messageTo(guards[i], "attackThief", null, 0, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int setupChest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id fake_chest = self;
		if (!isValidId(fake_chest) || !exists(fake_chest))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(fake_chest, "owner");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		location treasureLoc = getLocation(fake_chest);
		obj_id treasureChest = createObject(REAL_CHEST, treasureLoc );
		
		if (isValidId(treasureChest) && exists(treasureChest))
		{
			attachScript(treasureChest, SCRIPT_REAL_CHEST);
			if (hasObjVar(fake_chest, "treasureMapWaypoint"))
			{
				setObjVar(treasureChest, "treasureMapWaypoint", getObjIdObjVar(fake_chest, "treasureMapWaypoint"));
			}
			obj_id owner = getObjIdObjVar(fake_chest, "owner");
			
			if (hasObjVar(fake_chest, "owner"))
			{
				setObjVar(treasureChest, "owner", owner);
				setOwner(treasureChest, owner);
			}
			
			int treasureLevel = getIntObjVar(fake_chest, "treasureLevel");
			int groupModifier = getIntObjVar(fake_chest, "groupModifier");
			sendSystemMessage(owner, "groupModifier: "+groupModifier, null);
			
			String loot_table = getStringObjVar(fake_chest, "loot_table");
			
			messageTo(treasureChest, "cleanUpChest", null, 1000, false);
			
			boolean lootIssued = fillTreasureChestWithLoot(player, treasureChest, groupModifier, loot_table, treasureLevel);
			if (lootIssued)
			{
				messageTo(fake_chest, "cleanUpFakeChest", null, 0, false);
			}
			
		}
		else
		{
			
			CustomerServiceLog("treasureMap", "a treasure chest failed spawn and fill with loot for player "+ player + ". [ setupChest() ]");
			messageTo(fake_chest, "cleanUpFakeChest", null, 0, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpFakeChest(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id fake_chest = self;
		if (!isValidId(fake_chest) || !exists(fake_chest))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(fake_chest, "owner");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(fake_chest, "treasureMapWaypoint"))
		{
			obj_id mapWaypoint = getObjIdObjVar(fake_chest, "treasureMapWaypoint");
			if (isIdValid(mapWaypoint))
			{
				destroyWaypointInDatapad(mapWaypoint, player);
				removeObjVar(fake_chest, "treasureMapWaypoint");
			}
		}
		playClientEffectLoc(self, "clienteffect/lair_damage_light.cef", getLocation(self), 1f);
		
		destroyObject(fake_chest);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnTreasureGuards(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id fake_chest = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector guards = null;
		obj_id player = params.getObjId( "player");
		location treasureLoc = params.getLocation( "location");
		String type = params.getString( "type");
		int count = params.getInt( "count");
		int mobLevel = params.getInt( "mobLevel");
		String loot_table = params.getString( "loot_table");
		String boss_mob = params.getString("boss_mob");
		int groupModifier = params.getInt("groupModifier");
		setObjVar(fake_chest, "groupModifier", groupModifier);
		
		if (type.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!boss_mob.equals("none"))
		{
			count++;
			CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
			+ " has eight group members and will receive a boss mob" + ". [ spawnTreasureGuards() ]");
		}
		for (int i = 0; i < count; i++)
		{
			testAbortScript();
			
			location guardLocation = groundquests.getRandom2DLocationAroundLocation(fake_chest, 1, 1, 5, 12);
			
			obj_id antagonist = null;
			if (i == 0 && !boss_mob.equals("none"))
			{
				antagonist = create.object(boss_mob, guardLocation, mobLevel);
			}
			else
			{
				antagonist = create.object(type, guardLocation, mobLevel);
			}
			
			if (!isIdNull(antagonist))
			{
				setYaw( antagonist, rand(0, 359) );
				
				ai_lib.setDefaultCalmBehavior(antagonist, ai_lib.BEHAVIOR_WANDER);
				ai_lib.setLoiterRanges(antagonist, 0, 7);
				setObjVar( antagonist, "player", player );
				attachScript( antagonist, SCRIPT_TREASUREGUARD );
				setObjVar(antagonist, "fake_chest", fake_chest);
				
				if (i == 0)
				{
					faceTo(antagonist, player);
					messageTo( antagonist, "barkAttack", params, 2, false );
					
				}
				
				guards = utils.addElement( guards, antagonist );
			}
		}
		
		if (guards != null && guards.size() > 0)
		{
			setObjVar(fake_chest, "guards", guards);
			
			for (int i = 0; i < guards.size(); i++)
			{
				testAbortScript();
				CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
				+ " has spawned boss mob level: "+ ((obj_id)(guards.get(i)))
				+ " at: "+ getLocation(((obj_id)(guards.get(i))))
				+ ". [ spawnTreasureGuards() ]");
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int recalculateGuardArray(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id fake_chest = self;
		obj_id player = getObjIdObjVar(fake_chest, "owner");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		Vector latestGuardList = new Vector();
		obj_id[] guards = getObjIdArrayObjVar(fake_chest, "guards");
		if (guards == null || guards.length == 0)
		{
			sendSystemMessage(player, SID_UNLOCK_CHEST, null);
			messageTo(fake_chest, "setupChest", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < guards.length; i++)
		{
			testAbortScript();
			if ((isIdNull(guards[i])) || !exists(guards[i]) || isDead(guards[i]))
			{
				continue;
			}
			utils.addElement(latestGuardList, guards[i]);
		}
		obj_id[] newGuardArray = new obj_id[latestGuardList.size()];
		if (newGuardArray == null || newGuardArray.length == 0)
		{
			sendSystemMessage(player, SID_UNLOCK_CHEST, null);
			messageTo(fake_chest, "setupChest", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		latestGuardList.toArray(newGuardArray);
		setObjVar(fake_chest, "guards", newGuardArray);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean fillTreasureChestWithLoot(obj_id player, obj_id treasureChest, int groupModifier, String loot_table, int treasureLevel) throws InterruptedException
	{
		loot.makeLootInContainer(treasureChest, loot_table, TREASURE_LOOT_ITEMS, treasureLevel);
		
		CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
		+ " has spawned a treasure chest: "+ treasureChest
		+ " with loot of level: "+ treasureLevel
		+ " using loot table: "+ loot_table
		+ ". This function gives the chest 6 items. [ fillTreasureChestWithLoot() ]");
		
		if (groupModifier < 4)
		{
			return true;
		}
		
		if (!loot_table.equals(LOOT_TABLE_81_90))
		{
			return true;
		}
		
		if (groupModifier >= 4 && groupModifier < 8)
		{
			int bonus = rand(1,100);
			boolean bonusReceived = fourParticipantBonusLootItem(treasureChest, treasureLevel, bonus);
			
			CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
			+ " has received a bonus treasure item in: "+ treasureChest
			+ " because the player was in a group of 4 or more (but less than 8 total)" + ". This function gives the chest 1 item and the chest should now have 7 items total. [ fillTreasureChestWithLoot() ]");
		}
		
		else if (groupModifier >= 8)
		{
			
			int firstBonus = rand(1,100);
			int secondBonus = rand(1,100);
			boolean firstBonusReceived = eightParticipantBonusLootItem(treasureChest, treasureLevel, firstBonus);
			boolean secondBonusReceived = eightParticipantBonusLootItem(treasureChest, treasureLevel, secondBonus);
			
			CustomerServiceLog("treasureMap", "Player: "+ player + " "+ getName(player)
			+ " has received a TWO bonus treasure items in: "+ treasureChest
			+ " because the player was in a group of 8" + ". This function gives the chest 2 items and the chest should now have 8 items total. [ fillTreasureChestWithLoot() ]");
		}
		return true;
	}
	
	
	public boolean eightParticipantBonusLootItem(obj_id treasureChest, int treasureLevel, int randomNumber) throws InterruptedException
	{
		if (randomNumber <= EIGHT_GROUP_PERCENT_CHANCE_FOR_SUPER_RARE)
		{
			loot.makeLootInContainer(treasureChest, RARE_BONUS_LOOT_TABLE_81_90, NUMBER_OF_BONUS_LOOT_ITEMS, treasureLevel);
			
			CustomerServiceLog("treasureMap", "Treasure Chest: "+ treasureChest
			+ " has received a VERY RARE treasure item because they rolled a: "+ randomNumber
			+ ". The player should have this rare item added to their chest. [ eightParticipantBonusLootItem() ]");
			
			return true;
		}
		else if (randomNumber > EIGHT_GROUP_PERCENT_CHANCE_FOR_SUPER_RARE && randomNumber <= EIGHT_GROUP_PERCENT_CHANCE_FOR_RARE)
		{
			loot.makeLootInContainer(treasureChest, BONUS_LOOT_TABLE_81_90, NUMBER_OF_BONUS_LOOT_ITEMS, treasureLevel);
			
			CustomerServiceLog("treasureMap", "Treasure Chest: "+ treasureChest
			+ " has received a SOMEWHAT RARE treasure item because they rolled a: "+ randomNumber
			+ ". The player should have this rare item added to their chest. [ eightParticipantBonusLootItem() ]");
			
			return true;
		}
		
		loot.makeLootInContainer(treasureChest, LOOT_TABLE_81_90, NUMBER_OF_BONUS_LOOT_ITEMS, treasureLevel);
		
		CustomerServiceLog("treasureMap", "Treasure Chest: "+ treasureChest
		+ " has received a regular roll: "+ randomNumber
		+ ". They still have a slight chance of receiving a somewhat rare item." + " The player should have this rare item added to their chest. [ eightParticipantBonusLootItem() ]");
		
		return true;
	}
	
	
	public boolean fourParticipantBonusLootItem(obj_id treasureChest, int treasureLevel, int randomNumber) throws InterruptedException
	{
		if (randomNumber <= FOUR_GROUP_PERCENT_CHANCE_FOR_RARE)
		{
			loot.makeLootInContainer(treasureChest, BONUS_LOOT_TABLE_81_90, NUMBER_OF_BONUS_LOOT_ITEMS, treasureLevel);
			
			CustomerServiceLog("treasureMap", "Treasure Chest: "+ treasureChest
			+ " has received a SOMEWHAT RARE treasure item because they rolled a: "+ randomNumber
			+ ". The player should have this rare item added to their chest. [ fourParticipantBonusLootItem() ]");
			
			return true;
		}
		
		loot.makeLootInContainer(treasureChest, LOOT_TABLE_81_90, NUMBER_OF_BONUS_LOOT_ITEMS, treasureLevel);
		
		CustomerServiceLog("treasureMap", "Treasure Chest: "+ treasureChest
		+ " has received a regular roll: "+ randomNumber
		+ ". They still have a slight chance of receiving a somewhat rare item." + " The player should have this rare item added to their chest. [ fourParticipantBonusLootItem() ]");
		
		return true;
	}
}
