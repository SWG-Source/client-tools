package script.item.wearable;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;



public class set_non_armor_wearable_socket extends script.base_script
{
	public set_non_armor_wearable_socket()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "setSocket", null, 1, false);
		
		return SCRIPT_CONTINUE;
	}
	
	
	public int setSocket(obj_id self, dictionary params) throws InterruptedException
	{
		int socketCount = 0;
		
		if (hasObjVar(self,"socket_count"))
		{
			socketCount = getIntObjVar(self,"socket_count");
		}
		
		if (socketCount > 0)
		{
			setSkillModSockets(self, socketCount);
		}
		
		if (getSkillModSockets(self) > 0)
		{
			detachScript(self, "item.wearable.set_non_armor_wearable_socket");
		}
		
		return SCRIPT_CONTINUE;
	}
}
