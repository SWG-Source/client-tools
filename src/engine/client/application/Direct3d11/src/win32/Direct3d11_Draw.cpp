// ======================================================================
//
// Direct3d11_Draw.cpp
// copyright (c) 2026 Galaxies Reborn
//
// Binding geometry, and the state a draw reads from it.
//
// The engine binds a vertex buffer in one scope and issues the draw in a later,
// separate call, and the buffer object it bound is frequently a stack local that
// has been destroyed by then. So binding does not defer anything: every value a
// draw needs is snapshotted here into six pieces of slice state, exactly as DX9
// does, and nothing is read back from an engine object at draw time.
//
// One genuine difference from DX9 in the shape of the bind. DX9 could set its
// vertex declaration here, because a declaration depends only on the vertex
// format. A D3D11 input layout is validated against specific shader bytecode, so
// it cannot be built until the vertex shader is also known -- which happens later,
// when the static shader data binds a pass. The formats are therefore recorded
// here and the layout is resolved in prepareToDraw.
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11.h"

#include "Direct3d11_ConstantBuffers.h"
#include "Direct3d11_Device.h"
#include "Direct3d11_DynamicIndexBufferData.h"
#include "Direct3d11_DynamicVertexBufferData.h"
#include "Direct3d11_InputLayoutCache.h"

#include "clientGraphics/StaticShader.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_LightManager.h"
#include "Direct3d11_PointSprite.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_Transforms.h"
#include "Direct3d11_StaticIndexBufferData.h"
#include "Direct3d11_StaticVertexBufferData.h"

#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferVector.h"

#include <vector>

// ======================================================================

namespace Direct3d11_DrawNamespace
{
	// The six pieces of slice state, mirroring Direct3d9.cpp:388-395. Nothing else
	// in the draw path is stateful.
	int ms_sliceNumberOfVertices;
	int ms_sliceFirstVertex;

	ID3D11Buffer *ms_savedIndexBuffer;
	int ms_sliceNumberOfIndices;
	int ms_sliceFirstIndex;

	// What is actually on the context, as opposed to what the slice state says the next draw wants.
	// The format is always R16_UINT and the offset always zero, so the buffer identity is the whole
	// of the binding and a pointer compare is a complete redundancy test. Every site that calls
	// IASetIndexBuffer -- here, the two primitive-emulation paths, and a device ClearState -- has to
	// keep this in step, or a skipped bind draws through the previous buffer.
	ID3D11Buffer *ms_boundIndexBuffer;

	// How many streams were bound last time. Still needed, and for a sharper
	// reason than in DX9: binding stream 0 does not unbind streams 1..N, and in
	// D3D11 a stale stream referenced by a later input layout is a hard error where
	// D3D9 merely wasted a slot.
	int ms_lastVertexBufferCount;

	// What the bound streams look like, for the layout the draw will need.
	uint32 ms_streamFormatFlags[Direct3d11_InputLayoutCache::MAX_STREAMS];
	int ms_streamCount;

	void const *ms_currentVertexShaderBytecode;
	unsigned int ms_currentVertexShaderBytecodeSize;
	uint32 ms_currentVertexShaderSignatureHash;

	// Which vertex buffer texture coordinate set each of the bound program's tags reads, in
	// declaration order. This is what DX9 compiled into the shader as a texture coordinate
	// key; here the program is compiled once and the input layout does the routing, so the
	// mapping travels with the bind instead. A count of zero means the program addresses its
	// sets by number rather than by tag.
	int ms_currentTextureCoordinateSetMapping[Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS];
	int ms_currentTextureCoordinateSetMappingCount;

	bool ms_warnedAboutMissingVertexShader;
	// The two emulation index buffers, and how many primitives each currently covers.
	ID3D11Buffer *ms_fanIndexBuffer;
	int ms_fanIndexBufferVertices;

	ID3D11Buffer *ms_quadIndexBuffer;
	int ms_quadIndexBufferQuads;

	bool ms_warnedAboutIndexedFan;

	bool ensureFanIndexBuffer(int vertexCount);
	bool ensureQuadIndexBuffer(int quadCount);
} // namespace Direct3d11_DrawNamespace
using namespace Direct3d11_DrawNamespace;

// ======================================================================

void Direct3d11::setVertexBuffer(HardwareVertexBuffer const &vertexBuffer)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	ID3D11Buffer *buffer = NULL;
	UINT stride = 0;

	// The dynamic ring's offset is carried in the slice's first vertex, NOT in the
	// stream's byte offset -- exactly as DX9 does it, where byteOffset is
	// initialised to zero and never changed on this path. Applying it in both
	// places double-counts it, which renders another mesh's geometry.
	UINT byteOffset = 0;

	switch (vertexBuffer.getType())
	{
		case HardwareVertexBuffer::T_static:
		{
			StaticVertexBuffer const *const staticVertexBuffer = safe_cast<StaticVertexBuffer const *>(&vertexBuffer);
			Direct3d11_StaticVertexBufferData *const data = safe_cast<Direct3d11_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

#ifdef _DEBUG
			// Not optional bookkeeping. StaticVertexBuffer::lock has a heuristic
			// that warns when a buffer is locked often relative to how often it
			// is set; without this increment the denominator stays zero and the
			// warning fires spuriously on every buffer written more than twenty
			// times.
			++staticVertexBuffer->m_sets;
#endif

			buffer = data->getBuffer();
			stride = static_cast<UINT>(data->getVertexSize());

			// Count comes from the ENGINE object here, and from the DATA object on
			// the dynamic path. That asymmetry is DX9's and it is correct: a static
			// buffer's size is fixed at construction, while a dynamic buffer's
			// usable count is whatever the last unlock claimed.
			ms_sliceNumberOfVertices = staticVertexBuffer->getNumberOfVertices();
			ms_sliceFirstVertex = 0;
			ms_streamFormatFlags[0] = staticVertexBuffer->getFormat().getFlags();
		}
		break;

		case HardwareVertexBuffer::T_dynamic:
		{
			DynamicVertexBuffer const *const dynamicVertexBuffer = safe_cast<DynamicVertexBuffer const *>(&vertexBuffer);
			Direct3d11_DynamicVertexBufferData *const data = safe_cast<Direct3d11_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

			buffer = Direct3d11_DynamicVertexBufferData::getRing();
			stride = static_cast<UINT>(data->getVertexSize());

			ms_sliceNumberOfVertices = data->getNumberOfVertices();
			ms_sliceFirstVertex = data->getOffset();
			ms_streamFormatFlags[0] = dynamicVertexBuffer->getFormat().getFlags();
		}
		break;

		default:
			DEBUG_FATAL(true, ("Direct3d11: unknown vertex buffer type %d", static_cast<int>(vertexBuffer.getType())));
			return;
	}

	ms_streamCount = 1;

	context->IASetVertexBuffers(0, 1, &buffer, &stride, &byteOffset);
	++Direct3d11_Metrics::vertexBufferBindCalls;
	++Direct3d11_Metrics::vertexBufferBindMisses;

	// Unbind whatever a previous multi-stream bind left behind.
	for (int stream = 1; stream < ms_lastVertexBufferCount; ++stream)
	{
		ID3D11Buffer *nothing = NULL;
		UINT const zero = 0;
		context->IASetVertexBuffers(static_cast<UINT>(stream), 1, &nothing, &zero, &zero);
	}

	ms_lastVertexBufferCount = 1;
}

// ----------------------------------------------------------------------
/**
 * Bind several streams at once.
 *
 * D3D9 had to choose between putting a dynamic stream's ring offset in the stream
 * offset or in the slice, because per-stream offsets were an optional capability,
 * and it hard-failed when a static stream at offset zero was mixed with a dynamic
 * stream at a nonzero one. D3D11 always supports per-stream offsets, so the mixed
 * case the skinned path creates just works, and the offset goes in the stream where
 * it belongs -- with the slice's first vertex left at zero so it is applied once.
 */

void Direct3d11::setVertexBufferVector(VertexBufferVector const &vertexBufferVector)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	NOT_NULL(vertexBufferVector.m_vertexBufferList);

	ms_sliceNumberOfVertices = 0;
	ms_sliceFirstVertex = 0;

	ID3D11Buffer *buffers[Direct3d11_InputLayoutCache::MAX_STREAMS];
	UINT strides[Direct3d11_InputLayoutCache::MAX_STREAMS];
	UINT offsets[Direct3d11_InputLayoutCache::MAX_STREAMS];

	int stream = 0;

	for (VertexBufferVector::VertexBufferList::const_iterator i = vertexBufferVector.m_vertexBufferList->begin(); i != vertexBufferVector.m_vertexBufferList->end(); ++i)
	{
		HardwareVertexBuffer const *const vertexBuffer = *i;
		NOT_NULL(vertexBuffer);

		FATAL(stream >= Direct3d11_InputLayoutCache::MAX_STREAMS, ("Direct3d11: a vertex buffer vector bound more than %d streams.", static_cast<int>(Direct3d11_InputLayoutCache::MAX_STREAMS)));

		switch (vertexBuffer->getType())
		{
			case HardwareVertexBuffer::T_static:
			{
				StaticVertexBuffer const *const staticVertexBuffer = safe_cast<StaticVertexBuffer const *>(vertexBuffer);
				Direct3d11_StaticVertexBufferData *const data = safe_cast<Direct3d11_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

#ifdef _DEBUG
				++staticVertexBuffer->m_sets;
#endif

				buffers[stream] = data->getBuffer();
				strides[stream] = static_cast<UINT>(data->getVertexSize());
				offsets[stream] = 0;

				ms_streamFormatFlags[stream] = staticVertexBuffer->getFormat().getFlags();

				if (!ms_sliceNumberOfVertices)
					ms_sliceNumberOfVertices = staticVertexBuffer->getNumberOfVertices();
			}
			break;

			case HardwareVertexBuffer::T_dynamic:
			{
				DynamicVertexBuffer const *const dynamicVertexBuffer = safe_cast<DynamicVertexBuffer const *>(vertexBuffer);
				Direct3d11_DynamicVertexBufferData *const data = safe_cast<Direct3d11_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

				buffers[stream] = Direct3d11_DynamicVertexBufferData::getRing();
				strides[stream] = static_cast<UINT>(data->getVertexSize());

				// Here, and only here, the ring offset rides in the stream offset.
				offsets[stream] = static_cast<UINT>(data->getOffset() * data->getVertexSize());

				ms_streamFormatFlags[stream] = dynamicVertexBuffer->getFormat().getFlags();

				if (!ms_sliceNumberOfVertices)
					ms_sliceNumberOfVertices = data->getNumberOfVertices();
			}
			break;

			default:
				DEBUG_FATAL(true, ("Direct3d11: unknown vertex buffer type in a vector"));
				return;
		}

		++stream;
	}

	FATAL(!stream, ("Direct3d11: a vertex buffer vector bound no streams at all."));

	context->IASetVertexBuffers(0, static_cast<UINT>(stream), buffers, strides, offsets);
	++Direct3d11_Metrics::vertexBufferBindCalls;
	++Direct3d11_Metrics::vertexBufferBindMisses;

	for (int stale = stream; stale < ms_lastVertexBufferCount; ++stale)
	{
		ID3D11Buffer *nothing = NULL;
		UINT const zero = 0;
		context->IASetVertexBuffers(static_cast<UINT>(stale), 1, &nothing, &zero, &zero);
	}

	ms_streamCount = stream;
	ms_lastVertexBufferCount = stream;
}

// ----------------------------------------------------------------------

void Direct3d11::setIndexBuffer(HardwareIndexBuffer const &indexBuffer)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	switch (indexBuffer.getType())
	{
		case HardwareIndexBuffer::T_static:
		{
			StaticIndexBuffer const *const staticIndexBuffer = safe_cast<StaticIndexBuffer const *>(&indexBuffer);
			Direct3d11_StaticIndexBufferData *const data = safe_cast<Direct3d11_StaticIndexBufferData *>(staticIndexBuffer->m_graphicsData);

			ms_savedIndexBuffer = data->getBuffer();
			ms_sliceNumberOfIndices = staticIndexBuffer->getNumberOfIndices();
			ms_sliceFirstIndex = 0;
		}
		break;

		case HardwareIndexBuffer::T_dynamic:
		{
			DynamicIndexBuffer const *const dynamicIndexBuffer = safe_cast<DynamicIndexBuffer const *>(&indexBuffer);
			Direct3d11_DynamicIndexBufferData *const data = safe_cast<Direct3d11_DynamicIndexBufferData *>(dynamicIndexBuffer->m_graphicsData);

			ms_savedIndexBuffer = Direct3d11_DynamicIndexBufferData::getRing();
			ms_sliceNumberOfIndices = data->getNumberOfIndices();

			// In INDICES, consumed as the draw's start index -- which is why the
			// byte offset handed to IASetIndexBuffer below stays zero.
			ms_sliceFirstIndex = data->getOffset();
		}
		break;

		default:
			DEBUG_FATAL(true, ("Direct3d11: unknown index buffer type %d", static_cast<int>(indexBuffer.getType())));
			return;
	}

	// Always R16_UINT: Index is an unsigned short throughout the engine.
	++Direct3d11_Metrics::indexBufferBindCalls;
	if (ms_savedIndexBuffer != ms_boundIndexBuffer)
	{
		context->IASetIndexBuffer(ms_savedIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		ms_boundIndexBuffer = ms_savedIndexBuffer;
		++Direct3d11_Metrics::indexBufferBindMisses;
	}
}

// ----------------------------------------------------------------------

void Direct3d11::forgetIndexBuffer(void *buffer)
{
	if (!buffer || ms_boundIndexBuffer == buffer)
		ms_boundIndexBuffer = NULL;
}

// ======================================================================

void Direct3d11::setCurrentVertexShaderBytecode(void const *bytecode, unsigned int size, uint32 signatureHash)
{
	ms_currentVertexShaderBytecode = bytecode;
	ms_currentVertexShaderBytecodeSize = size;
	ms_currentVertexShaderSignatureHash = signatureHash;
}

// ----------------------------------------------------------------------

/**
 * Reach a static shader's graphics data.
 *
 * StaticShader keeps it private and names class Direct3d11 a friend. The Gl_api slot that
 * needs it is a namespace function, and a namespace function has no friend access, so the
 * reach-through has to be a member of the class the engine actually befriended.
 */

StaticShaderGraphicsData *Direct3d11::getStaticShaderGraphicsData(StaticShader const &shader)
{
	return shader.m_graphicsData;
}

// ----------------------------------------------------------------------

void Direct3d11::setCurrentTextureCoordinateSetMapping(int const *mapping, int count)
{
	if (count > Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS)
	{
		DEBUG_WARNING(true, ("Direct3d11: a program declares %d texture coordinate set tags; only %d are carried.", count, static_cast<int>(Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS)));
		count = Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS;
	}

	ms_currentTextureCoordinateSetMappingCount = count;
	for (int i = 0; i < count; ++i)
		ms_currentTextureCoordinateSetMapping[i] = mapping[i];
}

// ----------------------------------------------------------------------
/**
 * Everything that has to happen before any draw, in one place.
 *
 * DX9 has the same single chokepoint and both of its device-call wrappers go
 * through it. Two of the things it does are load-bearing rather than tidy: the
 * transform constants are uploaded lazily on a dirty flag, and the alpha-fade path
 * rewrites blend and colour-write state per draw. A draw path that bypasses this
 * gets stale transforms or wrong blending on that path only, which is the kind of
 * bug that gets attributed to content.
 *
 * Returns false to skip the draw, having counted it.
 */

bool Direct3d11::prepareToDraw()
{
	Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::drawPrepareTicks);

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	if (!context)
		return false;

	// An input layout needs both the stream formats and the vertex shader. The shader
	// arrives with the static shader data, so a draw reaching here without one means either
	// no material was applied or its vertex program failed to build -- both of which the
	// program class has already named. Stopping here, counted, beats issuing a draw with no
	// layout that the debug layer would reject and the log would not explain.
	if (!ms_currentVertexShaderBytecode || !ms_streamCount)
	{
		++Direct3d11_Metrics::droppedDraws;

		if (!ms_warnedAboutMissingVertexShader)
		{
			ms_warnedAboutMissingVertexShader = true;
			WARNING(true, ("Direct3d11: a draw was reached with no vertex shader bound, so no input layout can be built and the draw is being skipped. Every such draw is counted in droppedDraws. Reported once."));
		}

		Direct3d11::reportBadVertexBufferVertexShaderCombination();
		return false;
	}

	ID3D11InputLayout *const layout = Direct3d11_InputLayoutCache::getInputLayout(ms_streamFormatFlags, ms_streamCount, ms_currentVertexShaderBytecode, ms_currentVertexShaderBytecodeSize, ms_currentVertexShaderSignatureHash, ms_currentTextureCoordinateSetMapping, ms_currentTextureCoordinateSetMappingCount);
	if (!layout)
	{
		// The cache has already explained which format and shader combination could
		// not be satisfied, and counted it. Raising the engine's flag is what stops the
		// appearance asking again every frame.
		Direct3d11::reportBadVertexBufferVertexShaderCombination();
		return false;
	}

	Direct3d11_StateCache::setInputLayout(layout);

	// Pick the eight lights and upload them if anything changed. Ahead of the transform
	// concatenation only because both write constants and this one is the larger; neither
	// depends on the other.
	Direct3d11_LightManager::selectLights();

	// Concatenate the object, camera and projection matrices if any of them moved. This has to
	// happen before the constant flush below, because it writes the per-object slice that the
	// flush uploads.
	Direct3d11_Transforms::flush();

	// Push whatever constants changed. At most one flush per stage, and the
	// per-object slice either way.
	Direct3d11_ConstantBuffers::flush();

	Direct3d11_Metrics::vertices += ms_sliceNumberOfVertices;

	return true;
}

// ======================================================================

int Direct3d11::getSliceNumberOfVertices()
{
	return ms_sliceNumberOfVertices;
}

// ----------------------------------------------------------------------

int Direct3d11::getSliceFirstVertex()
{
	return ms_sliceFirstVertex;
}

// ----------------------------------------------------------------------

int Direct3d11::getSliceNumberOfIndices()
{
	return ms_sliceNumberOfIndices;
}

// ----------------------------------------------------------------------

int Direct3d11::getSliceFirstIndex()
{
	return ms_sliceFirstIndex;
}

// ======================================================================

// ======================================================================
// The draw entry points.
//
// Every one funnels through prepareToDraw, which builds the input layout and flushes the
// constants. Nothing below touches state directly.
//
// Three of D3D9's seven primitive types do not exist in D3D11 and are emulated with a
// shared index buffer of a fixed pattern:
//
//   TRIANGLEFAN  no D3D11 equivalent. The engine leans on it: a four vertex fan is the
//                classic full-screen quad, which Bloom, the post-processing manager and
//                GlowAppearance all draw. Expanded to a triangle list as (0, i, i+1).
//
//   QUADLIST     no D3D9 equivalent either -- DX9 already emulates it exactly this way,
//                with its own resizeQuadListIndexBuffer -- so this is the same trick with
//                the same pattern, (0,1,2) (0,2,3) per quad.
//
// Both patterns are built once, grown on demand, and never rebuilt in a steady frame. They
// are 16-bit because every index in this engine is: Index is a typedef for unsigned short,
// which also caps a fan or quad run at 65535 vertices, the same ceiling D3D9 had.
//
// The INDEXED fan is refused rather than emulated, and that is a considered choice. The
// pattern trick cannot work for it: the fan's vertex order comes from the engine's own index
// buffer, so expanding it means reading those indices back and rewriting them, which needs a
// staging copy of a buffer created without CPU access. That is implementable -- copy to a
// staging buffer once per unique (buffer, first, count) and cache the expansion -- but it is
// only reachable from asset data that declares SPSPT_indexedTriangleFan, and no code path
// asks for it. So it reports itself by name and counts the dropped draw, which turns "is this
// used" into an answer on the first run instead of a guess now.
// ======================================================================

// ----------------------------------------------------------------------
/**
 * Grow the fan expansion buffer to cover a fan of this many vertices.
 *
 * Grown in one step to the size asked for rather than doubled: fans in this engine are
 * either four vertices or a terrain patch, so there is no long ramp to amortise, and a
 * creation inside a frame is something Direct3d11_Metrics is watching for.
 */

bool Direct3d11_DrawNamespace::ensureFanIndexBuffer(int vertexCount)
{
	if (vertexCount < 3)
		return false;

	if (ms_fanIndexBuffer && vertexCount <= ms_fanIndexBufferVertices)
		return true;

	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	if (!device)
		return false;

	FATAL(vertexCount > 65535, ("Direct3d11: a triangle fan of %d vertices cannot be indexed with 16-bit indices.", vertexCount));

	if (ms_fanIndexBuffer)
	{
		Direct3d11::forgetIndexBuffer(ms_fanIndexBuffer);
		ms_fanIndexBuffer->Release();
		ms_fanIndexBuffer = NULL;
	}

	int const triangles = vertexCount - 2;
	int const indexCount = triangles * 3;

	uint16 *const indices = new uint16[indexCount];
	for (int i = 0; i < triangles; ++i)
	{
		indices[(i * 3) + 0] = 0;
		indices[(i * 3) + 1] = static_cast<uint16>(i + 1);
		indices[(i * 3) + 2] = static_cast<uint16>(i + 2);
	}

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.ByteWidth = static_cast<UINT>(indexCount * isizeof(uint16));
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initial;
	Zero(initial);
	initial.pSysMem = indices;

	HRESULT const hresult = device->CreateBuffer(&description, &initial, &ms_fanIndexBuffer);
	delete[] indices;

	if (FAILED(hresult) || !ms_fanIndexBuffer)
	{
		WARNING(true, ("Direct3d11: the triangle fan index buffer for %d vertices could not be created (%s).", vertexCount, Direct3d11_Device::describeHresult(hresult)));
		ms_fanIndexBuffer = NULL;
		ms_fanIndexBufferVertices = 0;
		return false;
	}

	ms_fanIndexBufferVertices = vertexCount;
	return true;
}

// ----------------------------------------------------------------------

bool Direct3d11_DrawNamespace::ensureQuadIndexBuffer(int quadCount)
{
	if (quadCount < 1)
		return false;

	if (ms_quadIndexBuffer && quadCount <= ms_quadIndexBufferQuads)
		return true;

	ID3D11Device1 *const device = Direct3d11_Device::getDevice();
	if (!device)
		return false;

	FATAL(quadCount * 4 > 65535, ("Direct3d11: %d quads need more than 65535 vertices, which 16-bit indices cannot reach.", quadCount));

	if (ms_quadIndexBuffer)
	{
		Direct3d11::forgetIndexBuffer(ms_quadIndexBuffer);
		ms_quadIndexBuffer->Release();
		ms_quadIndexBuffer = NULL;
	}

	int const indexCount = quadCount * 6;
	uint16 *const indices = new uint16[indexCount];

	for (int q = 0; q < quadCount; ++q)
	{
		uint16 const base = static_cast<uint16>(q * 4);
		indices[(q * 6) + 0] = static_cast<uint16>(base + 0);
		indices[(q * 6) + 1] = static_cast<uint16>(base + 1);
		indices[(q * 6) + 2] = static_cast<uint16>(base + 2);
		indices[(q * 6) + 3] = static_cast<uint16>(base + 0);
		indices[(q * 6) + 4] = static_cast<uint16>(base + 2);
		indices[(q * 6) + 5] = static_cast<uint16>(base + 3);
	}

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.ByteWidth = static_cast<UINT>(indexCount * isizeof(uint16));
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initial;
	Zero(initial);
	initial.pSysMem = indices;

	HRESULT const hresult = device->CreateBuffer(&description, &initial, &ms_quadIndexBuffer);
	delete[] indices;

	if (FAILED(hresult) || !ms_quadIndexBuffer)
	{
		WARNING(true, ("Direct3d11: the quad list index buffer for %d quads could not be created (%s).", quadCount, Direct3d11_Device::describeHresult(hresult)));
		ms_quadIndexBuffer = NULL;
		ms_quadIndexBufferQuads = 0;
		return false;
	}

	ms_quadIndexBufferQuads = quadCount;
	return true;
}

// ======================================================================
// Non-indexed draws.
// ======================================================================

void Direct3d11::draw(int topology, int firstVertex, int vertexCount, int triangleCount)
{
	if (vertexCount <= 0)
		return;

	if (!prepareToDraw())
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	Direct3d11_StateCache::setPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));

	// The only place point sprites can apply, and the place that has to take them away
	// again: a geometry shader left bound would expand the next triangle list's vertices
	// into quads, which is not a subtle failure.
	Direct3d11_PointSprite::apply(topology == D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// ms_sliceFirstVertex is where the bound buffer's usable range starts. For a dynamic
	// buffer that is the ring offset, which setVertexBuffer deliberately does NOT put in the
	// stream byte offset -- doing both would double-count it.
	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::drawSubmitTicks);
		context->Draw(static_cast<UINT>(vertexCount), static_cast<UINT>(ms_sliceFirstVertex + firstVertex));
	}

	++Direct3d11_Metrics::drawCalls;
	Direct3d11_Metrics::triangles += triangleCount;
}

// ----------------------------------------------------------------------

void Direct3d11::drawIndexed(int topology, int firstIndex, int indexCount, int baseVertex, int triangleCount)
{
	if (indexCount <= 0)
		return;

	if (!prepareToDraw())
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();
	Direct3d11_StateCache::setPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topology));

	Direct3d11_PointSprite::apply(topology == D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::drawSubmitTicks);
		context->DrawIndexed(static_cast<UINT>(indexCount), static_cast<UINT>(ms_sliceFirstIndex + firstIndex), ms_sliceFirstVertex + baseVertex);
	}

	++Direct3d11_Metrics::drawIndexedCalls;
	Direct3d11_Metrics::triangles += triangleCount;
}

// ----------------------------------------------------------------------
/**
 * A fan, expanded to a triangle list through the shared pattern buffer.
 *
 * baseVertex carries the fan's first vertex, so the pattern buffer's indices stay relative
 * to it and one buffer serves every fan regardless of where in the vertex buffer it sits.
 */

void Direct3d11::drawFan(int firstVertex, int vertexCount)
{
	if (vertexCount < 3)
		return;

	if (!ensureFanIndexBuffer(vertexCount))
	{
		++Direct3d11_Metrics::droppedDraws;
		return;
	}

	if (!prepareToDraw())
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();

	// Borrowing the engine's index slot for this backend's own pattern buffer. The shadow has to
	// follow, or the next setIndexBuffer sees its own last buffer still recorded, skips the bind,
	// and draws that geometry through the fan pattern. DX9 borrows the slot the same way for its
	// quad list, and gets away without this only because it never shadowed the binding.
	context->IASetIndexBuffer(ms_fanIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	ms_boundIndexBuffer = ms_fanIndexBuffer;
	++Direct3d11_Metrics::indexBufferBindCalls;
	++Direct3d11_Metrics::indexBufferBindMisses;

	Direct3d11_StateCache::setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// A fan is expanded to a triangle list, so the point expander must not be bound.
	Direct3d11_PointSprite::apply(false);

	int const triangles = vertexCount - 2;
	// The pattern buffer's own indices start at zero, so the whole fan is placed by the base
	// vertex. No slice FIRST INDEX here: the index buffer being read is this backend's, not
	// the engine's.
	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::drawSubmitTicks);
		context->DrawIndexed(static_cast<UINT>(triangles * 3), 0, ms_sliceFirstVertex + firstVertex);
	}

	++Direct3d11_Metrics::drawIndexedCalls;
	Direct3d11_Metrics::triangles += triangles;
}

// ======================================================================

// ----------------------------------------------------------------------
/**
 * A quad list, expanded to a triangle list through the shared pattern buffer.
 *
 * DX9 emulates this identically -- D3D9 had no quad primitive either -- so the pattern and
 * the winding are its, (0,1,2) then (0,2,3) per quad, and a quad's four vertices are
 * consecutive.
 */

void Direct3d11::drawQuads(int firstVertex, int quadCount)
{
	if (quadCount < 1)
		return;

	if (!ensureQuadIndexBuffer(quadCount))
	{
		++Direct3d11_Metrics::droppedDraws;
		return;
	}

	if (!prepareToDraw())
		return;

	ID3D11DeviceContext1 *const context = Direct3d11_Device::getContext();

	context->IASetIndexBuffer(ms_quadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	ms_boundIndexBuffer = ms_quadIndexBuffer;
	++Direct3d11_Metrics::indexBufferBindCalls;
	++Direct3d11_Metrics::indexBufferBindMisses;

	Direct3d11_StateCache::setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Direct3d11_PointSprite::apply(false);

	int const triangles = quadCount * 2;
	{
		Direct3d11_Metrics::ScopedTimer timer(Direct3d11_Metrics::drawSubmitTicks);
		context->DrawIndexed(static_cast<UINT>(triangles * 3), 0, ms_sliceFirstVertex + firstVertex);
	}

	++Direct3d11_Metrics::drawIndexedCalls;
	Direct3d11_Metrics::triangles += triangles;
}

// ----------------------------------------------------------------------
/**
 * An indexed triangle fan, which this backend does not emulate.
 *
 * The pattern trick the non-indexed fan uses cannot work here: the fan's vertex order comes
 * from the ENGINE's index buffer, so expanding it means reading those indices back and
 * rewriting them. That is implementable -- copy the range to a staging buffer once per unique
 * (buffer, first, count) and cache the expansion, which is cheap after the first draw -- but
 * nothing in the engine calls this. It is reachable only from asset data declaring
 * SPSPT_indexedTriangleFan, and rather than write and ship an untested readback path for a
 * case that may not exist, this names itself and counts the dropped draw. If it ever fires,
 * the log says so and the design above is what to build.
 */

void Direct3d11::drawIndexedFanUnsupported()
{
	++Direct3d11_Metrics::droppedDraws;

	if (!ms_warnedAboutIndexedFan)
	{
		ms_warnedAboutIndexedFan = true;
		WARNING(true, ("Direct3d11: an indexed triangle fan was drawn. D3D11 has no fan primitive and this backend expands only non-indexed fans, so this geometry is missing. Reported once; see Direct3d11_Draw.cpp for what to implement."));
	}
}

void Direct3d11::releaseDrawResources()
{
	ms_boundIndexBuffer = NULL;
	ms_savedIndexBuffer = NULL;

	if (ms_fanIndexBuffer)
	{
		ms_fanIndexBuffer->Release();
		ms_fanIndexBuffer = NULL;
		ms_fanIndexBufferVertices = 0;
	}
	if (ms_quadIndexBuffer)
	{
		ms_quadIndexBuffer->Release();
		ms_quadIndexBuffer = NULL;
		ms_quadIndexBufferQuads = 0;
	}
}

// ======================================================================
