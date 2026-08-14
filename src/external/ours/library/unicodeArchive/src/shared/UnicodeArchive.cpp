//======================================================================
//
// UnicodeArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "unicodeArchive/FirstUnicodeArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "Archive/Archive.h"

//======================================================================

namespace Archive
{
	//-----------------------------------------------------------------------
	void get(ReadIterator & source, Unicode::String & target)
	{
		unsigned int size = 0;
		Archive::get (source, size);

		// Sanity-bail: claimed size (in unicode chars) times 2 bytes-per-char
		// must not exceed remaining buffer. Without this, target.assign(ubuf,
		// ubuf+size) reads past valid memory and crashes inside
		// std::wstring::_M_mutate.
		const unsigned int readSize = size * sizeof(Unicode::unicode_char_t);
		if (readSize > source.getSize())
		{
			throw ReadException("Unicode::String get: size exceeds remaining buffer");
		}

		const unsigned char * const buf            = source.getBuffer();
		const Unicode::unicode_char_t * const ubuf = reinterpret_cast<const Unicode::unicode_char_t *>(buf);

		target.assign (ubuf, ubuf + size);

		source.advance(readSize);
	}
	
	//-----------------------------------------------------------------------
	
	void put(ByteStream & target, const Unicode::String & source)
	{
		const unsigned int size = source.size ();
		Archive::put (target, size);
		target.put (source.data(), size * sizeof (Unicode::unicode_char_t));
	}
}
	
//======================================================================
