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

#include <cstring>
#include <d3dcompiler.h>
#include <map>

// ======================================================================

namespace Direct3d11_InputLayoutCacheNamespace
{
	constexpr uint32 cs_fnvOffsetBasis = 2166136261u;
	constexpr uint32 cs_fnvPrime = 16777619u;
	constexpr int cs_maxInputElements = 32;

	struct Key
	{
		uint32 formatFlags[Direct3d11_InputLayoutCache::MAX_STREAMS];
		int streamCount;
		uint32 signatureHash;

		// Which vertex buffer texture coordinate set each of the shader's tags reads, in
		// the shader's declaration order. This is what used to be baked into the shader as
		// a compile-time key; it belongs here instead, because it describes how a material
		// wires a vertex buffer to a program rather than anything about the program.
		int8 textureCoordinateSetMapping[Direct3d11_InputLayoutCache::MAX_TEXTURE_COORDINATE_SETS];
		int mappingCount;

		bool operator<(Key const &rhs) const
		{
			return memcmp(this, &rhs, sizeof(Key)) < 0;
		}
	};

	typedef std::map<Key, ID3D11InputLayout *> LayoutMap;
	LayoutMap *ms_layouts;

	// The phantom stream: a stride-zero buffer of zeros that backs vertex shader
	// inputs the bound buffers do not supply. Slot 15 is far above MAX_STREAMS, so
	// the stale-stream unbind loops in Direct3d11_Draw.cpp never touch it. Created
	// the first time a layout actually needs it; NULL until then.
	ID3D11Buffer *ms_phantomBuffer;
	constexpr int cs_phantomStreamSlot = 15;
	constexpr int cs_maxPhantomElements = 8;
	constexpr UINT cs_phantomWhiteOffset = 16;   // float4(1,1,1,1) lives at bytes 16..31

	// Semantic-name storage for phantom elements. CreateInputLayout copies the
	// descriptors it is given, but only during the call -- and the reflection
	// interface that produced the names is released before the caller retries the
	// creation, so the names are copied here rather than pointed at.
	char ms_phantomNames[cs_maxPhantomElements][32];

	int appendPhantomElements(void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize, D3D11_INPUT_ELEMENT_DESC *elements, int elementCount, int maxElements);

	// Where one vertex buffer texture coordinate set lives.
	struct SetLocation
	{
		int stream;
		int offset;
		int dimension;
	};

	uint32 hashBytes(void const *data, unsigned int size);
	int locateTextureCoordinateSets(uint32 const *formatFlags, int streamCount, SetLocation *locations, int maxLocations);
	int buildElements(uint32 const *formatFlags, int streamCount, int const *mapping, int mappingCount, D3D11_INPUT_ELEMENT_DESC *elements, int maxElements);
} // namespace Direct3d11_InputLayoutCacheNamespace
using namespace Direct3d11_InputLayoutCacheNamespace;

// ======================================================================

uint32 Direct3d11_InputLayoutCacheNamespace::hashBytes(void const *data, unsigned int size)
{
	// FNV-1a. The only requirement is that different signatures collide rarely;
	// a collision here would pair a layout with the wrong shader, so the hash is
	// over the whole signature rather than a prefix.
	uint32 hash = cs_fnvOffsetBasis;
	uint8 const *const bytes = static_cast<uint8 const *>(data);

	for (unsigned int i = 0; i < size; ++i)
	{
		hash ^= bytes[i];
		hash *= cs_fnvPrime;
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
			element.SemanticName = "POSITION";
			element.SemanticIndex = 0;
			element.Format = (descriptor.offsetOoz >= 0) ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetPosition);
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
		}

		if (descriptor.offsetNormal >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName = "NORMAL";
			element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetNormal);
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetPointSize >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName = "PSIZE";
			element.Format = DXGI_FORMAT_R32_FLOAT;
			element.InputSlot = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetPointSize);
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetColor0 >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName = "COLOR";
			element.SemanticIndex = 0;
			element.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			element.InputSlot = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetColor0);
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		}

		if (descriptor.offsetColor1 >= 0)
		{
			FATAL(elementCount >= maxElements, ("Direct3d11: too many input elements"));
			D3D11_INPUT_ELEMENT_DESC &element = elements[elementCount++];
			Zero(element);
			element.SemanticName = "COLOR";
			element.SemanticIndex = 1;
			element.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			element.InputSlot = static_cast<UINT>(stream);
			element.AlignedByteOffset = static_cast<UINT>(descriptor.offsetColor1);
			element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
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
		element.SemanticName = "TEXCOORD";
		element.SemanticIndex = static_cast<UINT>(i);

		// The element's width is the vertex buffer's, not the shader's. The input assembler
		// fills components the buffer does not supply with 0, and w with 1, so a two
		// component set feeding a four component declaration is legal and is what D3D9 did
		// for an unwritten register component.
		switch (location.dimension)
		{
			case 1:
				element.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case 2:
				element.Format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case 3:
				element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case 4:
				element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				FATAL(true, ("Direct3d11: texture coordinate set %d has dimension %d, which is not 1 to 4.", set, location.dimension));
				break;
		}

		element.InputSlot = static_cast<UINT>(location.stream);
		element.AlignedByteOffset = static_cast<UINT>(location.offset);
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
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

			locations[count].stream = stream;
			locations[count].offset = descriptor.offsetTextureCoordinateSet[set];
			locations[count].dimension = format.getTextureCoordinateSetDimension(set);
			++count;
		}
	}

	return count;
}

// ----------------------------------------------------------------------
/**
 * Append an element on the phantom stream for every shader input the built
 * elements do not cover.
 *
 * Every phantom element is a four-component float at offset zero of the
 * stride-zero phantom buffer, so the shader reads (0,0,0,0) -- which is what
 * D3D9 handed a register whose declaration element was absent. Width does not
 * need to match the shader's declared type: the input assembler widens and
 * narrows freely, and every source byte is zero regardless.
 *
 * Returns the new element count; returns elementCount unchanged when
 * reflection fails or nothing was missing.
 */

int Direct3d11_InputLayoutCacheNamespace::appendPhantomElements(void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize, D3D11_INPUT_ELEMENT_DESC *elements, int elementCount, int maxElements)
{
	ID3D11ShaderReflection *reflection = NULL;
	if (FAILED(D3DReflect(vertexShaderBytecode, vertexShaderBytecodeSize, __uuidof(ID3D11ShaderReflection), reinterpret_cast<void **>(&reflection))) || !reflection)
		return elementCount;

	D3D11_SHADER_DESC shaderDescription;
	Zero(shaderDescription);
	if (FAILED(reflection->GetDesc(&shaderDescription)))
	{
		reflection->Release();
		return elementCount;
	}

	int result = elementCount;
	int phantomCount = 0;

	for (UINT parameter = 0; parameter < shaderDescription.InputParameters; ++parameter)
	{
		D3D11_SIGNATURE_PARAMETER_DESC parameterDescription;
		Zero(parameterDescription);
		if (FAILED(reflection->GetInputParameterDesc(parameter, &parameterDescription)) || !parameterDescription.SemanticName)
			continue;

		// System values (SV_VertexID and friends) are not fed by the input assembler.
		if (_strnicmp(parameterDescription.SemanticName, "SV_", 3) == 0)
			continue;

		bool covered = false;
		for (int i = 0; i < result && !covered; ++i)
			covered = elements[i].SemanticIndex == parameterDescription.SemanticIndex
				&& _stricmp(elements[i].SemanticName, parameterDescription.SemanticName) == 0;
		if (covered)
			continue;

		if (result >= maxElements || phantomCount >= cs_maxPhantomElements)
		{
			WARNING(true, ("Direct3d11: more phantom input elements needed than fit; the layout will stay unsatisfiable."));
			break;
		}

		char *const name = ms_phantomNames[phantomCount++];
		strncpy(name, parameterDescription.SemanticName, sizeof(ms_phantomNames[0]) - 1);
		name[sizeof(ms_phantomNames[0]) - 1] = '\0';

		D3D11_INPUT_ELEMENT_DESC &element = elements[result++];
		Zero(element);
		element.SemanticName = name;
		element.SemanticIndex = parameterDescription.SemanticIndex;
		element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		element.InputSlot = static_cast<UINT>(cs_phantomStreamSlot);
		// D3D9's defaults for an unsupplied input differed by usage: COLOR read back
		// opaque white, everything else zeros -- and shipped data leans on the white
		// (the space nebula quads multiply their texture by an unsupplied COLOR0;
		// zeros would draw them fully transparent). The phantom buffer carries zeros
		// at offset 0 and float4(1,1,1,1) at offset 16; route by semantic.
		element.AlignedByteOffset = (_stricmp(name, "COLOR") == 0) ? cs_phantomWhiteOffset : 0;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;
	}

	reflection->Release();
	return result;
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
	if (ms_phantomBuffer)
	{
		ms_phantomBuffer->Release();
		ms_phantomBuffer = NULL;
	}

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
/**
 * Bind the phantom stream, if any layout has ever needed it.
 *
 * Called by Direct3d11_Draw with every vertex buffer bind: the slot itself is
 * out of reach of the stale-stream unbind loops, but a device ClearState wipes
 * it like everything else, and rebinding one slot is cheaper than tracking that.
 * A no-op until the first phantom layout exists.
 */

void Direct3d11_InputLayoutCache::bindPhantomStream(ID3D11DeviceContext1 *context)
{
	if (!ms_phantomBuffer || !context)
		return;

	UINT const zero = 0;
	context->IASetVertexBuffers(static_cast<UINT>(cs_phantomStreamSlot), 1, &ms_phantomBuffer, &zero, &zero);
}

// ----------------------------------------------------------------------

uint32 Direct3d11_InputLayoutCache::hashVertexShaderSignature(void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize)
{
	if (!vertexShaderBytecode || !vertexShaderBytecodeSize)
		return 0;

	// Key on the shader's INPUT SIGNATURE, so two shaders whose bodies differ but whose inputs
	// match share one layout. Keying on the bytecode pointer would let a freed blob's address
	// alias a live entry, which is why this hashes bytes rather than an address.
	ID3DBlob *signature = NULL;
	HRESULT const signatureResult = D3DGetInputSignatureBlob(vertexShaderBytecode, vertexShaderBytecodeSize, &signature);

	uint32 result;

	if (SUCCEEDED(signatureResult) && signature)
	{
		result = hashBytes(signature->GetBufferPointer(), static_cast<unsigned int>(signature->GetBufferSize()));
		signature->Release();
	}
	else
	{
		// Falling back to the whole bytecode is correct but coarser: two shaders
		// with identical inputs get separate layouts.
		DEBUG_WARNING(true, ("Direct3d11: a vertex shader's input signature could not be extracted; keying the input layout on its whole bytecode instead."));
		result = hashBytes(vertexShaderBytecode, vertexShaderBytecodeSize);
	}

	return result;
}

// ----------------------------------------------------------------------

ID3D11InputLayout *Direct3d11_InputLayoutCache::getInputLayout(uint32 const *formatFlags, int streamCount, void const *vertexShaderBytecode, unsigned int vertexShaderBytecodeSize, uint32 signatureHash, int const *textureCoordinateSetMapping, int mappingCount)
{
	NOT_NULL(ms_layouts);
	NOT_NULL(formatFlags);
	NOT_NULL(vertexShaderBytecode);

	FATAL(streamCount <= 0 || streamCount > MAX_STREAMS, ("Direct3d11: %d vertex streams were bound; this backend handles 1 to %d.", streamCount, static_cast<int>(MAX_STREAMS)));

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

	key.signatureHash = signatureHash;

	LayoutMap::const_iterator const existing = ms_layouts->find(key);
	if (existing != ms_layouts->end())
		return existing->second;

	D3D11_INPUT_ELEMENT_DESC elements[cs_maxInputElements];
	int const elementCount = buildElements(formatFlags, streamCount, textureCoordinateSetMapping, mappingCount, elements, cs_maxInputElements);

	ID3D11InputLayout *layout = NULL;

	if (elementCount > 0)
	{
		ID3D11Device1 *const device = Direct3d11_Device::getDevice();
		NOT_NULL(device);

		HRESULT const hresult = device->CreateInputLayout(elements, static_cast<UINT>(elementCount), vertexShaderBytecode, vertexShaderBytecodeSize, &layout);

		if (FAILED(hresult) || !layout)
		{
			// D3D9 tolerated a shader reading an input the buffers did not supply: the
			// register read back zeros, and shipped data depends on it -- the space nebula
			// quads (position/color/texcoord) feed a_vertexlit.vsh, which also declares a
			// normal, and retail drew them. D3D11 refuses the layout outright, so build
			// D3D9's behaviour explicitly: reflect the shader's inputs, back every
			// unsatisfied one with an element on the phantom stream (slot 15, a
			// stride-zero buffer of zeros, bound by Direct3d11_Draw with the real
			// streams), and create the layout again.
			int const augmentedCount = appendPhantomElements(vertexShaderBytecode, vertexShaderBytecodeSize, elements, elementCount, cs_maxInputElements);
			if (augmentedCount > elementCount)
			{
				HRESULT const retryResult = device->CreateInputLayout(elements, static_cast<UINT>(augmentedCount), vertexShaderBytecode, vertexShaderBytecodeSize, &layout);
				if (SUCCEEDED(retryResult) && layout)
				{
					if (!ms_phantomBuffer)
					{
						D3D11_BUFFER_DESC description;
						Zero(description);
						description.ByteWidth = 32;
						description.Usage = D3D11_USAGE_IMMUTABLE;
						description.BindFlags = D3D11_BIND_VERTEX_BUFFER;

						// Bytes 0..15 zeros; 16..31 float4(1,1,1,1) for COLOR semantics.
						float const contents[8] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
						D3D11_SUBRESOURCE_DATA initial;
						Zero(initial);
						initial.pSysMem = contents;

						HRESULT const bufferResult = device->CreateBuffer(&description, &initial, &ms_phantomBuffer);
						if (FAILED(bufferResult) || !ms_phantomBuffer)
						{
							// Without the backing buffer the layout would read an unbound
							// slot; drop the layout and fall through to the old skip.
							WARNING(true, ("Direct3d11: the phantom stream buffer could not be created (%s).", Direct3d11_Device::describeHresult(bufferResult)));
							layout->Release();
							layout = NULL;
							ms_phantomBuffer = NULL;
						}
					}

					if (layout)
					{
						// The current draw's streams are already bound; make the phantom
						// stream current too rather than waiting for the next bind.
						Direct3d11_InputLayoutCache::bindPhantomStream(Direct3d11_Device::getContext());

						WARNING(true, ("Direct3d11: input layout needed %d phantom element(s) reading zeros -- a vertex shader reads input(s) these %d stream(s) do not supply (first format flags 0x%08x). D3D9 handed such registers zeros and drew; the phantom stream reproduces that.", augmentedCount - elementCount, streamCount, formatFlags[0]));
						++Direct3d11_Metrics::inputLayoutCreations;
					}
				}
			}

			if (!layout)
			{
				// Name the formats. This is the message that has to identify which
				// combination is genuinely unsatisfiable even with phantom elements.
				WARNING(true, ("Direct3d11: CreateInputLayout failed (%s) for %d stream(s) with %d element(s); first format flags 0x%08x. A vertex shader is reading an input these buffers do not supply and the phantom-element retry did not satisfy it.", Direct3d11_Device::describeHresult(hresult), streamCount, elementCount, formatFlags[0]));
				++Direct3d11_Metrics::unsatisfiableInputLayouts;
			}
		}
		else
			++Direct3d11_Metrics::inputLayoutCreations;
	}

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
