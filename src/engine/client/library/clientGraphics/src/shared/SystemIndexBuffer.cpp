// ======================================================================
//
// SystemIndexBuffer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SystemIndexBuffer.h"

// ======================================================================

SystemIndexBuffer::SystemIndexBuffer(int numberOfIndices)
:
	m_numberOfIndices(numberOfIndices),
	m_data(new Index[m_numberOfIndices])
{
}

// ----------------------------------------------------------------------

SystemIndexBuffer::~SystemIndexBuffer()
{
	delete [] m_data;
}

// ----------------------------------------------------------------------

void SystemIndexBuffer::copyIndices(int const destinationIndex, Index const * const sourceIndexBuffer, int const sourceIndex, int const numberOfIndices)
{
	DEBUG_FATAL(destinationIndex + numberOfIndices > m_numberOfIndices, ("copy overflow"));
	memcpy(m_data + destinationIndex, sourceIndexBuffer + sourceIndex, numberOfIndices * sizeof(Index));
}

// ======================================================================
