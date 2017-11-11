// ======================================================================
//
// SoftwareBlendSkeletalShaderPrimitive.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SoftwareBlendSkeletalShaderPrimitive.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/CustomizableShader.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferVector.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/MeshConstructionHelper.h"
#include "clientSkeletalAnimation/OwnerProxyShader.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/PoseModelTransform.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/Plane.h"
#include "sharedMath/SSeMath.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/Object.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <malloc.h>

//-----------------------------------
#undef TRY_FOR_SSE
#define TRY_FOR_SSE WIN32

#if TRY_FOR_SSE
#include "sharedMath/SseMath.h"
#endif

// ==============================================================================
// SofwareBlendSkeletalShaderPrimitive Namespace
// ==============================================================================
namespace SofwareBlendSkeletalShaderPrimitiveNamespace
{
	typedef SoftwareBlendSkeletalShaderPrimitive::Dot3Vector Dot3Vector;
	typedef SoftwareBlendSkeletalShaderPrimitive::SourceVertex SourceVertex;
	typedef SoftwareBlendSkeletalShaderPrimitive::TransformData TransformData;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct __declspec(align(16)) PaddedVector : public Vector
	{
		PaddedVector &operator=(const PaddedVector &o) { x=o.x; y=o.y; z=o.z; return *this; }
		PaddedVector &operator=(const Vector &o) { x=o.x; y=o.y; z=o.z; return *this; }

		float pad;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef stdvector<PackedArgb>::fwd          PackedArgbVector;
	typedef stdvector<std::string const*>::fwd  StringVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_MAIN = TAG(M,A,I,N);

	std::string const cs_skinColorVariable("/shared_owner/index_color_skin");
	std::string const cs_privateColorVariable("/private/index_color_1");
	std::string const cs_indexColor0("index_color_0");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	PackedArgbVector  s_hueColors;
	StringVector      s_hueVariableNames;

	bool              s_useSSE;
}

using namespace SofwareBlendSkeletalShaderPrimitiveNamespace;

// ======================================================================

bool                SoftwareBlendSkeletalShaderPrimitive::ms_installed;
bool                SoftwareBlendSkeletalShaderPrimitive::ms_useMultiStreamVertexBuffers;

// ======================================================================

const int cs_maxTextureSetCount = 8;

// ======================================================================

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct SoftwareBlendSkeletalShaderPrimitive::TransformData
{
	int  m_transformIndex;
	real m_transformWeight;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct SoftwareBlendSkeletalShaderPrimitive::Dot3Vector
{
	Dot3Vector() {}
	Dot3Vector(const Vector &v, float f) : m_dot3Vector(v), m_flipState(f) {}

	Vector  m_dot3Vector;
	float   m_flipState;
};
#define SOURCE_DOT3_SIZE 16
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
struct SoftwareBlendSkeletalShaderPrimitive::SourceVertex
{
	TransformData m_firstTransformData;
	int           m_extraTransformDataCount;
	Vector        m_position;
	Vector        m_normal;
};
#define SOURCE_VERTEX_SIZE 36 // TODO: this is lame.  Need to figure out how to get the inline assembler to do this
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class SoftwareBlendSkeletalShaderPrimitive::RenderCommand
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	void render() const;

	static RenderCommand *createTriList(int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static RenderCommand *createTriStrip(int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount, bool flipCullMode);

private:

	typedef void (*DrawPrimitiveCommand)(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);

private:

	RenderCommand(int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount, bool flipCullMode, DrawPrimitiveCommand drawPrimitiveCommand);

private:

	DrawPrimitiveCommand  m_drawPrimitiveCommand;
	int                   m_minimumVertexBufferIndex;
	int                   m_numberOfVertices;
	int                   m_startIndexBufferIndex;
	int                   m_primitiveCount;
	bool                  m_flipCullMode;

private:
	// disabled
	RenderCommand();
};

// ======================================================================
struct fill_vb_work;

void _fillDot3VertexBufferHard_sse(const fill_vb_work *w);
void _fillVertexBufferHard_sse(const fill_vb_work *w);

struct fill_vb_work
{
	void construct(
		int                       i_vertexCount, 
		const SourceVertex       *i_sourceVectors, 
		const Dot3Vector         *i_sourceDot3Vectors, 
		const PoseModelTransform *i_transformArray,
		int                       i_vertexSize,
		byte                     *i_viter,
		byte                     *i_dot3viter
		)
	{
		vertexCount          = i_vertexCount;
		m_sourceVectors      = i_sourceVectors;
		m_sourceDot3Vectors  = i_sourceDot3Vectors;
		transformArray       = i_transformArray;
		vertexSize           = i_vertexSize;
		viter                = i_viter;
		dot3viter            = i_dot3viter;
		//------------------------------------------
		m_sourceVectorsEnd     = m_sourceVectors + vertexCount;
		m_sourceVectorPrefetch = ((uint8 *)m_sourceVectors) + 256;
		if (m_sourceDot3Vectors)
		{
			m_sourceDot3VectorsEnd = m_sourceDot3Vectors + vertexCount;
			m_sourceVectorDot3Prefetch = ((uint8 *)m_sourceDot3Vectors) + 256;
		}
		xf=0;
		identity.makeIdentity();
		//------------------------------------------
		minVector.set( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
		maxVector.set(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
		//------------------------------------------
	}

	void fillDot3VertexBufferHard() const;
	void fillVertexBufferHard() const;

#if TRY_FOR_SSE
	void fillDot3VertexBufferHard_sse() const { _fillDot3VertexBufferHard_sse(this); }
	void fillVertexBufferHard_sse() const { _fillVertexBufferHard_sse(this); }
#endif

	PoseModelTransform identity;

	mutable PaddedVector        position;
	mutable PaddedVector        normal;
	mutable PaddedVector        dot3;
	mutable PaddedVector        minVector;
	mutable PaddedVector        maxVector;
	mutable const PoseModelTransform *xf;
	mutable byte               *viter;
	mutable byte               *dot3viter;
	mutable const SourceVertex *m_sourceVectors;
	mutable const uint8        *m_sourceVectorPrefetch;
	mutable const Dot3Vector   *m_sourceDot3Vectors;
	mutable const uint8        *m_sourceVectorDot3Prefetch;

	const PoseModelTransform *transformArray;
	int                 vertexCount;
	const SourceVertex *m_sourceVectorsEnd;
	const Dot3Vector   *m_sourceDot3VectorsEnd;
	int                 vertexSize;
};

// ======================================================================

// ======================================================================
// class SoftwareBlendSkeletalShaderPrimitive::RenderCommand
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SoftwareBlendSkeletalShaderPrimitive::RenderCommand, true, 0, 0, 0);

// ======================================================================

SoftwareBlendSkeletalShaderPrimitive::RenderCommand *SoftwareBlendSkeletalShaderPrimitive::RenderCommand::createTriList(
	int minimumVertexIndex, 
	int numberOfVertices, 
	int startIndex, 
	int primitiveCount
	)
{
	DrawPrimitiveCommand dpc = Graphics::drawIndexedTriangleList;
	return new RenderCommand(minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, false, dpc);
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive::RenderCommand *SoftwareBlendSkeletalShaderPrimitive::RenderCommand::createTriStrip(
	int  minimumVertexIndex, 
	int  numberOfVertices, 
	int  startIndex, 
	int  primitiveCount, 
	bool flipCullMode
	)
{
	DrawPrimitiveCommand dpc = Graphics::drawIndexedTriangleStrip;
	return new RenderCommand(minimumVertexIndex, numberOfVertices, startIndex, primitiveCount, flipCullMode, dpc);
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive::RenderCommand::RenderCommand(int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount, bool flipCullMode, DrawPrimitiveCommand drawPrimitiveCommand) :
	m_drawPrimitiveCommand(drawPrimitiveCommand),
	m_minimumVertexBufferIndex(minimumVertexIndex),
	m_numberOfVertices(numberOfVertices),
	m_startIndexBufferIndex(startIndex),
	m_primitiveCount(primitiveCount),
	m_flipCullMode(flipCullMode)
{
}

// ----------------------------------------------------------------------

inline void SoftwareBlendSkeletalShaderPrimitive::RenderCommand::render() const
{
	//-- set the cull mode.  we do this because removing hidden triangles from a mesh strip can
	//   force us to toggle the cull mode.
	const GlCullMode cullMode = (m_flipCullMode ? GCM_clockwise : GCM_counterClockwise);
	Graphics::setCullMode(cullMode);

	NOT_NULL(m_drawPrimitiveCommand);
	(*m_drawPrimitiveCommand)(0, m_minimumVertexBufferIndex, m_numberOfVertices, m_startIndexBufferIndex, m_primitiveCount);
}

// ======================================================================
// class SoftwareBlendSkeletalShaderPrimitive
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(SoftwareBlendSkeletalShaderPrimitive, true, 0, 0, 0);

// ======================================================================

void SoftwareBlendSkeletalShaderPrimitive::install()
{
	DEBUG_FATAL(ms_installed, ("SoftwareBlendSkeletalShaderPrimitive already installed"));

	installMemoryBlockManager();
	RenderCommand::install();

	ms_useMultiStreamVertexBuffers = 
		   (Graphics::getMaximumVertexBufferStreamCount() > 1) 
		&& Graphics::supportsStreamOffsets()
		&& !ConfigClientGraphics::getDisableMultiStreamVertexBuffers()
		;

#if TRY_FOR_SSE
	s_useSSE = SseMath::canDoSseMath();
#else
	s_useSSE = false;
#endif

	ms_installed = true;
	ExitChain::add(remove, "SoftwareBlendSkeletalShaderPrimitive");
}

// ======================================================================

void SoftwareBlendSkeletalShaderPrimitive::remove()
{
	DEBUG_FATAL(!ms_installed, ("SoftwareBlendSkeletalShaderPrimitive not installed"));

	removeMemoryBlockManager();
}

// ======================================================================

SoftwareBlendSkeletalShaderPrimitive::SoftwareBlendSkeletalShaderPrimitive(class SkeletalAppearance2 &appearance, int lodIndex, const MeshConstructionHelper &mesh, int shaderIndex) :
	ShaderPrimitive(),
	m_shader(0),
	m_appearance(appearance),
	m_lodIndex(lodIndex),
	m_renderCommands(new RenderCommandVector()),
	m_dynamicStream(0),
	m_staticStream(0),
	m_systemStream(0),
	m_vertexBufferVector(0),
	m_indexBuffer(0),
	m_vertexCount(0),
	m_sourceData(0),
	m_sourceVectors(0),
	m_sourceDot3Vectors(0),
	m_transformData(0),
	m_shadowVolume(0),
	m_skinningMode(SM_softSkinning),
	m_hasBeenSkinned(false),
	m_hasDot3Vector(false),
	m_dot3TextureCoordinateSetIndex(-1),
	m_haveRepresentativeColor(false),
	m_representativeColor(PackedArgb::solidGray),
	m_boxExtent(),
	m_everyOtherFrameSkinningEnabled(false)
{
	DEBUG_FATAL(!ms_installed, ("SoftwareBlendSkeletalShaderPrimitive not installed"));

	_initialize(mesh, shaderIndex, ms_useMultiStreamVertexBuffers);

	//-- Assert post-initialization conditions.
	NOT_NULL(m_dynamicStream);
	NOT_NULL(m_shader);
	NOT_NULL(m_sourceData);
	NOT_NULL(m_sourceVectors);
	NOT_NULL(m_transformData);
	NOT_NULL(m_vertexBufferVector);
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive::~SoftwareBlendSkeletalShaderPrimitive()
{
	m_shader->release();
	m_shader = 0;

	delete [] m_sourceData;

	delete m_indexBuffer;
	delete m_staticStream;
	delete m_systemStream;
	delete m_dynamicStream;
	delete m_vertexBufferVector;

	std::for_each(m_renderCommands->begin(), m_renderCommands->end(), PointerDeleter());
	delete m_renderCommands;

	delete m_shadowVolume;
	m_shadowVolume = 0;
}

// ----------------------------------------------------------------------

float SoftwareBlendSkeletalShaderPrimitive::alter(real deltaTime)
{
	return m_shader->alter(deltaTime);
}

// ----------------------------------------------------------------------

const Vector SoftwareBlendSkeletalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w().getPosition_p();
}
	
// ----------------------------------------------------------------------

float SoftwareBlendSkeletalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_appearance.getTransform_w().getPosition_p().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int SoftwareBlendSkeletalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_dynamicStream->getSortKey();
}

// ----------------------------------------------------------------------

const StaticShader &SoftwareBlendSkeletalShaderPrimitive::prepareToView() const
{
	return m_shader->prepareToView();
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::prepareToDraw() const
{
	NOT_NULL(m_dynamicStream);

	NP_PROFILER_AUTO_BLOCK_DEFINE("SoftwareBlendSkeletalShaderPrimitive::prepareToDraw");

	//-- Set transform.
	const Transform &transform_apw = m_appearance.getTransform_w();
	Graphics::setObjectToWorldTransformAndScale(transform_apw, Vector::xyz111);

	//-- Get skeleton.
	const Skeleton &skeleton = m_appearance.getSkeleton(m_lodIndex);

	//-- Get transforms
	const int                 transformCount = skeleton.getTransformCount();
	const PoseModelTransform *transformArray = skeleton.getBindPoseModelToRootTransforms();

	//-- Setup which vertex buffers will be used for rendering and compute vertex buffer data.
	if (ms_useMultiStreamVertexBuffers)
	{
		// STATE: Using multi streaming.

		// Perform skinning on the system vertex buffer.  Collision and shadows require CPU
		// access to the position info.
		VertexBufferWriteIterator  writeIterator = m_systemStream->beginWriteOnly();
		skinData(transformCount, transformArray, writeIterator);

		m_dynamicStream->lock(m_vertexCount);
		{
			VertexBufferWriteIterator outIt = m_dynamicStream->begin();
			outIt.copy(m_systemStream->beginReadOnly(), m_systemStream->getNumberOfVertices());
		}
		m_dynamicStream->unlock();
	}
	else
	{
		// STATE: Not using multi streaming.
		// @todo -TRF- look at this: the following code path should not be legal.
		if (!m_systemStream)
		{
			// STATE: There's only skinned data for this primitive.  Skin directly to the dynamic
			//        vertex buffer.

			// Compute and fill the dynamic vertex buffer.
			m_dynamicStream->lock(m_vertexCount);
			{
				VertexBufferWriteIterator writeIterator = m_dynamicStream->begin();
				skinData(transformCount, transformArray, writeIterator);
			}
			m_dynamicStream->unlock();
		}
		else
		{
			// STATE: Not multi streaming and there's both constant and skinned data to be used
			//        for rendering.

			// Compute and fill the system vertex buffer with skinned data.  The system vertex buffer
			// also stores any non-skinned (static) data (e.g. UVs, color).
			VertexBufferWriteIterator  writeIterator = m_systemStream->beginWriteOnly();
			skinData(transformCount, transformArray, writeIterator);

			// Copy the system vertex buffer in one chunk into the dynamic vertex buffer that we render with.
			// This is how the geometry gets into a buffer readable by the GPU.  We can't render out of system
			// VBs, we don't want to store a GPU-renderable VB for every shader prim, and we do want a fast
			// single mem copy into the destination dynamic VB.  This is the solution: tradeoff memory by using
			// a fixed system VB for each shader prim.
			m_dynamicStream->lock(m_vertexCount);
			{
				VertexBufferWriteIterator outIt = m_dynamicStream->begin();
				outIt.copy(m_systemStream->beginReadOnly(), m_systemStream->getNumberOfVertices());
			}
			m_dynamicStream->unlock();
		}
	}

	//-- set the vb and ib to the shadow system
	if (ShadowManager::getEnabled() 
		&& ShadowManager::getAllowed() 
		&& ShadowManager::getSkeletalShadowsVolumetric() 
		&& ShadowVolume::getEnabled() 
		&& m_shader->getShaderTemplate().castsShadows() 
		&& (m_appearance.getFadeState() == SkeletalAppearance2::FS_notFading)
		&& (m_appearance.getHologramType() == SkeletalAppearance2::HT_none)
		&& !m_appearance.getIsBlueGlowie()
		&& !m_appearance.getIsHolonet()
		)
	{
		//-- Create shadow volume.
		if (!m_shadowVolume)
			m_shadowVolume = new ShadowVolume(m_shader->usesVertexShader () ? ShadowVolume::ST_vertexShader : ShadowVolume::ST_fixedFunction, ShadowVolume::PT_animating, m_appearance.getAppearanceTemplate()->getName());

		m_shadowVolume->addPrimitive(m_systemStream, m_indexBuffer);
		m_shadowVolume->render(m_appearance.getOwner(), &m_appearance);
	}
	else
	{
		if (m_shadowVolume)
		{
			delete m_shadowVolume;
			m_shadowVolume = 0;
		}
	}

	// Set the vertex buffer(s).
	Graphics::setVertexBuffer(*m_vertexBufferVector);

	// Set the index buffer.
	NOT_NULL(m_indexBuffer);
	Graphics::setIndexBuffer(*m_indexBuffer);
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("SoftwareBlendSkeletalShaderPrimitive::draw");
	std::for_each(m_renderCommands->begin(), m_renderCommands->end(), VoidMemberFunction(&RenderCommand::render));
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::setCustomizationData(CustomizationData *customizationData)
{
	NOT_NULL(m_shader);
	m_shader->setCustomizationData(customizationData);
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::addCustomizationVariables(CustomizationData &customizationData) const
{
	NOT_NULL(m_shader);
	m_shader->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------
/**
 * Calculate the bounding box coordinates for this blended mesh.
 *
 * Since this instance represents a mesh that is modified by transforms,
 * the bounding box will necessarily change when the transforms change.
 * Therefore, it makes no sense for this instance to try to cache this
 * data internally.  Users of this class that want a cheap bounding box
 * calculation will need to be smart about (1) how accurate the bounding
 * box must be and (2) how often they need to run over this data to
 * caculate it.
 *
 * Performance side-effects: this function will force the skeleton to evaluate 
 * its transforms if that event hasn't already happened.
 *
 * @param minVector  the minimum x, y and z value for the bounding box
 *                   are returned in this vector.
 * @param maxVector  the maximum x, y and z value for the bounding box
 *                   are returned in this vector.
 */

void SoftwareBlendSkeletalShaderPrimitive::calculateBoundingBox(Vector &minVector, Vector &maxVector) const
{
	//-- initialize min and max value
	minVector.set(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	maxVector.set(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	//-- get the current transform data from the appearance
	const Skeleton &skeleton = m_appearance.getSkeleton(m_lodIndex);

#ifdef _DEBUG
	const int              transformCount = skeleton.getTransformCount();
#endif
	const PoseModelTransform *const transformArray = skeleton.getBindPoseModelToRootTransforms();
	NOT_NULL(transformArray);

	const TransformData *transformData = m_transformData;

	//-- calculate transformed xyz, track min and max
	for (int vertexIndex = 0; vertexIndex < m_vertexCount; ++vertexIndex)
	{
		const SourceVertex &sourceVertex = m_sourceVectors[vertexIndex];
		const Vector &sourcePosition     = sourceVertex.m_position;

		//-- initialize first position with first influencing transform
		const TransformData &firstTransformData = sourceVertex.m_firstTransformData;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, firstTransformData.m_transformIndex, transformCount);

		const PoseModelTransform &firstTransform = transformArray[firstTransformData.m_transformIndex];

		Vector blendPosition = firstTransform.rotateTranslate_l2p(sourcePosition) * firstTransformData.m_transformWeight;

		//-- apply remaining transforms
		for (int j=sourceVertex.m_extraTransformDataCount;j>0;j--, transformData++)
		{
			// get transform data
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformData->m_transformIndex, transformCount);

			// apply it
			const PoseModelTransform &transform = transformArray[transformData->m_transformIndex];

			blendPosition += transform.rotateTranslate_l2p(sourcePosition) * transformData->m_transformWeight;
		}

		//-- apply blended position to min/max
		minVector.set(std::min(minVector.x, blendPosition.x), std::min(minVector.y, blendPosition.y), std::min(minVector.z, blendPosition.z));
		maxVector.set(std::max(maxVector.x, blendPosition.x), std::max(maxVector.y, blendPosition.y), std::max(maxVector.z, blendPosition.z));
	}
}

// ----------------------------------------------------------------------

SystemVertexBuffer const *SoftwareBlendSkeletalShaderPrimitive::getSkinnedVertexBuffer() const
{
	//-- Can't return anything if multi-streaming: no cached system VB data.
	if (ms_useMultiStreamVertexBuffers)
		return 0;

	//-- Do proper skinning, which may be to do no skinning.
	const Skeleton &skeleton = m_appearance.getSkeleton(m_lodIndex);

	NOT_NULL(m_systemStream);
	VertexBufferWriteIterator  writeIterator = m_systemStream->beginWriteOnly();
	skinData(skeleton.getTransformCount(), skeleton.getBindPoseModelToRootTransforms(), writeIterator);

	return m_systemStream;
}

// ----------------------------------------------------------------------

StaticIndexBuffer const *SoftwareBlendSkeletalShaderPrimitive::getIndexBuffer() const
{
	//-- Don't return anything if multi-streaming since we can't return VB data.
	if (ms_useMultiStreamVertexBuffers)
		return 0;

	return m_indexBuffer;
}

// ----------------------------------------------------------------------

PackedArgb SoftwareBlendSkeletalShaderPrimitive::getRepresentativeColor() const
{
	if (!m_haveRepresentativeColor)
	{
		m_representativeColor = computeRepresentativeColor();
		m_haveRepresentativeColor = true;
	}

	return m_representativeColor;
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::setEveryOtherFrameSkinningEnabled(bool enabled)
{
	m_everyOtherFrameSkinningEnabled = enabled;
}

// ----------------------------------------------------------------------

bool SoftwareBlendSkeletalShaderPrimitive::getEveryOtherFrameSkinningEnabled() const
{
	return m_everyOtherFrameSkinningEnabled;
}

// ----------------------------------------------------------------------

bool SoftwareBlendSkeletalShaderPrimitive::collide(const Vector &start_o, const Vector &end_o, CollisionInfo &result) const
{
	bool found = false;

	result.setPoint(end_o);

	NOT_NULL(m_shader);
	if (m_shader->getShaderTemplate().isCollidable() && m_boxExtent.intersect(start_o, result.getPoint()))
	{
		if (!m_systemStream)
			return false;

		NOT_NULL(m_indexBuffer);
		m_indexBuffer->lock();

			const Index *indices       = m_indexBuffer->beginReadOnly();
			const int    numberOfFaces = m_indexBuffer->getNumberOfIndices() / 3;
			const Vector dir           = end_o - start_o;

			Vector normal;
			Plane  plane;
			Vector intersection;

			NOT_NULL(m_systemStream);
			VertexBufferReadIterator readIt = m_systemStream->begin();

			int i;
			for (i = 0; i < numberOfFaces; i++)
			{
				const Vector &v0 = (readIt + *indices++).getPosition();
				const Vector &v1 = (readIt + *indices++).getPosition();
				const Vector &v2 = (readIt + *indices++).getPosition();

				//-- compute normal
				normal = (v0 - v2).cross(v1 - v0);
				normal *= 100.f;

				//-- skinning may have caused the face to become zero-area, so verify
				if (normal.magnitudeSquared() <= sqr(Vector::NORMALIZE_THRESHOLD))
					continue;

				//-- ignore backfaces
				if (dir.dot(normal) < 0.f)
				{
					//-- it doesn't matter that the normal is not normalized
					plane.set(normal, v0);

					//-- see if the end points intersect the plane the polygon lies on, lies within the polygon, and is closer to start than the previous point
					if ((plane.findIntersection(start_o, result.getPoint(), intersection)) &&
						(start_o.magnitudeBetweenSquared(intersection) < start_o.magnitudeBetweenSquared(result.getPoint())) &&
						(intersection.inPolygon(v0, v1, v2)))
					{
						found = true;

						IGNORE_RETURN(normal.normalize());

						result.setPoint(intersection);
						result.setNormal(normal);
						result.setTime(start_o.magnitudeBetween(intersection));
					}
				}
			}

		m_indexBuffer->unlock();
	}

	return found;
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive *SoftwareBlendSkeletalShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive()
{
	return this;
}

// ----------------------------------------------------------------------

SoftwareBlendSkeletalShaderPrimitive const *SoftwareBlendSkeletalShaderPrimitive::asSoftwareBlendSkeletalShaderPrimitive() const
{
	return this;
}

// ----------------------------------------------------------------------

void SoftwareBlendSkeletalShaderPrimitive::calculateSkinnedGeometryNow()
{
	//-- Get skeleton transform data.
	const Skeleton        &skeleton                 = m_appearance.getSkeleton(m_lodIndex);
	const PoseModelTransform *const transformArray  = skeleton.getBindPoseModelToRootTransforms();
	NOT_NULL(transformArray);
	const int              transformCount     = skeleton.getTransformCount();

	const TransformData *transformData        = m_transformData;

	UNREF(transformCount);

	//-- fill xyz and normal
	for (int i = 0; i < m_vertexCount; ++i)
	{
		const SourceVertex  &sourceVertex   = m_sourceVectors[i];
		const Vector        &sourcePosition = sourceVertex.m_position;

		// initialize first position and vector
		const TransformData &firstTransformData = sourceVertex.m_firstTransformData;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, firstTransformData.m_transformIndex, transformCount);
		const PoseModelTransform  &firstTransform = transformArray[firstTransformData.m_transformIndex];

		Vector blendPosition = firstTransform.rotateTranslate_l2p(sourcePosition) * firstTransformData.m_transformWeight;

		// apply remaining transforms
		for (int j=sourceVertex.m_extraTransformDataCount;j>0;j--, transformData++)
		{
			// get transform data
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformData->m_transformIndex, transformCount);

			// apply it
			const PoseModelTransform &transform = transformArray[transformData->m_transformIndex];
			blendPosition += transform.rotateTranslate_l2p(sourcePosition) * transformData->m_transformWeight;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Switch between soft skinning (multiple weights per vertex) and hard skinning (100% weighting to a single vertex).
 *
 * @useSoftSkinning  if true, soft skinning will be used; otherwise, hard skinning will be used.
 */

void SoftwareBlendSkeletalShaderPrimitive::setSkinningMode(SkinningMode skinningMode)
{
	m_skinningMode = skinningMode;
}

// ======================================================================
// class SoftwareBlendSkeletalShaderPrimitive: private member functions
// ======================================================================

void SoftwareBlendSkeletalShaderPrimitive::_initialize(const MeshConstructionHelper &mesh, int shaderIndex, bool multiStream)
{
	/*
		NOTES

		In multi stream rendering, the skinned components (position, normal,
		dot3 if present) are skinned directly into a dynamic vertex buffer.  The
		non-skinned, constant components (color and non-dot3 texture coordinates)
		are stored in a per-instance static buffer.

		In single stream rendering, the skinned components (position, normal,
		dot3 if present) are skinned in a common system vertex buffer.  The
		non-skinned, constant components (color and non-dot3 texture coordinates) are
		stored in an instance system buffer.  The dynamic vertex buffer
		contains the full set of vertex information and is constructed by
		merging skinned common system vertex buffer with the constant component
		system vertex buffer.
	*/

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- Setup index buffer and render commands.
	buildIndexBufferAndRenderCommands(mesh, shaderIndex);

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- Retrieve some common values from the mesh.
	const MeshConstructionHelper::PerShaderData *const meshPerShaderData = mesh.getPerShaderData(shaderIndex);
	const uint inputFormat = mesh.getVertexFormat(meshPerShaderData);
	const bool hasArgb     = (inputFormat & MeshConstructionHelper::SVF_argb) != 0;
	const int  tcSetCount  = mesh.getTextureCoordinateSets(meshPerShaderData);

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- Create the shader.
	m_shader = ShaderTemplateList::fetchModifiableShader(mesh.getShaderTemplateName(meshPerShaderData));

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- some debug checks
	DEBUG_FATAL((inputFormat & MeshConstructionHelper::SVF_xyz) == 0, ("input mesh does not include xyz data"));
	DEBUG_FATAL((inputFormat & MeshConstructionHelper::SVF_normal) == 0, ("support for non-normal skeletal meshes has been removed, re-export data with normals."));
	UNREF(inputFormat);

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- get # vertices
	m_vertexCount = mesh.getVertexCount(meshPerShaderData);
	DEBUG_FATAL(m_vertexCount < 1, ("there are no vertices!"));

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- Construct the constant static vertex buffer.  This format will contain
	//   color and non-dot3 texture coordinates.
	VertexBufferFormat  constantFormat;
	if (multiStream)
	{
		constantFormat.setColor0(hasArgb);
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	// Setup dot3 texture coordinate set info and stuff constant texture coordinate set info
	// into constantFormat.
	//
	// Count # non-dot3 texture coordinate sets.
	int constantTcSetCount = 0;
	for (int i = 0; i < tcSetCount; ++i)
	{
		const int dimensionality = mesh.getTextureCoordinateDimensionality(meshPerShaderData, i);
		if (dimensionality != 4)
		{
			++constantTcSetCount;
		}
		else
		{
			// This tc set is the dot3 vector.
			DEBUG_FATAL(m_hasDot3Vector, ("2 sets of 4d texture coordinates are not supported, detected at tc set index [%d] and [%d].", m_dot3TextureCoordinateSetIndex, i));
			DEBUG_FATAL(i != tcSetCount - 1, ("dot3 4d texture coordinates are only supported as the last texture coordinate set, but it is [%d] of [%d].", i + 1, tcSetCount));

			m_hasDot3Vector                 = true;
			m_dot3TextureCoordinateSetIndex = ms_useMultiStreamVertexBuffers ? 0 : i;
		}
	}

	// Set constant texture coordinate set info in the vertex buffer.
	if (constantTcSetCount > 0)
	{
		constantFormat.setNumberOfTextureCoordinateSets(constantTcSetCount);

		int constantTcIndex = 0;
		for (int j = 0; j < tcSetCount; ++j)
		{
			// Retrieve dimensionality.
			const int dimensionality = mesh.getTextureCoordinateDimensionality(meshPerShaderData, j);
			if (dimensionality != 4)
			{
				// Set this non-dot3 texture set into the constant VB format.
				constantFormat.setTextureCoordinateSetDimension(constantTcIndex, dimensionality);
				++constantTcIndex;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- Construct the dynamic vertex buffer.
	VertexBufferFormat  dynamicFormat;

	// Dynamic always contains position and normal.
	dynamicFormat.setPosition(true);
	dynamicFormat.setNormal(true);

	if (!multiStream)
	{
		dynamicFormat.setColor0(hasArgb);
		dynamicFormat.setNumberOfTextureCoordinateSets(tcSetCount);
		{
			for (int i = 0; i < tcSetCount; ++i)
			{
				const int dimensionality = mesh.getTextureCoordinateDimensionality(meshPerShaderData, i);
				dynamicFormat.setTextureCoordinateSetDimension(i, dimensionality);
			}
		}
	}
	else
	{
		if (m_hasDot3Vector)
		{
			dynamicFormat.setNumberOfTextureCoordinateSets(1);
			dynamicFormat.setTextureCoordinateSetDimension(0, 4);
		}
		
	}
	// Construct the dynamic vertex buffer.
	m_dynamicStream = new DynamicVertexBuffer(dynamicFormat);
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	// compute data block size
	int alloc_size=0;

	// account for source vectors (position, normal)
	int sourceVectorSize = m_vertexCount * sizeof(SourceVertex);
	alloc_size +=  sourceVectorSize + 31;

	// account for dot3 vectors (position, normal)
	int dot3VectorSize;
	if (m_hasDot3Vector)
	{
		dot3VectorSize = m_vertexCount * sizeof(Dot3Vector);
		alloc_size +=  dot3VectorSize + 31;
	}
	else
	{
		dot3VectorSize=0;
	}

	// account for transform weight data
	int vertexIndex;
	int numberOfExtraTransformWeightPairs=0;
	for (vertexIndex = 0; vertexIndex < m_vertexCount; ++vertexIndex)
	{
		const MeshConstructionHelper::VertexData &vertexData = mesh.getVertexData(meshPerShaderData, vertexIndex);
		const int localVertexTransformCount = mesh.getVertexTransformCount(vertexData);
		if (localVertexTransformCount>1)
		{
			numberOfExtraTransformWeightPairs += localVertexTransformCount-1;
		}
	}
	const int transformDataSize = numberOfExtraTransformWeightPairs * sizeof(TransformData);
	alloc_size += transformDataSize;
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	// alloc data block and partition
	m_sourceData = new byte[alloc_size];
	memset(m_sourceData, 0, alloc_size);
	byte *diter = (byte *)m_sourceData;

	// account for vector data
	diter=POINTER_ALIGN_32(diter); // align to 16 byte boundary.
	m_sourceVectors=(SourceVertex *)diter;
	diter+=sourceVectorSize;

	// account for dot3 data
	if (m_hasDot3Vector)
	{
		diter=POINTER_ALIGN_32(diter); // align to 16 byte boundary.
		m_sourceDot3Vectors=(Dot3Vector *)diter;
		diter+=dot3VectorSize;
	}

	// account for transform weight data
	m_transformData = (TransformData *)diter;
	diter += transformDataSize;
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	//-- copy positions, weights and normals
	TransformData *destTransformData = m_transformData;
	for (vertexIndex = 0; vertexIndex < m_vertexCount; ++vertexIndex)
	{
		const MeshConstructionHelper::VertexData &vertexData = mesh.getVertexData(meshPerShaderData, vertexIndex);

		SourceVertex &sourceVertex = m_sourceVectors[vertexIndex];

		// grab position
		sourceVertex.m_position = mesh.getPosition(vertexData);

		// grab normal
		sourceVertex.m_normal = mesh.getNormal(vertexData);

		// grab # transforms affecting vertex
		const int localVertexTransformCount = mesh.getVertexTransformCount(vertexData);
		DEBUG_WARNING(localVertexTransformCount < 1, ("invalid local vertex transform count [%d], appearance=[%s], object=[%s]", 
			localVertexTransformCount, 
			m_appearance.getAppearanceTemplateName() ? m_appearance.getAppearanceTemplateName() : "<NULL appearance template name>",
			(m_appearance.getOwner() && m_appearance.getOwner()->getObjectTemplateName()) ? m_appearance.getOwner()->getObjectTemplateName() : "<NULL object template name>"));

		if (localVertexTransformCount<1)
		{
			sourceVertex.m_firstTransformData.m_transformIndex = 0;
			sourceVertex.m_firstTransformData.m_transformWeight = 0;
		}
		else
		{
			const MeshConstructionHelper::TransformData *sourceTransformData;

			sourceTransformData = &mesh.getVertexTransformData(vertexData, 0);
			sourceVertex.m_firstTransformData.m_transformIndex = sourceTransformData->m_transformIndex;
			sourceVertex.m_firstTransformData.m_transformWeight = sourceTransformData->m_transformWeight;

			sourceVertex.m_extraTransformDataCount = localVertexTransformCount-1;
			
			// grab transform data
			for (int j = 1; j < localVertexTransformCount; ++j, ++destTransformData)
			{
				sourceTransformData = &mesh.getVertexTransformData(vertexData, j);
				destTransformData->m_transformIndex  = sourceTransformData->m_transformIndex;
				destTransformData->m_transformWeight = sourceTransformData->m_transformWeight;
			}
		}
	}

	WARNING(destTransformData-m_transformData != numberOfExtraTransformWeightPairs, ("logic error: TransfromData count [%d] != numberOfExtraTransformWeightPairs [%d]", destTransformData-m_transformData, numberOfExtraTransformWeightPairs));
	/////////////////////////////////////////////////////////////////////////////////////////

	IS_NULL(m_vertexBufferVector);
	if (multiStream)
	{
		///////////////////////////////////////////////////////////////////////
		//-- Fill the constant static vertex buffer.
		m_staticStream = new StaticVertexBuffer(constantFormat, m_vertexCount);
		m_staticStream->lock();
		VertexBufferWriteIterator outIt = m_staticStream->begin();
		fillConstantVertexBufferData(mesh, shaderIndex, outIt);
		m_staticStream->unlock();
		///////////////////////////////////////////////////////////////////////

		m_systemStream = new SystemVertexBuffer(dynamicFormat, m_vertexCount);

		m_vertexBufferVector = new VertexBufferVector(*m_staticStream, *m_dynamicStream);
	}
	else
	{
		///////////////////////////////////////////////////////////////////////
		//-- Construct and fill the system vertex buffer with constant data.
		//   We'll stuff both constant and per-frame-skinned geometry in the system vertex buffer
		//   so we can do a single memory copy to the dynamic VB used at render time.
		m_systemStream = new SystemVertexBuffer(dynamicFormat, m_vertexCount);
		VertexBufferWriteIterator outIt = m_systemStream->begin();
		fillConstantVertexBufferData(mesh, shaderIndex, outIt);
		///////////////////////////////////////////////////////////////////////

		//-- Track which vertex buffers we render with.  Non-multi-streaming always uses just the
		//   dynamic vertex buffer.
		m_vertexBufferVector = new VertexBufferVector(*m_dynamicStream);
	}
}

// ----------------------------------------------------------------------
struct IndexedTriangle
{
	IndexedTriangle() {}

	IndexedTriangle(int i_i0, int i_i1, int i_i2)
	{
		indeces[0]=i_i0;
		indeces[1]=i_i1;
		indeces[2]=i_i2;
	}

	bool operator<(const IndexedTriangle &o) const
	{
		if (indeces[0]<o.indeces[0])
		{
			return true;
		}
		else if (indeces[0]>o.indeces[0])
		{
			return false;
		}
		else if (indeces[1]<o.indeces[1])
		{
			return true;
		}
		else if (indeces[1]>o.indeces[1])
		{
			return false;
		}
		else
		{
			return indeces[2]<o.indeces[2];
		}

	}
	int indeces[3];
};

void SoftwareBlendSkeletalShaderPrimitive::buildIndexBufferAndRenderCommands(const MeshConstructionHelper &mesh, int perShaderDataIndex)
{
	//-- get mesh data for this shader
	const MeshConstructionHelper::PerShaderData *const meshPerShaderData = mesh.getPerShaderData(perShaderDataIndex);

	//-- figure out # indices we need
	const int indexCount = mesh.getNumberOfWeightedPrimitiveIndices(meshPerShaderData);
	m_indexBuffer        = new StaticIndexBuffer(indexCount);

	//-- reserve space for render commands
	const int triStripCount = mesh.getTriStripCount(meshPerShaderData);
	const int triListCount  = mesh.getTriListCount(meshPerShaderData);

	m_renderCommands->reserve(static_cast<size_t>(triStripCount + triListCount));

	//-- populate index buffer and construct primitive rendering commands
	m_indexBuffer->lock();

	std::vector<IndexedTriangle> tempIndexBuffer;
	tempIndexBuffer.reserve(indexCount);

		int          currentIndex = 0;
		Index *const indexData    = m_indexBuffer->begin();

		// handle tri lists
		{
			for (int triListIndex = 0; triListIndex < triListCount; ++triListIndex)
			{
				tempIndexBuffer.clear();

				//-- Get # triangles in the tri list.
				const MeshConstructionHelper::TriListHeader &triListHeader = mesh.getTriList(meshPerShaderData, triListIndex);
				const int                                    triCount      = mesh.getTriListTriangleCount(triListHeader);
				DEBUG_FATAL(triCount < 1, ("invalid tri count %d", triCount));

				const int firstTriListIndex = currentIndex;

				int index[3];

				//-- initialize vertex buffer index min/max
				Index  minVbIndex;
				Index  maxVbIndex;
				{
					mesh.getTriListTriangle(meshPerShaderData, triListHeader, 0, index[0], index[1], index[2]);
					minVbIndex = static_cast<Index>(index[0]);
					maxVbIndex = static_cast<Index>(index[0]);
				}			

				//-- copy index data
				for (int triIndex = 0; triIndex < triCount; ++triIndex)
				{
					// Retrieve index values for the tri.
					mesh.getTriListTriangle(meshPerShaderData, triListHeader, triIndex, index[0], index[1], index[2]);

					tempIndexBuffer.push_back(IndexedTriangle(index[0], index[1], index[2]));

					// Copy index values into the index buffer.
					for (int triVertIndex = 0; triVertIndex < 3; ++triVertIndex/*, ++currentIndex*/)
					{
						const Index indexValue  = static_cast<Index>(index[triVertIndex]);

						// keep track of min/max
						minVbIndex = std::min(minVbIndex, indexValue);
						maxVbIndex = std::max(maxVbIndex, indexValue);
					}
				}

				std::sort(tempIndexBuffer.begin(), tempIndexBuffer.end());

				for (unsigned i=0;i<tempIndexBuffer.size();i++)
				{
					const IndexedTriangle &itri = tempIndexBuffer[i];
					indexData[currentIndex++] = static_cast<Index>(itri.indeces[0]);
					indexData[currentIndex++] = static_cast<Index>(itri.indeces[1]);
					indexData[currentIndex++] = static_cast<Index>(itri.indeces[2]);
				}

				//-- create the primitive
				RenderCommand *const renderCommand = RenderCommand::createTriList(static_cast<int>(minVbIndex), static_cast<int>((maxVbIndex - minVbIndex) + 1), firstTriListIndex, triCount);
				m_renderCommands->push_back(renderCommand);
			}
		}

		// handle tri strips
		{
			for (int triStripIndex = 0; triStripIndex < triStripCount; ++triStripIndex)
			{
				const MeshConstructionHelper::TriStripHeader &triStripHeader = mesh.getTriStrip(meshPerShaderData, triStripIndex);
				const int vertexCount = mesh.getTriStripVertexCount(triStripHeader);
				DEBUG_FATAL(vertexCount < 3, ("invalid vertex count %d", vertexCount));

				const int firstTriStripIndex = currentIndex;

				//-- initialize vertex buffer index min/max
				Index  minVbIndex;
				Index  maxVbIndex;
				{
					const int vertexIndex = mesh.getTriStripVertexIndex(triStripHeader, 0);
					minVbIndex = static_cast<Index>(vertexIndex);
					maxVbIndex = static_cast<Index>(vertexIndex);
				}			

				//-- copy index data
				for (int triStripVertexIndex = 0; triStripVertexIndex < vertexCount; ++triStripVertexIndex, ++currentIndex)
				{
					// copy index values
					const Index indexValue  = static_cast<Index>(mesh.getTriStripVertexIndex(triStripHeader, triStripVertexIndex));
					indexData[currentIndex] = indexValue;

					// keep track of min/max
					minVbIndex = std::min(minVbIndex, indexValue);
					maxVbIndex = std::max(maxVbIndex, indexValue);
				}

				//-- create the primitive
				const bool flipCullMode            = mesh.getTriStripCullModeFlipped(triStripHeader);
				RenderCommand *const renderCommand = RenderCommand::createTriStrip(static_cast<int>(minVbIndex), static_cast<int>((maxVbIndex - minVbIndex) + 1), firstTriStripIndex, vertexCount, flipCullMode);
				m_renderCommands->push_back(renderCommand);
			}
		}

	/*
	if(ConfigClientSkeletalAnimation::getOptimizeSkinnedIndexBuffers())
	{
		
		DEBUG_FATAL(sizeof(Index) != sizeof(WORD), ("sizes not equal"));
		Graphics::optimizeIndexBuffer((WORD *)m_indexBuffer->begin(), m_indexBuffer->getNumberOfIndices());
	}
	*/

	m_indexBuffer->unlock();
	
}

// ----------------------------------------------------------------------
/**
 * Fills the specified VB iterator with color and non-dot3 texture coordinate
 * data and extracts this instance's dot3 data.
 */

void SoftwareBlendSkeletalShaderPrimitive::fillConstantVertexBufferData(const MeshConstructionHelper &mesh, int shaderIndex, VertexBufferWriteIterator &destVertexIt)
{
	const MeshConstructionHelper::PerShaderData *const meshPerShaderData = mesh.getPerShaderData(shaderIndex);
	const uint inputFormat = mesh.getVertexFormat(meshPerShaderData);
	const bool hasArgb     = (inputFormat & MeshConstructionHelper::SVF_argb) != 0;
	const int  tcSetCount  = mesh.getTextureCoordinateSets(meshPerShaderData);

	//-- Cache tc set dimensionality.
	int  tcSetDimensionality[cs_maxTextureSetCount];

	for (int i = 0; i < tcSetCount; ++i)
	{
		tcSetDimensionality[i] = mesh.getTextureCoordinateDimensionality(meshPerShaderData, i);
	}

	//-- Extract data for each vertex.
	float textureCoordinate[4];
	for (int vertexIndex = 0; vertexIndex < m_vertexCount; ++vertexIndex, ++destVertexIt)
	{
		const MeshConstructionHelper::VertexData &vertexData = mesh.getVertexData(meshPerShaderData, vertexIndex);

		// copy color
		if (hasArgb)
		{
			const PackedArgb &argb = mesh.getDiffuseColor(vertexData);
			destVertexIt.setColor0(argb.getArgb());
		}

		// copy texture data
		for (int tcSetIndex = 0; tcSetIndex < tcSetCount; ++tcSetIndex)
		{
			// Get texture set dimensionality.
			const int tcCount = tcSetDimensionality[tcSetIndex]; //lint !e644 !e771 // Warning -- Variable 'tcSetDimensionality' may not have been initialized // false

			// Get texture set values.
			switch (tcCount)
			{
			case 2:
				mesh.getTextureCoordinates(meshPerShaderData, vertexData, tcSetIndex, textureCoordinate[0], textureCoordinate[1]);
				destVertexIt.setTextureCoordinates(tcSetIndex, textureCoordinate[0], textureCoordinate[1]);
				break;

			case 4:
			{
				// Extract the dot3 coordinate.
				DEBUG_FATAL(!m_hasDot3Vector, ("we should not be receiving 4d texture coordinates."));
				mesh.getTextureCoordinates(meshPerShaderData, vertexData, tcSetIndex, textureCoordinate[0], textureCoordinate[1], textureCoordinate[2], textureCoordinate[3]);

				// Save it.
				NOT_NULL(m_sourceDot3Vectors);
				Dot3Vector &d3v = m_sourceDot3Vectors[vertexIndex];
				d3v.m_dot3Vector.x=textureCoordinate[0];
				d3v.m_dot3Vector.y=textureCoordinate[1];
				d3v.m_dot3Vector.z=textureCoordinate[2];
				d3v.m_flipState   =textureCoordinate[3];
			}	break;

			default:
				DEBUG_FATAL(true, ("not supported."));
			}
		}
	}
}

// ===========================================================================

void SoftwareBlendSkeletalShaderPrimitive::skinData(int transformCount, const PoseModelTransform *transformArray, VertexBufferWriteIterator &iterator) const
{
	//-- Initialize doSkinning to true only on the first render.
	bool doSkinning = true;

	if (m_hasBeenSkinned && m_everyOtherFrameSkinningEnabled)	
	{
		// We're doing every other frame skinning.  Figure out if 
		// we should skin for this appearance this frame.

		//-- If last bit of object id and frame number both are even 
		//   or both are odd, do the skinning.
		Object *const owner = m_appearance.getOwner();
		if (owner)
		{
			uint32 const ownerId     = static_cast<uint32>(owner->getNetworkId().getValue());
			uint32 const frameNumber =  static_cast<uint32>(Os::getNumberOfUpdates());

			doSkinning = (((ownerId & 0x01) ^ (frameNumber & 0x01)) == 0);
		}
	}

	if (!doSkinning)
	{
		return;
	}

	if (m_skinningMode==SM_noSkinning && m_hasBeenSkinned)
	{
		return;
	}

	//-- Handle skinning if we're going to do it.
	switch (m_skinningMode)
	{
		case SM_softSkinning:
		{
			fillVertexBuffer(transformCount, transformArray, iterator);
		} break;

		case SM_hardSkinning:
		case SM_noSkinning:
		{
			const VertexBufferDescriptor *vbi = iterator.getDescriptor();
			DEBUG_FATAL(!vbi, ("Vertex buffer iterator has no descriptor"));
		#ifdef _DEBUG
			DEBUG_FATAL(vbi->offsetPosition!=0, ("Vertex buffer has an unsupported format."));
			DEBUG_FATAL(vbi->offsetNormal!=12, ("Vertex buffer has an unsupported format."));
		#endif
			const int vertexSize = vbi->vertexSize;
			byte *viter = (byte *)iterator.getDataPointer();

			fill_vb_work *w = (fill_vb_work *)STACK_ALLOC_ALIGN_16(sizeof(fill_vb_work));
			memset(w, 0, sizeof(*w));

			if (m_hasDot3Vector)
			{
				NOT_NULL(m_sourceDot3Vectors);
				const int dot3Offset = vbi->offsetTextureCoordinateSet[m_dot3TextureCoordinateSetIndex];
				DEBUG_FATAL(dot3Offset<=0, ("Vertex buffer has an unsupported format."));

				w->construct(
					m_vertexCount, 
					m_sourceVectors, 
					m_sourceDot3Vectors, 
					transformArray, 
					vertexSize, 
					viter, 
					viter + dot3Offset
				);

			#if TRY_FOR_SSE
				if (s_useSSE)
				{
					w->fillDot3VertexBufferHard_sse();
				}
				else
			#endif
				{
					w->fillDot3VertexBufferHard();
				}
			}
			else
			{
				w->construct(
					m_vertexCount, 
					m_sourceVectors, 
					0, 
					transformArray, 
					vertexSize, 
					viter, 
					0
				);

			#if TRY_FOR_SSE
				if (s_useSSE)
				{
					w->fillVertexBufferHard_sse();
				}
				else
			#endif
				{
					w->fillVertexBufferHard();
				}
			}

			const Vector &minVector=w->minVector;
			const Vector &maxVector=w->maxVector;
			m_boxExtent.set(minVector, maxVector, (maxVector + minVector) * 0.5f, (maxVector - minVector).magnitude() * 0.5f);
		} break;
	}

	m_hasBeenSkinned = true;
}

// ----------------------------------------------------------------------
/**
 * Soft.
 */

void SoftwareBlendSkeletalShaderPrimitive::fillVertexBuffer(int transformCount, const PoseModelTransform *transformArray, VertexBufferWriteIterator &destVertexIt) const
{
	bool const updateExtentsThisFrame = (m_appearance.getRenderedFrameNumber() & 1) != 0;

	//-- We will be updating m_boxExtent every frame in fillDynamicVertexBuffer*.  
	//   m_boxExtent will be used to help speed up collision.
	if (updateExtentsThisFrame)
	{
		m_boxExtent.setNegative();
	}

	UNREF(transformCount);

	const SourceVertex *sourceVectors = m_sourceVectors;
	const TransformData *transformData = m_transformData;

	if (m_hasDot3Vector)
	{
		NOT_NULL(m_sourceDot3Vectors);

		const Dot3Vector *sourceDot3Vectors = m_sourceDot3Vectors;

		//-- Skin position, normal and dot3 vector.
		for (int i = 0; i < m_vertexCount; ++i, ++destVertexIt, sourceVectors++)
		{
			const Vector        &sourcePosition = sourceVectors->m_position;
			const Vector        &sourceNormal   = sourceVectors->m_normal;
			const Dot3Vector    &sourceDot3     = sourceDot3Vectors[i];

			// initialize first position and vector
			const TransformData &firstTransformData = sourceVectors->m_firstTransformData;
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, firstTransformData.m_transformIndex, transformCount);
			const PoseModelTransform     &firstTransform = transformArray[firstTransformData.m_transformIndex];

			Vector blendPosition = firstTransform.rotateTranslate_l2p(sourcePosition) * firstTransformData.m_transformWeight;
			Vector blendNormal   = firstTransform.rotate_l2p(sourceNormal)            * firstTransformData.m_transformWeight;
			Vector blendDot3     = firstTransform.rotate_l2p(sourceDot3.m_dot3Vector) * firstTransformData.m_transformWeight;

			// apply remaining transforms
			for (int j=sourceVectors->m_extraTransformDataCount;j>0;j--, transformData++)
			{
				// get transform data
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformData->m_transformIndex, transformCount);

				// apply it
				const PoseModelTransform &transform = transformArray[transformData->m_transformIndex];

				blendPosition += transform.rotateTranslate_l2p(sourcePosition) * transformData->m_transformWeight;
				blendNormal   += transform.rotate_l2p(sourceNormal)            * transformData->m_transformWeight;
				blendDot3     += transform.rotate_l2p(sourceDot3.m_dot3Vector) * transformData->m_transformWeight;
			}

			//-- Write out render geometry.
			destVertexIt.setPosition(blendPosition);
			destVertexIt.setNormal(blendNormal);
			destVertexIt.setTextureCoordinates(m_dot3TextureCoordinateSetIndex, blendDot3.x, blendDot3.y, blendDot3.z, sourceDot3.m_flipState);

#ifdef _DEBUG
			if (GraphicsDebugFlags::renderVertexMatrices)
			{
				_renderVertexMatrices(blendPosition, blendNormal, blendDot3, sourceDot3.m_flipState);
			}
#endif

			if (updateExtentsThisFrame && ((i & 1) != 0))
			{
				m_boxExtent.updateMinAndMax(blendPosition);
			}
		}
	}
	else
	{
		//-- Skin position and normal.
		for (int i = 0; i < m_vertexCount; ++i, ++destVertexIt, sourceVectors++)
		{
			const Vector        &sourcePosition = sourceVectors->m_position;
			const Vector        &sourceNormal   = sourceVectors->m_normal;

			// initialize first position and vector
			const TransformData &firstTransformData = sourceVectors->m_firstTransformData;
			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, firstTransformData.m_transformIndex, transformCount);
			const PoseModelTransform     &firstTransform = transformArray[firstTransformData.m_transformIndex];

			Vector blendPosition = firstTransform.rotateTranslate_l2p(sourcePosition) * firstTransformData.m_transformWeight;
			Vector blendNormal   = firstTransform.rotate_l2p(sourceNormal)            * firstTransformData.m_transformWeight;

			// apply remaining transforms
			for (int j=sourceVectors->m_extraTransformDataCount;j>0;j--, transformData++)
			{
				// get transform data
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, transformData->m_transformIndex, transformCount);

				// apply it
				const PoseModelTransform &transform = transformArray[transformData->m_transformIndex];
				blendPosition += transform.rotateTranslate_l2p(sourcePosition) * transformData->m_transformWeight;
				blendNormal   += transform.rotate_l2p(sourceNormal)            * transformData->m_transformWeight;
			}

			//-- Write out render geometry.
			destVertexIt.setPosition(blendPosition);
			destVertexIt.setNormal(blendNormal);

			if (updateExtentsThisFrame && ((i & 1) != 0))
			{
				m_boxExtent.updateMinAndMax(blendPosition);
			}
		}
	}

	if (updateExtentsThisFrame)
	{
		m_boxExtent.calculateCenterAndRadius();
	}
}

// ============================================================================
#if TRY_FOR_SSE
// ============================================================================
void _fillDot3VertexBufferHard_sse(const fill_vb_work *const w)
{
	uint32 mxcsrSave, mxcsrTemp;

	_asm {
		mov	esi, w // "this" pointer

		mov	eax, [esi]fill_vb_work.m_sourceVectors
		mov	ebx, [esi]fill_vb_work.m_sourceVectorsEnd
		mov	ecx, [esi]fill_vb_work.m_sourceDot3Vectors
		mov	edx, [esi]fill_vb_work.m_sourceDot3VectorsEnd

		sub	ebx, eax // length of sourceVectors array
		sub	edx, ecx // length of sourceDot3Vectors array

		// ----------------------------------------------------
		// check the length of sourceVectors array.  If it is
		// greater than 256, pre-fetch all the cache lines
		// for the first 256 bytes.
		cmp	      ebx, 256
		jl		      skip_vec_prefetch
		prefetchnta [eax+  0]
		prefetchnta [eax+ 32]
		prefetchnta [eax+ 64]
		prefetchnta [eax+ 96]
		prefetchnta [eax+128]
		prefetchnta [eax+160]
		prefetchnta [eax+192]
		prefetchnta [eax+224]
	skip_vec_prefetch:		
		// ----------------------------------------------------

		// ----------------------------------------------------
		// check the length of sourceDot3Vectors data array.  If it is
		// greater than 256, pre-fetch all the cache lines
		// for the first 256 bytes.
		cmp	      edx, 256
		jl		      skip_dot3_prefetch
		prefetchnta [ecx+  0]
		prefetchnta [ecx+ 32]
		prefetchnta [ecx+ 64]
		prefetchnta [ecx+ 96]
		prefetchnta [ecx+128]
		prefetchnta [ecx+160]
		prefetchnta [ecx+192]
		prefetchnta [ecx+224]
	skip_dot3_prefetch:
		// ----------------------------------------------------

		// ----------------------------------------------------
		// save the mxcsr register then mask exceptions and enable 
		// "flush-to_zero" mode.
		stmxcsr     mxcsrSave
		mov         eax, mxcsrSave
		or          eax, (MXCSR_FLUSH_TO_ZERO|MXCSR_PRECISION_MASK|MXCSR_UNDERFLOW_MASK|MXCSR_OVERFLOW_MASK|MXCSR_DENORMAL_MASK)
		mov         mxcsrTemp, eax
		ldmxcsr     mxcsrTemp
		// ----------------------------------------------------

		// ----------------------------------------------------
		//-- Skin position, normal and dot3.
		mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer
		jmp	main_loop_test
	main_loop_top:
			// ----------------------------------------------------
			// pre-fectch sourceVector data

			// check to make sure the pre-fetcher is at least 32 bytes 
			// from the end of the array.
			mov	eax, [esi]fill_vb_work.m_sourceVectorPrefetch
			mov	ebx, [esi]fill_vb_work.m_sourceVectorsEnd
			sub	ebx, eax
			cmp	ebx, 32
			jl		skip_vec_prefetch2
				// ----------------------------------------------
				// while the pre-fetcher is less than 256 bytes away
				// from the current source location, pre-fetch in
				// 32 byte (cache line) increments
				//
				// eax contains m_sourceVectorPrefetch from above
				mov	      ebx, [esi]fill_vb_work.m_sourceVectors
				lea	      ebx, [ebx + 256]	// pointer 256 bytes ahead of current source
				jmp	      vec_prefetch_loop
			vec_prefetch2:
				prefetchnta [eax]                 // pre-fetch
				lea			eax, [eax + 32]       // advance pre-fetcher by cache line
			vec_prefetch_loop:
				cmp         eax, ebx              // compare pre-fetch to source+256
				jl          vec_prefetch2         // if less, pre-fetch
				mov         [esi]fill_vb_work.m_sourceVectorPrefetch, eax // update pre-fetcher var
				// ----------------------------------------------
		skip_vec_prefetch2:
			// ----------------------------------------------------

			// ----------------------------------------------------
			// pre-fectch m_sourceVectorDot3 data

			// check to make sure the pre-fetcher is at least 32 bytes 
			// from the end of the array.
			mov	eax, [esi]fill_vb_work.m_sourceVectorDot3Prefetch
			mov	ebx, [esi]fill_vb_work.m_sourceDot3VectorsEnd
			sub	ebx, eax
			cmp	ebx, 32
			jl		skip_dot3_prefetch2

				// ----------------------------------------------
				// while the dot3 pre-fetcher is less than 256 bytes away
				// from the current source location, pre-fetch in
				// 32 byte (cache line) increments
				//
				// eax contains m_sourceVectorDot3Prefetch from above
				mov	      ebx, [esi]fill_vb_work.m_sourceDot3Vectors
				lea	      ebx, [ebx + 256]   	// pointer 256 bytes ahead of current source
				jmp	      dot3_prefetch_loop
			dot3_prefetch2:
				prefetchnta [eax]                 // pre-fetch
				lea			eax, [eax + 32]       // advance pre-fetcher by cache line
			dot3_prefetch_loop:
				cmp         eax, ebx              // compare pre-fetch to source+256
				jl          dot3_prefetch2         // if less, pre-fetch
				mov         [esi]fill_vb_work.m_sourceVectorDot3Prefetch, eax // update dot3 pre-fetcher var
				// ----------------------------------------------
		skip_dot3_prefetch2:
			// ----------------------------------------------------

			mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer

			// ----------------------------------------------------
			// Initialize first position and normal.
			mov         edi, [ebx]TransformData.m_transformIndex    // current vertex transform index
			shl         edi, 6                                      // current vertex transform offset
			mov         edx, [esi]fill_vb_work.transformArray       // transfrom array
			lea         eax, [edx + edi]
			// ----------------------------------------------------

			///////////////////////////////////////////////////////////////
			// rotateTranslate the source position
			//----------------------------------------------------------
			// rotateTranslate_l2p
			// eax -> 4x4 column-major float transform (16 byte aligned).
			// ebx -> source vertex
			movaps xmm0, [eax +  0]                          // column 0 (Cx_)
			movss  xmm4, [ebx +  0]SourceVertex.m_position   // source vector x
			movaps xmm1, [eax + 16]                          // column 1 (Cy_)
			movss  xmm5, [ebx +  4]SourceVertex.m_position   // source vector y
			movaps xmm2, [eax + 32]                          // column 2 (Cz_)
			movss  xmm6, [ebx +  8]SourceVertex.m_position   // source vector z
			movaps xmm3, [eax + 48]                          // column 3 (O_)

			shufps xmm4, xmm4, 0      // x -> x.x.x.x
			shufps xmm5, xmm5, 0      // y -> y.y.y.y
			shufps xmm6, xmm6, 0      // z -> z.z.z.z

			mulps  xmm0, xmm4         // col0 * x.x.x.x
			mulps  xmm1, xmm5         // col1 * y.y.y.y
			mulps  xmm2, xmm6         // col2 * z.z.z.z

			addps  xmm0, xmm1         // col0*x + col1*y
			addps  xmm2, xmm3         // col2*z + col3
			addps  xmm0, xmm2         // col0*x + col1*y + col2*z + col3

			movaps [esi]fill_vb_work.position, xmm0           // save result out
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////
			// update the bounding box with the position currently in xmm0
			movaps	xmm1, xmm0      // save off a copy of the position for the "min" test
			maxps		xmm0, [esi]fill_vb_work.maxVector
			minps		xmm1, [esi]fill_vb_work.minVector
			movaps	[esi]fill_vb_work.maxVector, xmm0
			movaps	[esi]fill_vb_work.minVector, xmm1
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////
			// rotate the source normal
			//----------------------------------------------------------
			// rotate_l2p
			// eax -> 4x4 column-major float transform (16 byte aligned).
			// ebx -> source vertex
			movaps xmm0, [eax +  0]                        // column 0 (Cx_)
			movss  xmm4, [ebx +  0]SourceVertex.m_normal   // source vector x
			movaps xmm1, [eax + 16]                        // column 1 (Cy_)
			movss  xmm5, [ebx +  4]SourceVertex.m_normal   // source vector y
			movaps xmm2, [eax + 32]                        // column 2 (Cz_)
			movss  xmm6, [ebx +  8]SourceVertex.m_normal   // source vector z

			shufps xmm4, xmm4, 0      // x -> x.x.x.x
			shufps xmm5, xmm5, 0      // y -> y.y.y.y
			shufps xmm6, xmm6, 0      // z -> z.z.z.z

			mulps  xmm0, xmm4         // col0 * x.x.x.x
			mulps  xmm1, xmm5         // col1 * y.y.y.y
			mulps  xmm2, xmm6         // col2 * z.z.z.z

			addps  xmm0, xmm1         // col0*x + col1*y
			addps  xmm0, xmm2         // col0*x + col1*y + col2*z

			movaps [esi]fill_vb_work.normal, xmm0           // save result out
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////
			// rotate the source dot3 vector
			mov	ebx, [esi]fill_vb_work.m_sourceDot3Vectors
			//----------------------------------------------------------
			// rotate_l2p
			// eax -> 4x4 column-major float transform (16 byte aligned).
			// ebx -> 3 float source vector
			movaps xmm0, [eax +  0]                          // column 0 (Cx_)
			movss  xmm4, [ebx +  0]Dot3Vector.m_dot3Vector   // source vector x
			movaps xmm1, [eax + 16]                          // column 1 (Cy_)
			movss  xmm5, [ebx +  4]Dot3Vector.m_dot3Vector   // source vector y
			movaps xmm2, [eax + 32]                          // column 2 (Cz_)
			movss  xmm6, [ebx +  8]Dot3Vector.m_dot3Vector   // source vector z

			shufps xmm4, xmm4, 0      // x -> x.x.x.x
			shufps xmm5, xmm5, 0      // y -> y.y.y.y
			shufps xmm6, xmm6, 0      // z -> z.z.z.z

			mulps  xmm0, xmm4         // col0 * x.x.x.x
			mulps  xmm1, xmm5         // col1 * y.y.y.y
			mulps  xmm2, xmm6         // col2 * z.z.z.z

			addps  xmm0, xmm1         // col0*x + col1*y
			addps  xmm0, xmm2         // col0*x + col1*y + col2*z

			movaps [esi]fill_vb_work.dot3, xmm0           // save result out
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////
			//-- Write out render geometry.
			//*(Vector *)(w->viter +  0) = w->position;
			//*(Vector *)(w->viter + 12) = w->normal;
			//*(float *)(w->dot3viter + 0) = w->dot3.x;
			//*(float *)(w->dot3viter + 4) = w->dot3.y;
			//*(float *)(w->dot3viter + 8) = w->dot3.z;
			//*(float *)(w->dot3viter +12) = w->m_sourceDot3Vectors->m_flipState;
			//
			// ebx contains m_sourceDot3Vectors from above

			mov         eax, [esi]fill_vb_work.dot3viter    // destination dot3 pointer
			mov         edi, [esi]fill_vb_work.viter        // destination pointer

			// position and normal
			mov         ecx, [esi + 0]fill_vb_work.position // xf'ed position.x
			mov         edx, [esi + 4]fill_vb_work.position // xf'ed position.y
			mov         [edi + 0], ecx
			mov         [edi + 4], edx
			mov         ecx, [esi + 8]fill_vb_work.position // xf'ed position.z
			mov         edx, [esi + 0]fill_vb_work.normal   // xf'ed normal.x
			mov         [edi + 8], ecx
			mov         [edi +12], edx
			mov         ecx, [esi + 4]fill_vb_work.normal   // xf'ed normal.y
			mov         edx, [esi + 8]fill_vb_work.normal   // xf'ed normal.z
			mov         [edi +16], ecx
			mov         [edi +20], edx

			// dot-3
			mov         ecx, [esi + 0]fill_vb_work.dot3     // xf'ed dot3.x
			mov         edx, [esi + 4]fill_vb_work.dot3     // xf'ed dot3.y
			mov         [eax + 0], ecx
			mov         [eax + 4], edx
			mov         ecx, [esi + 8]fill_vb_work.dot3     // xf'ed dot3.z
			mov         edx, [ebx]Dot3Vector.m_flipState    // source Dot3Vector flip-state
			mov         [eax + 8], ecx
			mov         [eax +12], edx
			///////////////////////////////////////////////////////////////

			// ------------------------------
			//w->m_sourceVectors++;
			//w->m_sourceDot3Vectors++;
			//w->viter+=w->vertexSize;
			//w->dot3viter+=w->vertexSize;
			//
			// eax contains dot3viter from above
			// edi contains viter from above
			add         eax, [esi]fill_vb_work.vertexSize
			add         edi, [esi]fill_vb_work.vertexSize
			mov         [esi]fill_vb_work.dot3viter, eax
			mov         [esi]fill_vb_work.viter, edi

			mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer
			mov         ecx, [esi]fill_vb_work.m_sourceDot3Vectors  // current dot3 pointer

			lea         ebx, BYTE PTR [ebx + SOURCE_VERTEX_SIZE]
			add         ecx, SOURCE_DOT3_SIZE

			mov         [esi]fill_vb_work.m_sourceVectors, ebx
			mov         [esi]fill_vb_work.m_sourceDot3Vectors, ecx
			// ------------------------------

		// end of main loop body
		// ------------------------------
	main_loop_test:
		//while (w->m_sourceVectors!=w->m_sourceVectorsEnd)
		cmp            ebx, [esi]fill_vb_work.m_sourceVectorsEnd
		jnz            main_loop_top

		// -----------------------------------
		// restore mxcsr
		ldmxcsr mxcsrSave
		// -----------------------------------
	}
	//--------------------------------------------------------------------------------------------
}
// ============================================================================
void _fillVertexBufferHard_sse(const fill_vb_work *const w)
{
	uint32 mxcsrSave, mxcsrTemp;

	_asm {
		mov	esi, w // "this" pointer

		mov	eax, [esi]fill_vb_work.m_sourceVectors
		mov	ebx, [esi]fill_vb_work.m_sourceVectorsEnd

		sub	ebx, eax // length of sourceVectors array

		// ----------------------------------------------------
		// check the length of sourceVectors array.  If it is
		// greater than 256, pre-fetch all the cache lines
		// for the first 256 bytes.
		cmp	      ebx, 256
		jl		      skip_vec_prefetch
		prefetchnta [eax+  0]
		prefetchnta [eax+ 32]
		prefetchnta [eax+ 64]
		prefetchnta [eax+ 96]
		prefetchnta [eax+128]
		prefetchnta [eax+160]
		prefetchnta [eax+192]
		prefetchnta [eax+224]
	skip_vec_prefetch:		
		// ----------------------------------------------------

		// ----------------------------------------------------
		// save the mxcsr register then mask exceptions and enable 
		// "flush-to_zero" mode.
		stmxcsr     mxcsrSave
		mov         eax, mxcsrSave
		or          eax, (MXCSR_FLUSH_TO_ZERO|MXCSR_PRECISION_MASK|MXCSR_UNDERFLOW_MASK|MXCSR_OVERFLOW_MASK|MXCSR_DENORMAL_MASK)
		mov         mxcsrTemp, eax
		ldmxcsr     mxcsrTemp
		// ----------------------------------------------------

		// ----------------------------------------------------
		//-- Skin position, normal and dot3.
		mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer
		jmp	      main_loop_test
	main_loop_top:
			// ----------------------------------------------------
			// pre-fectch sourceVector data

			// check to make sure the pre-fetcher is at least 32 bytes 
			// from the end of the array.
			mov	eax, [esi]fill_vb_work.m_sourceVectorPrefetch
			mov	ebx, [esi]fill_vb_work.m_sourceVectorsEnd
			sub	ebx, eax
			cmp	ebx, 32
			jl		skip_vec_prefetch2
				// ----------------------------------------------
				// while the pre-fetcher is less than 256 bytes away
				// from the current source location, pre-fetch in
				// 32 byte (cache line) increments
				//
				// eax contains m_sourceVectorPrefetch from above
				mov	      ebx, [esi]fill_vb_work.m_sourceVectors
				lea	      ebx, [ebx + 256]	// pointer 256 bytes ahead of current source
				jmp	      vec_prefetch_loop
			vec_prefetch2:
				prefetchnta [eax]                 // pre-fetch
				lea			eax, [eax + 32]       // advance pre-fetcher by cache line
			vec_prefetch_loop:
				cmp         eax, ebx              // compare pre-fetch to source+256
				jl          vec_prefetch2         // if less, pre-fetch
				mov         [esi]fill_vb_work.m_sourceVectorPrefetch, eax // update pre-fetcher var
				// ----------------------------------------------
		skip_vec_prefetch2:
			// ----------------------------------------------------

			mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer

			// ----------------------------------------------------
			// Initialize first position and normal.
			mov         edi, [ebx]TransformData.m_transformIndex    // current vertex transform index
			shl         edi, 6                                      // current vertex transform offset
			mov         edx, [esi]fill_vb_work.transformArray       // transfrom array
			lea         eax, [edx + edi]
			// ----------------------------------------------------

			///////////////////////////////////////////////////////////////
			// rotateTranslate the source position
			//----------------------------------------------------------
			// rotateTranslate_l2p
			// eax -> 4x4 column-major float transform (16 byte aligned).
			// ebx -> source vertex
			movaps xmm0, [eax +  0]                          // column 0 (Cx_)
			movss  xmm4, [ebx +  0]SourceVertex.m_position   // source vector x
			movaps xmm1, [eax + 16]                          // column 1 (Cy_)
			movss  xmm5, [ebx +  4]SourceVertex.m_position   // source vector y
			movaps xmm2, [eax + 32]                          // column 2 (Cz_)
			movss  xmm6, [ebx +  8]SourceVertex.m_position   // source vector z
			movaps xmm3, [eax + 48]                          // column 3 (O_)

			shufps xmm4, xmm4, 0      // x -> x.x.x.x
			shufps xmm5, xmm5, 0      // y -> y.y.y.y
			shufps xmm6, xmm6, 0      // z -> z.z.z.z

			mulps  xmm0, xmm4         // col0 * x.x.x.x
			mulps  xmm1, xmm5         // col1 * y.y.y.y
			mulps  xmm2, xmm6         // col2 * z.z.z.z

			addps  xmm0, xmm1         // col0*x + col1*y
			addps  xmm2, xmm3         // col2*z + col3
			addps  xmm0, xmm2         // col0*x + col1*y + col2*z + col3

			movaps [esi]fill_vb_work.position, xmm0           // save result out
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////
			// update the bounding box with the position currently in xmm0
			movaps	xmm1, xmm0      // save off a copy of the position for the "min" test
			maxps		xmm0, [esi]fill_vb_work.maxVector
			minps		xmm1, [esi]fill_vb_work.minVector
			movaps	[esi]fill_vb_work.maxVector, xmm0
			movaps	[esi]fill_vb_work.minVector, xmm1
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////
			// rotate the source normal
			//----------------------------------------------------------
			// rotate_l2p
			// eax -> 4x4 column-major float transform (16 byte aligned).
			// ebx -> source vertex
			movaps xmm0, [eax +  0]                        // column 0 (Cx_)
			movss  xmm4, [ebx +  0]SourceVertex.m_normal   // source vector x
			movaps xmm1, [eax + 16]                        // column 1 (Cy_)
			movss  xmm5, [ebx +  4]SourceVertex.m_normal   // source vector y
			movaps xmm2, [eax + 32]                        // column 2 (Cz_)
			movss  xmm6, [ebx +  8]SourceVertex.m_normal   // source vector z

			shufps xmm4, xmm4, 0      // x -> x.x.x.x
			shufps xmm5, xmm5, 0      // y -> y.y.y.y
			shufps xmm6, xmm6, 0      // z -> z.z.z.z

			mulps  xmm0, xmm4         // col0 * x.x.x.x
			mulps  xmm1, xmm5         // col1 * y.y.y.y
			mulps  xmm2, xmm6         // col2 * z.z.z.z

			addps  xmm0, xmm1         // col0*x + col1*y
			addps  xmm0, xmm2         // col0*x + col1*y + col2*z

			movaps [esi]fill_vb_work.normal, xmm0           // save result out
			///////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////
			//-- Write out render geometry.
			//*(Vector *)(w->viter +  0) = w->position;
			//*(Vector *)(w->viter + 12) = w->normal;
			mov         edi, [esi]fill_vb_work.viter        // destination pointer

			// position and normal
			mov         ecx, [esi + 0]fill_vb_work.position // xf'ed position.x
			mov         edx, [esi + 4]fill_vb_work.position // xf'ed position.y
			mov         [edi + 0], ecx
			mov         [edi + 4], edx
			mov         ecx, [esi + 8]fill_vb_work.position // xf'ed position.z
			mov         edx, [esi + 0]fill_vb_work.normal   // xf'ed normal.x
			mov         [edi + 8], ecx
			mov         [edi +12], edx
			mov         ecx, [esi + 4]fill_vb_work.normal   // xf'ed normal.y
			mov         edx, [esi + 8]fill_vb_work.normal   // xf'ed normal.z
			mov         [edi +16], ecx
			mov         [edi +20], edx
			///////////////////////////////////////////////////////////////

			// ------------------------------
			//w->m_sourceVectors++;
			//w->viter+=w->vertexSize;
			//
			// edi contains viter from above
			add         edi, [esi]fill_vb_work.vertexSize
			mov         [esi]fill_vb_work.viter, edi

			mov         ebx, [esi]fill_vb_work.m_sourceVectors      // current vertex pointer
			lea         ebx, BYTE PTR [ebx + SOURCE_VERTEX_SIZE]
			mov         [esi]fill_vb_work.m_sourceVectors, ebx
			// ------------------------------

		// end of main loop body
		// ------------------------------
	main_loop_test:
		//while (w->m_sourceVectors!=w->m_sourceVectorsEnd)
		cmp            ebx, [esi]fill_vb_work.m_sourceVectorsEnd
		jnz            main_loop_top

		// -----------------------------------
		// restore mxcsr
		ldmxcsr mxcsrSave
		// -----------------------------------
	}
	//--------------------------------------------------------------------------------------------
}
// ============================================================================
#endif

void fill_vb_work::fillDot3VertexBufferHard() const
{
	//--------------------------------------------------------------------------------------------
	//-- Skin position, normal and dot3.
	while (m_sourceVectors!=m_sourceVectorsEnd)
	{
		// Initialize first position and normal.
		const TransformData &firstTransformData = m_sourceVectors->m_firstTransformData;
		xf = transformArray + firstTransformData.m_transformIndex;

		position = xf->rotateTranslate_l2p(m_sourceVectors->m_position);
		normal   = xf->rotate_l2p(m_sourceVectors->m_normal);
		dot3     = xf->rotate_l2p(m_sourceDot3Vectors->m_dot3Vector);

		if (position.x>maxVector.x) { maxVector.x=position.x; }
		if (position.y>maxVector.y) { maxVector.y=position.y; }
		if (position.z>maxVector.z) { maxVector.z=position.z; }
		if (position.x<minVector.x) { minVector.x=position.x; }
		if (position.y<minVector.y) { minVector.y=position.y; }
		if (position.z<minVector.z) { minVector.z=position.z; }

		//-- Write out render geometry.
		*(Vector *)(viter +  0) = position;
		*(Vector *)(viter + 12) = normal;
		*(float *)(dot3viter + 0) = dot3.x;
		*(float *)(dot3viter + 4) = dot3.y;
		*(float *)(dot3viter + 8) = dot3.z;
		*(float *)(dot3viter +12) = m_sourceDot3Vectors->m_flipState;

		// ------------------------------
		m_sourceVectors++;
		m_sourceDot3Vectors++;
		viter+=vertexSize;
		dot3viter+=vertexSize;
		// ------------------------------
	}
}

void fill_vb_work::fillVertexBufferHard() const
{
	//--------------------------------------------------------------------------------------------
	//-- Skin position, normal and dot3.
	while (m_sourceVectors!=m_sourceVectorsEnd)
	{
		// Initialize first position and normal.
		const TransformData &firstTransformData = m_sourceVectors->m_firstTransformData;
		xf = transformArray + firstTransformData.m_transformIndex;

		position = xf->rotateTranslate_l2p(m_sourceVectors->m_position);
		normal   = xf->rotate_l2p(m_sourceVectors->m_normal);

		if (position.x>maxVector.x) { maxVector.x=position.x; }
		if (position.y>maxVector.y) { maxVector.y=position.y; }
		if (position.z>maxVector.z) { maxVector.z=position.z; }
		if (position.x<minVector.x) { minVector.x=position.x; }
		if (position.y<minVector.y) { minVector.y=position.y; }
		if (position.z<minVector.z) { minVector.z=position.z; }

		//-- Write out render geometry.
		*(Vector *)(viter +  0) = position;
		*(Vector *)(viter + 12) = normal;

		// ------------------------------
		m_sourceVectors++;
		viter+=vertexSize;
		// ------------------------------
	}
}

// ============================================================================

// ----------------------------------------------------------------------
#ifdef _DEBUG
void SoftwareBlendSkeletalShaderPrimitive::_renderVertexMatrices(const Vector &position, const Vector &normal, const Vector &dot3Normal, float dot3Flip)
{
	Vector pos = position;
	Vector i(dot3Normal.x, dot3Normal.y, dot3Normal.z);
	Vector k = normal;
	Vector j = k.cross(i) * dot3Flip;
	float scale = 0.01f;

	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, pos, pos+(i*scale), PackedArgb::solidRed));
	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, pos, pos+(j*scale), PackedArgb::solidGreen));
	ShaderPrimitiveSorter::getCurrentCamera().addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, pos, pos+(k*scale), (dot3Flip >= 0) ? PackedArgb::solidBlue : PackedArgb::solidCyan));
}
#endif
// ----------------------------------------------------------------------

PackedArgb SoftwareBlendSkeletalShaderPrimitive::computeRepresentativeColor() const
{
#ifdef _DEBUG
	PackedArgb color(PackedArgb::solidYellow);
#else
	PackedArgb color(PackedArgb::solidGray);
#endif

	//-- First check for a customizable shader.  We'll take the color as the first hue color.
	bool getTextureColor = true;

	CustomizableShader const *customizableShader = dynamic_cast<CustomizableShader const*>(m_shader);
	if (!customizableShader)
	{
		//-- Check for owner proxy shader.
		OwnerProxyShader const *const opShader = dynamic_cast<OwnerProxyShader const*>(m_shader);
		if (opShader)
		{
			//-- Try to get a customizable shader from the op shader.
			customizableShader = dynamic_cast<CustomizableShader const*>(opShader->getRealShader());
		}
	}

	if (customizableShader)
	{
		//-- Grab the list of hue colors provided by the customizable shader.
		bool const hasHueColors  = customizableShader->getHueInfo(s_hueVariableNames, s_hueColors);
		int        hueColorCount = static_cast<int>(s_hueVariableNames.size());

		if (hasHueColors)
		{
			//-- Initialize the hue color.
			int hueColorIndex       = 0;
			int selectedColorNumber = std::numeric_limits<int>::max();

			// Skip index_color_0 used for non-color info.
			if (*NON_NULL(s_hueVariableNames[static_cast<size_t>(hueColorIndex)]) == cs_indexColor0)
				++hueColorIndex;

			if (hueColorIndex < hueColorCount)
			{
				color = s_hueColors[static_cast<size_t>(hueColorIndex)];
				getTextureColor = false;

				//-- Now loop through all the colors
				for (; hueColorIndex < hueColorCount; ++hueColorIndex)
				{
					// Find the position of the last underscore in the variable name.
					std::string const *variableName = s_hueVariableNames[static_cast<size_t>(hueColorIndex)];
					NOT_NULL(variableName);

					std::string::size_type lastUnderscorePosition = variableName->rfind('_');
					if (static_cast<int>(lastUnderscorePosition) != static_cast<int>(std::string::npos))
					{
						// Find the int value of the string after the last underscore.
						int       colorNumber    = 0;
						int const convertedCount = sscanf(variableName->c_str() + lastUnderscorePosition + 1, "%d", &colorNumber);
						if ((convertedCount == 1) && (colorNumber > 0))
						{
							if (colorNumber < selectedColorNumber)
							{
								selectedColorNumber = colorNumber;
								color               = s_hueColors[static_cast<size_t>(hueColorIndex)];
							}
						}
					}
				}
			}
#if 0
			DEBUG_REPORT_LOG(true, ("SBSSP: using hue variable [%s] with color [%d,%d,%d].\n", s_hueVariableNames[0]->c_str(), s_hueColors[0].getR(), s_hueColors[0].getG(), s_hueColors[0].getB()));
#endif
		}
	}

	if (getTextureColor)
	{
		//-- Get representative color from MAIN texture in shader.
		StaticShader const &staticShader = m_shader->prepareToView();
		if (staticShader.hasTexture(TAG_MAIN))
		{
			Texture const *texture = 0;
			if (staticShader.getTexture(TAG_MAIN, texture) && texture)
			{
				// Strategy: get main texture color, modulate against hue or skin color.
				color = texture->getRepresentativeColor();
			}
		}
		else
		{
			// No texture.
#ifdef _DEBUG
			if (ConfigSharedObject::getLogCustomizationDataIssues())
			{
				char const *objectName = "<NULL name>";
				if (m_appearance.getOwner())
				{
					if (m_appearance.getOwner()->getObjectTemplateName())
						objectName = m_appearance.getOwner()->getObjectTemplateName();
					else if (m_appearance.getOwner()->getDebugName())
						objectName = m_appearance.getOwner()->getDebugName();
				}

				DEBUG_REPORT_LOG(true, ("No MAIN texture tag for prim:shader[%s],appearance[%s],object[%s].\n", 
					m_shader->getName() ? m_shader->getName() : "<NULL name>",
					m_appearance.getAppearanceTemplateName() ? m_appearance.getAppearanceTemplateName() : "<NULL name>",
					objectName));
			}					
#endif
		}
	}

	//-- Darken up the resulting color.  This seems to be necessary by empirical observation.
	color = PackedArgb(VectorArgb::linearInterpolate(VectorArgb(color), VectorArgb::solidBlack, 0.09f));

	return color;
}

// ======================================================================
