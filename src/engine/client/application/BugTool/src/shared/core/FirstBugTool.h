// ======================================================================
//
// FirstBugTool.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FirstBugTool_H
#define INCLUDED_FirstBugTool_H

// ======================================================================

#include <qwidget.h>
#include <string>
#include <vector>

// ======================================================================

#define UNREF(a) static_cast<void>(a)

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
