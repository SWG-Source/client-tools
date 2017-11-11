#ifndef __DPVSSPACEMANAGER_HPP
#define __DPVSSPACEMANAGER_HPP
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
 * Description: 	Class for handling conversions between spaces
 *
 * $Archive: /dpvs/implementation/include/dpvsSpaceManager.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 11.06.01 15:51 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSRECTANGLE_HPP)
#	include "dpvsRectangle.hpp"
#endif
#if !defined (__DPVSVECTOR_HPP)
#	include "dpvsVector.hpp"
#endif

namespace DPVS
{

	class SpaceManager
	{
	public:
		static DPVS_FORCE_INLINE void rasterToOcclusion(IntRectangle& d,const FloatRectangle& s, const Vector2& scale)
		{
			DPVS_ASSERT(s.getSpace() == SPACE_RASTER);

			d.x0 = Math::intFloor(s.x0*scale.x);
			d.y0 = Math::intFloor(s.y0*scale.y);
			d.x1 = Math::intCeil (s.x1*scale.x);
			d.y1 = Math::intCeil (s.y1*scale.y);

			d.setSpace(SPACE_OCCLUSION);
		}

/*		static void occlusionToBlock(IntRectangle& br, const IntRectangle &r)
		{
			DPVS_ASSERT(r.getSpace() == SPACE_OCCLUSION);

			br.x0 = (r.x0)	 >> 3;
			br.y0 = (r.y0)	 >> 3;
			br.x1 = (r.x1+7) >> 3;
			br.y1 = (r.y1+7) >> 3;

			br.setSpace(SPACE_OCCLUSION_BLOCK);
		}
*/
		static DPVS_FORCE_INLINE void occlusionToBucket(IntRectangle& br, const IntRectangle &r)
		{
			DPVS_ASSERT(r.getSpace() == SPACE_OCCLUSION);

			br.x0 = (r.x0)						>> DPVS_TILE_BITS;
			br.y0 = (r.y0)						>> 6;
			br.x1 = (r.x1+DPVS_TILE_WIDTH-1)	>> DPVS_TILE_BITS;
			br.y1 = (r.y1+64-1)					>> 6;

			br.setSpace(SPACE_OCCLUSION_BUCKET);
		}

/*		static void floatToInt(IntRectangle& i, const FloatRectangle &f)
		{
			i.x0 = Math::intFloor(f.x0);
			i.y0 = Math::intFloor(f.y0);
			i.x1 = Math::intCeil (f.x1);
			i.y1 = Math::intCeil (f.y1);
			i.copySpace(f);
		}
		*/
	};

} // DPVS


//------------------------------------------------------------------------
#endif //__DPVSSPACEMANAGER_HPP
