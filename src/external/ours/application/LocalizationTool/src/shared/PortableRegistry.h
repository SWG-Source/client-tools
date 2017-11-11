// ======================================================================
//
// PortableRegistry.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PortableRegistry_H
#define INCLUDED_PortableRegistry_H

#include "Unicode.h"

// ======================================================================
class PortableRegistry
{
public:

	PortableRegistry () {}

	virtual bool                   getStringValue (const Unicode::NarrowString & key, Unicode::NarrowString & value) = 0;
	virtual bool                   getIntValue    (const Unicode::NarrowString & key, int & value) = 0;

	virtual bool                   putStringValue (const Unicode::NarrowString & key, const Unicode::NarrowString & value) = 0;
	virtual bool                   putIntValue    (const Unicode::NarrowString & key, const int value) = 0;

	virtual ~PortableRegistry () = 0;

private:
	PortableRegistry (const PortableRegistry & rhs);
	PortableRegistry & operator= (const PortableRegistry & rhs);
};

// ======================================================================

#endif
