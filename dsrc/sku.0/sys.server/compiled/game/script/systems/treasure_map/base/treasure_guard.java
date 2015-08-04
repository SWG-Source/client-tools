package script.systems.treasure_map.base;

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
import script.library.prose;
import script.library.utils;
import script.ai.ai_combat;


public class treasure_guard extends script.base_script
{
	public treasure_guard()
	{
	}
	public static final String RARE_CHAT = "rare_elite_chat";
	
	
	public int OnCreatureDamaged(obj_id self, obj_id attacker, obj_id weapon, int[] damage) throws InterruptedException
	{
		if (!ai_lib.isAttackable(self))
		{
			CustomerServiceLog("treasureMap", "Mob: "+ self + " "+ getName(self)
			+ " is NOT ATTACKABLE to: "+ attacker
			+ " Mob location: "+ getLocation(self) 
			+". [ OnCreatureDamaged() ]");
		}
		
		if (getBehavior(self) != BEHAVIOR_CALM && getBehavior(self) != BEHAVIOR_ATTACK)
		{
			CustomerServiceLog("treasureMap", "Mob: "+ self + " "+ getName(self)
			+ " is being attacked by: "+ attacker
			+ " at location: "+ getLocation(self) 
			+" with a behavior int: "+ getBehavior(self)
			+". [ OnCreatureDamaged() ]");
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo( self, "cleanUpGuard", null, 1000, true );
		
		messageTo( self, "attackThief", null, 1, false );
		
		detachScript( self, "npc.converse.npc_convo");
		detachScript( self, "npc.converse.npc_converse_menu");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDeath(obj_id self, obj_id killer, obj_id corpseId) throws InterruptedException
	{
		CustomerServiceLog("treasureMap", "Mob: "+ self + " "+ getName(self)
		+ " died at the hands of: "+ killer
		+ " at location: "+ getLocation(self) 
		+". [ OnDeath() ]");
		
		if (utils.hasObjVar(self, "glow"))
		{
			messageTo(self, "glow", null, 2, false );
			return SCRIPT_CONTINUE;
		}
		
		obj_id fake_chest = getObjIdObjVar(self, "fake_chest");
		if (isValidId(fake_chest) && exists(fake_chest))
		{
			
			messageTo(fake_chest, "recalculateGuardArray", null, 0, false );
		}
		
		boolean wookieeNPC = isWookiee(self);
		if (wookieeNPC)
		{
			return SCRIPT_CONTINUE;
		}
		
		int combatLevel = getIntObjVar(self, "intCombatDifficulty");
		if (combatLevel < 88)
		{
			return SCRIPT_CONTINUE;
		}
		
		int glow = rand(1,100);
		if (glow == 50 || glow == 100)
		{
			messageTo(self, "glow", null, 2, false );
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		obj_id glowingMob = getObjIdObjVar(self, "glowing_mob");
		if (!isValidId(glowingMob) || !exists(glowingMob))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(glowingMob);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpGuard(obj_id self, dictionary params) throws InterruptedException
	{
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int attackThief(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id player = getObjIdObjVar( self, "player");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!group.isGrouped(player))
		{
			startCombat(self, player);
			return SCRIPT_CONTINUE;
		}
		else
		{
			
			obj_id groupObj = getGroupObject(player);
			if (!isValidId(groupObj) || !exists(groupObj))
			{
				return SCRIPT_CONTINUE;
			}
			
			obj_id[] members = getGroupMemberIds(groupObj);
			if ((members == null) || (members.length == 0))
			{
				return SCRIPT_CONTINUE;
			}
			
			boolean success = getRandomGroupMember(self, player, members);
			
			if (!success && members.length >= 4)
			{
				boolean secondTry = getRandomGroupMember(self, player, members);
				if (!secondTry)
				{
					startCombat(self, player);
					CustomerServiceLog("treasureMap", "TREASURE MAP ERROR: Mob: "+ self + " "+ getName(self)
					+ " is attacking: "+ player + " "+ getName(player)
					+ " by default. This means the function failed to have the mob attach a group member" + " [ attackThief() ]");
				}
			}
			else
			{
				startCombat(self, player);
				
				CustomerServiceLog("treasureMap", "Mob: "+ self + " "+ getName(self)
				+ " is attacking: "+ player + " "+ getName(player)
				+ ". [ attackThief() ]");
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int glow(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id[] primaryKillers = getObjIdArrayObjVar(self, "scenario.primaryKillers");
		if ((primaryKillers == null) || (primaryKillers.length == 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id priKiller = primaryKillers[0];
		if (!isValidId(priKiller) || !exists(priKiller))
		{
			return SCRIPT_CONTINUE;
		}
		
		String mobTemplate = getTemplateName(self);
		if (mobTemplate.equals(""))
		{
			return SCRIPT_CONTINUE;
		}
		
		location glowLocation = getLocation(self);
		if (glowLocation == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id glowingMob = create.object( mobTemplate, glowLocation, 1, true, false );
		if (!isValidId(glowingMob) || !exists(glowingMob))
		{
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(self, "glowing_mob", glowingMob);
		setInvulnerable(glowingMob, true);
		setState(glowingMob, STATE_GLOWING_JEDI, true);
		setName(glowingMob, "Force Apparition");
		
		faceTo(glowingMob, priKiller);
		
		chat.chat(glowingMob, chat.CHAT_SHOUT, chat.MOOD_ANGRY, new string_id("treasure_map/treasure_map", RARE_CHAT));
		
		messageTo( self, "expression_fist", null, 1, true );
		messageTo( self, "expression_disappear", null, 6, true );
		
		messageTo( self, "cleanUpGlowie", null, 10, true );
		return SCRIPT_CONTINUE;
	}
	
	
	public int expression_fist(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id glowingMob = getObjIdObjVar(self, "glowing_mob");
		if (!isValidId(glowingMob) || !exists(glowingMob))
		{
			return SCRIPT_CONTINUE;
		}
		doAnimationAction (glowingMob, anims.PLAYER_POUND_FIST_PALM);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int expression_disappear(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id glowingMob = getObjIdObjVar(self, "glowing_mob");
		if (!isValidId(glowingMob) || !exists(glowingMob))
		{
			return SCRIPT_CONTINUE;
		}
		
		doAnimationAction (glowingMob, anims.PLAYER_FORCE_ILLUSION);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpGlowie(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id glowingMob = getObjIdObjVar(self, "glowing_mob");
		if (!isValidId(glowingMob) || !exists(glowingMob))
		{
			return SCRIPT_CONTINUE;
		}
		
		destroyObject(glowingMob);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean getRandomGroupMember(obj_id self, obj_id player, obj_id[] members) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if ((members == null) || (members.length == 0))
		{
			return false;
		}
		
		int j = rand(0, members.length-1);
		if (getDistance(getLocation(self), getLocation(members[j])) < 100)
		{
			startCombat(self, members[j]);
			
			CustomerServiceLog("treasureMap", "Mob: "+ self + " "+ getName(self)
			+ " is attacking: "+ members[j] + " "+ getName(members[j])
			+ " located: "+ getLocation(members[j]) 
			+". [ getRandomGroupMember() ]");
			
			return true;
		}
		return false;
	}
	
	
	public boolean isWookiee(obj_id playerOrMob) throws InterruptedException
	{
		int species = getSpecies(playerOrMob);
		if (species == SPECIES_WOOKIEE)
		{
			return true;
		}
		
		return false;
	}
	
	
	public int barkAttack(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id guard = self;
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean wookieeNPC = isWookiee(guard);
		
		if (!wookieeNPC)
		{
			String type = params.getString("type");
			chat.chat(self, chat.CHAT_SHOUT, chat.MOOD_ANGRY, new string_id("treasure_map/treasure_map", "bark_"+type));
		}
		else
		{
			chat.chat(self, chat.CHAT_SHOUT, chat.MOOD_ANGRY, new string_id("treasure_map/treasure_map", "bark_wookiee"));
		}
		
		return SCRIPT_CONTINUE;
	}
}
