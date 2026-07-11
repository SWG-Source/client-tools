// ======================================================================
//
// NetworkIdAutoDeltaPackedMap.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NetworkIdAutoDeltaPackedMap_H
#define INCLUDED_NetworkIdAutoDeltaPackedMap_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "Archive/AutoDeltaPackedMap.h"

// ======================================================================

namespace Archive
{
	template <> void AutoDeltaPackedMap<int, NetworkId>::pack(ByteStream & target, const std::string & buffer);
	template <> void AutoDeltaPackedMap<int, NetworkId>::unpack(ReadIterator & source, std::string & buffer);
}

// ======================================================================

#endif
