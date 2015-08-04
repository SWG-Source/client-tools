package script.creature;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.anims;
import script.library.chat;
import script.library.create;
import script.library.group;
import script.library.groundquests;
import script.library.prose;
import script.library.utils;
import script.ai.ai_combat;


public class foraging_enemy extends script.base_script
{
	public foraging_enemy()
	{
	}
	public static final String STRING_PREFIX = "foraging/forage_enemy";
	public static final string_id SID_STEAL_SLOT = new string_id(STRING_PREFIX,"slot_removed");
	public static final String FORAGING_ENEMY_TABLE = "datatables/foraging/forage_enemy.iff";
	public static final String FORAGING_COLLECTION = "kill_forage_worm";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "cleanUpGuard", null, 400, true);
		
		messageTo(self, "attackForager", null, 1, false);
		messageTo(self, "barkAttack", null, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitateTarget(obj_id self, obj_id victim) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (player != victim)
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "expression", null, 7, true);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLoiterMoving(obj_id self) throws InterruptedException
	{
		messageTo(self, "cleanUpGuard", null, 6, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpGuard(obj_id self, dictionary params) throws InterruptedException
	{
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int expression(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		faceTo(self, player);
		messageTo(self, "barkDefeat", null, 1, false);
		
		location body = getLocation(player);
		pathTo(self, body);
		
		messageTo(self, "runAwayThenCleanUpGuard", null, 4, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int runAwayThenCleanUpGuard(obj_id self, dictionary params) throws InterruptedException
	{
		location l = groundquests.getRandom2DLocationAroundPlayer(self, 20, 50);
		pathTo(self, l);
		messageTo( self, "cleanUpGuard", null, 12, true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int attackForager(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		startCombat(self, player);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean isThief(obj_id mob) throws InterruptedException
	{
		if (!isValidId(mob))
		{
			return false;
		}
		
		String mobName = getStringObjVar(mob, "creature_type");
		
		if ((mobName == null) || (mobName.equals("")))
		{
			return false;
		}
		
		int index = mobName.indexOf("forage_");
		if (index == 0)
		{
			return true;
		}
		
		return false;
	}
	
	
	public boolean removeFirstSlotFound(obj_id player) throws InterruptedException
	{
		if (!isValidId(player))
		{
			return false;
		}
		
		if (hasCompletedCollection(player, FORAGING_COLLECTION))
		{
			return false;
		}
		
		String[] slotsInCollection = getAllCollectionSlotsInCollection(FORAGING_COLLECTION);
		
		for (int i = 0; i < slotsInCollection.length; i++)
		{
			testAbortScript();
			
			long collectionSlotValue = getCollectionSlotValue(player, slotsInCollection[i]);
			if (collectionSlotValue > 0)
			{
				
				modifyCollectionSlotValue(player, slotsInCollection[i], (collectionSlotValue * -1));
				
				prose_package pp = new prose_package();
				prose.setStringId(pp, SID_STEAL_SLOT);
				prose.setTT(pp, new string_id("collection_n", slotsInCollection[i]));
				sendSystemMessageProse(player, pp);
				
				break;
			}
		}
		
		return true;
	}
	
	
	public int barkAttack(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id guard = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean isThief = isThief(guard);
		if (!isThief)
		{
			return SCRIPT_CONTINUE;
		}
		
		String mob = getStringObjVar(guard, "creature_type");
		if ((mob == null) || (mob.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int row = dataTableSearchColumnForString(mob, "enemy", FORAGING_ENEMY_TABLE);
		String type = dataTableGetString(FORAGING_ENEMY_TABLE, row, "bark_attack");
		if ((type == null) || (type.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		chat.chat(self, chat.CHAT_SHOUT, chat.MOOD_ANGRY, new string_id("foraging/forage_enemy", "bark_"+type));
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int barkDefeat(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id guard = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean isThief = isThief(guard);
		if (!isThief)
		{
			return SCRIPT_CONTINUE;
		}
		
		String mob = getStringObjVar(guard, "creature_type");
		if ((mob == null) || (mob.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		int row = dataTableSearchColumnForString(mob, "enemy", FORAGING_ENEMY_TABLE);
		String type = dataTableGetString(FORAGING_ENEMY_TABLE, row, "bark_defeat");
		if ((type == null) || (type.equals("")))
		{
			return SCRIPT_OVERRIDE;
		}
		
		chat.chat(self, chat.CHAT_SHOUT, chat.MOOD_ANGRY, new string_id("foraging/forage_enemy", "bark_"+type));
		messageTo( self, "removeASlot", null, 4, true );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeASlot(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id guard = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(self, "player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean success = removeFirstSlotFound(player);
		if (success)
		{
			doAnimationAction(self, anims.PLAYER_FLEX_BICEPS);
		}
		
		return SCRIPT_CONTINUE;
	}
}
