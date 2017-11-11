#ifndef __DPVSX86_HPP
#define __DPVSX86_HPP
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
 * Desc:	X86-specific stuff
 *
 * $Archive: /dpvs/implementation/include/dpvsX86.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 18:01 $
 * $Date: 2003/01/22 $
 * 
 *****************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#if defined (DPVS_CPU_X86)

namespace DPVS
{
/******************************************************************************
 *
 * Class:			DPVS::X86
 *
 * Description:		X86-specific functionality
 *
 * Notes:			This class is available only when compiled with DPVS_CPU_X86
 *
 ******************************************************************************/
/******************************************************************************
 *
 * Class:			DPVS::X86::FPUControl
 *
 * Description:		Class for fast and simple FPU mode modification (rounding/precision)
 *
 ******************************************************************************/

class X86
{
public:
	enum InstructionSet
	{
		S_MMX		= (1<<0),						// support MMX instructions?
		S_CMOV		= (1<<1),						// support CMOV instruction?
		S_3DNOW		= (1<<2),						// support AMD 3DNow instructions?
		S_ATHLON	= (1<<3),						// support Athlon-specific instructions?
		S_SSE		= (1<<4)						// support SSE instructions?
	};

	static DPVS_FORCE_INLINE unsigned int	getInstructionSets  (void);
	static DPVS_FORCE_INLINE void			prefetch			(const void* mem);
	static double							getCycleCount		(void);

#if defined (DPVS_X86_ASSEMBLY)
	class FPUControl
	{
	public:
		enum e_precision
		{
			PRECISION_24 = 0,				// 24-bit precision
			PRECISION_53 = 512,				// 53-bit precision
			PRECISION_64 = 512|256			// 64-bit precision
		};

		enum e_rounding
		{
			ROUND_NEAR = 0,					// round to near
			ROUND_DOWN = 1024,				// round down
			ROUND_UP   = 2048,				// round up
			ROUND_CHOP = 1024|2048			// truncate (i.e. chop)
		};
 
		inline FPUControl (e_precision p)
		{
			UINT32	tmp;
			DPVS_ASM
			{
				fwait
				mov		ecx,this
				mov		ebx,p
				fstcw	[ecx] FPUControl.oldMode				// store control word to oldMode
				mov		eax,[ecx] FPUControl.oldMode			// load it to eax
				mov		edx,eax									// take copy
				and		eax,~(256|512)							// mask out certain bits
				or		eax,ebx									// or with precision control value
				sub		edx,eax									// did it change?
				jz		skip									// .. if not, skip

				mov		tmp,eax									// .. change control word
				fldcw	tmp
	skip:
				mov	[ecx] FPUControl.chg,edx					// mark the changed status
			}
		}

		inline FPUControl (e_rounding r)
		{
			UINT32	tmp;
			DPVS_ASM
			{
				fwait
				mov		ecx,this
				mov		ebx,r
				fstcw	[ecx] FPUControl.oldMode				// store control word to oldMode
				mov		eax,[ecx] FPUControl.oldMode			// load it to eax
				mov		edx,eax									// take copy
				and		eax,~(1024|2048)						// mask out certain bits
				or		eax,ebx									// or with precision control value
				sub		edx,eax									// did it change?
				jz		skip									// .. if not, skip

				mov		tmp,eax									// .. change control word
				fldcw	tmp
	skip:
				mov	[ecx] FPUControl.chg,edx					// mark the changed status
			}
		}

		inline FPUControl (e_precision p, e_rounding r)
		{
			UINT32	tmp;
			DPVS_ASM
			{
				fwait
				mov		ecx,this
				mov		ebx,r
				or		ebx,p
				fstcw	[ecx] FPUControl.oldMode				// store control word to oldMode
				mov		eax,[ecx] FPUControl.oldMode			// load it to eax
				mov		edx,eax									// take copy
				and		eax,~(256|512|1024|2048)				// mask out certain bits
				or		eax,ebx									// or with precision control value
				sub		edx,eax									// did it change?
				jz		skip									// .. if not, skip

				mov		tmp,eax									// .. change control word
				fldcw	tmp
	skip:
				mov	[ecx] FPUControl.chg,edx					// mark the changed status
			}

		}

		inline ~FPUControl ()
		{
			DPVS_ASM
			{
				mov		ecx,this
				mov		eax,dword ptr [ecx] FPUControl.chg
				cmp		eax,0									// did we change the value?
				je		skip									// nope... skip now...
				fwait
				fldcw	[ecx] FPUControl.oldMode;
	skip:
			}
		}

	private:

		UINT32	oldMode;										// store old FPU mode here
		UINT32	chg;											// if zero, value didn't change (speedup)
	};

#endif // DPVS_X86_ASSEMBLY

private:

	#if !defined (DPVS_OS_XBOX)
		static const unsigned int s_instructionSetMask;		// instruction set mask
	#endif // !defined(DPVS_OS_XBOX)
};


//------------------------------------------------------------------------
// Define some macros if the compiler does not support conditional move
// instruction
//------------------------------------------------------------------------

#if defined (DPVS_X86_ASSEMBLY)
#	define	FCMOVB(r)	 DPVS_ASM DPVS_EMIT 0xda  DPVS_ASM DPVS_EMIT (0xc0|(r&7))
#	define	FCMOVNB(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xc0|(r&7))
#	define	FCMOVE(r)	 DPVS_ASM DPVS_EMIT 0xda  DPVS_ASM DPVS_EMIT (0xc8|(r&7))
#	define	FCMOVNE(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xc8|(r&7))
#	define	FCMOVBE(r)	 DPVS_ASM DPVS_EMIT 0xda  DPVS_ASM DPVS_EMIT (0xd0|(r&7))
#	define	FCMOVNBE(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xd0|(r&7))
#	define	FCMOVU(r)	 DPVS_ASM DPVS_EMIT 0xda  DPVS_ASM DPVS_EMIT (0xd8|(r&7))
#	define	FCMOVNU(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xd8|(r&7))
#	define	FUCOMI(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xe8|(r&7))
#	define	FUCOMIP(r)	 DPVS_ASM DPVS_EMIT 0xdf  DPVS_ASM DPVS_EMIT (0xe8|(r&7))
#	define	FCOMI(r)	 DPVS_ASM DPVS_EMIT 0xdb  DPVS_ASM DPVS_EMIT (0xf0|(r&7))
#	define	FCOMIP(r)	 DPVS_ASM DPVS_EMIT 0xdf  DPVS_ASM DPVS_EMIT (0xf0|(r&7))
#	define	CMOVO		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x40
#	define	CMOVNO		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x41
#	define	CMOVB		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x42
#	define	CMOVNB		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x43
#	define	CMOVZ		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x44
#	define	CMOVNZ		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x45
#	define	CMOVBE		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x46
#	define	CMOVNBE		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x47
#	define	CMOVS		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x48
#	define	CMOVNS		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x49
#	define	CMOVP		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4a
#	define	CMOVNP		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4b
#	define	CMOVL		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4c
#	define	CMOVNL		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4d
#	define	CMOVLE		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4e
#	define	CMOVNLE		 DPVS_ASM DPVS_EMIT 0x0f  DPVS_ASM DPVS_EMIT 0x4f
#endif

//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------

DPVS_FORCE_INLINE unsigned int X86::getInstructionSets (void)					
{ 
#if defined (DPVS_OS_XBOX)								// when we have an X-box build we know the masks..
	return X86::S_MMX | X86::S_CMOV | X86::S_SSE;		// .. by having this inline we can avoid generating useless C code..
#else
	return s_instructionSetMask; 
#endif
}

#if defined (DPVS_X86_ASSEMBLY)


// NOTE THAT THE PREFETCH MUST BE MADE TO A MEMORY AREA WHERE THERE IS A READ ACCESS!!
/*DPVS_FORCE_INLINE void	X86::prefetch (const void* mem)		
{
	DPVS_ASSERT(mem);

	DPVS_ASM
	{
		mov		eax,[mem]								// get address
		test	eax,DWORD PTR [eax]						// read memory from address..
	}
}

*/
DPVS_FORCE_INLINE void X86::prefetch (const void* mem)		
{ 
	if (getInstructionSets()&S_SSE) 
	{ 
		DPVS_ASM
		{
			mov eax,[mem]						
			DPVS_EMIT 0x0f			// prefetcht0 [eax]
			DPVS_EMIT 0x18
			DPVS_EMIT 0x08
		}
	} 
}

#else

DPVS_FORCE_INLINE void		X86::prefetch			(const void*)		{}
DPVS_FORCE_INLINE double	X86::getCycleCount		(void)				{ return 0.0; }

#endif // DPVS_X86_ASSEMBLY

} // DPVS

#endif // DPVS_CPU_X86

//------------------------------------------------------------------------
#endif // __DPVSX86_HPP
