// ======================================================================
//
// CuiLayerRenderer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayerRenderer.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/VertexBuffer.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedMath/VectorArgb.h"

#include "UITypes.h"

// ======================================================================

namespace CuiLayerRendereNamespace
{
#if PRODUCTION == 0
	CuiLayerRenderer::Metrics  s_metrics;
#endif

	real s_z   = 1.0f;
	real s_ooz = 1.0f;

	const Shader * s_curShader;

	enum VertexInfoType
	{
		VIT_none,
		VIT_quad,
		VIT_quadStrip,
		VIT_line,
		VIT_lineStrip,
		VIT_triangleList
	};

	VertexInfoType s_vertexType = VIT_none;

	bool                      s_installed;
	DynamicVertexBuffer *     s_vertexBuffer = 0;
	VertexBufferWriteIterator s_vertexBufferWriteIterator;
	int                       s_numberOfVertices;
	int                       s_maxNumberOfVertices;

	void lockDynamicVertexBuffer();
	bool lockDynamicVertexBuffer(int minVerticesRequired);
}
using namespace CuiLayerRendereNamespace;

//----------------------------------------------------------------------

void CuiLayerRendereNamespace::lockDynamicVertexBuffer()
{
	s_maxNumberOfVertices = s_vertexBuffer->getNumberOfLockableDynamicVertices(false);
	if (s_maxNumberOfVertices < 128)
		s_maxNumberOfVertices = s_vertexBuffer->getNumberOfLockableDynamicVertices(true);

	s_vertexBuffer->lock(s_maxNumberOfVertices);
	s_vertexBufferWriteIterator = s_vertexBuffer->begin();
}

//----------------------------------------------------------------------

bool CuiLayerRendereNamespace::lockDynamicVertexBuffer(int minVerticesRequired)
{
	s_maxNumberOfVertices = s_vertexBuffer->getNumberOfLockableDynamicVertices(false);
	
	if (s_maxNumberOfVertices < minVerticesRequired)
		s_maxNumberOfVertices = s_vertexBuffer->getNumberOfLockableDynamicVertices(true);

	if (s_maxNumberOfVertices < minVerticesRequired)
	{
		s_maxNumberOfVertices = 0;
		return false;
	}

	s_vertexBuffer->lock(s_maxNumberOfVertices);
	s_vertexBufferWriteIterator = s_vertexBuffer->begin();

	return true;
}


//-----------------------------------------------------------------

void           CuiLayerRenderer::install ()
{
	DEBUG_FATAL (s_installed, ("already installed.\n"));

	InstallTimer const installTimer("CuiLayerRenderer");

	s_curShader = 0;
	s_installed = true;

	//-- build vertexarray
	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	s_vertexBuffer = new DynamicVertexBuffer(format);
}

//-----------------------------------------------------------------

void           CuiLayerRenderer::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	s_installed = false;

	if (s_curShader)
	{
		s_curShader->release ();
		s_curShader = 0;
	}

	delete s_vertexBuffer;
	s_vertexBuffer = NULL;
}

//-----------------------------------------------------------------

#if PRODUCTION == 0

const CuiLayerRenderer::Metrics & CuiLayerRenderer::getMetrics ()
{
	return s_metrics;
}

#endif

//-----------------------------------------------------------------

#if PRODUCTION == 0

void CuiLayerRenderer::resetMetrics ()
{
	s_metrics.quadCount          = 0;
	s_metrics.quadCallCount      = 0;
	s_metrics.lineCount          = 0;
	s_metrics.lineCallCount      = 0;
	s_metrics.triangleCount      = 0;
	s_metrics.triangleCallCount  = 0;
}

#endif

//-----------------------------------------------------------------

void           CuiLayerRenderer::setZ (real z, real ooz)
{
	s_z = z;
	s_ooz = ooz;
}

//-----------------------------------------------------------------

void CuiLayerRenderer::render (const Shader * shader, const UIFloatPoint verts [4], const UIFloatPoint UVs [4], const UIColor Colors [4])
{
	if (s_vertexType != VIT_quad || (s_curShader && shader != s_curShader || s_numberOfVertices+4 > s_maxNumberOfVertices))
		flushRenderQueue ();

	if (s_numberOfVertices == 0)
		lockDynamicVertexBuffer();

	s_vertexType = VIT_quad;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	// This guarantees the quads are input in clockwise order
	// which the graphics system expects

	static const int s_transform_verts [4] =
	{
		0, 1, 3, 2
	};

	for (int i = 0; i < 4; ++i, ++s_vertexBufferWriteIterator, ++s_numberOfVertices)
	{
		int transformIndex = s_transform_verts [i];
		const UIFloatPoint & vert = verts [transformIndex];
		UIColor const & vertColor = Colors[transformIndex];

		s_vertexBufferWriteIterator.setPosition(vert.x + pixOffset, vert.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(vertColor.FormatRGBA());
		s_vertexBufferWriteIterator.setTextureCoordinates(0, UVs[transformIndex].x, UVs[transformIndex].y);
	}
}

//-----------------------------------------------------------------

void CuiLayerRenderer::render (const Shader * shader, const UIFloatPoint verts [4], const UIFloatPoint UVs [4], const VectorArgb & color)
{
	UNREF (shader);
	UNREF (verts);
	UNREF (UVs);
	UNREF (color);

	UNREF (verts);
	UNREF (UVs);

	if (s_vertexType != VIT_quad || (s_curShader && shader != s_curShader || s_numberOfVertices+4 > s_maxNumberOfVertices))
		flushRenderQueue ();

	if (s_numberOfVertices == 0)
		lockDynamicVertexBuffer();

	s_vertexType = VIT_quad;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	// This guarantees the quads are input in clockwise order
	// which the graphics system expects

	static const int s_transform_verts [4] =
	{
		0, 1, 3, 2
	};

	uint32 const color32 = color.convertToUint32NoClamp();
	for (int i = 0; i < 4; ++i, ++s_vertexBufferWriteIterator, ++s_numberOfVertices)
	{
		const UIFloatPoint & vert = verts [s_transform_verts [i]];
		s_vertexBufferWriteIterator.setPosition(vert.x + pixOffset, vert.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, UVs[s_transform_verts [i]].x, UVs[s_transform_verts [i]].y);
	}
}

//----------------------------------------------------------------------

void  CuiLayerRenderer::renderPointsGeneric (int type, const Shader * shader, const int numPoints, const UIFloatPoint * pts,  const UIFloatPoint * uvs, const VectorArgb & color)
{
	if (s_vertexType != type || (s_curShader && shader != s_curShader || s_numberOfVertices + numPoints > s_maxNumberOfVertices ))
		flushRenderQueue ();

	if (s_numberOfVertices == 0 && !lockDynamicVertexBuffer(numPoints))
	{
		DEBUG_WARNING(true, ("CuiLayerRenderer::renderPointsGeneric: can't allocate sufficient vertices [%d]", numPoints));
		return;
	}

	s_vertexType = static_cast<VertexInfoType>(type);

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	uint32 const color32 = color.convertToUint32NoClamp();
	for (int i = 0; i < numPoints; ++i, ++s_vertexBufferWriteIterator, ++s_numberOfVertices)
	{
		const UIFloatPoint & pt = pts [i];
		const UIFloatPoint & uv = uvs [i];

		s_vertexBufferWriteIterator.setPosition(pt.x + pixOffset, pt.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.x, uv.y);
	}
}

//----------------------------------------------------------------------

void CuiLayerRenderer::renderLineStrip  (const Shader * shader, const int numPoints, const UIFloatPoint * pts,  const UIFloatPoint * uvs, const VectorArgb & color)
{
	renderPointsGeneric (VIT_lineStrip, shader, numPoints, pts, uvs, color);
}

//----------------------------------------------------------------------

void CuiLayerRenderer::renderLines     (const Shader * shader, const int numLines,  const UILine       * lines,  const UILine * uvs, const VectorArgb & color)
{
	int const requiredVertices = numLines * 2;
	if (s_vertexType != VIT_line || (s_curShader && shader != s_curShader || s_numberOfVertices > 500 || s_numberOfVertices + requiredVertices > s_maxNumberOfVertices ))
		flushRenderQueue ();

	if (s_numberOfVertices == 0 && !lockDynamicVertexBuffer(requiredVertices))
	{
		DEBUG_WARNING(true, ("CuiLayerRenderer::renderLines #1: can't allocate sufficient vertices [%d]", requiredVertices));
		return;
	}

	s_vertexType = VIT_line;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();
	uint32 const color32 = color.convertToUint32NoClamp();

	for (int i = 0; i < numLines; ++i)
	{
		const UILine & line = lines [i];
		const UILine & uv   = uvs   [i];

		s_vertexBufferWriteIterator.setPosition(line.p1.x + pixOffset, line.p1.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p1.x, uv.p1.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;

		s_vertexBufferWriteIterator.setPosition(line.p2.x + pixOffset, line.p2.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p2.x, uv.p2.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;
	}
}

//----------------------------------------------------------------------

void CuiLayerRenderer::renderLines(const Shader * shader, const int numLines, const UILine * lines, const UILine * uvs, const VectorArgb * colors)
{
	int const requiredVertices = numLines * 2;
	if (s_vertexType != VIT_line || (s_curShader && shader != s_curShader || s_numberOfVertices > 500 || s_numberOfVertices + requiredVertices > s_maxNumberOfVertices ))
		flushRenderQueue ();

	if (s_numberOfVertices == 0 && !lockDynamicVertexBuffer(requiredVertices))
	{
		DEBUG_WARNING(true, ("CuiLayerRenderer::renderLines #2: can't allocate sufficient vertices [%d]", requiredVertices));
		return;
	}

	s_vertexType = VIT_line;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	for (int i = 0; i < numLines; ++i)
	{
		const UILine & line = lines [i];
		const UILine & uv   = uvs   [i];

		int transformIndex = i * 2;

		s_vertexBufferWriteIterator.setPosition(line.p1.x + pixOffset, line.p1.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(colors[transformIndex].convertToUint32NoClamp());
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p1.x, uv.p1.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;

		s_vertexBufferWriteIterator.setPosition(line.p2.x + pixOffset, line.p2.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(colors[transformIndex+1].convertToUint32NoClamp());
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p2.x, uv.p2.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;
	}
}

//----------------------------------------------------------------------

void CuiLayerRenderer::renderTriangles (const Shader * shader, int numTris, const UITriangle * tris, const UITriangle * uvs, const VectorArgb & color)
{
	DEBUG_FATAL(numTris == 0, ("numTris is 0"));

	int const requiredVertices = numTris * 3;
	if (s_vertexType != VIT_triangleList || (s_curShader && shader != s_curShader || s_numberOfVertices > 500 || s_numberOfVertices + requiredVertices > s_maxNumberOfVertices ))
		flushRenderQueue ();

	if (s_numberOfVertices == 0 && !lockDynamicVertexBuffer(requiredVertices))
	{
		DEBUG_WARNING(true, ("CuiLayerRenderer::renderTriangles: can't allocate sufficient vertices [%d]", requiredVertices));
		return;
	}

	s_vertexType = VIT_triangleList;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	uint32 const color32 = color.convertToUint32NoClamp();

	for (int i = 0; i < numTris; ++i)
	{
		const UITriangle & tri = tris  [i];
		const UITriangle & uv  = uvs   [i];

		s_vertexBufferWriteIterator.setPosition(tri.p1.x + pixOffset, tri.p1.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p1.x, uv.p1.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;


		s_vertexBufferWriteIterator.setPosition(tri.p2.x + pixOffset, tri.p2.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p2.x, uv.p2.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;


		s_vertexBufferWriteIterator.setPosition(tri.p3.x + pixOffset, tri.p3.y + pixOffset, s_z);
		s_vertexBufferWriteIterator.setOoz(s_ooz);
		s_vertexBufferWriteIterator.setColor0(color32);
		s_vertexBufferWriteIterator.setTextureCoordinates(0, uv.p3.x, uv.p3.y);
		++s_vertexBufferWriteIterator;
		++s_numberOfVertices;
	}
}

//----------------------------------------------------------------------

void CuiLayerRenderer::renderLine (const Shader * shader, const UILine & verts, const UILine & UVs, const VectorArgb & color)
{
	if (s_vertexType != VIT_line || (s_curShader && shader != s_curShader || s_numberOfVertices > 500))
		flushRenderQueue ();

	if (s_numberOfVertices == 0)
		lockDynamicVertexBuffer();

	s_vertexType = VIT_line;

	if (shader != s_curShader)
	{
		if (shader)
			shader->fetch ();
		if (s_curShader)
			s_curShader->release ();
		s_curShader = shader;
	}

	const real pixOffset = CuiManager::getPixelOffset ();

	uint32 const color32 = color.convertToUint32NoClamp();

	s_vertexBufferWriteIterator.setPosition(verts.p1.x + pixOffset, verts.p1.y + pixOffset, s_z);
	s_vertexBufferWriteIterator.setOoz(s_ooz);
	s_vertexBufferWriteIterator.setColor0(color32);
	s_vertexBufferWriteIterator.setTextureCoordinates(0, UVs.p1.x, UVs.p1.y);
	++s_vertexBufferWriteIterator;
	++s_numberOfVertices;

	s_vertexBufferWriteIterator.setPosition(verts.p2.x + pixOffset, verts.p2.y + pixOffset, s_z);
	s_vertexBufferWriteIterator.setOoz(s_ooz);
	s_vertexBufferWriteIterator.setColor0(color32);
	s_vertexBufferWriteIterator.setTextureCoordinates(0, UVs.p2.x, UVs.p2.y);
	++s_vertexBufferWriteIterator;
	++s_numberOfVertices;
}

//-----------------------------------------------------------------

void CuiLayerRenderer::flushRenderQueueIfCurShader (const Shader & shader)
{
	if (s_curShader == &shader)
		flushRenderQueue ();
}

//----------------------------------------------------------------------

void CuiLayerRenderer::flushRenderQueueQuads ()
{
	DEBUG_FATAL(!s_vertexBuffer, ("not installed"));
	DEBUG_FATAL (s_numberOfVertices == 0, ("vertex info is empty\n"));

#if PRODUCTION == 0
	++s_metrics.quadCallCount;
	s_metrics.quadCount += s_numberOfVertices / 4;
#endif

	s_vertexBuffer->unlock(s_numberOfVertices);

	Graphics::setStaticShader (s_curShader->prepareToView());
	Graphics::setVertexBuffer ( *s_vertexBuffer );
	Graphics::drawQuadList    ();

	s_numberOfVertices = 0;
}

//----------------------------------------------------------------------

void CuiLayerRenderer::flushRenderQueueLines ()
{
	DEBUG_FATAL (s_numberOfVertices == 0, ("vertex info is empty\n"));

#if PRODUCTION == 0
	++s_metrics.lineCallCount;
	s_metrics.lineCount += s_numberOfVertices / 2;
#endif

	s_vertexBuffer->unlock(s_numberOfVertices);

	Graphics::setStaticShader   (s_curShader->prepareToView());
	Graphics::setVertexBuffer   (*s_vertexBuffer);
	Graphics::drawLineList  ();

	s_numberOfVertices = 0;
}

//----------------------------------------------------------------------

void CuiLayerRenderer::flushRenderQueueTriangles ()
{
	DEBUG_FATAL (s_numberOfVertices == 0, ("vertex info is empty\n"));

#if PRODUCTION == 0
	++s_metrics.triangleCallCount;
	s_metrics.triangleCount += s_numberOfVertices / 3;
#endif

	s_vertexBuffer->unlock(s_numberOfVertices);

	Graphics::setStaticShader   (s_curShader->prepareToView());
	Graphics::setVertexBuffer   (*s_vertexBuffer);
	Graphics::drawTriangleList  ();

	s_numberOfVertices = 0;
}

//----------------------------------------------------------------------

void CuiLayerRenderer::flushRenderQueue ()
{
	if (s_curShader == 0 || s_numberOfVertices == 0)
		return;

	if (s_vertexType == VIT_quad)
		flushRenderQueueQuads ();
	else if (s_vertexType == VIT_line)
		flushRenderQueueLines ();
	else if (s_vertexType == VIT_triangleList)
		flushRenderQueueTriangles ();
	else
		WARNING (true, ("Unhandled vertex info type in CuiLayer"));
}

// ======================================================================
