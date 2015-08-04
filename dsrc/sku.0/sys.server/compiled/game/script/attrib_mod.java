/**
 * Title:        attrib_mod
 * Description:  Wrapper for an attribute modification.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

import java.io.Serializable;

public final class attrib_mod implements Comparable, Serializable
{
	private final static long serialVersionUID = 1253041308943833323L;

	// m_flags values
	private final static int AMF_changeMax     = 0x00000001;      // the mod affects the max attrib value
	private final static int AMF_attackCurrent = 0x00000002;      // the mod attack/sustain affects the current attrib value
	private final static int AMF_decayCurrent  = 0x00000004;      // the mod decay affects the current attrib value
	private final static int AMF_triggerOnDone = 0x00000008;      // cause a trigger to go off when the mod ends
	private final static int AMF_visible       = 0x00000010;      // the attrib mod info should be sent to the player
	private final static int AMF_skillMod      = 0x00000020;      // this mod affects skill mods, not attribs
	private final static int AMF_directDamage  = 0x00000040;      // the mod is direct damage that should be applied to the creature

	private String m_name;      // mod name (optional)
	private String m_skill;		// which skill the modifier affects (if this is a skill mod)
	private int m_attrib;		// which attribute the modifier affects (if this is an attrib mod)
	private int m_value;		// the modifier value
	private float m_duration;	// how long it lasts
	private float m_attack;		// time it takes to go from 0 to m_value
	private float m_decay;		// time it takes to go from m_value to 0
	private int m_flags;		// flags the behavior of the mod


	/**
	 * Class constructor. This is for use when creating an attrib_mod from C.
	 */
	private attrib_mod()
	{
	}

	/**
	 * Class constructor. Makes an old-style attrib mod.
	 *
	 * @param attrib		modifier attribute
	 * @param value			modifier value
	 * @param duration		how long it lasts
	 * @param attack		time it takes to go from 0 to m_value
	 * @param decay			time it takes to go from m_value to 0 (MOD_WOUND = wound, MOD_FAUCET = use attrib faucet)
	 */
	public attrib_mod(int attrib, int value, float duration, float attack, float decay)
	{
		m_name = null;
		m_skill = null;
		m_attrib = attrib;
		m_value = value;
		m_duration = duration;
		m_attack = attack;
		m_decay = decay;
		m_flags = AMF_changeMax | AMF_attackCurrent | AMF_decayCurrent;
		if (attack == 0 && duration == 0 && decay == base_class.MOD_POOL)
		{
			m_flags |= AMF_directDamage;
		}
	}	// attrib_mod

	/**
	 * Class constructor. Makes a new-style attrib mod.
	 *
	 * @param name          	the mod name
	 * @param attrib			modifier attribute
	 * @param value				modifier value
	 * @param duration			how long it lasts
	 * @param attack			time it takes to go from 0 to m_value
	 * @param decay				time it takes to go from m_value to 0 (MOD_WOUND = wound, MOD_FAUCET = use attrib faucet)
	 * @param changeCurrent		flag to update the current value before the decay time is reached
	 * @param triggerOnDone		flag to trigger OnAttribModDone when the mod is removed from the creature
	 * @param visible			flag to send a message to the player giving info about the mod
	 */
	public attrib_mod(String name, int attrib, int value, float duration, float attack, float decay,
		boolean changeCurrent, boolean triggerOnDone, boolean visible)
	{
		m_name = name;
		m_skill = null;
		m_attrib = attrib;
		m_value = value;
		m_duration = duration;
		m_attack = attack;
		m_decay = decay;
		m_flags = AMF_changeMax;
		if (changeCurrent)
			m_flags |= AMF_attackCurrent;
		if (triggerOnDone)
			m_flags |= AMF_triggerOnDone;
		if (visible)
			m_flags |= AMF_visible;
	}	// attrib_mod

	/**
	 * Class constructor. Makes a new-style attrib mod, as a damage/heal over time mod.
	 *
	 * @param attrib			modifier attribute
	 * @param value				modifier value
	 * @param duration			how long it lasts
	 */
	public attrib_mod(int attrib, int value, float duration)
	{
		m_name = null;
		m_skill = null;
		m_attrib = attrib;
		m_value = value;
		m_duration = 0;
		m_attack = duration;
		m_decay = base_class.MOD_POOL;
		m_flags = AMF_attackCurrent;
	}	// attrib_mod

	/**
	 * Class constructor. Makes a new-style attrib mod, as a skillmod mod.
	 *
	 * @param name          	the mod name
	 * @param skill				the skill to effect
	 * @param value				modifier value
	 * @param duration			how long it lasts
	 * @param triggerOnDone		flag to trigger OnSkillModDone when the mod is removed from the creature
	 * @param visible			flag to send a message to the player giving info about the mod
	 */
	public attrib_mod(String name, String skill, int value, float duration, boolean triggerOnDone, boolean visible)
	{
		m_name = name;
		m_skill = skill;
		m_value = value;
		m_duration = duration;
		m_attack = 0;
		m_decay = 0;
		m_flags = AMF_skillMod;
		if (triggerOnDone)
			m_flags |= AMF_triggerOnDone;
		if (visible)
			m_flags |= AMF_visible;
	}	// attrib_mod

	/**
	 * Class constructor. Unpacks string data created by the pack() function.
	 *
	 * @param packedData		string returned from the pack() function
	 */
	public attrib_mod(String packedData)
	{
		String[] values = base_class.split(packedData, ' ');
		if (values == null || (values.length != 8 && values.length != 5))
		{
			System.err.println("ERROR in Java attrib_mod constructor, invalid data " + packedData);
			m_attrib = -1;
		}
		else if (values.length == 8)
		{
			// new style mod
			m_name = values[0];
			if (m_name.equals("?"))
				m_name = null;
			m_skill = values[1];
			if (m_skill.equals("?"))
				m_skill = null;
			m_attrib = Integer.parseInt(values[2]);
			m_value = Integer.parseInt(values[3]);
			m_duration = Float.parseFloat(values[4]);
			m_attack = Float.parseFloat(values[5]);
			m_decay = Float.parseFloat(values[6]);
			m_flags = Integer.parseInt(values[7]);
		}
		else
		{
			// old style mod
			m_name = null;
			m_skill = null;
			m_attrib = Integer.parseInt(values[0]);
			m_value = Integer.parseInt(values[1]);
			m_duration = Float.parseFloat(values[2]);
			m_attack = Float.parseFloat(values[3]);
			m_decay = Float.parseFloat(values[4]);
			m_flags = AMF_changeMax | AMF_attackCurrent | AMF_decayCurrent;
		}
	}	// attrib_mod

	 /**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	public attrib_mod(attrib_mod src)
	{
		m_name = src.m_name;
		m_skill = src.m_skill;
		m_attrib = src.m_attrib;
		m_value = src.m_value;
		m_duration = src.m_duration;
		m_attack = src.m_attack;
		m_decay = src.m_decay;
		m_flags = src.m_flags;
	}	// attrib_mod(attrib_mod)

	/**
	 * Tests if this mod is an attrib mod.
	 *
	 * @return true if we are an attrib mod, false if not
	 */
	public boolean isAttribMod()
	{
		return (m_flags & AMF_changeMax) != 0;
	}	// isAttribMod

	/**
	 * Tests if this mod is a skillmod mod.
	 *
	 * @return true if we are an skillmod mod, false if not
	 */
	public boolean isSkillModMod()
	{
		return (m_flags & AMF_skillMod) != 0;
	}	// isSkillModMod

	/**
	 * Tests if this mod is a dot/hot mod.
	 *
	 * @return true if we are a dot mod, false if not
	 */
	public boolean isDotMod()
	{
		return !isAttribMod() && !isSkillModMod();
	}	// isDotMod

	/**
	 * Accessor function.
	 *
	 * @return the modifier name
	 */
	public String getName()
	{
		return m_name;
	}	// getName

	/**
	 * Accessor function.
	 *
	 * @return the modifier skillmod name
	 */
	public String getSkillModName()
	{
		return m_skill;
	}	// getSkillModName

	/**
	 * Accessor function.
	 *
	 * @return the modifier attribute
	 */
	public int getAttribute()
	{
		return m_attrib;
	}	// getAttribute

	/**
	 * Accessor function.
	 *
	 * @return the modifier value
	 */
	public int getValue()
	{
		return m_value;
	}	// getValue

	/**
	 * Accessor function.
	 *
	 * @return the modifier duration
	 */
	public float getDuration()
	{
		return m_duration;
	}	// getDuration

	/**
	 * Accessor function.
	 *
	 * @return the modifier attack
	 */
	public float getAttack()
	{
		return m_attack;
	}	// getAttack

	/**
	 * Accessor function.
	 *
	 * @return the modifier decay
	 */
	public float getDecay()
	{
		return m_decay;
	}	// getDecay

	/**
	 * Packs the attrib mod into a string for storing as an objvar.
	 *
	 * @return the packed data
	 */
	public String pack()
	{
		String name = m_name;
		if (name == null || name.length() == 0)
			name = "?";
		String skill = m_skill;
		if (skill == null || skill.length() == 0)
			skill = "?";
		return name + " " +
			skill + " " +
			Integer.toString(m_attrib) + " " +
			Integer.toString(m_value) + " " +
			Float.toString(m_duration) + " " +
			Float.toString(m_attack) + " " +
			Float.toString(m_decay) + " " +
			Integer.toString(m_flags);
	}	// pack

	/**
	 * Conversion function.
	 *
	 * @return the modifier as a string.
	 */
	public String toString()
	{
		String name = m_name;
		if (name == null)
			name = "";
		String skill = m_skill;
		if (skill == null)
			skill = "";
		return "(name=" + name +
			", skill=" + skill +
			", attrib=" + Integer.toString(m_attrib) +
			", value=" + Integer.toString(m_value) +
			", duration=" + Float.toString(m_duration) +
			", attack=" + Float.toString(m_attack) +
			", decay=" + Float.toString(m_decay) +
			", flags=" + Integer.toString(m_flags) +
			")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a attribMod, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((attrib_mod)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another attribMod.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(attrib_mod id)
	{
		if (m_attrib == id.m_attrib)
		{
			if (m_value == id.m_value)
			{
				if (m_duration == id.m_duration)
				{
					if (m_attack == id.m_attack)
					{
						if (m_decay == id.m_decay)
							return 0;
						return m_decay < id.m_decay ? -1 : 1;
					}
					return m_attack < id.m_attack ? -1 : 1;
				}
				return m_duration < id.m_duration ? -1 : 1;
			}
			return m_value - id.m_value;
		}
		return m_attrib - id.m_attrib;
	}	// compareTo(attrib_mod)

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

}	// class objId
