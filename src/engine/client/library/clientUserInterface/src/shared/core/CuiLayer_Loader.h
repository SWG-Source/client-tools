
//======================================================================
//
// CuiLayer_Loader.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLayer_Loader_H
#define INCLUDED_CuiLayer_Loader_H

//======================================================================

#include "clientUserInterface/CuiLayer.h"
#include "UILoader.h"

//-------------------------------------------------------------------
//
// SkelLoader
//


class CuiLayer::Loader : public UILoader
{
public:
	
	Loader ();
	virtual bool            LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out );

	static void             install ();
	static void             remove  ();
};


//======================================================================

#endif
