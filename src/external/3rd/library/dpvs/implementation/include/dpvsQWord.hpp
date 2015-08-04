#ifndef __DPVSQWORD_HPP
#define __DPVSQWORD_HPP
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
 * Description: 	64-bit integer manipulation routines
 *
 * $Archive: /dpvs/implementation/include/dpvsQWord.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 20.06.01 16:04 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSBITMATH_HPP)
#	include "dpvsBitMath.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:			DPVS::QWORD
 *
 * Description:		Class for representing a 64-bit unsigned integer.
 *
 * Notes:			This is the emulated version with 32-bit integers
 *
 ******************************************************************************/

#if !defined (DPVS_UINT64_DEFINED)

class QWORD
{
private:
	UINT32	l;														// low part 
	UINT32	h;														// high part

	DPVS_FORCE_INLINE UINT32&				low				(void)										{ return DPVS_SELECT_ENDIAN(l,h); }
	DPVS_FORCE_INLINE UINT32&				high			(void)										{ return DPVS_SELECT_ENDIAN(h,l); }
	DPVS_FORCE_INLINE int					getBitCount1x1	(void) const;
	DPVS_FORCE_INLINE int					getBitCount2x2	(void) const;
	DPVS_FORCE_INLINE int					getBitCount4x4	(void) const;

public:
	
	DPVS_FORCE_INLINE 						QWORD				(void) : l(0), h(0)						{}
	DPVS_FORCE_INLINE						QWORD				(const QWORD& s)						{ l = s.l; h = s.h; }
	DPVS_FORCE_INLINE 						QWORD				(UINT32 qh,UINT32 ql) : DPVS_SELECT_ENDIAN(l(ql),l(qh)), DPVS_SELECT_ENDIAN(h(qh),h(ql))	{}

	DPVS_FORCE_INLINE void					operator|=			(const QWORD &s);
	DPVS_FORCE_INLINE void					operator&=			(const QWORD &s);
	DPVS_FORCE_INLINE void					operator^=			(const QWORD &s);
	DPVS_FORCE_INLINE bool					operator==			(const QWORD &s) const					{ return l==s.l && h==s.h; }
	DPVS_FORCE_INLINE bool					operator!=			(const QWORD &s) const					{ return l!=s.l || h!=s.h; }
	DPVS_FORCE_INLINE QWORD&				operator=			(const QWORD &s)						{ l = s.l; h = s.h; return *this; }
	DPVS_FORCE_INLINE bool					changesIfMergedWith	(const QWORD& s) const					{ return ((s.l&~l) | (s.h&~h))!=0; }
	DPVS_FORCE_INLINE void					dec					(void)									{ if (!low()) high()--; low()--; }
	DPVS_FORCE_INLINE bool					empty				(void) const							{ return (l|h)==0; }
	DPVS_FORCE_INLINE bool					full				(void) const							{ return (l&h)==0xFFFFFFFF; }
	DPVS_FORCE_INLINE int					getBitCount			(void) const;
	DPVS_FORCE_INLINE unsigned char			getByte				(int n) const							{ DPVS_ASSERT(n>=0&&n<=7);const unsigned char* ptr=(const unsigned char*)&l;		return DPVS_SELECT_ENDIAN( ptr[n], ptr[7-n]); }
	DPVS_FORCE_INLINE unsigned char*		getBytePtr			(int n)									{ DPVS_ASSERT(n>=0&&n<=7);unsigned char* ptr=(unsigned char*)&l;					return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[7-n]); }
	DPVS_FORCE_INLINE const unsigned char*	getBytePtr			(int n)	const							{ DPVS_ASSERT(n>=0&&n<=7);const unsigned char* ptr=(const unsigned char*)&l;		return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[7-n]); }
	DPVS_FORCE_INLINE UINT32*				getDWORDPtr			(int n)									{ DPVS_ASSERT(n>=0&&n<=1);UINT32*ptr=(UINT32*)&l;				return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[1-n]); }
	DPVS_FORCE_INLINE const UINT32*			getDWORDPtr			(int n) const							{ DPVS_ASSERT(n>=0&&n<=1);const UINT32*ptr=(const UINT32*)&l;	return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[1-n]); }
	DPVS_FORCE_INLINE UINT32				getHigh				(void) const							{ return DPVS_SELECT_ENDIAN(h,l);		}
	DPVS_FORCE_INLINE UINT32				getLow				(void) const							{ return DPVS_SELECT_ENDIAN(l,h);		}
	DPVS_FORCE_INLINE void					setFull				(void)									{ l = 0xFFFFFFFF; h = 0xFFFFFFFF; }
	DPVS_FORCE_INLINE void					setZero				(void)									{ l = 0; h = 0; }

};

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

DPVS_FORCE_INLINE void		QWORD::operator|=	(const QWORD &s)					{ l|=s.l; h|=s.h; /*return *this;*/ }
DPVS_FORCE_INLINE void		QWORD::operator&=	(const QWORD &s)					{ l&=s.l; h&=s.h; /*return *this;*/ }
DPVS_FORCE_INLINE void		QWORD::operator^=	(const QWORD &s)					{ l^=s.l; h^=s.h; /*return *this;*/ }
DPVS_FORCE_INLINE QWORD	operator~			(const QWORD& a)					{ return QWORD(~a.getHigh(),~a.getLow()); }
DPVS_FORCE_INLINE QWORD	operator|			(const QWORD& a,const QWORD& b)		{ return QWORD(a.getHigh()|b.getHigh(), a.getLow()|b.getLow()); }
DPVS_FORCE_INLINE QWORD	operator&			(const QWORD& a,const QWORD& b)		{ return QWORD(a.getHigh()&b.getHigh(), a.getLow()&b.getLow()); }
DPVS_FORCE_INLINE QWORD	operator^			(const QWORD& a,const QWORD& b)		{ return QWORD(a.getHigh()^b.getHigh(), a.getLow()^b.getLow()); }

DPVS_FORCE_INLINE int QWORD::getBitCount1x1(void) const
{
	UINT32 ml = l;
	UINT32 mh = h;
	ml = ((ml >>  1) & 0x55555555) + (ml & 0x55555555);		// space for 3  (max 2)
	mh = ((mh >>  1) & 0x55555555) + (mh & 0x55555555);
	ml = ((ml >>  2) & 0x33333333) + (ml & 0x33333333);		// space for 15 (max 4)
	mh = ((mh >>  2) & 0x33333333) + (mh & 0x33333333);
	ml+= mh;
	ml = ((ml >>  4) & 0x0F0F0F0F) + (ml & 0x0F0F0F0F);
	ml = ((ml >>  8) + ml) & 0x00ff00ff;
	ml = ((ml >> 16) + ml) & 0x0000FF;
	return int(ml);
}

DPVS_FORCE_INLINE int QWORD::getBitCount2x2(void) const
{
	UINT32 ml = l;
	UINT32 mh = h;
	ml = ((ml >> 1) | ml) & 0x55555555;					// 01010101 01010101 01010101 01010101
	mh = ((mh >> 1) | mh) & 0x55555555;					// 01010101 01010101 01010101 01010101
	ml = ((ml >> 8) | ml) & 0x00ff00ff;					// 00000000 01010101 00000000 01010101
	mh = ((mh >> 8) | mh) & 0x00ff00ff;					// 00000000 01010101 00000000 01010101
	ml+= mh;
	ml = (ml >> 2 & 0x00330033) + (ml & 0x00330033);	// 00000000 -1-1-1-1 00000000 -1-1-1-1
	ml = (ml >> 4 & 0x0f0f0f0f) + (ml & 0x0f0f0f0f);
	ml = ((ml>>16) + (ml)) & 0xFF;

	return (int)(ml);
}

DPVS_FORCE_INLINE int QWORD::getBitCount4x4(void) const
{
	UINT32 ml = l;
	UINT32 mh = h;
	ml = ((ml >> 1) | ml) & 0x55555555;
	mh = ((mh >> 1) | mh) & 0x55555555;
	ml = ((ml >> 2) | ml) & 0x33333333;
	mh = ((mh >> 2) | mh) & 0x33333333;
	ml = ((ml >> 8) | ml) & 0x00ff00ff;
	mh = ((mh >> 8) | mh) & 0x00ff00ff;
	ml = ((ml >>16) | ml) & 0x00000033;
	mh = ((mh >>16) | mh) & 0x00000033;
	ml+= mh;
	ml = ((ml >> 4) + ml) & 0xF;
	return (int)(ml);
}

#else


/******************************************************************************
 *
 * Class:			DPVS::QWORD
 *
 * Description:		Class for representing a 64-bit unsigned integer.
 *
 * Notes:			This is the native version with 64-bit integers
 *
 ******************************************************************************/

class QWORD
{

	int						getBitCount1x1	(void) const;
	int						getBitCount2x2	(void) const;
	int						getBitCount4x4	(void) const;

public:
	UINT64	m_value;
	explicit	QWORD		(UINT64 q) : m_value(q)					{}
				QWORD		(UINT32 high,UINT32 low)				{ m_value = (((UINT64)(high))<<32) | (UINT64)low; }
				QWORD		(void) : m_value(0)						{}
//	explicit	QWORD		(UINT32 v)								{ m_value = (UINT64)v; }
	void		setZero		(void)									{ m_value = 0; }
	void		setFull		(void)									{ m_value = 0xFFFFFFFFFFFFFFFF; }
//int			getBitCount	(void) const;
	UINT32		getHigh		(void) const							{ return (UINT32)(m_value>>32);	}
	UINT32		getLow		(void) const							{ return (UINT32)(m_value);		}
	int			getBitCount	(void) const;
	void		dec			(void)									{ --m_value	; }
	void		operator|=	(const QWORD& s)						{ m_value |= s.m_value; }
	void		operator&=	(const QWORD& s)						{ m_value &= s.m_value; }
	void		operator^=	(const QWORD& s)						{ m_value ^= s.m_value; }
	bool		operator==	(const QWORD &s) const					{ return m_value == s.m_value; } 
	bool		operator!=	(const QWORD &s) const					{ return m_value != s.m_value; }
	
	bool		empty		(void) const							{ return m_value==0;  }
	bool		full		(void) const							{ return (m_value+1)==0; }
	bool		changesIfMergedWith(const QWORD& s) const			{ return (s.m_value & ~m_value) != 0; }

	unsigned char			getByte		(int n) const				{ DPVS_ASSERT(n>=0&&n<=7);const unsigned char* ptr=(const unsigned char*)&m_value;		return DPVS_SELECT_ENDIAN( ptr[n], ptr[7-n]); }
//	UINT32					getDWORD	(int n) const				{ DPVS_ASSERT(n>=0&&n<=1);const UINT32*ptr=(const UINT32*)&m_value;		return DPVS_SELECT_ENDIAN( ptr[n], ptr[1-n]); }

	unsigned char*			getBytePtr	(int n)						{ DPVS_ASSERT(n>=0&&n<=7);unsigned char* ptr=(unsigned char*)&m_value;					return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[7-n]); }
	const unsigned char*	getBytePtr	(int n)	const				{ DPVS_ASSERT(n>=0&&n<=7);const unsigned char* ptr=(const unsigned char*)&m_value;		return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[7-n]); }
	UINT32*					getDWORDPtr	(int n)						{ DPVS_ASSERT(n>=0&&n<=1);UINT32* ptr=(UINT32*)&m_value;					return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[1-n]); }
	const UINT32*			getDWORDPtr	(int n) const				{ DPVS_ASSERT(n>=0&&n<=1);const UINT32* ptr=(const UINT32*)&m_value;		return DPVS_SELECT_ENDIAN(&ptr[n],&ptr[1-n]); }

};

DPVS_FORCE_INLINE QWORD  operator~			(const QWORD& a)					{ return QWORD(~a.m_value);  }
DPVS_FORCE_INLINE QWORD  operator|			(const QWORD& a,const QWORD& b)		{ return QWORD(a.m_value|b.m_value); }
DPVS_FORCE_INLINE QWORD  operator&			(const QWORD& a,const QWORD& b)		{ return QWORD(a.m_value&b.m_value); }
DPVS_FORCE_INLINE QWORD  operator^			(const QWORD& a,const QWORD& b)		{ return QWORD(a.m_value^b.m_value); }

DPVS_FORCE_INLINE int QWORD::getBitCount1x1(void) const
{
	UINT64 t = m_value;
	t = (t >> 1 & 0x5555555555555555) + (t & 0x5555555555555555);
	t = (t >> 2 & 0x3333333333333333) + (t & 0x3333333333333333);
	t = (t >> 4 & 0x0f0f0f0f0f0f0f0f) + (t & 0x0f0f0f0f0f0f0f0f);
	t = (t >> 8 & 0x00ff00ff00ff00ff) + (t & 0x00ff00ff00ff00ff);
	t = (t >>16 & 0x0000ffff0000ffff) + (t & 0x0000ffff0000ffff); 
	t = (t >>32 & 0x00000000ffffffff) + (t & 0x00000000ffffffff); 
	return (int)(t);
}

DPVS_FORCE_INLINE int QWORD::getBitCount2x2(void) const
{
	UINT64 t = m_value;
	t = ((t >> 1) | t) & 0x5555555555555555;
	t = ((t >> 8) | t) & 0x00ff00ff00ff00ff;
	t = (t >> 2 & 0x3333333333333333) + (t & 0x3333333333333333);
	t = (t >> 4 & 0x0f0f0f0f0f0f0f0f) + (t & 0x0f0f0f0f0f0f0f0f); 
	t = (t >>16 & 0x0000ffff0000ffff) + (t & 0x0000ffff0000ffff);
	t = ((t >>32) + t) & 0x00000000000000FF;
	return (int)(t);
}

DPVS_FORCE_INLINE int QWORD::getBitCount4x4(void) const
{
	UINT64 t = m_value;
	t = ((t >> 1) | t) & 0x5555555555555555;
	t = ((t >> 2) | t) & 0x3333333333333333;
	t = ((t >> 8) | t) & 0x00ff00ff00ff00ff;
	t = ((t >>16) | t) & 0x0000ffff0000ffff;
	t = (t >> 4 & 0x0f0f0f0f0f0f0f0f) + (t & 0x0f0f0f0f0f0f0f0f); 
	t = ((t >>32) + t) & 0x000000000000000f;
	return (int)(t);
}

#endif // DPVS_UINT64_DEFINED

//------------------------------------------------------------------------
// Some Bitmath routines that utilize QWords
//------------------------------------------------------------------------


DPVS_CT_ASSERT(sizeof(QWORD)==8);			// make sure


DPVS_FORCE_INLINE int QWORD::getBitCount(void) const
{
//	int partial4x4 = getBitCount4x4();		// how many partially full 4x4 blocks (max 4)
//	int partial2x2 = getBitCount2x2();		// how many partially full 2x2 blocks (max 16)
	int	partial1x1 = getBitCount1x1();		// how many full 1x1 blocks (max 64)

	return partial1x1;// - (16-partial2x2) - 2*(4-partial4x4);
}

// returns -1 if value == 0 (i.e. no bits have been sit)
DPVS_FORCE_INLINE int getHighestSetBit (const QWORD& value)
{
	if (value.getHigh()) 
		return getHighestSetBit(value.getHigh()) + 32;
	return getHighestSetBit(value.getLow());
}

DPVS_FORCE_INLINE int maskToList(int* list, const QWORD& mask)
{
	UINT32	t;
	int		cnt = 0;

	// lower DWORD
	t = mask.getLow();
	while(t)
	{
		UINT32 t2 = (t&(t-1));
		list[cnt++]	= getHighestSetBit(t^t2);		//bitfield -> number
		t = t2;
	}

	// higher DWORD
	t = mask.getHigh();
	while(t)
	{
		UINT32 t2 = (t&(t-1));
		list[cnt++]	= 32 + getHighestSetBit(t^t2);	//bitfield -> number
		t = t2;
	}

	return cnt;
}

} // DPVS
//------------------------------------------------------------------------
#endif // __DPVSQWORD_HPP
