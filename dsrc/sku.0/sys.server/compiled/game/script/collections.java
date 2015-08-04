/* Copyright (C) 2004 Sony Online Entertainment
 * Title:        collections.java
 * Description:  adds functionality similar to the java.util.Collections class
 * @author       $Author: Steve Jakab$
 * @version      $Revision: 0$
 */

package script;

import java.util.HashMap;
import java.util.Map;

public class collections
{
	/**
	 * Creates a new Map from two parallel arrays. The arrays must be non-null and of the same length.
	 * The data in the keys array must be non-null, but the values data may be null.
	 *
	 * @return the mapping of the keys to the values, or null on error
	 */
	public static Map newMap(Object[] keys, Object[] values)
	{
		if ( keys == null || values == null || keys.length != values.length )
			return null;
		Map map = new HashMap(keys.length);
		for ( int i = 0; i < keys.length; ++i )
		{
			if ( keys[i] == null )
				return null;
			map.put(keys[i], values[i]);
		}
		return map;
	}
}

