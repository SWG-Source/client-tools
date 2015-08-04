package script;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamException;
import java.io.Serializable;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Vector;
import script.dictionary;

public final class deltadictionary
{
	private transient HashSet potentiallyDirty = new HashSet();
	private transient dictionary currentValue = new dictionary();

	public deltadictionary()
	{
	}

	public int size()
	{
		return currentValue.size();
	}

	public Enumeration keys()
	{
		return currentValue.keys();
	}

	public Set keySet()
	{
		return currentValue.keySet();
	}

	public boolean hasKey(Object key)
	{
		return currentValue.containsKey(key);
	}

    /**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, Object value)
	{
		if (value == null)
		{
			System.err.println("WARNING: deltadictionary.put passed null value");
			Thread.dumpStack();
			return null;
		}

		potentiallyDirty.add(key);
		if (value instanceof Vector)
		{
			// save value as an array instead
			Vector v = (Vector)value;
			if (v.isEmpty())
			{
				System.err.println("WARNING: deltadictionary.put passed empty vector value");
				Thread.dumpStack();
				return null;
			}
			Object test = v.get(0);
			if (test == null)
			{
				System.err.println("WARNING: deltadictionary.put passed vector with null data");
				Thread.dumpStack();
				return null;
			}
			if (test instanceof Integer)
			{
				int[] newValue = new int[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Integer)v.get(i)).intValue();
				value = newValue;
			}
			else if (test instanceof Long)
			{
				long[] newValue = new long[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Long)v.get(i)).longValue();
				value = newValue;
			}
			else if (test instanceof Float)
			{
				float[] newValue = new float[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Float)v.get(i)).floatValue();
				value = newValue;
			}
			else if (test instanceof Double)
			{
				double[] newValue = new double[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Double)v.get(i)).doubleValue();
				value = newValue;
			}
			else if (test instanceof Boolean)
			{
				boolean[] newValue = new boolean[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Boolean)v.get(i)).booleanValue();
				value = newValue;
			}
			else if (test instanceof Character)
			{
				char[] newValue = new char[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Character)v.get(i)).charValue();
				value = newValue;
			}
			else if (test instanceof Byte)
			{
				byte[] newValue = new byte[v.size()];
				for ( int i = 0; i < newValue.length; ++i )
					newValue[i] = ((Byte)v.get(i)).byteValue();
				value = newValue;
			}
			else if (test instanceof String)
			{
				String[] newValue = new String[v.size()];
				newValue = (String[])v.toArray(newValue);
				value = newValue;
			}
			else if (test instanceof location)
			{
				location[] newValue = new location[v.size()];
				newValue = (location[])v.toArray(newValue);
				value = newValue;
			}
			else if (test instanceof string_id)
			{
				string_id[] newValue = new string_id[v.size()];
				newValue = (string_id[])v.toArray(newValue);
				value = newValue;
			}
			else if (test instanceof obj_id)
			{
				obj_id[] newValue = new obj_id[v.size()];
				newValue = (obj_id[])v.toArray(newValue);
				value = newValue;
			}
			else if (test instanceof transform)
			{
				transform[] newValue = new transform[v.size()];
				newValue = (transform[])v.toArray(newValue);
				value = newValue;
			}
			else if (test instanceof vector)
			{
				vector[] newValue = new vector[v.size()];
				newValue = (vector[])v.toArray(newValue);
				value = newValue;
			}
			else
			{
				System.err.println("WARNING: deltadictionary.put passed unhandled vector data type " + test.getClass());
				Thread.dumpStack();
				return null;
			}
		}
		return currentValue.put(key, value);
	}	// put(Object, Object)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, int value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Integer(value));
	}	// put(Object, int)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, long value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Long(value));
	}	// put(Object, long)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, float value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Float(value));
	}	// put(Object, float)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, double value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Double(value));
	}	// put(Object, double)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, boolean value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Boolean(value));
	}	// put(Object, boolean)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, char value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Character(value));
	}	// put(Object, char)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, byte value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new Byte(value));
	}	// put(Object, byte)

	/**
	 * Puts a key-value pair in the dictionary.
	 */
	public Object put(Object key, String value)
	{
		potentiallyDirty.add(key);
		return currentValue.put(key, new String(value));
	}	// put(Object, String)

	public Object getObject(Object key)
	{
		return currentValue.get(key);
	}

	/**
	 * Gets a value from the dictionary.
	 */
	public int getInt(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Integer)
			return ((Integer)value).intValue();
		return 0;
	}	// getInt()

	/**
	 * Gets a value from the dictionary.
	 */
	public int[] getIntArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (int[])value;
	}	// getIntArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public Vector getResizeableIntArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null)
		{
			potentiallyDirty.add(key);
			int[] array = (int[])value;
			Vector resizeableArray = new Vector(array.length + 10);
			for ( int i = 0; i < array.length; ++i )
				resizeableArray.add(new Integer(array[i]));
			return resizeableArray;
		}
		return null;
	}	// getIntArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public long getLong(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Long)
			return ((Long)value).longValue();
		return 0;
	}	// getLong()

	/**
	 * Gets a value from the dictionary.
	 */
	public long[] getLongArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (long[])value;
	}	// getLongArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public float getFloat(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Float)
			return ((Float)value).floatValue();
		return 0;
	}	// getFloat()

	/**
	 * Gets a value from the dictionary.
	 */
	public float[] getFloatArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (float[])value;
	}	// getFloatArray()


	/**
	 * Gets a value from the dictionary.
	 */
	public Vector getResizeableFloatArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null)
		{
			potentiallyDirty.add(key);
			float[] array = (float[])value;
			Vector resizeableArray = new Vector(array.length + 10);
			for ( int i = 0; i < array.length; ++i )
				resizeableArray.add(new Float(array[i]));
			return resizeableArray;
		}
		return null;
	}	// getIntArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public double getDouble(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Double)
			return ((Double)value).doubleValue();
		return 0;
	}	// getDouble()

	/**
	 * Gets a value from the dictionary.
	 */
	public double[] getDoubleArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (double[])value;
	}	// getDoubleArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public boolean getBoolean(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Boolean)
			return ((Boolean)value).booleanValue();
		return false;
	}	// getBoolean()

	/**
	 * Gets a value from the dictionary.
	 */
	public boolean[] getBooleanArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (boolean[])value;
	}	// getBooleanArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public char getChar(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Character)
			return ((Character)value).charValue();
		return 0;
	}	// getChar()

	/**
	 * Gets a value from the dictionary.
	 */
	public char[] getCharArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (char[])value;
	}	// getCharArray()

	/**
	 * Gets a value from the dictionary.
	 */
	public byte getByte(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Byte)
			return ((Byte)value).byteValue();
		return 0;
	}	// getByte()

	/**
	 * Gets a value from the dictionary.
	 */
	public byte[] getByteArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (byte[])value;
	}	// getByteArray()

	/**
	 * Get an obj_var from a dictionary
	 */
	public obj_var getObjVar(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof obj_var)
			return (obj_var)value;
		return null;
	}

	/**
	 * Get an obj_var_list from a dictionary
	 */
	public obj_var_list getObjVarList(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof obj_var_list)
			return (obj_var_list)value;
		return null;
	}

	/**
	 * Get a dictionary from a dictionary
	 */
	public dictionary getDictionary(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof dictionary)
			return (dictionary)value;

		return null;
	}

	/**
	 * Get a location from a dictionary
	 */
	public location getLocation(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof location)
			return (location)value;
		return null;
	}

	/**
	 * Get a location array from a dictionary
	 */
	public location[] getLocationArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (location[])value;
	}

	/**
	 * Get a location array from a dictionary
	 */
	public Vector getResizeableLocationArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null)
			return new Vector(Arrays.asList((location[])value));
		return null;
	}

	public region getRegion(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof region)
			return (region)value;
		return null;
	}

	/**
	 * Get a string id from a dictionary
	 */
	public string_id getStringId(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof string_id)
			return (string_id)value;
		return null;
	}

	/**
	 * Get a string_id array from a dictionary
	 */
	public string_id [] getStringIdArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (string_id[])value;
	}

	/**
	 * Get a string from a dictionary
	 */
	public String getString(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof String)
			return (String)value;
		return null;
	}

	/**
	 * Get a string array from a dictionary
	 */
	public String[] getStringArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (String[])value;
	}

	/**
	 * Get a string array array from a dictionary
	 */
	public String[][] getStringArrayArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (String[][])value;
	}

	/**
	 * Get a string array from a dictionary
	 */
	public Vector getResizeableStringArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null)
			return new Vector(Arrays.asList((String[])value));
		return null;
	}

	/**
	 * Get an obj_id from a dictionary
	 */
	public obj_id getObjId(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if(value != null && value instanceof obj_id)
			return (obj_id)value;
		return null;
	}

	/**
	 * Get an obj_id array from a dictionary
	 */
	public obj_id[] getObjIdArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (obj_id[])value;
	}

	/**
	 * Get an obj_id array from a dictionary
	 */
	public Vector getResizeableObjIdArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null)
			return new Vector(Arrays.asList((obj_id[])value));
		return null;
	}

	/**
	 * Get a transform from a dictionary
	 */
	public transform getTransform(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof transform)
			return (transform)value;
		return null;
	}

	/**
	 * Get a transform array from a dictionary
	 */
	public transform[] getTransformArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (transform[])value;
	}
	/**
	 * Get a transform array from a dictionary
	 */
	public Vector getResizeableTransformArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null)
			return new Vector(Arrays.asList((transform[])value));
		return null;
	}


	/**
	 * Get a vector from a dictionary
	 */
	public vector getVector(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof vector)
			return (vector)value;
		return null;
	}

	/**
	 * Get a vector array from a dictionary
	 */
	public vector[] getVectorArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		return (vector[])value;
	}

	/**
	 * Get a vector array from a dictionary
	 */
	public Vector getResizeableVectorArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null)
			return new Vector(Arrays.asList((vector[])value));
		return null;
	}

	/**
	 * Tests to see if a value for a key is an int value.
	 */
	public boolean isInt(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof Integer)
			return true;
		return false;
	}	// isInt()

	/**
	 * Tests to see if a value for a key is an array of int values.
	 */
	public boolean isIntArray(Object key)
	{
		Object value = currentValue.get(key);
		potentiallyDirty.add(key);
		if (value != null && value instanceof int[])
			return true;
		return false;
	}	// isIntArray()

	/**
	 * Tests to see if a value for a key is a long value.
	 */
	public boolean isLong(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Long)
			return true;
		return false;
	}	// isLong()

	/**
	 * Tests to see if a value for a key is an array of long values.
	 */
	public boolean isLongArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof long[])
			return true;
		return false;
	}	// isLongArray()

	/**
	 * Tests to see if a value for a key is a float value.
	 */
	public boolean isFloat(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Float)
			return true;
		return false;
	}	// isFloat()

	/**
	 * Tests to see if a value for a key is an array of float values.
	 */
	public boolean isFloatArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof float[])
			return true;
		return false;
	}	// isFloatArray()

	/**
	 * Tests to see if a value for a key is a double value.
	 */
	public boolean isDouble(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Double)
			return true;
		return false;
	}	// isDouble()

	/**
	 * Tests to see if a value for a key is an array of double values.
	 */
	public boolean isDoubleArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof double[])
			return true;
		return false;
	}	// isDoubleArray()

	/**
	 * Tests to see if a value for a key is a boolean value.
	 */
	public boolean isBoolean(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Boolean)
			return true;
		return false;
	}	// isBoolean()

	/**
	 * Tests to see if a value for a key is an array of boolean values.
	 */
	public boolean isBooleanArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof boolean[])
			return true;
		return false;
	}	// isBooleanArray()

	/**
	 * Tests to see if a value for a key is a char value.
	 */
	public boolean isChar(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Character)
			return true;
		return false;
	}	// isChar()

	/**
	 * Tests to see if a value for a key is an array of char values.
	 */
	public boolean isCharArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof char[])
			return true;
		return false;
	}	// isCharArray()

	/**
	 * Tests to see if a value for a key is a byte value.
	 */
	public boolean isByte(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof Byte)
			return true;
		return false;
	}	// isByte()

	/**
	 * Tests to see if a value for a key is an array of byte values.
	 */
	public boolean isByteArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof byte[])
			return true;
		return false;
	}	// isByteArray()

	/**
	 * Tests to see if a value for a key is a dictionary value.
	 */
	public boolean isDictionary(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof dictionary)
			return true;
		return false;
	}	// isDictionary()

	/**
	 * Tests to see if a value for a key is a string_id value.
	 */
	public boolean isStringId(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof string_id)
			return true;
		return false;
	}	// isStringId()

	/**
	 * Tests to see if a value for a key is an array of string_id values.
	 */
	public boolean isStringIdArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof string_id[])
			return true;
		return false;
	}	// isStringIdArray()

	/**
	 * Tests to see if a value for a key is a String value.
	 */
	public boolean isString(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof String)
			return true;
		return false;
	}	// isString()

	/**
	 * Tests to see if a value for a key is an array of String values.
	 */
	public boolean isStringArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof String[])
			return true;
		return false;
	}	// isStringArray()

	/**
	 * Tests to see if a value for a key is an array of array of String values.
	 */
	public boolean isStringArrayArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof String[][])
			return true;
		return false;
	}	// isStringArrayArray()


	/**
	 * Tests to see if a value for a key is a location value.
	 */
	public boolean isLocation(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof location)
			return true;
		return false;
	}	// isLocation()

	/**
	 * Tests to see if a value for a key is an array of location values.
	 */
	public boolean isLocationArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof location[])
			return true;
		return false;
	}	// isLocationArray()

	/**
	 * Tests to see if a value for a key is an obj_id value.
	 */
	public boolean isObjId(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof obj_id)
			return true;
		return false;
	}	// idObjId()

	/**
	 * Tests to see if a value for a key is am array of obj_id values.
	 */
	public boolean isObjIdArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof obj_id[])
			return true;
		return false;
	}	// idObjIdArray()

	/**
	 * Tests to see if a value for a key is a transform value.
	 */
	public boolean isTransform(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof transform)
			return true;
		return false;
	}

	/**
	 * Tests to see if a value for a key is an array of transform values.
	 */
	public boolean isTransformArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof transform[])
			return true;
		return false;
	}

	/**
	 * Tests to see if a value for a key is a vector value.
	 */
	public boolean isVector(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof vector)
			return true;
		return false;
	}

	/**
	 * Tests to see if a value for a key is an array of vector values.
	 */
	public boolean isVectorArray(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof vector[])
			return true;
		return false;
	}

	/**
	 * Tests to see if a value for a key is an obj_var value.
	 */
	public boolean isObjVar(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof obj_var)
			return true;
		return false;
	}	// isObjVar()

	/**
	 * Tests to see if a value for a key is an obj_var_list value.
	 */
	public boolean isObjVarList(Object key)
	{
		Object value = currentValue.get(key);
		if (value != null && value instanceof obj_var_list)
			return true;
		return false;
	}	// isObjVarList()

	public void unpack(byte[] source)
	{
		currentValue = dictionary.unpack(source);
		if (currentValue == null)
		{
			System.err.println("WARNING: failed to unpack deltadictionary, setting currentValue to empty dictionary");
			currentValue = new dictionary();
		}
	}

	public void unpackDelta(byte[] input) throws IOException, ClassNotFoundException
	{
		ByteArrayInputStream byteInput = new ByteArrayInputStream(input);
		ObjectInputStream in = new ObjectInputStream(byteInput);

		int count = in.readInt();
		int j = 0;
		while(j < count)
		{
			j++;
			byte cmd = in.readByte();
			Object key = null;
			key = in.readObject();


			if(key != null)
			{
				switch(cmd)
				{
					case 0:
					{
						currentValue.remove(key);
					}
					break;
					case 1:
					{
						Object value = in.readObject();
						if(value != null)
						{
							currentValue.put(key, value);
						}
					}
					break;
					default:
						break;
				}
			}
		}
	}

	public void clearDelta()
	{
		potentiallyDirty.clear();
	}

	public byte[] packDelta() throws IOException
	{
		byte[] result = null;
		
		try
		{
			ByteArrayOutputStream byteOutput = new ByteArrayOutputStream();
			ObjectOutputStream out = new ObjectOutputStream(byteOutput);

			Iterator i = potentiallyDirty.iterator();
			boolean wasDirty = false;
			out.writeInt(potentiallyDirty.size());

			i = potentiallyDirty.iterator();
			while(i.hasNext())
			{
				Object key = i.next();
				if(key != null)
				{
					Object cv = currentValue.get(key);
					if (cv != null)
					{
						wasDirty = true;
						out.writeByte(1);
						out.writeObject(key);
						out.writeObject(cv);
					}
					else
					{
						wasDirty = true;
						out.writeByte(0);
						out.writeObject(key);
					}
				}
			}

			potentiallyDirty.clear();
			if(wasDirty)
			{
				out.flush();
				byteOutput.flush();
				result = byteOutput.toByteArray();
			}
		}
		catch (java.io.IOException e)
		{
			System.err.println("ERROR in initialization of dictionary: " + e.getMessage());
		}
		
		return result;
	}

	public byte[] pack()
	{
		return currentValue.pack();
	}

	public String toString()
	{
		return currentValue.toString();
	}

	/**
	 * Removes an element from the dictionary
	 */
	public Object remove(Object key)
	{
		potentiallyDirty.add(key);
		return currentValue.remove(key);
	}

	/**
	 * Removes all elements from the dictionary.
	 */
	public void removeAll()
	{
		Enumeration keys = currentValue.keys();
		while ( keys.hasMoreElements() )
			potentiallyDirty.add(keys.nextElement());
		currentValue.clear();
	}
}
