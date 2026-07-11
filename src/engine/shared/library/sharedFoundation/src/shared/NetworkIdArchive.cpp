// ======================================================================
//
// NetworkIdArchive.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include "Archive/Archive.h"   // for the inline int64/uint64 get/put overloads
#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

namespace Archive
{
	// uint64 / int64 get/put overloads were moved to Archive/Archive.h as inline
	// `unsigned long long` / `signed long long` so they're visible everywhere
	// AutoDeltaMap/Set/Vector serialization needs them on x64 (where size_t is
	// also `unsigned __int64`). They lived here only because the original 32-bit
	// build never needed them visible from inside the archive headers.

	//-----------------------------------------------------------------------

	void get(ReadIterator & source, NetworkId & target)
	{
		int64 id;         // Using int64 instead of NetworkIdType so that we get a compile error if we change it
		get(source, id);
		target = NetworkId(id);
	}

	//-----------------------------------------------------------------------

	void put(ByteStream & target, const NetworkId & source)
	{
		put(target, source.getValue());
	}

	//-----------------------------------------------------------------------
}

// ======================================================================
