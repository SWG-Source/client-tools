// ======================================================================
//
// Direct3d11_PointSprite.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_PointSprite.h"

#include "Direct3d11_Device.h"
#include "Direct3d11_Metrics.h"
#include "Direct3d11_ShaderCompiler.h"
#include "Direct3d11_ShaderSignature.h"
#include "Direct3d11_StateCache.h"
#include "Direct3d11_SwapChain.h"

#include <string>

// ======================================================================

namespace Direct3d11_PointSpriteNamespace
{
	// The expander.
	//
	// The signature is stars.vsh's output exactly, which is the only vertex program that ever
	// reaches here; see the header for why one shader is the whole feature. A geometry shader's
	// input has to be a subset of the bound vertex shader's output, so anything else enabling
	// point sprites would fail to bind rather than render wrongly, and that is reported.
	//
	// Two triangles as a strip, corners in the order (-x,-y) (-x,+y) (+x,-y) (+x,+y), which is
	// the strip winding D3D11 wants for a front face under the same cull mode the rest of the
	// scene uses.
	//
	// No texture coordinates. The star pixel program samples nothing -- it multiplies the
	// interpolated colour by the texture factor and stops -- so generating UVs would be output
	// the pixel stage never reads.
	//
	// "point" is a keyword here, and this backend compiles the shader corpus with a global
	// macro renaming it because vertex_shader_constants.inc uses it as a field name. This is
	// compiled with no macros at all, so the keyword survives.
	// The geometry stage uses the SAME shared interpolant declaration as the other two, and that is
	// the whole point rather than tidiness.
	//
	// This shader was written with its own hand-rolled signature -- SV_Position, COLOR0, and FOG as a
	// float -- which was correct in isolation and became wrong the moment the vertex and pixel stages
	// were canonicalised onto twelve float4 slots. A geometry shader sits BETWEEN them, so it has to
	// satisfy both: its input must match the vertex output and its output must satisfy the pixel
	// input. It matched neither, and the D3D11 debug layer produced 397 linkage errors in a single
	// minute in-world:
	//
	//   Vertex Shader - Geometry Shader linkage error: Semantic 'FOG' is defined for mismatched
	//   hardware registers between the output stage and input stage.
	//   Geometry Shader - Pixel Shader linkage error: The input stage requires Semantic/Index ...
	//
	// Every point sprite draw was rejected, so the star field was missing rather than merely
	// single-pixel. Taking the declaration from Direct3d11_ShaderSignature instead of restating it
	// means this cannot drift again: there is one definition and three stages use it.
	//
	// The body still only touches position -- everything else passes through untouched, which is what
	// a sprite expander should do.
	char const cms_sourceTail[] =
		"[maxvertexcount(4)]\n"
		"void main(point SwgInterpolants input[1], inout TriangleStream<SwgInterpolants> stream)\n"
		"{\n"
		"\tSwgInterpolants source = input[0];\n"
		"\n"
		"\t// Multiplying by w cancels the perspective divide, so the quad is a constant pixel size at\n"
		"\t// any depth. That is D3D9's behaviour with POINTSCALEENABLE off.\n"
		"\tfloat2 extent = swgPointSprite.xy * source.swgPosition.w;\n"
		"\n"
		"\tSwgInterpolants corner = source;\n"
		"\n"
		"\tcorner.swgPosition = float4(source.swgPosition.x - extent.x, source.swgPosition.y - extent.y, source.swgPosition.z, source.swgPosition.w);\n"
		"\tstream.Append(corner);\n"
		"\tcorner.swgPosition = float4(source.swgPosition.x - extent.x, source.swgPosition.y + extent.y, source.swgPosition.z, source.swgPosition.w);\n"
		"\tstream.Append(corner);\n"
		"\tcorner.swgPosition = float4(source.swgPosition.x + extent.x, source.swgPosition.y - extent.y, source.swgPosition.z, source.swgPosition.w);\n"
		"\tstream.Append(corner);\n"
		"\tcorner.swgPosition = float4(source.swgPosition.x + extent.x, source.swgPosition.y + extent.y, source.swgPosition.z, source.swgPosition.w);\n"
		"\tstream.Append(corner);\n"
		"\n"
		"\tstream.RestartStrip();\n"
		"}\n";

	// The constant buffer, then the shared declaration, then the body. Assembled at install because
	// the declaration is not a literal here.
	char const cms_constantBuffer[] =
		"cbuffer SwgPointSprite : register(b2)\n"
		"{\n"
		"\tfloat4 swgPointSprite;\n"
		"};\n";

	ID3D11GeometryShader *ms_shader;
	ID3D11Buffer         *ms_constants;

	bool   ms_enabled;
	float  ms_size = 1.0f;
	float  ms_sizeMinimum = 1.0f;
	float  ms_sizeMaximum = 64.0f;
	bool   ms_scaleEnabled;

	// What was last uploaded, so a draw that changes nothing writes nothing.
	float  ms_uploadedX = -1.0f;
	float  ms_uploadedY = -1.0f;

	bool   ms_bound;
	bool   ms_reportedScale;

	int const cms_constantSlot = 2;
}

using namespace Direct3d11_PointSpriteNamespace;

// ======================================================================

void Direct3d11_PointSprite::install()
{
	ID3D11Device1 * const device = Direct3d11_Device::getDevice();
	if (!device)
		return;

	// Compiled once at install rather than on first use. A shader compile inside a frame is
	// exactly what Direct3d11_Metrics::shaderCompiles exists to catch, and the star field draws
	// in the first frame of a scene.
	std::string source;
	source.reserve(4096);
	source.append(cms_constantBuffer);
	source.append(Direct3d11_ShaderSignature::getInterpolantDeclaration());
	source.append(cms_sourceTail);

	ID3DBlob * const bytecode = Direct3d11_ShaderCompiler::compileGeometryShader(source.c_str(), static_cast<int>(source.size()), "Direct3d11 point sprite expander");

	if (!bytecode)
	{
		// The compiler has already said why. Point sprites simply stay off, which leaves single
		// pixel points rather than nothing at all.
		WARNING(true, ("Direct3d11: the point sprite geometry shader did not compile, so point sprites will render as single pixels."));
		return;
	}

	HRESULT hresult = device->CreateGeometryShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &ms_shader);
	bytecode->Release();

	if (FAILED(hresult) || !ms_shader)
	{
		WARNING(true, ("Direct3d11: the point sprite geometry shader could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		ms_shader = NULL;
		return;
	}

	D3D11_BUFFER_DESC description;
	Zero(description);
	description.ByteWidth      = 16;
	description.Usage          = D3D11_USAGE_DYNAMIC;
	description.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hresult = device->CreateBuffer(&description, NULL, &ms_constants);
	if (FAILED(hresult) || !ms_constants)
	{
		WARNING(true, ("Direct3d11: the point sprite constant buffer could not be created (%s).", Direct3d11_Device::describeHresult(hresult)));
		ms_shader->Release();
		ms_shader = NULL;
		ms_constants = NULL;
		return;
	}

	++Direct3d11_Metrics::constantBufferCreations;
}

// ----------------------------------------------------------------------

void Direct3d11_PointSprite::remove()
{
	if (ms_constants)
	{
		ms_constants->Release();
		ms_constants = NULL;
	}

	if (ms_shader)
	{
		ms_shader->Release();
		ms_shader = NULL;
	}

	ms_bound = false;
}

// ======================================================================

void Direct3d11_PointSprite::setEnabled(bool enabled)
{
	ms_enabled = enabled;
}

// ----------------------------------------------------------------------

void Direct3d11_PointSprite::setSize(float size)
{
	ms_size = size;
}

// ----------------------------------------------------------------------

float Direct3d11_PointSprite::getSize()
{
	return ms_size;
}

// ----------------------------------------------------------------------

bool Direct3d11_PointSprite::isEnabled()
{
	return ms_enabled;
}

// ----------------------------------------------------------------------

void Direct3d11_PointSprite::setSizeMinimum(float size)
{
	ms_sizeMinimum = size;
}

// ----------------------------------------------------------------------

void Direct3d11_PointSprite::setSizeMaximum(float size)
{
	ms_sizeMaximum = size;
}

// ----------------------------------------------------------------------
/**
 * Distance attenuation, which has no caller and is not implemented.
 *
 * Recorded rather than dropped so the state is not silently lost, and reported if it is ever
 * switched on -- with nothing in the engine asking for it there is no behaviour to reproduce and
 * no way to check a guess.
 */

void Direct3d11_PointSprite::setScaleEnabled(bool enabled)
{
	ms_scaleEnabled = enabled;

	if (enabled && !ms_reportedScale)
	{
		ms_reportedScale = true;
		WARNING(true, ("Direct3d11: point size distance attenuation was switched on. Nothing in the engine did this when the backend was written, so it is not implemented and point sprites will stay a constant pixel size. The scale factors are in Direct3d11_PointSprite."));
	}
}

// ----------------------------------------------------------------------

void Direct3d11_PointSprite::setScaleFactor(float a, float b, float c)
{
	UNREF(a);
	UNREF(b);
	UNREF(c);
}

// ======================================================================

void Direct3d11_PointSprite::apply(bool isPointList)
{
	bool const want = isPointList && ms_enabled && ms_shader && ms_constants;

	if (!want)
	{
		// Unbinding matters more than binding. A geometry shader left bound would expand the
		// next triangle list's vertices into quads, which is not a subtle failure.
		if (ms_bound)
		{
			ms_bound = false;
			Direct3d11_StateCache::setGeometryShader(NULL);
		}
		return;
	}

	ID3D11DeviceContext1 * const context = Direct3d11_Device::getContext();
	if (!context)
		return;

	// The half-extent in clip space. NDC spans -1..1 across the viewport, so one pixel is
	// 2/dimension and half of a size-pixel quad is size/dimension.
	float const width  = static_cast<float>(Direct3d11_SwapChain::getWidth());
	float const height = static_cast<float>(Direct3d11_SwapChain::getHeight());

	if (width <= 0.0f || height <= 0.0f)
		return;

	float size = ms_size;
	if (size < ms_sizeMinimum)
		size = ms_sizeMinimum;
	if (size > ms_sizeMaximum)
		size = ms_sizeMaximum;

	float const extentX = size / width;
	float const extentY = size / height;

	if (extentX != ms_uploadedX || extentY != ms_uploadedY)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		Zero(mapped);

		if (SUCCEEDED(context->Map(ms_constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			float const values[4] = { extentX, extentY, 0.0f, 0.0f };
			memcpy(mapped.pData, values, sizeof(values));
			context->Unmap(ms_constants, 0);

			ms_uploadedX = extentX;
			ms_uploadedY = extentY;

			Direct3d11_Metrics::constantBufferBytes += static_cast<int>(sizeof(values));
			++Direct3d11_Metrics::constantBufferUpdates;
		}
	}

	if (!ms_bound)
	{
		ms_bound = true;
		Direct3d11_StateCache::setGeometryShader(ms_shader);
		context->GSSetConstantBuffers(static_cast<UINT>(cms_constantSlot), 1, &ms_constants);
	}
}

// ======================================================================
