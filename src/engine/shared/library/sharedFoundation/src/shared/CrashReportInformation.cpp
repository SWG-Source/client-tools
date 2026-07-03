// ======================================================================
//
// CrashReportInformation.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedSynchronization/Mutex.h"

// ======================================================================
//
// THREADING (2026-07-03 zone-in corruption fix). These tables are written from
// EVERY loading thread -- FileStreamer logs a "<type> file: <path>" static entry
// per loose-file open (async TreeFile loader, ClientTerrain worker, main), and
// templates add/remove dynamic entries as they install/uninstall. The previous
// implementation used two bare std::vectors: concurrent push_back could double-
// delete[] a grow buffer, poisoning the allocator freelist so a LIVE object's
// block was handed out again -- observed as a StaticShaderTemplate MaterialMap
// overwritten with "...ader file..." path text (terrain-thread AV in
// std::map::find), and as the crash handler itself AVing on a garbage entry
// while walking getEntry(i) (which is why that crash produced no minidump).
//
// New shape -- safe for the crash handler BY CONSTRUCTION:
//  * fixed-capacity arrays, NEVER reallocated (no grow, no delete[] race);
//  * writers serialized by a leaf Mutex (same shape as the TreeFile::SearchCache
//    fix 9c03f53c5);
//  * static-entry strings are never freed until ExitChain remove();
//  * getEntry() takes NO lock (it runs inside the unhandled-exception filter and
//    must not deadlock on a lock the crashed thread may hold): aligned pointer
//    loads are atomic on x86/x64, every slot is null-checked, and nothing it can
//    read is ever freed mid-run -- so a concurrent add can at worst hide the
//    newest entry, never fault the handler.
//  * when the static table fills, new entries are DROPPED (breadcrumbs, not
//    critical); the last slot is replaced by a "(table full)" marker so a
//    truncated crash report is self-describing.
//
// ======================================================================

namespace CrashReportInformationNamespace
{
	int const cms_maxStaticEntries  = 1024;
	int const cms_maxDynamicEntries = 256;

	// Zero-initialized at load (static storage), so getEntry() is safe even
	// before install() and mid-crash.
	char *              ms_staticText[cms_maxStaticEntries];
	int volatile        ms_staticTextCount;

	char const *        ms_dynamicText[cms_maxDynamicEntries];   // holes allowed (removed entries)

	Mutex               ms_writerLock;   // writers only -- getEntry() never takes it

	bool                ms_staticTextFullReported;
}
using namespace CrashReportInformationNamespace;

// ======================================================================

void CrashReportInformation::install()
{
	ExitChain::add(&remove, "CrashReportInformation::remove");
}

// ----------------------------------------------------------------------

void CrashReportInformation::remove()
{
	ms_writerLock.enter();

		for (int i = 0; i < cms_maxStaticEntries; ++i)
		{
			delete [] ms_staticText[i];
			ms_staticText[i] = 0;
		}
		ms_staticTextCount = 0;

		for (int i = 0; i < cms_maxDynamicEntries; ++i)
			ms_dynamicText[i] = 0;

	ms_writerLock.leave();
}

// ----------------------------------------------------------------------

void CrashReportInformation::addStaticText(char const * format, ...)
{
	va_list va;

	va_start(va, format);

		char buffer[256];
		vsnprintf(buffer, sizeof(buffer), format, va);
		buffer[sizeof(buffer) - 1] = '\0';

		ms_writerLock.enter();

			int const count = ms_staticTextCount;
			if (count < cms_maxStaticEntries)
			{
				// Store the string BEFORE publishing the count so a lock-free
				// getEntry() never sees a slot whose pointer is not yet valid.
				ms_staticText[count] = DuplicateString(buffer);
				ms_staticTextCount = count + 1;
			}
			else if (!ms_staticTextFullReported)
			{
				// table full: drop, but make the truncation visible in the report
				ms_staticTextFullReported = true;
				delete [] ms_staticText[cms_maxStaticEntries - 1];
				ms_staticText[cms_maxStaticEntries - 1] = DuplicateString("(crash report static text table full -- newer entries dropped)\n");
			}

		ms_writerLock.leave();

	va_end(va);
}

// ----------------------------------------------------------------------

void CrashReportInformation::addDynamicText(char const * text)
{
	ms_writerLock.enter();

		for (int i = 0; i < cms_maxDynamicEntries; ++i)
		{
			if (!ms_dynamicText[i])
			{
				ms_dynamicText[i] = text;
				break;
			}
		}
		// table full: drop silently (dynamic entries are diagnostic-only)

	ms_writerLock.leave();
}

// ----------------------------------------------------------------------

void CrashReportInformation::removeDynamicText(char const * text)
{
	ms_writerLock.enter();

		for (int i = 0; i < cms_maxDynamicEntries; ++i)
		{
			if (ms_dynamicText[i] == text)
			{
				ms_dynamicText[i] = 0;
				break;
			}
		}

	ms_writerLock.leave();
}

// ----------------------------------------------------------------------
// Called from the unhandled-exception filter (SetupSharedFoundation.cpp) on the
// crashed thread while other threads may still be running. Deliberately takes
// NO lock (deadlock risk) -- safe because the tables never move and static
// strings are never freed mid-run; a null slot just ends/skips the walk.
// Contract preserved: returns the index-th live entry (statics first, then
// dynamics), NULL past the end.
// ----------------------------------------------------------------------

char const * CrashReportInformation::getEntry(int index)
{
	if (index < 0)
		return NULL;

	int const staticCount = ms_staticTextCount;
	if (index < staticCount && index < cms_maxStaticEntries)
		return ms_staticText[index];

	// dynamic entries: return the (index - staticCount)-th non-null slot
	int remaining = index - staticCount;
	for (int i = 0; i < cms_maxDynamicEntries; ++i)
	{
		char const * const entry = ms_dynamicText[i];
		if (entry)
		{
			if (remaining == 0)
				return entry;
			--remaining;
		}
	}

	return NULL;
}

// ======================================================================
