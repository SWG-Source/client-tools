/**
* Data element for menu array
*/

package script;

import java.io.Serializable;

final public class menu_info_data implements Serializable
{
	private final static long serialVersionUID = -3689934288561342691L;

	private int       m_id       =  0;
	private int       m_parent   =  0;
	private int       m_type     =  0;
	private string_id m_label    =  null;
	private boolean   m_enabled      = true;
	private boolean   m_serverNotify = false;

	//-----------------------------------------------------------------------------

	/**
	*
	*/

	menu_info_data ()
	{
		m_label = new string_id ("", 0);
	}

	//-----------------------------------------------------------------------------

	/**
	* The constructor, yo.
	*/

	menu_info_data (int id, int parent, int type, string_id stringId, boolean enabled, boolean serverNotify)
	{
		m_id       = id;
		m_parent   = parent;
		m_type     = type;
		m_enabled  = enabled;
		m_serverNotify = serverNotify;

		if (stringId != null)
			m_label = stringId;
		else
			m_label = new string_id ();
	}

	//-----------------------------------------------------------------------------

	public String toString ()
	{
		return "{menu_info_data id=" + m_id + ", parent=" + m_parent + ", type=" + m_type + ", label=" + m_label + ", enabled=" + m_enabled + ", servernotify=" + m_serverNotify + "}";
	}

	//-----------------------------------------------------------------------------

	/**
	* get the item's id number
	*/

	public int getId ()
	{
		return m_id;
	}

	//-----------------------------------------------------------------------------

	/**
	* get the item's parent id number.  0 indicates no parent
	*/

	public int getParentId ()
	{
		return m_parent;
	}

	//-----------------------------------------------------------------------------

	/**
	* get the item's canonical name.  This is used to identify the menu selection for processing.
	*/

	public int getType ()
	{
		return m_type;
	}

	//-----------------------------------------------------------------------------

	/**
	* get the item's string id for label display.  This is localized at the last moment before the server sends the data to the client,
	* and if it fails, the client attempts to localize the string
	*/

	public string_id getLabel ()
	{
		return m_label;
	}

	//-----------------------------------------------------------------------------

	/**
	* Sets the label of a menu item
	*
	*/

	public void setLabel (string_id label)
	{
		if (label != null)
			m_label = label;
		else
			m_label = new string_id ();
	}

	//-----------------------------------------------------------------------------

	/**
	* is the menu item enabled?  Enabled means that the item is in rane.
	*/

	public boolean getEnabled ()
	{
		return m_enabled;
	}

	//-----------------------------------------------------------------------------

	/**
	* Set the enabled status of the menu item.  Use with care.
	*/

	public void setEnabled (boolean b)
	{
		m_enabled = b;
	}

	//-----------------------------------------------------------------------------

	/**
	* The menu item cause a server callback to OnObjectMenuSelected ()
	*/

	public boolean getServerNotify ()
	{
		return m_serverNotify;
	}

	//-----------------------------------------------------------------------------

	/**
	* Set the serverNotify status of the menu item.
	*/

	public void setServerNotify (boolean b)
	{
		m_serverNotify = b;
	}
}
