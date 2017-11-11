// ======================================================================
//
// DynamicVertexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/DynamicVertexBuffer.h"

#include "clientGraphics/Graphics.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MemoryBlockManager  *DynamicVertexBuffer::ms_memoryBlockManager;

#ifdef _DEBUG
int                               DynamicVertexBuffer::ms_dynamicGlobalId;
bool                              DynamicVertexBuffer::ms_dynamicGlobalLocked;
#endif

// ======================================================================

DynamicVertexBufferGraphicsData::~DynamicVertexBufferGraphicsData()
{
}

// ======================================================================

void DynamicVertexBuffer::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("DynamicVertexBuffer", true, sizeof(DynamicVertexBuffer), 0, 0, 0);
	ExitChain::add(remove, "DynamicVertexBuffer::remove()");
}

// ----------------------------------------------------------------------

void DynamicVertexBuffer::remove()
{
	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *DynamicVertexBuffer::operator new(size_t size)
{
	UNREF(size);	
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(DynamicVertexBuffer), ("Looks like a decendent class is trying to use our new routine"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  DynamicVertexBuffer::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

DynamicVertexBuffer::DynamicVertexBuffer(const VertexBufferFormat &format)
: HardwareVertexBuffer(T_dynamic, format),
	m_graphicsData(0),
	m_sortKey(0),
	m_data(0),
	m_numberOfVertices(0)
#ifdef _DEBUG
	,
	m_dynamicId(0)
#endif
{
	m_graphicsData = Graphics::createVertexBufferData(*this);
	m_descriptor = &m_graphicsData->getDescriptor();
	m_sortKey = m_graphicsData->getSortKey();
}

// ----------------------------------------------------------------------

DynamicVertexBuffer::~DynamicVertexBuffer()
{
	DEBUG_FATAL(m_data, ("Destroying VB while its locked"));
	delete m_graphicsData;
	m_data = NULL; //lint !e672 // possible memory leak
}

// ----------------------------------------------------------------------

VertexBufferWriteIterator DynamicVertexBuffer::preLoad(int numberOfVertices)
{
	DEBUG_FATAL(true, ("Cannot load into a dynamic vertex buffer"));
	UNREF(numberOfVertices);

	return VertexBufferWriteIterator(*this, NULL);
}

// ----------------------------------------------------------------------

void DynamicVertexBuffer::postLoad()
{
	DEBUG_FATAL(true, ("Cannot load into a dynamic vertex buffer"));
}

// ======================================================================
