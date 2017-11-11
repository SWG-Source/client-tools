// ======================================================================
//
// Direct3d9.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_H
#define INCLUDED_Direct3d9_H

// ======================================================================

struct Gl_api;
struct Gl_install;
class  HardwareIndexBuffer;
class  HardwareVertexBuffer;
class  StaticShader;
class  Transform;
class  Vector;
class  VertexBufferVector;

#include <d3d9.h>
#include <dxerr9.h>

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "clientGraphics/Texture.def"

class VectorRgba;

// ======================================================================
// Fancy FATAL_DX_HR macro with debug string.
#define FATAL_DX_HR(a,b)       FATAL(FAILED(b), (a, DXGetErrorString9(b)))

class Direct3d9
{
public:

	static IDirect3D9 *         getDirect3d();
	static int                  getAdapter();
	static D3DFORMAT            getAdapterFormat();
	static int                  getShaderCapability();
	static int                  getVideoMemoryInMegabytes();
	static IDirect3DDevice9 *   getDevice();
	static D3DDEVTYPE           getDeviceType();
	static D3DFORMAT            getDepthStencilFormat();
	static D3DCUBEMAP_FACES     getD3dCubeFace(CubeFace cubeFace);

	static bool                 engineOwnsWindow();
	static int                  getMaxRenderTargetWidth();
	static int                  getMaxRenderTargetHeight();

	static bool                 supportsPixelShaders();
	static bool                 supportsVertexShaders();
	static bool                 supportsTwoSidedStencil();
	static bool                 supportsStreamOffsets();
	static bool                 supportsDynamicTextures();
	static bool                 supportsAntialias();
	static DWORD                getMaxAnisotropy();

	static float                getCurrentTime();
	static int                  getFrameNumber();
	static DWORD                getFogColor();

	static void                 setAlphaBlendEnable(bool alphaBlendEnable);
	static void                 setAlphaTestReferenceValue(uint8 alphaTestReferenceValue);
	static void                 setColorWriteEnable(uint8 colorWriteEnable);

#ifdef _DEBUG
	static void                 clearStaticShader();
#endif

	static void *               getTemporaryBuffer(int size);

	static VectorRgba const &   getAlphaFadeAndBloomSettings();

	// the following functions are only here because they need friend access as Direct3d9
	static bool                 install(Gl_install * gl_install);
	static void                 setStaticShader(StaticShader const & shader, int pass);
	static void                 setVertexBuffer(HardwareVertexBuffer const & vertexBuffer);
	static void                 setVertexBufferVector(VertexBufferVector const & vertexBufferVector);
	static void                 setIndexBuffer(HardwareIndexBuffer const & indexBuffer);
	static void                 convertTransformToMatrix(Transform const & transform, D3DMATRIX & matrix);
	static void                 convertScaleAndTransformToMatrix(Vector const & scale, Transform const & transform, D3DMATRIX & matrix);
	static bool                 drawPrimitive();
	static void                 drawPrimitive(D3DPRIMITIVETYPE primitiveType, int startVertex, int primitiveCount);
	static void                 drawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount, int numberOfIndices);

	static void                 startPerformanceTimer();
	static float                stopPerformanceTimer();

	static void                 setAntialiasEnabled(bool enabled);
};

// ======================================================================

#endif
