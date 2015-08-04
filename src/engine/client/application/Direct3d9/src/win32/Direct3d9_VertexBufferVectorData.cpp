// ======================================================================
//
// Direct3d9_VertexBufferVectorData.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_VertexBufferVectorData.h"
#include "Direct3d9_VertexBufferDescriptorMap.h"
#include "Direct3d9_VertexDeclarationMap.h"

#include "clientGraphics/HardwareVertexBuffer.h"

#include <vector>

// ======================================================================

namespace Direct3d9_VertexBufferVectorDataNamespace
{
	const int MAX_VERTEX_BUFFERS = 2;
}
using namespace Direct3d9_VertexBufferVectorDataNamespace;

// ======================================================================

Direct3d9_VertexBufferVectorData::Direct3d9_VertexBufferVectorData(VertexBufferVector const & vertexBufferVector)
: VertexBufferVectorGraphicsData(),
	m_vertexDeclaration(0)
{
	DEBUG_FATAL(vertexBufferVector.m_vertexBufferList->size() > MAX_VERTEX_BUFFERS, ("too many vertex buffers in a vector"));

	int j = 0;
	VertexBufferFormat const * vertexBufferFormat[MAX_VERTEX_BUFFERS];
	VertexBufferVector::VertexBufferList::const_iterator iEnd = vertexBufferVector.m_vertexBufferList->end();
	for (VertexBufferVector::VertexBufferList::const_iterator i = vertexBufferVector.m_vertexBufferList->begin(); i != iEnd; ++i, ++j)
		vertexBufferFormat[j] = & (*i)->getFormat();

	m_vertexDeclaration = Direct3d9_VertexDeclarationMap::fetchVertexDeclaration(vertexBufferFormat, j);
}

// ----------------------------------------------------------------------

Direct3d9_VertexBufferVectorData::~Direct3d9_VertexBufferVectorData()
{
	m_vertexDeclaration->Release();
}

// ======================================================================
