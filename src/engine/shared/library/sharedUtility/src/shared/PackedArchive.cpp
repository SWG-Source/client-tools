// ======================================================================
//
// PackedArchive.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

// ======================================================================

#include "PackedArchive.h"
#include "sharedUtility/PackedTransform.h"

// ======================================================================

namespace Archive
{
	// ----------------------------------------------------------------------

	void get(ReadIterator &source, PackedTransform &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	void put(ByteStream &target, PackedTransform const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

} // namespace Archive
