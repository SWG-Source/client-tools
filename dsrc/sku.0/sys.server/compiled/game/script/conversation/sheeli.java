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
import script.library.utils;


public class sheeli extends script.base_script
{
	public sheeli()
	{
	}
	String c_stringFile = "conversation/sheeli";
	
	
	public boolean sheeli_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean sheeli_condition_isRescuer(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_rescue_sheeli", "rescueSheeli");
	}
	
	
	public boolean sheeli_condition_hasArrived(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_rescue_sheeli", "waitingForShuttle");
	}
	
	
	public boolean sheeli_condition_isTraveling(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return hasObjVar(npc, "traveling") && getIntObjVar(npc, "traveling") == 1 && groundquests.isTaskActive(player, "u16_nym_themepark_rescue_sheeli", "getToPickUp");
	}
	
	
	public void sheeli_action_startEscort(obj_id player, obj_id npc) throws InterruptedException
	{
		groundquests.sendSignal(player, "hasRescuedSheeli");
		ai_lib.aiFollow(npc, player, 1.0f, 3.0f);
		
		setMovementRun(npc);
		setBaseRunSpeed(npc, (getBaseRunSpeed(npc)-8));
		setObjVar(npc, "traveling", 1);
		setObjVar(npc, "playerOwner", player);
		setInvulnerable(npc, false);
		setMaster(npc, player);
		messageTo(npc, "checkArrivalLoopHandler", null, 5, false);
	}
	
	
	public void sheeli_action_walkFaster(obj_id player, obj_id npc) throws InterruptedException
	{
		setMovementRun(npc);
		setBaseRunSpeed(npc, (getBaseRunSpeed(npc)-8));
		ai_lib.aiFollow(npc, player, 1.0f, 3.0f);
	}
	
	
	public int sheeli_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_8"))
		{
			
			if (sheeli_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "thank");
				
				sheeli_action_startEscort (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_10");
				utils.removeScriptVar (player, "conversation.sheeli.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isTangible (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.sheeli");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		CustomerServiceLog("nyms_themepark","OnAttach - init.");
		messageTo(self, "startFollowingOwner", null, 3, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info menuInfo) throws InterruptedException
	{
		if (!hasObjVar(self, "quest.owner"))
		{
			CustomerServiceLog("nyms_themepark","OnObjectMenuRequest - quest.owner not valid.");
			messageTo(self, "handleDestroySelf", null, 0, false);
			return SCRIPT_OVERRIDE;
		}
		
		if (player != getObjIdObjVar(self, "quest.owner"))
		{
			CustomerServiceLog("nyms_themepark","OnObjectMenuRequest - quest.owner var found but not valid.");
			return SCRIPT_OVERRIDE;
		}
		
		int menu = menuInfo.addRootMenu (menu_info_types.CONVERSE_START, null);
		menu_info_data menuInfoData = menuInfo.getMenuItemById (menu);
		menuInfoData.setServerNotify (false);
		setCondition (self, CONDITION_CONVERSABLE);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnIncapacitated(obj_id self, obj_id killer) throws InterruptedException
	{
		clearCondition (self, CONDITION_CONVERSABLE);
		detachScript (self, "conversation.sheeli");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroySelf(obj_id self, dictionary params) throws InterruptedException
	{
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int startFollowingOwner(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "quest.owner"))
		{
			CustomerServiceLog("nyms_themepark","startFollowingOwner - quest.owner not valid.");
			messageTo(self, "handleDestroySelf", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		
		obj_id owner = getObjIdObjVar(self, "quest.owner");
		if (!isValidId(owner) || !exists(owner))
		{
			messageTo(self, "handleDestroySelf", null, 0, false);
			return SCRIPT_CONTINUE;
		}
		setInvulnerable(self, true);
		ai_lib.aiFollow(self, owner, 1.0f, 3.0f);
		
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		setObjVar(self, "traveling", 1);
		setInvulnerable(self, false);
		setMaster(self, owner);
		messageTo(self, "checkArrivalLoopHandler", null, 5, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int checkArrivalLoopHandler(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - Setting move to run.");
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		
		CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - init.");
		obj_id player = getObjIdObjVar(self, "quest.owner");
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
		
		CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - player: "+ player);
		
		if (!groundquests.isQuestActive(player, "u16_nym_themepark_rescue_sheeli"))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - player no longer had required quest active!");
			failRescueQuest(self);
			return SCRIPT_CONTINUE;
		}
		else if (groundquests.isTaskActive(player, "u16_nym_themepark_rescue_sheeli", "waitingForShuttle"))
		{
			CustomerServiceLog("nyms_themepark","checkArrivalLoopHandler - Calling SHuttle!");
			callShuttle(self, player);
			return SCRIPT_CONTINUE;
		}
		
		ai_lib.aiFollow(self, player, 1.0f, 3.0f);
		messageTo(self, "checkArrivalLoopHandler", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleUpdatePrisonerRescueQuest(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (!isValidId(self))
		{
			CustomerServiceLog("nyms_themepark","handleUpdatePrisonerRescueQuest - NPC is not a valid OID.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = getObjIdObjVar(self, "quest.owner");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			CustomerServiceLog("nyms_themepark","handleUpdatePrisonerRescueQuest - could not get valid player OID.");
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, "shuttleHasArrived");
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean callShuttle(obj_id self, obj_id player) throws InterruptedException
	{
		CustomerServiceLog("nyms_themepark","callShuttle - Shuttle Called");
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
		
		location[] prisonerPath = new location[1];
		prisonerPath[0] = new location(-42, 11, 2741, "lok", null);
		setYaw(self, 27.0f);
		setMovementRun(self);
		setBaseRunSpeed(self, (getBaseRunSpeed(self)-8));
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, prisonerPath);
		utils.setScriptVar(self, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
		patrolOnce(self, prisonerPath, 0);
		
		location landLocation = new location(-28, 11, 2759, "lok", null);
		obj_id shuttle = create.object("object/creature/npc/theme_park/player_shuttle.iff", landLocation);
		if (!isIdValid(shuttle))
		{
			return false;
		}
		
		setYaw(shuttle, -140);
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
	
	
	public boolean failRescueQuest(obj_id self) throws InterruptedException
	{
		obj_id player = getObjIdObjVar(self, "quest.owne");
		if (!isValidId(player))
		{
			messageTo(self, "handleDestroySelf", null, 1, false);
			return false;
		}
		
		groundquests.sendSignal(player, "hasFailedQuest");
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
		
		if (sheeli_condition_hasArrived (player, npc))
		{
			doAnimationAction (npc, "scream");
			
			string_id message = new string_id (c_stringFile, "s_11");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (sheeli_condition_isTraveling (player, npc))
		{
			sheeli_action_walkFaster (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_13");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (sheeli_condition_isRescuer (player, npc))
		{
			doAnimationAction (npc, "nervous");
			
			string_id message = new string_id (c_stringFile, "s_6");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (sheeli_condition__defaultCondition (player, npc))
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
				
				utils.setScriptVar (player, "conversation.sheeli.branchId", 3);
				
				npcStartConversation (player, npc, "sheeli", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (sheeli_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_14");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("sheeli"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.sheeli.branchId");
		
		if (branchId == 3 && sheeli_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.sheeli.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
