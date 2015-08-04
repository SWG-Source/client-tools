/**
 * Title:        mental_state
 * Description:  Wrapper for an mental_state.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.Serializable;

public class mental_state implements Comparable, Serializable
{
	private final static long serialVersionUID = 346089957458143933L;

	private int m_type;
	private float m_value;

	/**
	 * Class constructor.
	 *
	 * @param type		mental_state type
	 * @param value		mental_state value
	 */
	public mental_state(int type, float value)
	{
		m_type = type;
		m_value = value;
	}	// mental_state

	 /**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	 public mental_state(mental_state src)
	 {
		m_type = src.m_type;
		m_value = src.m_value;
	 }	// mental_state(mental_state)

	/**
	 * Accessor function.
	 *
	 * @return the mental_state type
	 */
	public int getType()
	{
		return m_type;
	}	// gettype

	/**
	 * Accessor function.
	 *
	 * @return the mental_state value
	 */
	public float getValue()
	{
		return m_value;
	}	// getValue

	/**
	 * Conversion function.
	 *
	 * @return the mental_state as a string.
	 */
	public String toString()
	{
		return "(type=" + Integer.toString(m_type) +
			", value=" + Float.toString(m_value) +
			")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is an mental_state, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((mental_state)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another mental_state.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(mental_state id)
	{
		if (m_type == id.m_type)
      if (m_value < id.m_value)
        return -1;
      else
        return 1;
    else
      return m_type - id.m_type;
	}	// compareTo(mental_state)

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

}	// class mental_state
