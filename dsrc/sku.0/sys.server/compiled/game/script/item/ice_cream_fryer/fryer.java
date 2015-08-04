package script.item.ice_cream_fryer;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import java.util.HashSet;
import script.library.collection;
import script.library.create;
import script.library.prose;
import script.library.static_item;
import script.library.sui;
import script.library.utils;


public class fryer extends script.base_script
{
	public fryer()
	{
	}
	public static final int NO_DECAY = 100;
	public static final int MAX_DECAY = 6;
	public static final int BASIC_BUFF_ROWS = 3;
	
	public static final String ITEM_COMBINATION_TABLE = "datatables/item/ice_cream_fryer/fryer_components.iff";
	
	public static final String STRING_FILE = "dispenser";
	public static final string_id SID_BIOLINK_NEEDED = new string_id(STRING_FILE,"biolink_needed");
	public static final string_id SID_ACTIVATE = new string_id(STRING_FILE, "fry");
	public static final string_id SID_NOT_OWNER = new string_id(STRING_FILE,"not_owner");
	public static final string_id SID_FRYER_FAILURE = new string_id(STRING_FILE,"fryer_failure");
	public static final string_id SID_FRYER_SUCCESS = new string_id(STRING_FILE,"fryer_successful");
	public static final string_id SID_FRYER_BROKEN = new string_id(STRING_FILE,"fryer_broken");
	public static final string_id SID_FRYER_REPAIRED = new string_id(STRING_FILE,"fryer_repaired");
	public static final string_id SID_NO_TOOL_FRYER = new string_id(STRING_FILE,"no_tool_fryer");
	
	public static final String OWNER_OID = "owner";
	public static final String DECAY = "decay";
	public static final String FORAGE_BUFF = "item_ice_cream_buff_forage_buff_01_01";
	public static final String PID_NAME = "fryer_repair";
	public static final int DOMESTICS_ONLY_BUFF = 7;
	
	
	public int OnReceivedItem(obj_id self, obj_id srcContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		
		if (getTemplateName(item).startsWith("object/tangible/component/food/component_fryer_repair_tool.iff"))
		{
			CustomerServiceLog("ice_cream_fryer", "Ice Cream Fryer Container "+srcContainer+" has received a repair tool from "+ transferer);
			
			repairFryerPopup(self, item, transferer);
			return SCRIPT_CONTINUE;
		}
		
		double ingredientCRC = 0;
		if (utils.hasScriptVar(self,"ingredientCRC"))
		{;
		}
		{
			ingredientCRC = utils.getDoubleScriptVar(self, "ingredientCRC");
			blog("OnReceivedItem - found a pre-existing CRC. It was: "+ ingredientCRC);
		}
		
		ingredientCRC = ingredientCRC + getStringCrc(getTemplateName(item));
		
		blog("OnReceivedItem - calculating a new CRC. New item was: "+getStringCrc(getTemplateName(item))+" and new total is: "+ingredientCRC);
		
		utils.setScriptVar(self, "ingredientCRC", ingredientCRC);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnLostItem(obj_id self, obj_id destContainer, obj_id transferer, obj_id item) throws InterruptedException
	{
		
		blog("OnLostItem - entered");
		CustomerServiceLog("ice_cream_fryer", "Ice Cream Fryer has lost item "+item+" as it was transferred to container: "+destContainer+ " by "+transferer);
		
		double ingredientCRC = utils.getDoubleScriptVar(self, "ingredientCRC");
		ingredientCRC = ingredientCRC - getStringCrc(getTemplateName(item));
		
		if (ingredientCRC < 0)
		{
			utils.setScriptVar(self, "ingredientCRC", 0);
		}
		else
		{
			utils.setScriptVar(self, "ingredientCRC", ingredientCRC);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		obj_id fryer = self;
		
		if (!hasObjVar(fryer, collection.ICE_CREAM_BUFF_ARRAY[2]))
		{
			if (!burnRandomRecipes(fryer))
			{
				CustomerServiceLog("ice_cream_fryer", "Ice Cream Fryer has failed to receive recipes when player "+player+ " attempted to interact with it for the first time.");
				blog("burnRandomRecipes - FAILED");
				return SCRIPT_CONTINUE;
			}
			
		}
		if (utils.isNestedWithin(fryer, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.isInHouseCellSpace(fryer))
		{
			mi.addRootMenu(menu_info_types.SERVER_MENU1, SID_ACTIVATE);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		
		blog("OnObjectMenuSelect - entered");
		obj_id fryer = self;
		
		obj_id biolink = getBioLink(fryer);
		
		if (!isValidId(biolink) || biolink == utils.OBJ_ID_BIO_LINK_PENDING)
		{
			blog("activateFryer - FRYER BIOLINK PENDING");
			sendSystemMessage(player, SID_BIOLINK_NEEDED);
			return SCRIPT_CONTINUE;
		}
		else if (biolink != player)
		{
			sendSystemMessage(player, SID_NOT_OWNER);
			return SCRIPT_CONTINUE;
		}
		
		if (!hasObjVar(fryer, collection.ICE_CREAM_BUFF_ARRAY[2]))
		{
			if (!burnRandomRecipes(fryer))
			{
				blog("burnRandomRecipes - FAILED");
				return SCRIPT_CONTINUE;
			}
			
		}
		if (utils.isNestedWithin(fryer, player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item == menu_info_types.SERVER_MENU1)
		{
			obj_id clients[] = new obj_id[1];
			clients[0] = player;
			
			blog("OnObjectMenuSelect - chose menu option 1");
			
			blog("OnObjectMenuSelect - activating the assembly tool");
			obj_id newItem = activateFryer(self, player);
			if (isIdValid(newItem))
			{
				playClientEffectObj(clients, "sound/item_som_jenha_tar_cube_combine.snd", player, "");
				sendSystemMessage(player,SID_FRYER_SUCCESS);
				decayFryer(fryer, player);
				return SCRIPT_CONTINUE;
			}
			else
			{
				blog("OnObjectMenuSelect - obj_id we got back was invalid: "+newItem);
				playClientEffectObj(clients, "sound/item_electronics_break.snd", player, "");
				sendSystemMessage(player, SID_FRYER_FAILURE);
				decayFryer(fryer, player);
				return SCRIPT_CONTINUE;
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		
		int idx = utils.getValidAttributeIndex(names);
		if (idx == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		
		int currentStatus = getIntObjVar(self, DECAY);
		
		names[idx] = "fryer_status";
		attribs[idx] = ""+currentStatus+"%";
		idx++;
		
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public obj_id activateFryer(obj_id fryer, obj_id objPlayer) throws InterruptedException
	{
		blog("activateFryer - entered");
		obj_id biolink = getBioLink(fryer);
		
		if (!isValidId(biolink) || biolink == utils.OBJ_ID_BIO_LINK_PENDING)
		{
			blog("activateFryer - FRYER BIOLINK PENDING");
			sendSystemMessage(objPlayer, SID_BIOLINK_NEEDED);
			return null;
		}
		
		else if (biolink != objPlayer)
		{
			sendSystemMessage(objPlayer, SID_NOT_OWNER);
			return null;
		}
		
		int currentStatus = getIntObjVar(fryer, DECAY);
		if (currentStatus <= 0)
		{
			sendSystemMessage(objPlayer, SID_FRYER_BROKEN);
			blog("activateFryer - FRYER BROKEN");
			return null;
		}
		
		double ingredientCRC = calcCurrentContentsCRC(fryer);
		if (ingredientCRC == 0)
		{
			blog("activateFryer - no ingredientCRC. Returning null");
			decayFryer(fryer, objPlayer);
			return null;
		}
		
		if (!utils.hasScriptVar(fryer, "itemCRCs"))
		{
			blog("activateFryer - made the crc table and stored it");
			utils.setScriptVar(fryer, "itemCRCs", makeItemCRCArray());
		}
		
		double[] itemCRCs = utils.getDoubleArrayScriptVar(fryer, "itemCRCs");
		
		for (int i = 0; i < itemCRCs.length; i++)
		{
			testAbortScript();
			blog("activateFryer - looping through stored CRC array. This is loop number: "+i+" and there will be a total of around: "+itemCRCs.length);
			
			if (itemCRCs[i] != ingredientCRC)
			{
				continue;
			}
			
			blog("activateFryer - Found a match in loop # "+i);
			blog("activateFryer - CRC FOUND # "+itemCRCs[i]);
			blog("activateFryer - CRC LOOKING FOR "+ingredientCRC);
			
			dictionary correctRow = dataTableGetRow(ITEM_COMBINATION_TABLE, i);
			obj_id contents[] = getContents(fryer);
			
			obj_id generatedItem = createEdible(fryer, objPlayer, i);
			if (!isIdValid(generatedItem))
			{
				return null;
			}
			
			CustomerServiceLog("ice_cream_fryer", "%TU has created an ice cream buff (id: "+ generatedItem + ")", objPlayer);
			
			for (int j = 0; j< contents.length; j++)
			{
				testAbortScript();
				blog("activateFryer - Going through each thing in the assembly tool. this is loop #: ->: "+j);
				
				if (contents[j] != generatedItem)
				{
					blog("activateFryer - found something that doesn't match the obj_id of the new item. Destroying it");
					destroyObject(contents[j]);
					
				}
			}
			
			putInOverloaded(generatedItem, fryer);
			
			if (!hasObjVar(generatedItem, collection.FRYER_BUFF_TYPE) && getIntObjVar(generatedItem, collection.FRYER_BUFF_TYPE) != collection.ICE_CREAM_DOMESTICS_ONLY)
			{
				setObjVar(generatedItem, OWNER_OID, objPlayer);
			}
			return generatedItem;
		}
		return obj_id.NULL_ID;
	}
	
	
	public double[] makeItemCRCArray() throws InterruptedException
	{
		blog("makeItemCRCArray - entered");
		int numRows = dataTableGetNumRows(ITEM_COMBINATION_TABLE);
		blog("makeItemCRCArray - there are this many rows in the table -> "+numRows);
		
		double[] crcs = new double[numRows];
		
		for (int i = 0; i<numRows; i++)
		{
			testAbortScript();
			blog("makeItemCRCArray - reading in row number: "+i);
			
			dictionary currentRow = dataTableGetRow(ITEM_COMBINATION_TABLE, i);
			for (int j = 1; j <=4; j++)
			{
				testAbortScript();
				String currentObject = currentRow.getString("item"+j);
				blog("makeItemCRCArray - currentObject is: "+currentObject+" it is item# "+j);
				int currentItemCRC = getStringCrc(currentObject);
				crcs[i]+= currentItemCRC;
			}
		}
		return crcs;
	}
	
	
	public obj_id createEdible(obj_id fryer, obj_id player, int row) throws InterruptedException
	{
		if (!isValidId(fryer) || !isValidId(player))
		{
			return obj_id.NULL_ID;
		}
		
		obj_id generatedItem = obj_id.NULL_ID;
		
		if (row < BASIC_BUFF_ROWS)
		{
			generatedItem = static_item.createNewItemFunction(FORAGE_BUFF, player);
			boolean success = setFourthIngredient(fryer, generatedItem, row);
			blog("setBuffTypeOnEdible - SETTING BUFF ON EDIBLE TO HARVEST BUFF: ");
			return generatedItem;
		}
		
		for (int i = 2; i < (collection.ICE_CREAM_BUFF_ARRAY.length); i++)
		{
			testAbortScript();
			
			String loopStr = getStringObjVar(fryer, collection.ICE_CREAM_BUFF_ARRAY[i]);
			blog("setBuffTypeOnEdible - Grabbing Int Val off of Fryer for: "+collection.ICE_CREAM_BUFF_ARRAY[i]+" STRING value is: "+loopStr);
			int loopInt = utils.stringToInt(loopStr);
			blog("setBuffTypeOnEdible - Grabbing Int Val off of Fryer for: "+collection.ICE_CREAM_BUFF_ARRAY[i]+" INT value is: "+loopInt);
			blog("setBuffTypeOnEdible - ROW: "+row+" loopInt: "+loopInt);
			if (loopInt == row)
			{
				String currentBuff = collection.ICE_CREAM_BUFF_ARRAY[i];
				String domesticBuff = collection.ICE_CREAM_BUFF_ARRAY[DOMESTICS_ONLY_BUFF];
				
				if (!currentBuff.equals(domesticBuff))
				{
					blog("setBuffTypeOnEdible - The player created a buff edible. Set a buff type: "+i);
					generatedItem = static_item.createNewItemFunction(currentBuff, player);
					boolean success = setFourthIngredient(fryer, generatedItem, row);
					return generatedItem;
				}
				else
				{
					
					blog("setBuffTypeOnEdible - The player getSkillTemplate: "+getSkillTemplate(player));
					if (getSkillTemplate(player).startsWith("trader_0a"))
					{
						blog("setBuffTypeOnEdible - DOMESTICS BUFF BEING CREATED: "+i);
						generatedItem = static_item.createNewItemFunction(currentBuff, player);
						setObjVar(generatedItem, collection.FRYER_BUFF_TYPE, collection.ICE_CREAM_DOMESTICS_ONLY);
						boolean success = setFourthIngredient(fryer, generatedItem, row);
						return generatedItem;
					}
					blog("setBuffTypeOnEdible - DOMESTICS BUFF FAILED: ");
				}
			}
		}
		
		boolean success = setFourthIngredient(fryer, generatedItem, row);
		generatedItem = static_item.createNewItemFunction(collection.ICE_CREAM_BUFF_ARRAY[0], player);
		return generatedItem;
	}
	
	
	public boolean setFourthIngredient(obj_id fryer, obj_id generatedItem, int row) throws InterruptedException
	{
		if (!isValidId(fryer) || !isValidId(generatedItem))
		{
			return false;
		}
		
		dictionary dic = dataTableGetRow(ITEM_COMBINATION_TABLE, row);
		blog("setBuffTypeOnEdible - getting Dictionary "+dic);
		
		String fourthComponent = dic.getString("item4");
		blog("setBuffTypeOnEdible - burning fourthComponent "+fourthComponent);
		setObjVar(generatedItem, "special_ingredient", fourthComponent);
		return true;
	}
	
	
	public String[] getRandomRowNumbersWithoutDuplicates(obj_id fryer) throws InterruptedException
	{
		if (!isValidId(fryer))
		{
			return null;
		}
		
		int datatableLength = dataTableGetNumRows(ITEM_COMBINATION_TABLE);
		HashSet randomRows = new HashSet();
		
		while (randomRows.size() < (collection.ICE_CREAM_BUFF_ARRAY.length - 2))
		{
			testAbortScript();
			int randomNum = rand(BASIC_BUFF_ROWS, datatableLength);
			randomRows.add(""+randomNum);
			blog("getRandomRowNumbersWithoutDuplicates - adding this to hash: "+randomNum);
		}
		
		String[] allNumbers = new String[randomRows.size()];
		randomRows.toArray(allNumbers);
		blog("getRandomRowNumbersWithoutDuplicates - sending list of random numbers with length of: "+allNumbers.length);
		
		return allNumbers;
	}
	
	
	public boolean burnRandomRecipes(obj_id fryer) throws InterruptedException
	{
		if (!isValidId(fryer))
		{
			return false;
		}
		
		setObjVar(fryer, DECAY, NO_DECAY);
		
		String[] uniqueRandomRowNumbers = getRandomRowNumbersWithoutDuplicates(fryer);
		if (uniqueRandomRowNumbers == null)
		{
			return false;
		}
		else if ((collection.ICE_CREAM_BUFF_ARRAY.length - 2) != uniqueRandomRowNumbers.length)
		{
			return false;
		}
		
		blog("burnRandomRecipes - Received uniqueRandomRowNumbers.length: "+uniqueRandomRowNumbers.length);
		
		for (int i = 0; i< uniqueRandomRowNumbers.length; i++)
		{
			testAbortScript();
			String uniqueRow = uniqueRandomRowNumbers[i];
			int uniqueRowInt = utils.stringToInt(uniqueRow);
			if (uniqueRowInt < 0)
			{
				blog("burnRandomRecipes - FAILED to convert string var into integer: "+uniqueRow);
				return false;
			}
			
			blog("burnRandomRecipes - burning ICE_CREAM_BUFF_ARRAY "+collection.ICE_CREAM_BUFF_ARRAY[i+2]+" with row: "+uniqueRow);
			setObjVar(fryer, collection.ICE_CREAM_BUFF_ARRAY[i+2], uniqueRow);
			
			blog("burnRandomRecipes - burning uniqueRandomRowNumbers[i] "+uniqueRow);
			
			dictionary dic = dataTableGetRow(ITEM_COMBINATION_TABLE, uniqueRowInt);
			blog("burnRandomRecipes - burning Dictionary "+dic);
			
			String fourthComponent = dic.getString("item4");
			blog("burnRandomRecipes - burning fourthComponent "+fourthComponent);
			
			setObjVar(fryer, collection.ICE_CREAM_BUFF_ARRAY[i+2]+"_component", fourthComponent);
		}
		
		return true;
	}
	
	
	public double calcCurrentContentsCRC(obj_id fryer) throws InterruptedException
	{
		if (!isValidId(fryer))
		{
			return 0;
		}
		
		blog("calcCurrentContentsCRC - entered");
		double ingredientCRC = 0;
		
		obj_id contents[] = getContents(fryer);
		blog("calcCurrentContentsCRC - Got contents of the assembly tool. There are this many things: ->: "+contents.length);
		for (int i = 0; i< contents.length; i++)
		{
			testAbortScript();
			ingredientCRC += getStringCrc(getTemplateName(contents[i]));
			blog("calcCurrentContentsCRC - Loop Number: "+i+" and current ingredient CRC total is: "+ingredientCRC);
		}
		return ingredientCRC;
	}
	
	
	public boolean decayFryer(obj_id fryer, obj_id user) throws InterruptedException
	{
		if (!isValidId(fryer))
		{
			return false;
		}
		
		obj_id biolink = getBioLink(fryer);
		
		if (!isValidId(biolink) || biolink == utils.OBJ_ID_BIO_LINK_PENDING)
		{
			return false;
		}
		
		if (biolink != user)
		{
			return false;
		}
		
		int newDecay = 0;
		int currentStatus = getIntObjVar(fryer, DECAY);
		if (currentStatus > 0)
		{
			int decayAmount = rand(0, MAX_DECAY);
			if (decayAmount == 0)
			{
				return true;
			}
			
			if (decayAmount > currentStatus)
			{
				setObjVar(fryer, DECAY, 0);
				blog("decayFryer - DECAY SET TO ZERO");
				return true;
			}
			
			newDecay = currentStatus - decayAmount;
			if (newDecay > 0)
			{
				setObjVar(fryer, DECAY, newDecay);
				blog("decayFryer - DECAY SET TO: "+newDecay);
				return true;
			}
		}
		if (currentStatus < 0)
		{
			setObjVar(fryer, DECAY, 0);
		}
		
		return false;
	}
	
	
	public boolean repairFryerPopup(obj_id fryer, obj_id repairTool, obj_id player) throws InterruptedException
	{
		if (!isValidId(fryer) || !isValidId(repairTool) || !isValidId(player))
		{
			return false;
		}
		
		if (sui.hasPid(player, PID_NAME))
		{
			int pid = sui.getPid(player, PID_NAME);
			forceCloseSUIPage(pid);
		}
		
		utils.setScriptVar(fryer, "repairTool", repairTool);
		int pid = sui.msgbox(fryer, player, "Selecting YES will repair the fryer. Selecting NO will cancel the operation.", sui.YES_NO, "Repair?", "repairFryer");
		sui.setPid(player, pid, PID_NAME);
		
		return true;
	}
	
	
	public int repairFryer(obj_id self, dictionary params) throws InterruptedException
	{
		
		obj_id fryer = self;
		
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id player = sui.getPlayerId(params);
		
		if (!isIdValid(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!sui.hasPid(player, PID_NAME))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.hasScriptVar(fryer, "repairTool"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int bp = sui.getIntButtonPressed(params);
		
		if (bp == sui.BP_CANCEL)
		{
			sui.removePid(player, PID_NAME);
			utils.removeScriptVar(fryer, "repairTool");
			return SCRIPT_CONTINUE;
		}
		
		obj_id repairTool = utils.getObjIdScriptVar(fryer, "repairTool");
		if (!isIdValid(repairTool))
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id toolContainer = getContainedBy(repairTool);
		if (toolContainer != fryer)
		{
			sendSystemMessage(player, SID_NO_TOOL_FRYER);
			return SCRIPT_CONTINUE;
		}
		
		setObjVar(fryer, DECAY, NO_DECAY);
		sendSystemMessage(player, SID_FRYER_REPAIRED);
		destroyObject(repairTool);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String message) throws InterruptedException
	{
		
		return true;
	}
}
