/**
 * Title:        resource_density
 * Description:  combines a resource type id and its density at a point
 * Copyright:    Copyright (c) 2004
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.Serializable;


public final class resource_density implements Comparable, Serializable
{
	private final static long serialVersionUID =  2730214252807778780L;

	private obj_id m_resourceType;
	private float m_density;

	public resource_density(obj_id resourceType, float density)
	{
		m_resourceType = resourceType;
		m_density = density;
	}

	public obj_id getResourceType()
	{
		return m_resourceType;
	}

	public float getDensity()
	{
		return m_density;
	}

	public int hashCode()
	{
		// formula from Effective Java, item 8
		int result = 17;
		result = 37 * result + Float.floatToIntBits(m_density);
		result = 37 * result + m_resourceType.hashCode();
		return result;
	}

	/**
	 * Conversion function.
	 *
	 * @return the location as a string.
	 */
	public String toString()
	{
		return "(resource="+m_resourceType+", density="+m_density+")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a location, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((resource_density)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another resource_density.
	 *
	 * @returns <, =, or > 0
	 *
	 * @todo: should we add an epsilon for equals test?
	 * @todo: should we use a different way to find > or <?
	 */
	public int compareTo(resource_density src)
	{
		int result = m_resourceType.compareTo(src.m_resourceType);
		if (result == 0)
		{
			float diff = m_density - src.m_density;
			if (diff == 0.0f)
				result = 0;
			else
				result = diff > 0.0f ? 1 : -1;
		}
		return result;
	}	// compareTo(resource_density)

	/**
	 * Compares this to a generic object.
	 *
	 * @returns true if the objects have the same data, false if not
	 *
	 * @todo: should we add an epsilon for the comparison?
	 */
	public boolean equals(Object o)
	{
		if (o != null)
		{
			try
			{
				resource_density rd = (resource_density)o;
				return rd.m_resourceType == m_resourceType && rd.m_density == m_density;
			}
			catch (ClassCastException err)
			{
			}
		}
		return false;
	}	// equals
}
