//======================================================================
//
// CuiConsoleHelperOutputGenerated.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiConsoleHelperOutputGenerated_H
#define INCLUDED_CuiConsoleHelperOutputGenerated_H

//======================================================================

class CuiConsoleHelper;

//----------------------------------------------------------------------


class CuiConsoleHelperOutputGenerated
{
public:
	const CuiConsoleHelper & helper;
	const Unicode::String &  str;
	mutable bool             processed;
	
	CuiConsoleHelperOutputGenerated (const CuiConsoleHelper & _helper, const Unicode::String & _str);
	
private:
	CuiConsoleHelperOutputGenerated (const CuiConsoleHelperOutputGenerated & rhs);
	CuiConsoleHelperOutputGenerated & operator= (const CuiConsoleHelperOutputGenerated & rhs);
};

//======================================================================

#endif
