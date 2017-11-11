// ======================================================================
//
// ShaderPrimitiveSetWriter.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef SHADER_PRIMITIVE_SET_WRITER_H
#define SHADER_PRIMITIVE_SET_WRITER_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "clientGraphics/Graphics.def"
#include "clientGraphics/ShaderPrimitiveSet.def"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/StlForwardDeclaration.h"

#include <vector>
#include "sharedMath/Vector.h"

class Iff;
class Sphere;
class Vector;
class SystemVertexBuffer;

// A DirectionSortedIndexArray is an array of indices that have been sorted
// so that when the triangles they reference are rendered, those triangles
// are ordered back-to-front when viewed from the sort direction.

// By creating a set of DirectionSortedIndexArrays for an object and rendering
// the object with whichever sorted array has a sort direction closest to the 
// view direction, transparent triangles in the object will be rendered in 
// mostly-correct order without needing to resort the triangles each frame.

struct DirectionSortedIndexArray
{
	DirectionSortedIndexArray();
	~DirectionSortedIndexArray();

	typedef std::vector<Index> TIndexVec;

	TIndexVec indices;
	Vector    sortDir;
};

typedef std::vector<DirectionSortedIndexArray>  TSortedIndexArrayVec;

// ======================================================================

class ShaderPrimitiveSetWriter
{
private:

	// ----------------------------------------------------------------------

	class PerPrimitiveData
	{

	//lint -esym(1536, PerPrimitiveData::nextValue) // exposing low access member

	private:

		PerPrimitiveData                    *nextValue;
	
		CrcLowerString                       shaderTemplateName;
		int                                  priority;
		ShaderPrimitiveSetPrimitiveType      primitiveType;
		SystemVertexBuffer                  *vertexBuffer;
		int                                  indexCount;
		Index                               *indexArray;

		TSortedIndexArrayVec                 sortedIndexArrays;

		bool                                 ownsData;

	private:

		// disabled
		PerPrimitiveData();
		PerPrimitiveData(const PerPrimitiveData&);
		PerPrimitiveData &operator =(const PerPrimitiveData&);

	public:

		PerPrimitiveData(
			const char                      *newShaderTemplateName, 
			int                              newPriority, 
			ShaderPrimitiveSetPrimitiveType  newPrimitiveType, 
			SystemVertexBuffer              *newVertexBuffer, 
			int                              newIndexCount, 
			Index                           *newIndexArray, 
			bool                             newOwnsData
		);
		~PerPrimitiveData();

		void write(Iff & iff, bool bHasAlpha) const;

		PerPrimitiveData                *&next()                              { return nextValue; }
		const PerPrimitiveData           *next() const                        { return nextValue; }

		ShaderPrimitiveSetPrimitiveType  getPrimitiveType() const             { return primitiveType; }
		const SystemVertexBuffer        *getVertexBuffer() const              { return vertexBuffer; }
		int                              getIndexCount() const                { return indexCount; }
		const Index                     *getIndexArray() const                { return indexArray; }

		int                              getPriority() const                  { return priority; }
		const CrcLowerString            &getShaderTemplateName() const        { return shaderTemplateName; }
	};

	// ----------------------------------------------------------------------

private:

	int               primitiveCount;
	PerPrimitiveData *firstPrimitive;
	bool              ownsData;
	bool              hasAlpha;
	
private:

	int                     getShaderTemplateCount() const;
	const PerPrimitiveData *getPerPrimitiveData(int index) const;

public:

	explicit ShaderPrimitiveSetWriter(bool writerOwnsAllData = false);
	~ShaderPrimitiveSetWriter();

	// ----------------------------------------------------------------------
	// Add a primitive to the ShaderPrimitiveSetWriter.
	// 
	// Primitives at a given priority level are guaranteed to draw before
	// primitives with a lower-valued priority.
	// 
	// No more than one Graphics::setShader will occur per priority level per
	// ShaderTemplate at that priority level.
	void add(
		const char                      *shaderTemplateName, 
		ShaderPrimitiveSetPrimitiveType  primitiveType, 
		SystemVertexBuffer              *vertexBuffer, 
		int                              indexCount, 
		Index                           *indexArray, 
		int                              priority = 0
	);
	// ----------------------------------------------------------------------

	void setHasAlpha(bool bHasAlpha);

	void getBoundingBox(Vector *minVector, Vector *maxVector) const;

	Sphere getBoundingSphere() const;

	bool write(Iff *iff) const;
	bool write(const char *filename) const;

	// ----------------------------------------------------------------------
	// Primitive list accessors 
	int                                getNumberOfPrimitives() const                   { return primitiveCount; }

	const char                        *getShaderTemplateName(int primtiveIndex) const;
	ShaderPrimitiveSetPrimitiveType    getPrimitiveType(int primitiveIndex) const;
	const SystemVertexBuffer          *getVertexBuffer(int primitiveIndex) const;
	int                                getIndexCount(int primitiveIndex) const;
	const Index                       *getIndexArray(int primitiveIndex) const;
	// ----------------------------------------------------------------------
};

#endif
