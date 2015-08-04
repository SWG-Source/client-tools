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
import script.library.features;
import script.library.groundquests;
import script.library.space_quest;
import script.library.utils;


public class shuttle_control_terminal extends script.base_script
{
	public shuttle_control_terminal()
	{
	}
	String c_stringFile = "conversation/shuttle_control_terminal";
	
	
	public boolean shuttle_control_terminal_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean shuttle_control_terminal_condition_hasQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, "outbreak_undead_rancor_boss_fight");
	}
	
	
	public boolean shuttle_control_terminal_condition_hasGroup(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id groupid = getGroupObject(player);
		if (!isValidId(groupid))
		{
			
			return false;
		}
		return true;
	}
	
	
	public boolean shuttle_control_terminal_condition_grantPermissionLandTask(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isTaskActive(player, "u16_nym_themepark_shuttle_ambush", "grantPermissionToLand");
	}
	
	
	public boolean shuttle_control_terminal_condition_hasQuestAndGroup(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!shuttle_control_terminal_condition_hasGroup(player,npc))
		{
			return false;
		}
		if (!shuttle_control_terminal_condition_hasQuest(player,npc))
		{
			return false;
		}
		return groundquests.isTaskActive(player, "outbreak_undead_rancor_boss_fight", "defeatUndeadRancor");
	}
	
	
	public boolean shuttle_control_terminal_condition_wave_event_active(obj_id player, obj_id npc) throws InterruptedException
	{
		if (utils.hasScriptVar(npc, "terminalLocked"))
		{
			return true;
		}
		
		int wave = utils.getIntScriptVar(npc, "waveEventCurrentWave");
		return wave > 0;
	}
	
	
	public void shuttle_control_terminal_action_createShuttleSendSignal(obj_id player, obj_id npc) throws InterruptedException
	{
		
		dictionary dict = new dictionary();
		dict.put("player", player);
		messageTo(npc, "waveEventControllerNPCStart", dict, 0, false);
		
		utils.setScriptVar(npc, "terminalLocked", 1);
		messageTo(npc, "removeTerminalLock", null, 90, false);
		
		location landLocation = new location( 475.3f, 12.3f, 4867.6f, "lok", null);
		obj_id shuttle = create.object("object/creature/npc/theme_park/player_shuttle.iff", landLocation);
		if (!isIdValid(shuttle))
		{
			return;
		}
		
		setYaw(shuttle, 75);
		detachScript(shuttle, "ai.ai");
		detachScript(shuttle, "ai.creature_combat");
		detachScript(shuttle, "skeleton.humanoid");
		detachScript(shuttle, "systems.combat.combat_actions");
		detachScript(shuttle, "systems.combat.combat_actions_creature");
		detachScript(shuttle, "systems.combat.credit_for_kills");
		stop(shuttle);
		
		utils.setScriptVar(shuttle, "terminal", npc);
		utils.setScriptVar(shuttle, "owner", player);
		setPosture(shuttle, POSTURE_PRONE);
		attachScript(shuttle, "systems.spawning.dropship.player_shuttle");
		
		messageTo(shuttle, "landShuttle", null, 2, false);
		groundquests.sendSignal(player, "hasGrantedPermissionToLand");
		
		location explosionLocaitonList[] = new location[6];
		explosionLocaitonList[0] = new location(475, 12, 4867, "lok", null);
		explosionLocaitonList[1] = new location(469, 12, 4867, "lok", null);
		explosionLocaitonList[2] = new location(471, 12, 4864, "lok", null);
		explosionLocaitonList[3] = new location(477, 12, 4864, "lok", null);
		explosionLocaitonList[4] = new location(480, 12, 4871, "lok", null);
		explosionLocaitonList[5] = new location(472, 12, 4872, "lok", null);
		
		dictionary webster = new dictionary();
		webster.put("player", player);
		webster.put("explosionLocaitonList", explosionLocaitonList);
		
		messageTo(npc, "signalPlayerEnvoy", webster, 32, false);
		
		messageTo(npc, "spawnEnemies", webster, 35, false);
		
		messageTo(npc, "spawnHeroes", webster, 42, false);
		
		messageTo(shuttle, "explodeThenDestroySelf", webster, 120, false);
		
	}
	
	
	public void shuttle_control_terminal_action_unauthorizedUse(obj_id player, obj_id npc) throws InterruptedException
	{
		string_id barkString = new string_id("theme_park_nym/messages", "terminal_unauthorized");
		chat.chat(npc, barkString);
	}
	
	
	public void shuttle_control_terminal_action_busyWithOtherPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		string_id barkString = new string_id("theme_park_nym/messages", "terminal_busy");
		chat.chat(npc, barkString);
	}
	
	
	public int shuttle_control_terminal_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_36"))
		{
			
			if (shuttle_control_terminal_condition__defaultCondition (player, npc))
			{
				shuttle_control_terminal_action_createShuttleSendSignal (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_37");
				utils.removeScriptVar (player, "conversation.shuttle_control_terminal.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int shuttle_control_terminal_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_9"))
		{
			
			if (shuttle_control_terminal_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_10");
				utils.removeScriptVar (player, "conversation.shuttle_control_terminal.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
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
	
	
	public int signalPlayerEnvoy(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		obj_id player = params.getObjId("player");
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!groundquests.isTaskActive(player, "u16_nym_themepark_shuttle_ambush", "greetEnvoy"))
		{
			return SCRIPT_CONTINUE;
		}
		
		groundquests.sendSignal(player, "hasGreetedEnvoy");
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnEnemies(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		
		messageTo(self, "waveEventControllerNPCStart", params, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleQuestFlavorObjectCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		if (!utils.hasScriptVar(self, "handleQuestFlavorObjectCleanup"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id objList[] = utils.getObjIdArrayScriptVar(self, "handleQuestFlavorObjectCleanup");
		if (objList == null || objList.length <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		for (int i = 0; 1 < objList.length; i++)
		{
			testAbortScript();
			if (!isValidId(objList[i]))
			{
				continue;
			}
			
			messageTo(objList[i], "destroySelf", null, 0, false);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int spawnHeroes(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("nym_themepark_log", "spawnHeroes: init");
		obj_id player = utils.getObjIdScriptVar(self, "waveEventPlayer");
		if (!isValidId(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		LOG("nym_themepark_log", "spawnHeroes: player OID received: "+player);
		location locNymSpawn = new location(471, 13, 4833, "lok", null);
		obj_id nym = create.object("nym_themepark_nym_ambush", locNymSpawn);
		if (isValidId(nym))
		{
			LOG("nym_themepark_log", "spawnHeroes: Nym Created: "+nym);
			setObjVar(nym, "parent", self);
			location[] locNymPath = new location[1];
			locNymPath[0] = new location(459, 12, 4854, "lok", null);
			setYaw(nym, -6.0f);
			setMovementRun(nym);
			setBaseRunSpeed(nym, (getBaseRunSpeed(nym)-8));
			setInvulnerable(nym, false);
			utils.setScriptVar(nym, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, locNymPath);
			utils.setScriptVar(nym, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
			patrolOnce(nym, locNymPath, 0);
			
		}
		
		location locKoleSpawn = new location(465, 13, 4832, "lok", null);
		obj_id kole = create.object("nym_themepark_kole_ambush", locKoleSpawn);
		if (isValidId(kole))
		{
			setObjVar(kole, "parent", self);
			location[] locKolePath = new location[1];
			locKolePath[0] = new location(469, 12, 4851, "lok", null);
			setYaw(kole, -6.0f);
			setMovementRun(kole);
			setBaseRunSpeed(kole, (getBaseRunSpeed(kole)-8));
			setInvulnerable(kole, false);
			utils.setScriptVar(kole, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, locKolePath);
			utils.setScriptVar(kole, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
			patrolOnce(kole, locKolePath, 0);
			
		}
		
		location locVanaSpawn = new location(480, 13, 4833, "lok", null);
		obj_id vana = create.object("nym_themepark_vana_ambush", locVanaSpawn);
		if (isValidId(vana))
		{
			setObjVar(vana, "parent", self);
			location[] locVanaPath = new location[1];
			locVanaPath[0] = new location(469, 12, 4851, "lok", null);
			setYaw(vana, -6.0f);
			setMovementRun(vana);
			setBaseRunSpeed(vana, (getBaseRunSpeed(vana)-8));
			setInvulnerable(vana, false);
			utils.setScriptVar(vana, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, locVanaPath);
			utils.setScriptVar(vana, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
			patrolOnce(vana, locVanaPath, 0);
		}
		
		location locJinkinsSpawn = new location(487, 13, 4831, "lok", null);
		obj_id jinkins = create.object("nym_themepark_jinkins_ambush", locJinkinsSpawn);
		if (isValidId(jinkins))
		{
			setObjVar(jinkins, "parent", self);
			location[] locJinkinsPath = new location[1];
			locJinkinsPath[0] = new location(484, 12, 4852, "lok", null);
			setYaw(jinkins, -6.0f);
			setMovementRun(jinkins);
			setBaseRunSpeed(jinkins, (getBaseRunSpeed(jinkins)-8));
			setInvulnerable(jinkins, false);
			utils.setScriptVar(jinkins, ai_lib.SCRIPTVAR_CACHED_PATROL_NAMED_PATH, locJinkinsPath);
			utils.setScriptVar(jinkins, ai_lib.SCRIPTVAR_CACHED_PATROL_TYPE, 0);
			patrolOnce(jinkins, locJinkinsPath, 0);
		}
		
		obj_id objectList[] = new obj_id[4];
		objectList[0] = vana;
		objectList[1] = jinkins;
		objectList[2] = kole;
		objectList[3] = nym;
		
		utils.setScriptVar(self, "handleQuestFlavorObjectCleanup", objectList);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int removeTerminalLock(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "terminalLocked");
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
		
		if (shuttle_control_terminal_condition_wave_event_active (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_7");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (shuttle_control_terminal_condition_grantPermissionLandTask (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_35");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (shuttle_control_terminal_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_36");
				}
				
				utils.setScriptVar (player, "conversation.shuttle_control_terminal.branchId", 2);
				
				npcStartConversation (player, npc, "shuttle_control_terminal", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (shuttle_control_terminal_condition__defaultCondition (player, npc))
		{
			shuttle_control_terminal_action_unauthorizedUse (player, npc);
			
			string_id message = new string_id (c_stringFile, "s_34");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (shuttle_control_terminal_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_9");
				}
				
				utils.setScriptVar (player, "conversation.shuttle_control_terminal.branchId", 4);
				
				npcStartConversation (player, npc, "shuttle_control_terminal", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("shuttle_control_terminal"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.shuttle_control_terminal.branchId");
		
		if (branchId == 2 && shuttle_control_terminal_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && shuttle_control_terminal_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.shuttle_control_terminal.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
