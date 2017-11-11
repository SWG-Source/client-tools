//======================================================================
//
// CuiStringVariablesData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringVariablesData_H
#define INCLUDED_CuiStringVariablesData_H

//======================================================================

class ClientObject;

//----------------------------------------------------------------------

class CuiStringVariablesData
{
public:
	const ClientObject * source;
	const ClientObject * target;
	const ClientObject * other;
	int                  digit_i;
	float                digit_f;

	Unicode::String      sourceName;
	Unicode::String      targetName;
	Unicode::String      otherName;

	CuiStringVariablesData ();
};

//======================================================================

#endif
