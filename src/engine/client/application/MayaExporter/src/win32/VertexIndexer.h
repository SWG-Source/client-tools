// ======================================================================
//
// VertexIndexer.h
//
// Copyright 2002, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VertexIndexer_H
#define INCLUDED_VertexIndexer_H

// ======================================================================

class MemoryBlockManager;
class SystemVertexBuffer;
class VertexBufferFormat;
class VertexBufferWriteIterator;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/Vector.h"
#include "sharedMath/PackedArgb.h"

// ======================================================================

class VertexIndexer
{
public:

	enum
	{
		MAX_TEXTURE_COORDINATE_SETS = 8,
		MAX_DIMENSION = 4
	};

	struct Vertex
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	public:

		struct TextureCoordinateSet
		{
			float coordinate[MAX_DIMENSION];
		};

		Vector               position;
		Vector               normal;
		PackedArgb           color0;
		PackedArgb           color1;
		TextureCoordinateSet textureCoordinateSets[MAX_TEXTURE_COORDINATE_SETS];
	};

public:

	VertexIndexer();
	~VertexIndexer();

	void reserve(int numberOfVertices);

	void addPosition();
	void addNormal();
	void addColor0();
	void addColor1();
	void addTextureCoordinateSet(int dimension);
	void addVertexBufferFormat(const VertexBufferFormat &format);

	void setPositionEpsilon(float epsilonDistance=0.0f);
	void setNormalEpsilon(float epsilonRadians=0.0f);

	int  addVertex(const Vertex &vertex);

	int                 getNumberOfVertices() const;
	int                 getNumberOfTextureCoordinateSets() const;
	VertexBufferFormat  getVertexBufferFormat() const;
	void                writeVertex(int vertexIndex, VertexBufferWriteIterator iterator) const;
	void                fillVertexBuffer(VertexBufferWriteIterator iterator) const;

	void                clear();
	const Vertex       &getVertex(int index) const;

private:

	VertexIndexer(VertexIndexer const &);
	VertexIndexer & operator=(VertexIndexer const &);

private:

	typedef stdvector<Vertex *>::fwd VertexList;
	typedef stdvector<int>::fwd IndexList;
	typedef stdmap<int, IndexList *>::fwd VertexMap;

	bool        m_position;
	float       m_positionEpsilonSquared;
	bool        m_normal;
	float       m_normalEpsilon;
	bool        m_color0;
	bool        m_color1;
	int         m_numberOfTextureCoordinateSets;
	int         m_textureCoordinateSetDimension[MAX_TEXTURE_COORDINATE_SETS];
	VertexList * const m_vertexList;
	VertexMap * const m_vertexMap;
};

// ======================================================================

#endif
