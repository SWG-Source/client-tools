package script.npc.vendor;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.factions;
import script.library.money;
import script.library.prose;
import script.library.static_item;
import script.library.township;
import script.library.trial;
import script.library.utils;


public class object_for_sale extends script.base_script
{
	public object_for_sale()
	{
	}
	public static final String VENDOR_TOKEN_TYPE = "item.token.type";
	public static final string_id SID_INV_FULL = new string_id("spam", "npc_vendor_player_inv_full");
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		setObjVar(self, township.OBJECT_FOR_SALE_ON_VENDOR, true);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (item == menu_info_types.ITEM_PUBLIC_CONTAINER_USE1)
		{
			if (hasObjVar(self, "faction_recruiter.faction"))
			{
				
				if (!factions.isInFriendlyFaction(player, self))
				{
					sendSystemMessage(player, new string_id("spam", "wrong_faction"));
					return SCRIPT_OVERRIDE;
				}
			}
			if (!confirmInventory(self, player))
			{
				return SCRIPT_OVERRIDE;
			}
			
			if (confirmFunds(self, player))
			{
				processItemPurchase(self, player);
			}
			else
			{
				sendSystemMessage(player, new string_id("spam", "buildabuff_nsf_buffee"));
			}
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		if (names == null || attribs == null || names.length != attribs.length)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		final int firstFreeIndex = getFirstFreeIndex(names);
		if (firstFreeIndex >= 0 && firstFreeIndex < names.length)
		{
			names[firstFreeIndex] = utils.packStringId( new string_id ( "set_bonus", "vendor_cost"));
			attribs[firstFreeIndex] = createPureConcatenatedJoy(self);
		}
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public String createPureConcatenatedJoy(obj_id self) throws InterruptedException
	{
		
		String pureConcatenatedJoy = getString(new string_id("set_bonus", "vendor_sale_object_justify_line"));
		
		int creditCost = 0;
		int tokenArrayLength = 0;
		
		creditCost = getIntObjVar(self, "item.object_for_sale.cash_cost");
		int[] tokenCost = getIntArrayObjVar(self, "item.object_for_sale.token_cost");
		
		if (!hasObjVar(self, VENDOR_TOKEN_TYPE))
		{
			
			if (tokenCost.length > trial.HEROIC_TOKENS.length)
			{
				tokenArrayLength = trial.HEROIC_TOKENS.length;
			}
			else if (tokenCost.length > 0)
			{
				tokenArrayLength = tokenCost.length;
			}
			
			for (int i = 0; i < tokenArrayLength; i++)
			{
				testAbortScript();
				if (tokenCost[i] > 0)
				{
					pureConcatenatedJoy += "["+tokenCost[i] + "] "+ getString(new string_id("static_item_n", trial.HEROIC_TOKENS[i])) +"\n";
				}
			}
		}
		
		else
		{
			String tokenList = getStringObjVar(self, VENDOR_TOKEN_TYPE);
			String[] differentTokens = split(tokenList, ',');
			
			if (tokenCost.length > differentTokens.length)
			{
				tokenArrayLength = differentTokens.length;
			}
			else if (tokenCost.length > 0)
			{
				tokenArrayLength = tokenCost.length;
			}
			
			for (int i = 0; i < tokenArrayLength; i++)
			{
				testAbortScript();
				if (tokenCost[i] > 0)
				{
					pureConcatenatedJoy += "["+tokenCost[i] + "] "+ getString(new string_id("static_item_n", differentTokens[i])) +"\n";
				}
			}
		}
		
		if (creditCost > 0)
		{
			pureConcatenatedJoy += creditCost + getString(new string_id("set_bonus", "vendor_credits"));
		}
		
		return pureConcatenatedJoy;
	}
	
	
	public boolean confirmInventory(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id pInv = utils.getInventoryContainer(player);
		
		if (!isValidId(pInv) || !exists(pInv))
		{
			return false;
		}
		
		if (getVolumeFree(pInv) <= 0)
		{
			sendSystemMessage(player, SID_INV_FULL);
			return false;
		}
		return true;
	}
	
	
	public boolean confirmFunds(obj_id self, obj_id player) throws InterruptedException
	{
		boolean canPay = false;
		boolean hasTheCredits = false;
		boolean hasTheTokens = false;
		boolean foundTokenHolderBox = false;
		
		obj_id[] inventoryContents = getInventoryAndEquipment(player);
		
		int creditCost = getIntObjVar(self, "item.object_for_sale.cash_cost");
		int[] tokenCostInThisFunction = getIntArrayObjVar(self, "item.object_for_sale.token_cost");
		
		if (creditCost < 1)
		{
			hasTheCredits = true;
		}
		
		if (creditCost >= 1)
		{
			if (money.hasFunds(player, money.MT_TOTAL, creditCost))
			{
				hasTheCredits = true;
			}
		}
		
		int owedTokenCount = 0;
		boolean owesTokens = false;
		for (int o = 0; o < tokenCostInThisFunction.length; o++)
		{
			testAbortScript();
			owedTokenCount += tokenCostInThisFunction[o];
		}
		
		if (owedTokenCount > 0)
		{
			owesTokens = true;
		}
		
		if (owesTokens)
		{
			for (int i = 0; i < inventoryContents.length; i++)
			{
				testAbortScript();
				String itemName = getStaticItemName(inventoryContents[i]);
				if (itemName != null && !itemName.equals(""))
				{
					
					if (hasObjVar(self, VENDOR_TOKEN_TYPE))
					{
						String tokenList = getStringObjVar(self, VENDOR_TOKEN_TYPE);
						String[] differentTokens = split(tokenList, ',');
						
						for (int j = 0; j < differentTokens.length; j++)
						{
							testAbortScript();
							if (itemName.equals(differentTokens[j]) && tokenCostInThisFunction[j] > 0)
							{
								if (getCount(inventoryContents[i]) > 1)
								{
									
									for (int m = 0; m < getCount(inventoryContents[i]); m++)
									{
										testAbortScript();
										if (tokenCostInThisFunction[j] > 0)
										{
											tokenCostInThisFunction[j]--;
										}
									}
								}
								else
								{
									tokenCostInThisFunction[j]--;
								}
							}
						}
					}
					else
					{
						
						for (int j = 0; j < trial.HEROIC_TOKENS.length; j++)
						{
							testAbortScript();
							if (itemName.equals(trial.HEROIC_TOKENS[j]) && tokenCostInThisFunction[j] > 0)
							{
								if (getCount(inventoryContents[i]) > 1)
								{
									
									for (int m = 0; m < getCount(inventoryContents[i]); m++)
									{
										testAbortScript();
										if (tokenCostInThisFunction[j] > 0)
										{
											tokenCostInThisFunction[j]--;
										}
									}
								}
								else
								{
									tokenCostInThisFunction[j]--;
								}
							}
						}
						
						if (!foundTokenHolderBox && itemName.equals("item_heroic_token_box_01_01"))
						{
							foundTokenHolderBox = true;
							
							if (hasObjVar(inventoryContents[i], "item.set.tokens_held"))
							{
								int[] virtualTokens = getIntArrayObjVar(inventoryContents[i], "item.set.tokens_held");
								for (int k = 0; k < trial.HEROIC_TOKENS.length; k++)
								{
									testAbortScript();
									if (tokenCostInThisFunction[k] > 0 && virtualTokens[k] > 0)
									{
										
										int paymentIterations = tokenCostInThisFunction[k];
										for (int l = 0; l < paymentIterations; l++)
										{
											testAbortScript();
											if (virtualTokens[k] > 0)
											{
												virtualTokens[k]--;
												tokenCostInThisFunction[k]--;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		int outstandingTokensOwed = 0;
		for (int n = 0; n < tokenCostInThisFunction.length; n++)
		{
			testAbortScript();
			outstandingTokensOwed += tokenCostInThisFunction[n];
		}
		
		if (outstandingTokensOwed == 0)
		{
			hasTheTokens = true;
		}
		
		return hasTheCredits && hasTheTokens;
	}
	
	
	public void processItemPurchase(obj_id self, obj_id player) throws InterruptedException
	{
		obj_id inventory = utils.getInventoryContainer(player);
		obj_id[] inventoryContents = getInventoryAndEquipment(player);
		
		int creditCost = getIntObjVar(self, "item.object_for_sale.cash_cost");
		int[] tokenCostForReals = getIntArrayObjVar(self, "item.object_for_sale.token_cost");
		
		String myName = static_item.getStaticItemName(self);
		
		obj_id purchasedItem = static_item.createNewItemFunction(myName, inventory);
		
		if (hasScript(purchasedItem, "npc.faction_recruiter.biolink_item"))
		{
			setBioLink(purchasedItem, player);
		}
		
		CustomerServiceLog("Heroic-Token: ", "player "+ getFirstName(player) + "("+ player + ") purchased item "+ myName + "("+ purchasedItem + ")");
		
		if (!exists(purchasedItem))
		{
			sendSystemMessage(player, new string_id("set_bonus", "vendor_cant_purchase"));
			return;
		}
		
		String readableName = getString(parseNameToStringId(getName(self), self));
		prose_package pp = new prose_package();
		pp = prose.setStringId(pp, new string_id("set_bonus", "vendor_item_purchased"));
		pp = prose.setTT(pp, readableName);
		sendSystemMessageProse(player, pp);
		
		if (creditCost >= 1)
		{
			
			obj_id containedBy = getContainedBy(getContainedBy(getContainedBy(self)));
			money.requestPayment(player, containedBy, creditCost, "no_handler", null, false);
		}
		
		boolean foundTokenHolderBox = false;
		
		for (int i = 0; i < inventoryContents.length; i++)
		{
			testAbortScript();
			String itemName = getStaticItemName(inventoryContents[i]);
			if (itemName != null && !itemName.equals(""))
			{
				
				if (hasObjVar(self, VENDOR_TOKEN_TYPE))
				{
					String tokenList = getStringObjVar(self, VENDOR_TOKEN_TYPE);
					String[] differentTokens = split(tokenList, ',');
					
					for (int j = 0; j < differentTokens.length; j++)
					{
						testAbortScript();
						if (itemName.equals(differentTokens[j]) && tokenCostForReals[j] > 0)
						{
							if (getCount(inventoryContents[i]) > 1)
							{
								int numInStack = getCount(inventoryContents[i]);
								
								for (int m = 0; m < numInStack-1; m++)
								{
									testAbortScript();
									if (tokenCostForReals[j] > 0)
									{
										tokenCostForReals[j]--;
										setCount(inventoryContents[i], getCount(inventoryContents[i]) - 1);
									}
								}
								
							}
							
							if (getCount(inventoryContents[i]) <= 1 && tokenCostForReals[j] > 0)
							{
								destroyObject(inventoryContents[i]);
								tokenCostForReals[j]--;
							}
						}
					}
				}
				else
				{
					
					for (int j = 0; j < trial.HEROIC_TOKENS.length; j++)
					{
						testAbortScript();
						if (itemName.equals(trial.HEROIC_TOKENS[j]) && tokenCostForReals[j] > 0)
						{
							if (getCount(inventoryContents[i]) > 1)
							{
								int numInStack = getCount(inventoryContents[i]);
								
								for (int m = 0; m < numInStack-1; m++)
								{
									testAbortScript();
									if (tokenCostForReals[j] > 0)
									{
										tokenCostForReals[j]--;
										setCount(inventoryContents[i], getCount(inventoryContents[i]) - 1);
									}
								}
								
							}
							
							if (getCount(inventoryContents[i]) <= 1 && tokenCostForReals[j] > 0)
							{
								destroyObject(inventoryContents[i]);
								tokenCostForReals[j]--;
							}
						}
					}
					
					if (!foundTokenHolderBox && itemName.equals("item_heroic_token_box_01_01"))
					{
						foundTokenHolderBox = true;
						
						if (hasObjVar(inventoryContents[i], "item.set.tokens_held"))
						{
							int[] virtualTokens = getIntArrayObjVar(inventoryContents[i], "item.set.tokens_held");
							for (int k = 0; k < trial.HEROIC_TOKENS.length; k++)
							{
								testAbortScript();
								if (tokenCostForReals[k] > 0 && virtualTokens[k] > 0)
								{
									
									int paymentCounter = tokenCostForReals[k];
									for (int l = 0; l < paymentCounter; l++)
									{
										testAbortScript();
										if (virtualTokens[k] > 0)
										{
											virtualTokens[k]--;
											tokenCostForReals[k]--;
										}
									}
								}
							}
							setObjVar(inventoryContents[i], "item.set.tokens_held", virtualTokens);
						}
					}
				}
				
			}
		}
		
		return;
	}
	
	
	public string_id parseNameToStringId(String itemName, obj_id item) throws InterruptedException
	{
		String[] parsedString = split(itemName, ':');
		
		string_id itemNameSID;
		
		if (static_item.isStaticItem(item))
		{
			itemNameSID = static_item.getStaticItemStringIdName(item);
		}
		
		else if (parsedString.length > 1)
		{
			String stfFile = parsedString[0];
			String reference = parsedString[1];
			itemNameSID = new string_id(stfFile, reference);
		}
		else
		{
			String stfFile = parsedString[0];
			itemNameSID = new string_id(stfFile, " ");
		}
		
		return itemNameSID;
		
	}
}
