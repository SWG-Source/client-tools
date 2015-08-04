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
import script.library.faction_perk;
import script.library.factions;
import script.library.utils;


public class faction_recruiter_rebel extends script.base_script
{
	public faction_recruiter_rebel()
	{
	}
	String c_stringFile = "conversation/faction_recruiter_rebel";
	
	
	public boolean faction_recruiter_rebel_condition__defaultCondition(obj_id player, obj_id npc) throws InterruptedException
	{
		return true;
	}
	
	
	public boolean faction_recruiter_rebel_condition_isInFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.isInFriendlyFaction(player, npc)&&(!factions.isOnLeaveFromFriendlyFaction(player,npc)))&&(!factions.isPVPStatusChanging(player));
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_isEnemyFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.isInEnemyFaction(player, npc)&&(!factions.isPVPStatusChanging(player)));
	}
	
	
	public boolean faction_recruiter_rebel_condition_isBlockedByJoinTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		int factionId = pvpGetAlignedFaction(npc);
		String faction = factions.getFactionNameByHashCode(factionId);
		
		return factions.isGatedByJoinTimer(player, faction);
	}
	
	
	public boolean faction_recruiter_rebel_condition_playerQualifiesForPromotion(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.qualifiesForPromotion(player, pvpGetAlignedFaction(npc))&&(!factions.isPVPStatusChanging(player)));
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_isChangingFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.isPVPStatusChanging(player);
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_onLeaveFromFriendlyFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.isOnLeaveFromFriendlyFaction(player, npc)&&(!factions.isPVPStatusChanging(player)));
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_onLeaveFromEnemyFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.isOnLeaveFromEnemyFaction(player, npc)&&(!factions.isPVPStatusChanging(player)));
	}
	
	
	public boolean faction_recruiter_rebel_condition_isCovert(obj_id player, obj_id npc) throws InterruptedException
	{
		return(pvpGetType(player)==PVPTYPE_COVERT)&&(!factions.isPVPStatusChanging(player));
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_isOvert(obj_id player, obj_id npc) throws InterruptedException
	{
		return ((pvpGetType(player)==PVPTYPE_DECLARED)&&(!factions.isPVPStatusChanging(player)));
	}
	
	
	public boolean faction_recruiter_rebel_condition_playerCanBuyFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return (factions.isSmuggler(player)&&(factions.canBuyFaction(player, npc))&&(!factions.isPVPStatusChanging(player)));
		
	}
	
	
	public boolean faction_recruiter_rebel_condition_isAllowedToJoinFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.canJoinFaction(player, pvpGetAlignedFaction(npc));
	}
	
	
	public boolean faction_recruiter_rebel_condition_isRebelMercForGcw(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.isRebelHelper(player);
	}
	
	
	public boolean faction_recruiter_rebel_condition_canChangeStatus(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.canGoOnLeave(player);
	}
	
	
	public boolean faction_recruiter_rebel_condition_canBeMerc(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.canChangeNeutralMercenaryStatus(player);
	}
	
	
	public boolean faction_recruiter_rebel_condition_isRebOrImpMerc(obj_id player, obj_id npc) throws InterruptedException
	{
		return faction_recruiter_rebel_condition_isRebelMercForGcw(player, npc) || faction_recruiter_rebel_condition_isImpMercForGcw(player, npc);
	}
	
	
	public boolean faction_recruiter_rebel_condition_isImpMercForGcw(obj_id player, obj_id npc) throws InterruptedException
	{
		return factions.isImperialHelper(player);
	}
	
	
	public void faction_recruiter_rebel_action_showFactionGcwRewardUi(obj_id player, obj_id npc) throws InterruptedException
	{
		dictionary d = new dictionary();
		d.put("player", player);
		messageTo(npc, "showInventorySUI", d, 0, false);
	}
	
	
	public void faction_recruiter_rebel_action_enablePVPTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.goOvertWithDelay(player,30);
	}
	
	
	public void faction_recruiter_rebel_action_playerJoinFaction(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.joinFaction(player,pvpGetAlignedFaction(npc));
	}
	
	
	public void faction_recruiter_rebel_action_playerGoOnLeaveTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.goOnLeaveWithDelay(player,300);
	}
	
	
	public void faction_recruiter_rebel_action_makeRebMercSF(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.setNeturalMercenaryOvert(player, factions.FACTION_FLAG_REBEL);
	}
	
	
	public void faction_recruiter_rebel_action_makeRebMercCombatant(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.setNeturalMercenaryCovert(player, factions.FACTION_FLAG_REBEL);
	}
	
	
	public void faction_recruiter_rebel_action_playerGoCovertTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.goCovertWithDelay(player,300);
	}
	
	
	public void faction_recruiter_rebel_action_grantPromotion(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.applyPromotion(player, pvpGetAlignedFaction(npc));
	}
	
	
	public void faction_recruiter_rebel_action_buy1250FactionPoints(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.buyFaction(player, npc, 100000);
	}
	
	
	public void faction_recruiter_rebel_action_buy250FactionPoints(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.buyFaction(player, npc, 20000);
	}
	
	
	public void faction_recruiter_rebel_action_completelyResign(obj_id player, obj_id npc) throws InterruptedException
	{
		
		factions.leaveFaction(player, pvpGetAlignedFaction(npc));
		
	}
	
	
	public void faction_recruiter_rebel_action_goCovertShortTimer(obj_id player, obj_id npc) throws InterruptedException
	{
		factions.goCovertWithDelay(player,30);
	}
	
	
	public void faction_recruiter_rebel_action_showGcwRewardsList(obj_id player, obj_id npc) throws InterruptedException
	{
		int playerGcwRank = pvpGetCurrentGcwRank(player);
		int faction_id = pvpGetAlignedFaction(player);
		String playerGcwFaction = factions.getFactionNameByHashCode(faction_id);
		
		faction_perk.displayAvailableFactionItemRanks(player, npc, playerGcwRank, playerGcwFaction);
		
		return;
	}
	
	
	public String faction_recruiter_rebel_tokenTO_getFactionRankName(obj_id player, obj_id npc) throws InterruptedException
	{
		int current_rank = pvpGetCurrentGcwRank(player);
		String faction = factions.getFaction(player);
		if (current_rank > factions.MAXIMUM_RANK)
		{
			current_rank = 1;
		}
		current_rank = current_rank+1;
		return "@faction_recruiter:" + factions.getRankName(current_rank, faction);
	}
	
	
	public String faction_recruiter_rebel_tokenTO_getRebelScoreString(obj_id player, obj_id npc) throws InterruptedException
	{
		return ""+getIntObjVar(npc, "Rebel.controlScore");
	}
	
	
	public int faction_recruiter_rebel_tokenDI_getImperialScore(obj_id player, obj_id npc) throws InterruptedException
	{
		return getIntObjVar(npc, "Imperial.controlScore");
	}
	
	
	public int faction_recruiter_rebel_handleBranch1(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_218"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_220");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_222");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_226");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 2);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch2(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_222"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_goCovertShortTimer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_224");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_226"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_228");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch7(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch10(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_52"))
		{
			
			if (faction_recruiter_rebel_condition_isOvert (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_54");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_56");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_60");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 11);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isCovert (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_64");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_66");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_70");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 14);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_86"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_88");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_90");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_94");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 17);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_74"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_76");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_78");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_82");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 20);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_55"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_57");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch11(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_56"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_playerGoCovertTimer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_58");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_60"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_62");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch13(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch14(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_66"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_enablePVPTimer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_68");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_70"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_72");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch16(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch17(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_90"))
		{
			faction_recruiter_rebel_action_playerGoOnLeaveTimer (player, npc);
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_playerGoOnLeaveTimer (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_92");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_94"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_96");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch19(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch20(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_78"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				doAnimationAction (npc, "shake_head_disgust");
				
				faction_recruiter_rebel_action_completelyResign (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_80");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_82"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_84");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch23(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch24(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_73"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_75");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_248"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showGcwRewardsList (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_250");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_49"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_50");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canChangeStatus (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_52");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_86");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_74");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_55");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 10);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_330"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_332");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcSpeak (player, pp);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					prose_package pp = new prose_package ();
					pp.stringId = message;
					pp.actor.set (player);
					pp.target.set (npc);
					pp.other.set (faction_recruiter_rebel_tokenTO_getRebelScoreString (player, npc));
					pp.digitInteger = faction_recruiter_rebel_tokenDI_getImperialScore (player, npc);
					
					npcEndConversationWithMessage (player, pp);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch25(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_69"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_showFactionGcwRewardUi (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_71");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_83"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_85");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_87");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 27);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_348"))
		{
			
			if (faction_recruiter_rebel_condition_isAllowedToJoinFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_352");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_354");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_360");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 34);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isBlockedByJoinTimer (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_117");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_350");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch27(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_87"))
		{
			
			if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_89");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_93");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_95");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 28);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_91");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch28(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_93"))
		{
			
			if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				faction_recruiter_rebel_action_makeRebMercSF (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_97");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_98");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_95"))
		{
			
			if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
			{
				doAnimationAction (npc, "handshake_tandem");
				
				doAnimationAction (player, "handshake_tandem");
				
				faction_recruiter_rebel_action_makeRebMercCombatant (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_99");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_100");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch34(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_354"))
		{
			
			if (faction_recruiter_rebel_condition_isRebOrImpMerc (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_113");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_114");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 35);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_playerJoinFaction (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_356");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_358");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 37);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		if (response.equals("s_360"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_362");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch35(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_114"))
		{
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				faction_recruiter_rebel_action_playerJoinFaction (player, npc);
				
				string_id message = new string_id (c_stringFile, "s_115");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_116");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 36);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch36(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_116"))
		{
			
			if (faction_recruiter_rebel_condition_onLeaveFromFriendlyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_218");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 1);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_onLeaveFromEnemyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isEnemyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_232");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isInFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_234");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!faction_recruiter_rebel_condition_isChangingFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_334");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_isRebelMercForGcw (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_69");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_83");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_348");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 25);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_364");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int faction_recruiter_rebel_handleBranch37(obj_id player, obj_id npc, string_id response) throws InterruptedException
	{
		
		if (response.equals("s_358"))
		{
			
			if (faction_recruiter_rebel_condition_onLeaveFromFriendlyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_216");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_218");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 1);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_onLeaveFromEnemyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_230");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isEnemyFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_232");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition_isInFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_234");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse2 = true;
				}
				
				boolean hasResponse3 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse3 = true;
				}
				
				if (hasResponse)
				{
					int responseIndex = 0;
					string_id responses[] = new string_id[numberOfResponses];
					
					if (hasResponse0)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_73");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_248");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_49");
					}
					
					if (hasResponse3)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_330");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (!faction_recruiter_rebel_condition_isChangingFaction (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_334");
				int numberOfResponses = 0;
				
				boolean hasResponse = false;
				
				boolean hasResponse0 = false;
				if (faction_recruiter_rebel_condition_isRebelMercForGcw (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse0 = true;
				}
				
				boolean hasResponse1 = false;
				if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
				{
					++numberOfResponses;
					hasResponse = true;
					hasResponse1 = true;
				}
				
				boolean hasResponse2 = false;
				if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
						responses[responseIndex++] = new string_id (c_stringFile, "s_69");
					}
					
					if (hasResponse1)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_83");
					}
					
					if (hasResponse2)
					{
						responses[responseIndex++] = new string_id (c_stringFile, "s_348");
					}
					
					utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 25);
					
					npcSpeak (player, message);
					npcSetConversationResponses (player, responses);
				}
				else
				{
					utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
					
					npcEndConversationWithMessage (player, message);
				}
				
				return SCRIPT_CONTINUE;
			}
			
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				
				string_id message = new string_id (c_stringFile, "s_364");
				utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
				
				npcEndConversationWithMessage (player, message);
				
				return SCRIPT_CONTINUE;
			}
			
		}
		
		return SCRIPT_DEFAULT;
	}
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		if ((!isMob (self)) || (isPlayer (self)))
		{
			detachScript(self, "conversation.faction_recruiter_rebel");
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
		detachScript (self, "conversation.faction_recruiter_rebel");
		
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
		
		if (faction_recruiter_rebel_condition_onLeaveFromFriendlyFaction (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_216");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_218");
				}
				
				utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 1);
				
				npcStartConversation (player, npc, "faction_recruiter_rebel", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (faction_recruiter_rebel_condition_onLeaveFromEnemyFaction (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_230");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (faction_recruiter_rebel_condition_isEnemyFaction (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_232");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		if (faction_recruiter_rebel_condition_isInFaction (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_234");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse2 = true;
			}
			
			boolean hasResponse3 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse3 = true;
			}
			
			if (hasResponse)
			{
				int responseIndex = 0;
				string_id responses[] = new string_id[numberOfResponses];
				
				if (hasResponse0)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_73");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_248");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_49");
				}
				
				if (hasResponse3)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_330");
				}
				
				utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 7);
				
				npcStartConversation (player, npc, "faction_recruiter_rebel", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (!faction_recruiter_rebel_condition_isChangingFaction (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_334");
			int numberOfResponses = 0;
			
			boolean hasResponse = false;
			
			boolean hasResponse0 = false;
			if (faction_recruiter_rebel_condition_isRebelMercForGcw (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse0 = true;
			}
			
			boolean hasResponse1 = false;
			if (faction_recruiter_rebel_condition_canBeMerc (player, npc))
			{
				++numberOfResponses;
				hasResponse = true;
				hasResponse1 = true;
			}
			
			boolean hasResponse2 = false;
			if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
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
					responses[responseIndex++] = new string_id (c_stringFile, "s_69");
				}
				
				if (hasResponse1)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_83");
				}
				
				if (hasResponse2)
				{
					responses[responseIndex++] = new string_id (c_stringFile, "s_348");
				}
				
				utils.setScriptVar (player, "conversation.faction_recruiter_rebel.branchId", 25);
				
				npcStartConversation (player, npc, "faction_recruiter_rebel", message, responses);
			}
			else
			{
				chat.chat (npc, player, message);
			}
			
			return SCRIPT_CONTINUE;
		}
		
		if (faction_recruiter_rebel_condition__defaultCondition (player, npc))
		{
			
			string_id message = new string_id (c_stringFile, "s_364");
			chat.chat (npc, player, message);
			
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: All conditions for OnStartNpcConversation were false.");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnNpcConversationResponse(obj_id self, String conversationId, obj_id player, string_id response) throws InterruptedException
	{
		if (!conversationId.equals("faction_recruiter_rebel"))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id npc = self;
		
		int branchId = utils.getIntScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
		
		if (branchId == 1 && faction_recruiter_rebel_handleBranch1 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 2 && faction_recruiter_rebel_handleBranch2 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 7 && faction_recruiter_rebel_handleBranch7 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 10 && faction_recruiter_rebel_handleBranch10 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 11 && faction_recruiter_rebel_handleBranch11 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 13 && faction_recruiter_rebel_handleBranch13 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 14 && faction_recruiter_rebel_handleBranch14 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 16 && faction_recruiter_rebel_handleBranch16 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 17 && faction_recruiter_rebel_handleBranch17 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 19 && faction_recruiter_rebel_handleBranch19 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 20 && faction_recruiter_rebel_handleBranch20 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 23 && faction_recruiter_rebel_handleBranch23 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 24 && faction_recruiter_rebel_handleBranch24 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 25 && faction_recruiter_rebel_handleBranch25 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 27 && faction_recruiter_rebel_handleBranch27 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 28 && faction_recruiter_rebel_handleBranch28 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 34 && faction_recruiter_rebel_handleBranch34 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 35 && faction_recruiter_rebel_handleBranch35 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 36 && faction_recruiter_rebel_handleBranch36 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (branchId == 37 && faction_recruiter_rebel_handleBranch37 (player, npc, response) == SCRIPT_CONTINUE)
		{
			return SCRIPT_CONTINUE;
		}
		
		chat.chat (npc, "Error: Fell through all branches and responses for OnNpcConversationResponse.");
		
		utils.removeScriptVar (player, "conversation.faction_recruiter_rebel.branchId");
		
		return SCRIPT_CONTINUE;
	}
	
}
