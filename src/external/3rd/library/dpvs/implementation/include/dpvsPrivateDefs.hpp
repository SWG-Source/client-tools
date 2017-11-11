#ifndef __DPVSPRIVATEDEFS_HPP
#define __DPVSPRIVATEDEFS_HPP
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
 * Description: 	Private global definitions
 *
 * $Archive: /dpvs/implementation/include/dpvsPrivateDefs.hpp $
 * $Author: wili $ 
 * $Revision: #12 $
 * $Modtime: 12.11.02 18:37 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

//------------------------------------------------------------------------
// Uncomment this to make the paranoid build (debug build only)
//------------------------------------------------------------------------

//#define DPVS_PARANOID

//------------------------------------------------------------------------
// Forward-declare some commonly used data structures.
//------------------------------------------------------------------------

#define DPVS_OVERRIDE_VECTOR_TYPES

namespace DPVS
{
	class Vector2i;									
	class Vector3i;									
	class Vector2;								
	class Vector3;								
	class Vector3d;								
	class Vector4;								
	class Vector4d;								
	class Matrix4x3;
	class Matrix4x4;
	class Matrix4x4d;

	template <class Key, class Value> class Hash;
	template <class T> class Range;

} // namespace DPVS

//------------------------------------------------------------------------
// Include the public interface definition header file
//------------------------------------------------------------------------

#if !defined (__DPVSDEFS_HPP)
#	include "dpvsDefs.hpp"
#endif

#include <cstddef>	

									// we need to include this (not all compilers have size_t as built-in)

//------------------------------------------------------------------------
// Begin the DPVS namespace
//------------------------------------------------------------------------

namespace DPVS 
{

//------------------------------------------------------------------------
// We need to use lower-case "null" instead of the NULL macro because
// of some problems in CodeWarrior PS2 compiler.
//------------------------------------------------------------------------

#define null 0

//------------------------------------------------------------------------
// Automatically detect operating system. Note that if you have a 64-bit
// platform, you should define the DPVS_UINT64_DEFINED macro and then
// typedef UINT64 to match the unsigned 64-bit data type. On 32-bit 
// platforms there's automatic emulation for the 64-bit integers (in
// class QWord)
//------------------------------------------------------------------------

#undef DPVS_VFT_IN_END

#if defined (_WIN32) || defined (WIN32)						// Automatically detect Win32
#	undef DPVS_OS_WIN32
#	define DPVS_OS_WIN32
#endif

#if defined (DPVS_OS_GAMECUBE_EMULATOR)
//NGC-BEGIN
#	undef DPVS_OS_WIN32
#	define DPVS_LITTLE_ENDIAN
#	define DPVS_CPU_GAMECUBE_EMULATOR
#	define DPVS_CPU_NAME "GameCube Emulator (PC)"
//NGC-END
#endif

#if defined (DPVS_OS_GAMECUBE)
//NGC-BEGIN
#	undef DPVS_LITTLE_ENDIAN
#	define DPVS_CPU_GAMECUBE
#	define DPVS_CPU_NAME "Nintendo GameCube HW2"
//#	define DPVS_UINT64_DEFINED								// 64-bit integer support
//	typedef signed long long   INT64;
//	typedef unsigned long long UINT64;
//NGC-END
#endif

#if defined (__SGI__)										// Silicon Graphics
#	define DPVS_CPU_MIPS
#	define DPVS_CPU_NAME "MIPS"
#	undef DPVS_LITTLE_ENDIAN								// big-endian architecture
#endif

#if defined (macintosh)										// Detect Macintosh (CodeWarrior defines this)
#	undef DPVS_OS_MAC
#	define DPVS_OS_MAC
#endif

#if defined (DPVS_OS_WIN32)
#	define DPVS_CPU_X86										// x86 series CPU
#	define DPVS_LITTLE_ENDIAN								// x86 processors are little-endian
#elif defined (DPVS_OS_MAC)									// Apple Macintosh
#	define DPVS_CPU_PPC
#	define DPVS_CPU_NAME		"PowerPC"
#	define DPVS_PPC_ASSEMBLY								// support x86 inline assembly
#	define DPVS_DOUBLE_MATH_H								// math.h supports only double ops!
#elif defined (__HPUX_SOURCE)
#	define DPVS_CPU_HPPA
#	define DPVS_OS_HPUX
#	define DPVS_CPU_NAME		"HP-PA"
//#	define DPVS_UINT64_DEFINED								// DEBUG DEBUG WE SHOULD CHECK WHETHER IT'S A 64 or 32-BIT COMPILATION
//typedef unsigned long UINT64;								// UINT64 data type
//typedef signed long INT64;								// INT64 data type
#	pragma pack 8
#elif defined (DPVS_OS_LINUX)								// DPVS_OS_LINUX must be in the makefile
// assume DPVS_CPU_X86, DPVS_LITTLE_ENDIAN and DPVS_CPU_NAME have been defined in the makefile
#elif defined (DPVS_PS2)									// PS2/CodeWarrior build
//PS2-BEGIN
#	undef DPVS_LITTLE_ENDIAN
#	define DPVS_LITTLE_ENDIAN								// little endian
#	define DPVS_CPU_PS2
#	define DPVS_OS_PS2
#	define DPVS_CPU_NAME		"PlayStation2"			
#	define DPVS_EMULATED_DOUBLE								// doubles are emulated!
#	define DPVS_UINT64_DEFINED								// 64-bit integer support
	typedef signed long INT64;					
	typedef unsigned long UINT64;
//PS2-END
#elif defined (DPVS_OS_XBOX)								// Microsoft X-Box
#	define DPVS_CPU_X86
#	define DPVS_CPU_XBOX
#	define DPVS_LITTLE_ENDIAN
#	define DPVS_CPU_NAME		"XBox"
#elif defined (__IBMVISUALAGE__)							// Use this to detect AIX.. blah!
#	define DPVS_CPU_RS6000
#	define DPVS_OS_AIX
#	define DPVS_CPU_NAME		"RS6000"
#	define DPVS_UINT64_DEFINED								// 64-bit integer support
	typedef signed long    INT64;
	typedef unsigned long UINT64;
#endif

#if defined (DPVS_CPU_X86)	&& !defined (DPVS_CPU_NAME)								
#	define DPVS_CPU_NAME "X86"
#endif

#if !defined (DPVS_CPU_NAME)
#	error Unrecognized CPU name (define DPVS_CPU_NAME somewhere)	// cannot recognize CPU
#endif

//------------------------------------------------------------------------
// Attempt to automatically recognized different compilers
//------------------------------------------------------------------------

#if defined (_MSC_VER) && !defined(__MWERKS__)				// for some reason CodeWarrior also defines _MSC_VER..
#	define DPVS_BUILD_MSC									// Microsoft Visual C++ (any version)
#	if (_MSC_VER >= 1200)									
#		define DPVS_BUILD_MSC6								// Microsoft Visual C++ 6.0 or higher (also DPVS_BUILD_MSC defined)
#	endif
#	pragma warning(disable:4710)
#	if defined (_MSC_FULL_VER) 
#		if (_MSC_FULL_VER > 12008804)
#			if defined (DPVS_CPU_X86)	
#				define DPVS_BUILD_MSC_PPACK					// MSVC6 Processor Pack supported
#				define DPVS_ALIGN_VECTORS					// perform vector alignment	
#			endif
#		endif
#	endif // _MSC_FULL_VER
#	define DPVS_X86_RETURN_EAX								// even inline functions return parameters correctly in eax
#	define DPVS_COMPILER "Microsoft Visual C++"
#	if defined (_MT)										// multi-threaded build
#		undef DPVS_MT
#		define DPVS_MT
#	endif
#endif

#if defined (__BORLANDC__)									// Borland C++
#	define DPVS_BUILD_BC
#	define DPVS_COMPILER "Borland C++"
#endif

#if defined (__MWERKS__)									// MetroWerks CodeWarrior
#	define DPVS_BUILD_CW
#	define DPVS_COMPILER "MetroWerks CodeWarrior"
#	undef DPVS_LITTLE_ENDIAN								// first undef (in case it was already defined)
#	if __option (little_endian)								// detect endianess using CodeWarrior-specific instruction..
#		define DPVS_LITTLE_ENDIAN								
#	else
#		undef DPVS_LITTLE_ENDIAN
#	endif
#endif

#if defined (__GNUC__)										// GNU C++
#	undef DPVS_BUILD_GCC									// .. in case it was defined..
#	define DPVS_BUILD_GCC									
#	define DPVS_COMPILER "Gnu C++"
#	define DPVS_VFT_IN_END									// VFT is stored in end of structure
#endif

#if defined (__ICL)											// Intel C++ (note that DPVS_BUILD_MSC is also defined for this compiler)
#	define DPVS_BUILD_INTELC								
#	undef DPVS_COMPILER
#	define DPVS_COMPILER "Intel C++"
#	define DPVS_RESTRICT restrict							// support "restrict" keyword
#	pragma warning (disable:171)							
#	pragma warning (disable:383)							
#	pragma warning (disable:869)							
#	pragma warning (disable:981)							
#	define DPVS_UINT64_DEFINED								// DEBUG DEBUG WE SHOULD CHECK WHETHER IT'S A 64 or 32-BIT COMPILATION
	typedef unsigned __int64 UINT64;						// UINT64 data type
	typedef signed __int64 INT64;							// INT64 data type
#endif

#if defined (__HPUX_SOURCE)									// aCC (HPUX-HPPA)
#	define DPVS_BUILD_ACC
#	define DPVS_COMPILER "aCC"
#endif

#if defined (__IBMVISUALAGE__)								// IBM Visual Age
#	define DPVS_BUILD_IVA
#	define DPVS_COMPILER "IBM VisualAge"
#endif

#if defined (__KCC)
#	define DPVS_BUILD_KCC									// Kai C++
#	define DPVS_COMPILER "KAI C++"
#endif

#if defined (__SGI__)										// SGI MIPS-PRO compiler
#	define DPVS_BUILD_SGI
#	define DPVS_COMPILER "SGI Mips Pro"
#endif

#if !defined (DPVS_COMPILER)
#	error The macro DPVS_COMPILER must be defined (cannot recognize compiler)
#endif

//------------------------------------------------------------------------
// DPVS_SELECT_ENDIAN macro. Note that the OS definitions above must
// define or undefine the DPVS_LITTLE_ENDIAN macro properly.
//------------------------------------------------------------------------

#ifdef DPVS_LITTLE_ENDIAN
#	define DPVS_SELECT_ENDIAN(little_expression,big_expression) little_expression
#else
#	define DPVS_SELECT_ENDIAN(little_expression,big_expression) big_expression
#endif

//------------------------------------------------------------------------
// Null statement
//------------------------------------------------------------------------

#define DPVS_NULL_STATEMENT (static_cast<void>(0))			// no statement

//------------------------------------------------------------------------
// Debug Build (react automatically to the _DEBUG macro)
//------------------------------------------------------------------------

#if defined(_DEBUG) || defined (DEBUG)						// debug build
#	undef DPVS_DEBUG
#	define DPVS_DEBUG
#endif // _DEBUG

#if !defined (DPVS_DEBUG)									// no paranoid-release build
#	undef DPVS_PARANOID
#endif // !DPVS_DEBUG

//------------------------------------------------------------------------
// Profiling
//------------------------------------------------------------------------

#define DPVS_ENABLE_PROFILE								// define this to enable internal profiling
#if defined (DPVS_ENABLE_PROFILE)
#	define DPVS_PROFILE(X) (X)
#else
#	define DPVS_PROFILE(X) 
#endif

//------------------------------------------------------------------------
// Some compiler-specific definitions
//------------------------------------------------------------------------

#undef DPVS_SUPPORT_ATTRIBUTE

#if defined (DPVS_BUILD_MSC)
#	if defined (DPVS_CPU_X86)								// running on X86????
#		define DPVS_X86_ASSEMBLY							// support x86 inline assembly
#		define DPVS_EMIT				_emit				// assembly emit instruction
#	endif
#	if defined (DPVS_BUILD_MSC6)
#		define DPVS_FORCE_INLINE	__forceinline			// this works only on MSVC6..
#	endif
#	if defined (DPVS_CPU_GAMECUBE_EMULATOR)
//NGC-BEGIN
#		pragma warning (disable:4710)						// inline blah blah
//NGC-END
#	endif
#	pragma warning(disable:4514)							// unreferenced inline function has been removed
#endif // DPVS_BUILD_MSC

#if defined (DPVS_BUILD_BC)
#	define DPVS_EMIT				db						// assembly emit instruction
#endif

#if defined (DPVS_BUILD_CW)
#	define DPVS_EMIT				EMIT					// assembly emit instruction
#	if defined (DPVS_CPU_X86)						
#		define DPVS_X86_ASSEMBLY							// support x86 inline assembly
#	endif
#	if defined (DPVS_CPU_PPC)								// support PPC inline assembly
#		define DPVS_PPC_ASSEMBLY
#	endif
#	if defined (DPVS_CPU_GAMECUBE)							// support Gekko assembly
//NGC-BEGIN
#		define DPVS_GAMECUBE_ASSEMBLY
//NGC-END
#	endif
#	if !defined (DPVS_DEBUG)								// release build pragmas
#		pragma always_inline on
//#		pragma ARM_conform on 
#		pragma float_constants on 
#		pragma inline_depth(1024)
#		pragma opt_common_subs on
#		pragma opt_dead_assignments on
#		pragma opt_dead_code on
#		pragma opt_lifetimes on
#		pragma opt_loop_invariants on
#		pragma opt_propagation on
#		pragma opt_strength_reduction on
#		pragma opt_unroll_loops off
#		pragma opt_vectorize_loops off
#		pragma optimization_level 4
#		pragma peephole on
#		if defined (DPVS_CPU_PS2)							// PlayStation2-specific pragmas
//PS2-BEGIN
#			pragma alignlabel on
#			pragma conditional_move on
#			pragma inline_intrinsics on 
#			pragma opt_unroll_loops on
#			pragma opt_vectorize_loops on
//PS2-END
#		elif defined (DPVS_OS_MAC)
//#			pragma opt_unroll_loops on
#			pragma opt_vectorize_loops on
#			pragma optimizewithasm on
#			pragma scheduling altivec
#		elif defined (DPVS_CPU_GAMECUBE)
//NGC-BEGIN
#			pragma fp_contract on 
#			pragma scheduling on
//NGC-END
#		endif
#	else 
#		pragma extended_errorcheck on						// extended error-checking
#	endif // DPVS_DEBUG
#endif // DPVS_BUILD_CW

#if defined (DPVS_BUILD_GCC)
#	define DPVS_ASM	__asm__									// assembly language block
#	define DPVS_SUPPORT_ATTRIBUTE							// support __attribute__
#	if defined (DPVS_PS2)
//PS2-BEGIN
#		define DPVS_ALIGN_VECTORS							// request vector alignment
#		define DPVS_PS2_ASSEMBLY							// use PS2 FPU assembler routines?
#		define DPVS_PS2_VU_ASSEMBLY							// use PS2 VU assembler routines?
#		define DPVS_PS2_USE_SCRATCHPAD						// define this to use the SPAD RAM on the PS2
//PS2-END
#	endif // DPVS_PS2
#endif // DPVS_BUILD_GCC

//------------------------------------------------------------------------
// Some compilation changes we must make if checking the code with
// PC-Lint or FlexeLint
//------------------------------------------------------------------------

#if defined (DPVS_LINT)									// seems like LINT doesn't support this
#	undef DPVS_FORCE_INLINE
#endif

//------------------------------------------------------------------------
// Compilation-time assertion. If the expression 'expr' evaluates to false,
// the compilation will be aborted and an error message will be given.
// Unfortunately the error message itself won't display the expression,
// so you will have to scan the source to the line given by the message
// to see which assertion went wrong. This variant of CT assertion can
// be used even inside class descriptions etc.
//
// Note that the CT assertion is evaluated in both release and debug
// builds (as it doesn't generate any real code).
//------------------------------------------------------------------------

#define DPVS_CT_ASSERT(expr) typedef bool DPVSCTAssertion[(expr) ? 1 : -1] 

//------------------------------------------------------------------------
// Define 16-bit integers (modify code below with #ifdefs if compiler
// asserts here about the size!)
//------------------------------------------------------------------------

typedef unsigned char			UINT8;					// 8-bit unsigned integer
typedef short int				INT16;                  // 16-bit signed integer
typedef unsigned short int		UINT16;                 // 16-bit unsigned integer
typedef unsigned int			UPTR;					// unsigned integer large enough to hold a void*

//------------------------------------------------------------------------
// Make sure that certain typedefs really do have the intended sizes
//------------------------------------------------------------------------

DPVS_CT_ASSERT(sizeof(UINT8)==1);
DPVS_CT_ASSERT(sizeof(INT16)==2);
DPVS_CT_ASSERT(sizeof(UINT16)==2);
DPVS_CT_ASSERT(sizeof(INT32)==4);
DPVS_CT_ASSERT(sizeof(UINT32)==4);
DPVS_CT_ASSERT(sizeof(float)==4);
DPVS_CT_ASSERT(sizeof(UPTR)>=sizeof(void*));
DPVS_CT_ASSERT(sizeof(size_t) >= sizeof(void*));

#if defined (DPVS_UINT64_DEFINED)
	DPVS_CT_ASSERT(sizeof(UINT64)==8);
	DPVS_CT_ASSERT(sizeof(INT64)==8);
#endif

//------------------------------------------------------------------------
// The macro DPVS_VFT_IN_END can be used for compiling conditional code
// based on the compiler's VFT table placement. The macro is defined for
// compilers that place the VFT to the _end_ of the structure (rather
// than the beginning). The code below ensures that the macro has been
// set correctly. If you get a compilation error, then go upwards to the
// compile recognition code and set DPVS_VFT_IN_END accordingly
// \todo [wili 021008] use offset_of or offsetof macros on some
//                     compilers?
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_SGI) || defined (DPVS_BUILD_CW) || defined (DPVS_BUILD_INTELC) || defined (DPVS_BUILD_GCC) || defined (DPVS_BUILD_COMO)

enum
{
	VFT_IN_BEGIN = true
};

#else

	struct VFTTest
	{
		virtual ~VFTTest (void) {}
		int		m_data;
	};

	enum
	{
		VFT_IN_BEGIN = &((*((VFTTest*)(0))).m_data) == 0 ? false : true
	};

	#if defined (DPVS_VFT_IN_END)
		DPVS_CT_ASSERT(VFT_IN_BEGIN==false);
	#else
		DPVS_CT_ASSERT(VFT_IN_BEGIN==true);
	#endif // DPVS_VFT_IN_END

#endif

//------------------------------------------------------------------------
// Define DPVS_ATTRIBUTE for compilers that support __attribute__. Other
// compilers just ignore the attribute definition.
//------------------------------------------------------------------------

#if defined (DPVS_SUPPORT_ATTRIBUTE)
#	define DPVS_ATTRIBUTE(X) __attribute__ (X)
#else
#	define DPVS_ATTRIBUTE(X) (DPVS_NULL_STATEMENT)
#endif

// useful DPVS_ATTRIBUTES:
//funcs: pure - uses only arguments & globals
//funcs: const - uses only arguments, no globals, no effects apart from return value (cannot read global memory)
//funcs: malloc - pointer returned cannot alias
//variables & structs: aligned(X) - alignment

//------------------------------------------------------------------------
// If the compiler supports ALIGN directives, then define them here
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC_PPACK) || defined (DPVS_BUILD_INTELC)
#	define DPVS_ALIGN(X,T) __declspec(align(X)) T
#elif defined (DPVS_BUILD_CW) && !defined (DPVS_OS_GAMECUBE)
#	define DPVS_ALIGN(X,T) __attribute__((aligned(X))) T
#elif defined (DPVS_BUILD_CW) && defined (DPVS_OS_GAMECUBE)
//NGC-BEGIN
#	define DPVS_ALIGN(X,T) T __attribute__((aligned(X)))
//NGC-END
#elif defined (DPVS_BUILD_GCC)
#	define DPVS_ALIGN(X,T) __attribute__((aligned(X))) T
#else
#	define DPVS_ALIGN(X,T) T 
#endif

#define DPVS_ALIGN8(T) DPVS_ALIGN(8,T)
#define DPVS_ALIGN16(T) DPVS_ALIGN(16,T)
#define DPVS_ALIGN32(T) DPVS_ALIGN(32,T)

//------------------------------------------------------------------------
// Ensure that all macros used are defined somehow (even empty is ok)
//------------------------------------------------------------------------

#if !defined (DPVS_FORCE_INLINE)
#	define DPVS_FORCE_INLINE inline
#endif

#if !defined (DPVS_ASM)
#	define DPVS_ASM __asm									// assembly language block
#endif

#if !defined (DPVS_RESTRICT)								// the "restrict" keyword
#	define DPVS_RESTRICT
#endif

//------------------------------------------------------------------------
// Macros for testing alignment of data
//------------------------------------------------------------------------

DPVS_FORCE_INLINE bool isDWordAligned (const void* p) { return !(reinterpret_cast<UPTR>(p)&3);		}
DPVS_FORCE_INLINE bool isQWordAligned (const void* p) { return !(reinterpret_cast<UPTR>(p)&7);		}
DPVS_FORCE_INLINE bool is128Aligned   (const void* p) { return !(reinterpret_cast<UPTR>(p)&15);	}

//------------------------------------------------------------------------
// The for() fix macro for compilers that don't respect the C++ standard.
//------------------------------------------------------------------------

#if defined (DPVS_BUILD_MSC) || defined (DPVS_BUILD_SGI)
DPVS_FORCE_INLINE bool getFalse (void) { return false; }
#	define for if(DPVS::getFalse()){} else for
#elif defined (DPVS_BUILD_CW) && !defined(DPVS_OS_MAC)
#	define for if(false){} else for
#endif

//------------------------------------------------------------------------
// Some Commonly used templates
//------------------------------------------------------------------------

template <class T> DPVS_FORCE_INLINE void swap (T& a, T& b) { T t(a); a = b; b = t; }

//------------------------------------------------------------------------
// Data alignment functions
//------------------------------------------------------------------------

const size_t CACHE_LINE_SIZE = 32;										// use 32-byte cache lines (this is conservative and valid assumption on modern hardware)

template <class T> DPVS_FORCE_INLINE T* alignPow2		(T* foo, int ALIGN)	{ return reinterpret_cast<T*>(((size_t)(foo)+(size_t)(ALIGN-1))&~(size_t)(ALIGN-1)); }
template <class T> DPVS_FORCE_INLINE T* alignCacheLine	(T* foo)			{ return alignPow2(foo, CACHE_LINE_SIZE);	}
template <class T> DPVS_FORCE_INLINE T* align128		(T* foo)			{ return alignPow2(foo, 16);				}

//------------------------------------------------------------------------
// This function returns a "bad pointer" i.e. a pointer that cannot
// be returned by any true allocation. This pointer can be used to
// distinguish non-pointer values if null is used for some other
// purpose.
//------------------------------------------------------------------------

extern "C" void* g_invalidPointer;
DPVS_FORCE_INLINE const void* getInvalidPointer (void)
{
	return (const void*) g_invalidPointer;
}

//------------------------------------------------------------------------
// Function for changing endianess of a 32-bit value
//------------------------------------------------------------------------

DPVS_FORCE_INLINE UINT32 changeEndian(UINT32& a)
{
#if defined(DPVS_X86_ASSEMBLY)
	DPVS_ASM
	{
		mov		eax,a
		mov		ecx,[eax]
		bswap	ecx
		mov		[eax],ecx	
	}
#else
	a = (a<<24) | ((a<<8)&0xFF0000) | ((a>>8)&0xFF00) | (a>>24);
#endif
	return a;
}

//------------------------------------------------------------------------
// Union of a 32-bit float and a 32-bit integer. This union is used
// if we need to manipulate floats as integers as reinterpret_cast
// may not work correctly on all compilers (for example MSVC).
//------------------------------------------------------------------------

union FloatInt
{
	float	f;					// floating-point
	UINT32	i;					// integer (unsigned)
};

DPVS_CT_ASSERT(sizeof(FloatInt)==4);

//------------------------------------------------------------------------
// Some compilers define macros that may clash with out function naming
//------------------------------------------------------------------------

#undef min
#undef max

//------------------------------------------------------------------------
// Assertions for debug build ("assume construction" not used any more as
// it inteferes with code optimization on MSVC6)
//------------------------------------------------------------------------

void assertFail (const char* expr, const char* file, int line, const char* message); 

#if defined (DPVS_DEBUG) && !defined (DPVS_DISABLE_ASSERTS)
#	define DPVS_ASSERT(e)			((e) ? DPVS_NULL_STATEMENT : DPVS::assertFail(#e,__FILE__,__LINE__,""))
#	define DPVS_ASSERT_PRINT(a,b)	((a) ? DPVS_NULL_STATEMENT : DPVS::assertFail(#a,__FILE__,__LINE__,#b))
#	define DPVS_API_ASSERT(e)		((e) ? DPVS_NULL_STATEMENT : DPVS::assertFail("API Error:"#e,__FILE__,__LINE__,""))
#else
#	define DPVS_ASSERT(e)			(DPVS_NULL_STATEMENT)					// nada in release build
#	define DPVS_ASSERT_PRINT(a,b)	(DPVS_NULL_STATEMENT)					// nada in release build
#	define DPVS_API_ASSERT(e)		(DPVS_NULL_STATEMENT)					// nada in release build
#endif // DPVS_DEBUG

//------------------------------------------------------------------------
// Alignment for matrices & vectors (needed only on some platforms)
//------------------------------------------------------------------------

#if defined (DPVS_ALIGN_VECTORS)
#	define DPVS_VECTOR_ALIGN(T) DPVS_ALIGN16(T)
#	define DPVS_CHECK_ALIGN(X) DPVS_ASSERT(is128Aligned(X))
#else
#	define DPVS_VECTOR_ALIGN(T) T
#	define DPVS_CHECK_ALIGN(X) (DPVS_NULL_STATEMENT)
#endif

//------------------------------------------------------------------------
// Stack checking class (doesn't do anything in release build). This can
// be used when debugging (as it traps stack pointer corruption).
//------------------------------------------------------------------------

#if defined (DPVS_DEBUG)

class StackCheck
{
public:
					StackCheck	(void)	{ m_this = this; }
	StackCheck*		m_this;				// pointer to self
private:
					StackCheck	(const StackCheck&);
	StackCheck&		operator=	(const StackCheck&);
};

#define DPVS_STACK_CHECK_INIT() StackCheck dpvsStackCheck
#define DPVS_STACK_CHECK()	   { if (dpvsStackCheck.m_this != &dpvsStackCheck) DPVS::assertFail("Stack check failure",__FILE__,__LINE__,"");}

#else
#	define DPVS_STACK_CHECK_INIT() 
#	define DPVS_STACK_CHECK()	
#endif   

//------------------------------------------------------------------------
// Trick for avoiding constructors in certain cases
//------------------------------------------------------------------------

enum DisableConstructor
{
	NO_CONSTRUCTOR									// do not call the constructor
};

//------------------------------------------------------------------------
// Trick for avoiding calling constructors for temporary vectors and
// matrices
//------------------------------------------------------------------------

template <class T, int LEN> class RawVector
{
public:
				RawVector	(void)				{ m_data = align128(m_alloc);DPVS_CHECK_ALIGN(m_data);				}
				operator T* (void)				{ DPVS_CHECK_ALIGN(m_data);return reinterpret_cast<T*>(&m_data[0]);	}
	T&			operator[]	(int index)			{ DPVS_CHECK_ALIGN(m_data);DPVS_ASSERT (index>=0 && index < LEN); return reinterpret_cast<T*>(&m_data[0])[index]; }
	const T&	operator[]	(int index) const	{ DPVS_CHECK_ALIGN(m_data);DPVS_ASSERT (index>=0 && index < LEN); return reinterpret_cast<const T*>(&m_data[0])[index]; }
private:
				RawVector	(const RawVector&);
	RawVector&	operator=	(const RawVector&);
	char*		m_data;
	char		m_alloc[sizeof(T)*LEN+16] ;		// data as raw bytes
};
/*
template <class T, int LEN> class RawVector
{
public:
				RawVector	(void)				{ DPVS_CHECK_ALIGN(&m_data[0]);				}
				operator T* (void)				{ return reinterpret_cast<T*>(&m_data[0]);	}
	T&			operator[]	(int index)			{ DPVS_ASSERT (index>=0 && index < LEN); return *reinterpret_cast<T*>(&m_data[index*sizeof(T)]); }
	const T&	operator[]	(int index) const	{ DPVS_ASSERT (index>=0 && index < LEN); return *reinterpret_cast<const T*>(&m_data[index*sizeof(T)]); }
private:
				RawVector	(const RawVector&);
	RawVector&	operator=	(const RawVector&);
	char DPVS_VECTOR_ALIGN m_data[sizeof(T)*LEN] ;		// data as raw bytes
};
*/
//------------------------------------------------------------------------
// Output traversal into std::cout
//------------------------------------------------------------------------

//#define DPVS_OUTPUT_PORTAL_TRAVERSAL					// dumps the portal traversal process to std::cout

#if defined(DPVS_OUTPUT_PORTAL_TRAVERSAL)
#define DPVS_OUTPUT(x) (x)
#else
#define	DPVS_OUTPUT(x) (DPVS_NULL_STATEMENT)
#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL

#if defined (DPVS_DEBUG)
#	define DPVS_DEBUG_CODE(X) X
#else
#	define DPVS_DEBUG_CODE(X)
#endif

//------------------------------------------------------------------------
// Definitions of optimization algorithms used inside DPVS. These should
// be left untouched except for debugging purposes.
//------------------------------------------------------------------------

//#define DPVS_STABLE									// disables all adapting features

#ifndef DPVS_STABLE
#	define DPVS_VISIBLE_POINT_TRACKING					// are visible point tracking algorithms used?
#	define DPVS_OCCLUDER_SELECTION						// remove bad occluders?
#	define DPVS_POSTPONE_OCCLUDERS						// can occluder writes be postponed?
#	define DPVS_APPROXIMATE_SILHOUETTES					// allow conservative silhouettes (applies to old silhouette cache code only!)
#	define DPVS_HIERARCHICAL_DATABASE					// allow voxel splitting?		DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
//#	define DPVS_FLUSH_TABOO							// allow flush taboos?
#endif //DPVS_STABLE

//#define DPVS_OCCLUSIONBUFFER_INDEXBUFFER				// visualize index buffer (i.e. object order) instead of depth buffer?

#define DPVS_FRONT_CLIPPING_OCCLUDERS					// use front-clipping occluders?
#define DPVS_EXACT_OBJECT_VF_CULLING					// allow exact object-level VF culling?
//#define DPVS_EMERGENCY_RESOLVE							// allow re-resolving of visibility in emergency situations

//------------------------------------------------------------------------
// Internal tile sizes
//------------------------------------------------------------------------

//#define DPVS_TILE_16
#define DPVS_TILE_32
//#define DPVS_TILE_64
//#define DPVS_TILE_128

#if defined (DPVS_TILE_16)
#	define DPVS_TILE_BITS 4
#elif defined(DPVS_TILE_32)
#	define DPVS_TILE_BITS 5
#elif defined(DPVS_TILE_64)
#	define DPVS_TILE_BITS 6
#elif defined (DPVS_TILE_128)
#	define DPVS_TILE_BITS 7
#else
#	error DPVS_TILE must be set!!
#endif

#define DPVS_TILE_WIDTH (1<<DPVS_TILE_BITS)

//------------------------------------------------------------------------
// Debug function for querying the FPU Mode
//------------------------------------------------------------------------

static inline UINT32 getFPUMode (void)
#if defined (DPVS_X86_ASSEMBLY)
{
	UINT16 tmp;
	DPVS_ASM
	{
		fwait
		lea		eax,tmp
		fstcw	[eax]
	}
	return (UINT32)tmp;
}
#else
{
	return 0; // this is a debug function, supported only on x86/win32
}
#endif // DPVS_X86_ASSEMBLY

//------------------------------------------------------------------------
// Generic rotation -> left code and implementation for platforms that
// have native support for it (such as x86)
//------------------------------------------------------------------------

#if defined (DPVS_X86_ASSEMBLY) && defined(DPVS_X86_RETURN_EAX)
#pragma warning (disable:4035)
DPVS_FORCE_INLINE UINT32 rotateLeft (UINT32 x, INT32 r)  
{ 
	__asm
	{
		mov eax,[x]
		mov ecx,[r]
		rol eax,cl
	}
}
#pragma warning (default:4035)
#elif defined (DPVS_CPU_GAMECUBE) && defined (DPVS_BUILD_CW)
//NGC-BEGIN
DPVS_FORCE_INLINE UINT32 rotateLeft(UINT32 x, INT32 r)
{
	DPVS_ASSERT(__rlwnm(x, r, 0, 31) == (x << r) | (x >> (sizeof(x)*8-r)));

	return __rlwnm(x, r, 0, 31);
}
//NGC-END
#else
DPVS_FORCE_INLINE UINT32 rotateLeft (UINT32 x, INT32 r)  { return (x << r) | (x >> (sizeof(x)*8-r));   }
#endif // DPVS_X86_ASSEMBLY && DPVS_X86_RETURN_EAX

//------------------------------------------------------------------------
// OK, we're ready to rock!
//------------------------------------------------------------------------

} // namespace DPVS

// We need to include this here so that files can perform proper memory allocations...

#if !defined (__DPVSNEW_HPP)
#	include "dpvsNew.hpp"												
#endif

//------------------------------------------------------------------------
#endif // __DPVSPRIVATEDEFS_HPP
