package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.attrib;
import script.library.chat;
import script.library.collection;
import script.library.conversation;
import script.library.create;
import script.library.groundquests;
import script.library.spawning;
import script.library.utils;


public class camp_survivor extends script.base_script
{
	public camp_survivor()
	{
	}
	String c_stringFile = "conversation/camp_survivor";
	
	
	public boolean camp_survivor_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean camp_survivor_condition_isOwner(obj_id player, obj_id npc) throws InterruptedException
	{
		return hasObjVar(npc, "myEscort") && player == getObjIdObjVar(npc, "myEscort");
	}
	
	
	public boolean camp_survivor_condition_isCloseToCamp(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "objParent"))
		{
			return false;
		}
		obj_id objParent = getObjIdObjVar(npc, "objParent");
		if (!isValidId(objParent) || !exists(objParent))
		{
			return false;
		}
		if (utils.getDistance2D(getLocation(objParent), getLocation(npc)) <= 70)
		{
			return true;
		}
		
		return false;
	}
	
	
	public boolean camp_survivor_condition_isUnclaimedSpawner(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (!hasObjVar(npc, "myEscort"));
	}
	
	
	public void camp_survivor_action_startDestroyTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		
		messageTo(npc, "cleanUpWhileConversing", null, 90, false);
	}
	
	
	public void camp_survivor_action_claimSpawner(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!hasObjVar(npc, "myEscort"))
		{
			setObjVar(npc, "myEscort", player);
		}
		
		if (getObjIdObjVar(npc, "myEscort") == player)
		{
			String questName = getStringObjVar(npc, "questName");
			if (questName != null && questName.length() > 0)
			{
				if (groundquests.isQuestActiveOrComplete(player, questName))
				{
					groundquests.clearQuest(player, questName);
				}
				
				groundquests.grantQuest(player, questName);
			}
			
			obj_id parentObject = getObjIdObjVar(npc, "objParent");
			if (!isValidId(parentObject) || !exists(parentObject))
			{
				CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.spawnSurvivor() Master Object does not exist");
				return;
			}
			
			setInvulnerable(npc, false);
			pathTo(npc,getLocation(parentObject));
			
			setObjVar(npc, "ignoreEarlyDestroy", 1);
		}
		messageTo(npc, "spawnEnemies", null, 0, false);
		messageTo(npc, "checkOwnerValidity", null, 10, false);
	}
	
	
	public int camp_survivor_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_96"))
		{
			
			if (camp_survivor_condition__defaultCondition (player, npc))
			{
				camp_survivor_action_claimSpawner (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_97");
				utils.removeScriptVar (player, "conversation.camp_survivor.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	public static final string_id SID_YOU_WENT_TOO_FAR = new string_id("theme_park/outbreak/outbreak","delivery_you_went_too_far");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.camp_survivor");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		setCondition (self, CONDITION_INTERESTING);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnMovePathComplete(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id parentObject = getObjIdObjVar(self, "objParent");
		if (!isValidId(parentObject) || !exists(parentObject))
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.OnMovingPathComplete() Master Object does not exist");
			return SCRIPT_CONTINUE;
		}
		pathTo(self,getLocation(parentObject));
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info menuInfo) throws InterruptedException
	{
		int menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data menuInfoData = menuInfo.getMenuItemById (menu);
		menuInfoData.setServerNotify (false);
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.camp_survivor");
		
		if (hasObjVar(self, "myEscort"))
		{
			obj_id player = getObjIdObjVar(self, "myEscort");
			if (isValidId(player) && exists(player))
			{
				String questName = getStringObjVar(self, "questName");
				if (questName != null && questName.length() > 0)
				{
					String failMessage = getStringObjVar(self,"failMessage");
					if (failMessage != null && failMessage.length() > 0)
					{
						if (groundquests.isQuestActive(player, questName))
						{
							groundquests.sendSignal(player, failMessage);
						}
					}
				}
			}
		}
		
		obj_id parent = getObjIdObjVar(self,"mySpawner");
		if (isValidId(parent) && exists(parent))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.cleanUpCampNpcAndParent() Destroying parent: "+parent);
			destroyObject(parent);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpWhileConversing(obj_id self, dictionary params) throws InterruptedException
	{
		if (hasObjVar(self, "ignoreEarlyDestroy"))
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "cleanUpCampNpcAndParent", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int cleanUpCampNpcAndParent(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "survivor_pathing.cleanUpCampNpcAndParent() Initalized Message Handler.");
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (hasObjVar(self, "myEscort"))
		{
			obj_id player = getObjIdObjVar(self, "myEscort");
			if (isValidId(player) && exists(player))
			{
				String questName = getStringObjVar(self, "questName");
				if (questName != null && questName.length() > 0)
				{
					String creatureType = getStringObjVar(self,"creature_type");
					if (creatureType != null && creatureType.length() > 0)
					{
						if (groundquests.isQuestActive(player, questName))
						{
							groundquests.sendSignal(player, creatureType);
						}
						if (!hasCompletedCollectionSlot(player, creatureType + "_counter"))
						{
							modifyCollectionSlotValue(player, creatureType + "_counter", 1);
						}
					}
				}
			}
		}
		
		obj_id parent = getObjIdObjVar(self,"mySpawner");
		if (isValidId(parent) && exists(parent))
		{
			CustomerServiceLog("outbreak_themepark", "survivor_pathing.cleanUpCampNpcAndParent() Destroying parent: "+parent);
			destroyObject(parent);
		}
		CustomerServiceLog("outbreak_themepark", "survivor_pathing.cleanUpCampNpcAndParent() Destroying self.");
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnEnemies(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("outbreak_themepark", "survivor_pathing.spawnEnemies() Initalized Message Handler.");
		if (!isValidId(self) || !exists(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(self, "myEscort"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(self, "myEscort");
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		int combatLevel = getLevel(player);
		if (combatLevel < 0 || combatLevel > 90)
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor.spawnEnemies() Player "+player+" has a level that is invalid. Player level is: "+combatLevel+". Aborting the camp defense.");
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; i < 2; i++)
		{
			testAbortScript();
			
			obj_id mob = create.object("outbreak_afflicted_defense", spawning.getRandomLocationAtDistance(getLocation(self), 10.0f));
			if (!isValidId(mob) || !exists(mob))
			{
				CustomerServiceLog("outbreak_themepark", "camp_survivor.spawnEnemies() Mob: "+mob+" could not be created!");
				return SCRIPT_CONTINUE;
			}
			
			CustomerServiceLog("outbreak_themepark", "camp_survivor.spawnEnemies() Mob level is set to match player level");
			
			setObjVar(mob, create.INITIALIZE_CREATURE_DO_NOT_SCALE_OBJVAR, 1);
			
			dictionary creatureDict = utils.dataTableGetRow("datatables/mob/creatures.iff", "outbreak_afflicted_defense");
			if (creatureDict != null)
			{
				create.initializeCreature (mob, "outbreak_afflicted_defense", creatureDict, combatLevel);
			}
			CustomerServiceLog("outbreak_themepark", "camp_survivor.spawnEnemies() Mob level is set to match player level.");
			
			setInvulnerable(mob, false);
			clearCondition(mob, CONDITION_CONVERSABLE);
			setMovementRun(mob);
			setBaseRunSpeed(mob, (getBaseRunSpeed(self)-8));
			setAttributeAttained(mob, attrib.OUTBREAK_AFFLICTED);
			setObjVar(mob, "ow/sener", player);
			
			attachScript(mob, "theme_park.outbreak.dynamic_enemy");
			startCombat(mob, self);
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkOwnerValidity(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "myEscort"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = getObjIdObjVar(self, "myEscort");
		if (!isValidId(owner) || !exists(owner))
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor.checkOwnerValidity() Mob: "+self+" FAILED TO FIND OWNER OBJVAR.");
			return SCRIPT_CONTINUE;
		}
		
		location ownerLoc = getLocation(owner);
		float distanceCheck = utils.getDistance2D(getLocation(self), ownerLoc);
		if (distanceCheck > 20)
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor.checkOwnerValidity() Mob: "+self+" has been left alone and is destroying self, failing quest for player. The distance between player and NPC was: "+distanceCheck);
			
			sendSystemMessage(owner, SID_YOU_WENT_TOO_FAR);
			if (groundquests.isQuestActive(owner, "camp_alpha_survivor_rescue"))
			{
				groundquests.sendSignal(owner, "outbreak_camp_alpha_survivor_failed");
			}
			else if (groundquests.isQuestActive(owner, "camp_beta_survivor_rescue"))
			{
				groundquests.sendSignal(owner, "outbreak_camp_beta_survivor_failed");
			}
			else if (groundquests.isQuestActive(owner, "camp_gamma_survivor_rescue"))
			{
				groundquests.sendSignal(owner, "outbreak_camp_gamma_survivor_failed");
			}
			else
			{
				groundquests.sendSignal(owner, "outbreak_camp_delta_survivor_failed");
			}
			
		}
		
		obj_id parentObject = getObjIdObjVar(self, "objParent");
		if (!isValidId(parentObject) || !exists(parentObject))
		{
			CustomerServiceLog("outbreak_themepark", "camp_survivor_spawning.OnMovingPathComplete() Master Object does not exist");
			return SCRIPT_CONTINUE;
		}
		pathTo(self,getLocation(parentObject));
		
		messageTo(self, "checkOwnerValidity", null, 10, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean npcStartConversation(obj_id player, obj_id npc, String convoName, string_id greetingId, prose_package greetingProse, string_id[] responses) throws InterruptedException
	{
		Object[] objects = new Object[responses.length];
		System.arraycopy(responses, 0, objects, 0, responses.length);
		return npcStartConversation(player, npc, convoName, greetingId, greetingProse, objects);
	}
	
	
	public int OnStartNpcConversation(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id npc = self;
		
		if (ai_lib.isInCombat (npc) || ai_lib.isInCombat (player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (camp_survivor_condition_isCloseToCamp (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_9");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (camp_survivor_condition_isUnclaimedSpawner (player, npc))
		{
			camp_survivor_action_startDestroyTimer (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_73");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (camp_survivor_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_96");
				}
				
				utils.setScriptVar (player, "conversation.camp_survivor.branchId", 2);
				
				npcStartConversation (player, npc, "camp_survivor", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (camp_survivor_condition_isOwner (player, npc))
		{
			camp_survivor_action_claimSpawner (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_7");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (camp_survivor_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_60");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("camp_survivor"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.camp_survivor.branchId");
		
		if (branchId == 2 && camp_survivor_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.camp_survivor.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
