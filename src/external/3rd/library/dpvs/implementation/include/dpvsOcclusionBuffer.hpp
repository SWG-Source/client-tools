#ifndef __DPVSOCCLUSIONBUFFER_HPP
#define __DPVSOCCLUSIONBUFFER_HPP
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
 * Desc:	Occlusion Buffer
 *
 * $Archive: /dpvs/implementation/include/dpvsOcclusionBuffer.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 4/22/02 5:28p $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSSILHOUETTE_HPP)
#	include "dpvsSilhouette.hpp"
#endif
#if !defined (__DPVSMATH_HPP)
#	include "dpvsMath.hpp"
#endif
#if !defined (__DPVSBITMATH_HPP)
#	include "dpvsBitMath.hpp"
#endif
#if !defined (__DPVSRECTANGLE_HPP)
#	include "dpvsRectangle.hpp"
#endif
#if !defined (__DPVSQWORD_HPP)
#	include "dpvsQWord.hpp"
#endif
#if !defined (__DPVSOCCLUSIONBUFFERZGRADIENT_HPP)
#	include "dpvsOcclusionBuffer_ZGradient.hpp"
#endif

#define OCC_USE_REFLECTION_TARGETS

namespace DPVS
{ 
class Surface;
class IHZBuffer;
class ImpCamera;

const unsigned int FULL_MASK = 0xFFFFFFFF;

/******************************************************************************
 *
 * Class:			DPVS::OcclusionBuffer
 *
 * Description:		
 *
 * Notes:			
 *
 ******************************************************************************/

class OcclusionBuffer
{
public:
					OcclusionBuffer				(int screenWidth, int screenHeight, float sx=1.f,float sy=1.f);
					~OcclusionBuffer			(void);

	int				getLastWriteCost			(void) const						{ return m_lastWriteCost; }

	void			clear						(void);
	void			enableContributionCulling	(bool v)							{ m_contributionCullingEnabled = v; }
	void			frameEnd					(void);
	void			frameStart					(void);
	float			getClosingDepth				(void) const;
	bool		 	getPreviousVisiblePoint		(Vector3& vpt) const				{ vpt = m_visiblePoint; return m_visiblePointValid; }
	void			getQueryResolution			(int &x,int &y) const				{ x=m_blockClipRectangle.width(); y=m_blockClipRectangle.height(); }
	void			getResolution				(IntRectangle& br, Vector2& scale)	{ br=m_bucketRectangle; scale=m_subSampling; }
	bool			isEmpty						(void) const						{ return m_empty; }
	bool			isFull						(void) const						{ return m_bucketRectangle.area()==m_totalBucketsFilled; }
	void			minimizeMemoryUsage			(void);
	void			setCoverageThreshold		(float t)							{ t*=64.0f; m_contributionThreshold = Math::intChop(t); }
	bool			setDirtyRectangle			(const IntRectangle&);
	void			setScissor					(const ImpCamera*);
	void			setStaticCoverageBuffer		(const UINT8* buf,int w,int h,int pitch);
	void			setStaticZBuffer			(const float* buf,int w,int h,int pitch,float farValue);
	void			setZConstant				(ZGradient::Mode mode, float z)				{ m_ZGradient.setConstant(mode,z); }
	void			setZGradient				(ZGradient::Mode mode, const Vector3& a, const Vector3& b, const Vector3& c, float minz, float maxz) { m_ZGradient.setGradient(mode,a,b,c,minz,maxz); }
	void			setZGradients				(ZGradient::Mode mode, const Vector3* planes, int nPlanes, float minz, float maxz) { m_ZGradient.setGradients(mode,planes,nPlanes,minz,maxz); }

	//test
	bool			isPointOccluded				(int x,int y,float z) const;
	bool			isRectangleOccluded			(const IntRectangle &r, float zval);
	bool			test						(EdgeSilhouette& es);

	//write
	bool			write						(EdgeSilhouette& es, BitVector& contributedToBucket, BitVector& fullyCoveredTheBucket);

	//stencil
	void			initStencil					(ImpCamera* c);					// initialize stencil plane for SCISSOR area
	void			clearAccordingToStencil		(void);
	bool			stencilOp					(EdgeSilhouette& es);

	//debug
	float*			grabZBuffer					(int&w,int&h);
	Surface*		visualizeBBuffer			(void);
	Surface*		visualizeCBuffer			(void);
	Surface*		visualizeSBuffer			(void);
	Surface*		visualizeZBuffer			(void);

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	void			setIndexBufferIndex			(UINT32 index) { m_indexBufferIndex = index; }
#endif

//------------------------------------------------------------------------
// Private stuff
//------------------------------------------------------------------------
private:

						OcclusionBuffer	(const OcclusionBuffer&);	// not allowed
	OcclusionBuffer&	operator=		(const OcclusionBuffer&);	// not allowed	

	void			updateDepthBuffer		(void);
	
	class Bucket;
	class ParallelMath;
	class Cache;
	friend class Cache;
	friend class Bucket;

	enum Mode
	{
		TEST	= 0,						// testing
		WRITE	= 1,						// writing
		STENCIL	= 2							// stencil op
	};


	enum
	{
		MAX_EDGECOUNT	= 65535,			// maximum supported (16 bit indices!)
		BUCKET_YBITS	= 6,
		BUCKET_XBITS	= DPVS_TILE_BITS,	//BW 5
		BLOCK_BITS		= 3,
		BUCKET_W		= (1<<BUCKET_XBITS),
		BUCKET_H		= (1<<BUCKET_YBITS),	//QWORD bitcount, for code clarity
		BUCKET_MASK		= (BUCKET_W-1),
		BLOCK_W			= 8,				//for code clarity
		BLOCK_H			= 8					//for code clarity
		// removed to SRBYTE_BITS etc. stuff (see the bitsToBytes() etc. functions below)..
	};

				OcclusionBuffer					(void);

	static DPVS_FORCE_INLINE int	bitsToBytes		(int bits) { DPVS_ASSERT(bits>=0); return (int)((unsigned int)(bits)>>3); }
	static DPVS_FORCE_INLINE int	bitsToDWords	(int bits) { DPVS_ASSERT(bits>=0); return (int)((unsigned int)(bits)>>5); }
	static DPVS_FORCE_INLINE int	bitsToQWords	(int bits) { DPVS_ASSERT(bits>=0); return (int)((unsigned int)(bits)>>6); }
	static DPVS_FORCE_INLINE UINT8	sampleMinimum	(const UINT8* buf, const IntRectangle &rect, int w);
	static DPVS_FORCE_INLINE float	sampleMaximum	(const float* buf, const IntRectangle &rect, int w);

	void		setScissor						(const FloatRectangle&);
	bool		setEdges						(EdgeSilhouette& es, bool updateDirtyRectangle);
	void		setEdge							(int fx0,int fy0,int fx1,int fy1,bool leftEdge);
												
											
	void		testBucketStability				(void) const;
	void		initializeBuffers				(void);
	void		clearCoverageBuffer				(QWORD *cbuf);
	void		padCoverageBuffer				(QWORD *cbuf);
	void		clearZBuffer					(void);
											
	void		createBlockMasks				(void);
	bool		coverageBufferOccludes			(int blockx,int blocky,IntRectangle& r);
	void		clearBucketArea					(const Bucket &bucket, QWORD *cbuf);
	void		clearPotentiallyAffectedBuckets	(void);

	void		setVisiblePoint					(const QWORD& block,float z,int bucketx,int buckety,int blockx, int blocky);

	void		cacheToCoverageBufferREPLACE	(int bx,int by,int bmin,int bmax);
	void		cacheToCoverageBufferFUSION		(int bx,int by,int bmin,int bmax);
	void		cacheToCoverageBufferFAST		(int bx,int by,int bmax);
	bool		cacheToCoverageBufferTEST		(int bx,int by,int bmin,int bmax,bool pendingClear);

	//-----------------------------------------------------
	//-----------------------------------------------------

	class Edge
	{
	public:

						Edge	(void);
		void			setup	(void);

		INT32			m_fx0;
		INT32			m_fx1;					// with subpixels
		INT32			m_fy0;
		INT32			m_fy1;					// with subpixels
		INT32			m_xEdge;
		INT32			m_xStep;

		UINT32			m_initialized		: 1;	// have xEdge and xStep been calculated
		UINT32			m_leftEdge			: 1;
		UINT32			m_singleBucket		: 1;
	};

	class BucketEdge
	{
	public:
		UINT8		bx0;					// bucket coordinates (inclusive)
		UINT8		bx1;
		UINT8		by0;
		UINT8		by1;
	};

	void	appendEdge				(int fx0,int fx1,int fy0,int fy1,bool leftEdge);
	void	assignEdgesToBuckets	(void);
	bool	rasterizeEdges			(Bucket& bucket, int& left,int &right);
	void	rasterizeLeftEdge		(int y0, int y1, int xEdge, int xStep);
	void	rasterizeRightEdge		(int y0, int y1, int xEdge, int xStep);
	void	rasterizeTestEdge		(int y0, int y1, int xEdge, int xStep);

	//-----------------------------------------------------
	//-----------------------------------------------------

	class EdgeManager
	{
	public:
		struct Entry
		{
					Entry	(void) : m_next(0), m_edgeNum(0)									{};
					Entry	(const Entry& s) : m_next(s.m_next), m_edgeNum(s.m_edgeNum)		{};
			void	set		(int edgeNum)													{ m_edgeNum=edgeNum; }

			// DEBUG DEBUG TODO: use INT16 instead??
			int				m_next		: 16;	// next index
			int				m_edgeNum	: 16;	// index to edge buffer
		};

		Array<Entry>	m_entries;				// array of entries
		int				m_used;					// number of used entries

						EdgeManager				(void)				{ clear(); }
						~EdgeManager			(void)				{}
		void			ensureSpaceFor			(int n)				{ int required  = m_used + n; int	allocated = m_entries.getSize(); if(allocated < required) resize(required+32); }
		void			clear					(void)				{ m_used = 0; }
		int				addEntry				(int edgeNum)		{ m_entries[m_used].set(edgeNum); return m_used++; }
		void			link					(int from,int to)	{ m_entries[from].m_next = to; }
	private:
		void			resize					(int newSize);
	};

	//-----------------------------------------------------
	//-----------------------------------------------------

	//Bucket is BUCKET_WIDTH * 64 (QWORD) pixels
	//- - - - - - - - - - - - - - - - - - - - - - - - - - -
	// the non-inline code can be found from dpvsOcclusionBuffer_Render.cpp

	class Bucket
	{
	private:
					Bucket					(const Bucket&);		// not allowed
		Bucket&		operator=				(const Bucket&);		// not allowed	
		
		//=================================================
		// the following data members are modified pretty
		// often (maybe even hundreds of times per frame).
		//=================================================

		INT16			m_firstEdge;				// max 65536 edges
		INT16			m_reflectionTarget;			// where to reflect attempted write? [INDEX]

		UINT8			m_fullBlockCount;			// easier to detect full bucket from this [64]
		UINT8			m_bx;						// bucket x - [128]
		UINT8			m_by;						// bucket y - [64]
		UINT8			m_pendingClear		: 1;	// there is a pending clear
		UINT8			m_padded			: 1;	// padded - cannot be cleared easily
		UINT8			m_paddedTwice		: 1;	// padded vertically and horizontally

		//=================================================
		// following data is mostly read-only (i.e. modified
		// at most once a frame)
		//=================================================

		UINT32			m_maxDepth;					// 32 bits
		int				m_coverageBufferOffset;		// 8192x8192 (27 bits)


	public:
						Bucket					(void);
		void			clear					(void)					{ m_firstEdge=-1; }
		void			clearState				(void)					{ clearReflectionTarget(); m_fullBlockCount = 0; m_maxDepth=0; }
		void			clearReflectionTarget	(void)					{ m_reflectionTarget = -1; }

		int				findNumber				(int w) const			{ return m_by*w + m_bx; }

		int				getBucketX				(void) const			{ return m_bx; }
		int				getBucketY				(void) const			{ return m_by; }
		int				getCoverageBufferOffset	(void) const			{ return m_coverageBufferOffset; }
		UINT32			getMaxDepth				(void) const			{ DPVS_ASSERT(isFull()); return m_maxDepth; }
		int				getReflectionTarget		(void) const			{ return m_reflectionTarget; }
						
		bool			hasPendingClear			(void) const			{ return m_pendingClear==1; }
		bool			hasReflectionTarget		(void) const			{ return m_reflectionTarget!=-1; }
		bool			isFull					(void) const			{ return m_fullBlockCount==(OcclusionBuffer::BUCKET_W*OcclusionBuffer::BUCKET_H)/64; }
		bool			isPadded				(void) const			{ return m_padded==1; }
		bool			isPaddedTwice			(void) const			{ return m_paddedTwice==1; }

		void			setCoverageBufferOffset	(int offset)			{ m_coverageBufferOffset = offset; }
		void			setMaxDepth				(UINT32 d)				{ m_maxDepth = d; }
		void			setNumber				(int x,int y)			{ m_bx = (UINT8)x; m_by = (UINT8)y; }
		void			setPendingClear			(bool v)				{ m_pendingClear = v; }
		void			setPadded				(bool once, bool twice)	{ m_padded = once; m_paddedTwice = twice; }
		void			setReflectionTarget		(int rt)				{m_reflectionTarget = (INT16)rt; }

		int				getFullBlockCount		(void) const			{ return m_fullBlockCount; }
		void			setFullBlockCount		(int n)					{ m_fullBlockCount  = (UINT8)n; }
		void			addFullBlockCount		(int n)					{ m_fullBlockCount = (UINT8)(m_fullBlockCount+n); DPVS_ASSERT(m_fullBlockCount<=(OcclusionBuffer::BUCKET_W*OcclusionBuffer::BUCKET_H)/64);}
		int				getFirstEdge			(void) const			{ return m_firstEdge; }
		void			setFirstEdge			(int n)					{ m_firstEdge = (INT16)n; }
	};


	class BlockBuffer;

	//=================================================
	// following data is mostly read-only (i.e. modified
	// at most once a frame)
	//=================================================

	EdgeManager					m_edgeManager;

	DynamicArray<Edge>			m_edges;			// edge info
	DynamicArray<BucketEdge>	m_edges2;			// bucket coordinates
					
	IntRectangle		m_clipRectangle;			// scissoring rectangle in internal resolution
	IntRectangle		m_memoryRectangle;			// allocated rectangle (padded) in internal resolution
	IntRectangle		m_bucketRectangle;			// bucket counts
	IntRectangle		m_loresRectangle;
	IntRectangle		m_blockClipRectangle;
	IntRectangle		m_blockMemoryRectangle;
	IntRectangle		m_silhouetteRectangle;		// for lores silhouette query
						
	UINT8*				m_bufferAlloc;				// zbuffer,cbuffer and blockbuffer are allocated using this!
	UINT32*				m_ZBuffer;
	QWORD*				m_CBuffer;
	QWORD*				m_stencilBuffer;
	UINT8*				m_blockBufferF;				// full
	UINT8*				m_blockBufferC;				// contribution
	UINT8*				m_blockBufferP;				// partial
	BlockBuffer*		m_blockBuffer;
	IHZBuffer*			m_HZBuffer;					// pointer to HDEB
	Cache*				m_cache;					// pointer to cache

	Array<Bucket>		m_bucket;
	Array<ByteLoc>		m_justFilledBuckets;
	Array<ByteLoc>		m_blockCoordinates;

	UINT32*				m_staticZBuffer;			// pointer to static Z buffer
	QWORD*				m_staticCoverageBuffer;		// pointer to static coverage buffer
	UINT8*				m_staticFullBlocks;
	UINT8*				m_staticPartialBlocks;

	int					m_dummy;
	Vector2				m_subSampling;				// sub-sample factor
	Vector2				m_ooSubSampling;			// reciprocal of sub-sample factors

	int					m_verticalBucketCrossings;
	int					m_coverageBufferBlockQWPitch;
	int					m_coverageBufferBlockRowQWPitch;
	int					m_coverageBufferBucketQWPitch;
	int					m_coverageBufferBucketRowQWPitch;
	const ImpCamera*	m_scissorCamera;			// pointer to camera responsible for current scissor
	EdgeSilhouette*		m_scissorSilhouette;		// this used by clearing???

	QWORD				m_blockMasks0[8*8];			// top left			(DEBUG DEBUG this could be static data, i.e. s_blockMasks)
	QWORD				m_blockMasks1[8*8];			// bottom right		(DEBUG DEBUG this could be static data, i.e. s_blockMasks)


	//=================================================
	// the following data members are modified pretty
	// often (maybe even hundreds of times per frame).
	//=================================================

	FloatRectangle		m_floatScissor;
	IntRectangle		m_intScissor;
	IntRectangle		m_int16Scissor;
	IntRectangle		m_blockScissor;

	ZGradient			m_ZGradient;				// current gradient setup 
	IntRectangle		m_dirtyRectangle;			// minimum/maximum (x,y) used
	BitVector			m_dummyContributed;			// used internally when clearing the occ buffer (DEBUG DEBUG could we get rid of this?)
	IntRectangle		m_dirtyBucketRectangle;

	QWORD				m_dirtyScanlines;			// mask of dirty scanlines

	int					m_edgeCount;				// number of edges in edge buffer
	int					m_totalBlocksFilled;
	int					m_totalBucketsFilled;
	int					m_previousBlocksFilled;
	int					m_previousBucketsFilled;
	int					m_ZChangeCount;				// number of pixels in m_ZChangeList
	int					m_contributionThreshold;
	Mode				m_mode;						// current mode

	Vector3				m_visiblePoint;				// visible point of last query
	bool				m_visiblePointValid;		// is the visible point valid?

	bool				m_useStencil;
	bool				m_empty;
	bool				m_dirtyRectangleSet;
	bool				m_contributionCullingEnabled;

	int					m_lastWriteCost;			// cost of previously rasterized object

#if defined (DPVS_OCCLUSIONBUFFER_INDEXBUFFER)	
	UINT32*				m_indexBuffer;
	UINT32				m_indexBufferIndex;
#endif


};

} //DPVS






using namespace DPVS;

//------------------------------------------------------------------------
#endif //__DPVSOCCLUSIONBUFFER_HPP
