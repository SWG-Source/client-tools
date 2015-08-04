//======================================================================
//
// CuiSetName.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSetName_H
#define INCLUDED_CuiSetName_H

//======================================================================

#include "clientUserInterface/CuiInputBox.h"

//-----------------------------------------------------------------

class CuiSetName : public CuiInputBox
{
public:
	explicit                CuiSetName        (UIPage & page);
	                        ~CuiSetName       ();

	static void             createNewInstance (Unicode::String const & params);

protected:

	virtual void            handleResult      (bool affirmative, Unicode::String const & s);

private:
	//disabled
	                        CuiSetName             (CuiSetName const & rhs);
	                        CuiSetName & operator= (CuiSetName const & rhs);
};

//======================================================================

#endif
