/**
* menu info for constructing a client-side radial menu
*/

package script;

import java.util.Vector;
import java.util.Collection;
import java.util.Iterator;
import java.io.Serializable;

final public class menu_info implements Serializable
{
	private final static long serialVersionUID = -3334262904982229749L;

	Vector  m_vector  = new Vector  ();
	private int m_nextId = 1;

	/**
	 * Class constructor.
	 *
	 * @param type		attribute type
	 * @param value		attribute value
	 */
	public menu_info()
	{
	}

	//-----------------------------------------------------------------------------

	/**
	  * Copy constructor.
	  *
	  * @param src		class instance to copy
	  */
	public menu_info(menu_info src)
	{
		m_vector.clear   ();
		m_vector.add     (src.m_vector);
		m_nextId = src.m_nextId;
	}

	//-----------------------------------------------------------------------------

	/**
	* Add a root menu item to the menu.  It is an error to add 2 menus with the
	* same type to the same parent menu.
	*
	* @param name the canonical name of this menu.  This string is used to indicate the appropriate processing when the menu item is selected.
	* @param label the label to display on this menu
	*
	* @return zero on failure, a valid menu id otherwise
	*/
	public int addRootMenu (int type, string_id label)
	{
		return addMenuItemInternal (0, type, label);
	}

	//-----------------------------------------------------------------------------

	/**
	* Add the specified menu to the specified parent.  Menus can only have
	* 2 levels of heirarchy.  It is an error to add a submenu to another submenu.
	* Submenus must be added to root menus only.
	*
	* @return zero on failure, a valid menu id otherwise.
	*/

	public int addSubMenu (int parentId, int type, string_id label)
	{
		final menu_info_data parentData = getMenuItemById (parentId);

		if (parentData == null)
			return 0;

		return addMenuItemInternal (parentId, type, label);
	}

	//-----------------------------------------------------------------------------

	/**
	* Obtain a newly created array of menu items.  This is an expensive routine.
	* @return a new menu_info_data[]
	*/

	public menu_info_data[] getRootMenuItems ()
	{
		return getSubMenuItems (0);
	}

	//-----------------------------------------------------------------------------

	/**
	* Obtain a newly created array of menu items.  This is an expensive routine.
	* @return a new menu_info_data[]
	*/

	public menu_info_data[] getSubMenuItems (int parent)
	{
		final Vector vector = new Vector ();

		for (Iterator it = m_vector.iterator (); it.hasNext (); )
		{
			final menu_info_data data = (menu_info_data) it.next ();

			if (data.getParentId () == parent)
				vector.add (data);
		}

		final Object[] array = vector.toArray ();
		final menu_info_data[] blob = new menu_info_data [array.length];
		java.lang.System.arraycopy (array, 0, blob, 0, array.length);
		return blob;
	}

	//-----------------------------------------------------------------------------

	/*
	* Obtain a menu_info_data by id number.  This is a linearly fast routine.
	* @return null if the id did not specify a valid menu item
	*/

	public menu_info_data getMenuItemById (int id)
	{
		for (Iterator it = m_vector.iterator (); it.hasNext (); )
		{
			final menu_info_data data = (menu_info_data) it.next ();
			if (data.getId () == id)
				return data;
		}

		return null;
	}

	//-----------------------------------------------------------------------------

	/*
	* Obtain a menu_info_data by type.  This is a moderately fast routine.
	* If multiple menu items exist with the same type, the return value is
	* the one with the lesser numerical id.
	*/

	public menu_info_data getMenuItemByType (int type)
	{
		for (Iterator it = m_vector.iterator (); it.hasNext (); )
		{
			final menu_info_data data = (menu_info_data) it.next ();
			if (data.getType () == type)
				return data;
		}

		return null;
	}

	//-----------------------------------------------------------------------------

	/*
	* Obtain a menu_info_data by type.  This is a moderately fast routine.
	* If multiple menu items exist with the same type, the return value is
	* the one with the lesser numerical id.
	*
	* @param name the type to search for
	* @param parentId the menu item whose childs to search threw. zero indicates to search the roote items onley.  thanx.
	* @return null if the name did not specify a valid menu item
	*/

	public menu_info_data getMenuItem (int type, int parentId)
	{
		for (Iterator it = m_vector.iterator (); it.hasNext (); )
		{
			final menu_info_data data = (menu_info_data) it.next ();

			if (data.getParentId () != parentId)
				continue;

			if (data.getType () == type)
				return data;
		}

		return null;
	}

	//-----------------------------------------------------------------------------

	/**
	*
	*/

	public menu_info_data addRootMenuOrServerNotify (int type, string_id label)
	{
		menu_info_data data = getMenuItemByType (type);
		if (data != null)
		{
			data.setServerNotify (true);
			data.setLabel        (label);
		}
		else
		{
			int id = addRootMenu (type, label);
			if (id > 0)
				data = getMenuItemById (id);
		}

		return data;
	}

	//-----------------------------------------------------------------------------

	private void debugDump ()
	{
		StringBuffer sbuf = new StringBuffer ();

		int index = 0;

		for (Iterator it = m_vector.iterator (); it.hasNext (); ++index)
		{
			final menu_info_data data = (menu_info_data) it.next ();
			sbuf.append ("[" + index + "]");
			sbuf.append (data.toString ());
			sbuf.append ("\n");
		}

		System.out.println (sbuf.toString ());
	}

	//-----------------------------------------------------------------------------

	private boolean validateData (menu_info_data data, boolean forInsert)
	{
		//-- negative numbers are not valid
		if (data.getParentId () < 0)
		{
			System.out.println ("Attempt to set a menu item {" + data.toString () + "} with an invalid parent.");
			debugDump ();
			return false;
		}

		//-- attempt to insert duplicate id keys!
		if (forInsert && getMenuItemById (data.getId ()) != null)
		{
			System.out.println ("Attempt to insert duplicate menu id keys {" + data.toString () + "}");
			debugDump ();
			return false;
		}

		if (data.getType() < 0 || data.getType() >= 65535)
		{
			System.out.println ("Menu type out of range: {" + data.toString () + "}");
			return false;
		}

		//-- root items require no further validation

		if (data.getParentId () == 0)
			return true;

		if (forInsert && getMenuItemById (data.getParentId ()) == null)
		{
			System.out.println ("Menu inserted submenu with no valid parent. {" + data.toString () + "}");
			debugDump ();
			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

	/*
	*
	*
	*/

	private boolean setMenuItemsInternal (menu_info_data[] info_data)
	{
		m_vector.clear ();
		m_nextId = 1;

		for (int i = 0; i < info_data.length; ++i)
		{
			final menu_info_data data = info_data [i];

			if (!validateData (data, true))
			{
				System.out.println ("setMenuItemsInternal failed for " + data.toString ());
				m_vector.clear ();
				m_nextId = 1;
				return false;
			}

			m_vector.add (data);
			m_nextId = java.lang.Math.max (m_nextId, data.getId () + 1);
		}

		return true;
	}


	//-----------------------------------------------------------------------------

	/**
	*
	*/

	private int addMenuItemInternal (int parentId, int type, string_id label)
	{
		menu_info_data data = new menu_info_data (m_nextId, parentId, type, label, true, true);

		if (!validateData (data, true))
		{
			System.out.println ("addMenuItemInternal failed for parentId=" + parentId + ", type=" + type + ", label=" + label);
			return 0;
		}

		m_vector.add (data);
		return m_nextId++;
	}


	//-----------------------------------------------------------------------------


	/**
	*
	*
	*/

	private Object[] getMenuItemsInternal ()
	{
		return m_vector.toArray ();
	}
}
