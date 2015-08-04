//======================================================================
//
// CuiObjectDataSourceCallback.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiObjectDataSourceCallback_H
#define INCLUDED_CuiObjectDataSourceCallback_H

class Object;

//======================================================================

class CuiObjectDataSourceCallback
{
public:
	virtual void addObject    (Object & obj) = 0;
	virtual void removeObject (Object & obj) = 0;
	virtual ~CuiObjectDataSourceCallback () = 0 {};
};

//======================================================================

#endif
