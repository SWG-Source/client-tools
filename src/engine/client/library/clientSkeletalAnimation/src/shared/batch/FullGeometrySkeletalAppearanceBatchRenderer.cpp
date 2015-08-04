// ======================================================================
//
// FullGeometrySkeletalAppearanceBatchRenderer.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "FullGeometrySkeletalAppearanceBatchRenderer.h"

#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SoftwareBlendSkeletalShaderPrimitive.h"
#include "clientTextureRenderer/TextureRendererShaderPrimitive.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/PackedArgb.h"
#include "sharedObject/Object.h"

#include <vector>

// ======================================================================

namespace FullGeometrySkeletalAppearanceBatchRendererNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<SystemVertexBuffer const*>   VertexBufferVector;
	typedef std::vector<StaticIndexBuffer const*>    IndexBufferVector;
	typedef std::vector<Transform const*>            TransformVector;
	typedef std::vector<PackedArgb>                  PackedArgbVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class LocalShaderPrimitive: public ShaderPrimitive
	{
	public:

		virtual const StaticShader &prepareToView() const;
		virtual const Vector        getPosition_w() const;
		virtual float               getDepthSquaredSortKey() const;
		virtual int                 getVertexBufferSortKey() const;
		virtual void                prepareToDraw() const;
		virtual void                draw() const;

	private:

		Vector getReasonablePosition_w() const;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	void getBufferCounts(int beginIndex, int endIndex, int &vertexCount, int &indexCount);
	int  findEndBufferForCount(int beginIndex, int destVerticesAvailable);
	void renderBuffers(bool allowDiscard, int beginIndex, int endIndex);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	FullGeometrySkeletalAppearanceBatchRenderer *s_instance;
	bool                                         s_installed;

	VertexBufferVector                           s_vertexBuffers;
	IndexBufferVector                            s_indexBuffers;
	TransformVector                              s_transformVector_a2w;
	PackedArgbVector                             s_colors;

	LocalShaderPrimitive                         s_batchShaderPrimitive;
	StaticShader const                          *s_batchShader;
	DynamicVertexBuffer                         *s_batchVertexBuffer;
	DynamicIndexBuffer                          *s_batchIndexBuffer;
}

using namespace FullGeometrySkeletalAppearanceBatchRendererNamespace;

// ======================================================================
// namespace FullGeometrySkeletalAppearanceBatchRendererNamespace
// ======================================================================

void FullGeometrySkeletalAppearanceBatchRendererNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("FullGeometrySkeletalAppearanceBatchRendererNamespace not installed."));
	s_installed = false;

	//-- Clear the appearance vector.
	VertexBufferVector().swap(s_vertexBuffers);
	IndexBufferVector().swap(s_indexBuffers);
	TransformVector().swap(s_transformVector_a2w);
	PackedArgbVector().swap(s_colors);

	delete s_instance;
	s_instance = 0;

	delete s_batchVertexBuffer;
	s_batchVertexBuffer = 0;

	delete s_batchIndexBuffer;
	s_batchIndexBuffer = 0;

	s_batchShader = 0;
}

// ----------------------------------------------------------------------

void FullGeometrySkeletalAppearanceBatchRendererNamespace::getBufferCounts(int beginIndex, int endIndex, int &vertexCount, int &indexCount)
{
	vertexCount = 0;
	indexCount  = 0;

	for (int bufferIndex = beginIndex; bufferIndex < endIndex; ++bufferIndex)
	{
		//-- Handle VB.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_vertexBuffers.size()));
		SystemVertexBuffer const *const sourceVb = s_vertexBuffers[static_cast<size_t>(bufferIndex)];
		NOT_NULL(sourceVb);

		vertexCount += sourceVb->getNumberOfVertices();

		//-- Handle IB.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_indexBuffers.size()));
		StaticIndexBuffer const *const sourceIb = s_indexBuffers[static_cast<size_t>(bufferIndex)];
		NOT_NULL(sourceIb);

		indexCount += sourceIb->getNumberOfIndices();
	}
}

// ----------------------------------------------------------------------

int FullGeometrySkeletalAppearanceBatchRendererNamespace::findEndBufferForCount(int beginIndex, int destVerticesAvailable)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, beginIndex, static_cast<int>(s_vertexBuffers.size()));
	DEBUG_FATAL(destVerticesAvailable < 0, ("invalid destVerticesAvailable count [%d].", destVerticesAvailable));

	int endIndex = beginIndex;
	int const bufferCount = static_cast<int>(s_vertexBuffers.size());
	int indexCount = 0;

	do
	{
		//-- Get # vertices in source vb.
		SystemVertexBuffer const *const sourceVb = s_vertexBuffers[static_cast<size_t>(endIndex)];
		NOT_NULL(sourceVb);

		int const vbVertexCount = sourceVb->getNumberOfVertices();
		destVerticesAvailable -= vbVertexCount;

		//-- Get # indices in source ib.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, endIndex, static_cast<int>(s_indexBuffers.size()));
		StaticIndexBuffer const *const sourceIb = s_indexBuffers[static_cast<size_t>(endIndex)];
		NOT_NULL(sourceIb);		

		indexCount += sourceIb->getNumberOfIndices();

		if ((destVerticesAvailable > 0) && (indexCount <= 32768))
		{
			// endIndex points one past the last vertex buffer index to include.
			++endIndex;
		}

	} while ((destVerticesAvailable > 0) && (endIndex < bufferCount) && (indexCount <= 32768));

	return endIndex;
}

// ----------------------------------------------------------------------

void FullGeometrySkeletalAppearanceBatchRendererNamespace::renderBuffers(bool allowDiscard, int beginIndex, int endIndex)
{
	//-- Get # vertices/indices to lock.
	int  vertexCount       = 0;
	int  indexCount        = 0;
	int  copiedVertexCount = 0;

	getBufferCounts(beginIndex, endIndex, vertexCount, indexCount);
	DEBUG_FATAL(vertexCount < 1, ("skel mesh batcher: bad vertex count [%d].", vertexCount));
	DEBUG_FATAL(indexCount < 1, ("skel mesh batcher: bad index count [%d].", indexCount));

	//-- Lock down dest buffers.
	NOT_NULL(s_batchVertexBuffer);
	s_batchVertexBuffer->lock(vertexCount, allowDiscard);
	VertexBufferWriteIterator destVbIt = s_batchVertexBuffer->begin();

	NOT_NULL(s_batchIndexBuffer);
	s_batchIndexBuffer->lock(indexCount);
	Index *destIbIt = s_batchIndexBuffer->begin();

	//-- Add each vertex buffer's data.
	for (int bufferIndex = beginIndex; bufferIndex < endIndex; ++bufferIndex)
	{
		//-- Get source buffers.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_vertexBuffers.size()));
		SystemVertexBuffer const *const sourceVb = s_vertexBuffers[static_cast<size_t>(bufferIndex)];
		NOT_NULL(sourceVb);

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_indexBuffers.size()));
		StaticIndexBuffer const *const sourceIb = s_indexBuffers[static_cast<size_t>(bufferIndex)];
		NOT_NULL(sourceIb);

		//-- Get source to world transform.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_transformVector_a2w.size()));
		Transform const *transform_a2w = s_transformVector_a2w[static_cast<size_t>(bufferIndex)];
		NOT_NULL(transform_a2w);

		//-- Get color.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, bufferIndex, static_cast<int>(s_colors.size()));
		PackedArgb const color = s_colors[static_cast<size_t>(bufferIndex)];

		//-- Lock applicable source buffers.
		sourceIb->lockReadOnly();

		//-- Copy vertex buffer data.
		int const sourceVertexCount = sourceVb->getNumberOfVertices();
		VertexBufferReadIterator sourceVbEndIt = sourceVb->endReadOnly();
		for (VertexBufferReadIterator sourceVbIt = sourceVb->beginReadOnly(); sourceVbIt != sourceVbEndIt; ++sourceVbIt, ++destVbIt)
		{
			//-- Transform positions and normals from object space to world space.
			destVbIt.setPosition(transform_a2w->rotateTranslate_l2p(sourceVbIt.getPosition()));
			destVbIt.setNormal(transform_a2w->rotate_l2p(sourceVbIt.getNormal()));
			
			// @todo get color from appropriate spot.
			destVbIt.setColor0(color);
		}

		//-- Copy adjusted indices.
		Index const *const sourceIbEndIt = sourceIb->endReadOnly();
		for (Index const *sourceIbIt = sourceIb->beginReadOnly(); sourceIbIt != sourceIbEndIt; ++sourceIbIt, ++destIbIt)
			*destIbIt = static_cast<unsigned short>(*sourceIbIt + copiedVertexCount);

		//-- Unlock applicable source buffers.
		sourceIb->unlock();

		//-- Update base vertex adjustment.
		copiedVertexCount += sourceVertexCount;
	}

	//-- Unlock dest buffers.
	s_batchIndexBuffer->unlock();
	s_batchVertexBuffer->unlock();

	//-- Render.
	Graphics::setVertexBuffer(*s_batchVertexBuffer);
	Graphics::setIndexBuffer(*s_batchIndexBuffer);
	Graphics::drawIndexedTriangleList();
}

// ======================================================================
// Batcher Shader Primitive
// ======================================================================

const StaticShader &FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::prepareToView() const
{
	NOT_NULL(s_batchShader);
	return s_batchShader->prepareToView();
}

// ----------------------------------------------------------------------

const Vector FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::getPosition_w() const
{
	return getReasonablePosition_w();
}

// ----------------------------------------------------------------------

float FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return getReasonablePosition_w().magnitudeBetweenSquared(ShaderPrimitiveSorter::getCurrentCameraPosition());
}

// ----------------------------------------------------------------------

int FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return s_batchVertexBuffer->getSortKey();
}

// ----------------------------------------------------------------------

void FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::prepareToDraw() const
{
	// Vertices are in world space, so set object to world to identity.
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
}

// ----------------------------------------------------------------------

void FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::draw() const
{
	if (s_vertexBuffers.empty())
	{
		// Nothing to do.  Most likely case is that we have a bogus appearance with no or non-constructable shader primitives.
		return;
	}

	DEBUG_FATAL(s_vertexBuffers.empty(), ("Full geometry batch renderer shader prim: nothing in batch."));
	DEBUG_FATAL((s_vertexBuffers.size() != s_indexBuffers.size()) || (s_vertexBuffers.size() != s_transformVector_a2w.size()) || (s_vertexBuffers.size() != s_colors.size()), ("Batch renderer buffers out of sync, should be identical sizes [%d/%d/%d/%d].", static_cast<int>(s_vertexBuffers.size()), static_cast<int>(s_indexBuffers.size()), static_cast<int>(s_transformVector_a2w.size()), static_cast<int>(s_colors.size())));
	NOT_NULL(s_batchVertexBuffer);
	NOT_NULL(s_batchIndexBuffer);

	int const bufferCount = static_cast<int>(s_vertexBuffers.size());
	bool      allowDiscard;
	int beginIndex = 0;
	int endIndex   = 0;	

	// Copy as much geometry into the dynamic VB/IB as possible, then render.
	do
	{
		//-- Find the vertex buffers to copy.

		// Check how many VBs we can copy if we don't discard dynamic VBs (more efficient dynamic VB usage).
		int destVerticesAvailable = s_batchVertexBuffer->getNumberOfLockableDynamicVertices(false);
		endIndex = findEndBufferForCount(beginIndex, destVerticesAvailable);

		// Handle case where we can't fit even a single source VB entirely into the dynamic draw VB.
		if (endIndex <= beginIndex)
		{
			// Check how many VBs we can copy if we do allow discarding first.
			destVerticesAvailable = s_batchVertexBuffer->getNumberOfLockableDynamicVertices(true);
			endIndex = findEndBufferForCount(beginIndex, destVerticesAvailable);
			if (endIndex <= beginIndex)
			{
				// A fatal here indicates there was a single source character VB that could not fit in an entirely
				// empty dynamic VB.  One fix would be to implement source VB splitting.  Another is to increase
				// dynamic VB size.
				DEBUG_FATAL(true, ("cannot fit entire batched skeletal appearance VB into dynamic batch render VB."));
				return; //lint !e527 // Unreachable // Reachable in release.
			}

			allowDiscard = true;
		}
		else
			allowDiscard = false;


		//-- Fill in the specified buffers.
		renderBuffers(allowDiscard, beginIndex, endIndex);

		//-- Start next loop rendering at next buffer.
		beginIndex = endIndex;

	} while (endIndex < bufferCount);

	//-- Cleanup.
	s_vertexBuffers.clear();
	s_indexBuffers.clear();
	s_transformVector_a2w.clear();
	s_colors.clear();
}

// ======================================================================
/** 
 * Return a reasonable world position for all batched appearances.
 */

Vector FullGeometrySkeletalAppearanceBatchRendererNamespace::LocalShaderPrimitive::getReasonablePosition_w() const
{
	DEBUG_FATAL(s_transformVector_a2w.empty(), ("full geometry batch renderer shader prim: nothing in batch."));
	return NON_NULL(s_transformVector_a2w.front())->getPosition_p();
}

// ======================================================================
// class FullGeometrySkeletalAppearanceBatchRenderer: PUBLIC STATIC
// ======================================================================

void FullGeometrySkeletalAppearanceBatchRenderer::install()
{
	DEBUG_FATAL(s_installed, ("FullGeometrySkeletalAppearanceBatchRenderer already installed."));

	//-- Setup the vertex buffer format for the batched data.
	VertexBufferFormat  format;
	format.setPosition(true);
	format.setNormal(true);
	format.setColor0(true);

	s_batchVertexBuffer = new DynamicVertexBuffer(format);
	s_batchIndexBuffer  = new DynamicIndexBuffer();

	s_instance    = new FullGeometrySkeletalAppearanceBatchRenderer();
	s_batchShader = &ShaderTemplateList::get3dVertexColorLZStaticShader();

	s_installed = true;
	ExitChain::add(remove, "FullGeometrySkeletalAppearanceBatchRenderer");
}

// ----------------------------------------------------------------------

FullGeometrySkeletalAppearanceBatchRenderer *FullGeometrySkeletalAppearanceBatchRenderer::getInstance()
{
	NOT_NULL(s_instance);
	return s_instance;
}

// ======================================================================
// class FullGeometrySkeletalAppearanceBatchRenderer: PUBLIC
// ======================================================================

void FullGeometrySkeletalAppearanceBatchRenderer::submit(SkeletalAppearance2 const &appearance)
{
	int submitCount            = 0;
	bool const wasEmptyOnStart = s_vertexBuffers.empty();

	//-- Add appearance shader primitives to list for batching.
	const Transform &transform_a2w = appearance.getTransform_w();

	SkeletalAppearance2::ShaderPrimitiveVector const &shaderPrimitives = appearance.getDisplayLodShaderPrimitives();
	SkeletalAppearance2::ShaderPrimitiveVector::const_iterator const endIt = shaderPrimitives.end();
	for (SkeletalAppearance2::ShaderPrimitiveVector::const_iterator it = shaderPrimitives.begin(); it != endIt; ++it)
	{
		if (*it)
		{
			// @todo make derived class that has necessary functions.  OR make texture renderers a derived class of Texture and
			//       get rid of the other tr shader primitive.

			// Get skeletal shader primitive.
			SoftwareBlendSkeletalShaderPrimitive const *sbPrimitive = (*it)->asSoftwareBlendSkeletalShaderPrimitive();

			// Collect data from the shader primitive.
			if (sbPrimitive)
			{
				SystemVertexBuffer const *const vb = sbPrimitive->getSkinnedVertexBuffer();
				StaticIndexBuffer const *const  ib = sbPrimitive->getIndexBuffer();
				PackedArgb const color             = sbPrimitive->getRepresentativeColor();

				if (vb && ib)
				{
					s_vertexBuffers.push_back(vb);
					s_indexBuffers.push_back(ib);
					s_transformVector_a2w.push_back(&transform_a2w);
					s_colors.push_back(color);

					++submitCount;
				}
			}
			else
			{
				DEBUG_FATAL(true, ("Could not resolve shader primitive to a skeletal shader primitive."));
			}
		}
	}

		//-- Add the shader primitive if nothing has been submitted successfully and this call contained at least one submission.
	if (wasEmptyOnStart && (submitCount > 0))
	{
		ShaderPrimitiveSorter::add(s_batchShaderPrimitive);
	}
}

// ======================================================================
// class FullGeometrySkeletalAppearanceBatchRenderer: PRIVATE STATIC
// ======================================================================

// ======================================================================
// class FullGeometrySkeletalAppearanceBatchRenderer: PRIVATE
// ======================================================================

FullGeometrySkeletalAppearanceBatchRenderer::FullGeometrySkeletalAppearanceBatchRenderer() :
	SkeletalAppearanceBatchRenderer()
{
}

// ======================================================================
