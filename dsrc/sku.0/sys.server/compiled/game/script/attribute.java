/**
 * Title:        attribute
 * Description:  Wrapper for an attribute.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.Serializable;

public final class attribute implements Comparable, Serializable
{
	private final static long serialVersionUID = 2234290627397819837L;

	private int m_type;
	private int m_value;

	/**
	 * Class constructor.
	 *
	 * @param type		attribute type
	 * @param value		attribute value
	 */
	public attribute(int type, int value)
	{
		m_type = type;
		m_value = value;
	}	// attribute

	 /**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	 public attribute(attribute src)
	 {
		m_type = src.m_type;
		m_value = src.m_value;
	 }	// attribute(attribute)

	/**
	 * Accessor function.
	 *
	 * @return the attribute type
	 */
	public int getType()
	{
		return m_type;
	}	// gettype

	/**
	 * Accessor function.
	 *
	 * @return the attribute value
	 */
	public int getValue()
	{
		return m_value;
	}	// getValue

	/**
	 * Conversion function.
	 *
	 * @return the attribute as a string.
	 */
	public String toString()
	{
		return "(type=" + Integer.toString(m_type) +
			", value=" + Integer.toString(m_value) +
			")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is an attribute, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((attribute)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another attribute.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(attribute id)
	{
		if (m_type == id.m_type)
			return m_value - id.m_value;
		return m_type - id.m_type;
	}	// compareTo(attribute)

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

}	// class attribute
