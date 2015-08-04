// ======================================================================
//
// UniqueVertexSet.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef	UNIQUE_VERTEX_SET_H
#define UNIQUE_VERTEX_SET_H

// ======================================================================
// forward declarations

class PackedArgb;

// ======================================================================

class UniqueVertexSet
{
public:

	struct TcSet;
	struct Vertex;
	class  VertexCompare;

	enum
	{
		MAX_TEXTURE_SETS = 8,
		MAX_TEXTURE_SET_DIMENSIONALITY = 3
	};

public:

	static void install();
	static void remove();

public:

	UniqueVertexSet();
	~UniqueVertexSet();

	// configuration that applies to all vertices handled by this vertex set
	void    setTextureSetCount(int textureSetCount);
	int     getTextureSetCount() const;

	void    setTextureSetDimensionality(int setIndex, int dimensionality);
	int     getTextureSetDimensionality(int setIndex) const;

	// vertex operations
	Vertex &createVertex();

	void    setPositionIndex(Vertex &vertex, int positionIndex);
	int     getPositionIndex(const Vertex &vertex) const;

	void    setNormalIndex(Vertex &vertex, int normalIndex);
	int     getNormalIndex(const Vertex &vertex) const;

	void              setArgb(Vertex &vertex, const PackedArgb &argb);
	const PackedArgb &getArgb(const Vertex &vertex) const;

	void    setUserInt01(Vertex &vertex, int int01);
	int     getUserInt01(const Vertex &vertex) const;

	void    setTextureSetValue(Vertex &vertex, int setIndex, real tc0, real tc1);
	real    getTextureSetValue(Vertex &vertex, int setIndex, int coordinateIndex);

	const Vertex &submitVertex(Vertex &vertex, bool *isUnique);

private:

	struct VertexSet;
	struct VertexContainer;

private:

	int              m_textureSetCount;
	int              m_textureSetDimensionality[MAX_TEXTURE_SETS];

	VertexContainer *m_vertices;
	VertexSet       *m_vertexSet;

private:
	// disabled
	UniqueVertexSet(const UniqueVertexSet&);
	UniqueVertexSet &operator =(const UniqueVertexSet&);
};

// ======================================================================

inline void UniqueVertexSet::setTextureSetCount(int textureSetCount)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, textureSetCount, static_cast<int>(MAX_TEXTURE_SETS));
	m_textureSetCount = textureSetCount;
}

// ----------------------------------------------------------------------

inline int UniqueVertexSet::getTextureSetCount() const
{
	return m_textureSetCount;
}

// ----------------------------------------------------------------------

inline void UniqueVertexSet::setTextureSetDimensionality(int setIndex, int dimensionality)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, m_textureSetCount);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, dimensionality, static_cast<int>(MAX_TEXTURE_SET_DIMENSIONALITY));
	m_textureSetDimensionality[setIndex] = dimensionality;
}

// ----------------------------------------------------------------------

inline int UniqueVertexSet::getTextureSetDimensionality(int setIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, m_textureSetCount);
	return m_textureSetDimensionality[setIndex];
}

// ======================================================================

#endif
