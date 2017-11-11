// ======================================================================
//
// VertexBufferIterator.cpp
// Copyright 2000-01, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/VertexBufferIterator.h"

// ======================================================================

VertexBufferBaseIterator::~VertexBufferBaseIterator()
{
	m_vertexBuffer = NULL;
	m_descriptor = NULL;
	m_data = NULL;
}

// ----------------------------------------------------------------------

VertexBufferReadIterator::~VertexBufferReadIterator()
{
}

// ----------------------------------------------------------------------

VertexBufferWriteIterator::~VertexBufferWriteIterator()
{
}

// ----------------------------------------------------------------------

VertexBufferReadWriteIterator::~VertexBufferReadWriteIterator()
{
}

// ======================================================================
