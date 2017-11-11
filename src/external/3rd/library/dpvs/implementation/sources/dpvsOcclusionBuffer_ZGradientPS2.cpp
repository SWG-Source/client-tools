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
 * Description: 	Occlusion buffer Z Gradient code
 *
 * $Archive: /dpvs/implementation/sources/dpvsOcclusionBuffer_ZGradientPS2.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 27.08.02 14:18 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsOcclusionBuffer_ZGradient.hpp"

using namespace DPVS;

//------------------------------------------------------------------------
// PS2 implementations
//-----------------------------------------------------------------------

#if defined (DPVS_PS2_VU_ASSEMBLY)
//PS2-BEGIN	

void ZGradient::interpolateListPS2_8	(float* dst, const ByteLoc* src, int N)
{
	/*for (int i = 0; i < N; i++)
	{
		int x = src[i].getX();
		int y = src[i].getY();

		DPVS_ASSERT( x >= 0 && y >= 0);
		register float z = m_zmax;
		for (int i = 0; i < 8; i++)
		{
			register float zz = 
				m_base[i] + m_xstep[i] * (x-m_xref[i]) + m_ystep[i] * (y-m_yref[i]);

			if ( zz < z)
				z = zz;
		}
		if (z < m_zmin) 
			z = m_zmin;
		DPVS_ASSERT ( z <= m_zmax);

		dst[i] = z;
	}*/
	DPVS_ASSERT( is128Aligned(m_base) );
	DPVS_ASSERT( is128Aligned(m_xstep) );
	DPVS_ASSERT( is128Aligned(m_ystep) );
	DPVS_ASSERT( is128Aligned(m_xref) );
	DPVS_ASSERT( is128Aligned(m_yref) );
	DPVS_ASSERT( is128Aligned(m_zmax4) );

	asm __volatile__ (
	"	lqc2		vf20, 0(%3)"
	"\n	lqc2		vf21, 0(%4)"
	"\n	lqc2		vf22, 0(%5)"
	"\n	lqc2		vf23, 0(%6)"
	"\n	lqc2		vf24, 0(%7)"
	"\n	lqc2		vf25, 0(%8)"
	"\n	mfc1		$8, %9"
	"\n	qmtc2		$8, vf26"
		/*	m_zmax4		= vf20
			m_base		= vf21
			m_xstep		= vf22
			m_ystep		= vf23
			m_xref		= vf24
			m_yref		= vf25 
			m_zmin		= vf26.x */
	"\n	lqc2		vf27, 0x10(%4)"
	"\n	lqc2		vf28, 0x10(%5)"
	"\n	lqc2		vf29, 0x10(%6)"
	"\n	lqc2		vf30, 0x10(%7)"
	"\n	lqc2		vf31, 0x10(%8)"
		/*	m_base2		= vf27
			m_xstep2	= vf28
			m_ystep2	= vf29
			m_xref2		= vf30
			m_yref2		= vf31 */

"\n	ipolPS2_8_loop:"
	"\n	beq			%2, $0, ipolPS2_8_done"
	"\n	nop"

		/*	int x = src[i].getX();
			int y = src[i].getY(); */
	"\n	lw			$9, 0(%1)"
	"\n	andi		$8, $9, 0x7FFF"
	"\n	srl			$9, $9, 16"
	"\n	andi		$9, $9, 0xFFFF"
		/*	x = $8
			y = $9 */

	"\n	qmtc2		$8, vf1"
	"\n	qmtc2		$9, vf2"
	"\n	vmr32.xyzw	vf5, vf1"
	"\n	vmove.x		vf5, vf2"
	"\n	vmr32.xyzw	vf6, vf5"
	"\n	vmr32.xyzw	vf7, vf6"
	"\n	vmr32.xyzw	vf8, vf7"
		/*	vf5 = {x,0,0,y}, [wzyx]
			vf6 = {y,x,0,0}
			vf7 = {0,y,x,0}
			vf8 = {0,0,y,x} */

	"\n	vmove.x		vf1, vf8"
	"\n	vmove.y		vf1, vf7"
	"\n	vmove.z		vf1, vf6"
	"\n	vmove.w		vf1, vf5"
	"\n	vmove.x		vf2, vf5"
	"\n	vmove.y		vf2, vf8"
	"\n	vmove.z		vf2, vf7"
	"\n	vmove.w		vf2, vf6"
	"\n	vitof0.xyzw	vf10, vf1"
	"\n	vitof0.xyzw	vf11, vf2"
		/*	vf10 = {xf,xf,xf,xf}
			vf11 = {yf,yf,yf,yf} */

		/*	vec4 a = x - m_xref;
			vec4 b = y - m_yref;
			vec4 zz = m_base + m_xstep * a + m_ystep * b; */
	"\n	vsub.xyzw	vf5, vf10, vf24"
	"\n	vsub.xyzw	vf6, vf11, vf25"
	"\n	vmulaw.xyzw	ACC, vf21, vf0"
	"\n	vmadda.xyzw	ACC, vf22, vf5"
	"\n	vmadd.xyzw	vf1, vf23, vf6"

	"\n	vsub.xyzw	vf5, vf10, vf30"
	"\n	vsub.xyzw	vf6, vf11, vf31"
	"\n	vmulaw.xyzw	ACC, vf27, vf0"
	"\n	vmadda.xyzw	ACC, vf28, vf5"
	"\n	vmadd.xyzw	vf3, vf29, vf6"
	"\n	vmini.xyzw	vf1, vf1, vf3"
	"\n	vmini.xyzw	vf1, vf1, vf20"
		/*	vf1 = min(zz,zmax) */

	"\n	vmr32.xyzw	vf2, vf1"
	"\n	vmr32.xyzw	vf3, vf2"
	"\n	vmr32.xyzw	vf4, vf3"
	"\n	vmini.x		vf1, vf1, vf2"
	"\n	vmini.x		vf1, vf1, vf3"
	"\n	vmini.x		vf1, vf1, vf4"
		/* vf1.x = min(vf1.x,vf1.y,vf1.z,vf1.w) */

	"\n	vmax.x		vf2, vf1, vf26"
		/* vf2.x = max( min(zz,zmax), zmin ) */

	"\n	qmfc2		$8, vf2"
	"\n	sw			$8, 0(%0)"

	"\n	addi		%0, %0, 4"
	"\n	addi		%1, %1, 4"
	"\n	addi		%2, %2, -1"
	"\n	j			ipolPS2_8_loop"
	"\n	nop"

"\n	ipolPS2_8_done:"
	 : : "r"(dst), "r"(src), "r"(N), "r"(m_zmax4), "r"(m_base), 
		"r"(m_xstep), "r"(m_ystep), "r"(m_xref), "r"(m_yref), "f"(m_zmin) : "$8", "$9" );
}

void ZGradient::interpolateListPS2_4	(float* dst, const ByteLoc* src, int N)
{
	/*for (int i = 0; i < N; i++)
	{
		int x = src[i].getX();
		int y = src[i].getY();

		DPVS_ASSERT( x >= 0 && y >= 0);
		register float z = m_zmax;
		for (int i = 0; i < 4; i++)
		{
			register float zz = m_base[i] + m_xstep[i] * (x-m_xref[i]) + m_ystep[i] * (y-m_yref[i]);
			if ( zz < z)
				z = zz;
		}
		if (z < m_zmin) 
			z = m_zmin;
		DPVS_ASSERT ( z <= m_zmax);

		dst[i] = z;
	}*/

	DPVS_ASSERT( is128Aligned(m_base) );
	DPVS_ASSERT( is128Aligned(m_xstep) );
	DPVS_ASSERT( is128Aligned(m_ystep) );
	DPVS_ASSERT( is128Aligned(m_xref) );
	DPVS_ASSERT( is128Aligned(m_yref) );
	DPVS_ASSERT( is128Aligned(m_zmax4) );

	asm __volatile__ (
	"	lqc2		vf20, 0(%3)"
	"\n	lqc2		vf21, 0(%4)"
	"\n	lqc2		vf22, 0(%5)"
	"\n	lqc2		vf23, 0(%6)"
	"\n	lqc2		vf24, 0(%7)"
	"\n	lqc2		vf25, 0(%8)"
	"\n	mfc1		$8, %9"
	"\n	qmtc2		$8, vf26"
		/*	m_zmax4 = vf20
			m_base	= vf21
			m_xstep	= vf22
			m_ystep	= vf23
			m_xref	= vf24
			m_yref	= vf25 
			m_zmin	= vf26.x */

"\n	ipolPS2_4_loop:"
	"\n	beq			%2, $0, ipolPS2_4_done"
	"\n	nop"

		/*	int x = src[i].getX();
			int y = src[i].getY(); */
	"\n	lw			$9, 0(%1)"
	"\n	andi		$8, $9, 0x7FFF"
	"\n	srl			$9, $9, 16"
	"\n	andi		$9, $9, 0xFFFF"
		/*	x = $8
			y = $9 */

	"\n	qmtc2		$8, vf1"
	"\n	qmtc2		$9, vf2"
	"\n	vmr32.xyzw	vf5, vf1"
	"\n	vmove.x		vf5, vf2"
	"\n	vmr32.xyzw	vf6, vf5"
	"\n	vmr32.xyzw	vf7, vf6"
	"\n	vmr32.xyzw	vf8, vf7"
		/*	vf5 = {x,0,0,y}, [wzyx]
			vf6 = {y,x,0,0}
			vf7 = {0,y,x,0}
			vf8 = {0,0,y,x} */

	"\n	vmove.x		vf1, vf8"
	"\n	vmove.y		vf1, vf7"
	"\n	vmove.z		vf1, vf6"
	"\n	vmove.w		vf1, vf5"
	"\n	vmove.x		vf2, vf5"
	"\n	vmove.y		vf2, vf8"
	"\n	vmove.z		vf2, vf7"
	"\n	vmove.w		vf2, vf6"
	"\n	vitof0.xyzw	vf10, vf1"
	"\n	vitof0.xyzw	vf11, vf2"
		/*	vf10 = {xf,xf,xf,xf}
			vf11 = {yf,yf,yf,yf} */

		/*	vec4 a = x - m_xref;
			vec4 b = y - m_yref;
			vec4 zz = m_base + m_xstep * a + m_ystep * b; */
	"\n	vsub.xyzw	vf5, vf10, vf24"
	"\n	vsub.xyzw	vf6, vf11, vf25"
	"\n	vmulaw.xyzw	ACC, vf21, vf0"
	"\n	vmadda.xyzw	ACC, vf22, vf5"
	"\n	vmadd.xyzw	vf3, vf23, vf6"
	"\n	vmini.xyzw	vf1, vf3, vf20"
		/*	vf1 = min(zz,zmax) */

	"\n	vmr32.xyzw	vf2, vf1"
	"\n	vmr32.xyzw	vf3, vf2"
	"\n	vmr32.xyzw	vf4, vf3"
	"\n	vmini.x		vf1, vf1, vf2"
	"\n	vmini.x		vf1, vf1, vf3"
	"\n	vmini.x		vf1, vf1, vf4"
		/* vf1.x = min(vf1.x,vf1.y,vf1.z,vf1.w) */

	"\n	vmax.x		vf2, vf1, vf26"
		/* vf2.x = max( min(zz,zmax), zmin ) */

	"\n	qmfc2		$8, vf2"
	"\n	sw			$8, 0(%0)"

	"\n	addi		%0, %0, 4"
	"\n	addi		%1, %1, 4"
	"\n	addi		%2, %2, -1"
	"\n	j			ipolPS2_4_loop"
	"\n	nop"

"\n ipolPS2_4_done:"
	 : : "r"(dst), "r"(src), "r"(N), "r"(m_zmax4), "r"(m_base), 
		"r"(m_xstep), "r"(m_ystep), "r"(m_xref), "r"(m_yref), "f"(m_zmin) : "$8", "$9" );
}


//PS2-END
#endif // DPVS_PS2_VU_ASSEMBLY
