/**
 * Title:        dictionary
 * Description:  Wrapper for java.util.Hashtable, with extra functionality.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OptionalDataException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Vector;
import java.lang.reflect.Array;

public class dictionary implements Serializable
{
	private final static long serialVersionUID = 3648446132389419658L;

	private transient Hashtable m_data = null;

	/**
	 * Class constructor.
	 */
	public dictionary()
	{
		m_data = new Hashtable();
	}	// dictionary()

	/**
	 * Class constructor.
	 */
	public dictionary(int initialCapacity, float loadFactor)
	{
		m_data = new Hashtable(initialCapacity, loadFactor);
	}	// dictionary(int, float)

	/**
	 * Class constructor.
	 */
	public dictionary(int initialCapacity)
	{
		m_data = new Hashtable(initialCapacity);
	}	// dictionary(int)

	/**
	 * Class constructor.
	 */
	public dictionary(Map t)
	{
		m_data = new Hashtable(t);
	}	// dictionary(Map)

	/**
	 * Serializes this dictionary into a string.
	 *
	 * @return the packed string
	 */
	public byte[] pack()
	{
		byte[] ret = null;
		
		try
		{
			dictionary_byte_array_output_stream byteOutput = new dictionary_byte_array_output_stream();
			ObjectOutputStream objectOutput = new ObjectOutputStream(byteOutput);

			try
			{
				objectOutput.writeObject(this);
				ret = byteOutput.toByteArray();
			}
			catch ( IOException err )
			{
				System.err.println("ERROR in Java dictionary.pack(): " + err.getMessage());
			}
		}
		catch (java.io.IOException e)
		{
			System.err.println("ERROR in initialization of dictionary: " + e.getMessage());
		}
		
		return ret;
	}	// pack()

	/**
	 * Serializes this dictionary into a string.
	 *
	 * @return the packed string
	 */
	public String pack_debug()
	{
		String ret = null;
		
		try
		{
			dictionary_byte_array_output_stream byteOutput = new dictionary_byte_array_output_stream();
			ObjectOutputStream objectOutput = new ObjectOutputStream(byteOutput);

			try
			{
				objectOutput.writeObject(this);
				ret = byteOutput.toStringDebug();
			}
			catch ( IOException err )
			{
				System.err.println("ERROR in Java dictionary.pack(): " + err.getMessage());
			}
		}
		catch (java.io.IOException e)
		{
			System.err.println("ERROR in initialization of dictionary: " + e.getMessage());
		}
		
		return ret;
	}	// pack_debug()

	/**
	 * Unserializes a string into a dictionary.
	 *
	 * @param source		the string to unpack
	 *
	 * @return the unpacked dictionary
	 */
	public static dictionary unpack(byte[] source)
	{
		try
		{
			dictionary_byte_array_input_stream bais = new dictionary_byte_array_input_stream(source);
			ObjectInputStream ois = new ObjectInputStream(bais);
			Object result = ois.readObject();
			if (result instanceof script.dictionary)
			{
				return (dictionary)result;
			}
			else
			{
				System.err.println("Could not convert string '" + source +"' to dictionary.");
				return null;
			}
		}
		catch (ClassNotFoundException x)
		{
			System.err.println("ClassNotFoundException:  "+x.toString());
		}
		catch (IOException x)
		{
			System.err.println("IOException:  "+x.toString());
		}

		return null;
	}	// unpack()

	/**
	 * Writes the dictionary out to a stream.
	 */
	private void writeObject(ObjectOutputStream out) throws IOException
	{
		out.defaultWriteObject();
		int count = m_data.size();
		out.writeInt(count);

		Set keySet = m_data.keySet();
		Iterator i = keySet.iterator();
		while (i.hasNext())
		{
			Object key = i.next();
			Object value = m_data.get(key);
			out.writeObject(key);
			if (value instanceof Boolean)
			{
				out.writeByte('z');
				out.writeBoolean(((Boolean)value).booleanValue());
			}
			else if (value instanceof Byte)
			{
				out.writeByte('b');
				out.writeByte(((Byte)value).byteValue());
			}
			else if (value instanceof Character)
			{
				out.writeByte('c');
				out.writeChar(((Character)value).charValue());
			}
			else if (value instanceof Integer)
			{
				out.writeByte('i');
				out.writeInt(((Integer)value).intValue());
			}
			else if (value instanceof Long)
			{
				out.writeByte('l');
				out.writeLong(((Long)value).longValue());
			}
			else if (value instanceof Float)
			{
				out.writeByte('f');
				out.writeFloat(((Float)value).floatValue());
			}
			else if (value instanceof Double)
			{
				out.writeByte('d');
				out.writeDouble(((Double)value).doubleValue());
			}
			else if (value instanceof obj_id)
			{
				out.writeByte('o');
				out.writeLong(((obj_id)value).getValue());
			}
			else
			{
				out.writeByte('x');
				out.writeObject(value);
			}
		}
	}	// writeObject()

	/**
	 * Reads the dictionary in from a stream.
	 */
	private void readObject(ObjectInputStream in) throws IOException, ClassNotFoundException
	{
		in.defaultReadObject();

		if (m_data == null)
			m_data = new Hashtable();

		int count = in.readInt();
		for (int i = 0; i < count; ++i)
		{
			Object key = in.readObject();
			if (key == null)
			{
				System.err.println("ERROR: dictionary.readObject: Error reading key " + i);
				continue;
			}
			byte tag = in.readByte();
			switch (tag)
			{
				case 'z' :
					m_data.put(key, new Boolean(in.readBoolean()));
					break;
				case 'b' :
					m_data.put(key, new Byte(in.readByte()));
					break;
				case 'c' :
					m_data.put(key, new Character(in.readChar()));
					break;
				case 'i':
					m_data.put(key, new Integer(in.readInt()));
					break;
				case 'l' :
					m_data.put(key, new Long(in.readLong()));
					break;
				case 'f':
					m_data.put(key, new Float(in.readFloat()));
					break;
				case 'd' :
					m_data.put(key, new Double(in.readDouble()));
					break;
				case 'o':
				{
					long value = in.readLong();
					if (value == 0)
						m_data.put(key, obj_id.NULL_ID);
					else
					{
						obj_id id = obj_id.getObjId(value);
						if (id != null)
							m_data.put(key, id);
						else
						{
							System.err.println("ERROR: dictionary.readObject: Read null obj_id for value " + value +
								", key <" + key + "> will not be added to the dictionary");
						}
					}
					break;
				}
				default:
				{
					Object value = in.readObject();
					if (value == null)
						System.err.println("ERROR: dictionary.readObject: Error reading value for key " + key + "(" + i + ")");
					else
						m_data.put(key, value);
					break;
				}
			}
		}
	}	// readObject()

    /**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, Object value)
	{
		if ( key == null || value == null )
		{
			if ( key == null )
				System.err.println("ERROR calling dictionary.put(Object, Object): null key");
			if ( value == null )
				System.err.println("ERROR calling dictionary.put(Object, Object): null value");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, value);
	}	// put(Object, Object)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, int value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, int): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Integer(value));
	}	// put(Object, int)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, long value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, long): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Long(value));
	}	// put(Object, long)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, float value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, float): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Float(value));
	}	// put(Object, float)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, double value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, double): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Double(value));
	}	// put(Object, double)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, boolean value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, boolean): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Boolean(value));
	}	// put(Object, boolean)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, char value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, char): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Character(value));
	}	// put(Object, char)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, byte value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.put(Object, byte): null key");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, new Byte(value));
	}	// put(Object, byte)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, String value)
	{
		if ( key == null || value == null )
		{
			if ( key == null )
				System.err.println("ERROR calling dictionary.put(Object, String): null key");
			if ( value == null )
				System.err.println("ERROR calling dictionary.put(Object, String): null value");
			Thread.dumpStack();
			return null;
		}
		return m_data.put(key, value);
	}	// put(Object, String)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, Vector value)
	{
		if ( key == null || value == null || value.isEmpty() )
		{
			if ( key == null )
				System.err.println("ERROR calling dictionary.put(Object, Vector): null key");
			if ( value == null )
				System.err.println("ERROR calling dictionary.put(Object, Vector): null value");
			else if ( value.isEmpty() )
			{
				System.err.println("ERROR calling dictionary.put(Object, Vector): empty resizeable array value");
			}
			Thread.dumpStack();
			return null;
		}
		Object test = value.get(0);
		if (test == null)
		{
			System.err.println("ERROR calling dictionary.put(Object, Vector): resizeable array value with null entry");
			Thread.dumpStack();
			return null;
		}
		Object realValue = null;
		if (test instanceof Integer)
		{
			int[] newValue = new int[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Integer)value.get(i)).intValue();
			realValue = newValue;
		}
		else if (test instanceof Long)
		{
			long[] newValue = new long[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Long)value.get(i)).longValue();
			realValue = newValue;
		}
		else if (test instanceof Float)
		{
			float[] newValue = new float[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Float)value.get(i)).floatValue();
			realValue = newValue;
		}
		else if (test instanceof Double)
		{
			double[] newValue = new double[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Double)value.get(i)).doubleValue();
			realValue = newValue;
		}
		else if (test instanceof Boolean)
		{
			boolean[] newValue = new boolean[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Boolean)value.get(i)).booleanValue();
			realValue = newValue;
		}
		else if (test instanceof Character)
		{
			char[] newValue = new char[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Character)value.get(i)).charValue();
			realValue = newValue;
		}
		else if (test instanceof Byte)
		{
			byte[] newValue = new byte[value.size()];
			for ( int i = 0; i < newValue.length; ++i )
				newValue[i] = ((Byte)value.get(i)).byteValue();
			realValue = newValue;
		}
		else if (test instanceof String)
		{
			String[] newValue = new String[value.size()];
			newValue = (String[])value.toArray(newValue);
			realValue = newValue;
		}
		else if (test instanceof location)
		{
			location[] newValue = new location[value.size()];
			newValue = (location[])value.toArray(newValue);
			realValue = newValue;
		}
		else if (test instanceof string_id)
		{
			string_id[] newValue = new string_id[value.size()];
			newValue = (string_id[])value.toArray(newValue);
			realValue = newValue;
		}
		else if (test instanceof obj_id)
		{
			obj_id[] newValue = new obj_id[value.size()];
			newValue = (obj_id[])value.toArray(newValue);
			realValue = newValue;
		}
		else if (test instanceof transform)
		{
			transform[] newValue = new transform[value.size()];
			newValue = (transform[])value.toArray(newValue);
			realValue = newValue;
		}
		else if (test instanceof vector)
		{
			vector[] newValue = new vector[value.size()];
			newValue = (vector[])value.toArray(newValue);
			realValue = newValue;
		}
		else
		{
			System.err.println("ERROR calling dictionary.put(Object, Vector): resizeable array value " +
				"has unhandled vector data type " + test.getClass());
			Thread.dumpStack();
			return null;
		}
		return put(key, realValue);
	}	// put(Object, Vector)

	/**
	 * Wrapper for Hashtable.get().
	 */
	public Object get(Object key)
	{
		return m_data.get(key);
	}	// get()

	/**
	 * Gets a value from the dictionary.
	 */
	public int getInt(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getInt: null key");
			return 0;
		}
		Object value = m_data.get(key);
		if (value instanceof Integer)
			return ((Integer)value).intValue();
		return 0;
	}	// getInt()

	/**
	 * Gets a value from the dictionary.
	 */
	public int[] getIntArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getIntArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof int[])
			return (int[])value;
		return null;
	}	// getIntArray()

	public Vector getResizeableIntArray(Object key)
	{
		int[] array = getIntArray(key);
		if (array != null)
		{
			Vector resizeableArray = new Vector(array.length+10);
			for (int i = 0; i < array.length; ++i)
				resizeableArray.add(new Integer(array[i]));
			return resizeableArray;
		}
		return null;
	}

	/**
	 * Gets a value from the dictionary.
	 */
	public long getLong(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getLong: null key");
			return 0;
		}

		Object value = m_data.get(key);
		if (value instanceof Long)
			return ((Long)value).longValue();
		return 0;
	}	// getLong()

	/**
	 * Gets a value from the dictionary.
	 */
	public long[] getLongArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getLongArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof long[])
			return (long[])value;
		return null;
	}	// getLongArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public float getFloat(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getFloat: null key");
			return 0;
		}

		Object value = m_data.get(key);
		if (value instanceof Float)
			return ((Float)value).floatValue();
		return 0;
	}	// getFloat()

	/**
	 * Gets a value from the dictionary.
	 */
	public float[] getFloatArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getFloatArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof float[])
			return (float[])value;
		return null;
	}	// getFloatArray()

	public Vector getResizeableFloatArray(Object key)
	{
		float[] array = getFloatArray(key);
		if (array != null)
		{
			Vector resizeableArray = new Vector(array.length+10);
			for (int i = 0; i < array.length; ++i)
				resizeableArray.add(new Float(array[i]));
			return resizeableArray;
		}
		return null;
	}

	/**
	 * Gets a value from the dictionary.
	 */
	public double getDouble(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getDouble: null key");
			return 0;
		}

		Object value = m_data.get(key);
		if (value instanceof Double)
			return ((Double)value).doubleValue();
		return 0;
	}	// getDouble()

	/**
	 * Gets a value from the dictionary.
	 */
	public double[] getDoubleArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getDoubleArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof double[])
			return (double[])value;
		return null;
	}	// getDoubleArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public boolean getBoolean(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getBoolean: null key");
			return false;
		}

		Object value = m_data.get(key);
		if (value instanceof Boolean)
			return ((Boolean)value).booleanValue();
		return false;
	}	// getBoolean()

	/**
	 * Gets a value from the dictionary.
	 */
	public boolean[] getBooleanArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getBooleanArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof boolean[])
			return (boolean[])value;
		return null;
	}	// getBooleanArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public char getChar(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getChar: null key");
			return 0;
		}

		Object value = m_data.get(key);
		if (value instanceof Character)
			return ((Character)value).charValue();
		return 0;
	}	// getChar()

	/**
	 * Gets a value from the dictionary.
	 */
	public char[] getCharArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getCharArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof char[])
			return (char[])value;
		return null;
	}	// getCharArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public byte getByte(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getByte: null key");
			return 0;
		}

		Object value = m_data.get(key);
		if (value instanceof Byte)
			return ((Byte)value).byteValue();
		return 0;
	}	// getByte()

	/**
	 * Gets a value from the dictionary.
	 */
	public byte[] getByteArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getByteArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof byte[])
			return (byte[])value;
		return null;
	}	// getByteArray()

	/**
	 * Get an obj_var from a dictionary
	 */
	public obj_var getObjVar(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjVar: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof obj_var)
			return (obj_var)value;
		return null;
	}

	/**
	 * Get an obj_var_list from a dictionary
	 */
	public obj_var_list getObjVarList(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjVarList: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof obj_var_list)
			return (obj_var_list)value;
		return null;
	}

	/**
	 * Get a dictionary from a dictionary
	 */
	public dictionary getDictionary(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getDictionary: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof dictionary)
			return (dictionary)value;
		return null;
	}

	/**
	 * Get a location from a dictionary
	 */
	public location getLocation(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getLocation: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof location)
			return (location)value;
		return null;
	}

	/**
	 * Get a location array from a dictionary
	 */
	public location[] getLocationArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getLocationArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if ( value instanceof location[] )
			return (location[])value;
		return null;
	}

	public Vector getResizeableLocationArray(Object key)
	{
		location[] array = getLocationArray(key);
		if (array != null)
			return new Vector(Arrays.asList(array));
		return null;
	}

	public region getRegion(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getRegion: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof region)
			return (region)value;
		return null;
	}

	/**
	 * Get a string id from a dictionary
	 */
	public string_id getStringId(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getStringId: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof string_id)
			return (string_id)value;
		return null;
	}

	/**
	 * Get a string_id array from a dictionary
	 */
	public string_id [] getStringIdArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getStringIdArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof string_id[])
			return (string_id[])value;
		return null;
	}

	/**
	 * Get a string from a dictionary
	 */
	public String getString(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getString: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof String)
			return (String)value;
		return null;
	}

	/**
	 * Get a string array from a dictionary
	 */
	public String[] getStringArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getStringArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof String[])
			return (String[])value;
		return null;
	}

	public Vector getResizeableStringArray(Object key)
	{
		String[] array = getStringArray(key);
		if (array != null)
			return new Vector(Arrays.asList(array));
		return null;
	}

	/**
	 * Get an obj_id from a dictionary
	 */
	public obj_id getObjId(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjId: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof obj_id)
			return (obj_id)value;
		return null;
	}

	/**
	 * Get an obj_id array from a dictionary
	 */
	public obj_id[] getObjIdArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjIdArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof obj_id[])
			return (obj_id[])value;
		return null;
	}

	public Vector getResizeableObjIdArray(Object key)
	{
		obj_id[] array = getObjIdArray(key);
		if (array != null)
			return new Vector(Arrays.asList(array));
		return null;
	}

	/**
	 * Get an obj_id[] array from a dictionary
	 */
	public obj_id[][] getObjIdArrayArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjIdArrayArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof obj_id[][])
			return (obj_id[][])value;
		return null;
	}

	/**
	 * Get a transform from a dictionary
	 */
	public transform getTransform(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getTransform: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof transform)
			return (transform)value;
		return null;
	}

	/**
	 * Get a transform array from a dictionary
	 */
	public transform[] getTransformArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getTransformArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof transform[])
			return (transform[])value;
		return null;
	}

	public Vector getResizeableTransformArray(Object key)
	{
		transform[] array = getTransformArray(key);
		if (array != null)
			return new Vector(Arrays.asList(array));
		return null;
	}

	/**
	 * Get a vector from a dictionary
	 */
	public vector getVector(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getVector: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof vector)
			return (vector)value;
		return null;
	}

	/**
	 * Get a vector array from a dictionary
	 */
	public vector[] getVectorArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getVectorArray: null key");
			return null;
		}

		Object value = m_data.get(key);
		if (value instanceof vector[])
			return (vector[])value;
		return null;
	}

	public Vector getResizeableVectorArray(Object key)
	{
		vector[] array = getVectorArray(key);
		if (array != null)
			return new Vector(Arrays.asList(array));
		return null;
	}

	public ArrayList getArrayList(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.getObjIdArray: null key");
			return null;
		}
		Object value = m_data.get(key);
		if(value instanceof ArrayList)
			return (ArrayList)value;
		return null;
	}

	/**
	 * Tests to see if a value for a key is an int value.
	 */
	public boolean isInt(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isInt: null key");
			return false;
		}
		return m_data.get(key) instanceof Integer;
	}	// isInt()

	/**
	 * Tests to see if a value for a key is an array of int values.
	 */
	public boolean isIntArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isIntArray: null key");
			return false;
		}
		return m_data.get(key) instanceof int[];
	}	// isIntArray()

	/**
	 * Tests to see if a value for a key is a long value.
	 */
	public boolean isLong(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isLong: null key");
			return false;
		}
		return m_data.get(key) instanceof Long;
	}	// isLong()

	/**
	 * Tests to see if a value for a key is an array of long values.
	 */
	public boolean isLongArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isLongArray: null key");
			return false;
		}
		return m_data.get(key) instanceof long[];
	}	// isLongArray()

	/**
	 * Tests to see if a value for a key is a float value.
	 */
	public boolean isFloat(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isFloat: null key");
			return false;
		}
		return m_data.get(key) instanceof Float;
	}	// isFloat()

	/**
	 * Tests to see if a value for a key is an array of float values.
	 */
	public boolean isFloatArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isFloatArray: null key");
			return false;
		}
		return m_data.get(key) instanceof float[];
	}	// isFloatArray()

	/**
	 * Tests to see if a value for a key is a double value.
	 */
	public boolean isDouble(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isDouble: null key");
			return false;
		}
		return m_data.get(key) instanceof Double;
	}	// isDouble()

	/**
	 * Tests to see if a value for a key is an array of double values.
	 */
	public boolean isDoubleArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isDoubleArray: null key");
			return false;
		}
		return m_data.get(key) instanceof double[];
	}	// isDoubleArray()

	/**
	 * Tests to see if a value for a key is a boolean value.
	 */
	public boolean isBoolean(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isBoolean: null key");
			return false;
		}
		return m_data.get(key) instanceof Boolean;
	}	// isBoolean()

	/**
	 * Tests to see if a value for a key is an array of boolean values.
	 */
	public boolean isBooleanArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isBooleanArray: null key");
			return false;
		}
		return m_data.get(key) instanceof boolean[];
	}	// isBooleanArray()

	/**
	 * Tests to see if a value for a key is a char value.
	 */
	public boolean isChar(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isChar: null key");
			return false;
		}
		return m_data.get(key) instanceof Character;
	}	// isChar()

	/**
	 * Tests to see if a value for a key is an array of char values.
	 */
	public boolean isCharArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isCharArray: null key");
			return false;
		}
		return m_data.get(key) instanceof char[];
	}	// isCharArray()

	/**
	 * Tests to see if a value for a key is a byte value.
	 */
	public boolean isByte(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isByte: null key");
			return false;
		}
		return m_data.get(key) instanceof Byte;
	}	// isByte()

	/**
	 * Tests to see if a value for a key is an array of byte values.
	 */
	public boolean isByteArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isByteArray: null key");
			return false;
		}
		return m_data.get(key) instanceof byte[];
	}	// isByteArray()

	/**
	 * Tests to see if a value for a key is a dictionary value.
	 */
	public boolean isDictionary(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isDictionary: null key");
			return false;
		}
		return m_data.get(key) instanceof dictionary;
	}	// isDictionary()

	/**
	 * Tests to see if a value for a key is a string_id value.
	 */
	public boolean isStringId(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isStringId: null key");
			return false;
		}
		return m_data.get(key) instanceof string_id;
	}	// isStringId()

	/**
	 * Tests to see if a value for a key is an array of string_id values.
	 */
	public boolean isStringIdArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isStringIdArray: null key");
			return false;
		}
		return m_data.get(key) instanceof string_id[];
	}	// isStringIdArray()

	/**
	 * Tests to see if a value for a key is a String value.
	 */
	public boolean isString(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isString: null key");
			return false;
		}
		return m_data.get(key) instanceof String;
	}	// isString()

	/**
	 * Tests to see if a value for a key is an array of String values.
	 */
	public boolean isStringArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isStringArray: null key");
			return false;
		}
		return m_data.get(key) instanceof String[];
	}	// isStringArray()

	/**
	 * Tests to see if a value for a key is a location value.
	 */
	public boolean isLocation(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isLocation: null key");
			return false;
		}
		return m_data.get(key) instanceof location;
	}	// isLocation()

	/**
	 * Tests to see if a value for a key is an array of location values.
	 */
	public boolean isLocationArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isLocationArray: null key");
			return false;
		}
		return m_data.get(key) instanceof location[];
	}	// isLocationArray()

	/**
	 * Tests to see if a value for a key is an obj_id value.
	 */
	public boolean isObjId(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isObjId: null key");
			return false;
		}
		return m_data.get(key) instanceof obj_id;
	}	// idObjId()

	/**
	 * Tests to see if a value for a key is am array of obj_id values.
	 */
	public boolean isObjIdArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isObjIdArray: null key");
			return false;
		}
		return m_data.get(key) instanceof obj_id[];
	}	// idObjIdArray()

	/**
	 * Tests to see if a value for a key is a transform value.
	 */
	public boolean isTransform(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isTransform: null key");
			return false;
		}
		return m_data.get(key) instanceof transform;
	}

	/**
	 * Tests to see if a value for a key is an array of transform values.
	 */
	public boolean isTransformArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isTransformArray: null key");
			return false;
		}
		return m_data.get(key) instanceof transform[];
	}

	/**
	 * Tests to see if a value for a key is a vector value.
	 */
	public boolean isVector(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isVector: null key");
			return false;
		}
		return m_data.get(key) instanceof vector;
	}

	/**
	 * Tests to see if a value for a key is an array of vector values.
	 */
	public boolean isVectorArray(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isVectorArray: null key");
			return false;
		}
		return m_data.get(key) instanceof vector[];
	}

	/**
	 * Tests to see if a value for a key is an obj_var value.
	 */
	public boolean isObjVar(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isObjVar: null key");
			return false;
		}
		return m_data.get(key) instanceof obj_var;
	}	// isObjVar()

	/**
	 * Tests to see if a value for a key is an obj_var_list value.
	 */
	public boolean isObjVarList(Object key)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.isObjVarList: null key");
			return false;
		}
		return m_data.get(key) instanceof obj_var_list;
	}	// isObjVarList()

	/**
	 * Adds an integer value to a current value in the dictionary.
	 *
	 * @param key		the current value's dictionary key
	 * @param value     the value to add
	 *
	 * @return true on success, false if the current value isn't an integer type
	 */
	public boolean addInt(Object key, int value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.addInt: null key");
			return false;
		}

		Object obj = m_data.get(key);
		if ( obj != null && !(obj instanceof Integer) )
			return false;
		if ( obj == null )
			m_data.put(key, new Integer(value));
		else
		{
			Integer current = (Integer)obj;
			m_data.put(key, new Integer(value + current.intValue()));
		}
		return true;
	}

	/**
	 * Adds a float value to a current value in the dictionary.
	 *
	 * @param key		the current value's dictionary key
	 * @param value     the value to add
	 *
	 * @return true on success, false if the current value isn't a float type
	 */
	public boolean addFloat(Object key, float value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.addFloat: null key");
			return false;
		}

		Object obj = m_data.get(key);
		if ( obj != null && !(obj instanceof Float) )
			return false;
		if ( obj == null )
			m_data.put(key, new Float(value));
		else
		{
			Float current = (Float)obj;
			m_data.put(key, new Float(value + current.floatValue()));
		}
		return true;
	}

	/**
	 * Adds a float array to a current value in the dictionary. If the dictionary already
	 * has an array with the given key, the two arrays MUST be the same length.
	 *
	 * @param key		the current value's dictionary key
	 * @param value     the value to add
	 *
	 * @return true on success, false on error
	 */
	public boolean addFloatArray(Object key, float[] value)
	{
		if ( key == null )
		{
			System.err.println("ERROR calling dictionary.addFloatArray: null key");
			return false;
		}
		if ( value == null )
		{
			System.err.println("ERROR calling dictionary.addFloatArray: null value (key = " + key + ")");
			return false;
		}

		Object obj = m_data.get(key);
		if ( obj != null && !(obj instanceof float[]) )
			return false;
		if ( obj == null )
			m_data.put(key, value);
		else
		{
			float[] current = (float[])obj;
			if ( current.length != value.length )
			{
				System.err.println("ERROR calling dictionary.addFloatArray: mismatched array lengths (key = " + key +
					", value length = " + value.length + ", current length = " + current.length + ")");
				return false;
			}
			for ( int i = 0; i < current.length; ++i )
				current[i] += value[i];
		}
		return true;
	}


	/**
	 * Wrapper for Hashtable.clear().
	 */
	public void clear()
	{
		m_data.clear();
	}	// clear()

	/**
	 * Wrapper for Hashtable.contains().
	 */
	public boolean contains(Object value)
	{
		return m_data.contains(value);
	}	// contains()

	/**
	 * Wrapper for Hashtable.containsKey().
	 */
	public boolean containsKey(Object key)
	{
		return m_data.containsKey(key);
	}	// containsKey()

	/**
	 * Wrapper for Hashtable.containsValue().
	 */
	public boolean containsValue(Object key)
	{
		return m_data.containsValue(key);
	}	// containsValue()

	/**
	 * Wrapper for Hashtable.elements().
	 */
	public Enumeration elements()
	{
		return m_data.elements();
	}	// elements()

	/**
	 * Wrapper for Hashtable.entrySet().
	 */
	public Set entrySet()
	{
		return m_data.entrySet();
	}	// entrySet()

	/**
	 * Wrapper for Hashtable.equals().
	 */
	public boolean equals(Object o)
	{
		return m_data.equals(o);
	}	// equals()

	/**
	 * Wrapper for Hashtable.hashCode().
	 */
	public int hashCode()
	{
		return m_data.hashCode();
	}	// hashCode()

	/**
	 * Wrapper for Hashtable.isEmpty().
	 */
	public boolean isEmpty()
	{
		return m_data.isEmpty();
	}	// isEmpty()

	/**
	 * Wrapper for Hashtable.keys().
	 */
	public Enumeration keys()
	{
		return m_data.keys();
	}	// keys()

	/**
	 * Wrapper for Hashtable.keySet().
	 */
	public Set keySet()
	{
		return m_data.keySet();
	}	// keySet()

	/**
	 * Wrapper for Hashtable.putAll().
	 */
	public void putAll(Map t)
	{
		m_data.putAll(t);
	}	// putAll()

	/**
	 * Wrapper for Hashtable.putAll().
	 */
	public void putAll(dictionary d)
	{
		m_data.putAll(d.m_data);
	}	// putAll()

	/**
	 * Wrapper for Hashtable.remove().
	 */
	public Object remove(Object key)
	{
		return m_data.remove(key);
	}	// remove()

	/**
	 * Wrapper for Hashtable.size().
	 */
	public int size()
	{
		return m_data.size();
	}	// size()

	/**
	 * Wrapper for Hashtable.toString().
	 */
	public String toString()
	{
		StringBuffer buffer = new StringBuffer();
		buffer.append('<');

		Enumeration keys = m_data.keys();
		while ( keys.hasMoreElements() )
		{
			Object key = keys.nextElement();
			Object value = m_data.get(key);
			buffer.append('{');
			buffer.append(key);
			buffer.append(" = ");
			if (value instanceof Object[])
			{
				buffer.append('[');
				Object[] values = (Object[])value;
				for ( int i = 0; i < values.length; ++i )
				{
					buffer.append(values[i]);
					if (i < values.length - 1)
						buffer.append(',');
				}
				buffer.append(']');
			}
			else
				buffer.append(value);
			buffer.append("}\n");
		}
		buffer.append('>');
		return buffer.toString();
	}	// toString()

	/**
	 * Wrapper for Hashtable.values().
	 */
	public Collection values()
	{
		return m_data.values();
	}	// values()

	public String[] keysStringVector()
	{
		Vector results = new Vector();
		final Enumeration e = keys();
		while(e.hasMoreElements())
		{
			results.add(e.nextElement().toString());
		}
		String[] strings = new String[results.size()];
		for(int i = 0; i < strings.length; ++i)
		{
			strings[i] = (String)results.elementAt(i);
		}
		return strings;
	}

	public String[] valuesStringVector()
	{
		Vector results = new Vector();
		final Collection c = values();
		for(Iterator i = c.iterator(); i.hasNext(); )
		{
			results.add(i.next().toString());
		}
		String[] strings = new String[results.size()];
		for(int i = 0; i < strings.length; ++i)
		{
			strings[i] = (String)results.elementAt(i);
		}
		return strings;
	}



}	// class dictionary

