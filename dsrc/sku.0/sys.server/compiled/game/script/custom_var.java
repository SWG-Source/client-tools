// ======================================================================
//
// custom_var.java
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

package script;

public class custom_var
{
	public static final int UNKNOWN_TYPE      = 0;
	public static final int CONST_STRING_TYPE = 1;
	public static final int RANGED_INT_TYPE   = 2;
	public static final int PALCOLOR_TYPE     = 3;

	/**
	 * Retrieve the obj_id for the Object influenced by this customization
	 * variable.
	 *
	 * @return  the obj_id for the Object influenced by this customization
	 *          variable.
	 */
	public obj_id getObjId()
		{
			return m_objId;
		}

	/**
	 * Retrieve the full pathname of the customization variable.
	 *
	 * @return the full pathname of the customization variable.
	 */
	public String getVarName()
		{
			return m_varName;
		}

	/**
	 * Retrieve the type id for the most-derived type of this
	 * custom_var instance.
	 *
	 * @return  the type id for the most-derived type of this
	 *          custom_var instance.
	 */
	public int getTypeId()
		{
			return m_varTypeId;
		}

	/**
	 * Determine if this custom_var instance can be cast successfully
	 * to a ranged_int_custom_var.
	 *
	 * @return  true if the custom_var instance can be cast successfully
	 *          to a ranged_int_custom_var; false otherwise.
	 */
	public boolean isRangedInt()
		{
			return (m_varTypeId == RANGED_INT_TYPE) || (m_varTypeId == PALCOLOR_TYPE);
		}

	/**
	 * Determine if this custom_var instance can be cast successfully
	 * to a palcolor_custom_var.
	 *
	 * @return  true if the custom_var instance can be cast successfully
	 *          to a palcolor_custom_var; false otherwise.
	 */
	public boolean isPalColor()
		{
			return m_varTypeId == PALCOLOR_TYPE;
		}

	/**
	 * Determine if this custom_var instance can be cast successfully
	 * to a const_string_custom_var.
	 *
	 * @return  true if the custom_var instance can be cast successfully
	 *          to a const_string_custom_var; false otherwise.
	 */
	public boolean isConstString()
		{
			return m_varTypeId == CONST_STRING_TYPE;
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
			return "[custom_var: objId = " + m_objId + ", varName = " + getVarName() + ", typeId = " + Integer.toString(getTypeId()) + "]";
		}

	/**
	 * Construct a custom_var instance.
	 *
	 * Scripters should not try to construct these by hand.  custom_var
	 * instances are returned via the script function getAllCustomVars().
	 *
	 * @param objId      the obj_id for the Object containing the customization variable.
	 * @param varName    the full pathname for the customization variable.
	 * @param varTypeId  the type id of the variable.
	 *
	 * @see base_class.getAllCustomVars()
	 */
	protected custom_var(long objId, String varName, int varTypeId)
		{
			m_objId     = (objId == 0) ? null : obj_id.getObjId(objId);
			m_varName   = varName;
			m_varTypeId = varTypeId;
		}
	protected custom_var(obj_id objId, String varName, int varTypeId)
		{
			m_objId     = objId;
			m_varName   = varName;
			m_varTypeId = varTypeId;
		}

	private final obj_id  m_objId;
	private final String  m_varName;
	private final int     m_varTypeId;
};
