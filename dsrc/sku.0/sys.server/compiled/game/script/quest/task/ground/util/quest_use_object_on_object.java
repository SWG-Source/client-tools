package script.quest.task.ground.util;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;


import script.library.utils;
import script.library.static_item;
import script.library.buff;
import script.library.collection;
import script.library.sui;
import script.library.utils;


public class quest_use_object_on_object extends script.base_script
{
	public quest_use_object_on_object()
	{
	}
	public static final string_id SID_USE_CAMERA = new string_id("collection", "use_camera");
	public static final string_id SID_PICTURE_TAKEN = new string_id("collection", "picture_taken");
	public static final string_id SID_INVALID_TARGET = new string_id("collection", "invalid_target");
	public static final string_id SID_SUCCESS_SNAPSHOT = new string_id("collection", "successful_snapshot");
	public static final string_id SID_PHOTO_SLOT_COMPLETE = new string_id("collection", "photo_slot_complete");
	public static final string_id NOT_WHILE_INCAPPED = new string_id ("quest/ground/util/quest_giver_object", "not_while_incapped");
	public static final string_id SID_NOT_WHILE_IN_COMBAT = new string_id("base_player", "not_while_in_combat");
	public static final string_id MUST_DISMOUNT = new string_id ("collection", "must_dismount");
	public static final string_id CREATURE_IS_DEAD = new string_id ("collection", "creature_is_dead");
	
	public static final String OBJECT_KEY = "quest.object_key";
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!canManipulate(player, self, true, true, 15, true))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (!utils.isNestedWithinAPlayer(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.ITEM_USE);
		if (mid != null)
		{
			mid.setServerNotify(true);
		}
		else
		{
			mi.addRootMenu(menu_info_types.ITEM_USE, new string_id("spam", "costume_learn"));
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		sendDirtyObjectMenuNotification(self);
		
		if (utils.getContainingPlayer(self) != player)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (item != menu_info_types.ITEM_USE)
		{
			return SCRIPT_CONTINUE;
		}
		
		if (isIncapacitated(player) || isDead(player))
		{
			sendSystemMessage(player, new string_id("player_structure", "while_dead"));
			return SCRIPT_CONTINUE;
		}
		if (getState(player, STATE_RIDING_MOUNT) == 1)
		{
			sendSystemMessage(player, MUST_DISMOUNT);
			return SCRIPT_CONTINUE;
		}
		if (isDead(player) || isIncapacitated(player))
		{
			sendSystemMessage(player, NOT_WHILE_INCAPPED);
			return SCRIPT_CONTINUE;
		}
		
		obj_id intended = getIntendedTarget(player);
		
		if (!isIdValid(intended))
		{
			
			sendSystemMessage(player, SID_INVALID_TARGET);
		}
		else
		{
			
			checkValidTarget(player, intended, self);
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public boolean checkValidTarget(obj_id player, obj_id target, obj_id self) throws InterruptedException
	{
		if (!isValidId(player) || !exists(player))
		{
			return false;
		}
		
		if (!isValidId(target) || !exists(target))
		{
			return false;
		}
		
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		if (!hasObjVar(self, OBJECT_KEY))
		{
			return false;
		}
		
		if (!hasObjVar(target, OBJECT_KEY))
		{
			sendSystemMessage(player, SID_INVALID_TARGET);
			return false;
		}
		
		String keyString = getStringObjVar(self, OBJECT_KEY);
		if (keyString == null || keyString.length() <= 0)
		{
			return false;
		}
		
		if (!keyString.equals(getStringObjVar(target, OBJECT_KEY)))
		{
			return false;
		}
		
		if (isMob(target) && (isDead(target) || isIncapacitated(target)))
		{
			sendSystemMessage(player, CREATURE_IS_DEAD);
			return false;
		}
		
		return true;
	}
}
