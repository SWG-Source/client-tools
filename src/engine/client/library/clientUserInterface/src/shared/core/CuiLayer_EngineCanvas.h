//======================================================================
//
// CuiLayer_EngineCanvas.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLayer_EngineCanvas_H
#define INCLUDED_CuiLayer_EngineCanvas_H

#include "clientUserInterface/CuiLayer.h"
#include "UICanvas.h"

//======================================================================

class CuiLayer::EngineCanvas : public UICanvas
{
public:
	explicit                EngineCanvas         (const UISize & s) : UICanvas(s), m_z (CONST_REAL (1.0)), m_ooz (CONST_REAL (1.0)), m_depthTest (false) {}
	
	UIBaseObject *          Clone                () const { return 0; };
	
	void                    RenderQuad           (const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4] );
	void                    RenderQuad           (const UICanvas * const src, const UIFloatPoint Vertices[4], const UIFloatPoint UVs[4], const UIColor Colors[4]);
	
	void                    RenderLineStrip      (const UICanvas * const src, int PointCount, const UIFloatPoint *Points );
	void                    RenderLines          (const UICanvas * const src, int PointCount, const UILine * lines, const UILine * uvs);
	void                    RenderLines          (const UICanvas * const src, int numLines, const UILine * lines, const UIFloatPoint * uvs, const UIColor * colors);

	void                    RenderTriangles      (const UICanvas * const src, int TriangleCount, const UITriangle * tris, const UITriangle * uvs);

	void                    setDepthTestEnabled  (bool b);
	void                    setZ                 (float z);
	
private:
	                        EngineCanvas         (const EngineCanvas &);
	EngineCanvas &          operator=            (const EngineCanvas &);
	
	
	float                   m_z;
	float                   m_ooz;
	
	bool                    m_depthTest;
	
};

//======================================================================

#endif
