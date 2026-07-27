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

		// Which vertex buffer texture coordinate set each of the shader's tags reads, in
		// the shader's declaration order. This is what used to be baked into the shader as
		// a compile-time key; it belongs here instead, because it describes how a material
		// wires a vertex buffer to a program rather than anything about the program.
		int8    textureCoordinateSetMapping[Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS];
		int     mappingCount;

		bool operator<(Key const &rhs) const
		{
			return memcmp(this, &rhs, sizeof(Key)) < 0;
		}
	};

	typedef std::map<Key, ID3D11InputLayout *> LayoutMap;
	LayoutMap *ms_layouts;

	// Where one vertex buffer texture coordinate set lives.
	struct SetLocation
	{
		int  stream;
		int  offset;
		int  dimension;
	};

	uint32     hashBytes(void const *data, unsigned int size);
	int        locateTextureCoordinateSets(uint32 const *formatFlags, int streamCount, SetLocation *locations, int maxLocations);
	int        buildElements(uint32 const *formatFlags, int streamCount, int const *mapping, int mappingCount, D3D11_INPUT_ELEMENT_DESC *elements, int maxElements);
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
 *   texcoords  TEXCOORD, one per tag the shader declares, at the vertex buffer set's
 *              own dimension -- see the texture coordinate section below, which is where
 *              the specialisation DX9 baked into the shader now lives.
 *
 * Two of those repay attention.
 *
 * The colour format is B8G8R8A8, not R8G8B8A8. PackedArgb stores 0xAARRGGBB, which
 * on a little-endian machine is the bytes B, G, R, A ascending -- exactly what
 * D3DDECLTYPE_D3DCOLOR consumed. R8G8B8A8 would read the same bytes and hand the
 * shader red and blue exchanged.
 *
 * Texture coordinate sets are numbered across ALL streams and deliberately not reset per
 * stream. The skinned dot3 path binds a static stream carrying the ordinary sets and a
 * dynamic stream carrying one four-dimensional set, and the engine's index for that set
 * continues the count rather than restarting -- which is why locateTextureCoordinateSets
 * builds one flat table over every bound stream.
 */

int Direct3d11_InputLayoutCacheNamespace::buildElements(uint32 const *formatFlags, int streamCount, int const *mapping, int mappingCount, D3D11_INPUT_ELEMENT_DESC *elements, int maxElements)
{
	int elementCount = 0;

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

	}

	// ------------------------------------------------------------------
	// Texture coordinates. This is where the DX9 shader specialisation went.
	//
	// A program declares its sets by TAG, and a material decides which of the vertex
	// buffer's numbered sets each tag reads. D3D9 had to settle that at compile time,
	// because vertex inputs were fixed registers, so it compiled one shader variant per
	// mapping. Here the program is compiled once with tag i at TEXCOORD i, and the routing
	// happens below: the element for tag i is placed at the offset of the set the material
	// pointed that tag at.
	//
	// A program with no tags -- the converted assembly programs that address sets by number
	// rather than by tag, cloudlayer being the one -- gets the straight global ordering.

	SetLocation locations[Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS];
	int const locationCount = locateTextureCoordinateSets(formatFlags, streamCount, locations, isizeof(locations) / isizeof(locations[0]));

	int const emitCount = mappingCount ? mappingCount : locationCount;

	for (int i = 0; i < emitCount; ++i)
	{
		int set = mappingCount ? mapping[i] : i;

		if (set < 0 || set >= locationCount)
		{
			// DX9 warns and falls back to set 0 when a material names a set the vertex
			// buffer does not carry (Direct3d9_StaticShaderData.cpp:564). Same behaviour,
			// so a mismatch looks the same in both backends.
			DEBUG_WARNING(true, ("Direct3d11: a shader tag asks for texture coordinate set %d but this vertex buffer has %d; using set 0.", set, locationCount));
			set = 0;
			if (locationCount <= 0)
				break;
		}

		SetLocation const &location = locations[set];

		FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
		D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
		Zero(element);
		element.SemanticName  = "TEXCOORD";
		element.SemanticIndex = static_cast<UINT>(i);

		// The element's width is the vertex buffer's, not the shader's. The input assembler
		// fills components the buffer does not supply with 0, and w with 1, so a two
		// component set feeding a four component declaration is legal and is what D3D9 did
		// for an unwritten register component.
		switch (location.dimension)
		{
			case 1:  element.Format = DXGI_FORMAT_R32_FLOAT;          break;
			case 2:  element.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
			case 3:  element.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
			case 4:  element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			default:
				FATAL(true, ("Direct3d11: texture coordinate set %d has dimension %d, which is not 1 to 4.", set, location.dimension));
				break;
		}

		element.InputSlot         = static_cast<UINT>(location.stream);
		element.AlignedByteOffset = static_cast<UINT>(location.offset);
		element.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
	}

	return elementCount;
}

// ----------------------------------------------------------------------
/**
 * Where every texture coordinate set in this vertex buffer lives.
 *
 * Indexed by the set's GLOBAL ordinal across streams, which is the numbering the engine's
 * own set indices use: the skinned dot3 path binds a static stream carrying the ordinary
 * sets and a dynamic stream carrying one four-dimensional set, and that last set's index
 * continues the count rather than restarting.
 */

int Direct3d11_InputLayoutCacheNamespace::locateTextureCoordinateSets(uint32 const *formatFlags, int streamCount, SetLocation *locations, int maxLocations)
{
	int count = 0;

	for (int stream = 0; stream < streamCount; ++stream)
	{
		VertexBufferDescriptor const &descriptor = Direct3d11_VertexBufferDescriptorMap::getDescriptor(formatFlags[stream]);

		VertexBufferFormat format;
		format.setFlags(formatFlags[stream]);

		int const setCount = format.getNumberOfTextureCoordinateSets();
		for (int set = 0; set < setCount; ++set)
		{
			if (descriptor.offsetTextureCoordinateSet[set] < 0)
				continue;

			if (count >= maxLocations)
			{
				DEBUG_WARNING(true, ("Direct3d11: more than %d texture coordinate sets across the bound streams; the rest are ignored.", maxLocations));
				return count;
			}

			locations[count].stream    = stream;
			locations[count].offset    = descriptor.offsetTextureCoordinateSet[set];
			locations[count].dimension = format.getTextureCoordinateSetDimension(set);
			++count;
		}
	}

	return count;
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

ID3D11InputLayout *Direct3d11_InputLayoutCache::getInputLayout(uint32 const *formatFlags, int streamCount, void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize, int const *textureCoordinateSetMapping, int mappingCount)
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

	// The tag-to-set mapping is part of the key, because it is what distinguishes two
	// layouts that serve the same shader and the same vertex buffer for two materials that
	// wire their texture coordinate sets differently. Under DX9 that distinction lived in
	// the shader, as a separate compiled variant per mapping.
	DEBUG_FATAL(mappingCount < 0 || mappingCount > MAX_TEXTURE_COORDINATE_SETS, ("Direct3d11: a shader declares %d texture coordinate set tags; the engine supports %d.", mappingCount, static_cast<int>(MAX_TEXTURE_COORDINATE_SETS)));

	key.mappingCount = mappingCount;
	for (int i = 0; i < mappingCount && i < MAX_TEXTURE_COORDINATE_SETS; ++i)
		key.textureCoordinateSetMapping[i] = static_cast<int8>(textureCoordinateSetMapping[i]);

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
	int const elementCount = buildElements(formatFlags, streamCount, textureCoordinateSetMapping, mappingCount, elements, 32);

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
			WARNING(true, ("Direct3d11: CreateInputLayout failed (%s) for %d stream(s) with %d element(s); first format flags 0x%08x. A vertex shader is reading an input these buffers do not supply, which shipping DX9 does not draw either.", Direct3d11_Device::describeHresult(hresult), streamCount, elementCount, formatFlags[0]));
			++Direct3d11_Metrics::unsatisfiableInputLayouts;
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
