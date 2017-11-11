macro_file Mocha;
/******************************************************************************
														 Multi-Edit Macro File

	Function: Macros to update command maps, and configuration items

	$Header: /Me9/Src/cmdupd.s 9     8/17/00 3:54p Johnp $

							 Copyright (C) 2000 by American Cybernectics, Inc.
 *****************************************************************************/

#include Mew.sh
#include CmpError.sh
#include Win32.sh
#include Messages.sh
#include Mewlib32.sh
#include Metools.sh
#include Dialog.sh
//#include Logo.sh
#include RegExp.sh
#include StdDlgs.sh

#ifdef _Debug_
  #include MsgLog.sh
#endif

global {
	str g_mochaPath						"Mocha_Path";
	str g_mochaPerlPath 			"Mocha_Perl_Path";
	int g_hasMochaErrors  	  "HasMochaErrors";
}

/**
 * Tries to find the path to the mocha preprocessor.
 */
void FindMochaPath() trans2
{
	g_mochaPath = '';
	g_mochaPerlPath = '';
	g_hasMochaErrors = 0;

	str file_path = get_path(file_name);

	int script_pos = xpos('dsrc', file_path, 1);
	if ( script_pos == 0)
	{
		MessageBox(0, "Cannot find directory 'dsrc' from the source directory", "Multi-Edit", mb_Ok | mb_IconError);
		return();
	}

	str mocha_path = copy(file_path, 1, script_pos - 1);
	mocha_path = str_ins(mocha_path, 'exe\shared\', 1);

	int swg_pos = xpos('swg', file_path, 1);
	str mocha_perl_path = copy(file_path, 1, swg_pos + 3);
	mocha_perl_path = str_ins(mocha_perl_path, 'all\tools\mocha\', 1);

	// see if the preprocessor file exists in the directory
	str file = str_ins(mocha_path, 'script_prep.pyc', 1);
	if ( file_exists(file) != 0)
		g_mochaPath = mocha_path;

	file = str_ins(mocha_perl_path, 'mochac.pl', 1);
	if ( file_exists(file) != 0)
		g_mochaPerlPath = mocha_perl_path;
}  // FindMochaPath

/**
 * Highlights all the errors in the error window.
 */
void MarkMochaErrors( ) Trans2 {
	str javaStr = ReTranslate("^(<p>)[:(](<i>)\\)@\\: #(.#)$", 0);
	str mochaStr = ReTranslate("^[mM]ocha[a-zA-Z ]#(<i>)@: #(.#)$", 0);

	// save where the cursor is
	int line = c_line;
	int col = c_col;

	while ( !at_eof )
	{
		if ( find_text(mochaStr, 0, _RegExp) || find_text(javaStr, 0, _RegExp) )
		{
			line_attr = 16;
		}
		down;
	}

	// restore the cursor
	goto_line(line);
	goto_col(col);
}	// MarkMochaErrors

/**
 * Processes error messages from the Mocha preprocessor or JavaC compiler.
 */
void ErrMocha( ) Trans2 {
	str javaStr = ReTranslate("^(<p>)[:(](<i>)\\)@\\: #(.#)$", 0);
	str mochaStr = ReTranslate("^[mM]ocha[a-zA-Z ]#(<i>)@: #(.#)$", 0);

	int err = rc_NoErrors;
	str errStr = "";

	if ( find_text(mochaStr, 0, _RegExp) )
	{
		if ( line_attr != 16 )
		{
			MarkMochaErrors;
		}
		g_hasMochaErrors = 1;
		err = rc_DisplayError;
		errStr = Get_Line();
		g_LastFndRE = mochaStr;
		g_LastRepRE = "/L=\\0/M=\\1";
//		MessageBox(0, "Mocha error", "Multi-Edit", mb_Ok);
		Rm("CmpError");
	}
	else if ( find_text(javaStr, 0, _RegExp) )
	{
		if ( line_attr != 16 )
		{
			MarkMochaErrors;
		}

		// Fix to correctly set replace group data
    Find_Text( javaStr, 1, _RegExp );
		// Translate / to 0xFE character in RepStr
		int jx = 1;
		str repStr = "/F=\\0/L=\\3/M=\\4";
		while ( jx = XPos( "/", repStr, Jx ) ) {
			repStr = Str_Ins( "\xFE", str_del( repStr, jx, 1 ), jx );
		}
		Replace( RepStr );
    str tStr = TranslateCmdLine( Get_Line( ), '' );
		Undo;
    str fName = Parse_Str( "\xFE" + "F=", tStr );
    int line  = Parse_Int( "\xFE" + "L=", tStr );
    str msg   = Parse_Str( "\xFE" + "M=", tStr );

		g_hasMochaErrors = 1;
		err = rc_DisplayError;
		errStr = msg;

		// find a line with a single '^' - that is the column
		int col = 1;
		if (find_text("^[ \\t]@\\^", 0, _RegExp))
		{
			forward_till("^");
			col = c_col;
		}

		// put the cursor at the end of the error message
		find_text(javaStr, 0, _RegExp | _Backward);
		eol;

		Rm( "ErrFindError /F=" + fName + "/L=" + Str( line ) + "/C=" + Str( col ) );
	}
	else if ( g_hasMochaErrors == 1)
	{
		err = rc_NoMoreErrors;
	}
	else
		err = rc_NoErrors;

	Return_Int = err;
	Return_Str = errStr;
}  // ErrMocha


