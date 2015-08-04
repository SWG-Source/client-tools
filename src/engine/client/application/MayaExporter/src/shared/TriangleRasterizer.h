#ifndef INCLUDED_TriangleRasterizer_H
#define INCLUDED_TriangleRasterizer_H

// ======================================================================
//
// TriangleRasterizer.h
// Copyright 2005 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/Vector2d.h"
#include "sharedMath/Vector.h"

class TriangleRasterizer
{
public:

	//-----------------------------
	// Rasterization modes.
	enum FillMode {
		 MODE_POLY   // In this mode, all samples that lie inside the triangle are returned. (pixel-aligned with (.5,.5) offset_
		,MODE_EDGE   // In this mode, the edges are scanned in y like MODE_POLY but the actual left and right x values are used.
	};
	//-----------------------------

	//-----------------------------
	struct Vertex
	{
		float x() const { return coords.x; }
		float y() const { return coords.y; }

		Vector2d coords;  // coordinates used to scan convert.

		// interpolated coordinates.
		Vector   normal;
		Vector   dot3;
	};
	//-----------------------------

	TriangleRasterizer(int i_imageWidth, int i_imageHeight);
	~TriangleRasterizer();

	void     setFillMode(FillMode m)             { fillMode=m; }
	FillMode getFillMode()                 const { return fillMode; }

	//-------------------------------------------------------------
	// Iteration should be done as follows:
	//
	// TriangleRasterizer t(width, height);
	// TriangleRasterizer::Vertex verts[3];
	//
	// ...fill in verts here...
	// t.begin(verts);
	// while (t.advance())
	// {
	//    const Vertex &sample = t.sample;
	//    ...process pixel here...
	// }
	//
	// This can be repeated indefinately on the same
	// TriangleRasterizer object as long as the image
	// dimensions don't change.
	void begin(const Vertex triangle[3]);

	bool advance();

	const Vertex &sample(int &o_pixelX, int &o_pixelY) const 
	{ 
		o_pixelX = static_cast<int>(floor(m_scanLine.m_i.coords.x));
		o_pixelY = static_cast<int>(floor(m_scanLine.m_i.coords.y));
		return m_scanLine.m_i; 
	}

	// for edge-scanning, this tells which side of the triangle the current
	// sample is from.
	bool isSampleLeft() const { return isLeft; }
	//-------------------------------------------------------------

protected:

	static float _roundMin(float x);
	static float _roundMax(float x);

	struct Edge
	{
		float x() const { return m_i.x(); }
		float y() const { return m_i.y(); }

		void interpolateX(float x);
		void interpolateY(float y);
		void _interpolate(float f);

		const Vertex *m_v0;
		const Vertex *m_v1;

		Vertex m_i;
	};

	void _startTopEdges();
	void _startBottomEdges();
	void _startScanLine();
	bool _advance();

	enum STATE {
		 START
		,NEWLINE
		,SCAN
		,EDGE_SCAN_LEFT
		,EDGE_SCAN_RIGHT
		,FINISHED
	} state;
	bool bottomHalf, isLeft;
	FillMode fillMode;
	int imageWidth, imageHeight;
	float x, y, middleY, bottomY;
	Vertex m_topVertex, m_leftMiddleVertex, m_rightMiddleVertex, m_bottomVertex;
	Edge m_leftEdge, m_rightEdge, m_scanLine;
};

#endif
