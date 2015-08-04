package script.corpse;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.pet_lib;
import script.library.beast_lib;
import script.library.utils;
import script.library.corpse;
import script.library.permissions;
import script.library.money;
import script.library.prose;
import script.library.group;
import script.library.ai_lib;
import script.library.loot;
import script.library.smuggler;


public class ai_corpse extends script.base_script
{
	public ai_corpse()
	{
	}
	public static final string_id SID_NO_ITEMS = new string_id( "loot_n", "no_items");
	public static final string_id SID_NOT_IN_COMBAT = new string_id( "loot_n", "not_in_combat");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setCreatureStatic(self, true);
		
		if (beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id inventory = utils.getInventoryContainer(self);
		if (!hasScript(inventory, corpse.SCRIPT_AI_CORPSE_INVENTORY))
		{
			attachScript(inventory, corpse.SCRIPT_AI_CORPSE_INVENTORY);
		}
		utils.setScriptVar( self, "timeLootable", getGameTime() + 1 );
		messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, corpse.AI_CORPSE_EXPIRATION_TIME, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		boolean canOpen = corpse.hasLootPermissions(self, player);
		
		int mnu = -1;
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.LOOT);
		
		if (mid == null)
		{
			mnu = mi.addRootMenu (menu_info_types.LOOT, new string_id("",""));
		}
		else
		{
			mid.setServerNotify(true);
			mnu = mid.getId();
		}
		
		if (mnu > 0)
		{
			if (canOpen)
			{
				if (canHarvest( self, player ))
				{
					int[] hasResource = getIntArrayObjVar(self, corpse.VAR_HAS_RESOURCE);
					if (hasResource != null)
					{
						int hm = mi.addRootMenu( menu_info_types.SERVER_HARVEST_CORPSE, corpse.SID_HARVEST_CORPSE );
						if (hasResource[corpse.CCR_MEAT] > 0)
						{
							mi.addSubMenu( hm, menu_info_types.SERVER_MENU5, corpse.SID_HARVEST_MEAT );
						}
						
						if (hasResource[corpse.CCR_HIDE] > 0)
						{
							mi.addSubMenu( hm, menu_info_types.SERVER_MENU6, corpse.SID_HARVEST_HIDE );
						}
						
						if (hasResource[corpse.CCR_BONE] > 0)
						{
							mi.addSubMenu( hm, menu_info_types.SERVER_MENU7, corpse.SID_HARVEST_BONE );
						}
					}
				}
			}
		}
		
		String strLootTable = getStringObjVar(self, "loot.lootTable");
		
		if (strLootTable == null || strLootTable.length() < 1)
		{
			return SCRIPT_CONTINUE;
		}
		
		if ((!strLootTable.startsWith("npc") && !strLootTable.startsWith("droid")) || utils.hasScriptVar(self, "contrabandChecked"))
		{
			return SCRIPT_CONTINUE;
		}
		
		String playerClass = getSkillTemplate(player);
		
		if (isIdValid(player) && canOpen && playerClass.startsWith("smuggler") && getLevel(player) > 9)
		{
			mnu = mi.addRootMenu(menu_info_types.SERVER_MENU1, new string_id("smuggler/items", "mnu_find_illicit_goods"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (getState( player, STATE_COMBAT ) == 1)
		{
			sendSystemMessage( player, SID_NOT_IN_COMBAT );
			return SCRIPT_OVERRIDE;
		}
		
		if (utils.getDistance2D( self, player) > 10)
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (beast_lib.isBeast(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		int timeLootable = utils.getIntScriptVar( self, "timeLootable");
		int currentTime = getGameTime();
		if (timeLootable >= currentTime)
		{
			return SCRIPT_OVERRIDE;
		}
		
		boolean canOpen = corpse.hasLootPermissions(self, player);
		
		if (!isMob(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!isIncapacitated(self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (isIncapacitated(player))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (pet_lib.isPet (self))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (!hasObjVar(self, "readyToLoot"))
		{
			return SCRIPT_OVERRIDE;
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			String playerClass = getSkillTemplate(player);
			
			if (isIdValid(player) && canOpen && playerClass.startsWith("smuggler") && getLevel(player) > 9)
			{
				smuggler.inspectCorpseForContraband(player, self);
			}
		}
		
		obj_id inv = utils.getInventoryContainer(self);
		obj_id[] items = getContents(inv);
		boolean emptyCorpse = (items == null || items.length < 1);
		
		if (item == menu_info_types.LOOT)
		{
			if (canOpen)
			{
				if (isIdValid(inv))
				{
					if (!isIdValid(player) || !isPlayer(player) || getContainerType(self) == 0)
					{
						return SCRIPT_OVERRIDE;
					}
					
					if (pet_lib.isPet(self))
					{
						return SCRIPT_OVERRIDE;
					}
					
					if (group.isGrouped (player))
					{
						obj_id team = getGroupObject(player);
						int lootRule = getGroupLootRule (team);
						
						if (lootRule == group.MASTER_LOOTER)
						{
							
							if (!loot.doGroupLootAllCheck (player, self))
							{
								return SCRIPT_OVERRIDE;
							}
						}
						
						else if (lootRule == group.LOTTERY)
						{
							if (hasObjVar (self, "allowedToOpenFromFailedTransfer_" + player ))
							{
								
								if (!emptyCorpse)
								{
									queueCommand(player, (1880585606), inv, "", COMMAND_PRIORITY_DEFAULT);
								}
								else
								{
									sendSystemMessage( player, SID_NO_ITEMS );
									if (!canHarvest(self,player) && !utils.hasScriptVar(self,"quickDestroy"))
									{
										utils.setScriptVar(self,"quickDestroy", true);
										messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, 2.0f, false);
									}
								}
								return SCRIPT_CONTINUE;
							}
							
							if (!loot.doGroupLootAllCheck (player, self))
							{
								return SCRIPT_OVERRIDE;
							}
						}
					}
					
					if (!emptyCorpse)
					{
						queueCommand(player, (1880585606), inv, "", COMMAND_PRIORITY_DEFAULT);
					}
					else
					{
						sendSystemMessage( player, SID_NO_ITEMS );
						if (!canHarvest(self,player) && !utils.hasScriptVar(self,"quickDestroy"))
						{
							utils.setScriptVar(self,"quickDestroy", true);
							messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, 2.0f, false);
						}
					}
				}
				
				return SCRIPT_CONTINUE;
			}
		}
		
		else
		{
			if (canOpen)
			{
				if (canHarvest( self, player ))
				{
					if (item == menu_info_types.SERVER_HARVEST_CORPSE)
					{
						queueCommand( player, (43846853), self, "", COMMAND_PRIORITY_DEFAULT );
					}
					else if (item == menu_info_types.SERVER_MENU5)
					{
						queueCommand( player, (43846853), self, "meat", COMMAND_PRIORITY_DEFAULT );
					}
					else if (item == menu_info_types.SERVER_MENU6)
					{
						queueCommand( player, (43846853), self, "hide", COMMAND_PRIORITY_DEFAULT );
					}
					else if (item == menu_info_types.SERVER_MENU7)
					{
						queueCommand( player, (43846853), self, "bone", COMMAND_PRIORITY_DEFAULT );
					}
					
					sendDirtyObjectMenuNotification(self);
				}
			}
		}
		
		if (!canOpen)
		{
			sendSystemMessage( player, permissions.SID_NO_CORPSE_PERMISSION );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int harvestCorpse(obj_id self, dictionary params) throws InterruptedException
	{
		
		if (params == null)
		{
			return SCRIPT_CONTINUE;
		}
		obj_id player = params.getObjId( "player");
		if (!isIdValid( player ))
		{
			return SCRIPT_CONTINUE;
		}
		String args = params.getString( "args");
		
		corpse.harvestCreatureCorpse( player, self, args );
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLootAddPass(obj_id self, dictionary params) throws InterruptedException
	{
		int bankBalance = getBankBalance(self);
		
		withdrawCashFromBank(self, bankBalance, money.HANDLER_LOOT_WITHDRAW_PASS, money.HANDLER_LOOT_WITHDRAW_FAIL, params);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLootAddFail(obj_id self, dictionary params) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLootWithdrawPass(obj_id self, dictionary params) throws InterruptedException
	{
		if (params != null)
		{
			int amt = params.getInt(money.DICT_AMOUNT);
			
		}
		else
		{
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleLootWithdrawFail(obj_id self, dictionary params) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseExpire(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id inv = utils.getInventoryContainer(self);
		if (isIdValid(inv))
		{
			obj_id[] contents = getContents(inv);
			if ((contents != null) && (contents.length != 0))
			{
				int time = getGameTime();
				int lootTime = utils.getIntScriptVar(self, "lootTimestamp");
				
				if (time - lootTime < 60)
				{
					messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, 10, false);
					return SCRIPT_CONTINUE;
				}
			}
		}
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleCorpseEmpty(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("loot","handleCorpseEmpty INIT");
		
		obj_id inv = utils.getInventoryContainer(self);
		if (isIdValid(inv))
		{
			obj_id[] contents = getContents(inv);
			if ((contents == null) || (contents.length == 0))
			{
				int cash = getCashBalance(self);
				if (cash == 0)
				{
					if (!hasObjVar(self, corpse.VAR_HAS_RESOURCE))
					{
						messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, 0, false);
						return SCRIPT_CONTINUE;
					}
					else
					{
						messageTo(self, corpse.HANDLER_CORPSE_EXPIRE, null, corpse.AI_CORPSE_EMPTY_TIME, false);
						return SCRIPT_CONTINUE;
					}
				}
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleAttemptCorpseCleanup(obj_id self, dictionary params) throws InterruptedException
	{
		LOG("loot","handleAttemptCorpseCleanup:DESTROYING CORPSE");
		messageTo(self, "handleCorpseEmpty", null, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLootLotterySelected(obj_id self, obj_id player, obj_id[] selection_ids) throws InterruptedException
	{
		obj_id selfInv = utils.getInventoryContainer (self);
		
		int numWindowsOpen = getIntObjVar(self, "numWindowsOpen");
		numWindowsOpen = numWindowsOpen - 1;
		setObjVar (self, "numWindowsOpen", numWindowsOpen);
		
		for (int i = 0; i < selection_ids.length; ++i)
		{
			testAbortScript();
			obj_id thisObject = selection_ids[i];
			
			if (!hasObjVar (thisObject, "numLotteryPlayers"))
			{
				setObjVar (thisObject, "numLotteryPlayers", 1);
				setObjVar (thisObject, "lotteryPlayer1", player);
			}
			else
			{
				int numLotteryPlayers = getIntObjVar (thisObject, "numLotteryPlayers");
				numLotteryPlayers = numLotteryPlayers + 1;
				setObjVar (thisObject, "lotteryPlayer"+ numLotteryPlayers, player);
				setObjVar (thisObject, "numLotteryPlayers", numLotteryPlayers);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int fireLotteryPulse(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = params.getObjId ("player");
		obj_id corpseInv = params.getObjId ("corpseInv");
		obj_id corpseId = getContainedBy(corpseInv);
		int numTimesFired = params.getInt ("fired");
		int numWindowsOpen = getIntObjVar (corpseId, "numWindowsOpen");
		numTimesFired = numTimesFired + 1;
		
		if (!utils.hasScriptVar(self, "default_money_recipient_inventory"))
		{
			utils.setScriptVar(self, "default_money_recipient_inventory", utils.getInventoryContainer(player));
		}
		
		if (numTimesFired > 18 || numWindowsOpen <= 0)
		{
			boolean allTransfersComplete = true;
			
			obj_id[] corpseContents = getContents (corpseInv);
			for (int i = 0; i < corpseContents.length; ++i)
			{
				testAbortScript();
				obj_id item = corpseContents[i];
				
				int numLotteryPlayers = getIntObjVar (item, "numLotteryPlayers");
				
				int lotteryNumber = rand (1, numLotteryPlayers);
				obj_id lottoWinner = getObjIdObjVar (item, "lotteryPlayer"+ lotteryNumber);
				
				if (numWindowsOpen > 0)
				{
					setObjVar (item, "pickupable", lottoWinner);
				}
				
				obj_id winventory = utils.getInventoryContainer(lottoWinner);
				
				utils.setScriptVar(lottoWinner, "autostack.ignoreitems", 1);
				
				if (loot.isCashLootItem(item))
				{
					
					if (isIdNull(winventory))
					{
						winventory = utils.getObjIdScriptVar(self, "default_money_recipient_inventory");
					}
					
					putIn(item, winventory);
					continue;
				}
				
				if (!putIn(item, winventory))
				{
					
					if (!loot.isCashLootItem(item))
					{
						
						String transferProblem = "full_inventory_free_for_all";
						
						boolean inventoryHasRoom = (getVolumeFree(winventory) > 0);
						
						if (inventoryHasRoom)
						{
							
							transferProblem = "unable_to_transfer";
							setObjVar (corpseId, "allowedToOpenFromFailedTransfer_"+ lottoWinner, 1);
							setObjVar (item, "pickupable", lottoWinner);
						}
						else
						{
							
							loot.setAutoLootComplete(lottoWinner, corpseId, item);
						}
						
						dictionary proseParameters = new dictionary();
						proseParameters.put ("stf", group.GROUP_STF);
						proseParameters.put ("message", transferProblem);
						proseParameters.put ("TO", item);
						proseParameters.put ("TT", lottoWinner);
						
						obj_id gid = getGroupObject (lottoWinner);
						obj_id[] members = utils.getLocalGroupMemberIds(gid);
						
						for (int x = 0; x < members.length; ++x)
						{
							testAbortScript();
							messageTo(members[x], "sendSystemMessageProseAuthoritative", proseParameters, 1, true);
						}
						
						allTransfersComplete = false;
					}
					
				}
				else
				{
					
					loot.sendGroupLootSystemMessage (item, lottoWinner, "loot_n", "group_looted");
					
					messageTo(item, "msgStackItem", null, 1, false);
				}
				
				utils.removeScriptVar(lottoWinner, "autostack.ignoreitems");
			}
			
			if (allTransfersComplete)
			{
				
				messageTo(corpseId, "handleCorpseEmpty", null, 1f, isObjectPersisted(corpseId));
			}
			return SCRIPT_CONTINUE;
		}
		dictionary lottery = new dictionary();
		
		lottery.put ("fired", numTimesFired);
		lottery.put ("player", player);
		lottery.put ("corpseInv", corpseInv);
		
		obj_id team = getGroupObject (player);
		obj_id[] objMembersWhoExist = utils.getLocalGroupMemberIds(team);
		messageTo(self, "fireLotteryPulse", lottery, 2, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean canHarvest(obj_id self, obj_id player) throws InterruptedException
	{
		return ( hasObjVar(self, corpse.VAR_HAS_RESOURCE) && !utils.hasScriptVar(self, "harvestedBy." + player) );
	}
	
}
