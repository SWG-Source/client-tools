#ifndef __DPVSBITMATH_HPP
#define __DPVSBITMATH_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Bit manipulation routines for 32-bit integers
 *
 * $Archive: /dpvs/implementation/include/dpvsBitMath.hpp $
 * $Author: wili $ 
 * $Revision: #2 $
 * $Modtime: 13.02.02 12:56 $
 * $Date: 2003/02/21 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#include <cstring>

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::BitVector
 *
 * Description:		Class for representing a variable-size bit vector and
 *					performing fast operations on the bit data
 *
 ******************************************************************************/

class BitVector
{
private:
	UINT32*		m_array;		// data in UINT32 format
	size_t		m_dwords;		// number of dwords
public:
DPVS_FORCE_INLINE				BitVector	(size_t size=0)			: m_array(0),m_dwords(0) 	{  m_dwords = (size+31)/32; m_array = NEW_ARRAY<UINT32>(m_dwords);	}
DPVS_FORCE_INLINE				BitVector	(const BitVector& s)	: m_array(0),m_dwords(0)	{  *this = s;	}
DPVS_FORCE_INLINE				~BitVector	(void)					{ DELETE_ARRAY(m_array);																	}
DPVS_FORCE_INLINE	void		reset		(size_t size)			{ size = ((size_t)(size+31)/32); if (size > m_dwords) { DELETE_ARRAY(m_array); m_array = NEW_ARRAY<UINT32>(size); } m_dwords = size; }
					BitVector&	operator=	(const BitVector& s);
DPVS_FORCE_INLINE	void		clearAll	(void)					{ memset (m_array,0,m_dwords*sizeof(UINT32));										}
DPVS_FORCE_INLINE	void		setAll		(void)					{ memset (m_array,0xFF,m_dwords*sizeof(UINT32));									}
DPVS_FORCE_INLINE	bool		test		(size_t  bit) const		{ return (m_array[bit>>5] >> ((UINT32)(bit)&31))&1;}
DPVS_FORCE_INLINE	void		set			(size_t  bit)			{ size_t offs = bit>>5; DPVS_ASSERT (offs < m_dwords); m_array[offs] |= (UINT32)(1<<((UINT32)(bit)&31));		}
DPVS_FORCE_INLINE	void		clear		(size_t  bit)			{ size_t offs = bit>>5; DPVS_ASSERT (offs < m_dwords); m_array[offs] &= ~(UINT32)(1<<((UINT32)(bit)&31));		}
DPVS_FORCE_INLINE	void		eor			(size_t  bit)			{ size_t offs = bit>>5; DPVS_ASSERT (offs < m_dwords); m_array[offs] ^= (UINT32)(1<<((UINT32)(bit)&31));		} // XOR
};

// returns true if two memory blocks are equal, false otherwise
inline bool memEqual (const void* s0, const void* s1, size_t bytes)
{
	DPVS_ASSERT(s0 && s1);
	const UINT32* is0 = (const UINT32*)s0;
	const UINT32* is1 = (const UINT32*)s1;
	size_t dwords = bytes>>2;												// div by four
	size_t i;
	for (i = 0; i < dwords; i++)										// compare as dwords
	if (*is0++ != *is1++)
		return false;
	bytes&=3;															// last 0-3 bytes
	const unsigned char* bs0 = (const unsigned char*)is0;
	const unsigned char* bs1 = (const unsigned char*)is1;
	for (i = 0; i < bytes; i++)
	if (*bs0++ != *bs1++)
		return false;
	return true;														// blocks are equal
}
// memset() but checks the parameters in debug build..
DPVS_FORCE_INLINE void fillByte  (void* dest, unsigned char value, size_t bytes)
{
	DPVS_ASSERT(dest);
	if (bytes > 0)
		::memset (dest,value,bytes);									// system library memset
}

// N = number of dwords (not bytes!)
inline void fillDWord (UINT32* d, UINT32 pattern, size_t N)
{
// MMX version isn't any faster... (so that's why we don't
// have one here).. but we still _do_ have an assembly
// version because MSVC's memset() routine does *not* perform
// quadword alignment automatically (and can thus run 50%
// slower than our variant!)...

#if defined (DPVS_X86_ASSEMBLY)
	DPVS_ASM
	{
		push	edi
		mov		ecx,N
		cmp		ecx,0				// N <= 0
		jle		done			

		mov		edi,d
		mov		eax,pattern
		test	edi,7
		jz		skipPad				// aligned properly - so no padding needed
		mov		[edi],eax
		add		edi,4				// advance pointer
		dec		ecx					// decrease counter
		jz		done
		align 16
skipPad:
		rep		stosd				// qword aligned repeated store
done:
		pop		edi
	}
#elif defined (DPVS_CPU_GAMECUBE) && defined (DPVS_BUILD_CW)
		while(N > 0)
		{
			*d++ = pattern;
			N--;
		}
#else
		if (N<=0)
			return;

		size_t blocks = (N>>3);
		while (blocks)
		{
			d[0] = pattern;
			d[1] = pattern;
			d[2] = pattern;
			d[3] = pattern;
			d[4] = pattern;
			d[5] = pattern;
			d[6] = pattern;
			d[7] = pattern;
			d+=8;
			blocks--;
		}

		N&=7;
		while (N)
		{
			*d++ = pattern;
			N--;
		}
#endif
}

#if !defined(DPVS_X86_ASSEMBLY) && (!defined (DPVS_CPU_GAMECUBE) || !defined (DPVS_BUILD_CW))
namespace BitMath
{
	extern "C" const signed char s_highestLUT[256];
}
#endif // DPVS_X86_ASSEMBLY && !defined (DPVS_GAMECUBE_ASSEMBLY)

// returns -1 if value == 0 (i.e. no bits have been sit)
DPVS_FORCE_INLINE int getHighestSetBit (unsigned int value)
#if defined (DPVS_X86_ASSEMBLY)
#pragma warning (disable:4035) // don't whine about no return value (it's returned in eax)
{
#if defined (DPVS_X86_RETURN_EAX)
	__asm
	{
		mov eax,-1				// if (a==0), bsr doesn't set eax so we store -1 here now
		bsr eax,[value]
	}
#else
	int retVal;
	__asm
	{
		mov eax,-1				// if (a==0), bsr doesn't set eax so we store -1 here now
		bsr eax,[value]
		mov [retVal],eax
	}
	return retVal;
#endif
}
#pragma warning (default:4035) 
#elif defined (DPVS_BUILD_CW) && (defined (DPVS_CPU_GAMECUBE) || defined (DPVS_CPU_PPC))
{
	return 31 - __cntlzw(value);
}
#else
{
	if (value&0xffff0000)
	{
		if (value&0xff000000)
			return BitMath::s_highestLUT[value>>24]+24;
		return BitMath::s_highestLUT[value>>16]+16;
	}
	if (value&0xff00)
		return BitMath::s_highestLUT[value>>8]+8;
	return BitMath::s_highestLUT[value];
}
#endif // DPVS_X86_ASSEMBLY

/*
HERE'S A NEW VERSION THAT DOESN'T USE THE LUT (FOR PLATFORMS WITHOUT BITSCAN INSTRUCTIONS)
inline UINT32 genNonZeroMask (UINT32 x) { return (x==0) ? 0 : -1; }	// assume that compiler generates SBB or something..
int getHighestSetBitNew (UINT32 value)
{
	UINT32 pos = ~genNonZeroMask(value);
	pos += genNonZeroMask (value & 0xFFFF0000)		& 16;
	pos += genNonZeroMask ((value>>pos) & (0xFF00))	& 8;
	pos += genNonZeroMask ((value>>pos) & (0xF0))	& 4;
	pos += genNonZeroMask ((value>>pos) & (0xC))	& 2;
	pos += genNonZeroMask ((value>>pos) & (0x2))	& 1;
	return pos;
}
*/
// routine for testing if a dword value is an exact power of two. The number zero
// is regarded to be a power of two.

DPVS_FORCE_INLINE bool isPowerOfTwo (unsigned int a)
{
	return (((a-1)&a)==0);
}

DPVS_FORCE_INLINE unsigned int getNextPowerOfTwo(unsigned int value)
#if defined (DPVS_X86_ASSEMBLY)
#pragma warning (disable:4035) // don't whine about no return value (it's returned in eax)
{
#if defined (DPVS_X86_RETURN_EAX)
	__asm
	{
		mov ecx,-1
		mov eax,value
		dec eax
		bsr ecx,eax
		mov eax,1
		inc ecx
		shl eax,cl				// return value is always in eax
	}
#else
	unsigned int retVal;
	__asm
	{
		mov ecx,-1
		mov eax,value
		dec eax
		bsr ecx,eax
		mov eax,1
		inc ecx
		shl eax,cl				// return value is always in eax
		mov retVal,eax
	}
	return retVal;
#endif
}
#pragma warning (default:4035) 
#else
{
	if (value > 1)
		return (unsigned int)(1<<(getHighestSetBit(value-1)+1));
	else
		return 1;
}
#endif // DPVS_X86_ASSEMBLY

// Lists numbers of set bits so that time spent only depends on active bits.

inline int maskToList(int* list,UINT32 mask)
{
	UINT32	t=mask;
	int		cnt = 0;

	while(t)
	{
		UINT32 t2 = (t&(t-1));
		list[cnt++]	= getHighestSetBit(t^t2);	//bitfield -> number
		t = t2;
	}
	return cnt;
}

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSBITMATH_HPP
