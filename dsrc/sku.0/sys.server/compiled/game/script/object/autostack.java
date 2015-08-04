package script.object;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.loot;
import script.library.utils;
import script.library.sui;
import script.library.static_item;


public class autostack extends script.base_script
{
	public autostack()
	{
	}
	public static final boolean BLOGGING_ON = false;
	public static final String BLOGGING_CATEGORY = "auto_stack";
	
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		
		messageTo(self, "msgStackItem", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnTransferred(obj_id self, obj_id sourceContainer, obj_id destContainer, obj_id transferer) throws InterruptedException
	{
		
		if (getGameObjectType(destContainer) != GOT_misc_factory_crate)
		{
			blog("Sending Stack messageTo msgStackItem");
			messageTo(self, "msgStackItem", null, 1, false);
			
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuRequest(obj_id self, obj_id player, menu_info mi) throws InterruptedException
	{
		if (!canRestack(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getCount(self) > 1)
		{
			mi.addRootMenu (menu_info_types.SERVER_MENU49, new string_id("autostack","unstack"));
			menu_info_data mid = mi.getMenuItemByType(menu_info_types.SERVER_MENU49);
			if (mid == null)
			{
				return SCRIPT_CONTINUE;
			}
			else
			{
				mid.setServerNotify(true);
			}
		}
		
		mi.addRootMenu (menu_info_types.SERVER_MENU50, new string_id("autostack","stack"));
		menu_info_data mid = mi.getMenuItemByType(menu_info_types.SERVER_MENU50);
		if (mid == null)
		{
			return SCRIPT_CONTINUE;
		}
		else
		{
			mid.setServerNotify(true);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnObjectMenuSelect(obj_id self, obj_id player, int item) throws InterruptedException
	{
		if (!canRestack(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (getCount(self) > 1)
		{
			if (item == menu_info_types.SERVER_MENU49)
			{
				unstackIt(self, player);
			}
		}
		
		if (item == menu_info_types.SERVER_MENU50)
		{
			utils.removeScriptVar( player, "autostack.ignoreitems");
			restackIt( self );
		}
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		int free = getFirstFreeIndex(names);
		if (free == -1)
		{
			return super.OnGetAttributes(self, player, names, attribs);
		}
		int numInStack = getCount(self);
		names[free] = "num_in_stack";
		attribs[free] = "" + numInStack;
		return super.OnGetAttributes(self, player, names, attribs);
	}
	
	
	public boolean canRestack(obj_id self) throws InterruptedException
	{
		blog("canRestack init");
		if (isInSecureTrade(self))
		{
			blog("isInSecureTrade");
			return false;
		}
		
		obj_id containerObject = utils.getContainingPlayer(self);
		if (!isIdValid(containerObject))
		{
			if (!isContainerValidForStacking(self))
			{
				return false;
			}
			
			blog("containerObject is okay to use");
			containerObject = getContainedBy(self);
		}
		
		blog("containerObject is valid");
		return true;
	}
	
	
	public void restackIt(obj_id self) throws InterruptedException
	{
		blog("restackIt init");
		
		if (!isIdValid(self) || !exists(self))
		{
			return;
		}
		
		blog("restackIt validation complete");
		
		if (isInSecureTrade(self))
		{
			return;
		}
		
		blog("restackIt - not isInSecureTrade");
		
		obj_id containerObject = utils.getContainingPlayer(self);
		if (!isIdValid(containerObject))
		{
			blog("restackIt - not a player container");
			if (!isContainerValidForStacking(self))
			{
				return;
			}
			
			blog("restackIt - containerObject = getContainedBy");
			containerObject = getContainedBy(self);
		}
		
		blog("restackIt - containerObject = valid");
		if (utils.hasScriptVar(containerObject, "autostack.ignoreitems"))
		{
			return;
		}
		obj_id item = loot.findItemToStack(self);
		
		if (isIdValid(item) && !isInSecureTrade(item))
		{
			if (utils.hasScriptVar(self, "unstacking") || utils.hasScriptVar(item, "unstacking"))
			{
				return;
			}
			
			if (item != self)
			{
				loot.stackItem(self, item);
				if (!hasScript(item, "object.autostack"))
				{
					attachScript(item, "object.autostack");
				}
			}
		}
	}
	
	
	public void unstackIt(obj_id self, obj_id player) throws InterruptedException
	{
		if (!isInSecureTrade(self))
		{
			if (getCount( self ) == 2)
			{
				createNewStack( player, self, 1, 2);
				utils.setScriptVar(self, "unstacking", 1);
				messageTo(self, "clearUnstackingScriptVar", null, 3, false);
				
			}
			else
			{
				sui.inputbox(self, player, "@autostack:stacksize", "msgUnstackItem");
			}
		}
	}
	
	
	public int msgStackItem(obj_id self, dictionary params) throws InterruptedException
	{
		blog("msgStackItem init");
		restackIt( self );
		return SCRIPT_CONTINUE;
	}
	
	
	public int clearUnstackingScriptVar(obj_id self, dictionary params) throws InterruptedException
	{
		utils.removeScriptVar(self, "unstacking");
		return SCRIPT_CONTINUE;
	}
	
	
	public int msgUnstackItem(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id player = sui.getPlayerId(params);
		if (!canRestack(self))
		{
			return SCRIPT_CONTINUE;
		}
		
		String amount_str = sui.getInputBoxText(params);
		if (amount_str == null || amount_str.equals(""))
		{
			
			return SCRIPT_CONTINUE;
		}
		
		int oldStackSize = getCount( self );
		int newStackSize = utils.stringToInt(amount_str);
		if (newStackSize == 0)
		{
			sendSystemMessage( player, new string_id( "autostack", "zero_size"));
			return SCRIPT_CONTINUE;
		}
		else if (newStackSize < 0)
		{
			sendSystemMessage( player, new string_id( "autostack", "number_format_wrong"));
			return SCRIPT_CONTINUE;
		}
		else if (newStackSize > oldStackSize)
		{
			sendSystemMessage( player, new string_id( "autostack", "too_big"));
			return SCRIPT_CONTINUE;
		}
		else if (newStackSize == oldStackSize)
		{
			return SCRIPT_CONTINUE;
		}
		
		createNewStack( player, self, newStackSize, oldStackSize );
		utils.setScriptVar(self, "unstacking", 1);
		messageTo(self, "clearUnstackingScriptVar", null, 3, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public void createNewStack(obj_id player, obj_id item, int newStackSize, int oldStackSize) throws InterruptedException
	{
		obj_id container = getContainedBy(item);
		if (!isIdValid(container))
		{
			sendSystemMessage( player, new string_id( "autostack", "bad_container"));
			return;
		}
		
		if (getVolumeFree(container) <= 0)
		{
			sendSystemMessage( player, new string_id( "autostack", "full_container"));
			return;
		}
		
		utils.setScriptVar( player, "autostack.ignoreitems", true );
		
		obj_id newItem = null;
		
		if (static_item.isStaticItem (item))
		{
			newItem = static_item.createNewItemFunction( getStaticItemName(item), container);
		}
		else
		{
			newItem = createObject( item, container, "");
		}
		
		if (!isIdValid( newItem ))
		{
			sendSystemMessage( player, new string_id( "autostack", "unknown_failure"));
			utils.removeScriptVar( player, "autostack.ignoreitems");
			return;
		}
		setCount( item, (oldStackSize - newStackSize) );
		setCount( newItem, newStackSize );
		utils.removeScriptVar( player, "autostack.ignoreitems");
	}
	
	
	public boolean isContainerValidForStacking(obj_id self) throws InterruptedException
	{
		if (!isValidId(self) || !exists(self))
		{
			return false;
		}
		
		String containerTemplate = getTemplateName(getContainedBy(self));
		blog("isContainerValidForStacking containerTemplate: "+containerTemplate);
		
		if (isGameObjectTypeOf(getContainedBy(self), GOT_misc_container_wearable))
		{
			return true;
		}
		else if (isGameObjectTypeOf(getContainedBy(self), GOT_misc_container))
		{
			if (containerTemplate.startsWith("object/tangible/furniture/") || containerTemplate.startsWith("object/tangible/poi/object/"))
			{
				return true;
			}
			
			return false;
		}
		return false;
	}
	
	
	public boolean blog(String msg) throws InterruptedException
	{
		if (!BLOGGING_ON)
		{
			return false;
		}
		else if (msg == null || msg.equals(""))
		{
			return false;
		}
		
		LOG(BLOGGING_CATEGORY, msg);
		
		return true;
	}
}
