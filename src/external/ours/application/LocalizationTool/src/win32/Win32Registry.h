// ======================================================================
//
// Win32Registry.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Win32Registry_H
#define INCLUDED_Win32Registry_H

#include "PortableRegistry.h"
#include <windows.h>

// ======================================================================
class Win32Registry : public PortableRegistry
{
public:

	Win32Registry (const Unicode::NarrowString & root);

	bool         getStringValue (const Unicode::NarrowString & key, Unicode::NarrowString & value);
	bool         getIntValue    (const Unicode::NarrowString & key, int & value);

	bool         putStringValue (const Unicode::NarrowString & key, const Unicode::NarrowString & value);
	bool         putIntValue    (const Unicode::NarrowString & key, const int value);

	~Win32Registry ();

private:
	Win32Registry (const Win32Registry & rhs);
	Win32Registry & operator= (const Win32Registry & rhs);

	Unicode::NarrowString        m_root;

	HKEY                         m_regKey;
};

// ======================================================================

#endif
