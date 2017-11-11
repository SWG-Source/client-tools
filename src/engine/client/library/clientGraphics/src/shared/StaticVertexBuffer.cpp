// ======================================================================
//
// StaticVertexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/StaticVertexBuffer.h"

#include "clientGraphics/Graphics.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

MemoryBlockManager  *StaticVertexBuffer::ms_memoryBlockManager;

// ======================================================================

StaticVertexBufferGraphicsData::~StaticVertexBufferGraphicsData()
{
}

// ======================================================================

void StaticVertexBuffer::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("StaticVertexBuffer", true, sizeof(StaticVertexBuffer), 0, 0, 0);
	ExitChain::add(remove, "StaticVertexBuffer::remove()");
}

// ----------------------------------------------------------------------

void StaticVertexBuffer::remove()
{
	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *StaticVertexBuffer::operator new(size_t size)
{
	UNREF(size);	
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(StaticVertexBuffer), ("Looks like a decendent class is trying to use our new routine"));
	DEBUG_FATAL(size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void  StaticVertexBuffer::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ======================================================================

StaticVertexBuffer::StaticVertexBuffer(const VertexBufferFormat &format, int numberOfVertices)
: HardwareVertexBuffer(T_static, format),
	m_graphicsData(0),
	m_sortKey(0),
	m_numberOfVertices(numberOfVertices),
	m_data(0),
	m_lockedReadOnly(false)
#ifdef _DEBUG
	,
	m_locks(0),
	m_sets(0),
	m_lockedTooFrequentlyWarning(false)
#endif
{
	m_graphicsData = Graphics::createVertexBufferData(*this);
	m_descriptor = &m_graphicsData->getDescriptor();
	m_sortKey = m_graphicsData->getSortKey();
}

// ----------------------------------------------------------------------

StaticVertexBuffer::StaticVertexBuffer(Iff &iff)
: HardwareVertexBuffer(T_static),
	m_graphicsData(0),
	m_sortKey(0),
	m_numberOfVertices(0),
	m_data(0),
	m_lockedReadOnly(false)
#ifdef _DEBUG
	,
	m_locks(0),
	m_sets(0),
	m_lockedTooFrequentlyWarning(false)
#endif
{
	load(iff);
}

// ----------------------------------------------------------------------

StaticVertexBuffer::~StaticVertexBuffer()
{
	DEBUG_FATAL(m_data, ("Destroying locked static VB"));
	delete m_graphicsData;
}

// ----------------------------------------------------------------------

VertexBufferWriteIterator StaticVertexBuffer::preLoad(int numberOfVertices)
{
	m_numberOfVertices = numberOfVertices;
	m_graphicsData = Graphics::createVertexBufferData(*this);
	m_descriptor = &m_graphicsData->getDescriptor();
	m_sortKey = m_graphicsData->getSortKey();

	lock();

	return VertexBufferWriteIterator(*this, m_data);
}

// ----------------------------------------------------------------------

void StaticVertexBuffer::postLoad()
{
	unlock();
}

// ======================================================================
