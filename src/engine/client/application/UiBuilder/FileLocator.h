// ======================================================================
//
// FileLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FileLocator_H
#define INCLUDED_FileLocator_H

#include "UIString.h"
#include <vector>

// ======================================================================
class FileLocator
{
public:

	const bool                findFile (const char * filename, UINarrowString & result);

	void                      addPath (UINarrowString path);

	static FileLocator &      gFileLocator ();
	static void               ExplicitDestroy ();

private:
	                          FileLocator ();
	                          FileLocator (const FileLocator & rhs);
	FileLocator &             operator=    (const FileLocator & rhs);

	typedef std::vector <UINarrowString> PathVector_t;

	PathVector_t              m_paths;

	static FileLocator *      ms_gFileLocator;
};

// ======================================================================

inline FileLocator & FileLocator::gFileLocator ()
{
	if (ms_gFileLocator)
		return *ms_gFileLocator;

	return *(ms_gFileLocator = new FileLocator ());
}
//-----------------------------------------------------------------


		
#endif
