// ======================================================================
//
// NetworkIdArchive.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_NetworkIdArchive_H
#define	INCLUDED_NetworkIdArchive_H

#include "sharedFoundationTypes/FoundationTypes.h"

// ======================================================================

class NetworkId;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	// uint64 / int64 overloads now live in Archive/Archive.h (inline) so they're
	// reachable from header-only template code (AutoDeltaMap, etc.). Only the
	// NetworkId helpers remain here.
	void get(ReadIterator & source, NetworkId & target);
	void put(ByteStream & target, const NetworkId & source);
}

// ======================================================================

#endif	
