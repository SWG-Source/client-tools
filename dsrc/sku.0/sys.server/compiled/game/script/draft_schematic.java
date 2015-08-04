/**
 * Title:        draft_schematic
 * Description:  Info about a draft schematic for use in a crafting session.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.util.Hashtable;


public class draft_schematic
{
	// enum IngredientType
	public static final int IT_none             = 0;		// empty slot
	public static final int IT_item             = 1;		// a specific item (Skywalker barrel mark V)
	public static final int IT_template		    = 2;		// any item created from a template (any small_blaster_barrel)
	public static final int IT_resourceType	    = 3;		// a specific resource type (iron type 5)
	public static final int IT_resourceClass    = 4;		// any resource of a resource class (ferrous metal)
	public static final int IT_templateGeneric  = 5;		// same as IT_template, but if multiple components are required, they don't have to be the same exact type as the first component used
	public static final int IT_schematic        = 6;		// item crafted from a draft schematic
	public static final int IT_schematicGeneric = 7;		// same as IT_schematic, but if multiple components are required, they don't have to be the same exact type as the first component used

	public static class simple_ingredient
	{
		public obj_id	    ingredient;			// id of ingredient
		public int			count;				// number of ingredients
		public obj_id       source;             // who supplied the ingredient
		public int          xpType;             // type of xp to grant the source
		public int          xpAmount;           // amount of xp to grant
	}	// simple_ingredient

	public static class slot
	{
		public string_id           name;				// slot name
		public int                 slotOption;			// slot option selected
		public int                 ingredientType;		// type of ingredient used (from enum IngredientType)
		public String              ingredientName;      // reource class or template name for the slot
		public int                 amountRequired;		// number of ingredients needed to fill this slot
		public float               complexity;			// adjustment to complexity by using this slot
		public simple_ingredient[] ingredients;			// ingredients that are currently filling this slot
		public String              appearance;  		// hardpoint (for visible components) or appearance string used to define optional appearances for the object

		public slot() {}
	}	// class slot

	public static class attribute
	{
		public string_id name;
		public float     minValue;
		public float     maxValue;				// the maximum value the item would ever be able to have
		public float     resourceMaxValue;		// the maximum value the player will be able to get the
		                                        // attribute due to resources used, <= maxValue
		public float     currentValue;

		public int       scratch;				// dummy data for computational uses

		public attribute() {}

		// functions needed for mapping
		public int hashCode()
		{
			return name.getAsciiId().hashCode();
		}

		public boolean equals(Object obj)
		{
			return obj == this;
		}
	}	// class attribute

	public static class custom
	{
		public String  name;
		public int     value;
		public int     minValue;
		public int     maxValue;
//		public boolean locked;

		public custom() {}
	}	// class custom

	private int m_category;								// general crafting category (from enum CraftingType)
	private float m_complexity;							// base complexity for the object
	private slot[] m_slots = null;						// schematic slots used to make the object
	private attribute[] m_attribs = null;				// attributes the schematic can affect
	private attribute[] m_experimentalAttribs = null;	// attributes seen by the player during experimentation
	private custom[] m_customizations = null;			// customization data for objects created by this schematic
	private Hashtable m_attribMap = null;   			// map of object attributes to experimental attributes (assembly), or
														// map of experimental attributes to an array of object attributes (experimentation)
	private int m_objectTemplateCreated = 0;            // object template crc for the object the schematic creates
	private String[] m_scripts = null;                  // scripts that gets attached to the manf schematic/factory crate

	/**
	 * Class constructor, which does nothing, since this class should only be
	 * constructed by C code, which should fill in the values directly.
	 **/
	private draft_schematic()
	{
	}	// draft_schematic()

	public int getCategory()
	{
		return m_category;
	}	// getCategory()

	public float getBaseComplexity()
	{
		return m_complexity;
	}	// getBaseComplexity()

	public int getObjectTemplateCreated()
	{
		return m_objectTemplateCreated;
	}

	public String[] getScripts()
	{
		return m_scripts;
	}

	public slot[] getSlots()
	{
		return m_slots;
	}	// getSlots()

	public attribute[] getAttribs()
	{
		return m_attribs;
	}	// getAttribs()

	public attribute[] getExperimentalAttribs()
	{
		return m_experimentalAttribs;
	}	// getAttribs()

	public custom[] getCustomizations()
	{
		return m_customizations;
	}	// getCustomizations()

	public Hashtable getAttribMap()
	{
		return m_attribMap;
	} 	// getAttribMap()

	public attribute getExperimentalAttrib(attribute objectAttrib)
	{
		if ( m_attribMap == null )
			return null;

		return (attribute)m_attribMap.get(objectAttrib);
	}	// getExperimentalAttrib()

	public void setExperimentalAttrib(attribute objectAttrib, attribute experimentAttrib)
	{
		if ( m_attribMap != null )
			m_attribMap.put(objectAttrib, experimentAttrib);
	}	// setExperimentalAttrib()

	public attribute[] getObjectAttribs(attribute experimentalAttrib)
	{
		if ( m_attribMap == null )
			return null;

		return (attribute[])m_attribMap.get(experimentalAttrib);
	}	// getObjectAttribs()

}	// class draft_schematic
