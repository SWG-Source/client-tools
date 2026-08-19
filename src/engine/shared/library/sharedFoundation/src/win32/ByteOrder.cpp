// ======================================================================
//
// ByteOrder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ByteOrder.h"

#include <intrin.h> // _byteswap_ulong, _byteswap_ushort

// ======================================================================
//
// Was: x86 inline asm using `__declspec(naked)` + bswap. Both are
// x86-only in MSVC. Replace with `_byteswap_*` intrinsics, which the
// compiler lowers to a single bswap on both x86 and x64.

ulong ntohl(ulong netLong)
{
	return static_cast<ulong>(_byteswap_ulong(static_cast<unsigned long>(netLong)));
}

ulong htonl(ulong hostLong)
{
	return static_cast<ulong>(_byteswap_ulong(static_cast<unsigned long>(hostLong)));
}

ushort ntohs(ushort netShort)
{
	return static_cast<ushort>(_byteswap_ushort(static_cast<unsigned short>(netShort)));
}

ushort htons(ushort hostShort)
{
	return static_cast<ushort>(_byteswap_ushort(static_cast<unsigned short>(hostShort)));
}

// ======================================================================
