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
 * Description: 	Hierarchical depth estimation buffer
 *
 * $Archive: /dpvs/implementation/sources/dpvsHZBuffer.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 18.06.01 15:35 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsHZBuffer.hpp"
#include "dpvsBitMath.hpp"		
#include "dpvsStatistics.hpp"

namespace DPVS
{ 

/******************************************************************************
 *
 * Class:			DPVS::HZBuffer
 *
 * Description:		Hierarchical depth buffer implementation
 *
 ******************************************************************************/

class HZBuffer
{
public:
	typedef IHZBuffer::DepthValue DepthValue;			// pixel type
private:
				HZBuffer				(void);				// not allowed
				HZBuffer				(const HZBuffer&);	// not allowed
	HZBuffer&	operator=				(const HZBuffer&);	// not allowed
	
#if defined (DPVS_PARANOID)
	void		selfTest				(int level) const;
#endif

	enum 
	{ 
		MAX_LEVELS			= 12,					// max hierarchy levels
		MIN_LEVEL_DIM		= 2						// x/y dimension of smallest level
	};					

	struct Level
	{
		unsigned char*	m_modMask;					// x-direction modification mask (1 bit per pixel)
		unsigned char*	m_modMaskY;					// y-direction modification mask (1 bit per scanline)
		DepthValue*		m_data;						// level data (all levels are allocated in one large chunk!)
		int				m_width;					// level width
		int				m_height;					// level height

		int		getModMaskPitch			(void) const { return (int)(((unsigned int)(m_width)+31)>>5)<<2; }	// returns pitch in bytes
		void	clearModificationMasks	(void)
		{
			fillByte (m_modMask,0,getModMaskPitch()*m_height);
			fillByte (m_modMaskY,0, ((m_height+31)>>5)*sizeof(UINT32));
		}

		DepthValue	getFarthest (void) const { int sz = m_width*m_height; DepthValue f = *m_data; for (int i = 1; i < sz; i++) if (m_data[i]>f) f = m_data[i]; return f; }
	};

	const DepthValue*	m_buffer;					// pointer to source z-buffer buffer (full-size)
	UINT32				m_lModifiedMask;			// level modified mask
	DepthValue			m_farthest;					// global 'full screen' farthest depth value
	int					m_xWidth;					// width of source buffer
	int					m_yHeight;					// height of source buffer
	int					m_pitch;					// pitch of source buffer (in terms of DepthValue)
	int					m_numLevels;				// number of levels in hierarchy

	Level				m_hBuffer[MAX_LEVELS];		// hierarchical depth buffer levels

	unsigned char*		m_alloc;					// all buffers allocated with this ptr so that they are all aligned and close in memory
	int					m_allocSize;				// number of bytes allocated by m_alloc

	static unsigned char s_shuffle[256];			// lookup table for 8->4 bit shuffle

	void								updateDepthBufferLevels			(int lastLevel);
	void								updateDepthBufferLevel			(int levelIndex);
	void								propagateDepthBufferLevel		(int levelIndex);
	void								updateDepthBufferScanline		(unsigned char* mk, DepthValue* d, const DepthValue* s, int sp, int n);
	bool								isVisibleRecursive				(int level, int x0, int y0, int x1, int y1, DepthValue z) const;
	bool								isVisibleLevel0					(int x0, int y0, int x1, int y1, DepthValue z) const;
	bool								isVisibleLevel1					(int x0, int y0, int x1, int y1, DepthValue z) const;
	DPVS_FORCE_INLINE const DepthValue*	getBufferPtr					(int x, int y) const;
	bool								isPixelModified					(int level, int x, int y) const;

	static DPVS_FORCE_INLINE void		initializeLookupTables			(void);
	static DPVS_FORCE_INLINE DepthValue	max4							(DepthValue a, DepthValue b, DepthValue c, DepthValue d);
	static DPVS_FORCE_INLINE void		setMask32						(UINT32* d, UINT32 mask, int p, int n);
	static DPVS_FORCE_INLINE UINT32		calculateMask32					(int x0, int x1);
	static DPVS_FORCE_INLINE bool		copyModificationMaskScanline	(unsigned char* d, const unsigned char* s, int sp, int n);

public:

				HZBuffer				(const DepthValue* src, int log2xWidth, int log2yHeight, int p);
				~HZBuffer				(void);
	DepthValue	getFarDepth				(void) const;
	void		setRegionModified		(int x0, int y0, int x1, int y1);
	bool		isVisible				(int x, int y, DepthValue z) const;
	bool		isVisible				(int x0, int y0, int x1, int y1, DepthValue z) const;
	bool		isSilhouetteVisible		(const int* leftEdges, const int* rightEdges, DepthValue z, int y0, int n);
//	void		test					(void) const;


};
} // DPVS

#define CHECK_POINTER(X) DPVS_ASSERT((UPTR)(X)>=(UPTR)(m_alloc) && (UPTR)(X) < (UPTR)(m_alloc+m_allocSize))

using namespace DPVS;

unsigned char HZBuffer::s_shuffle[256];						// shuffle table

/*****************************************************************************
 *
 * Function:		HZBuffer::getBufferPtr()
 *
 * Description:		Returns pointer to source depth buffer at location (x,y)
 *
 * Parameters:		x = x-coordinate
 *					y = y-coordinate
 *
 * Returns:			pointer to depth buffer at specified location
 *
 *****************************************************************************/

DPVS_FORCE_INLINE const HZBuffer::DepthValue*	HZBuffer::getBufferPtr (int x, int y) const 
{ 
	DPVS_ASSERT (x >= 0 && y >= 0 && y < m_yHeight && x < m_xWidth);
	return m_buffer + m_pitch*y + x;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::initializeLookupTables()
 *
 * Description:		Initializes 8->4 bit extraction shuffle table
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void HZBuffer::initializeLookupTables (void)
{
	int	i;
	for (i = 0; i < 256; i++)
	{
		unsigned char mk = 0;
		for (int j = 0; j < 4; j++)
		if (i&(3<<(j*2)))
			mk|=(1<<j);
		s_shuffle[i] = mk;
	}
}

/*****************************************************************************
 *
 * Function:		HZBuffer::HZBuffer()
 *
 * Description:		Constructor
 *
 * Parameters:		src	= pointer to source depth buffer
 *					lxW = log2 of source depth buffer width in pixels (256 -> 8)
 *					lyH = log2 of source depth buffer height in pixels
 *					p	= source depth buffer pitch in bytes (must be divisible by sizeof(DepthValue))
 *
 *****************************************************************************/

DPVS_FORCE_INLINE HZBuffer::HZBuffer (const DepthValue* src, int lxW, int lyH, int p)
{
	initializeLookupTables ();

	DPVS_ASSERT (src);
	DPVS_ASSERT (lxW>=0 && lyH>=0);
	DPVS_ASSERT (p >= 1 && !(p % sizeof(DepthValue)));			// illegal pitch?

	m_allocSize		= 0;
	m_buffer		= src;										// source depth buffer pointer
	m_xWidth		= 1<<lxW;									// source width	
	m_yHeight		= 1<<lyH;									// source height
	m_pitch			= (unsigned int)(p) / sizeof(DepthValue);	// source data pitch in terms of DepthValue
	m_alloc			= 0;
	m_farthest		= 0xFFFFFFFF;

	int	i;

	fillByte (m_hBuffer, 0, MAX_LEVELS * sizeof(Level));	// clear the hBuffer structure

	//--------------------------------------------------------------------
	// Calculate level dimensions
	//--------------------------------------------------------------------
	
	int sz	= 0;								// total # of depth pixels
	int bd	= 0;								// total # of dwords needed to store modification mask
	int bdy  = 0;								// total # of dwords needed to store scanline modification bits
	int x	= (1<<lxW)>>1;						// x dimension of level
	int y	= (1<<lyH)>>1;						// y dimension of level

	m_numLevels	= 0;									// init number of hierarchy levels
	
	while (x >= MIN_LEVEL_DIM && y >= MIN_LEVEL_DIM && m_numLevels < MAX_LEVELS)
	{
		bdy += (y+31)>>5;							
		bd  += ((x+31)>>5)*y;					
		sz  += x*y;								
		
		m_hBuffer[m_numLevels].m_width  = x;
		m_hBuffer[m_numLevels].m_height = y;
		x >>= 1;
		y >>= 1;
		m_numLevels++;								
	};

	//--------------------------------------------------------------------
	// Allocate buffers (alloc all with a single call to new) and
	// initialize the hierarchical depth buffer level data pointers.
	// The internal allocs are cache-line aligned.
	//--------------------------------------------------------------------

	m_allocSize = CACHE_LINE_SIZE + sizeof(DepthValue)*sz + sizeof(UINT32)*bd + sizeof(UINT32)*bdy + sizeof(DepthValue*)*m_yHeight;

	m_alloc = NEW_ARRAY<unsigned char>(m_allocSize);
	fillByte (m_alloc,0,m_allocSize);													// clear the allocated memory

	DepthValue*		d	= reinterpret_cast<DepthValue*>(alignCacheLine(m_alloc));		// depth values
	unsigned char*	mm	= reinterpret_cast<unsigned char*>(d + sz);						// x-modification mask
	unsigned char*	my	= reinterpret_cast<unsigned char*>(mm + bd*sizeof(UINT32));		// y-modification mask

	for (i = 0; i < m_numLevels; i++)
	{
		DPVS_ASSERT (!((UPTR)(d)&3));							// make sure all data pointers are properly aligned
		DPVS_ASSERT (!((UPTR)(mm)&3));
		DPVS_ASSERT (!((UPTR)(my)&3));

		Level* lv = m_hBuffer + i;
		lv->m_data		= d;
		lv->m_modMask	= mm;
		lv->m_modMaskY	= my;
		d  += lv->m_width*lv->m_height;
		mm += ((lv->m_width+31)>>5)*lv->m_height*sizeof(UINT32);
		my += ((lv->m_height+31)>>5)*sizeof(UINT32);
	}

	m_lModifiedMask = 1;												// set all levels as modified
	setRegionModified (0,0, m_xWidth, m_yHeight);						// mark everything as dirty
}

/*****************************************************************************
 *
 * Function:		HZBuffer::~HZBuffer()
 *
 * Description:		Destructor
 *
 *****************************************************************************/

DPVS_FORCE_INLINE HZBuffer::~HZBuffer (void)
{
	DELETE_ARRAY(m_alloc);
}


/*****************************************************************************
 *
 * Function:		HZBuffer::isVisible (int,int, DepthValue)
 *
 * Description:		Queries visibility status of a single point
 *
 * Parameters:		x	= point's x-coordinate
 *					y	= point's y-coordinate
 *					z   = point's depth value
 *
 * Returns:			true if point is visible, false otherwise
 *
 * Notes:			The routine doesn't perform clipping of the x/y coordinates -
 *					the calling routine must take care of this.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::isVisible (int x, int y, DepthValue z) const
{
	DPVS_ASSERT (x <= m_xWidth && y <= m_yHeight);								// just some DPVS_ASSERTions
	return (z <= *getBufferPtr(x,y));
}

/*****************************************************************************
 *
 * Function:		HZBuffer::copyModificationMaskScanline()
 *
 * Description:		Converts 2x2 areas of modification mask into
 *					1x1 masks
 *
 * Parameters:		d	= destination mask buffer
 *					s	= source mask buffer
 *					sp	= source buffer pitch in bytes
 *					n	= number of destination pixels
 *
 * Returns:			zero if all destination pixels are 0, non-zero otherwise
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::copyModificationMaskScanline (unsigned char* d, const unsigned char* s, int sp, int n)
{
	unsigned int any = 0;

	for (int j = 0; j < n; j++, s+=2,d++)									// write 2x2->1x1
	{
		unsigned char b =   (unsigned char)(s_shuffle[ (int)(s[0] | s[sp]) ] | 
						   (s_shuffle[ (int)(s[1] | s[sp+1]) ]<<4));
		any|= b;								
		*d |= b;
	}
	return any ? true : false;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::max4()
 *
 * Description:		Returns largest of four depth values
 *
 * Parameters:		a	= first value
 *					b	= second value
 *					c	= third value
 *					d	= fourth value
 *
 * Returns:			Largest depth value
 *
 * Notes:			The routine uses integer arithmetics so that no branches
 *					are made.
 *
 *					The routine assumes that the depth values DO NOT HAVE
 *					the highest bit set!! (this is true, since the HZBuffer
 *					values are _positive_ floating point values). It would not
 *					work if highest bits could be set.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE HZBuffer::DepthValue HZBuffer::max4 (DepthValue a, DepthValue b, DepthValue c, DepthValue d)
{
#if defined (DPVS_DEBUG)
	UINT32 res = (a > b) ? a : b;
	res = (res > c) ? res : c;
	res = (res > d) ? res : d;
	DPVS_ASSERT (!(a&0x80000000));
	DPVS_ASSERT (!(b&0x80000000));
	DPVS_ASSERT (!(c&0x80000000));
	DPVS_ASSERT (!(d&0x80000000));
#endif

	UINT32 delta;
	delta = a-b;
	a-=(delta & ((int)(delta)>>31));
	delta = a-c;
	a-=(delta & ((int)(delta)>>31));
	delta = a-d;
	a-=(delta & ((int)(delta)>>31));

#if defined (DPVS_DEBUG)
	DPVS_ASSERT( a == res );
#endif

	return a;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::updateDepthBufferScanline()
 *
 * Description:		Updates single scanline in depth buffer
 *
 * Parameters:		mk = modification bit mask
 *					d  = destination depth values
 *					s  = source (2x2) depth values
 *					srcPitch = source pitch (typically 2*destination pitch)
 *					n  = number of destination pixels
 *
 *****************************************************************************/

DPVS_FORCE_INLINE void HZBuffer::updateDepthBufferScanline (unsigned char* modMask, DepthValue* d, const DepthValue* s, int srcPitch, int n)
{
	n = (n+7)>>3;												// divide by 8
	
	for (int j = 0; j < n; j++, s+=16, d+=8)					// step 8 pixels at a time
	{
		unsigned int mask = (unsigned int)(modMask[j]);			// read mask

		if (!mask)
			continue;

		//------------------------------------------------------------
		// For each modified 2x2 block of pixels, find the farthest
		// value in the 2x2 set and if its different to old farthest
		// value, mark the corresponding 1x1 block modified and store
		// new farthest value.
		//------------------------------------------------------------

		unsigned int	dmask	= 0;							// new output mask (true changes)
		int				index	= 0;

		do
		{
			if (mask & 1)										// is 2x2 block dirty?
			{
				DepthValue farthest = max4(s[index*2+0],s[index*2+1],s[index*2+srcPitch],s[index*2+srcPitch+1]);
				if (farthest != d[index])						// if changed, update dmask
				{
					d[index] = farthest;						// set new farthest value
					dmask |= (1<<index);
				}
			}


			index++;
			mask>>=1;											// remove lowest bit from source mask
		} while (mask);
		
		modMask[j] = (unsigned char)(dmask);					// write change mask
	}
}

/*****************************************************************************
 *
 * Function:		HZBuffer::propagateDepthBufferLevel()
 *
 * Description:		Propagates modification masks to next level
 *
 * Parameters:		l	= level index
 *
 * Notes:			This function is only called by HZBuffer::updateDepthBufferLevel()
 *
 *****************************************************************************/

void HZBuffer::propagateDepthBufferLevel (int levelIndex)
{
	//--------------------------------------------------------------------
	// If this is the final depth buffer level, just clear the 
	// modification mask. Otherwise copy 2x2->1x1 OR'ed version
	// of output modification mask to next level.
	//--------------------------------------------------------------------

	if (levelIndex == (m_numLevels-1))						// final level - just clear
	{
		Level*	lv = &m_hBuffer[levelIndex];				// get level pointer
		lv->clearModificationMasks();						// clear modification masks
		m_farthest = lv->getFarthest();						// find farthest value

		#if defined (DPVS_PARANOID)							// self-checking
			for (int y = 0; y < m_yHeight; y++)
			for (int x = 0; x < m_xWidth; x++)
				DPVS_ASSERT(m_farthest >= *getBufferPtr(x,y));
		#endif
	}
	else
	{
		DPVS_ASSERT(levelIndex >= 0 && levelIndex < (m_numLevels-1));

		Level*	ls	= &m_hBuffer[levelIndex];
		int		dw	= (ls[1].m_width+7)>>3;									// destination width in bytes
		int		dh	= ls[1].m_height;										// destination height
		int	    dp	= ls[1].getModMaskPitch();								// destination pitch in bytes
		int		sp	= ls->getModMaskPitch();								// source pitch in bytes
		unsigned char* ymk = ls->m_modMaskY;								// source y-direction modification mask
		
		unsigned int nextLevelMask = (1<<(levelIndex+1));

		for (int k = 0; k < dh; k+=4, ymk++)
		if (*ymk)															// if any of the 8 source scanlines have been changed...
		{
			CHECK_POINTER(ymk);
			unsigned int mk	 = s_shuffle[*ymk];								// extract 4-bit output mask
			DPVS_ASSERT(mk<16);												// make sure only 4 bits
			*ymk = 0;														// clear source ymask for 8 scanlines

			unsigned char*	s		= ls->m_modMask   + k*sp*2;				// source pointer
			unsigned char*	d		= ls[1].m_modMask + k*dp;				// destination pointer
			unsigned int	omask	= 0;									// output mask
			unsigned int	c		= 1;									// mask index

			for (;;)
			{
				if (mk&1)													// if scanline(s) have changed
				{
					if (copyModificationMaskScanline (d,s,sp,dw))			// copy scanline's modification mask
						omask |= c;
					fillByte (s, 0, sp*2);									// clear source modmask for the scanline
				} 
				mk>>=1;													
				if (!mk)													// we're done...
					break;
				s += sp*2;													// advance source modmask pointer
				d += dp;													// advance dest modmask pointer
				c += c;														// advance mask (next power of two)
			}

			if (omask)														// if there was any output for the 4 scanlines
			{
				DPVS_ASSERT(omask<16);
				m_lModifiedMask |= nextLevelMask;							// set next level's mask
				ls[1].m_modMaskY[k>>3] |= (unsigned char)(omask << (k&7));	// write output y-mask
			} 
		} 
	}
}

/*****************************************************************************
 *
 * Function:		HZBuffer::updateDepthBufferLevel()
 *
 * Description:		Updates single level in hierarchical depth buffer and
 *					sets next level's modification mask
 *
 * Parameters:		l	= level index
 *
 *****************************************************************************/

void HZBuffer::updateDepthBufferLevel (int l)
{	
	//--------------------------------------------------------------------
	// Update statistics
	//--------------------------------------------------------------------

	if (l == 0)
	{
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_HZBUFFERLEVELUPDATES,1));
	}

	DPVS_ASSERT (l >= 0 && l < m_numLevels);											
	DPVS_ASSERT (m_lModifiedMask & (1<<l));								// why was this called if the mask is not set??


	//--------------------------------------------------------------------
	// Update depth buffer based on the modification mask. Save a new
	// modification mask to show which bits really change so that
	// we can decide on recursion.
	//--------------------------------------------------------------------

	Level*				lv	= &m_hBuffer[l];							// get level pointer
	const DepthValue*	s	= (l == 0) ? m_buffer : lv[-1].m_data;		// source buffer
	int					sp	= (l == 0) ? m_pitch : lv[-1].m_width;		// source pitch in terms of Depth Value
	int					w   = lv->m_width;
	int					mp	= lv->getModMaskPitch();					// modification buffer pitch in dwords
	int					h	= lv->m_height;								// height (in scanlines)
	const UINT8*		ymk = lv->m_modMaskY;							// y-direction modification mask

	for (int k = 0; k < h; k+=8, ymk++)
	if (*ymk)															// if any of the 8 scanlines have been modified?
	{
		CHECK_POINTER(ymk);
		const DepthValue*	ss = s + k*sp*2;
		DepthValue*			dd = lv->m_data + k*w;
		unsigned char*		mm = lv->m_modMask + k*mp;
		unsigned int		mk = *ymk;	

		for (;;)
		{
			if (mk&1)
				updateDepthBufferScanline	(mm,dd,ss,sp,w);			// .. update the scanline
			mk>>=1;
			if (!mk)
				break;
			ss+=sp*2;
			dd+=lv->m_width;
			mm+=mp;
		}
	}

	#if defined (DPVS_PARANOID)											// run consistency check for the level
	selfTest(l);	
	#endif

	propagateDepthBufferLevel(l);										// propagate modification masks downwards in the hierarchy
	
	m_lModifiedMask &=~(1<<l);											// clear level's modification mask
}


/*****************************************************************************
 *
 * Function:		HZBuffer::updateDepthBufferLevels()
 *
 * Description:		Updates levels up to (and including) level 'lastLevel'
 *
 * Parameters:		lastLevel  = last level to update (inclusive)
 *
 *****************************************************************************/

void HZBuffer::updateDepthBufferLevels (int lEnd)
{
	unsigned int mask = (1<<(lEnd+1))-1;			// mask containing the levels up to lEnd

	if (m_lModifiedMask & mask)						// .. if any level in the range has been modified..
	for (int l = 0; l <= lEnd; l++)
		if (m_lModifiedMask&(1<<l))					// if level has been modified...
			updateDepthBufferLevel (l);				// .. update the level
}

/*****************************************************************************
 *
 * Function:		HZBuffer::setMask32()
 *
 * Description:		Or's a mask into an array
 *
 * Parameters:		d		= destination UINT32 array
 *					mask	= UINT32 mask to or
 *					p		= destination array m_pitch (in SRDWORDS !!)
 *					count	= iteration count
 *
 * Notes:			The masking is perform using 32-bit values. Thus the
 *					mask must've been constructed properly (based on the
 *					endianess of the value)
 *
 *					The routine is manually unrolled by four.
 *
 *****************************************************************************/

#	define UNROLL4(Y) {int _IX=0,_NM = count&0xfffffffc; for (_IX=0;_IX<_NM;_IX+=4){Y(_IX+0);Y(_IX+1);Y(_IX+2);Y(_IX+3);}for(;_IX<count;_IX++){Y(_IX);}}

DPVS_FORCE_INLINE void HZBuffer::setMask32 (UINT32* d, UINT32 mask, int p, int count)
{
#define OPER(X) *d |= mask; d+=p;
	UNROLL4(OPER)
#undef OPER
}

#undef UNROLL4

/*****************************************************************************
 *
 * Function:		HZBuffer::calculateMask32()
 *
 * Description:		Calculates 32-bit coverage mask for a span
 *
 * Parameters:		x0  = span left coordinate (inclusive)
 *					x1	= span right coordinate (exclusive)
 *
 * Notes:			The coverage mask is returned in the format that must
 *					be used to write the mask as a 32-bit integer into the
 *					memory. Thus the return value of this function varies
 *					based on whether we have a little or big endian machine.
 *
 *****************************************************************************/

static unsigned int mask32[33] =	// internal lookup table for calculating the mask
{
	(1<<0)-1,(1<<1)-1,(1<<2)-1,(1<<3)-1,(1<<4)-1,(1<<5)-1,(1<<6)-1,(1<<7)-1,(1<<8)-1,(1<<9)-1,(1<<10)-1,
	(1<<11)-1,(1<<12)-1,(1<<13)-1,(1<<14)-1,(1<<15)-1,(1<<16)-1,(1<<17)-1,(1<<18)-1,(1<<19)-1,(1<<20)-1,
	(1<<21)-1,(1<<22)-1,(1<<23)-1,(1<<24)-1,(1<<25)-1,(1<<26)-1,(1<<27)-1,(1<<28)-1,(1<<29)-1,(1<<30)-1,
	0x7FFFFFFF,
	0xFFFFFFFF				// not all compilers can handle the (1<<32)-1 expression
};

DPVS_FORCE_INLINE UINT32 HZBuffer::calculateMask32 (int x0, int x1)
{
	DPVS_ASSERT ((x1-x0) <= 32 && (x1 > x0));
	UINT32 val = mask32[x1-x0] << (x0&31);

#if !defined (DPVS_LITTLE_ENDIAN)
	changeEndian (val);				// perform a byte swap for big-endian machines
#endif

	return val;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::isPixelModified()
 *
 * Description:		Internal function used in debug build to determine if
 *					modification mask is correctly set
 *
 * Parameters:		level	= level
 *					x		= pixel's x-coordinate
 *					y		= pixel's y-coordinate
 *
 * Returns:			true if pixel has been modified, false otherwise
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::isPixelModified (int level, int x, int y) const
{
	DPVS_ASSERT(level>=0);
	const Level* lv = &m_hBuffer[level];		
	const unsigned char* buf = lv->m_modMask + y*lv->getModMaskPitch() + (x>>3);
	return (*buf & (1<<(x&7)))!=0;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::setRegionModified()
 *
 * Description:		Marks square region in source depth buffer as modified -
 *					the modified regions are flushed later when queries
 *					to the HZBuffer are made
 *
 * Parameters:		x0	= inclusive left x coordinate
 *					y0	= inclusive top y coordinate
 *					x1  = exclusive right x coordinate
 *					y1  = exclusive bottom y coordinate
 *
 * Notes:			The routine doesn't perform clipping of the x/y coordinates -
 *					the calling routine must take care of this.
 *
 *****************************************************************************/

void HZBuffer::setRegionModified (int x0, int y0, int x1, int y1)
{
	DPVS_ASSERT (x0 >= 0 && x0 <= x1 && x1 <= m_xWidth );
	DPVS_ASSERT (y0 >= 0 && y0 <= y1 && y1 <= m_yHeight);

	x0>>=1;										// convert from source coordinates to level 0 coordinates
	y0>>=1;
	x1 = (x1+1)>>1;
	y1 = (y1+1)>>1;

	Level* lv = &m_hBuffer[0];					// get level pointer for level 0

	if ((x0 == x1) || (y0 == y1))				// zero area..
		return;

	//--------------------------------------------------------------------
	// Turn on "pixel modified" bits in the modification mask buffer
	//--------------------------------------------------------------------

	int		mp = (lv->m_width+31)>>5;				// pitch in terms of dwords
	UINT32*	mm = reinterpret_cast<UINT32*>(lv->m_modMask) + y0*mp;	// beginning of modifiable area
	int		n  = (y1-y0);							// number of scanlines

	int xb = x0;
	while (xb < x1)
	{
		int xe = (xb+32)&~31;				// end of span
		if (xe > x1)							// clipping
			xe = x1;
		setMask32 (mm + (xb>>5), calculateMask32 (xb,xe), mp, n);	// write mask
		xb = xe;
	}

	//--------------------------------------------------------------------
	// Set corresponding "scanline modified bits" (i.e. y-direction
	// mask)
	//--------------------------------------------------------------------

	UINT32*	modMaskY	= reinterpret_cast<UINT32*>(lv->m_modMaskY);
	int		yb			= y0;
	while (yb < y1)
	{
		int ye = (yb+32)&~31;
		if (ye > y1)
			ye = y1;
		CHECK_POINTER(modMaskY+(yb>>5));
		modMaskY[yb>>5] |= calculateMask32 (yb,ye);		
		yb = ye;
	}

	m_lModifiedMask |= 1;								// mark level 0 as modified

	//--------------------------------------------------------------------
	// In debug build ensure that the modification masks have been set
	// up correctly.
	//--------------------------------------------------------------------

#if defined (DPVS_PARANOID)
	{
		int x,y;

		for (y = y0; y < y1; y++)				
		for (x = x0; x < x1; x++)
			DPVS_ASSERT(isPixelModified(0,x,y));					// make sure the X direction modification masks are ok
		
		for (y = y0; y < y1; y++)
		{
			DPVS_ASSERT(lv->m_modMaskY[y>>3] & (1<<(y&7)));		// make sure Y direction mask is ok
		}
	}
#endif

}

/*****************************************************************************
 *
 * Function:		HZBuffer::isVisibleLevel0()
 *
 * Description:		Internal function for querying visibility at the 
 *					lowest level (i.e. the true depth buffer)
 *
 * Parameters:		x0		= inclusive left x coordinate
 *					y0		= inclusive top y coordinate
 *					x1		= inclusive right x coordinate
 *					y1		= inclusive bottom y coordinate
 *					z		= depth value
 *
 * Returns:			true if area is even partially visible, false otherwise
 *
 * Notes:			The HZBuffer doesn't have to be updated if this function
 *					is used
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::isVisibleLevel0 (int x0, int y0, int x1, int y1, DepthValue z) const
{
	DPVS_ASSERT (y0 >= 0 && x0 <= x1 && x1 <= m_xWidth);			// just some DPVS_ASSERTions
	DPVS_ASSERT (y0 >= 0 && y0 <= y1 && y1 <= m_yHeight);
	DPVS_ASSERT (!(z&0x80000000));								// Z is negative !

	const DepthValue* b2  = getBufferPtr (0,y0);
	for (int y = y0; y <= y1; y++, b2+=m_pitch)
	for (int x = x0; x <= x1; x++)
	{
		DPVS_ASSERT(!(b2[x]&0x80000000));						// negative Z
		if (z <= b2[x])											// point is visible							
			return true;
	}
	return false;												// not visible
}

/*****************************************************************************
 *
 * Function:		HZBuffer::isVisibleLevel1()
 *
 * Description:		Internal function for querying visibility at the 
 *					second lowest level
 *
 * Parameters:		x0		= inclusive left x coordinate
 *					y0		= inclusive top y coordinate
 *					x1		= inclusive right x coordinate
 *					y1		= inclusive bottom y coordinate
 *					z		= depth value
 *
 * Returns:			true if area is even partially visible, false otherwise
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::isVisibleLevel1 (int x0, int y0, int x1, int y1, DepthValue z) const
{
	//--------------------------------------------------------------------
	// If the area is small enough, skip level 1 testing and use level
	// 0 testing instead.
	//--------------------------------------------------------------------
	
	if ((x1-x0)<=2 && (y1-y0)<=2)
		return isVisibleLevel0(x0,y0,x1,y1,z);

	//--------------------------------------------------------------------
	// Perform testing on level 1 - if visible, then quickly check
	// all four child pixels.
	//--------------------------------------------------------------------

	DPVS_ASSERT (! (m_lModifiedMask & 1));

	int					p	= m_hBuffer[0].m_width;
	const DepthValue*		b	= m_hBuffer[0].m_data + p * (y0>>1);					
	const DepthValue*		b2  = getBufferPtr (0,y0&~1);

	for (int y = (y0 >> 1); y <= (y1>>1); y++, b += p, b2+=2*m_pitch)
	for (int x = (x0 >> 1); x <= (x1>>1); x++)
	if (z <= b[x])									// not covered.. recurse
	{
		CHECK_POINTER(b+x);
		int sx = x<<1;
		int sy = y<<1;

		if (sy >= y0)
		{
			if (sx >= x0	 && z <= b2[sx])		return true;
			if ((sx+1) <= x1 && z <= b2[sx+1])		return true;
		}
		
		if ((sy+1)<=y1)
		{
			if (sx >= x0	 && z <= b2[m_pitch+sx])	return true;
			if ((sx+1) <= x1 && z <= b2[m_pitch+sx+1])return true;
		}
	}

	return false;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::isVisibleRecursive()
 *
 * Description:		Queries visibility status of an axis-aligned 2D rectangle
 *					at specified depth buffer level
 *
 * Parameters:		level	= depth buffer level + 1
 *					x0		= inclusive left x coordinate 
 *					y0		= inclusive top y coordinate
 *					x1		= inclusive right x coordinate
 *					y1		= inclusive bottom y coordinate
 *					z		= depth value
 *
 * Returns:			true if area is even partially visible, false otherwise
 *
 * Notes:			All coordinates are in original scaling
 *
 *****************************************************************************/

bool HZBuffer::isVisibleRecursive (int level, int x0, int y0, int x1, int y1, DepthValue z) const
{
	DPVS_ASSERT(x0>=0 && y0>= 0 && x1>=x0 && y1>=y0);
	DPVS_ASSERT(level>=1 && level <= m_numLevels);

	//--------------------------------------------------------------------
	// Handle final level with separate code
	//--------------------------------------------------------------------

	if (level == 1)
		return isVisibleLevel1 (x0,y0,x1,y1,z);

	DPVS_ASSERT (! (m_lModifiedMask & (1<<(level))));			// an update hasn't been performed (!)

	const Level* lv = m_hBuffer + level-1;

	int				p	= lv->m_width;
	DepthValue*		b	= lv->m_data + p * (y0>>level);					

	DPVS_ASSERT((x1>>level) < lv->m_width);
	DPVS_ASSERT((y1>>level) < lv->m_height);

	for (int y = (y0 >> level); y <= (y1>>level); y++, b+= p)
	for (int x = (x0 >> level); x <= (x1>>level); x++)
	if (z <= b[x])													// not covered.. recurse
	{
		CHECK_POINTER(b+x);
		int dx0			= x << level;
		int dy0			= y << level;
		int dx1			= ((x+1) << level)-1;
		int dy1			= ((y+1) << level)-1;

		// DEBUG DEBUG OPTIMIZE: THESE CAN BE DONE WITH MIN2 AND MAX2 OPERATIONS
		if (dx0 < x0) dx0	= x0;
		if (dx1 > x1) dx1	= x1;
		if (dy0 < y0) dy0	= y0;
		if (dy1 > y1) dy1	= y1;
		if (isVisibleRecursive(level-1,dx0,dy0,dx1,dy1,z))
			return true;
	}

	return false;													// not visible
}

/*****************************************************************************
 *
 * Function:		HZBuffer::isVisible(int,int,int,int,DepthValue)
 *
 * Description:		Queries visibility status of an axis-aligned 2D rectangle
 *
 * Parameters:		x0	= inclusive left x coordinate
 *					y0	= inclusive top y coordinate
 *					x1  = exclusive right x coordinate
 *					y1  = exclusive bottom y coordinate
 *					z   = depth value
 *
 * Returns:			true if any part of rectangle is visible, false otherwise
 *
 * Notes:			The routine doesn't perform clipping of the x/y coordinates -
 *					the calling routine must take care of this.
 *
 *****************************************************************************/

DPVS_FORCE_INLINE bool HZBuffer::isVisible (int x0, int y0, int x1, int y1, DepthValue z) const
{
	DPVS_ASSERT (x0>=0 && x0 < x1 && x1 <= m_xWidth);		// what?!
	DPVS_ASSERT (y0>=0 && y0 < y1 && y1 <= m_yHeight);
	DPVS_ASSERT (!(z&0x80000000));							// Z is negative !

	//--------------------------------------------------------------------
	// Calculate width and height of the query region
	//--------------------------------------------------------------------

	int dx = x1-x0;										// width of query area in pixels
	int dy = y1-y0;										// height of query area in pixels

	//--------------------------------------------------------------------
	// Convert x1 and y1 into inclusive format as all the internal
	// functions operate with inclusive coordinates
	//--------------------------------------------------------------------

	x1--;													// convert into inclusive
	y1--;

	//--------------------------------------------------------------------
	// If the area is small enough, there's not much point in perfoming
	// recursive hierarchical depth buffer testing - just sample the
	// highest-resolution buffer directly. This has the advantage
	// that the hierarchical depth buffer doesn't need to be updated
	// for small-area queries (that often happen at the "end" of the
	// query sequence when farther objects are tested)
	//--------------------------------------------------------------------

	const int LIMIT = 32;								// value found by experimentation
	if (dx*dy <= LIMIT)										// if area is smaller than LIMIT pixels...
		return isVisibleLevel0(x0,y0,x1,y1,z);				// .. perform a level 0 testing (against the real depth buffer)

	//--------------------------------------------------------------------
	// Here perform a recursive query. Select starting level on approximate
	// area of the object. The starting level selected is ~log2(smaller of 
	// dx/dy)-1.
	//--------------------------------------------------------------------

	// DEBUG DEBUG OPTIMIZE: USE MIN2 AND MAX2 OPERATIONS
	int lv = getHighestSetBit(dx < dy ? dx : dy);				// get ~log2 of smaller dimension

	// DEBUG DEBUG OPTIMIZE: clamping can be done using a 32-entry (byte) LUT
	if (lv < 1) lv = 1;
	if (lv > m_numLevels) lv = m_numLevels;

	//--------------------------------------------------------------------
	// If any part of the hierarchical depth buffer has been modified, 
	// update the changed regions starting recursively from level 0.
	//--------------------------------------------------------------------

	const_cast<HZBuffer*>(this)->updateDepthBufferLevels (lv);	// update levels [0,lv]
	bool visible = isVisibleRecursive (lv,x0,y0,x1,y1,z);

	//--------------------------------------------------------------------
	// In debug build perform a consistency test by performing the same
	// query against the z-buffer itself -- the return value must be the
	// same !! (otherwise something is broken)
	//--------------------------------------------------------------------

#if defined(DPVS_PARANOID)
	if (visible != isVisibleLevel0(x0,y0,x1,y1,z))
	{
		selfTest(lv);
		selfTest(lv-1);
		DPVS_ASSERT(false);
	}
#endif

	return visible;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::isSilhouetteVisible()
 *
 * Description:		Queries visibility status of a convex silhouette that
 *					has been pre-scanconverted
 *
 * Parameters:		leftEdges	= left edge x values (inclusive)
 *					rightEdges	= right edge x values (exclusive)
 *					z			= silhouette depth value
 *					y0			= start Y scanline
 *					n			= number of scanlines
 *
 * Returns:			true if area is even partially visible, false otherwise
 *
 *****************************************************************************/

// DEBUG DEBUG DEBUG TODO: WRITE QUERY THAT USES THE HIERARCHICAL
// DATA (at least some levels of it)

DPVS_FORCE_INLINE bool HZBuffer::isSilhouetteVisible (const int* leftEdges, const int* rightEdges, DepthValue z, int y0, int n)
{
	DPVS_ASSERT(leftEdges && rightEdges);
	DPVS_ASSERT(y0 >= 0 && n >= 0);
	DPVS_ASSERT(!(z&0x80000000));			// negative query value

	const DepthValue* b2 = getBufferPtr (0,y0);

	for (int i = 0; i < n; i++, b2+= m_pitch)
	{
		DPVS_ASSERT((y0+i)<m_yHeight);											// reading past buffer
		DPVS_ASSERT(leftEdges[i] >= 0 && leftEdges[i] <= rightEdges[i]);		// invalid data
		DPVS_ASSERT(rightEdges[i] <= m_xWidth);
	
		int xEnd = rightEdges[i];
		for (int x = leftEdges[i]; x < xEnd; x++)
		if (z <= b2[x])									
			return true;
	}

	return false;
}

/*****************************************************************************
 *
 * Function:		HZBuffer::getFarDepth()
 *
 * Description:		Returns farthest depth value in the buffer
 *
 * Returns:			farthest depth value in the buffer
 *
 *****************************************************************************/

DPVS_FORCE_INLINE HZBuffer::DepthValue HZBuffer::getFarDepth (void) const
{
	if (m_lModifiedMask)
		const_cast<HZBuffer*>(this)->updateDepthBufferLevels (m_numLevels); 

	return m_farthest;									// global far value
}

/*****************************************************************************
 *
 * Function:		HZBuffer::selfTest()
 *
 * Description:		Performs consistency test for specified depth buffer
 *					level
 *
 * Parameters:		level = depth buffer level index
 *
 * Notes:			The level must've been updated (otherwise it will
 *					of course contain invalid values) before this
 *					test is called.
 *
 *****************************************************************************/

#if defined (DPVS_PARANOID)

inline void	HZBuffer::selfTest (int l) const
{
	DPVS_ASSERT(l>=0 && l<m_numLevels);

	// perform self-test of the HZ buffer for specified level

	const Level*		lv  = &m_hBuffer[l];							// get level pointer
	const DepthValue*	s	= (l == 0) ? m_buffer : (lv-1)->m_data;		// source buffer
	int					sp	= (l == 0) ? m_pitch : (lv-1)->m_width;		// source pitch in terms of Depth Value
	const DepthValue*	s2	= lv->m_data;								// source buffer2
	int					sp2	= lv->m_width;								// source pitch2 in terms of Depth Value
	int					h	= lv->m_height;								// height (in scanlines)
	int					w   = lv->m_width;

	for (int y = 0; y < h; y++)
	for (int x = 0; x < w; x++)
	{
		CHECK_POINTER(s2+y*sp2+x);

		DepthValue z2 = s2[y*sp2+x];

		DPVS_ASSERT( z2 >= s[(y*2+0)*sp + x*2+0]);
		DPVS_ASSERT( z2 >= s[(y*2+0)*sp + x*2+1]);
		DPVS_ASSERT( z2 >= s[(y*2+1)*sp + x*2+0]);
		DPVS_ASSERT( z2 >= s[(y*2+1)*sp + x*2+1]);
	}
}

#endif

//------------------------------------------------------------------------
// Here's the wrapper code
//------------------------------------------------------------------------

IHZBuffer::DepthValue 
		IHZBuffer::getFarDepth			(void) const													{ return m_ptr->getFarDepth(); }

IHZBuffer::IHZBuffer (const DepthValue* src, int log2xWidth, int log2yHeight, int p) 
	: m_ptr(new (MALLOC(sizeof(HZBuffer))) HZBuffer(src,log2xWidth,log2yHeight,p))
{
}
		IHZBuffer::~IHZBuffer			(void)															{ DELETE (m_ptr); }
void	IHZBuffer::setRegionModified	(int x0, int y0, int x1, int y1)								{ m_ptr->setRegionModified (x0,y0,x1,y1); }
bool	IHZBuffer::isVisible			(int x, int y, DepthValue z) const								{ return m_ptr->isVisible(x,y,z); }
bool	IHZBuffer::isVisible			(int x0, int y0, int x1, int y1, DepthValue z) const			{ return m_ptr->isVisible(x0,y0,x1,y1,z); }
bool	IHZBuffer::isSilhouetteVisible	(const int* leftEdges, const int* rightEdges, DepthValue z, int y0, int n) const {  return m_ptr->isSilhouetteVisible(leftEdges,rightEdges,z,y0,n);  }
//------------------------------------------------------------------------
