package script.systems.gcw;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import java.util.HashSet;
import java.util.Iterator;

import script.library.ai_lib;
import script.library.armor;
import script.library.buff;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.resource;
import script.library.restuss_event;
import script.library.static_item;
import script.library.trial;
import script.library.utils;


public class gcw_defensive_general_boss extends script.base_script
{
	public gcw_defensive_general_boss()
	{
	}
	public static final boolean LOGGING_ON = false;
	public static final int MAX_HIT_POINTS = 1000000;
	public static final int MAX_GENERAL_BUFF_STACK = 100;
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		blog("OnAttach init");
		
		setAttrib(self, HEALTH, MAX_HIT_POINTS);
		setMaxAttrib(self, HEALTH, MAX_HIT_POINTS);
		
		blog("OnAttach maxHitPoints: "+ getMaxAttrib(self, HEALTH));
		
		messageTo(self, "checkConstructionPhase", null, 20.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnHateTargetAdded(obj_id self, obj_id target) throws InterruptedException
	{
		Vector allPlayersHatedList = new Vector();
		allPlayersHatedList.setSize(0);
		
		if (utils.hasScriptVar(self, "allPlayersEverHated"))
		{
			allPlayersHatedList = utils.getResizeableObjIdArrayScriptVar(self, "allPlayersEverHated");
		}
		
		utils.addElement(allPlayersHatedList, target);
		utils.setScriptVar(self, "allPlayersEverHated", allPlayersHatedList);
		trial.addNonInstanceFactionParticipant(target, self);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		
		obj_id[] attackerList = utils.getObjIdBatchScriptVar(self, "creditForKills.attackerList.attackers");
		
		Vector allPlayersHatedList = utils.getResizeableObjIdArrayScriptVar(self, "allPlayersEverHated");
		if ((allPlayersHatedList == null || allPlayersHatedList.size() <= 0) && (attackerList == null || attackerList.length <= 0))
		{
			return SCRIPT_CONTINUE;
		}
		
		HashSet allParticipants = new HashSet();
		
		if (allPlayersHatedList != null && allPlayersHatedList.size() > 0)
		{
			for (int i = 0; i < allPlayersHatedList.size(); ++i)
			{
				testAbortScript();
				allParticipants.add(((obj_id)(allPlayersHatedList.get(i))));
			}
		}
		
		if (attackerList != null && attackerList.length > 0)
		{
			for (int i = 0; i < attackerList.length; ++i)
			{
				testAbortScript();
				allParticipants.add(attackerList[i]);
			}
		}
		
		obj_id[] participantList = new obj_id[allParticipants.size()];
		
		Iterator participantIterator = allParticipants.iterator();
		int j = 0;
		while (participantIterator.hasNext())
		{
			testAbortScript();
			participantList[j] = (obj_id)participantIterator.next();
			j++;
		}
		
		if (participantList == null || participantList.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "factionFlag"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int factionFlag = getIntObjVar(self, "factionFlag");
		if (factionFlag != factions.FACTION_FLAG_IMPERIAL && factionFlag != factions.FACTION_FLAG_REBEL)
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("calculateGeneralHealthAndAbilities factionFlag: "+ factionFlag);
		
		String token = "";
		
		if (factionFlag == factions.FACTION_FLAG_IMPERIAL)
		{
			token = gcw.GCW_REBEL_TOKEN;
		}
		else if (factionFlag == factions.FACTION_FLAG_REBEL)
		{
			token = gcw.GCW_IMPERIAL_TOKEN;
		}
		
		if (token == null || token.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String information = getName(self);
		
		for (int i = 0; i < participantList.length; i++)
		{
			testAbortScript();
			
			if (!isPlayer(participantList[i]))
			{
				continue;
			}
			
			if (utils.getDistance2D(self, participantList[i]) > 150.0)
			{
				continue;
			}
			
			int damage = 0;
			int damageBase = 10000;
			int actualDmg = utils.getIntScriptVar(self, "creditForKills.attackerList."+ participantList[i] + ".damage");
			if (actualDmg > damageBase)
			{
				damage = actualDmg;
			}
			else
			{
				damage = damageBase;
			}
			
			if (damage <= 0)
			{
				continue;
			}
			
			gcw.grantModifiedGcwPoints(self, participantList[i], gcw.GCW_GENERAL_GCW_BASE_AMOUNT, false, gcw.GCW_POINT_TYPE_GROUND_PVE, information);
			
			obj_id inv = getObjectInSlot(participantList[i], "inventory");
			if (!isIdValid(inv))
			{
				continue;
			}
			
			obj_id tokenObject = static_item.createNewItemFunction(token, inv);
			if (!isValidId(tokenObject))
			{
				continue;
			}
			
			int count = damage / gcw.GCW_GENERAL_TOKEN_BONUS_DIVISOR;
			
			if (!incrementCount(tokenObject, count))
			{
				continue;
			}
			groundquests.sendPlacedMoreThanOneInInventorySystemMessage(participantList[i], tokenObject, count);
			
		}
		return SCRIPT_CONTINUE;
		
	}
	
	
	public int checkConstructionPhase(obj_id self, dictionary params) throws InterruptedException
	{
		blog("checkConstructionPhase init");
		
		if (hasObjVar(self, "offense") && getIntObjVar(self, "offense") == 1)
		{
			blog("OnAttach offensive general found, removing script.");
			detachScript(self, "systems.gcw.gcw_defensive_general_boss");
			
			return SCRIPT_CONTINUE;
		}
		
		obj_id parent = trial.getParent(self);
		
		if (!isValidId(parent) || !exists(parent))
		{
			blog("checkConstructionPhase parnet invalid");
			return SCRIPT_CONTINUE;
		}
		
		blog("checkConstructionPhase parent: "+parent);
		
		if (!utils.hasScriptVar(parent, "gcw.constructionEnded") && !hasObjVar(self, "initialKitCount"))
		{
			
			obj_id[] kitsAndPylons = trial.getObjectsInInstanceBySpawnId(parent, "defense_kit");
			blog("checkConstructionPhase kitsAndPylons: "+ kitsAndPylons.length);
			
			obj_id[] patrolAndPylons = trial.getObjectsInInstanceBySpawnId(parent, "defense_patrol");
			
			int combinedKitAmt = 0;
			if (kitsAndPylons != null && kitsAndPylons.length > 0)
			{
				blog("checkConstructionPhase initial kitsAndPylons.length: "+ kitsAndPylons.length);
				combinedKitAmt += kitsAndPylons.length;
			}
			if (patrolAndPylons != null && patrolAndPylons.length > 0)
			{
				blog("checkConstructionPhase initial patrolAndPylons.length: "+ patrolAndPylons.length);
				combinedKitAmt += patrolAndPylons.length;
			}
			
			setObjVar(self, "initialKitCount", combinedKitAmt / 2);
			blog("checkConstructionPhase initial kit count: "+ combinedKitAmt + " / 2 = "+ combinedKitAmt / 2);
		}
		else if (utils.hasScriptVar(parent, "gcw.constructionEnded"))
		{
			if (!hasObjVar(self, "initialKitCount"))
			{
				blog("checkConstructionPhase initialKitCount not found, default it to 0");
				setObjVar(self, "initialKitCount", 0);
			}
			
			blog("checkConstructionPhase removing invulnerable from general");
			
			messageTo(self, "calculateGeneralHealthAndAbilities", null, 10.0f, false);
			
			return SCRIPT_CONTINUE;
		}
		blog("checkConstructionPhase fell through, recalling this messageHandler in 10");
		
		messageTo(self, "checkConstructionPhase", null, 10.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int calculateGeneralHealthAndAbilities(obj_id self, dictionary params) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self) || ai_lib.isDead(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("calculateGeneralHealthAndAbilities init");
		
		obj_id parent = trial.getParent(self);
		
		if (!isValidId(parent) || !exists(parent))
		{
			blog("calculateGeneralHealthAndAbilities parent invalid");
			return SCRIPT_CONTINUE;
		}
		
		blog("calculateGeneralHealthAndAbilities parent: "+parent);
		
		if (!utils.hasScriptVar(parent, "gcw.constructionEnded"))
		{
			blog("calculateGeneralHealthAndAbilities parent is not in the correct phase");
			return SCRIPT_CONTINUE;
		}
		blog("calculateGeneralHealthAndAbilities parent is not in construction phase");
		
		if (!hasObjVar(self, "initialKitCount"))
		{
			blog("calculateGeneralHealthAndAbilities initialKitCount not found");
			return SCRIPT_CONTINUE;
		}
		
		int initialKitCount = getIntObjVar(self, "initialKitCount");
		
		if (initialKitCount < 0)
		{
			blog("calculateGeneralHealthAndAbilities initialKitCount is 0 or less");
			setObjVar(self, "initialKitCount", 0);
		}
		
		blog("calculateGeneralHealthAndAbilities initialKitCount: "+initialKitCount);
		
		int combinedAmt = 0;
		
		obj_id[] patrols = trial.getObjectsInInstanceBySpawnId(parent, "defense_quest_asset");
		if (patrols != null && patrols.length > 0)
		{
			blog("calculateGeneralHealthAndAbilities PATROLS FOUND: "+patrols.length);
			combinedAmt += patrols.length;
		}
		else
		{
			blog("calculateGeneralHealthAndAbilities PATROLS NOT FOUND");
		}
		
		obj_id[] kits = trial.getObjectsInInstanceBySpawnId(parent, "defense_kit");
		if (kits == null || kits.length <= 0)
		{
			blog("calculateGeneralHealthAndAbilities KITS NOT FOUND");
			return SCRIPT_CONTINUE;
		}
		blog("calculateGeneralHealthAndAbilities KITS FOUND: "+kits.length+" This number includes the kit as well as the crafted object.");
		combinedAmt += kits.length;
		
		blog("calculateGeneralHealthAndAbilities kits: "+ kits.length);
		
		if (!hasObjVar(self, "alertedPlayersOfMyDamage"))
		{
			blog("general_announce general's health being checked to see if announcement shoudl be sent: ");
			
			int maxHealth = getMaxAttrib(self, HEALTH);
			int lowHealth = maxHealth / 2;
			blog("general_announce general's health should not be lower than: "+lowHealth);
			
			int curHealth = getAttrib(self, HEALTH);
			blog("general_announce general's current health: "+curHealth);
			
			if (curHealth < lowHealth)
			{
				blog("general_announce HALP the general~!");
				messageTo(parent, "defenseGeneralUnderSustainedAttack", null, 0.0f, false);
				setObjVar(self, "alertedPlayersOfMyDamage", 1);
			}
		}
		
		if (!hasObjVar(self, "defenseObjectCount") && initialKitCount > 0)
		{
			blog("calculateGeneralHealthAndAbilities setting up defenseObjectCount FOR THE FIRST TIME");
			
			int defenseObjectCount = combinedAmt - initialKitCount;
			if (defenseObjectCount < 0)
			{
				blog("calculateGeneralHealthAndAbilities defenseObjectCount is less than zero!");
				setObjVar(self, "initialKitCount", 0);
				messageTo(self, "calculateGeneralHealthAndAbilities", null, 10.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			blog("calculateGeneralHealthAndAbilities defenseObjectCount --- THIS IS THE TOTAL NON PATROL DEFENSES CRAFTED: "+defenseObjectCount);
			if (patrols != null && patrols.length > 0)
			{
				blog("calculateGeneralHealthAndAbilities defenseObjectCount --- THIS IS THE TOTAL PATROL DEFENSES CRAFTED: "+patrols.length);
				defenseObjectCount += patrols.length;
				
				blog("calculateGeneralHealthAndAbilities defenseObjectCount --- TOTAL OF BOTH: "+defenseObjectCount);
			}
			
			blog("calculateGeneralHealthAndAbilities defenseObjectCount --- THIS IS THE TOTAL POSSIBLE DEFENSES: "+initialKitCount);
			blog("calculateGeneralHealthAndAbilities defenseObjectCount --- THIS IS THE AMT BEING SET AS INITIAL DEFENSE COUNT: "+initialKitCount);
			
			setObjVar(self, "defenseObjectCount", defenseObjectCount);
			
			messageTo(self, "calculateGeneralHealthAndAbilities", null, 0.25f, false);
			
			return SCRIPT_CONTINUE;
		}
		else if (initialKitCount > 0)
		{
			blog("calculateGeneralHealthAndAbilities checking for a change in the defensive count");
			
			float defenseObjectCount = (float)getIntObjVar(self, "defenseObjectCount");
			if (defenseObjectCount < 0)
			{
				blog("calculateGeneralHealthAndAbilities defenseObjectCount is less than 0!");
				setObjVar(self, "initialKitCount", 0);
				messageTo(self, "calculateGeneralHealthAndAbilities", null, 10.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			blog("calculateGeneralHealthAndAbilities This is what the defense count was when the fight first started: "+defenseObjectCount);
			
			float currentDefenseCount = combinedAmt - initialKitCount;
			if (currentDefenseCount < 0)
			{
				blog("calculateGeneralHealthAndAbilities current kit count is less than 0");
				setObjVar(self, "initialKitCount", 0);
				messageTo(self, "calculateGeneralHealthAndAbilities", null, 10.0f, false);
				return SCRIPT_CONTINUE;
			}
			
			blog("calculateGeneralHealthAndAbilities This is what the defense count is at this very moment: "+currentDefenseCount);
			if (patrols != null && patrols.length > 0)
			{
				blog("calculateGeneralHealthAndAbilities defenseObjectCount --- THIS IS THE TOTAL PATROL DEFENSES CRAFTED: "+patrols.length);
				currentDefenseCount += patrols.length;
				
				blog("calculateGeneralHealthAndAbilities defenseObjectCount --- TOTAL OF BOTH: "+currentDefenseCount);
			}
			
			if ((currentDefenseCount < defenseObjectCount || (!buff.hasBuff(self, gcw.BUFF_GENERAL_RESIST_STACK_IMP) && !buff.hasBuff(self, gcw.BUFF_GENERAL_RESIST_STACK_REB))))
			{
				blog("calculateGeneralHealthAndAbilities the currentDefenseCount has changed and is lower than the OG defenseObjectCount");
				
				if (!hasObjVar(self, "factionFlag"))
				{
					return SCRIPT_CONTINUE;
				}
				
				blog("calculateGeneralHealthAndAbilities has factionFlag objvar");
				
				int factionFlag = getIntObjVar(self, "factionFlag");
				
				if (factionFlag != factions.FACTION_FLAG_IMPERIAL && factionFlag != factions.FACTION_FLAG_REBEL)
				{
					return SCRIPT_CONTINUE;
				}
				
				blog("calculateGeneralHealthAndAbilities factionFlag: "+ factionFlag);
				
				String gcwBuffName = "";
				
				if (factionFlag == factions.FACTION_FLAG_IMPERIAL)
				{
					blog("calculateGeneralHealthAndAbilities faction is imp");
					gcwBuffName = gcw.BUFF_GENERAL_RESIST_STACK_IMP;
				}
				else if (factionFlag == factions.FACTION_FLAG_REBEL)
				{
					blog("calculateGeneralHealthAndAbilities faction is reb");
					gcwBuffName = gcw.BUFF_GENERAL_RESIST_STACK_REB;
				}
				
				if (gcwBuffName == null || gcwBuffName.length() <= 0)
				{
					return SCRIPT_CONTINUE;
				}
				
				float percent = currentDefenseCount / defenseObjectCount;
				blog("calculateGeneralHealthAndAbilities Percent: "+percent);
				
				int mobGeneralArmor = (int)getFloatObjVar(self, armor.OBJVAR_ARMOR_BASE + "."+ armor.OBJVAR_GENERAL_PROTECTION);
				blog("calculateGeneralHealthAndAbilities mobGeneralArmor: "+mobGeneralArmor);
				
				float newMobGeneralArmor = mobGeneralArmor * percent;
				blog("calculateGeneralHealthAndAbilities newMobGeneralArmor: "+newMobGeneralArmor);
				
				int newstackAmt = (int)(MAX_GENERAL_BUFF_STACK * percent);
				blog("calculateGeneralHealthAndAbilities newstackAmt: "+newstackAmt);
				
				buff.removeBuff(self, gcwBuffName);
				blog("calculateGeneralHealthAndAbilities making buff stack: "+newstackAmt);
				buff.applyBuffWithStackCount(self, gcwBuffName, newstackAmt);
				
				utils.setScriptVar(self, armor.SCRIPTVAR_CACHED_GENERAL_PROTECTION, newMobGeneralArmor);
			}
		}
		
		messageTo(self, "calculateGeneralHealthAndAbilities", null, 10.0f, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void blog(String text) throws InterruptedException
	{
		if (LOGGING_ON)
		{
			LOG("gcw_defensive_general_boss", text);
		}
	}
	
}
