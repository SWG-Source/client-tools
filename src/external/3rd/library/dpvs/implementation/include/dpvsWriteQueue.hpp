#ifndef __DPVSWRITEQUEUE_HPP
#define __DPVSWRITEQUEUE_HPP
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
 *
 * Desc:	Write queue for OcclusionBuffer depth complexity
 *			reduction
 *
 * $Archive: /dpvs/implementation/include/dpvsWriteQueue.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 6:16p $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSARRAY_HPP)
#	include "dpvsArray.hpp"
#endif
#if !defined (__DPVSBITMATH_HPP)
#	include "dpvsBitMath.hpp"
#endif
#if !defined (__DPVSSILHOUETTE_HPP)
#	include "dpvsSilhouette.hpp" 
#endif
#if !defined (__DPVSRECTANGLE_HPP)
#	include "dpvsRectangle.hpp" // to get rectangle
#endif
#if !defined (__DPVSQWORD_HPP)
#	include "dpvsQWord.hpp"	
#endif
#if !defined (__DPVSRANGE_HPP)
#	include "dpvsRange.hpp"	
#endif

namespace DPVS
{
class OcclusionBuffer;
class ImpObject;
class ImpCamera;

/******************************************************************************
 *
 * Class:			DPVS::WriteQueue
 *
 * Description:		
 *
 ******************************************************************************/

class WriteQueue;
class WriteQueue
{
public:
				 WriteQueue				(OcclusionBuffer *buf);
				~WriteQueue				(void);

	void		clear					(ImpCamera *c);
	void		evaluateBenefits		(void);
	int			freeEntriesUsing		(ImpCamera* c);
	void		forceFlush				(const FloatRectangle& rect);

	void		appendOccluder			(void);
	bool		isPointOccluded			(const Vector3&);
	bool		isObjectOccluded		(void);
	bool		isSilhouetteOccluded	(const Vector3* vloc, int cnt,float cost);

	// immediate mode
	bool		isPointOccluded_IM		(const Vector3&);
	bool		isRectangleOccluded_IM	(const FloatRectangle& r, float depth);

	struct SortEntry;

	enum
	{
		FLASH_RECTANGLE_Z   = 4
	};

private:
				WriteQueue				(const WriteQueue&);	// not allowed
	WriteQueue&	operator=				(const WriteQueue&);	// not allowed

	class Entry;
	class Bucket;

	enum
	{
		BUCKET_WBITS		= DPVS_TILE_BITS,		//BW (5, 32)
		BUCKET_HBITS		= 6,
		BUCKET_WIDTH		= (1<<BUCKET_WBITS),
		BUCKET_HEIGHT		= (1<<BUCKET_HBITS),
		WRITE_QUEUE_SIZE	= 64,
		FLASH_RECTANGLE_X   = 4,
		FLASH_RECTANGLE_Y   = 4,
		FLASH_RECTANGLE_FRAMES = FLASH_RECTANGLE_X*FLASH_RECTANGLE_Y*FLASH_RECTANGLE_Z
	};

	void						testConsistency			(void) const;
	bool						flush					(const IntRectangle& br,const IntRectangle& r,float z,bool exactOverlap);				//flush pending writes inside given rectangle having zvalue <= z
	void						sortFlushed				(SortEntry*, int);
	void						assignToBuckets			(Entry *e);															//NOTES: clips EXCLUSIVE input rectangle
	DPVS_FORCE_INLINE Entry&	allocateEntry			(void);																//Allocate an entry and mark as used (MAX 64)
	void						getBucketRectangle		(IntRectangle& br, const IntRectangle& r);
	void						setBenefitToRectangle	(const IntRectangle& rect,float cost);
	DPVS_FORCE_INLINE void		outOfCapacity			(float);
	void						clearMaskFromBuckets	(const IntRectangle& rect, const QWORD mask);
//	float						getClosingDepth			(void) const;

	//=================================================
	// Internal classes
	//=================================================

	struct Occluder
	{
		DPVS_FORCE_INLINE Occluder	(void) : object(0), benefit(0.f), next(0)			{};
		DPVS_FORCE_INLINE Occluder	(const Occluder& s) : object(s.object), benefit(s.benefit), next(s.next)		{};

		ImpObject*		object;		// pointer to object
		float			benefit;	// benefit
		int				next;		// next index
	};

	//-------------------------------------------------------------------------
	// OccluderManager class. Maintains liked list of contributing occluders
	//-------------------------------------------------------------------------

	class OccluderManager
	{
	public:
		DPVS_FORCE_INLINE 			OccluderManager			(void)				{ clear(); }
		DPVS_FORCE_INLINE			~OccluderManager		(void)				{ }
		DPVS_FORCE_INLINE void		ensureSpaceFor			(int n)				{ int required  = m_used + n; int	allocated = m_occluders.getSize(); if(allocated < required) resize(((required*11)>>3)+32); }
		DPVS_FORCE_INLINE void		clear					(void)				{ m_used = 0; }
		DPVS_FORCE_INLINE int		addEntry				(ImpObject* o)		{ m_occluders[m_used].object=o; m_occluders[m_used].benefit=0.f; return m_used++; }
		DPVS_FORCE_INLINE void		link					(int from,int to)	{ m_occluders[from].next = to; }

		Array<Occluder>				m_occluders;
	private:
		void						resize					(int newSize);
		int							m_used;
	};

	//-------------------------------------------------------------------------
	// WriteQueue entry class
	//-------------------------------------------------------------------------

	class Entry
	{
	public:
								Entry	();

		QWORD					mask;						// ???

#if defined (DPVS_DEBUG)
		UINT32					debugCode;					// debug code
#endif

		ImpObject*				object;						// pointer to object
		ImpCamera*				camera;						// pointer to camera
		IntRectangle			rectangle;					// ???
		Range<float>			depth;						// depth range
//		float					maxDepth;					// maximum depth value
#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
		UINT32					indexBufferIndex;			// used if we draw an index buffer (debug only)
#endif
		IntRectangle			bucketRectangle;			// ????

		float					getSortDepth (void) const	{ return depth.getMax(); }
	private:
								Entry		(const Entry&);	// not allowed
		Entry&					operator=	(const Entry&);	// not allowed
	};

	//-------------------------------------------------------------------------
	// WriteQueue bucket class (16 bytes)
	//-------------------------------------------------------------------------

	class Bucket
	{
	private:
					Bucket			(const Bucket&);		// not allowed
		Bucket&		operator=		(const Bucket&);		// not allowed

		QWORD		m_entryMask;
		int			m_lastIndex;
		int			m_occluderCount;

	public:
									Bucket				(void);
		DPVS_FORCE_INLINE void		clear				(const QWORD& m)					{ m_entryMask &= ~m; }
		DPVS_FORCE_INLINE void		clearList			(void)								{ m_occluderCount=0; m_lastIndex=-1; }
		DPVS_FORCE_INLINE bool		empty				(void) const						{ return m_entryMask.empty(); }
		int							getEntries			(int *list,const QWORD& clearMask) const;
		DPVS_FORCE_INLINE int		getLastIndex		(void) const						{ return m_lastIndex; }
		DPVS_FORCE_INLINE QWORD		getMask				(void) const						{ return m_entryMask; }
		DPVS_FORCE_INLINE void		set					(const QWORD& m)					{ m_entryMask |=  m;  }
																		
		void						appendObject		(OccluderManager& bm, ImpObject* o);
		int							giveBenefits		(const Array<Occluder>&);
	};

	//-------------------------------------------------------------------------
	// WriteQueue high-speed entry allocator
	//-------------------------------------------------------------------------

	class Manager
	{
	private:
		QWORD							m_entryMask;
	public:
		DPVS_FORCE_INLINE				Manager			(void) : m_entryMask()			{ }
		DPVS_FORCE_INLINE QWORD			getMask			(void) const					{ return m_entryMask;	}
		DPVS_FORCE_INLINE bool			isFull			(void) const					{ return m_entryMask.full(); }
		DPVS_FORCE_INLINE bool			isEmpty			(void) const					{ return m_entryMask.empty(); }
		QWORD							allocate		(void);							
		DPVS_FORCE_INLINE void			free			(const QWORD& mask)				{ m_entryMask &= ~mask; };

	};

	//-------------------------------------------------------------------------
	// Data that is mostly read-only
	//-------------------------------------------------------------------------

	OcclusionBuffer*				m_occlusionBuffer;				// pointer to occlusion buffer
	Guard<Bucket>					m_buckets;						// pointer to buckets
	Bucket*							m_bucketAlloc;					// if buckets were allocated from heap (ptr here)
	FloatRectangle					m_flashRectangle;				// rectangle used for flashing
	float							m_flashBudget;					// how much write cost can be used for flashing?
	int								m_flashZBucket;					// 
	float							m_ooScreenHeight;
	unsigned char					m_flashOrder[FLASH_RECTANGLE_FRAMES];// flash order table
	int								m_frame;						// current frame counter (used for flashRectangle frame selection)
	IntRectangle					m_bucketRectangle;				// this queried from occlusion buffer
	Vector2							m_subSampling;					// sub-sampling factors
	bool							m_overFlow;						// is there an overflow going on right now?
	//-------------------------------------------------------------------------
	// Data that is modified pretty often (multiple times per frame)
	//-------------------------------------------------------------------------

	OccluderManager					m_occluderManager;
	BitVector						m_contributed;					// did object contribute to buckets
	BitVector						m_fullyCovered;					// did object ALONE fully cover buckets
	EdgeSilhouette					m_testSilhouette;				// temp place for every silhouette
	EdgeSilhouette					m_writeSilhouette;				// temp place for every silhouette

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	UINT32							m_indexBufferIndex;				// used if index buffers are visualized (debug only)
#endif

	Manager							m_manager;						// alloc/free
	Entry							m_entries[WRITE_QUEUE_SIZE];	// entries
};

} // namespace DPVS


//------------------------------------------------------------------------
#endif //__DPVSWRITEQUEUE_HPP
