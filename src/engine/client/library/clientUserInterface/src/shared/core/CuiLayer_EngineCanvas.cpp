//======================================================================
//
// CuiLayer_EngineCanvas.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiLayer_EngineCanvas.h"
#include "clientUserInterface/CuiLayer_TextureCanvas.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedMath/PackedArgb.h"

#include <vector>

//======================================================================

namespace
{
	//----------------------------------------------------------------------
	
	UIFloatPoint clipPoint (const UIRect & clip, const UIFloatPoint & pt)
	{
		UIFloatPoint clippedPt (pt);
		clippedPt.x = std::max (static_cast<float>(clip.left),   clippedPt.x);
		clippedPt.x = std::min (static_cast<float>(clip.right),  clippedPt.x);
		clippedPt.y = std::max (static_cast<float>(clip.top),    clippedPt.y);
		clippedPt.y = std::min (static_cast<float>(clip.bottom), clippedPt.y);
		return clippedPt;
	}
	
	void clipPoint (const UIRect & clip, UIFloatPoint & pt)
	{
		pt.x = std::max (static_cast<float>(clip.left),   pt.x);
		pt.x = std::min (static_cast<float>(clip.right),  pt.x);
		pt.y = std::max (static_cast<float>(clip.top),    pt.y);
		pt.y = std::min (static_cast<float>(clip.bottom), pt.y);
	}
	
	/*
	* define the coding of end points
	*/
	enum code {TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8};
	
	/*
	* compute the code of a point relative to a rectangle
	*/
	inline uint32 ComputeCode (float x, float y, const UIRect & r)
	{
		uint32 c = 0;
		if (y > r.bottom)
			c |= BOTTOM;
		else if (y < r.top)
			c |= TOP;
		if (x > r.right)
			c |= RIGHT;
		else if (x < r.left)
			c |= LEFT;
		return c;
	}
	
	/*
	*	clip line P0(x0,y0)-P1(x1,y1) against a rectangle
	*/
	inline bool CohenSutherlandClip (float & x0, float & y0, float & x1, float & y1, const UIRect & clip)
	{
		float x, y;
		
		uint32 C0 = ComputeCode (x0, y0, clip);
		uint32 C1 = ComputeCode (x1, y1, clip);
		
		for (;;) {
			/* trivial accept: both ends in rectangle */
			if ((C0 | C1) == 0) {
				//				MidPointLineReal (x0, y0, x1, y1);
				return true;
			}
			
			/* trivial reject: both ends on the external side of the rectangle */
			if ((C0 & C1) != 0)
				return false;
			
			/* normal case: clip end outside rectangle */
			const uint32 C = C0 ? C0 : C1;
			if (C & BOTTOM)
			{
				x = x0 + (x1 - x0) * (clip.bottom - y0) / (y1 - y0);
				y = static_cast<float>(clip.bottom);
			}
			else if (C & TOP)
			{
				x = x0 + (x1 - x0) * (clip.top - y0) / (y1 - y0);
				y = static_cast<float>(clip.top);
			}
			else if (C & RIGHT)
			{
				x = static_cast<float>(clip.right);
				y = y0 + (y1 - y0) * (clip.right - x0) / (x1 - x0);
			}
			else
			{
				x = static_cast<float>(clip.left);
				y = y0 + (y1 - y0) * (clip.left - x0) / (x1 - x0);
			}
			
			/* set new end point and iterate */
			if (C == C0)
			{
				x0 = x;
				y0 = y;
				C0 = ComputeCode (x0, y0, clip);
			}
			else
			{
				x1 = x;
				y1 = y;
				C1 = ComputeCode (x1, y1, clip);
			}
		}
		
		return false;
		/* notreached */
	}
	
	//--  Cohen-Sutherland algorithm
	
	inline bool clipLine (const UIRect & clip, UILine & line)
	{
		return CohenSutherlandClip (line.p1.x, line.p1.y, line.p2.x, line.p2.y, clip);
	}

	typedef std::vector<UILine> LineVector;
	LineVector s_lineVector;
	LineVector s_uvVector;

	typedef std::vector<VectorArgb> ArgbVector;
	ArgbVector s_argbVector;
}
	
//-------------------------------------------------------------------
//
// EngineCanvas
//
// Verts are passed in in this order:
// 0-----1
// |    /|
// |   / |
// |  /  |
// | /   |
// |/    |
// 2-----3
//-------------------------------------------------------------------

void	CuiLayer::EngineCanvas::RenderQuad( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4] )
{
	UNREF (src);
	UNREF (Vertices);
	UNREF (UVs);

	const VectorArgb argb (PackedArgb(mState.Color.a, mState.Color.r, mState.Color.g, mState.Color.b));

	const Shader * shader = 0;

	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src); //lint !e1774
		shader = NON_NULL (tcanvas->getShader ());
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}

	if (shader == 0)
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();

	if (IsDeforming())
	{
		UIFloatPoint NewVertices[4];
		Deform(Vertices, NewVertices, 4);
		CuiLayerRenderer::render (shader, NewVertices, UVs, argb);
	}
	else
	{
		CuiLayerRenderer::render (shader, Vertices, UVs, argb);
	}

	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}
//----------------------------------------------------------------------

void	CuiLayer::EngineCanvas::RenderQuad( const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4], const UIColor Colors[4] )
{
	UNREF (src);
	UNREF (Vertices);
	UNREF (UVs);
	
	const Shader * shader = 0;
	
	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src); //lint !e1774
		shader = NON_NULL (tcanvas->getShader ());
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}
	
	if (shader == 0)
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();

	if (IsDeforming())
	{
		UIFloatPoint NewVertices[4];
		Deform(Vertices, NewVertices, 4);
		CuiLayerRenderer::render (shader, NewVertices, UVs, Colors);
	}
	else
	{
		CuiLayerRenderer::render (shader, Vertices, UVs, Colors);
	}
	
	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}

//----------------------------------------------------------------------

void CuiLayer::EngineCanvas::RenderLines        (const UICanvas * const src, int numLines, const UILine * lines, const UILine * uvs)
{
	const StaticShader * shader = 0;
	
	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src);
		shader = dynamic_cast<const StaticShader *>(NON_NULL (tcanvas->getShader ()));
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}

	if (!shader)
	{
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();
		CuiLayerRenderer::flushRenderQueue ();
	}

	const PackedArgb dwVertexColor (CuiUtils::convertToVectorArgb (mState.Color));

	const VectorArgb argb  (PackedArgb(mState.Color.a, mState.Color.r, mState.Color.g, mState.Color.b));
	const float pixOffset = CuiManager::getPixelOffset ();
	const UIFloatPoint translate (static_cast<float>(GetTranslation ().x) + pixOffset, static_cast<float>(GetTranslation ().y) + pixOffset);

	s_lineVector.clear   ();
	s_uvVector.clear     ();
	s_lineVector.reserve (numLines);
	s_uvVector.reserve   (numLines);

	for (int i = 0; i < numLines; ++i)
	{
		const UILine & line = lines [i];
		UILine oldTransformed (line.p1 + translate, line.p2 + translate);
		UILine transformed (oldTransformed);
		if (!clipLine (mState.ClippingRect, transformed))
			continue;

		UILine transformedUv;
		
		if (uvs)
		{
			const UILine & uv   = uvs   [i];			
			
			if (src)
			{
				transformedUv.p2.x =   (transformed.p2.x - oldTransformed.p1.x) * (uv.p2.x - uv.p1.x) / (oldTransformed.p2.x - oldTransformed.p1.x) + uv.p1.x;
				transformedUv.p1.x = - (oldTransformed.p2.x - transformed.p1.x) * (uv.p2.x - uv.p1.x) / (oldTransformed.p2.x - oldTransformed.p1.x) + uv.p2.x;
				
				transformedUv.p2.y =   (transformed.p2.y - oldTransformed.p1.y) * (uv.p2.y - uv.p1.y) / (oldTransformed.p2.y - oldTransformed.p1.y) + uv.p1.y;
				transformedUv.p1.y = - (oldTransformed.p2.y - transformed.p1.y) * (uv.p2.y - uv.p1.y) / (oldTransformed.p2.y - oldTransformed.p1.y) + uv.p2.y;
			}
		}

		s_lineVector.push_back (transformed);
		s_uvVector.push_back   (transformedUv);
	}

	if (!s_lineVector.empty ())
	{
		CuiLayerRenderer::renderLines (shader, static_cast<int>(s_lineVector.size ()), &s_lineVector.front (), &s_uvVector.front (), argb);
	}

	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}

//----------------------------------------------------------------------

void CuiLayer::EngineCanvas::RenderLines(const UICanvas * const src, int numLines, const UILine * lines, const UIFloatPoint * uvs, const UIColor * colors)
{
	const StaticShader * shader = 0;
	
	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src);
		shader = dynamic_cast<const StaticShader *>(NON_NULL (tcanvas->getShader ()));
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}
	
	if (!shader)
	{
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();
		CuiLayerRenderer::flushRenderQueue ();
	}

	if (numLines)
	{
		// Lines
		s_lineVector.clear();
		s_lineVector.reserve(numLines);
		for (int lineCount = 0; lineCount < numLines; ++lineCount)
		{
			s_lineVector.push_back(lines[lineCount]);
		}

		// UVs...
		s_uvVector.clear();
		s_uvVector.reserve(numLines);

		if (uvs)
		{
			for (int uvCount = 0; uvCount < (numLines * 2); uvCount += 2)
			{
				s_uvVector.push_back(UILine(uvs[uvCount], uvs[uvCount + 1]));
			}
		}
		else
		{
			UILine dummy;
			s_uvVector.insert(s_uvVector.begin(), numLines, dummy);
		}

		// Colors...
		s_argbVector.clear();
		s_argbVector.reserve(numLines * 2);
		
		if (colors)
		{
			for (int colorCount = 0; colorCount < (numLines * 2); ++colorCount)
			{
				UIColor const & color = colors[colorCount];
				s_argbVector.push_back(PackedArgb(color.a, color.r, color.g, color.b));
			}
		}
		else
		{
			UIColor const & color = UIColor::white;
			s_argbVector.insert(s_argbVector.begin(), numLines, VectorArgb(PackedArgb(color.a, color.r, color.g, color.b)));
		}

		if (!s_lineVector.empty ())
			CuiLayerRenderer::renderLines(shader, static_cast<int>(s_lineVector.size()), &s_lineVector.front(), &s_uvVector.front(), &s_argbVector.front());
	}

	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}

//-----------------------------------------------------------------

void CuiLayer::EngineCanvas::RenderLineStrip(const UICanvas * const src, int pointCount, const UIFloatPoint *points )
{
	const StaticShader * shader = 0;
	
	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src);
		shader = dynamic_cast<const StaticShader *>(NON_NULL (tcanvas->getShader ()));
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}

	if (!shader)
	{
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();
		CuiLayerRenderer::flushRenderQueue ();
	}

	const uint32 num = static_cast<uint32>(pointCount);
	const float pixOffset = CuiManager::getPixelOffset ();
	const UIFloatPoint translate (static_cast<float>(GetTranslation ().x) + pixOffset, static_cast<float>(GetTranslation ().y) + pixOffset);
	const PackedArgb dwVertexColor( mState.Color.a, mState.Color.r, mState.Color.g, mState.Color.b );

	//-- build vertexarray
	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setColor0();
	DynamicVertexBuffer vb (format);

	vb.lock (pointCount);
	{
		VertexBufferWriteIterator v = vb.begin();
		for (uint32 i = 0; i < num; ++i)
		{
			UIFloatPoint pt (points [i].x + translate.x, points [i].y + translate.y);
			clipPoint (mState.ClippingRect, pt);
			v.setPosition (pt.x, pt.y, 1.0f);
			v.setOoz (1.0f);
			v.setColor0 (dwVertexColor);
			++v;
		}
	}
	vb.unlock ();

	Graphics::setStaticShader (*shader);
	Graphics::setVertexBuffer (vb);
	Graphics::drawLineStrip ();

	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}

//----------------------------------------------------------------------

void CuiLayer::EngineCanvas::RenderTriangles  (const UICanvas * const src, int TriangleCount, const UITriangle * tris, const UITriangle * uvs)
{
	const VectorArgb argb (PackedArgb(mState.Color.a, mState.Color.r, mState.Color.g, mState.Color.b));

	const Shader * shader = 0;

	if (src)
	{
		const CuiLayer::TextureCanvas * tcanvas = static_cast<const CuiLayer::TextureCanvas *> (src); //lint !e1774
		shader = NON_NULL (tcanvas->getShader ());
	}

	UICanvasState const & currentState = GetCurrentState();
	if (currentState.DepthOverride) 
	{
		setZ(currentState.mDepth);
	}

	if (shader == 0)
		shader = &ShaderTemplateList::get2dVertexColorAStaticShader ();

	CuiLayerRenderer::renderTriangles (shader, TriangleCount, tris, uvs, argb);

	if (currentState.DepthOverride) 
	{
		setZ(1.0f);
	}
}

//-----------------------------------------------------------------

void CuiLayer::EngineCanvas::setDepthTestEnabled (bool b)
{
	m_depthTest = b;

	if (!m_depthTest)
		setZ(1.0f);
}

//-----------------------------------------------------------------

void CuiLayer::EngineCanvas::setZ(float z)
{
	if (m_z != z)
	{
		CuiLayerRenderer::flushRenderQueue();

		m_z = z;

		if (m_z == 0)
			m_ooz = 0.0f;
		else
			m_ooz = 1.0f / m_z;

		CuiLayerRenderer::setZ(m_z, m_ooz);
	}
}
//======================================================================
