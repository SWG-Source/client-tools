//======================================================================
//
// UIStandardLoader.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIStandardLoader_H
#define INCLUDED_UIStandardLoader_H

//======================================================================

#include "UILoaderExtension.h"

// Standard Object Instanciator Template
// =====================================

template <class T> class UIStandardLoader : public UILoaderExtension
{
public:

	UIBaseObject *          Create      () const { return new T; };
	const char   * const    GetTypeName () const { return T::TypeName; }
};

//======================================================================

#endif
