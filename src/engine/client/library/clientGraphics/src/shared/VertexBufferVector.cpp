// ======================================================================
//
// VertexBufferVector.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/VertexBufferVector.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/HardwareVertexBuffer.h"

#include <vector>

// ======================================================================

VertexBufferVectorGraphicsData::~VertexBufferVectorGraphicsData()
{
}

// ======================================================================

VertexBufferVector::VertexBufferVector(HardwareVertexBuffer const & vertexBuffer1)
:
	m_vertexBufferList(new VertexBufferList),
	m_graphicsData(NULL)
{
	m_vertexBufferList->push_back(&vertexBuffer1);
	m_graphicsData = Graphics::createVertexBufferVectorData(*this);
}

// ----------------------------------------------------------------------

VertexBufferVector::VertexBufferVector(HardwareVertexBuffer const & vertexBuffer1, HardwareVertexBuffer const & vertexBuffer2)
:
	m_vertexBufferList(new VertexBufferList),
	m_graphicsData(NULL)
{
	DEBUG_FATAL(vertexBuffer1.getType() == HardwareVertexBuffer::T_dynamic && vertexBuffer2.getType() == HardwareVertexBuffer::T_dynamic, ("Cannot stream two dynamic vertex buffers at the same time"));
	m_vertexBufferList->push_back(&vertexBuffer1);
	m_vertexBufferList->push_back(&vertexBuffer2);
	m_graphicsData = Graphics::createVertexBufferVectorData(*this);
}

// ----------------------------------------------------------------------

VertexBufferVector::~VertexBufferVector()
{
	delete m_vertexBufferList;
	delete m_graphicsData;
}

// ======================================================================
