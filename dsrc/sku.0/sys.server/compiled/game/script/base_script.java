package script;

import script.*;
import script.base_class.*;
import script.combat_engine.*;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Vector;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;



public class base_script extends script.base_class
{
	public base_script()
	{
	}
	
	public int OnLocateObject(obj_id self, dictionary params) throws InterruptedException
	{
		location l = getLocation(self);
		obj_id requestor = params.getObjId("requestor");
		if (requestor != null)
		{
			String callbackMethod = params.getString("callback");
			if (callbackMethod != null)
			{
				params.put("location", l);
				messageTo(requestor, callbackMethod, params, 0, false);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSetWaypointActive(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id waypoint = params.getObjId("waypoint");
		if (waypoint != null)
		{
			boolean isActive = params.getBoolean("isActive");
			_setWaypointActiveNative(waypoint, isActive);
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnSetWaypointLocation(obj_id self, dictionary params) throws InterruptedException
	{
		obj_id waypoint = params.getObjId("waypoint");
		if (waypoint != null)
		{
			location loc = params.getLocation("loc");
			if (loc != null)
			{
				_setWaypointLocationNative(waypoint, loc);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int getFirstFreeIndex(String[] array) throws InterruptedException
	{
		if (array != null)
		{
			for (int i = 0; i < array.length; i++)
			{
				testAbortScript();
				if (array[i] == null || array[i].length() == 0)
				{
					return i;
				}
			}
		}
		return -1;
	}
	
	
	public void dumpObjvars(obj_id s, obj_var_list objvars, String[] names, String[] attribs) throws InterruptedException
	{
		if (s == null || objvars == null || names == null || attribs == null || names.length != attribs.length)
		{
			return;
		}
		
		int count = objvars.getNumItems();
		int elem = getFirstFreeIndex(names);
		for (int i = 0; i < count && elem >= 0 && elem < names.length; ++i)
		{
			testAbortScript();
			obj_var o = objvars.getObjVar(i);
			if (o != null)
			{
				if (o instanceof obj_var_list)
				{
					dumpObjvars(s, (obj_var_list)o, names, attribs);
					elem = getFirstFreeIndex(names);
				}
				else
				{
					String name = "\""+ o.getName();
					Object v = o.getData();
					if (v != null)
					{
						String val = v.toString();
						names[elem] = name;
						attribs[elem] = val;
						elem++;
					}
				}
			}
		}
	}
	
	
	public void dumpScriptvars(String[] names, String[] attribs) throws InterruptedException
	{
		if ((names == null) || (attribs == null) || (names.length != attribs.length))
		{
			return;
		}
		
		int i = getFirstFreeIndex(names);
		if (i != -1 && i < names.length)
		{
			final obj_id self = getSelf();
			deltadictionary scriptvars = self.getScriptVars();
			java.util.Enumeration keys = scriptvars.keys();
			names[i] = "\"***SCRIPTVARS***";
			attribs[i] = "******";
			i++;
			
			Map sortedValues = new TreeMap();
			while (keys.hasMoreElements() && i < names.length)
			{
				testAbortScript();
				Object k = keys.nextElement();
				String oldname = "\""+ k.toString();
				String name = oldname.replace('.', '+');
				debugServerConsoleMsg(self, "scriptvar name: "+ name);
				Object v = scriptvars.getObject(k);
				String attrib = v.toString();
				
				sortedValues.put(name, attrib);
			}
			
			Iterator keysSet = sortedValues.keySet().iterator();
			
			while (keysSet.hasNext() && i < names.length)
			{
				testAbortScript();
				names[i] = (String)keysSet.next();
				attribs[i] = (String)sortedValues.get((String)names[i]);
				++i;
			}
		}
	}
	
	
	public int OnGetScriptvars(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		
		if (player != null)
		{
			
			ArrayList scripts = self.getScriptArrayList();
			if (scripts == null || scripts.size() == 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			String lastScript = (String)(scripts.get(scripts.size()-1));
			if (lastScript == null)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!lastScript.equals(getClass().getName()))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (isGod(player))
			{
				dumpScriptvars(names, attribs);
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public int OnGetAttributes(obj_id self, obj_id player, String[] names, String[] attribs) throws InterruptedException
	{
		if ((names == null) || (attribs == null) || (names.length != attribs.length))
		{
			return SCRIPT_CONTINUE;
		}
		
		if (player != null)
		{
			
			ArrayList scripts = self.getScriptArrayList();
			if (scripts == null || scripts.size() == 0)
			{
				return SCRIPT_CONTINUE;
			}
			
			String lastScript = (String)(scripts.get(scripts.size()-1));
			if (lastScript == null)
			{
				return SCRIPT_CONTINUE;
			}
			
			if (!lastScript.equals(getClass().getName()))
			{
				return SCRIPT_CONTINUE;
			}
			
			if (isGod(player))
			{
				dumpScriptvars(names, attribs);
				
				final int firstFreeIndex = getFirstFreeIndex(names);
				
				if (( (firstFreeIndex >= 0) && (firstFreeIndex < names.length)))
				{
					names[firstFreeIndex] = "\"***OBJVARS***";
					attribs[firstFreeIndex] = "******";
				}
				
				obj_var_list rootList = getObjVarList(self, "");
				if (rootList != null)
				{
					dumpObjvars(self, rootList, names, attribs);
				}
			}
		}
		return SCRIPT_CONTINUE;
	}
	
	
	public String UploadCharacter(obj_id player) throws InterruptedException
	{
		dictionary params = new dictionary();
		Object[] triggerParams = new Object[2];
		triggerParams[0] = player;
		triggerParams[1] = params;
		
		try
		{
			int err = script_entry.runScripts("OnUploadCharacter", triggerParams);
			if (err == SCRIPT_CONTINUE)
			{
				byte[] packedData = params.pack();
				return new String(packedData);
			}
		}
		catch(Throwable t)
		{
			return null;
		}
		
		return null;
	}
	
	public String DownloadCharacter(obj_id player, String packedData) throws InterruptedException
	{
		byte[] data = packedData.getBytes();
		dictionary params = dictionary.unpack(data);
		
		Object[] triggerParams = new Object[2];
		triggerParams[0] = player;
		triggerParams[1] = params;
		try
		{
			script_entry.runScripts("OnDownloadCharacter", triggerParams);
		}
		catch(Throwable t)
		{
			return null;
		}
		return "OK";
	}
	
}
