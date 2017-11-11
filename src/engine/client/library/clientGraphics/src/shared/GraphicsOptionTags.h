// ======================================================================
//
// GraphicsOptionTags.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GraphicsOptionTags_H
#define INCLUDED_GraphicsOptionTags_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

class GraphicsOptionTags
{
public:

	static void install();

	static void   set(Tag option, bool enabled);
	static bool   get(Tag option);
	static void   toggle(Tag option);
};

// ======================================================================

#endif
