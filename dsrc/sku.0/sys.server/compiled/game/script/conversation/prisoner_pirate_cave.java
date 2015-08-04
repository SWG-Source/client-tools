package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.chat;
import script.library.conversation;
import script.library.create;
import script.library.groundquests;
import script.library.prose;
import script.library.utils;


public class prisoner_pirate_cave extends script.base_script
{
	public prisoner_pirate_cave()
	{
	}
	String c_stringFile = "conversation/prisoner_pirate_cave";
	
	
	public boolean prisoner_pirate_cave_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean prisoner_pirate_cave_condition_hasRescueTask(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "findPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "findPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "findPrisoner");
	}
	
	
	public boolean prisoner_pirate_cave_condition_isTraveling(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(npc, "traveling") && getIntObjVar(npc, "traveling") == 1 && groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "isEscortingPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "isEscortingPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "isEscortingPrisoner");
	}
	
	
	public boolean prisoner_pirate_cave_condition_hasArrived(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "waitingForShuttle") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "waitingForShuttle") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "waitingForShuttle");
	}
	
	
	public void prisoner_pirate_cave_action_startEscort(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasFoundPrisoner");
		ai_lib.aiFollow(npc, player, 1.0f, 3.0f);
		
		setMovementRun(npc);
		setBaseRunSpeed(npc, (getBaseRunSpeed(npc)-8));
		setObjVar(npc, "traveling", 1);
		setObjVar(npc, "playerOwner", player);
		setInvulnerable(npc, false);
		setMaster(npc, player);
		messageTo(npc, "checkArrivalLoopHandler", null, 5, false);
		LOG("prisoner_error_log","prisoner being rescued. Sending message to checkArrivalLoopHandler in 60 seconds.");
	}
	
	
	public void prisoner_pirate_cave_action_walkFaster(obj_id player, obj_id npc) throws InterruptedException
	{
		setMovementRun(npc);
		setBaseRunSpeed(npc, (getBaseRunSpeed(npc)-8));
		ai_lib.aiFollow(npc, player, 1.0f, 3.0f);
	}
	
	
	public int prisoner_pirate_cave_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_8"))
		{
			
			if (prisoner_pirate_cave_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_10");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (prisoner_pirate_cave_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_12");
					}
					
					utils.setScriptVar (player, "conversation.prisoner_pirate_cave.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.prisoner_pirate_cave.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int prisoner_pirate_cave_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_12"))
		{
			
			if (prisoner_pirate_cave_condition__defaultCondition (player, npc))
			{
				prisoner_pirate_cave_action_startEscort (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_14");
				utils.removeScriptVar (player, "conversation.prisoner_pirate_cave.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	public static final string_id SID_TOO_FAR = new string_id("spam","too_far_from_prisoner");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.prisoner_pirate_cave");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		
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
		failRescueQuest(self);
		tellMomIDied(self);
		setObjVar(self, "toldMom", 1);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (hasObjVar(self, "toldMom"))
		{
			return SCRIPT_CONTINUE;
		}
		
		tellMomIDied(self);
		setObjVar(self, "toldMom", 1);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUpdatePrisonerRescueQuest(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!isValidId(self))
		{
			CustomerServiceLog("nyms_themepark","handleUpdatePrisonerRescueQuest - NPC is not a valid OID.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(self, "playerOwner");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			CustomerServiceLog("nyms_themepark","handleUpdatePrisonerRescueQuest - could not get valid player OID.");
			return SCRIPT_CONTINUE;
		}
		
		String signal = getStringObjVar(self, "signal");
		if (signal == null || signal.length() <= 0)
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			CustomerServiceLog("nyms_themepark","handleUpdatePrisonerRescueQuest - could not get valid signal to update quest.");
			
			return SCRIPT_CONTINUE;
		}
		
		if (!hasCompletedCollectionSlot(player, "icon_nyms_rescue_prisoner"))
		{
			modifyCollectionSlotValue(player, "icon_nyms_rescue_prisoner", 1);
		}
		
		groundquests.sendSignal(player, signal);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		tellMomIDied(self);
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkArrivalLoopHandler(obj_id self, dictionary params) throws InterruptedException
	{
		setMovementRun(self);
		setBaseRunSpeed(self, getBaseRunSpeed(self));
		
		obj_id player = getObjIdObjVar(self, "playerOwner");
		if (!isValidId(player))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - could not get valid player OID.");
			messageTo(self, "handleDestroySelf", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		if (isDead(player))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - Player Died.");
			failRescueQuest(self);
			return SCRIPT_CONTINUE;
		}
		
		if ((!groundquests.isQuestActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection") && !groundquests.isQuestActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01") && !groundquests.isQuestActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02")))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - player no longer had required quest active!");
			failRescueQuest(self);
			return SCRIPT_CONTINUE;
		}
		else if (((groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "waitingForShuttle") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "stuvCom")) || (groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "waitingForShuttle") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "stuvCom")) || (groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "waitingForShuttle") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "stuvCom"))))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - Calling SHuttle!");
			callShuttle(self, player);
			return SCRIPT_CONTINUE;
		}
		else if ((groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_collection", "findPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_01", "findPrisoner") || groundquests.isTaskActive(player, "u16_nym_themepark_pirate_prisoner_rescue_retry_02", "findPrisoner")))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - could not get valid player OID.");
			messageTo(self, "handleDestroySelf", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		if (getDistance(self, player) > 10)
		{
			if (utils.hasScriptVar(player, "timer"))
			{
				int timeStarted = utils.getIntScriptVar(player, "timer");
				
				if (getGameTime() - timeStarted > 10)
				{
					CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - player out of bounds too long, failing quest.");
					failRescueQuest(self);
					return SCRIPT_CONTINUE;
				}
				
				int count = 10 - (getGameTime() - timeStarted);
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_TOO_FAR);
				pp = prose.setDI(pp, count);
				sendQuestSystemMessage(player, pp);
			}
			else
			{
				
				int count = 10;
				prose_package pp = new prose_package();
				pp = prose.setStringId(pp, SID_TOO_FAR);
				pp = prose.setDI(pp, count);
				sendQuestSystemMessage(player, pp);
				utils.setScriptVar(player, "timer", getGameTime());
			}
		}
		else
		{
			utils.removeScriptVar(player, "timer");
		}
		
		ai_lib.aiFollow(self, player, 1.0f, 3.0f);
		messageTo(self, "checkArrivalLoopHandler", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean callShuttle(obj_id self, obj_id player) throws InterruptedException
	{
		LOG("prisoner_error_log","callShuttle - Shuttle Called");
		if (!isValidId(player))
		{
			return false;
		}
		if (!isValidId(self))
		{
			return false;
		}
		
		ai_lib.aiStopFollowing(self);
		removeObjVar(self, "traveling");
		setInvulnerable(self, true);
		
		location[] prisonerPath = new location[1];
		prisonerPath[0] = new location(-3048, 37, -719, "lok", null);
		setYaw(self, -95.0f);
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, prisonerPath);
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
		patrolOnce(self, prisonerPath, 0);
		
		location landLocation = new location(-3067, 34, -727, "lok", null);
		obj_id shuttle = create.object("object/creature/npc/theme_park/player_shuttle.iff", landLocation);
		if (!isIdValid(shuttle))
		{
			return false;
		}
		
		setYaw(shuttle, 75);
		detachScript(shuttle, "ai.ai");
		detachScript(shuttle, "ai.creature_combat");
		detachScript(shuttle, "skeleton.humanoid");
		detachScript(shuttle, "systems.combat.combat_actions");
		detachScript(shuttle, "systems.combat.combat_actions_creature");
		detachScript(shuttle, "systems.combat.credit_for_kills");
		stop(shuttle);
		
		utils.setScriptVar(shuttle, "prisoner", self);
		utils.setScriptVar(shuttle, "playerOwner", player);
		setPosture(shuttle, POSTURE_PRONE);
		attachScript(shuttle, "systems.spawning.dropship.player_shuttle");
		
		messageTo(shuttle, "landShuttle", null, 0, false);
		messageTo(shuttle, "takeOff", null, 37, false);
		messageTo(self, "handleUpdatePrisonerRescueQuest", null, 30, false);
		messageTo(self, "handleDestroySelf", null, 32, false);
		return true;
	}
	
	
	public boolean tellMomIDied(obj_id self) throws InterruptedException
	{
		if (!isValidId(self))
		{
			return false;
		}
		
		obj_id mom = getObjIdObjVar (self, "mom");
		if (!isValidId(mom))
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve Cave OID to respawn the NPC.");
			return false;
		}
		
		int spawnNum = getIntObjVar (self, "spawn_number");
		if (spawnNum <= 0)
		{
			CustomerServiceLog("bad_spawner_data", "droid_cave_died.OnIncapacitated() Nym Pirate Cave Spawner Failed. Could not retrieve NPC spawnNum from NPC Object.");
			return false;
		}
		
		dictionary info = new dictionary();
		info.put ("spawnNumber", spawnNum );
		info.put ("spawnMob", self );
		
		messageTo(mom, "tellingMomIDied", info, 5, false );
		return true;
	}
	
	
	public boolean failRescueQuest(obj_id self) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "playerOwner");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return false;
		}
		
		String failSignal = getStringObjVar(self, "failSignal");
		if (failSignal == null || failSignal.length() <= 0)
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return false;
		}
		
		groundquests.sendSignal(player, failSignal);
		messageTo(self, "handleDestroySelf", null, 1, false);
		return true;
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
		
		if (prisoner_pirate_cave_condition_hasArrived (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_16");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (prisoner_pirate_cave_condition_isTraveling (player, npc))
		{
			prisoner_pirate_cave_action_walkFaster (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_15");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (prisoner_pirate_cave_condition_hasRescueTask (player, npc))
		{
			doAnimationAction (npc, "scared");
			
			string_id message = new string_id (c_stringFile, "s_6");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (prisoner_pirate_cave_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_8");
				}
				
				utils.setScriptVar (player, "conversation.prisoner_pirate_cave.branchId", 3);
				
				npcStartConversation (player, npc, "prisoner_pirate_cave", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (prisoner_pirate_cave_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_18");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("prisoner_pirate_cave"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.prisoner_pirate_cave.branchId");
		
		if (branchId == 3 && prisoner_pirate_cave_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && prisoner_pirate_cave_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.prisoner_pirate_cave.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
