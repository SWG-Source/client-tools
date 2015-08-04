package script.ai;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;
import script.base_script;

import script.library.chat;


public class bark_string_on_initialize extends script.base_script
{
	public bark_string_on_initialize()
	{
	}
	
	public int OnAttach(obj_id self) throws InterruptedException
	{
		messageTo(self, "barkString", null, 1, false);
		return SCRIPT_CONTINUE;
	}
	
	
	public int barkString(obj_id self, dictionary params) throws InterruptedException
	{
		if (!hasObjVar(self, "bark_string_file") || !hasObjVar(self, "bark_string_id"))
		{
			if (!hasObjVar(self, "alreadyTriedBarking"))
			{
				setObjVar(self, "alreadyTriedBarking", 1);
			}
			else
			{
				
				if (getIntObjVar(self, "alreadyTriedBarking") > 2)
				{
					return SCRIPT_CONTINUE;
				}
				
			}
			
			messageTo(self, "barkString", null, 1, false);
			return SCRIPT_CONTINUE;
		}
		
		String stringFile = getStringObjVar(self, "bark_string_file");
		if (stringFile == null || stringFile.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		String stringId = getStringObjVar(self, "bark_string_id");
		if (stringId == null || stringId.length() <= 0)
		{
			return SCRIPT_CONTINUE;
		}
		
		string_id message = new string_id(stringFile, stringId);
		chat.chat(self, message);
		
		return SCRIPT_CONTINUE;
	}
}
