package script.player;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.healing;
import script.library.dot;
import script.library.utils;
import script.library.consumable;
import script.library.pet_lib;
import script.library.group;
import script.library.pclib;
import script.library.combat;
import script.library.colors;
import script.library.sui;


public class player_gm extends script.base_script
{
	public player_gm()
	{
	}
	public static final string_id SID_HEAL_DAMAGE = new string_id("sui", "heal_damage");
	public static final string_id SID_HEAL_WOUND = new string_id("sui", "heal_wound");
	
	public static final string_id SID_FLY_DRAG = new string_id("base_player","fly_drag");
	
	
	public int OnInitialize(obj_id self) throws InterruptedException
	{
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int cmdGenerateCraftedItem(obj_id self, obj_id target, String params, float defaultTime) throws InterruptedException
	{
		if (!isGod(self))
		{
			return SCRIPT_CONTINUE;
		}
		sendSystemMessageTestingOnly(self, "hello");
		
		LOG("LOG_CHANNEL", "player_gm.generateCraftedItem commandHandler called. IsGod check passed, and handler is executing.");
		
		debugServerConsoleMsg(self, "************ Entered cmdGenerateCraftedItem.");
		
		java.util.StringTokenizer st = new java.util.StringTokenizer(params);
		if (st.countTokens() != 2)
		{
			sendSystemMessageTestingOnly(self, "Did not find the correct parameters needed to create an item.");
			sendSystemMessageTestingOnly(self, "/generateCraftedItem command must at least have schematic template name and attribute percentage paramaters to function.");
			sendSystemMessageTestingOnly(self, "Correct usage is /generateCraftedItem <schematic name> <quality-percentage(integer)>");
			debugServerConsoleMsg(self, "************ Did not decode any string tokens from params passed into command handler. Unable to proceed with out at least schematic template name and attribute percentage.");
			return SCRIPT_CONTINUE;
		}
		sendSystemMessageTestingOnly(self, "hello");
		
		String template_string = (st.nextToken()).toLowerCase();
		String schematic = ("object/draft_schematic/"+ template_string + ".iff");
		
		sendSystemMessageTestingOnly(self, "template_string: "+template_string);
		sendSystemMessageTestingOnly(self, "schematic: "+schematic);
		
		String percentage_string = (st.nextToken()).toLowerCase();
		int attributePercentage = utils.stringToInt(percentage_string);
		if (attributePercentage == -1)
		{
			LOG("LOG_CHANNEL", "You must specify a valid item attribute percentage.");
			sendSystemMessageTestingOnly(self, "You must specify a valid item attribute percentage.");
			return SCRIPT_CONTINUE;
		}
		
		obj_id creationTarget = null;
		if (!isIdValid(target))
		{
			creationTarget = getLookAtTarget(self);
			if (!isIdValid(creationTarget))
			{
				creationTarget = self;
			}
		}
		else
		{
			creationTarget = target;
		}
		obj_id inventory = utils.getInventoryContainer(creationTarget);
		
		if (inventory != null)
		{
			obj_id item = makeCraftedItem(schematic, attributePercentage, inventory);
			sendSystemMessageTestingOnly(self, "Item created and placed into the inventory of "+getName(creationTarget));
			CustomerServiceLog("generateCraftedItem","Object obj_id "+ item + " was created of type "+ schematic + ". It was created in the inventory of object "+ creationTarget +" which is named "+ getName(creationTarget) +".");
			debugServerConsoleMsg(self, "Object obj_id "+ item + " was created of type "+ schematic + ". It was created in the inventory of object "+ creationTarget +" which is named "+ getName(creationTarget) +".");
		}
		return SCRIPT_CONTINUE;
	}
	
}
