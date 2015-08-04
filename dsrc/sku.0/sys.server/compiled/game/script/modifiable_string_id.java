/**
 * Title:        modifiable_string_id
 * Description:  Modifiable extension of a string_id.
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

public class modifiable_string_id extends string_id
{
	/**
	 * Class constructor.
	 *
	 * @param table		the string table
	 * @param id		the string table id
	 */
	public modifiable_string_id(String table, String id)
	{
		super(table, id);
	}	// modifiable_string_id(String, String)

	/**
	 * Class constructor.
	 *
	 * @param table		the string table
	 * @param id		the string table id
	 */
	public modifiable_string_id(String table, int id)
	{
		super(table, id);
	}	// modifiable_string_id(String, int)

	/**
	 * Copy constructor.
	 *
	 * @param src		class instance to copy
	 */
	public modifiable_string_id(string_id src)
	{
		super(src);
	}	// modifiable_string_id(string_id)

	/**
	 * Sets the table for the string id.
	 *
	 * @param table		the table name
	 */
	public void setTable(String table)
	{
		m_table = table;
	}	// setTable

	/**
	 * Sets the id for the string id.
	 *
	 * @param id		the name of the id
	 */
	public void setId(String id)
	{
		m_asciiId = id;
		m_indexId = -1;
	}	// setId(String)

	/**
	 * Sets the id for the string id.
	 *
	 * @param id		the index of the id
	 */
	public void setId(int id)
	{
		m_indexId = id;
		m_asciiId = "";
	}	// setId(int)

}	// class modifiable_string_id
