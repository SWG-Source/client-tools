/**
 * Title:        obj_var_list
 * Description:  Wrapper for a list of object variables.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.util.Enumeration;
import java.util.Map.Entry;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;
import java.io.Serializable;

public final class obj_var_list extends obj_var implements Serializable
{
	private final static long serialVersionUID = -1224033321256070223L;

	private static final int DEFAULT_SIZE = (int)(10 / 0.75) + 1;
	private static final char LIST_SEPARATOR = '.';

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var_list name
	 */
	public obj_var_list(String name)
	{
		super(name);
		m_data = new dictionary(DEFAULT_SIZE);
	}	// obj_var_list(String name)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var_list name
	 * @param numItems	number of items in the list
	 */
	public obj_var_list(String name, int numItems)
	{
		super(name);
		m_data = new dictionary((int)(numItems / 0.75) + 1);		// 0.75 is the default load factor
	}	// obj_var_list(String name, int numItems)

	/**
	 * Clone function.
	 *
	 * @return a copy of this object
	 */
	public Object clone()
	{
		int count = getNumItems();
		obj_var_list list = new obj_var_list(getName(),  count);
		for ( int i = 0; i < count; ++i )
		{
			list.setObjVar(getObjVar(i));
		}
		return list;
	}	// clone

	/**
	 * Accessor function.
	 *
	 * @return the number of items in the list
	 */
	public int getNumItems()
	{
		dictionary list = (dictionary)m_data;
		return list.size();
	}	// getNumItems

	/**
	 * Finds the obj_var with a given name.
	 *
	 * @param name		name of the obj_var to get
	 *
	 * @return true if the obj_var was found, false if not
	 */
	public boolean hasObjVar(String name)
	{
		dictionary list = (dictionary)m_data;
		int dotIndex = name.indexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
			return list.containsKey(name);
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var result = (obj_var)(list.get(listName));
			if (result == null || !(result instanceof script.obj_var_list))
				return false;
			return ((obj_var_list)result).hasObjVar(itemName);
		}
	}	// hasObjVar

	/**
	 * Finds the obj_var with a given name.
	 *
	 * @param name		name of the obj_var to get
	 *
	 * @return the obj_var
	 */
	public obj_var getObjVar(String name)
	{
		dictionary list = (dictionary)m_data;
		int dotIndex = name.indexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
			return (obj_var)(list.get(name));
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var result = (obj_var)(list.get(listName));
			if (result == null || !(result instanceof script.obj_var_list))
				return null;
			return ((obj_var_list)result).getObjVar(itemName);
		}
	}	// getObjVar(String name)

	/**
	 * Finds an integer obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the integer, or 0 if not found
	 */
	public int getIntObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getIntData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return 0;
	}

	/**
	 * Finds an integer array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the integer array, or null if not found
	 */
	public int[] getIntArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getIntArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a float obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the float, or 0 if not found
	 */
	public float getFloatObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getFloatData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return 0;
	}

	/**
	 * Finds a float array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the float array, or null if not found
	 */
	public float[] getFloatArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getFloatArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a string obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the string, or null if not found
	 */
	public String getStringObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getStringData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a string array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the string array, or null if not found
	 */
	public String[] getStringArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getStringArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds an obj_id obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the obj_id, or null if not found
	 */
	public obj_id getObjIdObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getObjIdData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds an obj_id array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the obj_id array, or null if not found
	 */
	public obj_id[] getObjIdArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getObjIdArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a location obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the location, or null if not found
	 */
	public location getLocationObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getLocationData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a location array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the location array, or null if not found
	 */
	public location[] getLocationArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getLocationArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a string_id obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the string_id, or null if not found
	 */
	public string_id getStringIdObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getStringIdData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a string_id array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the string_id array, or null if not found
	 */
	public string_id[] getStringIdArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			try
			{
				return ov.getStringIdArrayData();
			}
			catch( ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds an boolean obj_var with a given name. Note that since
	 * we don't have real boolean objvars we get the value as an int and convert it.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the boolean value
	 */
	public boolean getBooleanObjVar(String name)
	{
		int value = getIntObjVar(name);
		if ( value == 0 )
			return false;
		return true;
	}

	/**
	 * Finds an attrib_mod obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the attrib_mod, or null if not found
	 */
	public attrib_mod getAttribModObjVar(String name)
	{
		String data = getStringObjVar(name);
		if (data == null)
			return null;
		attrib_mod mod = new attrib_mod(data);
		if (mod.getAttribute() < 0)
			mod = null;
		return mod;
	}

	/**
	 * Finds an attrib_mod array obj_var with a given name.
	 *
	 * @param name		the objvar name to search for
	 *
	 * @return the attrib_mod array, or null if not found
	 */
	public attrib_mod[] getAttribModArrayObjVar(String name)
	{
		String[] data = getStringArrayObjVar(name);
		if (data == null)
			return null;
		attrib_mod[] mod = new attrib_mod[data.length];
		for (int i = 0; i < data.length; ++i)
		{
			mod[i] = new attrib_mod(data[i]);
			if (mod[i].getAttribute() < 0)
			{
				mod = null;
				break;
			}
		}
		return mod;
	}

	/**
	 * Finds a transform obj_var with a given name.
	 *
	 * @param name  the objvar name to search for
	 * @return      the transform, or null if not found
	 */
	public transform getTransformObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if (ov != null)
		{
			try
			{
				return ov.getTransformData();
			}
			catch (ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a transform array obj_var with a given name.
	 *
	 * @param name  the objvar name to search for
	 * @return      the transform array, or null if not found
	 */
	public transform[] getTransformArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if (ov != null)
		{
			try
			{
				return ov.getTransformArrayData();
			}
			catch (ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a vector obj_var with a given name.
	 *
	 * @param name  the objvar name to search for
	 * @return      the vector, or null if not found
	 */
	public vector getVectorObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if (ov != null)
		{
			try
			{
				return ov.getVectorData();
			}
			catch (ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds a vector array obj_var with a given name.
	 *
	 * @param name  the objvar name to search for
	 * @return      the vector array, or null if not found
	 */
	public vector[] getVectorArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if (ov != null)
		{
			try
			{
				return ov.getVectorArrayData();
			}
			catch (ClassCastException err)
			{
			}
		}
		return null;
	}

	/**
	 * Finds the obj_var_list sub-list with a given name.
	 *
	 * @param name		name of the obj_var_list to get
	 *
	 * @return the obj_var_list
	 */
	public obj_var_list getObjVarList(String name)
	{
		dictionary list = (dictionary)m_data;
		int dotIndex = name.indexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			obj_var result = (obj_var)(list.get(name));
			if (result != null && result instanceof script.obj_var_list)
				return (obj_var_list)result;
			return null;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var result = (obj_var)(list.get(listName));
			if (result == null || !(result instanceof script.obj_var_list))
				return null;
			return ((obj_var_list)result).getObjVarList(itemName);
		}
	}	// getObjVarList(String name)

	/**
	 * Finds the obj_var with a given index.
	 *
	 * @param index		index of the obj_var to get
	 *
	 * @return the obj_var
	 */
	public obj_var getObjVar(int index)
	{
		dictionary list = (dictionary)m_data;
		if (index < 0 || index >= list.size())
			return null;
		Iterator iter = list.values().iterator();
		for (; index > 0; --index)
			iter.next();
		return (obj_var)(iter.next());
	}	// getObjVar(int index)

	public boolean isIntObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof Integer )
				return true;
		}
		return false;
	}

	public boolean isIntArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof int[] )
				return true;
		}
		return false;
	}

	public boolean isFloatObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof Float )
				return true;
		}
		return false;
	}

	public boolean isFloatArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof float[] )
				return true;
		}
		return false;
	}

	public boolean isStringObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof String )
				return true;
		}
		return false;
	}

	public boolean isStringArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof String[] )
				return true;
		}
		return false;
	}

	public boolean isObjIdObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof obj_id )
				return true;
		}
		return false;
	}

	public boolean isObjIdArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof obj_id[] )
				return true;
		}
		return false;
	}

	public boolean isLocationObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof location )
				return true;
		}
		return false;
	}

	public boolean isLocationArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof location[] )
				return true;
		}
		return false;
	}

	public boolean isStringIdObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof string_id )
				return true;
		}
		return false;
	}

	public boolean isStringIdArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof string_id[] )
				return true;
		}
		return false;
	}

	public boolean isBooleanObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof Integer )
				return true;
		}
		return false;
	}

	public boolean isTransformObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof transform )
				return true;
		}
		return false;
	}

	public boolean isTransformArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof transform[] )
				return true;
		}
		return false;
	}

	public boolean isVectorObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof vector )
				return true;
		}
		return false;
	}

	public boolean isVectorArrayObjVar(String name)
	{
		obj_var ov = getObjVar(name);
		if ( ov != null )
		{
			if ( ov.getData() != null && ov.getData() instanceof vector[] )
				return true;
		}
		return false;
	}

	public Enumeration keys()
	{
		return ((dictionary)m_data).keys();
	}

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param data		the obj_var to add to the list
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(obj_var data)
	{
		dictionary list = (dictionary)m_data;
		list.put(data.getName(), data);
		return true;
	}	// setObjVar(obj_var data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, int data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, int data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, int[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, int[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, float data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, float data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, float[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, float[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, String data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, String data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, String[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, String[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, long data)
	{
		return setObjVar(name, (data == 0) ? null : obj_id.getObjId(data));
	}
	public boolean setObjVar(String name, obj_id data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, obj_id data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, long[] data) throws java.lang.ClassNotFoundException
	{
		obj_id[] _data = null;
		if (data != null)
		{
			_data = new obj_id[data.length];
			for (int _i = 0; _i < data.length; ++_i)
				_data[_i] = (data[_i] == 0) ? null : obj_id.getObjId(data[_i]);
		}
		return setObjVar(name, _data);
	}
	public boolean setObjVar(String name, obj_id[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, obj_id[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, location data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, location data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, location[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, location[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, string_id data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, obj_id data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, string_id[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, obj_id[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, attrib_mod data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, attrib_mod data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, attrib_mod[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}	// setObjVar(String name, attrib_mod[] data)

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, transform data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, transform[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, vector data)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}

	/**
	 * Sets/creates an obj_var in the list.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 *
	 * @return true on success, false on error
	 */
	public boolean setObjVar(String name, vector[] data) throws java.lang.ClassNotFoundException
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			list.put(name, new obj_var(name, data));
			return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = setObjVarList(listName);
			if (list != null)
				return list.setObjVar(itemName, data);
		}
		return false;
	}

	/**
	 * Adds a sub-list to this list.
	 *
	 * @param name		the name of the sub-list
	 *
	 * @return the sub-list
	 */
	public obj_var_list setObjVarList(String name)
	{
		int dotIndex = name.indexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			obj_var_list newList = new obj_var_list(name);
			list.put(name, newList);
			return newList;
		}
		else
		{
			String rootList = name.substring(0, dotIndex);
			String subList = name.substring(dotIndex + 1);
			obj_var_list newList = setObjVarList(rootList);
			return newList.setObjVarList(subList);
		}
	}	// setObjVarList()

	/**
	 * Removes an obj_var from this list.
	 *
	 * @param name		the name of the obj_var to remove
	 *
	 * @return true on success, false if name was not a valid obj_var name
	 */
	public boolean removeObjVar(String name)
	{
		int dotIndex = name.lastIndexOf(LIST_SEPARATOR);
		if (dotIndex == -1)
		{
			dictionary list = (dictionary)m_data;
			if (list.remove(name) != null);
				return true;
		}
		else
		{
			String listName = name.substring(0, dotIndex);
			String itemName = name.substring(dotIndex + 1);
			obj_var_list list = getObjVarList(listName);
			if (list != null)
				return list.removeObjVar(itemName);
		}
		return false;
	}	// removeObjVar(String)

	/**
	 * Removes an obj_var from this list.
	 *
	 * @param index		the index of the obj_var to remove
	 *
	 * @return true on success, false if index was out of range
	 */
	public boolean removeObjVar(int index)
	{
		dictionary list = (dictionary)m_data;
		if (index < 0 || index >= list.size())
			return false;

		Iterator iter = list.values().iterator();
		for (; index > 0; --index)
			iter.next();
		list.remove(((obj_var)(iter.next())).getName());
		return true;
	}	// removeObjVar(int)

	/**
	 * Get a list of the names of all objvars contained by this obj_var_list, recursively.  Return only the full names, relative from this obj_var_list, for final leaf obj_vars.
	 *
	 * @return string array of full objvar names
	 */
	public String[] getAllObjVarNames()
	{
		dictionary dict = (dictionary)m_data;
		Vector returnedNames = new Vector();
		Set keySet = dict.keySet();
		Iterator objvarIterator = keySet.iterator();
		while(objvarIterator.hasNext())
		{
			String key = (String)objvarIterator.next();
			obj_var_list list = getObjVarList(key);
			if (list != null)
			{
				String[] childstrings = list.getAllObjVarNames();
				for (int i = 0; i < childstrings.length; ++i)
				{
					returnedNames.add (key + "." + childstrings[i]);
				}
			}
			else
			{
				returnedNames.add (key);
			}
		}
		String[] toReturn = new String[returnedNames.size()];
		returnedNames.toArray(toReturn);
		return toReturn;
	}

}	// class obj_var_list