package script.conversation;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.ai_lib;
import script.library.buff;
import script.library.chat;
import script.library.conversation;
import script.library.create;
import script.library.factions;
import script.library.gcw;
import script.library.groundquests;
import script.library.holiday;
import script.library.trial;
import script.library.utils;


public class rebel_general extends script.base_script
{
	public rebel_general()
	{
	}
	String c_stringFile = "conversation/rebel_general";
	
	
	public boolean rebel_general_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean rebel_general_condition_isEntertainerPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.ENTERTAINER);
	}
	
	
	public boolean rebel_general_condition_isPlayerImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isImperial(player) || factions.isPlayerSameGcwFactionAsSchedulerObject(player, npc));
	}
	
	
	public boolean rebel_general_condition_isPlayerRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return (factions.isRebel(player) || factions.isPlayerSameGcwFactionAsSchedulerObject(player, npc));
	}
	
	
	public boolean rebel_general_condition_isPlayerNeutral(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return(!factions.isImperial(player) && !factions.isRebel(player) && pvpNeutralGetMercenaryFaction(player) == 0);
	}
	
	
	public boolean rebel_general_condition_isTraderPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.TRADER);
	}
	
	
	public boolean rebel_general_condition_isForceSensitivePlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.FORCE_SENSITIVE);
	}
	
	
	public boolean rebel_general_condition_isSmugglerPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.SMUGGLER);
	}
	
	
	public boolean rebel_general_condition_isMedicPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.MEDIC);
	}
	
	
	public boolean rebel_general_condition_isOfficerPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.OFFICER);
	}
	
	
	public boolean rebel_general_condition_isSpyPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.SPY);
	}
	
	
	public boolean rebel_general_condition_isBountyHunterPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.BOUNTY_HUNTER);
	}
	
	
	public boolean rebel_general_condition_isCommandoPlayer(obj_id player, obj_id npc) throws InterruptedException
	{
		faceTo(npc, player);
		return utils.isProfession(player, utils.COMMANDO);
	}
	
	
	public boolean rebel_general_condition_generalIsOffensive(obj_id player, obj_id npc) throws InterruptedException
	{
		return (hasObjVar(npc, "offense") && getIntObjVar(npc, "offense") == 1);
	}
	
	
	public boolean rebel_general_condition_generalIsImperial(obj_id player, obj_id npc) throws InterruptedException
	{
		return (hasObjVar(npc, "factionFlag") && getIntObjVar(npc, "factionFlag") == factions.FACTION_FLAG_IMPERIAL);
	}
	
	
	public boolean rebel_general_condition_generalIsDefensive(obj_id player, obj_id npc) throws InterruptedException
	{
		return (hasObjVar(npc, "offense") && getIntObjVar(npc, "offense") == 0);
	}
	
	
	public boolean rebel_general_condition_generalIsRebel(obj_id player, obj_id npc) throws InterruptedException
	{
		return (hasObjVar(npc, "factionFlag") && getIntObjVar(npc, "factionFlag") == factions.FACTION_FLAG_REBEL);
	}
	
	
	public boolean rebel_general_condition_isGeneralRebAndOffense(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isPlayerRebel(player, npc) && rebel_general_condition_generalIsRebel(player, npc) && rebel_general_condition_generalIsOffensive(player, npc);
	}
	
	
	public boolean rebel_general_condition_isCombatPhase(obj_id player, obj_id npc) throws InterruptedException
	{
		return (gcw.getGcwCityInvasionPhase(npc) == gcw.GCW_CITY_PHASE_COMBAT);
	}
	
	
	public boolean rebel_general_condition_isConstructionPhase(obj_id player, obj_id npc) throws InterruptedException
	{
		return (gcw.getGcwCityInvasionPhase(npc) == gcw.GCW_CITY_PHASE_CONSTRUCTION);
	}
	
	
	public boolean rebel_general_condition_isGeneralRebAndDefense(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isPlayerRebel(player, npc) && rebel_general_condition_generalIsRebel(player, npc) && rebel_general_condition_generalIsDefensive(player, npc);
	}
	
	
	public boolean rebel_general_condition_isGeneralImpAndOffense(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isPlayerImperial(player, npc) && rebel_general_condition_generalIsImperial(player, npc) && rebel_general_condition_generalIsOffensive(player, npc);
	}
	
	
	public boolean rebel_general_condition_isGeneralImpAndDefense(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isPlayerImperial(player, npc) && rebel_general_condition_generalIsImperial(player, npc) && rebel_general_condition_generalIsDefensive(player, npc);
	}
	
	
	public boolean rebel_general_condition_hasRebelQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isPlayerRebel(player, npc) && (rebel_general_condition_hasSpyScoutPatrolQuest(player, npc) || rebel_general_condition_hasSpyDestroyPatrolQuest(player, npc) || rebel_general_condition_hasSpyDestroyPatrolQuestNeedsExplosives(player, npc) || rebel_general_condition_hasDestroyBarricadeQuest(player, npc) || rebel_general_condition_hasDestroyTurretQuest(player, npc) );
	}
	
	
	public boolean rebel_general_condition_hasSpyDestroyPatrolQuestNeedsExplosives(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isSpyPlayer(player, npc) && groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST) && (buff.getBuffStackCount(player, "gcw_spy_destroy_patrol_explosive_stack") < 5);
	}
	
	
	public boolean rebel_general_condition_hasSpyScoutPatrolQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isSpyPlayer(player, npc) && groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST);
	}
	
	
	public boolean rebel_general_condition_hasSpyDestroyPatrolQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isSpyPlayer(player, npc) && groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST);
	}
	
	
	public boolean rebel_general_condition_hasDestroyTurretQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, gcw.GCW_DESTROY_TURRET);
	}
	
	
	public boolean rebel_general_condition_hasDestroyBarricadeQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, gcw.GCW_DESTROY_BARRICADE);
	}
	
	
	public boolean rebel_general_condition_hasDestroyQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_hasSpyDestroyPatrolQuest(player, npc) || rebel_general_condition_hasDestroyTurretQuest(player, npc) || rebel_general_condition_hasDestroyBarricadeQuest(player, npc);
	}
	
	
	public boolean rebel_general_condition_hasEntertainRallyQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY);
	}
	
	
	public boolean rebel_general_condition_enemyPatrolPointsExist(obj_id player, obj_id npc) throws InterruptedException
	{
		obj_id parent = trial.getParent(npc);
		if (!isValidId(parent) || !exists(parent))
		{
			return false;
		}
		
		if (rebel_general_condition_generalIsDefensive(player, npc))
		{
			obj_id[] patrols = trial.getObjectsInInstanceBySpawnId(parent, "offense_quest_asset");
			if (patrols == null || patrols.length <= 0)
			{
				return false;
			}
			
			return patrols.length > 0;
		}
		else if (rebel_general_condition_generalIsOffensive(player, npc))
		{
			obj_id[] patrols = trial.getObjectsInInstanceBySpawnId(parent, "defense_quest_asset");
			if (patrols == null || patrols.length <= 0)
			{
				return false;
			}
			
			return patrols.length > 0;
		}
		
		return false;
	}
	
	
	public boolean rebel_general_condition_enemyPatrolsExistAndIsCombatPhase(obj_id player, obj_id npc) throws InterruptedException
	{
		return (rebel_general_condition_enemyPatrolPointsExist(player, npc) && rebel_general_condition_isCombatPhase(player, npc));
	}
	
	
	public boolean rebel_general_condition_enemyPatrolPointsDestroyed(obj_id player, obj_id npc) throws InterruptedException
	{
		return (rebel_general_condition_isCombatPhase(player, npc) && !rebel_general_condition_enemyPatrolPointsExist(player, npc));
	}
	
	
	public boolean rebel_general_condition_canCommandoQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isCommandoPlayer(player, npc) && (!groundquests.isQuestActive(player, gcw.GCW_DESTROY_BARRICADE) || !groundquests.isQuestActive(player, gcw.GCW_DESTROY_TURRET));
	}
	
	
	public boolean rebel_general_condition_canEntertainerQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_isEntertainerPlayer(player, npc) && (!groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY) || !groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_FATIGUE));
	}
	
	
	public boolean rebel_general_condition_canBeMerc(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.canChangeNeutralMercenaryStatus(player);
	}
	
	
	public boolean rebel_general_condition_patrolPointsExistNoScoutQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_enemyPatrolPointsExist(player, npc) && !groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST);
	}
	
	
	public boolean rebel_general_condition_patrolPointsExistNoDestroyQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		return rebel_general_condition_enemyPatrolPointsExist(player, npc) && !groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST);
	}
	
	
	public void rebel_general_action_spawnGuards(obj_id player, obj_id npc) throws InterruptedException
	{
		
		for (int i = 0; i < 3; i++)
		{
			testAbortScript();
			location guardLocation = groundquests.getRandom2DLocationAroundLocation(npc, 1, 1, 5, 12);
			int mobLevel = getLevel(player);
			obj_id guard = create.object("rebel_emperorsday_ceremony_sentry", guardLocation, mobLevel);
			attachScript(guard, "event.emp_day.factional_guard_self_destruct");
			startCombat(guard, player);
		}
	}
	
	
	public void rebel_general_action_grantSpyDestroyPatrol(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST))
		{
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST))
		{
			groundquests.clearQuest(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST);
		}
		
		buff.applyBuffWithStackCount(player, gcw.BUFF_SPY_EXPLOSIVES, 5);
		groundquests.grantQuest(player, gcw.GCW_SPY_PATROL_DESTROY_QUEST);
	}
	
	
	public void rebel_general_action_grantSpyScoutPatrol(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST))
		{
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST))
		{
			groundquests.clearQuest(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST);
		}
		
		groundquests.grantQuest(player, gcw.GCW_SPY_PATROL_SCOUT_QUEST);
	}
	
	
	public void rebel_general_action_grantMoreSpyDestroyPatrolExplosives(obj_id player, obj_id npc) throws InterruptedException
	{
		buff.applyBuffWithStackCount(player, gcw.BUFF_SPY_EXPLOSIVES, 5);
	}
	
	
	public void rebel_general_action_grantCombatDestroyBarricade(obj_id player, obj_id npc) throws InterruptedException
	{
		
	}
	
	
	public void rebel_general_action_grantCombatDestroyTurret(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, gcw.GCW_DESTROY_TURRET))
		{
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, gcw.GCW_DESTROY_TURRET))
		{
			groundquests.clearQuest(player, gcw.GCW_DESTROY_TURRET);
		}
		
		groundquests.grantQuest(player, gcw.GCW_DESTROY_TURRET);
	}
	
	
	public void rebel_general_action_grantEntertainRallyQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY))
		{
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, gcw.GCW_ENTERTAIN_RALLY))
		{
			groundquests.clearQuest(player, gcw.GCW_ENTERTAIN_RALLY);
		}
		
		groundquests.grantQuest(player, gcw.GCW_ENTERTAIN_RALLY);
	}
	
	
	public void rebel_general_action_grantEntertainFatigueQuest(obj_id player, obj_id npc) throws InterruptedException
	{
		if (groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_FATIGUE))
		{
			return;
		}
		
		if (groundquests.hasCompletedQuest(player, gcw.GCW_ENTERTAIN_FATIGUE))
		{
			groundquests.clearQuest(player, gcw.GCW_ENTERTAIN_FATIGUE);
		}
		
		groundquests.grantQuest(player, gcw.GCW_ENTERTAIN_FATIGUE);
	}
	
	
	public void rebel_general_action_grantEntertainerQuests(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_FATIGUE))
		{
			if (groundquests.hasCompletedQuest(player, gcw.GCW_ENTERTAIN_FATIGUE))
			{
				groundquests.clearQuest(player, gcw.GCW_ENTERTAIN_FATIGUE);
			}
			
			groundquests.grantQuest(player, gcw.GCW_ENTERTAIN_FATIGUE);
		}
		
		if (!groundquests.isQuestActive(player, gcw.GCW_ENTERTAIN_RALLY))
		{
			if (groundquests.hasCompletedQuest(player, gcw.GCW_ENTERTAIN_RALLY))
			{
				groundquests.clearQuest(player, gcw.GCW_ENTERTAIN_RALLY);
			}
			
			groundquests.grantQuest(player, gcw.GCW_ENTERTAIN_RALLY);
		}
	}
	
	
	public void rebel_general_action_grantCombatQuests(obj_id player, obj_id npc) throws InterruptedException
	{
		if (!groundquests.isQuestActive(player, gcw.GCW_DESTROY_BARRICADE))
		{
			if (groundquests.hasCompletedQuest(player, gcw.GCW_DESTROY_BARRICADE))
			{
				groundquests.clearQuest(player, gcw.GCW_DESTROY_BARRICADE);
			}
			
			groundquests.grantQuest(player, gcw.GCW_DESTROY_BARRICADE);
		}
		
		if (!groundquests.isQuestActive(player, gcw.GCW_DESTROY_TURRET))
		{
			if (groundquests.hasCompletedQuest(player, gcw.GCW_DESTROY_TURRET))
			{
				groundquests.clearQuest(player, gcw.GCW_DESTROY_TURRET);
			}
			
			groundquests.grantQuest(player, gcw.GCW_DESTROY_TURRET);
		}
		
		if (!groundquests.isQuestActive(player, gcw.GCW_ELIMINATE_IMPERIALS))
		{
			if (groundquests.hasCompletedQuest(player, gcw.GCW_ELIMINATE_IMPERIALS))
			{
				groundquests.clearQuest(player, gcw.GCW_ELIMINATE_IMPERIALS);
			}
			
			groundquests.grantQuest(player, gcw.GCW_ELIMINATE_IMPERIALS);
		}
	}
	
	
	public void rebel_general_action_makeRebMercCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.setNeturalMercenaryCovert(player, factions.FACTION_FLAG_REBEL);
	}
	
	
	public void rebel_general_action_makeRebMercSF(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.setNeturalMercenaryOvert(player, factions.FACTION_FLAG_REBEL);
	}
	
	
	public int rebel_general_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_46"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_48");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_110");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_110"))
		{
			
			if (rebel_general_condition_canBeMerc (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_111");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_112");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 3);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_123");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch3(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_112"))
		{
			
			if (rebel_general_condition_canBeMerc (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_113");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_116");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_117");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 4);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_115");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch4(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_116"))
		{
			
			if (rebel_general_condition_canBeMerc (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				rebel_general_action_makeRebMercSF (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_118");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_119");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_117"))
		{
			
			if (rebel_general_condition_canBeMerc (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				rebel_general_action_makeRebMercCombatant (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_120");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_121");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_21"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantEntertainerQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_328");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				rebel_general_action_grantEntertainFatigueQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_331");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_31"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_33");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_183");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_36"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_192");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_330");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_51"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_194");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_332");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_61"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_196");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_333");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_71"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_201");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_335");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_81"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_222");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition_patrolPointsExistNoScoutQuest (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_general_condition_patrolPointsExistNoDestroyQuest (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_223");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_224");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_225");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 24);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_203");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_102"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_205");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_337");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_114"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_207");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_339");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_312"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantMoreSpyDestroyPatrolExplosives (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_314");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_316"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_318");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_320"))
		{
			
			if (rebel_general_condition_hasSpyDestroyPatrolQuest (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_322");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_hasDestroyBarricadeQuest (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_324");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_hasDestroyTurretQuest (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_326");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_223"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantSpyScoutPatrol (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_228");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_224"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantSpyDestroyPatrol (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_234");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_225"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantCombatQuests (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_246");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch38(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_126"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_128");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_281");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_304");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 39);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_185");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_295");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 43);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_146"))
		{
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_148");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_190");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_151"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_153");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_212");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_156"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_158");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_213");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_161"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_163");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_214");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_166"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_169");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_177"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_218");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition_patrolPointsExistNoScoutQuest (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_general_condition_patrolPointsExistNoDestroyQuest (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_227");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_247");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_138");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 55);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_286");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_enemyPatrolsExistAndIsCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_302");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_289"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_291");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_297");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_299"))
		{
			
			if (rebel_general_condition_isCombatPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_301");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (rebel_general_condition_isConstructionPhase (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_308");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch39(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_281"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_282");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_283");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 40);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_304"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantEntertainFatigueQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_305");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch40(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_283"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantEntertainRallyQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_284");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch43(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_295"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantEntertainFatigueQuest (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_296");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch55(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_227"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_235");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_239");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 56);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_247"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_251");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_261");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_273");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 58);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_138"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_139");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch56(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_239"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantSpyScoutPatrol (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_243");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch58(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_261"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_265");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (rebel_general_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_269");
					}
					
					utils.setScriptVar (player, "conversation.rebel_general.branchId", 59);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.rebel_general.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_273"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantSpyDestroyPatrol (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_276");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int rebel_general_handleBranch59(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_269"))
		{
			
			if (rebel_general_condition__defaultCondition (player, npc))
			{
				rebel_general_action_grantSpyDestroyPatrol (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_276");
				utils.removeScriptVar (player, "conversation.rebel_general.branchId");
				
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
			detachScript(self, "conversation.rebel_general");
		}
		
		setCondition (self, CONDITION_CONVERSABLE);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCondition (self, CONDITION_CONVERSABLE);
		dictionary params = new dictionary();
		location loc = getLocation(self);
		params.put("particleLoc", loc);
		int playIconTime = getGameTime();
		params.put("iconMessageTime", playIconTime);
		utils.setScriptVar(self, "iconMessageTime", playIconTime);
		messageTo(self, "playQuestIcon", params, 1.0f, false);
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
		detachScript (self, "conversation.rebel_general");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int playQuestIcon(obj_id self, dictionary params) throws InterruptedException
	{
		gcw.playQuestIconHandler(self, params);
		
		if (!utils.hasScriptVar(self, "faction"))
		{
			if (!hasObjVar(self, "factionFlag"))
			{
				return SCRIPT_CONTINUE;
			}
			
			int faction = getIntObjVar(self, "factionFlag");
			utils.setScriptVar(self, "faction", faction);
		}
		
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
		
		if (rebel_general_condition_isPlayerNeutral (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_44");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_general_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_46");
				}
				
				utils.setScriptVar (player, "conversation.rebel_general.branchId", 1);
				
				npcStartConversation (player, npc, "rebel_general", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_general_condition_isGeneralRebAndOffense (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_19");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_general_condition_canEntertainerQuest (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_general_condition_isTraderPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (rebel_general_condition_isCommandoPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (rebel_general_condition_isForceSensitivePlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (rebel_general_condition_isSmugglerPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			boolean hasResponse5 = false;
			if (rebel_general_condition_isBountyHunterPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse5 = true;
			}
			
			boolean hasResponse6 = false;
			if (rebel_general_condition_isSpyPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse6 = true;
			}
			
			boolean hasResponse7 = false;
			if (rebel_general_condition_isMedicPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse7 = true;
			}
			
			boolean hasResponse8 = false;
			if (rebel_general_condition_isOfficerPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse8 = true;
			}
			
			boolean hasResponse9 = false;
			if (rebel_general_condition_hasSpyDestroyPatrolQuestNeedsExplosives (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse9 = true;
			}
			
			boolean hasResponse10 = false;
			if (rebel_general_condition_hasSpyScoutPatrolQuest (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse10 = true;
			}
			
			boolean hasResponse11 = false;
			if (rebel_general_condition_hasDestroyQuest (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse11 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_21");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_31");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_36");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_51");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_61");
				}
				
				if (hasResponse5)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_71");
				}
				
				if (hasResponse6)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_81");
				}
				
				if (hasResponse7)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_102");
				}
				
				if (hasResponse8)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_114");
				}
				
				if (hasResponse9)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_312");
				}
				
				if (hasResponse10)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_316");
				}
				
				if (hasResponse11)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_320");
				}
				
				utils.setScriptVar (player, "conversation.rebel_general.branchId", 11);
				
				npcStartConversation (player, npc, "rebel_general", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_general_condition_isGeneralRebAndDefense (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_124");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (rebel_general_condition_isEntertainerPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (rebel_general_condition_isTraderPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (rebel_general_condition_isCommandoPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (rebel_general_condition_isForceSensitivePlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			boolean hasResponse4 = false;
			if (rebel_general_condition_isSmugglerPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse4 = true;
			}
			
			boolean hasResponse5 = false;
			if (rebel_general_condition_isBountyHunterPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse5 = true;
			}
			
			boolean hasResponse6 = false;
			if (rebel_general_condition_isSpyPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse6 = true;
			}
			
			boolean hasResponse7 = false;
			if (rebel_general_condition_isMedicPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse7 = true;
			}
			
			boolean hasResponse8 = false;
			if (rebel_general_condition_isOfficerPlayer (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse8 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_126");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_146");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_151");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_156");
				}
				
				if (hasResponse4)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_161");
				}
				
				if (hasResponse5)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_166");
				}
				
				if (hasResponse6)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_177");
				}
				
				if (hasResponse7)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_289");
				}
				
				if (hasResponse8)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_299");
				}
				
				utils.setScriptVar (player, "conversation.rebel_general.branchId", 38);
				
				npcStartConversation (player, npc, "rebel_general", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (rebel_general_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_310");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("rebel_general"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.rebel_general.branchId");
		
		if (branchId == 1 && rebel_general_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && rebel_general_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 3 && rebel_general_handleBranch3 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 4 && rebel_general_handleBranch4 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && rebel_general_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && rebel_general_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 38 && rebel_general_handleBranch38 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 39 && rebel_general_handleBranch39 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 40 && rebel_general_handleBranch40 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 43 && rebel_general_handleBranch43 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 55 && rebel_general_handleBranch55 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 56 && rebel_general_handleBranch56 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 58 && rebel_general_handleBranch58 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 59 && rebel_general_handleBranch59 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.rebel_general.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
