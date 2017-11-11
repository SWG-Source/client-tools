// ======================================================================
//
// FileLocator.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstUiBuilder.h"
#include "FileLocator.h"

#include <stdio.h>

// ======================================================================
FileLocator *   FileLocator::ms_gFileLocator = 0;

//-----------------------------------------------------------------

FileLocator::FileLocator () :
m_paths ()
{
}

//-----------------------------------------------------------------

const bool  FileLocator::findFile (const char * filename, UINarrowString & result)
{
	for (PathVector_t::iterator iter = m_paths.begin (); iter != m_paths.end (); ++iter)
	{
		UINarrowString pathToCheck = *iter + "/" + filename;

		FILE * fl = fopen (pathToCheck.c_str (), "r");

		if (fl == 0)
			continue;

		fclose (fl);

		result = pathToCheck;
		return true;
	}

	return false;
}
//-----------------------------------------------------------------
void  FileLocator::addPath (UINarrowString path)
{
	m_paths.push_back (path);
}
//-----------------------------------------------------------------
void FileLocator::ExplicitDestroy ()
{
	delete ms_gFileLocator;
	ms_gFileLocator = 0;
}

// ======================================================================
