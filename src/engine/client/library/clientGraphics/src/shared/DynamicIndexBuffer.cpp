// ======================================================================
//
// DynamicIndexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/DynamicIndexBuffer.h"

#include "clientGraphics/Graphics.h"

// ======================================================================

#ifdef _DEBUG
int                  DynamicIndexBuffer::ms_dynamicId;
bool                 DynamicIndexBuffer::ms_dynamicLocked;
#endif

// ======================================================================

DynamicIndexBufferGraphicsData::~DynamicIndexBufferGraphicsData()
{
}

// ======================================================================

DynamicIndexBuffer::DynamicIndexBuffer()
: HardwareIndexBuffer(T_dynamic),
	m_graphicsData(0),
	m_numberOfIndices(0),
	m_indexData(0)
#ifdef _DEBUG
	,
	m_dynamicId(0)
#endif
{
	m_graphicsData = Graphics::createIndexBufferData();
}

// ----------------------------------------------------------------------

DynamicIndexBuffer::~DynamicIndexBuffer()
{
	DEBUG_FATAL(m_indexData, ("Destroying locked dynamic IB"));
	m_indexData = NULL;
	delete m_graphicsData;
}

// ----------------------------------------------------------------------

void DynamicIndexBuffer::copyIndices(const int destinationIndex, const Index *sourceIndexBuffer, const int sourceIndex, const int numberOfIndices)
{
	DEBUG_FATAL(destinationIndex + numberOfIndices > m_numberOfIndices, ("copy overflow"));
	memcpy (m_indexData + destinationIndex, sourceIndexBuffer + sourceIndex, numberOfIndices * sizeof (Index));
}

// ======================================================================

