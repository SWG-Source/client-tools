// ======================================================================
//
// Win32Registry.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "Win32Registry.h"

#include "UnicodeUtils.h"

#include <cassert>
#include <cstdlib>
#include <tchar.h>
#include <windows.h>

// ======================================================================

Win32Registry::Win32Registry (const Unicode::NarrowString & root) :
PortableRegistry (),
m_root (root),
m_regKey (0)
{
	HRESULT hr = RegCreateKeyEx( HKEY_CURRENT_USER, Unicode::narrowToWide (root).c_str (), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &m_regKey, 0 );

	assert (hr == ERROR_SUCCESS);
	
	static_cast<void>(hr);
}

//-----------------------------------------------------------------

Win32Registry::~Win32Registry ()
{
}

//-----------------------------------------------------------------

bool Win32Registry::getStringValue (const Unicode::NarrowString & key, Unicode::NarrowString & value)
{
	if (m_regKey == 0)
		return false;

	DWORD lpType;
	DWORD bufSize = 1024;
	BYTE lpData [1024];

	HRESULT hr = RegQueryValueEx (m_regKey, Unicode::narrowToWide (key).c_str (), 0, &lpType, lpData, &bufSize);

	if (hr != ERROR_SUCCESS)
		return false;

	value = Unicode::wideToNarrow (reinterpret_cast<wchar_t *>(lpData));

	return true;

}

//-----------------------------------------------------------------

bool Win32Registry::getIntValue    (const Unicode::NarrowString & key, int & value)
{
	if (m_regKey == 0)
		return false;

	DWORD lpType;
	DWORD bufSize = 1024;
	BYTE lpData [1024];

	HRESULT hr = RegQueryValueEx (m_regKey, Unicode::narrowToWide (key).c_str (), 0, &lpType, lpData, &bufSize);

	if (hr != ERROR_SUCCESS)
		return false;


	if (lpType == REG_DWORD)
	{
		value = *(reinterpret_cast<DWORD *>(lpData));
	}
	else if (lpType == REG_SZ)
	{
		value = _wtoi (reinterpret_cast<wchar_t *>(lpData));
	}

	return true;
}

//-----------------------------------------------------------------

bool Win32Registry::putStringValue (const Unicode::NarrowString & key, const Unicode::NarrowString & value)
{
	HRESULT hr = RegSetValueEx (m_regKey, Unicode::narrowToWide (key).c_str (), 0, REG_SZ, reinterpret_cast<const BYTE *> (Unicode::narrowToWide (value).c_str ()), (value.length () + 1) * sizeof (unsigned short));
	return (hr == ERROR_SUCCESS);
}

//-----------------------------------------------------------------

bool Win32Registry::putIntValue    (const Unicode::NarrowString & key, const int value)
{
	HRESULT hr = RegSetValueEx (m_regKey, Unicode::narrowToWide (key).c_str (), 0, REG_DWORD, reinterpret_cast<const BYTE *> (&value), sizeof (value));
	return (hr == ERROR_SUCCESS);
}

// ======================================================================
