// ======================================================================
//
// PackedTransform.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedArchive_H
#define INCLUDED_PackedArchive_H

// ======================================================================

class PackedTransform;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	// ----------------------------------------------------------------------

	void get(ReadIterator& source, PackedTransform& target);
	void put(ByteStream& target, PackedTransform const& source);

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedArchive_H
