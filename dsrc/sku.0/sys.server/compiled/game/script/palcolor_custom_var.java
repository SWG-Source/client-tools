// ======================================================================
//
// palcolor_custom_var.java
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

package script;

public class palcolor_custom_var extends ranged_int_custom_var
{
	/**
	 * Retrieve the color currently selected for this variable.
	 *
	 * The caller could also use ranged_int_custom_var.getValue(), which returns
	 * the index of the color.  The index can be used to look up the color
	 * from the color palette associated with the variable.
	 *
	 * @return  the currently-selected color for this variable instance.
	 *
	 * @see color
	 * @see getAllColors()
	 */
	public color getSelectedColor()
		{
			return base_class.getPalcolorCustomVarSelectedColor(getObjId(), getVarName());
		}

	/**
	 * Set the customization variable to the index of the palette color most closely
	 * matching the specified targetColor.
	 *
	 * @param targetColor  the desired color for the customization variable.
	 */
	public void setToClosestColor(color targetColor)
		{
			base_class.setPalcolorCustomVarClosestColor(getObjId(), getVarName(), targetColor.getR(), targetColor.getG(), targetColor.getB(), targetColor.getA());
		}

	/**
	 * Retrieve all colors available in the palette associated with this variable.
	 *
	 * The caller should not call this frequently as it is an expensive function.
	 * If the caller doesn't need to work directly with color information, it is
	 * far more efficient to select colors using the ranged_int_custom_var functions
	 * for setting and getting the color index as an integer.  The integer represents
	 * the index into the color palette.
	 *
	 * @return  an array containing one color entry per entry in the custom_var's
	 *          color palette.  The first entry in the returned array corresponds to
	 *          a palette index of 0.
	 */
	public color[] getAllColors()
		{
			return base_class.getPalcolorCustomVarColors(getObjId(), getVarName());
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
			return "[palcolor_custom_var: ranged_int_custom_var = " + super.toString() + "]";
		}

	/**
	 * Construct a palcolor_custom_var instance.
	 *
	 * Scripters should not try to construct these by hand.  These
	 * are created from the C++ native code.
	 *
	 * @param objId              the obj_id for the Object containing the variable.
	 * @param varName            the name of the customization variable.
	 * @param minRangeInclusive  the minimum value that this variable's value can take on.
	 * @param maxRangeExclusive  one greater than the maximum value this variable can take on.
	 */
	public palcolor_custom_var(long objId, String varName, int varTypeId, int minRangeInclusive, int maxRangeExclusive)
		{
			super(objId, varName, varTypeId, minRangeInclusive, maxRangeExclusive);
		}
	public palcolor_custom_var(obj_id objId, String varName, int varTypeId, int minRangeInclusive, int maxRangeExclusive)
		{
			super(objId, varName, varTypeId, minRangeInclusive, maxRangeExclusive);
		}
}
