// ======================================================================
//
// TreeFileExtractor.h
// Copyright 2002 Sony Online Entertainment Inc
//
// ======================================================================

#ifndef INCLUDED_TreeFileExtractor_H
#define INCLUDED_TreeFileExtractor_H

// ======================================================================

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include <vector>
#include <string>

// ======================================================================

class Compressor;

// ======================================================================

class TreeFileExtractor
{
private:

	TreeFileExtractor();
	TreeFileExtractor(const TreeFileExtractor&);
	TreeFileExtractor &operator =(const TreeFileExtractor&);

public:

	static void run();
};

// ======================================================================

#endif
