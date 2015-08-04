/**
 * Title:        location
 * Description:  Represents a point in space.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.lang.Math;
import java.io.Serializable;

public final class location implements Comparable, Serializable
{
	private final static long serialVersionUID = -4232526054297175136L;
	
	// for equality checks between 2 locations
	private final static float LOCATION_EPSILON = 0.001f;

	/**
	 * Due to the desire for quick access and legacy standards, this is the only
	 * class to have it's data members public.
	 */

	// Cartesian coords
	public float x;
	public float y;
	public float z;

	// planet/area (in space?)
	public String area;			// @todo: set this to correct type

	// interior cell (null/0 = world)
	public obj_id cell;


	/**
	 * Class constructor.
	 */
	public location()
	{
		x = 0;
		y = 0;
		z = 0;
		area = base_class.getCurrentSceneName();		// @todo: set to a good neutral value
		cell = null;
	}	// location()

	/**
	 * Class constructor.
	 *
	 * @param x			x coord
	 * @param y			y coord
	 * @param z			z coord
	 */
	public location(float x, float y, float z)
	{
		this.x = x;
		this.y = y;
		this.z = z;
		this.area = base_class.getCurrentSceneName();		// @todo: set to a good neutral value
		cell = null;
	}	// location(float, float, float, String)

	/**
	 * Class constructor.
	 *
	 * @param x			x coord
	 * @param y			y coord
	 * @param z			z coord
	 * @param area		area/planet id
	 */
	public location(float x, float y, float z, String area)
	{
		this.x = x;
		this.y = y;
		this.z = z;
		this.area = area;
		cell = null;
	}	// location(float, float, float, String)

	/**
	 * Class constructor.
	 *
	 * @param x			x coord
	 * @param y			y coord
	 * @param z			z coord
	 * @param area		area/planet id
	 * @param cell		interior cell
	 */
	public location(float x, float y, float z, String area, obj_id cell)
	{
		this.x = x;
		this.y = y;
		this.z = z;
		this.area = area;
		this.cell = cell;
	}	// location(float, float, float, String, obj_id)

	 /**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	public location(location src)
	{
		this.x = src.x;
		this.y = src.y;
		this.z = src.z;
		this.area = src.area;
		this.cell = src.cell;
	}	// location(location)

	/**
	 * Clone function.
	 *
	 * @return a copy of this object
	 */
	public Object clone()
	{
		return new location(this);
	}	// clone

	 /**
	  * Assignment function.
	  *
	  * @param src		class instance to copy
	  */
	public void assign(location src)
	{
		if (src != null)
		{
			this.x = src.x;
			this.y = src.y;
			this.z = src.z;
			this.area = src.area;
			this.cell = src.cell;
		}
	}	// assign


	/**
	 * Conversion function.
	 *
	 * @return the location as a string.
	 */
	public String toString()
	{
		return "(x=" + x + ", y=" + y + ", z=" + z + ", area=" + area + ", cell=" + cell + ")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a location, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((location)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another location.
	 *
	 * @returns <, =, or > 0
	 *
	 * @todo: should we use a different way to find > or <?
	 */
	public int compareTo(location loc)
	{
		int result = area.compareTo(loc.area);

		if (result == 0 &&
		    ((x < (loc.x - LOCATION_EPSILON)) || (x > (loc.x + LOCATION_EPSILON)) ||
		     (y < (loc.y - LOCATION_EPSILON)) || (y > (loc.y + LOCATION_EPSILON)) ||
		     (z < (loc.z - LOCATION_EPSILON)) || (z > (loc.z + LOCATION_EPSILON))))
		{
			int count = 0;
			if (x > (loc.x + LOCATION_EPSILON))
				++count;
			if (y > (loc.y + LOCATION_EPSILON))
				++count;
			if (z > (loc.z + LOCATION_EPSILON))
				++count;
			if (count > 1)
				result = 1;
			else
				result = -1;
		}
		return result;
	}	// compareTo(location)

	/**
	 * Compares this to a generic object.
	 *
	 * @returns true if the objects have the same data, false if not
	 */
	public boolean equals(Object o)
	{
		if (o != null)
		{
			try
			{
				location l = (location)o;
				if ((x > (l.x - LOCATION_EPSILON)) && (x < (l.x + LOCATION_EPSILON)) &&
				    (y > (l.y - LOCATION_EPSILON)) && (y < (l.y + LOCATION_EPSILON)) &&
				    (z > (l.z - LOCATION_EPSILON)) && (z < (l.z + LOCATION_EPSILON)) &&
				    (cell == l.cell))
				{
					if ((area == null && l.area == null) || (area != null && l.area != null && area.equals(l.area)))
						return true;
				}
			}
			catch (ClassCastException err)
			{
			}
		}
		return false;
	}	// equals

	/**
	 * Tells the distance between two locations.
     *
	 * @param l		the location to test against
	 *
	 * @return the distance, or -1 if the location is not in the same area as we are
	 */
	public float distance(location l)
	{
		// no valid distance between planets
		if (l == null || !area.equals(l.area))
			return -1.0f;

		// check if the points are in the world
		if (cell == l.cell || ((cell == null || cell == obj_id.NULL_ID) && (l.cell == null || l.cell == obj_id.NULL_ID)))
		{
			float dx = x - l.x;
			float dy = y - l.y;
			float dz = z - l.z;
			return (float)Math.sqrt(dx*dx+dy*dy+dz*dz);
		}
		else
		{
			// we need to call into C so the coordinates can be translated to worldspace
			return base_class.getDistance(this, l);
		}
	}
}	// class location
