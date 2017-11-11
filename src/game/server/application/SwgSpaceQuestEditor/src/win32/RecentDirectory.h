// ======================================================================
//
// RecentDirectory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RecentDirectory_H
#define INCLUDED_RecentDirectory_H

// ======================================================================

class RecentDirectory
{
public:
	static void install(const char *registryKey);
	static void remove();

	static const char *find(const char *type);
	static bool        update(const char *type, const char *path);

private:

	static HKEY registryKey;
	static char buffer[_MAX_PATH];
};

// ======================================================================

#endif
