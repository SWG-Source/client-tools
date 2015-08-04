/**
 * Title:        slot_data
 * Description:  Wrapper for slot info.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

public class slot_data implements Comparable
{
	private int m_slotNumber;
	private int m_slotType;

	/**
	 * Class constructor.
	 *
	 * @param number	the slot number
	 * @param type		the slot type
	 */
	public slot_data(int number, int type)
	{
		m_slotNumber = number;
		m_slotType = type;
	}	// slot_data

	/**
	 * Copy constructor.
	 *
	 * @param src		class instance to copy
	 */
	public slot_data(slot_data src)
	{
		m_slotNumber = src.m_slotNumber;
		m_slotType = src.m_slotType;
	}	// slot_data(slot_data)

	/**
	 * Accessor function.
	 *
	 * @return the slot number
	 */
	public int getSlotNumber()
	{
		return m_slotNumber;
	}	// getSlotNumber

	/**
	 * Accessor function.
	 *
	 * @return the slot type
	 */
	public int getSlotType()
	{
		return m_slotType;
	}	// getSlotType

	/**
	 * Conversion function.
	 *
	 * @return the slot as a string.
	 */
	public String toString()
	{
		return "(number=" + Integer.toString(m_slotNumber) + ", type=" +
			Integer.toString(m_slotType) + ")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a slot_data, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((slot_data)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another slot_data.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(slot_data data)
	{
		if (m_slotNumber == data.m_slotNumber)
			return m_slotType - data.m_slotType;
		return m_slotNumber - data.m_slotNumber;
	}	// compareTo(slot_data)

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

}	// class slot_data
