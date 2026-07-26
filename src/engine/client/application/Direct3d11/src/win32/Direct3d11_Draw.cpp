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
#include "Direct3d11_Metrics.h"
#include "Direct3d11_StateCache.h"
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
	int             ms_sliceNumberOfVertices;
	int             ms_sliceFirstVertex;

	ID3D11Buffer   *ms_savedIndexBuffer;
	int             ms_sliceNumberOfIndices;
	int             ms_sliceFirstIndex;

	// How many streams were bound last time. Still needed, and for a sharper
	// reason than in DX9: binding stream 0 does not unbind streams 1..N, and in
	// D3D11 a stale stream referenced by a later input layout is a hard error where
	// D3D9 merely wasted a slot.
	int             ms_lastVertexBufferCount;

	// What the bound streams look like, for the layout the draw will need.
	uint32          ms_streamFormatFlags[Direct3d11_InputLayoutCache::MAX_STREAMS];
	int             ms_streamCount;

	void const     *ms_currentVertexShaderBytecode;
	unsigned int    ms_currentVertexShaderBytecodeSize;

	bool            ms_warnedAboutMissingVertexShader;
}
using namespace Direct3d11_DrawNamespace;

// ======================================================================

void Direct3d11::setVertexBuffer(HardwareVertexBuffer const &vertexBuffer)
{
	DX11_ASSERT_MAIN_THREAD();

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
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
				StaticVertexBuffer const * const staticVertexBuffer = safe_cast<StaticVertexBuffer const *>(&vertexBuffer);
				Direct3d11_StaticVertexBufferData * const data = safe_cast<Direct3d11_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

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
				ms_sliceFirstVertex      = 0;
				ms_streamFormatFlags[0]  = staticVertexBuffer->getFormat().getFlags();
			}
			break;

		case HardwareVertexBuffer::T_dynamic:
			{
				DynamicVertexBuffer const * const dynamicVertexBuffer = safe_cast<DynamicVertexBuffer const *>(&vertexBuffer);
				Direct3d11_DynamicVertexBufferData * const data = safe_cast<Direct3d11_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

				buffer = Direct3d11_DynamicVertexBufferData::getRing();
				stride = static_cast<UINT>(data->getVertexSize());

				ms_sliceNumberOfVertices = data->getNumberOfVertices();
				ms_sliceFirstVertex      = data->getOffset();
				ms_streamFormatFlags[0]  = dynamicVertexBuffer->getFormat().getFlags();
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

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	NOT_NULL(vertexBufferVector.m_vertexBufferList);

	ms_sliceNumberOfVertices = 0;
	ms_sliceFirstVertex      = 0;

	ID3D11Buffer *buffers[Direct3d11_InputLayoutCache::MAX_STREAMS];
	UINT strides[Direct3d11_InputLayoutCache::MAX_STREAMS];
	UINT offsets[Direct3d11_InputLayoutCache::MAX_STREAMS];

	int stream = 0;

	for (VertexBufferVector::VertexBufferList::const_iterator i = vertexBufferVector.m_vertexBufferList->begin(); i != vertexBufferVector.m_vertexBufferList->end(); ++i)
	{
		HardwareVertexBuffer const * const vertexBuffer = *i;
		NOT_NULL(vertexBuffer);

		FATAL(stream >= Direct3d11_InputLayoutCache::MAX_STREAMS, ("Direct3d11: a vertex buffer vector bound more than %d streams.", static_cast<int>(Direct3d11_InputLayoutCache::MAX_STREAMS)));

		switch (vertexBuffer->getType())
		{
			case HardwareVertexBuffer::T_static:
				{
					StaticVertexBuffer const * const staticVertexBuffer = safe_cast<StaticVertexBuffer const *>(vertexBuffer);
					Direct3d11_StaticVertexBufferData * const data = safe_cast<Direct3d11_StaticVertexBufferData *>(staticVertexBuffer->m_graphicsData);

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
					DynamicVertexBuffer const * const dynamicVertexBuffer = safe_cast<DynamicVertexBuffer const *>(vertexBuffer);
					Direct3d11_DynamicVertexBufferData * const data = safe_cast<Direct3d11_DynamicVertexBufferData *>(dynamicVertexBuffer->m_graphicsData);

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

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	switch (indexBuffer.getType())
	{
		case HardwareIndexBuffer::T_static:
			{
				StaticIndexBuffer const * const staticIndexBuffer = safe_cast<StaticIndexBuffer const *>(&indexBuffer);
				Direct3d11_StaticIndexBufferData * const data = safe_cast<Direct3d11_StaticIndexBufferData *>(staticIndexBuffer->m_graphicsData);

				ms_savedIndexBuffer     = data->getBuffer();
				ms_sliceNumberOfIndices = staticIndexBuffer->getNumberOfIndices();
				ms_sliceFirstIndex      = 0;
			}
			break;

		case HardwareIndexBuffer::T_dynamic:
			{
				DynamicIndexBuffer const * const dynamicIndexBuffer = safe_cast<DynamicIndexBuffer const *>(&indexBuffer);
				Direct3d11_DynamicIndexBufferData * const data = safe_cast<Direct3d11_DynamicIndexBufferData *>(dynamicIndexBuffer->m_graphicsData);

				ms_savedIndexBuffer     = Direct3d11_DynamicIndexBufferData::getRing();
				ms_sliceNumberOfIndices = data->getNumberOfIndices();

				// In INDICES, consumed as the draw's start index -- which is why the
				// byte offset handed to IASetIndexBuffer below stays zero.
				ms_sliceFirstIndex      = data->getOffset();
			}
			break;

		default:
			DEBUG_FATAL(true, ("Direct3d11: unknown index buffer type %d", static_cast<int>(indexBuffer.getType())));
			return;
	}

	// Always R16_UINT: Index is an unsigned short throughout the engine.
	context->IASetIndexBuffer(ms_savedIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	++Direct3d11_Metrics::indexBufferBindCalls;
	++Direct3d11_Metrics::indexBufferBindMisses;
}

// ======================================================================

void Direct3d11::setCurrentVertexShaderBytecode(void const *bytecode, unsigned int size)
{
	ms_currentVertexShaderBytecode = bytecode;
	ms_currentVertexShaderBytecodeSize = size;
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
	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return false;

	// An input layout needs both the stream formats and the vertex shader. The
	// shader arrives with the static shader data, which is not implemented yet, so
	// until then this is where a draw stops -- counted and named once, rather than
	// issuing a draw with no layout that the debug layer would reject and the log
	// would not explain.
	if (!ms_currentVertexShaderBytecode || !ms_streamCount)
	{
		++Direct3d11_Metrics::droppedDraws;

		if (!ms_warnedAboutMissingVertexShader)
		{
			ms_warnedAboutMissingVertexShader = true;
			WARNING(true, ("Direct3d11: a draw was reached with no vertex shader bound, so no input layout can be built and the draw is being skipped. This is expected until the shader data classes are implemented; every such draw is counted."));
		}

		return false;
	}

	ID3D11InputLayout * const layout = Direct3d11_InputLayoutCache::getInputLayout(ms_streamFormatFlags, ms_streamCount, ms_currentVertexShaderBytecode, ms_currentVertexShaderBytecodeSize);
	if (!layout)
	{
		// The cache has already explained which format and shader combination could
		// not be satisfied, and counted it.
		return false;
	}

	Direct3d11_StateCache::setInputLayout(layout);

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
