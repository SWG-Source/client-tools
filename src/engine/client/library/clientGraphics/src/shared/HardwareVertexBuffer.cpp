// ======================================================================
//
// HardwareVertexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/HardwareVertexBuffer.h"

// ======================================================================

HardwareVertexBuffer::HardwareVertexBuffer(Type type, const VertexBufferFormat &format)
: VertexBuffer(format),
	m_type(type)
{
}

// ----------------------------------------------------------------------

HardwareVertexBuffer::HardwareVertexBuffer(Type type)
: VertexBuffer(),
	m_type(type)
{
}

// ----------------------------------------------------------------------

HardwareVertexBuffer::~HardwareVertexBuffer()
{
}

// ======================================================================
