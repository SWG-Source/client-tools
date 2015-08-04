// ======================================================================
//
// ranged_int_custom_var.java
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

package script;

public class ranged_int_custom_var extends custom_var
{
	/**
	 * Retrieve the current value of this ranged int variable.
	 *
	 * @return  the current value of the variable.
	 */
	public int getValue()
		{
			// We implement this as a call into native code, rather than caching
			// the value here, so we don't ever worry about getting out of sync.
			return base_class.getRangedIntCustomVarValue(getObjId(), getVarName());
		}

	/**
	 * Set the current value of this ranged int variable.
	 *
	 * Changing a customization variable will propagate the
	 * change to all clients and the database.  It will cause any
	 * associated visual changes on the client.
	 *
	 * If the specified value falls outside of the valid range,
	 * this function call does not affect the current value
	 * of this variable.
	 *
	 * @param newValue  the new value to set for the
	 *                  customization variable.
	 */
	public void setValue(int newValue)
		{
			if ((newValue >= m_minRangeInclusive) && (newValue <= m_maxRangeInclusive))
				base_class.setRangedIntCustomVarValue(getObjId(), getVarName(), newValue);
		}

	/**
	 * Get the minimum valid value for this ranged int variable.
	 *
	 * The value of this variable must fall in the range
	 * of getMinRangeInclusive() to getMaxRangeInclusive(),
	 * including the min and max range value.
	 *
	 * @return the minimum allowable value for this variable.
	 */
	public int getMinRangeInclusive()
		{
			return m_minRangeInclusive;
		}

	/**
	 * Get the maximum valid value for this ranged int variable.
	 *
	 * The value of this variable must fall in the range
	 * of getMinRangeInclusive() to getMaxRangeInclusive(),
	 * including the min and max range value.
	 *
	 * @return the maximum allowable value for this variable.
	 */
	public int getMaxRangeInclusive()
		{
			return m_maxRangeInclusive;
		}

	/**
	 * Retrieve a String representation of the instance suitable for
	 * a debug dump.
	 *
	 * The caller should not assume anything about the format of this
	 * output.  It may change at any time.
	 *
	 * @return  a String representation of the instance suitable for
	 *          a debug dump.
	 */
	public String toString()
		{
			return "[ranged_int_custom_var: min_range_inclusive = " + Integer.toString(m_minRangeInclusive) + ", max_range_inclusive = " + Integer.toString(m_maxRangeInclusive) + ", custom_var = " + super.toString() + "]";
		}

	/**
	 * Construct a ranged_int_custom_var instance.
	 *
	 * Scripters should not try to construct these by hand.  These
	 * are created from the C++ native code.
	 *
	 * @param objId              the obj_id for the Object containing the variable.
	 * @param varName            the name of the customization variable.
	 * @param minRangeInclusive  the minimum value that this variable's value can take on.
	 * @param maxRangeExclusive  one greater than the maximum value this variable can take on.
	 */
	protected ranged_int_custom_var(long objId, String varName, int varTypeId, int minRangeInclusive, int maxRangeExclusive)
		{
			super(objId, varName, varTypeId);

			// We store max range inclusive because that is how the designers
			// would like to access the max value.
			m_minRangeInclusive = minRangeInclusive;
			m_maxRangeInclusive = maxRangeExclusive - 1;
		}
	protected ranged_int_custom_var(obj_id objId, String varName, int varTypeId, int minRangeInclusive, int maxRangeExclusive)
		{
			super(objId, varName, varTypeId);

			// We store max range inclusive because that is how the designers
			// would like to access the max value.
			m_minRangeInclusive = minRangeInclusive;
			m_maxRangeInclusive = maxRangeExclusive - 1;
		}

	private final int m_minRangeInclusive;
	private final int m_maxRangeInclusive;
}
