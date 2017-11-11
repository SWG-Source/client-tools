// ======================================================================
//
// CuiLayerRenderer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiLayerRenderer_H
#define INCLUDED_CuiLayerRenderer_H


// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Production.h"

#include "UITypes.h"

// ======================================================================

struct UIFloatPoint;
class  VectorArgb;
class  Shader;
struct UILine;
struct UITriangle;

//-----------------------------------------------------------------

/**
* CuiLayerRenderer is a UI optimization that attempts to enqueue as many
* drawing requests as possible before issuing a draw call to the Graphics Layer.
*
* It forces a draw call to the GL whenever the requested render uses a different
* shader than the last request, or the size of the queue exceeds 500.
*/

class CuiLayerRenderer
{
public:

#if PRODUCTION == 0
	struct Metrics
	{
		int quadCount;
		int lineCount;
		int lineCallCount;
		int quadCallCount;
		int triangleCount;
		int triangleCallCount;
	};
#endif

public:

	static void            install ();
	static void            remove ();

	static void            render          (const Shader * shader, const UIFloatPoint verts [4], const UIFloatPoint UVs [4], const VectorArgb & color);
	static void            render          (const Shader * shader, const UIFloatPoint verts [4], const UIFloatPoint UVs [4], const UIColor Colors [4]);
	static void            renderLine      (const Shader * shader, const UILine & line,          const UILine & uvs,         const VectorArgb & color);
	static void            renderLineStrip (const Shader * shader, const int numPoints, const UIFloatPoint * pts,    const UIFloatPoint * uvs, const VectorArgb & color);
	static void            renderLines     (const Shader * shader, const int numLines,  const UILine       * lines,  const UILine * uvs, const VectorArgb & color);
	static void            renderLines     (const Shader * shader, const int numLines,  const UILine       * lines,  const UILine * uvs, const VectorArgb * color);
	static void            renderTriangles (const Shader * shader, int numTris, const UITriangle * tris, const UITriangle * uvs, const VectorArgb & color);

	static void            setZ (real z, real ooz);

	static void            flushRenderQueue ();
	static void            flushRenderQueueIfCurShader (const Shader & shader);

#if PRODUCTION == 0
	static const Metrics & getMetrics ();
	static void            resetMetrics ();
#endif

private:

	                       CuiLayerRenderer ();
	                       CuiLayerRenderer (const CuiLayerRenderer & rhs);
	CuiLayerRenderer &     operator=    (const CuiLayerRenderer & rhs);

	static void            flushRenderQueueQuads     ();
	static void            flushRenderQueueLines     ();
	static void            flushRenderQueueTriangles ();

	static void            renderPointsGeneric (int type, const Shader * shader, const int numPoints, const UIFloatPoint * pts,  const UIFloatPoint * uvs, const VectorArgb & color);
};

// ======================================================================

#endif
