// ======================================================================
//
// RecentDirectory.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "RecentDirectory.h"

#include <assert.h>

// ======================================================================

HKEY RecentDirectory::registryKey;
char RecentDirectory::buffer[_MAX_PATH];

// ======================================================================

void RecentDirectory::install(const char *path)
{
	const LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &registryKey, NULL);
	UNREF(result); //for release
	assert(result == ERROR_SUCCESS);	
}

// ----------------------------------------------------------------------

void RecentDirectory::remove()
{
	RegCloseKey(registryKey);
}

// ----------------------------------------------------------------------

const char *RecentDirectory::find(const char *type)
{
	DWORD dword = sizeof(buffer);
	DWORD dataType = 0;
	const LONG result = RegQueryValueEx(registryKey, type, NULL, &dataType, reinterpret_cast<BYTE*>(buffer), &dword);
	if (result == ERROR_SUCCESS && dataType == REG_SZ)
		return buffer;

	return NULL;
}

// ----------------------------------------------------------------------

bool RecentDirectory::update(const char *type, const char *fileName)
{
	// chop off the file name
	strncpy(buffer, fileName, sizeof(buffer));
	buffer[sizeof(buffer)-1] = '\0';
	char *slash = strrchr(buffer, '\\');
	if (slash)
	{
		*slash = '\0';
	}
	else
	{
		slash = strrchr(buffer,'/');
		if(slash)
			*slash = '\0';
	}


	const LONG result = RegSetValueEx(registryKey, type, 0, REG_SZ, reinterpret_cast<const BYTE*>(buffer), strlen(buffer)+1);
	return (result == ERROR_SUCCESS);
}

// ======================================================================
