// ======================================================================
//
// VertexIndexer.cpp
//
// copyright 2002, Sony online entertainment
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "VertexIndexer.h"

#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(VertexIndexer::Vertex, false, 128, 0, 0);

// ======================================================================

VertexIndexer::VertexIndexer()
:
	m_position(false),
	m_positionEpsilonSquared(0.0f),
	m_normal(false),
	m_normalEpsilon(cos(0.0f)),
	m_color0(false),
	m_color1(false),
	m_numberOfTextureCoordinateSets(0),
	m_vertexList(new VertexList),
	m_vertexMap(new VertexMap)
{
	for (int i = 0; i < MAX_DIMENSION; ++i)
		m_textureCoordinateSetDimension[i] = 0;
}

// ----------------------------------------------------------------------

VertexIndexer::~VertexIndexer()
{
	clear();

	delete m_vertexList;
	delete m_vertexMap;
}

// ----------------------------------------------------------------------

void VertexIndexer::reserve(int numberOfVertices)
{
	m_vertexList->reserve(numberOfVertices);
}

// ----------------------------------------------------------------------

void VertexIndexer::addPosition()
{
	m_position = true;
}

// ----------------------------------------------------------------------

void VertexIndexer::addNormal()
{
	m_normal = true;
}

// ----------------------------------------------------------------------

void VertexIndexer::addColor0()
{
	m_color0 = true;
}

// ----------------------------------------------------------------------

void VertexIndexer::addColor1()
{
	m_color1 = true;
}

// ----------------------------------------------------------------------

void VertexIndexer::addTextureCoordinateSet(int dimension)
{
	DEBUG_FATAL(m_numberOfTextureCoordinateSets == MAX_TEXTURE_COORDINATE_SETS, ("Adding too many texture coordinate sets"));
	m_textureCoordinateSetDimension[m_numberOfTextureCoordinateSets] = dimension;
	++m_numberOfTextureCoordinateSets;
}

// ----------------------------------------------------------------------

void VertexIndexer::addVertexBufferFormat(const VertexBufferFormat &format)
{
	if (format.hasPosition())
		addPosition();

	if (format.hasNormal())
		addNormal();

	if (format.hasColor0())
		addColor0();

	if (format.hasColor1())
		addColor1();

	for (int i = 0; i < format.getNumberOfTextureCoordinateSets(); ++i)
		addTextureCoordinateSet(format.getTextureCoordinateSetDimension(i));
}

// ----------------------------------------------------------------------

void VertexIndexer::setPositionEpsilon(float epsilonDistance)
{
	m_positionEpsilonSquared = sqr(epsilonDistance);
}

// ----------------------------------------------------------------------

void VertexIndexer::setNormalEpsilon(float epsilonRadians)
{
	m_normalEpsilon = cos(epsilonRadians);
}

// ----------------------------------------------------------------------

int VertexIndexer::addVertex(const Vertex &vertex)
{
	DEBUG_FATAL(!m_position && !m_normal && !m_color0 && !m_color1 && m_numberOfTextureCoordinateSets == 0, ("No vertex elements to compare"));

	const int numberOfVertices = getNumberOfVertices();

	//-- Hash position into bucket
	int const key = static_cast<int>(vertex.position.x) / 2;

	//-- search the vertex map
	VertexMap::iterator end  = m_vertexMap->end();
	VertexMap::iterator iter = m_vertexMap->find(key);
	if (iter != end)
	{
		IndexList* const indexList = iter->second;

		for (uint i = 0; i < indexList->size(); ++i)
		{
			Vertex const & v = *(*m_vertexList)[(*indexList)[i]];

			if (m_position && vertex.position != v.position && vertex.position.magnitudeBetweenSquared(v.position) > m_positionEpsilonSquared)
				continue;

			if (m_normal && vertex.normal != v.normal && vertex.normal.dot(v.normal) < m_normalEpsilon)
				continue;

			if (m_color0 && vertex.color0 != v.color0)
				continue;

			if (m_color1 && vertex.color1 != v.color1)
				continue;

			bool found = true;
			for (int j = 0; found && j < m_numberOfTextureCoordinateSets; ++j)
				for (int k = 0; found && k < m_textureCoordinateSetDimension[j]; ++k)
					if (vertex.textureCoordinateSets[j].coordinate[k] != v.textureCoordinateSets[j].coordinate[k])
						found = false;

			if (found)
				return (*indexList)[i];
		}
	}
	else
	{
		std::pair<VertexMap::iterator, bool> result = m_vertexMap->insert(std::make_pair(key, new IndexList()));
		DEBUG_FATAL(!result.second, ("could not insert vertex"));

		iter = result.first;
	}

	//-- add the vertex to the vertex list
	m_vertexList->push_back(new Vertex(vertex));
	
	//-- add the index to the map
	iter->second->push_back(numberOfVertices);

	return numberOfVertices;
}

// ----------------------------------------------------------------------

int VertexIndexer::getNumberOfVertices() const
{
	return static_cast<int>(m_vertexList->size());
}

// ----------------------------------------------------------------------

int VertexIndexer::getNumberOfTextureCoordinateSets() const
{
	return m_numberOfTextureCoordinateSets;
}

// ----------------------------------------------------------------------

VertexBufferFormat VertexIndexer::getVertexBufferFormat() const
{
	VertexBufferFormat format;

	if (m_position)
		format.setPosition();

	if (m_normal)
		format.setNormal();

	if (m_color0)
		format.setColor0();

	if (m_color1)
		format.setColor1();

	format.setNumberOfTextureCoordinateSets(m_numberOfTextureCoordinateSets);
	for (int i = 0; i < m_numberOfTextureCoordinateSets; ++i)
		format.setTextureCoordinateSetDimension(i, m_textureCoordinateSetDimension[i]);

	return format;
}

// ----------------------------------------------------------------------

void VertexIndexer::writeVertex(int vertexIndex, VertexBufferWriteIterator iterator) const
{
	Vertex const & vertex = *(*m_vertexList)[vertexIndex];

	if (m_position)
		iterator.setPosition(vertex.position);

	if (m_normal)
		iterator.setNormal(vertex.normal);

	if (m_color0)
		iterator.setColor0(vertex.color0);

	if (m_color1)
		iterator.setColor1(vertex.color1);

	for (int j = 0; j < m_numberOfTextureCoordinateSets; ++j)
		for (int k = 0; k < m_textureCoordinateSetDimension[j]; ++k)
			iterator.setTextureCoordinate(j, k, vertex.textureCoordinateSets[j].coordinate[k]);
}

// ----------------------------------------------------------------------

void VertexIndexer::fillVertexBuffer(VertexBufferWriteIterator iterator) const
{
	const int numberOfVertices = getNumberOfVertices();
	for (int i = 0; i < numberOfVertices; ++i, ++iterator)
		writeVertex(i, iterator);
}

// ----------------------------------------------------------------------

void VertexIndexer::clear()
{
	std::for_each(m_vertexList->begin(), m_vertexList->end(), PointerDeleter());
	m_vertexList->clear();

	std::for_each(m_vertexMap->begin(), m_vertexMap->end(), PointerDeleterPairSecond());
	m_vertexMap->clear();
}

// ----------------------------------------------------------------------

const VertexIndexer::Vertex &VertexIndexer::getVertex(const int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfVertices());
	return *(*m_vertexList)[index];
}

// ======================================================================
