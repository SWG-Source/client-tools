// ======================================================================
//
// FirstCrashReporter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FirstCrashReporter_H
#define INCLUDED_FirstCrashReporter_H

// ======================================================================

#define _WIN32_WINNT 0x0501

#include <windows.h>

#include <string>
#include <vector>

#pragma warning (disable: 4786)
#pragma warning (disable: 4702)

// ======================================================================

/* These are needed to allow this non-engine app to link with our engine-bound stl libs
 */
// <hack>
enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}
// </hack>

// ======================================================================

#endif
