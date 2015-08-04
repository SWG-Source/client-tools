/**
 * Title:        internal_script_exception
 * Description:  exception class to be caught inside our C code
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

class internal_script_exception extends Exception
{
	internal_script_exception()
	{
	}

	internal_script_exception(String message)
	{
		super(message);
	}

}	// class internal_script_exception
