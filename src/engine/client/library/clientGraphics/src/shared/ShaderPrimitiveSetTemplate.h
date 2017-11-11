// ======================================================================
//
// ShaderPrimitiveSetTemplate.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ShaderPrimitiveSetTemplate_H
#define INCLUDED_ShaderPrimitiveSetTemplate_H

// ======================================================================

#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/StlForwardDeclaration.h"
#include "sharedMath/Vector.h"

class  Appearance;
class  AxialBox;
class CollideParameters;
class  CollisionInfo;
class  Iff;
class IndexedTriangleList;
class  Shader;
class  ShaderPrimitiveSet;
struct ShaderPrimitiveSortKey;
class  ShaderTemplate;
class  StaticIndexBuffer;
class  StaticVertexBuffer;
class  SystemIndexBuffer;
class  SystemVertexBuffer;
class  Transform;

// ======================================================================

class ShaderPrimitiveSetTemplate
{
	friend class ShaderPrimitiveSet;

public:

	static void setAllowUpwardIndicesCreation(bool allowUpwardIndicesCreation);
	static void setBuildingAsynchronousLoaderData(bool buildingAsynchronousLoaderData);
	static void setCollideAgainstAllGeometry(bool collideAgainstAllGeometry);

public:

	class LocalShaderPrimitiveTemplate
	{
	public:

		static void install();

	public:

		LocalShaderPrimitiveTemplate(const char *owner, const ShaderTemplate &shaderTemplate);
		~LocalShaderPrimitiveTemplate();

		const Shader *fetchShader() const;
		Shader       *fetchModifiableShader() const;
		int           getVertexBufferSortKey() const;

		const ShaderTemplate *getShaderTemplate() const;
		const StaticVertexBuffer *getVertexBuffer() const;
		const StaticIndexBuffer  *getIndexBuffer() const;
		Extent const * getExtent() const;
		IndexedTriangleList const * getCollisionIndexedTriangleList() const;
		stdvector<uint16>::fwd const * getUpwardIndices() const;
		void clearUpwardIndices() const;

		void          prepareToDraw(const Transform &objectToWorld) const;
		void          draw() const;

		void          load_old(Iff &iff);
		void          load(Iff & iff);

		void          getCostEstimate(int &numberOfVertices, int &numberOfTriangles, float &complexity) const;

		void          addToExtent(AxialBox &axialBox) const;

		bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;

		float getApproximateRadius() const;

	public:

		typedef void (*DrawFunction)();

	private:

		typedef std::pair<Vector, StaticIndexBuffer*> DirectionSortedIndexBuffer;
		typedef stdvector<DirectionSortedIndexBuffer>::fwd  SortedIndexBufferList;

	private:

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		class CollisionData
		{
		public:

			explicit CollisionData(IndexedTriangleList * indexedTriangleList);
			virtual ~CollisionData() = 0;

			virtual bool collide(Vector const & start_o, Vector const & end_o, CollisionInfo & result) const = 0;
			IndexedTriangleList const * getIndexedTriangleList() const;
			Extent const & getExtent() const;
			stdvector<uint16>::fwd const * getUpwardIndices() const;
			void clearUpwardIndices() const;

		protected:

			stdvector<uint16>::fwd mutable * m_upwardIndices;

		private:

			CollisionData();
			CollisionData(CollisionData const &);
			CollisionData & operator=(CollisionData const &);

			virtual void createUpwardIndices() const = 0;

		private:

			IndexedTriangleList * const m_indexedTriangleList;
			BoxExtent m_boxExtent;
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		class SimpleCollisionData : public CollisionData
		{
		public:

			SimpleCollisionData(IndexedTriangleList * indexedTriangleList);
			virtual ~SimpleCollisionData();

			virtual bool collide(Vector const & start_o, Vector const & end_o, CollisionInfo & result) const;

		private:

			SimpleCollisionData();
			SimpleCollisionData(SimpleCollisionData const &);
			SimpleCollisionData & operator=(SimpleCollisionData const &);

			virtual void createUpwardIndices() const;

		private:

			Vector * m_faceNormals;
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		class BucketedCollisionData : public CollisionData
		{
		public:

			BucketedCollisionData(IndexedTriangleList * indexedTriangleList);
			virtual ~BucketedCollisionData();

			virtual bool collide(Vector const & start_o, Vector const & end_o, CollisionInfo & result) const;

		private:

			BucketedCollisionData();
			BucketedCollisionData(BucketedCollisionData const &);
			BucketedCollisionData & operator=(BucketedCollisionData const &);

			virtual void createUpwardIndices() const;

		private:

			BoxExtent m_bucketedBoxExtent[9];
			stdvector<uint16>::fwd * m_bucketedIndices[9];
			Vector * m_bucketedFaceNormals[9];
		};

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	private:

		// save/load accessory functions
		static StaticIndexBuffer *loadStaticIndexBuffer16(Iff &iff);
		static StaticIndexBuffer *loadStaticIndexBuffer32(Iff &iff);

#ifdef _DEBUG

		void validateIndexBuffer(const StaticIndexBuffer *indexBuffer) const;
		void validateShaderTemplateWithVertexBuffer(const ShaderTemplate *shaderTemplate) const;
		void dataLint() const;

#endif

		void load_0000 (Iff& iff);
		void load_0001 (Iff& iff);

		void collisionSplit(char const * debugName);

		void computeApproximateRadius();

	private:

#ifdef _DEBUG
		const char                      *m_owner;
#endif

		const ShaderTemplate            *m_shaderTemplate;
		StaticVertexBuffer              *m_vertexBuffer;
		StaticIndexBuffer               *m_indexBuffer;
		SortedIndexBufferList           *m_sortedIndices;
		DrawFunction                     m_drawFunction;
		CollisionData * m_collisionData;
		float m_approximateRadius;

	private:

		// disabled
		LocalShaderPrimitiveTemplate(const LocalShaderPrimitiveTemplate &);
		LocalShaderPrimitiveTemplate &operator =(const LocalShaderPrimitiveTemplate &);
	};

public:

	ShaderPrimitiveSetTemplate(const char *owner, Iff &iff);
	~ShaderPrimitiveSetTemplate();

	ShaderPrimitiveSet *createShaderPrimitiveSet(const Appearance &owner) const;

	int getNumberOfShaderPrimitiveTemplates() const;
	LocalShaderPrimitiveTemplate const & getShaderPrimitiveTemplate(int shaderPrimitiveTemplateIndex) const;

#ifdef _DEBUG
	AxialBox            getExtent() const;
#endif

private:

	void load(const char *owner, Iff &iff);
	void load_sps(const char *owner, Iff &iff);
	void load_sps_0000(const char *owner, Iff &iff);
	void load_sps_0001(const char *owner, Iff &iff);

private:

	typedef stdvector<LocalShaderPrimitiveTemplate *>::fwd LocalShaderPrimitiveTemplateList;

private:

	LocalShaderPrimitiveTemplateList *m_localShaderPrimitiveTemplateList;

private:

	/// Disabled.
	ShaderPrimitiveSetTemplate();
	ShaderPrimitiveSetTemplate(const ShaderPrimitiveSetTemplate &);
	ShaderPrimitiveSetTemplate &operator =(const ShaderPrimitiveSetTemplate &);
};

//----------------------------------------------------------------------

inline float ShaderPrimitiveSetTemplate::LocalShaderPrimitiveTemplate::getApproximateRadius() const
{
	if (m_collisionData)
		return m_collisionData->getExtent().getSphere().getRadius();
	else
		return m_approximateRadius;
}

// ======================================================================

#endif
