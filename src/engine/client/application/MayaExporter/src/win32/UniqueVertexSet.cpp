// ======================================================================
//
// UniqueVertexSet.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "UniqueVertexSet.h"

#include "sharedMath/PackedArgb.h"

#include <list>
#include <set>

// ======================================================================
// lint supression
// ======================================================================

// error 754: local structure member unreferenced
//lint -esym(754, VertexSet::VertexSet)
//lint -esym(754, VertexSet::operator=)

// ======================================================================
// embedded entities
// ======================================================================

struct UniqueVertexSet::TcSet
{
	real  m_textureCoordinate[MAX_TEXTURE_SET_DIMENSIONALITY];
};

// ======================================================================

struct UniqueVertexSet::Vertex
{
public:

#if 0
	static void *operator new(size_t size);
	static void  operator delete(size_t size, void *data);
#endif

public:

	Vertex();

public:

	int         m_positionIndex;
	int         m_normalIndex;
	PackedArgb  m_argb;
	TcSet       m_textureCoordinateSet[MAX_TEXTURE_SETS];
	
	int         m_userInt01;
};

// ======================================================================

class UniqueVertexSet::VertexCompare
{
public:

	explicit VertexCompare(const UniqueVertexSet *uniqueVertexSet);

	bool operator ()(const UniqueVertexSet::Vertex *lhs, const UniqueVertexSet::Vertex *rhs);

private:

	const UniqueVertexSet *m_uniqueVertexSet;

private:
	// disabled
	VertexCompare();
};

// ======================================================================

struct UniqueVertexSet::VertexSet
{
public:

	typedef std::set<Vertex*, VertexCompare> Container;

public:

	explicit VertexSet(const VertexCompare &vertexCompare);

public:

	Container  m_container;

private:
	// disabled
	VertexSet();
	VertexSet(const VertexSet&);
	VertexSet &operator =(const VertexSet&);
};

// ======================================================================

struct UniqueVertexSet::VertexContainer
{
public:

	typedef std::list<Vertex*> Container;

public:

	Container  m_container;

};

// ======================================================================
// class UniqueVertexSet::Vertex
// ======================================================================

UniqueVertexSet::Vertex::Vertex()
:
	m_positionIndex(-1),
	m_normalIndex(-1),
	m_argb(PackedArgb::solidWhite),
	m_userInt01(0)
{
	for (unsigned i = 0; i < MAX_TEXTURE_SETS; ++i)
	{
		TcSet &tcSet = m_textureCoordinateSet[i];
		for (unsigned j = 0; j < MAX_TEXTURE_SET_DIMENSIONALITY; ++j)
		{
			tcSet.m_textureCoordinate[j] = CONST_REAL(0);
		}
	}
}

// ======================================================================
// class UniqueVertexSet::VertexCompare
// ======================================================================

inline UniqueVertexSet::VertexCompare::VertexCompare(const UniqueVertexSet *uniqueVertexSet)
: 
	m_uniqueVertexSet(NON_NULL(uniqueVertexSet)) 
{
}

// ----------------------------------------------------------------------

bool UniqueVertexSet::VertexCompare::operator ()(const UniqueVertexSet::Vertex *lhs, const UniqueVertexSet::Vertex *rhs)
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	//-- perform less-than operation
	if (lhs->m_positionIndex < rhs->m_positionIndex)
		return true;
	else if (lhs->m_positionIndex > rhs->m_positionIndex)
		return false;
	
	if (lhs->m_normalIndex < rhs->m_normalIndex)
		return true;
	else if (lhs->m_normalIndex > rhs->m_normalIndex)
		return false;

	if (lhs->m_argb.getR() < rhs->m_argb.getR())
		return true;
	else if (lhs->m_argb.getR() > rhs->m_argb.getR())
		return false;

	if (lhs->m_argb.getG() < rhs->m_argb.getG())
		return true;
	else if (lhs->m_argb.getG() > rhs->m_argb.getG())
		return false;

	if (lhs->m_argb.getB() < rhs->m_argb.getB())
		return true;
	else if (lhs->m_argb.getB() > rhs->m_argb.getB())
		return false;

	if (lhs->m_argb.getA() < rhs->m_argb.getA())
		return true;
	else if (lhs->m_argb.getA() > rhs->m_argb.getA())
		return false;

	const int tcSetCount = m_uniqueVertexSet->getTextureSetCount();
	for (int tcSetIndex = 0; tcSetIndex < tcSetCount; ++tcSetIndex)
	{
		const TcSet &lhsTcSet = lhs->m_textureCoordinateSet[tcSetIndex];
		const TcSet &rhsTcSet = rhs->m_textureCoordinateSet[tcSetIndex];

		const int tcDimensionCount = m_uniqueVertexSet->getTextureSetDimensionality(tcSetIndex);
		for (int tcDimension = 0; tcDimension < tcDimensionCount; ++tcDimension)
		{
			if (lhsTcSet.m_textureCoordinate[tcDimension] < rhsTcSet.m_textureCoordinate[tcDimension])
				return true;
			else if (lhsTcSet.m_textureCoordinate[tcDimension] > rhsTcSet.m_textureCoordinate[tcDimension])
				return false;
		}
	}

	// if we get here, they're equal, so < is false
	return false;
}

// ======================================================================
// class UniqueVertexSet::VertexSet
// ======================================================================

inline UniqueVertexSet::VertexSet::VertexSet(const VertexCompare &vertexCompare)
: m_container(vertexCompare)
{
}

// ======================================================================
// class UniqueVertexSet
// ======================================================================

void UniqueVertexSet::install()
{
	// -TRF- mem block manage Vertex if performance is lacking
}

// ----------------------------------------------------------------------

void UniqueVertexSet::remove()
{
}

// ----------------------------------------------------------------------

UniqueVertexSet::UniqueVertexSet()
:
	m_textureSetCount(0),
	m_vertices(0),
	m_vertexSet(0)
{
	m_vertices  = NON_NULL(new VertexContainer());

	VertexCompare vertexCompare(this);
	m_vertexSet = NON_NULL(new VertexSet(vertexCompare));

	for (int i = 0; i < MAX_TEXTURE_SETS; ++i)
		m_textureSetDimensionality[i] = 2;
}

// ----------------------------------------------------------------------

UniqueVertexSet::~UniqueVertexSet()
{
	// we can just delete this since all Vertex storage is accounted for in m_vertices
	delete m_vertexSet;

	// store and delete all vertices created in the m_vertices container
	NOT_NULL(m_vertices);
	VertexContainer::Container::iterator             it    = m_vertices->m_container.begin();
	const VertexContainer::Container::const_iterator itEnd = m_vertices->m_container.end();
	for (; it != itEnd; ++it)
	{
		Vertex *const vertex = *it;
		delete vertex;
	}

	delete m_vertices;
}

// ----------------------------------------------------------------------

UniqueVertexSet::Vertex &UniqueVertexSet::createVertex()
{
	Vertex *vertex = NON_NULL(new Vertex());

	NOT_NULL(m_vertices);
	m_vertices->m_container.push_back(vertex);
	
	return *vertex;	
}

// ----------------------------------------------------------------------

void UniqueVertexSet::setPositionIndex(Vertex &vertex, int positionIndex)
{
	vertex.m_positionIndex = positionIndex;
}

// ----------------------------------------------------------------------

int UniqueVertexSet::getPositionIndex(const Vertex &vertex) const
{
	return vertex.m_positionIndex;
}

// ----------------------------------------------------------------------

void UniqueVertexSet::setNormalIndex(Vertex &vertex, int normalIndex)
{
	vertex.m_normalIndex = normalIndex;
}

// ----------------------------------------------------------------------

int UniqueVertexSet::getNormalIndex(const Vertex &vertex) const
{
	return vertex.m_normalIndex;
}

// ----------------------------------------------------------------------

void UniqueVertexSet::setArgb(Vertex &vertex, const PackedArgb &argb)
{
	vertex.m_argb = argb;
}

// ----------------------------------------------------------------------

const PackedArgb &UniqueVertexSet::getArgb(const Vertex &vertex) const
{
	return vertex.m_argb;
}

// ----------------------------------------------------------------------

void UniqueVertexSet::setUserInt01(Vertex &vertex, int int01)
{
	vertex.m_userInt01 = int01;
}

// ----------------------------------------------------------------------

int UniqueVertexSet::getUserInt01(const Vertex &vertex) const
{
	return vertex.m_userInt01;
}

// ----------------------------------------------------------------------

void UniqueVertexSet::setTextureSetValue(Vertex &vertex, int setIndex, real tc0, real tc1)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, m_textureSetCount);
	DEBUG_FATAL(m_textureSetDimensionality[setIndex] != 2, ("tc dimensionality mismatch, expected 2, was %d", m_textureSetDimensionality[setIndex]));

	vertex.m_textureCoordinateSet[setIndex].m_textureCoordinate[0] = tc0;
	vertex.m_textureCoordinateSet[setIndex].m_textureCoordinate[1] = tc1;
}

// ----------------------------------------------------------------------

real UniqueVertexSet::getTextureSetValue(Vertex &vertex, int setIndex, int coordinateIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, m_textureSetCount);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, coordinateIndex, m_textureSetDimensionality[setIndex]);

	return vertex.m_textureCoordinateSet[setIndex].m_textureCoordinate[coordinateIndex];
}

// ----------------------------------------------------------------------

const UniqueVertexSet::Vertex &UniqueVertexSet::submitVertex(Vertex &vertex, bool *isUnique)
{
	NOT_NULL(isUnique);
	NOT_NULL(m_vertexSet);

	std::pair<VertexSet::Container::iterator, bool> result = m_vertexSet->m_container.insert(&vertex);
	*isUnique = result.second;

	if (result.second)
	{
		// the vertex was added to the set.  the vertex just added is unique, so return it.
		return vertex;
	}
	else
	{
		// the vertex was not added to the set.  return the vertex from the set.
		Vertex *const uniqueVertex = NON_NULL(*(result.first));
		return *uniqueVertex;
	}
}

// ======================================================================
