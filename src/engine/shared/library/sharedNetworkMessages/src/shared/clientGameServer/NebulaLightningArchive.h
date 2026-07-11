//======================================================================
//
// NebulaLightningArchive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaLightningArchive_H
#define INCLUDED_NebulaLightningArchive_H

//======================================================================
class NebulaLightningData;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator& source, NebulaLightningData& target);
	void put(ByteStream& target, NebulaLightningData const& source);
}

//======================================================================

#endif
