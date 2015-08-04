package script.terminal;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.player_structure;
import script.library.vendor_lib;
import script.library.static_item;



public class nonvendor extends script.terminal.base.base_terminal
{
	public nonvendor()
	{
	}
	public static final boolean LOGGING_ON = true;
	public static final String LOGGING_CATEGORY = "nonvendor";
	
	public static final string_id SID_FACE_ME = new string_id("player_vendor", "face_me");
	public static final string_id SID_RECOVER = new string_id("player_vendor", "recover_nonvendor");
	public static final string_id SID_MAKE_ROOM = new string_id("player_vendor", "recover_nonvendor_make_room");
	public static final string_id SID_NO_USE_WHILE_DEAD = new string_id("player_structure", "while_dead");
	
	
	public int OnDestroy(obj_id self) throws InterruptedException
	{
		if (!isValidId(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (utils.hasScriptVar(self, "messageCompleted"))
		{
			return SCRIPT_CONTINUE;
		}
		
		int currentTime = getGameTime();
		dictionary msgparams = new dictionary();
		msgparams.put("time", currentTime);
		utils.setScriptVar(self, "messageTime", currentTime);
		utils.setScriptVar(self, "messageCompleted", true);
		messageTo(self, "handleDestroyNonVendor", msgparams, 0, false);
		CustomerServiceLog("tcg", "TCG NonVendor: "+self+" is about to be destroyed and given to the owner. The nonvendor is being destroyed (probably via structure terminal).");
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnAboutToBeTransferred(obj_id self, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		obj_id ownerId = getObjIdObjVar(self, vendor_lib.GREETER_OWNER_OBJVAR);
		if (!isValidId(ownerId))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (transferer != ownerId)
		{
			return SCRIPT_OVERRIDE;
		}
		
		int currentTime = getGameTime();
		dictionary msgparams = new dictionary();
		msgparams.put("time", currentTime);
		utils.setScriptVar(self, "messageTime", currentTime);
		utils.setScriptVar(self, "messageCompleted", true);
		messageTo(self, "handleDestroyNonVendor", msgparams, 0, false);
		CustomerServiceLog("tcg", "TCG NonVendor: "+self+" is about to be destroyed and given to: "+ ownerId + ". The nonvendor is being transferred (probably picked up).");
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		if (isIncapacitated(player) || isDead(player))
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		blog("terminal.nonvendor.OnObjectMenuRequest init");
		obj_id ownerId = getObjIdObjVar(self, vendor_lib.GREETER_OWNER_OBJVAR);
		if (!isValidId(ownerId) || !exists(ownerId))
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		if (player != ownerId)
		{
			return super.OnObjectMenuRequest(self, player, mi);
		}
		
		return super.OnObjectMenuRequest(self, player, mi);
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return SCRIPT_CONTINUE;
		}
		if (isIncapacitated(player) || isDead(player))
		{
			return SCRIPT_CONTINUE;
		}
		
		blog("terminal.nonvendor.OnObjectMenuSelect init");
		
		obj_id ownerId = getObjIdObjVar(self, vendor_lib.GREETER_OWNER_OBJVAR);
		if (!isValidId(ownerId) || !exists(ownerId))
		{
			return SCRIPT_CONTINUE;
		}
		else if (player != ownerId)
		{
			return SCRIPT_CONTINUE;
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int handleDestroyNonVendor(obj_id self, dictionary params) throws InterruptedException
	{
		if (params == null || params.isEmpty())
		{
			return SCRIPT_CONTINUE;
		}
		
		int time = params.getInt("time");
		if (time <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		int messageTime = utils.getIntScriptVar(self, "messageTime");
		if (messageTime <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (time != messageTime)
		{
			return SCRIPT_CONTINUE;
		}
		
		obj_id ownerId = getObjIdObjVar(self, vendor_lib.GREETER_OWNER_OBJVAR);
		if (!isValidId(ownerId))
		{
			utils.removeScriptVar(self, "messageTime");
			return SCRIPT_CONTINUE;
		}
		CustomerServiceLog("tcg", "TCG NonVendor "+self+ " is about to be destroyed. The owner of this TCG item is: "+ownerId+".");
		
		String staticItem = getStringObjVar(self, vendor_lib.STATIC_ITEM_DEED_NAME);
		if (staticItem == null || staticItem.equals(""))
		{
			utils.removeScriptVar(self, "messageTime");
			return SCRIPT_CONTINUE;
		}
		
		dictionary reimburseParams = new dictionary();
		reimburseParams.put("reimburseItem", staticItem);
		
		blog("terminal.nonvendor.handleDestroyNonVendor OWNER ID: "+ ownerId);
		
		if (!ownerId.isLoaded())
		{
			blog("terminal.nonvendor.handleDestroyNonVendor OWNER OFFLINE");
			CustomerServiceLog("tcg", "TCG NonVendor "+self+" owner "+ownerId+ " is not online. The TCG NonVendor will be reimbursed at next login.");
			messageTo(ownerId, "reimburseTcgItem", reimburseParams, 0, true);
		}
		else
		{
			blog("terminal.nonvendor.handleDestroyNonVendor OWNER LOADED");
			
			obj_id pInv = utils.getInventoryContainer(ownerId);
			if (!isValidId(pInv) || !exists(pInv))
			{
				CustomerServiceLog("tcg", "TCG NonVendor: "+self+" could not be destroyed properly because owner: "+ ownerId + " had no player inventory container available. We have sent a message to reimburse the player at another time.");
				messageTo(ownerId, "reimburseTcgItem", reimburseParams, 0, true);
				destroyObject(self);
				return SCRIPT_CONTINUE;
			}
			
			obj_id newItem = static_item.createNewItemFunction(staticItem, pInv);
			if (!isValidId(newItem) || !exists(newItem))
			{
				CustomerServiceLog("tcg", "TCG NonVendor: "+self+" could not be destroyed for owner: "+ ownerId + " because the deed could not be created properly. We have sent a message to reimburse the player at another time.");
				messageTo(ownerId, "reimburseTcgItem", reimburseParams, 0, true);
				destroyObject(self);
				return SCRIPT_CONTINUE;
			}
			CustomerServiceLog("tcg", "TCG NonVendor "+self+" will be destroyed because the owner: "+ ownerId + " has received a new deed for the nonvendor destruction. The object: "+newItem);
		}
		
		destroyObject(self);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnPack(obj_id self, dictionary params) throws InterruptedException
	{
		CustomerServiceLog("tcg", "TCG NonVendor "+self+ " is being packed up.");
		
		location loc = getLocation(self);
		if (loc != null && isValidId(loc.cell))
		{
			CustomerServiceLog("tcg", "TCG NonVendor "+self+ " is being packed up. Current Location: "+loc);
			
			obj_id structure = player_structure.getStructure(self);
			if (isValidId(structure))
			{
				CustomerServiceLog("tcg", "TCG NonVendor "+self+" currently resides in "+structure);
				obj_id structureOwner = getOwner(structure);
				if (isValidId(structureOwner))
				{
					CustomerServiceLog("tcg", "TCG NonVendor "+self+" currently resides in "+structure+ " that is owned by "+structureOwner);
					obj_id ownerId = getObjIdObjVar(self, vendor_lib.GREETER_OWNER_OBJVAR);
					if (isValidId(ownerId) && ownerId != structureOwner)
					{
						CustomerServiceLog("tcg", "TCG NonVendor "+self+" is being packed up in a house that the NonVendor owner "+ownerId+" doesn't own.");
					}
				}
			}
		}
		int currentTime = getGameTime();
		dictionary msgparams = new dictionary();
		msgparams.put("time", currentTime);
		utils.setScriptVar(self, "messageTime", currentTime);
		utils.setScriptVar(self, "messageCompleted", true);
		messageTo(self, "handleDestroyNonVendor", msgparams, 0, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (LOGGING_ON && msg != null && !msg.equals(""))
		{
			LOG(LOGGING_CATEGORY, msg);
		}
		return true;
	}
}
