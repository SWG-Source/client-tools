/**
 * Title:        obj_var
 * Description:  Wrapper for an object variable.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.lang.reflect.Array;
import java.io.Serializable;

public class obj_var implements Comparable, Serializable
{
	private final static long serialVersionUID = -3373805124719784247L;

	protected String m_name;
	protected Object m_data;

	/**
	 *  Class constructor.
	 *
	 * @param name		the obj_var name
	 */
	protected obj_var(String name)
	{
		m_name = name;
		m_data = null;
	}	// obj_var(String)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, int data)
	{
		m_name = name;
		m_data = new Integer(data);
	}	// obj_var(String, int)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, int[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
//		m_data = Array.newInstance(Class.forName("java.lang.Integer"), data.length);
		m_data = new int[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, new Integer(data[i]));
	}	// obj_var(String, int[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, float data)
	{
		m_name = name;
		m_data = new Float(data);
	}	// obj_var(String, float)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, float[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
//		m_data = Array.newInstance(Class.forName("java.lang.Float"), data.length);
		m_data = new float[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, new Float(data[i]));
	}	// obj_var(String, float[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, String data)
	{
		m_name = name;
		m_data = data;
	}	// obj_var(String, String)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, String[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
//		m_data = Array.newInstance(Class.forName("java.lang.String"), data.length);
		m_data = new String[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i]);
	}	// obj_var(String, String[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, obj_id data)
	{
		m_name = name;
		m_data = data;
	}	// obj_var(String, obj_id)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, long data)
	{
		m_name = name;
		m_data = (data == 0) ? null : obj_id.getObjId(data);
	}	// obj_var(String, obj_id)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, obj_id[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
//		m_data = Array.newInstance(Class.forName("script.obj_id"), data.length);
		m_data = new obj_id[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i]);
	}	// obj_var(String, obj_id[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, long[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
		m_data = new obj_id[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, (data[i] == 0) ? null : obj_id.getObjId(data[i]));
	}	// obj_var(String, obj_id[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, location data)
	{
		m_name = name;
		m_data = (location)data.clone();
	}	// obj_var(String, location)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, location[] data) throws java.lang.ClassNotFoundException
	{
		m_name = name;
//		m_data = Array.newInstance(Class.forName("script.location"), data.length);
		m_data = new location[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i].clone());
	}	// obj_var(String, location[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, string_id data)
	{
		m_name = name;
		m_data = data;
	}	// obj_var(String, string_id)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, string_id[] data)
	{
		m_name = name;
		m_data = new string_id[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i]);
	}	// obj_var(String, string_id[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, attrib_mod data)
	{
		m_name = name;
		m_data = data.pack();
	}	// obj_var(String, attrib_mod)

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, attrib_mod[] data)
	{
		m_name = name;
		m_data = new String[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i].pack());
	}	// obj_var(String, attrib_mod[])

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, transform data)
	{
		m_name = name;
		m_data = data;
	}

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, transform[] data)
	{
		m_name = name;
		m_data = new transform[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i]);
	}

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, vector data)
	{
		m_name = name;
		m_data = data;
	}

	/**
	 * Class constructor.
	 *
	 * @param name		the obj_var name
	 * @param data		the obj_var value
	 */
	public obj_var(String name, vector[] data)
	{
		m_name = name;
		m_data = new vector[data.length];
		for ( int i = 0; i < data.length; ++i )
			Array.set(m_data, i, data[i]);
	}

	/**
	 * Accessor function.
	 *
	 * @return the obj_var name
	 */
	public String getName()
	{
		return m_name;
	}	// getName

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data
	 */
	public Object getData()
	{
		return m_data;
	}	// getData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an int
	 */
	public int getIntData() throws ClassCastException
	{
		try
		{
			return ((Integer)m_data).intValue();
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getIntData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return 0;
	}	// getIntData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an int[]
	 */
	public int[] getIntArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		int[] data = new int[length];
		for ( int i = 0; i < length; ++i )
			data[i] = Array.getInt(m_data, i);
		return data;
	}	// getIntArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a float
	 */
	public float getFloatData() throws ClassCastException
	{
		try
		{
			return ((Float)m_data).floatValue();
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getFloatData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return 0.0f;
	}	// getFloatData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a float[]
	 */
	public float[] getFloatArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		float[] data = new float[length];
		for ( int i = 0; i < length; ++i )
			data[i] = Array.getFloat(m_data, i);
		return data;
	}	// getFloatArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a boolean
	 */
	public boolean getBooleanData() throws ClassCastException
	{
		try
		{
			return ((Integer)m_data).intValue() != 0;
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getBooleanData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return false;
	}	// getBooleanData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a boolean[]
	 */
	public boolean[] getBooleanArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		boolean[] data = new boolean[length];
		for ( int i = 0; i < length; ++i )
			data[i] = Array.getInt(m_data, i) != 0;
		return data;
	}	// getBooleanArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a String
	 */
	public String getStringData() throws ClassCastException
	{
		try
		{
			return (String)m_data;
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getStringData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return "";
	}	// getStringData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a String[]
	 */
	public String[] getStringArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		String[] data = new String[length];
		for ( int i = 0; i < length; ++i )
			data[i] = (String)Array.get(m_data, i);
		return data;
	}	// getStringArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an obj_id
	 */
	public obj_id getObjIdData() throws ClassCastException
	{
		try
		{
			return (obj_id)m_data;
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getObjIdData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}	// getObjIdData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an obj_id[]
	 */
	public obj_id[] getObjIdArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		obj_id[] data = new obj_id[length];
		for ( int i = 0; i < length; ++i )
			data[i] = (obj_id)Array.get(m_data, i);
		return data;
	}	// getObjIdArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a location
	 */
	public location getLocationData() throws ClassCastException
	{
		try
		{
			return new location((location)m_data);
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getLocationData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}	// getLocationData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a location[]
	 */
	public location[] getLocationArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		location[] data = new location[length];
		for ( int i = 0; i < length; ++i )
			data[i] = new location((location)Array.get(m_data, i));
		return data;
	}	// getLocationArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an string_id
	 */
	public string_id getStringIdData() throws ClassCastException
	{
		try
		{
			return (string_id)m_data;
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getStringIdData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}	// getStringIdData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an string_id[]
	 */
	public string_id[] getStringIdArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		string_id[] data = new string_id[length];
		for ( int i = 0; i < length; ++i )
			data[i] = (string_id)Array.get(m_data, i);
		return data;
	}	// getStringIdArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an attrib_mod
	 */
	public attrib_mod getAttribModData() throws ClassCastException
	{
		try
		{
			return new attrib_mod((String)m_data);
		}
		catch( ClassCastException err )
		{
			System.err.println("WARNING: Java obj_var.getAttribModData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}	// getAttribModData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as an attrib_mod[]
	 */
	public attrib_mod[] getAttribModArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		attrib_mod[] data = new attrib_mod[length];
		for ( int i = 0; i < length; ++i )
			data[i] = new attrib_mod((String)Array.get(m_data, i));
		return data;
	}	// getAttribModArrayData

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a transform
	 */
	public transform getTransformData() throws ClassCastException
	{
		try
		{
			return (transform)m_data;
		}
		catch (ClassCastException err)
		{
			System.err.println("WARNING: Java obj_var.getTransformData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a transform[]
	 */
	public transform[] getTransformArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		transform[] data = new transform[length];
		for (int i = 0; i < length; ++i)
			data[i] = (transform)Array.get(m_data, i);
		return data;
	}

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a vector
	 */
	public vector getVectorData() throws ClassCastException
	{
		try
		{
			return (vector)m_data;
		}
		catch (ClassCastException err)
		{
			System.err.println("WARNING: Java obj_var.getVectorData tried to get item " + m_name + " of type " + m_data.getClass());
		}
		return null;
	}

	/**
	 * Accessor function.
	 *
	 * @return the obj_var data as a vector[]
	 */
	public vector[] getVectorArrayData() throws ClassCastException
	{
		int length = Array.getLength(m_data);
		vector[] data = new vector[length];
		for (int i = 0; i < length; ++i)
			data[i] = (vector)Array.get(m_data, i);
		return data;
	}

	/**
	 * Conversion function.
	 *
	 * @return the obj_var as a string.
	 */
	public String toString()
	{
		return "(" + m_name + " = " + m_data.toString() + ")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a objId, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((obj_var)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another obj_var.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(obj_var id)
	{
		int result = m_name.compareTo(id.m_name);
		if (result == 0)
			result = ((Comparable)m_data).compareTo(id.m_data);
		return result;
	}	// compareTo(obj_var)

	/**
	 * Compares this to a generic object.
	 *
	 * @returns true if the objects have the same data, false if not
	 */
	public boolean equals(Object o)
	{
		try
		{
			int result = compareTo(o);
			if (result == 0)
				return true;
		}
		catch (ClassCastException err)
		{
		}
		return false;
	}	// equals

}	// class obj_var
