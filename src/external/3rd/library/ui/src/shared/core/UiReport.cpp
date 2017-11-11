// ======================================================================
//
// UiReport.cpp
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UiReport.h"

#include <cstdarg>

// ======================================================================

namespace UiReportNamespace
{
	UiReport::SetFlagsFunction      ms_setFlagsFunction;
	UiReport::PrintfFunction        ms_printfFunction;
}

using namespace UiReportNamespace;

UiReport::ProfilerEnterFunction       UiReport::ms_profilerEnterFunction = 0;
UiReport::ProfilerLeaveFunction       UiReport::ms_profilerLeaveFunction = 0;
UiReport::MemoryManagerVerifyFunction UiReport::ms_memoryManagerVerifyFunction = 0;

// ======================================================================

void UiReport::install(SetFlagsFunction setFlagsFunction, PrintfFunction printfFunction, ProfilerEnterFunction profilerEnterFunction, ProfilerLeaveFunction profilerLeaveFunction, MemoryManagerVerifyFunction memoryManagerVerifyFunction)
{
	ms_setFlagsFunction      = setFlagsFunction;
	ms_printfFunction        = printfFunction;
	ms_profilerEnterFunction = profilerEnterFunction;
	ms_profilerLeaveFunction = profilerLeaveFunction;
	ms_memoryManagerVerifyFunction = memoryManagerVerifyFunction;
}

// ----------------------------------------------------------------------

void UiReport::setFlags(int flags)
{
	if (ms_setFlagsFunction)
		ms_setFlagsFunction(flags);
}

// ----------------------------------------------------------------------

void UiReport::printf(const char* format, ...)
{
	if (ms_printfFunction)
	{
		va_list va;

		va_start(va, format);

			ms_printfFunction(format, va);

		va_end(va);
	}
}

// ======================================================================
