//======================================================================
//
// NebulaLightningData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "NebulaLightningArchive.h"
#include "NebulaLightningData.h"

#include "Archive/Archive.h"
#include "sharedMathArchive/VectorArchive.h"

//======================================================================

namespace Archive
{
	void get(ReadIterator& source, NebulaLightningData& target)
	{
		Archive::get(source, target.lightningId);
		Archive::get(source, target.nebulaId);
		Archive::get(source, target.syncStampStart);
		Archive::get(source, target.syncStampEnd);
		Archive::get(source, target.endpoint0);
		Archive::get(source, target.endpoint1);
	}

	//----------------------------------------------------------------------

	void put(ByteStream& target, NebulaLightningData const& source)
	{
		Archive::put(target, source.lightningId);
		Archive::put(target, source.nebulaId);
		Archive::put(target, source.syncStampStart);
		Archive::put(target, source.syncStampEnd);
		Archive::put(target, source.endpoint0);
		Archive::put(target, source.endpoint1);
	}
}
//======================================================================
