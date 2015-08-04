/**
 * Title:        mental_state_mod
 * Description:  Wrapper for a mental state modification.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Acy Stapp
 * @version 1.0
 */

package script;

import java.io.Serializable;

public class mental_state_mod implements Comparable, Serializable
{
	private final static long serialVersionUID = 5947703242994479498L;

	private int m_mental_state;		// which mental state the modifier affects
	private float m_value;		// the modifier value
	private float m_duration;	// how long it lasts
	private float m_attack;		// time it takes to go from 0 to m_value
	private float m_decay;		// time it takes to go from m_value to 0

	/**
	 * Class constructor.
	 *
	 * @param state		modifier mental state
	 * @param value			modifier value
	 * @param duration		how long it lasts
	 * @param attack		time it takes to go from 0 to m_value
	 * @param decay			time it takes to go from m_value to 0 (MOD_WOUND = wound, MOD_FAUCET = use mentalState faucet)
	 */
	public mental_state_mod(int mentalState, float value, float duration, float attack, float decay)
	{
		m_mental_state = mentalState;
		m_value = value;
		m_duration = duration;
		m_attack = attack;
		m_decay = decay;
	}	// mental_state_mod

	 /**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	public mental_state_mod(mental_state_mod src)
	{
		m_mental_state = src.m_mental_state;
		m_value = src.m_value;
		m_duration = src.m_duration;
		m_attack = src.m_attack;
		m_decay = src.m_decay;
	}	// mental_state_mod(mental_state_mod)

	/**
	 * Accessor function.
	 *
	 * @return the modifier mental state
	 */
	public int getMentalState()
	{
		return m_mental_state;
	}	// getMentalState

	/**
	 * Accessor function.
	 *
	 * @return the modifier value
	 */
	public float getValue()
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
	 * Conversion function.
	 *
	 * @return the modifier as a string.
	 */
	public String toString()
	{
		return "(state=" + Integer.toString(m_mental_state) +
			", value=" + Float.toString(m_value) +
			", duration=" + Float.toString(m_duration) +
			", attack=" + Float.toString(m_attack) +
			", decay=" + Float.toString(m_decay) +
			")";
	}	// toString

	/**
	 * Compares this to a generic object.
	 *
	 * @returns <, =, or > 0 if the object is a mental_state_mod, else throws
	 *		ClassCastException
	 */
	public int compareTo(Object o) throws ClassCastException
	{
		return compareTo((mental_state_mod)o);
	}	// compareTo(Object)

	/**
	 * Compares this to another mental_state_mod.
	 *
	 * @returns <, =, or > 0
	 */
	public int compareTo(mental_state_mod id)
	{
		if (m_mental_state == id.m_mental_state)
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
      if (m_value < id.m_value)
        return -1;
      else
        return 1;
		}
		return m_mental_state - id.m_mental_state;
	}	// compareTo(mental_state_mod)

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
