//======================================================================
//
// CuiAliasHandler.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiAliasHandler_H
#define INCLUDED_CuiAliasHandler_H

//======================================================================

class CuiAliasHandler
{
public:
	virtual int handleAlias (const Unicode::String & str, Unicode::String & result) = 0;
	virtual ~CuiAliasHandler () = 0 {}
};


//======================================================================

#endif
