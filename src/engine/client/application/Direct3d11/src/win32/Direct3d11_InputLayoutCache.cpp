// ======================================================================
//
// Direct3d11_InputLayoutCache.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_InputLayoutCache.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_VertexBufferDescriptorMap.h"

#include "clientGraphics/VertexBufferDescriptor.h"
#include "clientGraphics/VertexBufferFormat.h"

#include <d3dcompiler.h>
#include <map>

// ======================================================================

namespace Direct3d11_InputLayoutCacheNamespace
{
	struct Key
	{
		uint32  formatFlags[Direct3d11_InputLayoutCache::MAX_STREAMS];
		int     streamCount;
		uint32  signatureHash;

		bool operator<(Key const &rhs) const
		{
			return memcmp(this, &rhs, sizeof(Key)) < 0;
		}
	};

	typedef std::map<Key, ID3D11InputLayout *> LayoutMap;
	LayoutMap *ms_layouts;

	uint32     hashBytes(void const *data, unsigned int size);
	int        buildElements(uint32 const *formatFlags, int streamCount, D3D11_INPUT_ELEMENT_DESC *elements, int maxElements);
}
using namespace Direct3d11_InputLayoutCacheNamespace;

// ======================================================================

uint32 Direct3d11_InputLayoutCacheNamespace::hashBytes(void const *data, unsigned int size)
{
	// FNV-1a. The only requirement is that different signatures collide rarely;
	// a collision here would pair a layout with the wrong shader, so the hash is
	// over the whole signature rather than a prefix.
	uint32 hash = 2166136261u;
	uint8 const * const bytes = static_cast<uint8 const *>(data);

	for (unsigned int i = 0; i < size; ++i)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}

	return hash;
}

// ======================================================================
/**
 * Turn the bound streams' formats into input elements.
 *
 * The order and the semantics are D3D9's, element for element, because the shaders
 * were authored against them:
 *
 *   position   POSITION0, three floats, or four when the position is already
 *              transformed. D3D9 used POSITIONT for that case; D3D11 has no such
 *              semantic, so it is POSITION either way and the shader is expected
 *              to be a real vertex shader rather than the fixed-function path
 *              gl05 used for screen-space geometry.
 *   normal     NORMAL0, three floats
 *   point size PSIZE0, one float
 *   colour 0/1 COLOR0 and COLOR1, as B8G8R8A8_UNORM
 *   texcoords  TEXCOORD with a GLOBAL index, at the set's own dimension
 *
 * Two of those repay attention.
 *
 * The colour format is B8G8R8A8, not R8G8B8A8. PackedArgb stores 0xAARRGGBB, which
 * on a little-endian machine is the bytes B, G, R, A ascending -- exactly what
 * D3DDECLTYPE_D3DCOLOR consumed. R8G8B8A8 would read the same bytes and hand the
 * shader red and blue exchanged.
 *
 * The texture coordinate index counts across ALL streams and is deliberately not
 * reset per stream. The skinned dot3 path binds a static stream carrying the
 * ordinary sets and a dynamic stream carrying one four-dimensional set, and the
 * shader's own index for that set is its global ordinal. Resetting per stream would
 * emit two TEXCOORD0 elements and no TEXCOORDn, CreateInputLayout would fail, and
 * with it every skinned character would vanish.
 */

int Direct3d11_InputLayoutCacheNamespace::buildElements(uint32 const *formatFlags, int streamCount, D3D11_INPUT_ELEMENT_DESC *elements, int maxElements)
{
	int elementCount = 0;
	int textureCoordinate = 0;

	for (int stream = 0; stream < streamCount; ++stream)
	{
		VertexBufferDescriptor const &descriptor = Direct3d11_VertexBufferDescriptorMap::getDescriptor(formatFlags[stream]);

		// The descriptor says where each component sits; the format flags say
		// whether it is present and, for texture coordinates, how wide it is.
		VertexBufferFormat format;
		format.setFlags(formatFlags[stream]);

		if (descriptor.offsetPosition >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName         = "POSITION";
			element.SemanticIndex        = 0;
			element.Format               = (descriptor.offsetOoz >= 0) ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot            = static_cast<UINT>(stream);
			element.AlignedByteOffset    = static_cast<UINT>(descriptor.offsetPosition);
			element.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}

		if (descriptor.offsetNormal >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName      = "NORMAL";
			element.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot         = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetNormal);
			element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetPointSize >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName      = "PSIZE";
			element.Format            = DXGI_FORMAT_R32_FLOAT;
			element.InputSlot         = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetPointSize);
			element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetColor0 >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName      = "COLOR";
			element.SemanticIndex     = 0;
			element.Format            = DXGI_FORMAT_B8G8R8A8_UNORM;
			element.InputSlot         = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetColor0);
			element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetColor1 >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName      = "COLOR";
			element.SemanticIndex     = 1;
			element.Format            = DXGI_FORMAT_B8G8R8A8_UNORM;
			element.InputSlot         = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetColor1);
			element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		}

		int const setCount = format.getNumberOfTextureCoordinateSets();
		for (int set = 0; set < setCount; ++set)
		{
			if (descriptor.offsetTextureCoordinateSet[set] < 0)
				continue;

			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName  = "TEXCOORD";

			// Global, not per stream. See the comment above.
			element.SemanticIndex = static_cast<UINT>(textureCoordinate++);

			switch (format.getTextureCoordinateSetDimension(set))
			{
				case 1:  element.Format = DXGI_FORMAT_R32_FLOAT;          break;
				case 2:  element.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
				case 3:  element.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
				case 4:  element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				default:
					FATAL(true, ("Direct3d11: texture coordinate set %d has dimension %d, which is not 1 to 4.", set, format.getTextureCoordinateSetDimension(set)));
					break;
			}

			element.InputSlot         = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetTextureCoordinateSet[set]);
			element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		}
	}

	return elementCount;
}

// ======================================================================

void Direct3d11_InputLayoutCache::install()
{
	DEBUG_FATAL(ms_layouts, ("Direct3d11_InputLayoutCache::install called twice"));
	ms_layouts = new LayoutMap;
}

// ----------------------------------------------------------------------

void Direct3d11_InputLayoutCache::remove()
{
	if (ms_layouts)
	{
		for (LayoutMap::iterator i = ms_layouts->begin(); i != ms_layouts->end(); ++i)
			if (i->second)
				i->second->Release();

		delete ms_layouts;
		ms_layouts = NULL;
	}
}

// ----------------------------------------------------------------------

ID3D11InputLayout *Direct3d11_InputLayoutCache::getInputLayout(uint32 const *formatFlags, int streamCount, void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize)
{
	NOT_NULL(ms_layouts);
	NOT_NULL(formatFlags);
	NOT_NULL(vertexShaderBytecode);

	FATAL(streamCount <= 0 || streamCount > MAX_STREAMS, ("Direct3d11: %d vertex streams were bound; this backend handles 1 to %d.", streamCount, static_cast<int>(MAX_STREAMS)));

	// Key on the shader's INPUT SIGNATURE, so two shaders whose bodies differ but
	// whose inputs match share one layout. Keying on the bytecode pointer would let
	// a freed blob's address alias a live entry.
	ID3DBlob *signature = NULL;
	HRESULT const signatureResult = D3DGetInputSignatureBlob(vertexShaderBytecode, vertexShaderBytecodeSize, &signature);

	Key key;
	Zero(key);
	key.streamCount = streamCount;
	for (int i = 0; i < streamCount; ++i)
		key.formatFlags[i] = formatFlags[i];

	if (SUCCEEDED(signatureResult) && signature)
		key.signatureHash = hashBytes(signature->GetBufferPointer(), static_cast<unsigned int>(signature->GetBufferSize()));
	else
	{
		// Falling back to the whole bytecode is correct but coarser: two shaders
		// with identical inputs get separate layouts.
		DEBUG_WARNING(true, ("Direct3d11: a vertex shader's input signature could not be extracted; keying the input layout on its whole bytecode instead."));
		key.signatureHash = hashBytes(vertexShaderBytecode, vertexShaderBytecodeSize);
	}

	LayoutMap::const_iterator const existing = ms_layouts->find(key);
	if (existing != ms_layouts->end())
	{
		if (signature)
			signature->Release();
		return existing->second;
	}

	D3D11_INPUT_ELEMENT_DESC elements[32];
	int const elementCount = buildElements(formatFlags, streamCount, elements, 32);

	ID3D11InputLayout *layout = NULL;

	if (elementCount > 0)
	{
		ID3D11Device1 * const device = Direct3d11_Device::getDevice();
		NOT_NULL(device);

		HRESULT const hresult = device->CreateInputLayout(elements, static_cast<UINT>(elementCount), vertexShaderBytecode, vertexShaderBytecodeSize, &layout);

		if (FAILED(hresult) || !layout)
		{
			// Name the formats. D3D9 tolerated a shader reading an input the buffer
			// did not supply; D3D11 refuses to build the layout at all, so this is
			// the message that has to identify which combination is unsatisfiable.
			WARNING(true, ("Direct3d11: CreateInputLayout failed (%s) for %d stream(s) with %d element(s); first format flags 0x%08x. A vertex shader is reading an input these buffers do not supply.", Direct3d11_Device::describeHresult(hresult), streamCount, elementCount, formatFlags[0]));
			++Direct3d11_Metrics::droppedDraws;
		}
		else
			++Direct3d11_Metrics::inputLayoutCreations;
	}

	if (signature)
		signature->Release();

	// Cached either way, including a null. Retrying a creation that cannot succeed
	// once per draw would be a per-frame cost for a permanent condition -- but the
	// warning above is issued once per combination rather than once per attempt.
	IGNORE_RETURN(ms_layouts->insert(std::make_pair(key, layout)));

	return layout;
}

// ----------------------------------------------------------------------

int Direct3d11_InputLayoutCache::getLayoutCount()
{
	return ms_layouts ? static_cast<int>(ms_layouts->size()) : 0;
}

// ======================================================================
