// ======================================================================
//
// UiReport.h
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_UiReport_H
#define INCLUDED_UiReport_H

// ======================================================================

#define UI_USE_PROFILER 0

#include <cstdarg>

class UiReport
{
public:

	enum
	{
		RF_print = 1,
		RF_log   = 2
	};

public:

	typedef void(*SetFlagsFunction)(int flags);
	typedef void(*PrintfFunction)(const char* format, va_list va);
	typedef void(*ProfilerEnterFunction)(const char * name);
	typedef void(*ProfilerLeaveFunction)(const char * name);
	typedef void(*MemoryManagerVerifyFunction)(bool guardPatterns, bool freePatterns);

public:

	static void install(SetFlagsFunction setFlagsFunction, PrintfFunction printfFunction, ProfilerEnterFunction profilerEnterFunction, ProfilerLeaveFunction profilerLeaveFunction, MemoryManagerVerifyFunction memoryManagerVerifyFunction);

public:

	static void setFlags(int flags);
	static void printf(const char* format, ...);

	static void profilerEnter (const char * name);
	static void profilerLeave (const char * name);
	static void memoryManagerVerify (bool guardPatterns, bool freePatterns);

private:

	static ProfilerEnterFunction       ms_profilerEnterFunction;
	static ProfilerLeaveFunction       ms_profilerLeaveFunction;
	static MemoryManagerVerifyFunction ms_memoryManagerVerifyFunction;
};

//----------------------------------------------------------------------

inline void UiReport::profilerEnter (const char * name)
{
	if (ms_profilerEnterFunction)
		ms_profilerEnterFunction (name);
}

//----------------------------------------------------------------------

inline void UiReport::profilerLeave (const char * name)
{
	if (ms_profilerLeaveFunction)
		ms_profilerLeaveFunction (name);
}

//----------------------------------------------------------------------

inline void UiReport::memoryManagerVerify (bool guardPatterns, bool freePatterns)
{
	if (ms_memoryManagerVerifyFunction)
		ms_memoryManagerVerifyFunction (guardPatterns, freePatterns);
}

// ======================================================================

#define UI_REPORT(expr, flags, printf2)             ((expr) ? UiReport::setFlags(flags), UiReport::printf printf2 : UI_NOP)
#define UI_REPORT_LOG(expr, printf)                 UI_REPORT(expr, UiReport::RF_log, printf)
#define UI_REPORT_PRINT(expr, printf)               UI_REPORT(expr, UiReport::RF_print, printf)
#define UI_REPORT_LOG_PRINT(expr, printf)           UI_REPORT(expr, UiReport::RF_log | UiReport::RF_print, printf)
#define UI_REPORT_LOG_PRINT_ALWAYS(printf2)         { UiReport::setFlags(UiReport::RF_log | UiReport::RF_print); UiReport::printf printf2; }

#ifdef _DEBUG

	#define UI_DEBUG_REPORT(expr, flags, printf)      UI_REPORT(expr, flags, printf)
	#define UI_DEBUG_REPORT_LOG(expr, printf)         UI_REPORT_LOG(expr, printf)
	#define UI_DEBUG_REPORT_PRINT(expr, printf)       UI_REPORT_PRINT(expr, printf)
	#define UI_DEBUG_REPORT_LOG_PRINT(expr,printf)    UI_REPORT_LOG_PRINT(expr, printf)

#if UI_USE_PROFILER
	#define UI_PROFILER_ENTER(name) UiReport::profilerEnter(name);
	#define UI_PROFILER_LEAVE(name) UiReport::profilerLeave(name);
#else
	#define UI_PROFILER_ENTER(name)                   UI_NOP
	#define UI_PROFILER_LEAVE(name)                   UI_NOP 
#endif

#else

	#define UI_DEBUG_REPORT(expr, flags, printf2)     UI_NOP
	#define UI_DEBUG_REPORT_LOG(expr, printf)         UI_NOP
	#define UI_DEBUG_REPORT_PRINT(expr, printf)       UI_NOP
	#define UI_DEBUG_REPORT_LOG_PRINT(expr, printf)   UI_NOP

	#define UI_PROFILER_ENTER(name)                   UI_NOP
	#define UI_PROFILER_LEAVE(name)                   UI_NOP

#endif


// ======================================================================

#endif
