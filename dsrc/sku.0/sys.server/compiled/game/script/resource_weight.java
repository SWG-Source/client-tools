/**
 * Title:        resource_weight.java
 * Description:  resource data used during crafting/manufacturing
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;


// This class is used to map a resource id to how much it influences crafting data
public class resource_weight
{
	public static class weight
	{
		public int resource;
		public int weight;

		public weight(int r, int w)
		{
			resource = r;
			weight = w;
		}
	}

	public String   attribName;
	public int      slot = -1;
	public weight[] weights;

	public resource_weight(String a, weight[] w)
	{
		attribName = a;
		weights = w;
	}

	public resource_weight(String a, int s, weight[] w)
	{
		attribName = a;
		slot = s;
		weights = w;
	}
}

