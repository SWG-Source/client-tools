/**
 * Title:        map_location
 * Description:  Wrapper for a map_location
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @version 1.0
 */

package script;

public class map_location
{
	protected obj_id          m_locationId;
	protected String          m_locationName;
	protected String          m_category;
	protected String          m_subCategory;
	protected long            m_x;
	protected long            m_y;
	protected byte            m_flags;

	public map_location (long locationId, String locationName, String category, String subCategory, long x, long y, byte flags)
	{
		m_locationId = (locationId == 0) ? null : obj_id.getObjId(locationId);
		m_locationName = locationName;
		m_category = category;
		m_subCategory = subCategory;
		m_x = x;
		m_y = y;
		m_flags = flags;
	}

	public map_location (obj_id locationId, String locationName, String category, String subCategory, long x, long y, byte flags)
	{
		m_locationId = locationId;
		m_locationName = locationName;
		m_category = category;
		m_subCategory = subCategory;
		m_x = x;
		m_y = y;
		m_flags = flags;
	}

	public obj_id getLocationId()
	{
		return m_locationId;
	}

	public String getLocationName()
	{
		return m_locationName;
	}

	public String getCategory()
	{
		return m_category;
	}

	public String getSubCategory()
	{
		return m_subCategory;
	}

	public long getX()
	{
		return m_x;
	}

	public long getY()
	{
		return m_y;
	}

	public byte getFlags()
	{
		return m_flags;
	}

	public boolean isInactive()
	{
		return (m_flags & base_class.MLF_INACTIVE) != 0;
	}

	public boolean isActive()
	{
		return (m_flags & base_class.MLF_ACTIVE) != 0;
	}
}	// class map_location
