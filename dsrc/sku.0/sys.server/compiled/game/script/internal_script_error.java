/**
 * Title:        internal_script_error
 * Description:  error class to be caught inside our C code
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

class internal_script_error extends Error
{
	private Error wrappedError = null;

	internal_script_error()
	{
	}

	internal_script_error(String message)
	{
		super(message);
	}

	internal_script_error(Error err)
	{
		wrappedError = err;
	}
}	// class internal_script_error
