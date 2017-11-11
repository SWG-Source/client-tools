// PRIVATE ==============================================================
//
// RenderWorld_Services.h
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RenderWorld_Services_H
#define INCLUDED_RenderWorld_Services_H

// ======================================================================

class Mutex;

#include "dpvsLibrary.hpp"

// ======================================================================

class RenderWorldServices : public DPVS::LibraryDefs::Services
{
public:

	RenderWorldServices();
	~RenderWorldServices();

	virtual void   error(const char * message);
	virtual void * allocateMemory(size_t bytes);
	virtual void   releaseMemory(void * pointer);

	virtual void   enterMutex();
	virtual void   leaveMutex();

private:

	RenderWorldServices(const RenderWorldServices &);
	RenderWorldServices &operator =(const RenderWorldServices &);

private:

	Mutex *m_mutex;
};

// ======================================================================

#endif
