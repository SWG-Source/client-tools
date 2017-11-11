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
 * Description: 	X86 instruction set detection code
 *
 * $Archive: /dpvs/implementation/sources/dpvsX86.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 14.06.01 18:00 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsX86.hpp"

#if defined (DPVS_CPU_X86)

#include <cstring>

// returns current cycle counter
#if defined (DPVS_X86_ASSEMBLY)
double DPVS::X86::getCycleCount (void)
{
	// DEBUG DEBUG TODO: CHECK THAT RDTSC INSTRUCTION IS SUPPORTED (some
	// Cyrix processors may not support it?)

	volatile UINT32 a,b;

	DPVS_ASM
	{
		push		eax
		push		edx
		DPVS_EMIT	0x0f								// RDTSC
		DPVS_EMIT	0x31
		mov			[b],eax
		mov			[a],edx
		pop			edx
		pop			eax
	}

	return (a*(65536.0*65536.0)) + (double)b;
}
#endif // DPVS_X86_ASSEMBLY

//------------------------------------------------------------------------
// If we're compiling for the X-Box, we don't need to detect the
// processor..
//------------------------------------------------------------------------


#if !defined (DPVS_OS_XBOX)

namespace DPVS
{
	#if defined (DPVS_X86_ASSEMBLY)

	// test if processor supports the CPUID instruction
	static DPVS_FORCE_INLINE bool supportCPUID (void)
	{
		INT32 cpuidOkay = 0;
		__asm
		{
			push	ebx

			pushf

			pop		eax
			mov		ebx,eax
			xor		eax,0x00200000
			push	eax
			popf
			pushf
			pop		eax
			xor		eax,ebx
			je		cq_endex
			mov		cpuidOkay,0x00000001
		cq_endex:
			push	ebx
			popf
			pop		ebx
		}
		return (cpuidOkay==1);
	}

	// test if processor supports the CMOV instructions or conditional floating point instructions
	static DPVS_FORCE_INLINE bool supportCMOV (void)
	{
		if (!supportCPUID())
			return false;

		int	cpuidFeatures	= 0;

		__asm
		{
			pusha

			mov		eax,1
			DPVS_EMIT	0x0f
			DPVS_EMIT	0xa2
			mov		cpuidFeatures,edx

			popa
		}

		if(cpuidFeatures & (1<<15))
			return true;
		return false;
	}

	// test if processor supports MMX instructions
	static DPVS_FORCE_INLINE bool supportMMX (void)
	{
		if (!supportCPUID())
			return false;

		int	cpuidFeatures	= 0;

		__asm
		{
			pusha

			mov		eax,1
			DPVS_EMIT	0x0f
			DPVS_EMIT	0xa2
			mov		cpuidFeatures,edx

			popa
		}

		if(cpuidFeatures & (1<<23))
			return true;
		
		return false;
	}

	// test if processor supports AMD 3DNow instructions
	static DPVS_FORCE_INLINE bool support3DNow (void)
	{
		if (!supportCPUID())
			return false;

		char	idString[13];
		int		k63d =0;

		idString[0] = 0;
		// see if we have k6-3d installed
		__asm 
		{
			pusha
			mov		eax,0
			DPVS_EMIT	0x0f
			DPVS_EMIT	0xa2
			mov		dword ptr[idString],ebx
			mov		dword ptr[idString+4],edx
			mov		dword ptr[idString+8],ecx
			mov		byte ptr[idString+12],0
			mov		eax, 0x80000001
			// cpuid:
			DPVS_EMIT	0xf
			DPVS_EMIT	0xa2
			test	edx, 0x80000000					// if highest bit set, then we got AMD
			jnz		yes_amd_k6_3d
			mov		[k63d],0
			jmp		done
	yes_amd_k6_3d:
			mov		[k63d],1
	done:
			popa
		};

		if(strcmp(idString,"AuthenticAMD"))
			return false;

		return (k63d == 1) ? true : false;
	}

	// test if processor supports Athlon extended instructions
	static DPVS_FORCE_INLINE bool supportAthlon (void)
	{
		if (!support3DNow())						// .. not a 3DNow.. so it cannot be Athlon
			return false;

		int athlon = 0;

		int mask = (1<<30) | (1<<22);				// 30 = extended 3DNow, 22 = extended MMX
		
		__asm 
		{
			pusha
			mov		eax, 0x80000001					// query cpu information
			DPVS_EMIT	0xf
			DPVS_EMIT	0xa2
			and		edx, [mask]						
			cmp		edx, [mask]						// .. if both are set.. then we have athlon..
			je		yes_athlon
			mov		dword ptr [athlon],0						
			jmp		athlon_done	
	yes_athlon:
			mov		dword ptr [athlon],1
	athlon_done:
			popa
		};

		return (athlon == 1) ? true: false;
	}

	/******************************************************************************
	 *
	 * Function:		supportSSE()
	 *
	 * Description:		Returns boolean value indicating whether the processor/OS
	 *					supports SSE instructions natively (i.e. not emulated)
	 *
	 ******************************************************************************/

#if defined (DPVS_BUILD_MSC) || defined (DPVS_BUILD_BC)

/*	static volatile bool beenThere = false;
	static void __cdecl contextTest (void*);						// prototype

	static void __cdecl contextTest (void *)
	{
		static const float foo = 1.943493f;
		__asm
		{
			lea		eax,foo
			DPVS_EMIT	0xf3					// movss	xmm0,[eax]
			DPVS_EMIT	0x0f
			DPVS_EMIT	0x10
			DPVS_EMIT	0x00
		}

		beenThere = true;
		_endthread();
	}
*/
	static inline bool supportSSE (void)
	{

		if (!supportCPUID())
			return false;
		
		int retval	= false;

		//--------------------------------------------------------------------
		// Detect if SSE instructions are available using the CPUID instruction.
		//--------------------------------------------------------------------
		__asm 
		{
			pushad
			xor		eax,eax
			cpuid
			cmp		ebx,0x756e6547
			jne		bad
			cmp		edx,0x49656e69
			jne		bad
			cmp		ecx,0x6c65746e
			jne		bad
			mov		eax,1
			cpuid
			and		edx,0x03808001		// XMM/FXSR/MMX/CMOV/FPU present
			cmp		edx,0x03808001
			jne		bad
			and		eax,0xfff
			cmp		eax,0x672
			jge		good
	bad:	popad
			mov		eax,0
			jmp		ojee
	good:	popad
			mov		eax,1
	ojee:	mov		[retval],eax
		}

		if (!retval)
			return false;

		//--------------------------------------------------------------------
		// Test arbitrary SSE instruction using a __try __except construction
		// (the __try __except construction is available only on MSVC/IntelC/Borland)
		//--------------------------------------------------------------------
 
		__try
		{
			__asm
			{
				DPVS_EMIT 0x0f			// xorps	xmm0,xmm0
				DPVS_EMIT 0x57
				DPVS_EMIT 0xc0
			}
		}

		__except (1)		// EXCEPTION_EXECUTE_HANDLER
		{
			retval = 0;
		}

		//--------------------------------------------------------------------
		// Test whether the operating system is capable of
		// preserving Katmai registers in a context switch.
		//--------------------------------------------------------------------

		// This code is commented out since we don't need to perform this test
		// anymore (the test took about a second or so..)

/*
		if (retval)
		{
			beenThere	= false;

			float orig = 64.0f;

			// init xmm0 to orig
			__asm
			{
				lea		eax,orig
				DPVS_EMIT	0xf3			// movss	xmm0,[eax]
				DPVS_EMIT	0x0f
				DPVS_EMIT	0x10
				DPVS_EMIT	0x00
			}

			if ((int)_beginthread (contextTest, 0, 0) != -1)
			{
				int cnt = 0;

				while(!beenThere && cnt<100000000) 
				{
					cnt++;						// this cnt<100000000 check just in case the thread hangs (???)
				};	

				float foo = 10.0f;

				__asm
				{
					lea		eax,foo
					DPVS_EMIT	0xf3		//  movss	[eax],xmm0
					DPVS_EMIT	0x0f
					DPVS_EMIT	0x11
					DPVS_EMIT	0x00
				}

				//------------------------------------------------------------
				// if foo != orig, the OS didn't preserve the xmm
				// registers in the context switch --> return false
				// to indicate crappy OS.
				//------------------------------------------------------------

				if(foo != orig)
					retval = 0;
				else
					retval = 1;
			} else
				retval = 0;						// failed to start thread
		}
*/

		return (retval!=0) ? true : false;
	}

#else
	static DPVS_FORCE_INLINE bool supportSSE (void)
	{
		return false;
	}
#endif

	//--------------------------------------------------------------------
	// Detects all special instruction sets supported by the current 
	// processor and returns mask containing this information.
	//--------------------------------------------------------------------

	static DPVS_FORCE_INLINE unsigned int detectX86Instructions (void)
	{
		unsigned int mask = 0;	

		if (supportMMX())		mask |= X86::S_MMX;     
		if (supportCMOV())		mask |= X86::S_CMOV;    
		if (support3DNow())		mask |= X86::S_3DNOW;   
		if (supportAthlon())	mask |= X86::S_ATHLON;  
		if (supportSSE())		mask |= X86::S_SSE;     

		return mask;
	}

	#else

	static DPVS_FORCE_INLINE  unsigned int detectX86Instructions (void)
	{
		return 0;							// no special ops supported...
	}

	#endif	// DPVS_X86_ASSEMBLY

	const unsigned int DPVS::X86::s_instructionSetMask  = detectX86Instructions();	 // detect instructions at program initialization

} // DPVS

#endif // !DPVS_OS_XBOX

#endif // DPVS_CPU_X86

//--------------------------------------------------------------------
