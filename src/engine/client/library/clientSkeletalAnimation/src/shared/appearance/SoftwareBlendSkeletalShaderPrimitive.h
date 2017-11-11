// PRIVATE

// ======================================================================
//
// SoftwareBlendSkeletalShaderPrimitive.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_SoftwareBlendSkeletalShaderPrimitive_H
#define INCLUDED_SoftwareBlendSkeletalShaderPrimitive_H

// ======================================================================

#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "sharedCollision/BoxExtent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/PackedArgb.h"

class CollisionInfo;
class DynamicVertexBuffer;
class MeshConstructionHelper;
class Shader;
class ShadowVolume;
class MemoryBlockManager;
class SkeletalAppearance2;
class StaticIndexBuffer;
class StaticVertexBuffer;
class SystemVertexBuffer;
class Transform;
class Vector;
class VertexBufferFormat;
class VertexBufferWriteIterator;
class VertexBufferVector;
class PoseModelTransform;

// ======================================================================

class SoftwareBlendSkeletalShaderPrimitive: public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();

public:

	SoftwareBlendSkeletalShaderPrimitive(class SkeletalAppearance2 &appearance, int lodIndex, const MeshConstructionHelper &mesh, int shaderIndex);
	virtual ~SoftwareBlendSkeletalShaderPrimitive();

	Shader                     &getShader();
	const Shader               &getShader() const;

	virtual float               alter(float deltaTime);
	virtual const Vector        getPosition_w() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader &prepareToView() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;
	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;
	virtual void                setSkinningMode(SkinningMode skinningMode);
	virtual void                calculateSkinnedGeometryNow();
	virtual bool                collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const;
	virtual float               getRadius() const;

	virtual SoftwareBlendSkeletalShaderPrimitive       *asSoftwareBlendSkeletalShaderPrimitive();
	virtual SoftwareBlendSkeletalShaderPrimitive const *asSoftwareBlendSkeletalShaderPrimitive() const;

	void                        calculateBoundingBox(Vector &minVector, Vector &maxVector) const;

	SystemVertexBuffer  const  *getSkinnedVertexBuffer() const;
	StaticIndexBuffer   const  *getIndexBuffer() const;
	PackedArgb                  getRepresentativeColor() const;

	void                        setEveryOtherFrameSkinningEnabled(bool enabled);
	bool                        getEveryOtherFrameSkinningEnabled() const;

public:

	struct Dot3Vector;
	struct SourceVertex;
	struct TransformData;

private:

	class  RenderCommand;
	friend class RenderCommand;

	typedef stdvector<RenderCommand*>::fwd           RenderCommandVector;

private:

	static void remove();

private:

	void _initialize(const MeshConstructionHelper &mesh, int shaderIndex, bool multiStream);

	void buildIndexBufferAndRenderCommands(const MeshConstructionHelper &mesh, int perShaderDataIndex);
	void fillConstantVertexBufferData(const MeshConstructionHelper &mesh, int shaderIndex, VertexBufferWriteIterator &destVertexIt);

	void skinData(int transformCount, const PoseModelTransform *transformArray, VertexBufferWriteIterator &iterator) const;

	void fillVertexBuffer(int transformCount, const PoseModelTransform *transformArray, VertexBufferWriteIterator &destVertexIt) const;

#ifdef _DEBUG
	static void _renderVertexMatrices(const Vector &position, const Vector &normal, const Vector &dot3Normal, float dot3Flip);
#endif

	PackedArgb  computeRepresentativeColor() const;

private:

	static bool                ms_installed;
	static bool                ms_useMultiStreamVertexBuffers;

private:

	Shader                 *m_shader;
	SkeletalAppearance2    &m_appearance;

	/**
	 * The LOD index for which this ShaderPrimitive is created.
	 *
	 * Needed for retrieving the proper LOD skeleton from the appearance
	 * at render time.
	 */
	const int               m_lodIndex;

	RenderCommandVector    *m_renderCommands;

	DynamicVertexBuffer    *m_dynamicStream;
	StaticVertexBuffer     *m_staticStream;
	SystemVertexBuffer     *m_systemStream;
	VertexBufferVector     *m_vertexBufferVector;

	StaticIndexBuffer      *m_indexBuffer;

	int                     m_vertexCount;

	//-----------------------------------------------
	void                   *m_sourceData;
	SourceVertex           *m_sourceVectors;
	Dot3Vector             *m_sourceDot3Vectors;
	TransformData          *m_transformData;
	//-----------------------------------------------

	mutable ShadowVolume   *m_shadowVolume;

	SkinningMode            m_skinningMode;
	mutable bool            m_hasBeenSkinned;

	int                     m_dot3TextureCoordinateSetIndex;
	bool                    m_hasDot3Vector;

	bool                    m_everyOtherFrameSkinningEnabled;
	mutable bool            m_haveRepresentativeColor;
	mutable PackedArgb      m_representativeColor;

	mutable BoxExtent       m_boxExtent;

private:

	// disabled
	SoftwareBlendSkeletalShaderPrimitive();
	SoftwareBlendSkeletalShaderPrimitive(const SoftwareBlendSkeletalShaderPrimitive&);
	SoftwareBlendSkeletalShaderPrimitive &operator =(const SoftwareBlendSkeletalShaderPrimitive&);

};

// ======================================================================

inline Shader &SoftwareBlendSkeletalShaderPrimitive::getShader()
{
	return *m_shader;
}

// ----------------------------------------------------------------------

inline const Shader &SoftwareBlendSkeletalShaderPrimitive::getShader() const
{
	return *m_shader;
}

//----------------------------------------------------------------------

inline float SoftwareBlendSkeletalShaderPrimitive::getRadius() const
{
	return m_boxExtent.getSphere().getRadius();
}

// ======================================================================

#endif
