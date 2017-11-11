//======================================================================
//
// UILoaderExtension.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UILoaderExtension_H
#define INCLUDED_UILoaderExtension_H

//======================================================================

// Abstract Object Instanciator
// =====================================

class UILoaderExtension
{
public:

	virtual UIBaseObject *  Create            () const = 0;
	virtual                ~UILoaderExtension () = 0 {};
};

//======================================================================

#endif
