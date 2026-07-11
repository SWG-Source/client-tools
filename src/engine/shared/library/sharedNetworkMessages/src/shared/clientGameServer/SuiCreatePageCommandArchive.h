// ======================================================================
//
// SuiCreatePageCommandArchive.h
// Archive helpers for SuiCreatePage_Command
//
// ======================================================================

#ifndef INCLUDED_SuiCreatePageCommandArchive_H
#define INCLUDED_SuiCreatePageCommandArchive_H

#include "Archive/Archive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "SuiCreatePageCommand.h"

// Tell Archive how to pack/unpack SuiCreatePage_Command and its enum Type
namespace Archive
{
	// Treat enum Type like an int
	inline void put(ByteStream& target, const SuiCreatePage_Command::Type& source)
	{
		put(target, static_cast<int>(source));
	}

	inline void get(ReadIterator& source, SuiCreatePage_Command::Type& target)
	{
		signed int tmp;
		get(source, tmp);
		target = static_cast<SuiCreatePage_Command::Type>(tmp);
	}

	// Pack/unpack the struct fields
	inline void put(ByteStream& target, const SuiCreatePage_Command& source)
	{
		put(target, source.m_type);
		put(target, source.m_parameters);
	}

	inline void get(ReadIterator& source, SuiCreatePage_Command& target)
	{
		get(source, target.m_type);
		get(source, target.m_parameters);
	}
}

#endif