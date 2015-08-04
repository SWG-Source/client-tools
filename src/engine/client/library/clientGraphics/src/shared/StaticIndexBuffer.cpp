// ======================================================================
//
// StaticIndexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/StaticIndexBuffer.h"

#include "clientGraphics/Graphics.h"

// ======================================================================

StaticIndexBufferGraphicsData::~StaticIndexBufferGraphicsData()
{
}

// ======================================================================

StaticIndexBuffer::StaticIndexBuffer(int numberOfIndices)
: HardwareIndexBuffer(T_static),
	m_numberOfIndices(numberOfIndices),
	m_graphicsData(0),
	m_indexData(0),
	m_readOnly(false)
{
	m_graphicsData = Graphics::createIndexBufferData(*this);
}

// ----------------------------------------------------------------------

StaticIndexBuffer::~StaticIndexBuffer()
{
	DEBUG_FATAL(m_indexData, ("Destroying locked static IB"));
	m_indexData = NULL;
	delete m_graphicsData;
}

// ======================================================================
