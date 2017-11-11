// ======================================================================
//
// Graphics.h
// Copyright 1998 Bootprint Entertainment
// Copyright 2000-2002 Sony Online Entertainment Inc
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_Graphics_H
#define INCLUDED_Graphics_H

// ======================================================================

struct Gl_api;

class DynamicIndexBuffer;
class DynamicIndexBufferGraphicsData;
class DynamicVertexBuffer;
class DynamicVertexBufferGraphicsData;
class Extent;
class HardwareIndexBuffer;
class HardwareVertexBuffer;
class Light;
class MeshConstructionHelper;
class Rectangle2d;
class ShaderImplementation;
class ShaderImplementationGraphicsData;
class ShaderImplementationPassPixelShaderProgram;
class ShaderImplementationPassPixelShaderProgramGraphicsData;
class ShaderImplementationPassVertexShader;
class ShaderImplementationPassVertexShaderGraphicsData;
class StaticIndexBuffer;
class StaticIndexBufferGraphicsData;
class StaticShader;
class StaticShaderGraphicsData;
class StaticVertexBuffer;
class StaticVertexBufferGraphicsData;
class StringId;
class Texture;
class TextureGraphicsData;
class Transform;
class Vector;
class VectorArgb;
class VectorRgba;
class VertexBufferFormat;
class VertexBufferReadIterator;
class VertexBufferVector;
class VertexBufferVectorGraphicsData;

#include "clientGraphics/Graphics.def"
#include "clientGraphics/Texture.def"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedMath/VectorArgb.h"

// ==================================================================

class Graphics
{
	friend class Bloom;

public:

	typedef void (*TranslatePointFromGameToScreen)(int &x, int &y);
	typedef void (*CallbackFunction)();
	typedef stdvector<const Light*>::fwd LightList;

public:

	static bool                          install();
	static void                          setTranslatePointFromGameToScreen(TranslatePointFromGameToScreen translatePointFromGameToScreen);

	static int                           getShaderCapability();
	static bool                          requiresVertexAndPixelShaders();
	static int                           getVideoMemoryInMegabytes();

	static StringId const &              getLastError();
	static DLLEXPORT void                setLastError(char const * stringTable, char const * stringText);
	static void                          clearLastError();

	static void                          flushResources(bool fullReset);

	static int                           getFrameNumber();
	static bool                          isGdiVisible();
	static bool                          wasDeviceReset();

	static void                          addDeviceLostCallback(CallbackFunction callbackFunction);
	static void                          removeDeviceLostCallback(CallbackFunction callbackFunction);
	static void                          addDeviceRestoredCallback(CallbackFunction callbackFunction);
	static void                          removeDeviceRestoredCallback(CallbackFunction callbackFunction);

#ifdef _DEBUG
	static void                          setTexturesEnabled(bool enabled);
	static void                          showMipmapLevels(bool enabled);
	static bool                          getShowMipmapLevels();
	static void                          setBadVertexBufferVertexShaderCombination(bool *badDrawFlag, const char *debugAppearanceName);
	static void                          clearStaticShader();
	static void                          getRenderedVerticesPointsLinesTrianglesCalls(int &vertices, int &points, int &lines, int &triangles, int &calls);
#endif

	static int                           getFrameBufferMaxWidth();
	static int                           getFrameBufferMaxHeight();
	static int                           getCurrentRenderTargetWidth();
	static int                           getCurrentRenderTargetHeight();
	static int                           getCurrentRenderTargetMaxWidth();
	static int                           getCurrentRenderTargetMaxHeight();
	static int                           getStencilBufferBitDepth();
	static int                           getMaximumVertexBufferStreamCount();
	static bool                          supportsMipmappedCubeMaps();
	static bool                          supportsScissorRect();
	static bool                          supportsTwoSidedStencil();
	static bool                          supportsStreamOffsets();
	static bool                          supportsDynamicTextures();

	static void                          getBrightnessContrastGamma(float &brightness, float &contrast, float &gamma);
	static void                          setBrightnessContrastGamma(float brightness, float contrast, float gamma);

	static float                         getBrightness();
	static float                         getContrast();
	static float                         getGamma();

	static float                         getDefaultBrightness();
	static float                         getDefaultContrast();
	static float                         getDefaultGamma();

	static void                          setBrightness(float brightness);
	static void                          setContrast(float contrast);
	static void                          setGamma(float gamma);

	static void                          resize(int newWidth, int newHeight);
	static void                          setWindowedMode(bool windowed);
	static void                          toggleWindowedMode();
	static bool                          isWindowed();

	static GlFillMode                    getFillMode();
	static void                          setFillMode(GlFillMode newFillMode);
	static GlCullMode                    getCullMode();
	static void                          setCullMode(GlCullMode newCullMode);

	static float                         getPointSize();
	static void                          setPointSize(float size);
	static float                         getPointSizeMin();
	static void                          setPointSizeMin(float size);
	static float                         getPointSizeMax();
	static void                          setPointSizeMax(float size);
	static bool                          getPointScaleEnable();
	static void                          setPointScaleEnable(bool bEnable);
	static void                          getPointScaleFactor(float & outA, float & outB, float & outC );
	static void                          setPointScaleFactor(float A, float B, float C);
	static bool                          getPointSpriteEnable();
	static void                          setPointSpriteEnable(bool bEnable);

	static void                          update(float elapsedTime);
	static void                          clearViewport(bool clearColor, uint32 colorValue, bool clearDepth, float depthValue, bool clearStencil, uint32 stencilValue);
	static void                          beginScene();
	static void                          endScene();

	static bool                          lockBackBuffer(Gl_pixelRect &o_pixels, const Gl_rect *i_lockRect=0);
	static bool                          unlockBackBuffer();

	static bool                          present();
	static bool                          present(HWND window, int width, int height);
	static void                          setRenderTarget(Texture *texture, CubeFace cubeFace, int mipmapLevel);
	static bool                          copyRenderTargetToNonRenderTargetTexture();

	static GlScreenShotFormat            getScreenShotFormat();
	static void                          setScreenShotFormat(GlScreenShotFormat screenShotFormat);
	static int                           getScreenShotQuality();
	static void                          setScreenShotQuality(int quality);
	static bool                          screenShot(const char *fileName);

	static ShaderImplementationGraphicsData  *createShaderImplementationGraphicsData(const ShaderImplementation &shaderImplementation);
	static StaticShaderGraphicsData          *createStaticShaderGraphicsData(const StaticShader &shader);
	static void                               setBadVertexShaderStaticShader(const StaticShader *shader);
	static void                               setStaticShader(const StaticShader &shader, int pass=0);

	static void                          setHardwareMouseCursorEnabled(bool enabled);
	static bool                          getHardwareMouseCursorEnabled();
	static bool                          setMouseCursor(const Texture &mouseCursorTexture, int hotSpotX, int hotSpotY);
	static bool                          showMouseCursor(bool cursorVisible);
	static void                          setSystemMouseCursorPosition(int x, int y);
	static void                          constrainMouseCursor(int x0, int y0, int x1, int y1);
	static void                          unconstrainMouseCursor();
	static void                          setAllowMouseCursorConstrained(bool allowMouseCursorConstrained);

	static void                          setViewport(int x, int y, int width, int height, float minZ=0.0f, float maxZ=1.0f);
	static void                          setScissorRect(bool enabled, int x, int y, int width, int height);
	static void                          setWorldToCameraTransform(const Transform &transform, const Vector &cameraPosition);
	static void                          setProjectionMatrix(const GlMatrix4x4 &projectionMatrix);
	static void                          setFog(bool enabled, float density, const PackedArgb &color);
	static void                          getFog(bool & enabled, float & density, PackedArgb & color);
	static void                          setObjectToWorldTransformAndScale(const Transform &objectToWorld, const Vector &scale);
	static void                          setGlobalTexture(Tag tag, const Texture &texture);
	static void                          releaseAllGlobalTextures();
	static void                          setTextureTransform(int stage, bool enabled, int dimension, bool projected, const float *transform);
	static void                          setVertexShaderUserConstants(int index, float c0, float c1=0.f, float c2=0.f, float c3=0.f);
	static void                          setPixelShaderUserConstants(VectorRgba const * constants, int count);

	static void                          setAlphaFadeOpacity(bool enabled, float opacity);

	// light routines
	static void                          setLights(LightList const & lightList);

	// vertex buffer routines
	static void                               setVertexBuffer(const HardwareVertexBuffer &vertexBuffer);
	static void                               setVertexBuffer(VertexBufferVector const & vertexBufferVector);
	static StaticVertexBufferGraphicsData    *createVertexBufferData(const StaticVertexBuffer &vertexBuffer);
	static DynamicVertexBufferGraphicsData   *createVertexBufferData(const DynamicVertexBuffer &vertexBuffer);
	static VertexBufferVectorGraphicsData    *createVertexBufferVectorData(VertexBufferVector const & vertexBufferVector);

	// index buffer routines
	static void                              setDynamicIndexBufferSize(int numberOfIndices);
	static void                              setIndexBuffer(const HardwareIndexBuffer &indexBuffer);
	static StaticIndexBufferGraphicsData    *createIndexBufferData(const StaticIndexBuffer &indexBuffer);
	static DynamicIndexBufferGraphicsData   *createIndexBufferData();

	// texturing-related routines
	static void                          getOneToOneUVMapping(int textureWidth, int textureHeight, float &u0, float &v0, float &u1, float &v1);
	static TextureGraphicsData          *createTextureData(const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats);

	static ShaderImplementationPassVertexShaderGraphicsData       *createVertexShaderData(ShaderImplementationPassVertexShader const &vertexShader);
	static ShaderImplementationPassPixelShaderProgramGraphicsData *createPixelShaderProgramData(ShaderImplementationPassPixelShaderProgram const &pixelShaderProgram);

	// helper draw routines
	static void                          drawPoint(int x, int y, const VectorArgb &argb);
	static void                          drawPoint(float x, float y, const VectorArgb &argb);
	static void                          drawLine(int x0, int y0, int x1, int y1, const VectorArgb &argb);
	static void                          drawLine(float x0, float y0, float x1, float y1, const VectorArgb &argb);
	static void                          drawCircle(int x, int y, int r, const VectorArgb &argb);
	static void                          drawRectangle(int x0, int y0, int x1, int y1, const VectorArgb &argb);
	static void                          drawRectangle(float x0, float y0, float x1, float y1, const VectorArgb &argb);
	static void                          drawRectangle(const Rectangle2d &rectangle, const VectorArgb &argb);
	static void                          drawPoint(const Vector &v, const VectorArgb &argb);
	static void                          drawLine(const Vector &v0, const Vector &v1, const VectorArgb &argb);
	static void                          drawFrame(float radius=1.0f, uint8 alpha=0);
	static void                          drawCube(const Vector &center, float radius,  const VectorArgb &argb);
	static void                          drawCircle(const Vector &center, const Vector &normal, float radius, int segments, const VectorArgb &argb);
	static void                          drawSphere(const Vector &center, float radius, int rings, int segments);
	static void                          drawSphere2(const Vector &center, float radius, int tessTheta, int nLongitudes, int nLatitudes, const VectorArgb & color);
	static void                          drawXZCircle(const Vector &center, float radius, int tessTheta, const VectorArgb & color);
	static void                          drawOctahedron(const Vector &center, float radius, const VectorArgb & color);
	static void                          drawCylinder(const Vector &base, float radius, float height, int tessTheta, int tessRho, int tessZ, int nSpokes, const VectorArgb &argb);
	static void                          drawFrustum(const Vector *frustumVertices, const PackedArgb &color);
	static void                          drawBox(const Vector &leftRearBottomCorner, float height, float width, float depth, const VectorArgb &argb);
	static void                          drawBox(const Vector &extentMin, const Vector& extentMax, const VectorArgb &argb);
	static void                          drawVertexNormals(VertexBufferReadIterator first, const VertexBufferReadIterator &last, float scale = 1, const VectorArgb &argbLine = VectorArgb::solidGreen,	const VectorArgb &argbEndpoint = VectorArgb::solidRed);
	static void                          drawVertexMatrixFrames(VertexBufferReadIterator first, const VertexBufferReadIterator &last, float scale = 0.1f);
	static void                          drawExtent(const Extent* extent, const VectorArgb& color);

	// draw routines

	static void                          drawPointList();
	static void                          drawLineList();
	static void                          drawLineStrip();
	static void                          drawTriangleList();
	static void                          drawTriangleStrip();
	static void                          drawTriangleFan();
	static void                          drawQuadList();

	static void                          drawIndexedPointList();
	static void                          drawIndexedLineList();
	static void                          drawIndexedLineStrip();
	static void                          drawIndexedTriangleList();
	static void                          drawIndexedTriangleStrip();
	static void                          drawIndexedTriangleFan();

	static void                          drawPointList(int startVertex, int primitiveCount);
	static void                          drawLineList(int startVertex, int primitiveCount);
	static void                          drawLineStrip(int startVertex, int primitiveCount);
	static void                          drawTriangleList(int startVertex, int primitiveCount);
	static void                          drawTriangleStrip(int startVertex, int primitiveCount);
	static void                          drawTriangleFan(int startVertex, int primitiveCount);

	static void                          drawIndexedPointList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static void                          drawIndexedLineList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static void                          drawIndexedLineStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static void                          drawIndexedTriangleList(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static void                          drawIndexedTriangleStrip(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);
	static void                          drawIndexedTriangleFan(int baseIndex, int minimumVertexIndex, int numberOfVertices, int startIndex, int primitiveCount);

	static void                          optimizeIndexBuffer(Index *indices, int numIndices);

	static void                          pixSetMarker(WCHAR const * markerName);
	static void                          pixBeginEvent(WCHAR const * eventName);
	static void                          pixEndEvent(WCHAR const * eventName);

	static bool DLLEXPORT                writeImage(char const * file, int const width, int const height, int const pitch, int const * pixelsARGB, bool const alphaExtend, Gl_imageFormat const imageFormat, Rectangle2d const * subRect);

	static bool                          supportsAntialias();
	static void                          setAntialiasEnabled(bool antialiasEnable);

	// video capture routines

#if PRODUCTION == 0
	static bool                          createVideoBuffers(int width, int height);
	static void                          fillVideoBuffers();
	static bool                          getVideoBufferData(void *buffer, size_t bufferSize);
	static void                          releaseVideoBuffers();
#endif // PRODUCTION

private:

	static void                          setBloomEnabled(bool enabled);

private:

	// disable these
	Graphics();
	Graphics(const Graphics &);
	Graphics &operator =(const Graphics &);
};

// ======================================================================

#endif
