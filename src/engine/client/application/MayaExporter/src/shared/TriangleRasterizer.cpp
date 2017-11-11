// ======================================================================
//
// TriangleRasterizer.cpp
// Copyright 2005 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "TriangleRasterizer.h"

// ======================================================================

void TriangleRasterizer::Edge::_interpolate(float f)
{
	float omf = 1.0f - f;

	m_i.coords.x = m_v0->coords.x * omf + m_v1->coords.x * f;
	m_i.coords.y = m_v0->coords.y * omf + m_v1->coords.y * f;
	m_i.normal.x = m_v0->normal.x * omf + m_v1->normal.x * f;
	m_i.normal.y = m_v0->normal.y * omf + m_v1->normal.y * f;
	m_i.normal.z = m_v0->normal.z * omf + m_v1->normal.z * f;
	m_i.dot3.x   = m_v0->dot3.x   * omf + m_v1->dot3.x   * f;
	m_i.dot3.y   = m_v0->dot3.y   * omf + m_v1->dot3.y   * f;
	m_i.dot3.z   = m_v0->dot3.z   * omf + m_v1->dot3.z   * f;
}

void TriangleRasterizer::Edge::interpolateX(float x)
{
	float f = (x - m_v0->coords.x) / (m_v1->coords.x - m_v0->coords.x);
	_interpolate(f);
	m_i.coords.x = x;
}

void TriangleRasterizer::Edge::interpolateY(float y)
{
	float f = (y - m_v0->coords.y) / (m_v1->coords.y - m_v0->coords.y);
	_interpolate(f);
	m_i.coords.y = y;
}

// ======================================================================

inline
float TriangleRasterizer::_roundMin(float x) 
{ 
	return static_cast<float>( ceil(x + 0.5f) - 0.5); 
}

//-----------------------------------------------------------------------

inline
float TriangleRasterizer::_roundMax(float x) 
{ 
	return static_cast<float>(floor(x - 0.5f) + 0.5); 
}

//-----------------------------------------------------------------------

TriangleRasterizer::TriangleRasterizer(int i_imageWidth, int i_imageHeight)
:	isLeft(false),
	imageWidth(i_imageWidth), 
	imageHeight(i_imageHeight)
{
}

TriangleRasterizer::~TriangleRasterizer()
{
}

void TriangleRasterizer::begin(const Vertex triangle[3])
{
	float y0 =(float)triangle[0].coords.y;
	float y1 =(float)triangle[1].coords.y;
	float y2 =(float)triangle[2].coords.y;

	// sort vertices in 'y'
	int top, middle, bottom;
	if (y0<y1)
	{
		if (y2<y0) 
		{
			top    = 2; 
			middle = 0; 
			bottom = 1;
		} 
		else // y0<=y2
		{
			top = 0;
			if (y1<y2) 
			{
				middle = 1; 
				bottom = 2;
			}
			else // y2<=y1
			{
				middle = 2; 
				bottom = 1;
			}
		}
	}
	else // y1<=y0
	{
		if (y2<y1) 
		{
			top = 2;
			middle = 1;
			bottom = 0;
		} 
		else
		{
			top = 1;
			if (y0<y2) 
			{
				middle = 0;
				bottom = 2;
			} 
			else // y2<=y0
			{
				middle = 2;
				bottom = 0;
			}
		}
	}

	float topY    = triangle[top].coords.y;

	bottomY = triangle[bottom].coords.y;

	y = _roundMin(topY);
	if (y>_roundMax(bottomY))
	{
		state=FINISHED;
		return;
	}

	m_topVertex = triangle[top];
	m_bottomVertex = triangle[bottom];

	Edge e;
	middleY = triangle[middle].coords.y;

	e.m_v0 = &m_topVertex;
	e.m_v1 = &m_bottomVertex;
	e.interpolateY(middleY);

	float middleX1 = triangle[middle].coords.x;
	float middleX2 = e.m_i.coords.x;
	if (middleX1<=middleX2)
	{
		m_leftMiddleVertex  = triangle[middle];
		m_rightMiddleVertex = e.m_i;
	}
	else // middleX2 < middleX1
	{
		m_leftMiddleVertex  = e.m_i;
		m_rightMiddleVertex = triangle[middle];
	}

	m_scanLine.m_v0=&m_leftEdge.m_i;
	m_scanLine.m_v1=&m_rightEdge.m_i;

	bottomHalf=false;
	state=START;
}

bool TriangleRasterizer::advance()
{
	while (_advance());
	return state!=FINISHED;
}

void TriangleRasterizer::_startTopEdges()
{
	state=NEWLINE;
	bottomHalf=false;

	m_leftEdge.m_v0  = &m_topVertex;
	m_rightEdge.m_v0 = &m_topVertex;
	m_leftEdge.m_v1  = &m_leftMiddleVertex;
	m_rightEdge.m_v1 = &m_rightMiddleVertex;
}

void TriangleRasterizer::_startBottomEdges()
{
	state=NEWLINE;
	bottomHalf=true;

	m_leftEdge.m_v0  = &m_leftMiddleVertex;
	m_rightEdge.m_v0 = &m_rightMiddleVertex;
	m_leftEdge.m_v1  = &m_bottomVertex;
	m_rightEdge.m_v1 = &m_bottomVertex;
}

void TriangleRasterizer::_startScanLine()
{
	m_leftEdge.interpolateY(y);
	m_rightEdge.interpolateY(y);
	float leftX = m_leftEdge.x();
	switch (fillMode)
	{
	case MODE_POLY:
	{
		if (leftX<0)
		{
			leftX=0;
		}
		x = _roundMin(leftX);
		state=SCAN;
	} break;

	case MODE_EDGE:
	{
		x=leftX;
		state=EDGE_SCAN_LEFT;
	} break;
	}
}

bool TriangleRasterizer::_advance()
{
	switch (state)
	{
	//-------------------------------
	case START:
		if (bottomHalf)
		{
			_startBottomEdges();
		}
		else
		{
			_startTopEdges();
		}

		return true; // call again.
	//-------------------------------

	//-------------------------------
	case NEWLINE:
		if (bottomHalf)
		{
			if (y>bottomY || y>=float(imageHeight))
			{
				state=FINISHED;
				return false;
			}
		}
		else if (y>middleY)
		{
			_startBottomEdges();
			return true;
		}

		if (y<0)
		{
			y+=1.0f;
			return true;
		}
		else
		{
			_startScanLine();
			return true; // call again.
		}
	//-------------------------------

	//-------------------------------
	case SCAN:
		if (x>float(imageWidth) || x>m_rightEdge.x())
		{
			y+=1.0f;
			state=NEWLINE;
			return true;
		}
		else
		{
			m_scanLine.interpolateX(x);
			x+=1.0f;
			return false; // new valid sample.
		}
	//-------------------------------

	//-------------------------------
	case EDGE_SCAN_LEFT: {
		bool skipped=x<0;
		x=m_rightEdge.x();
		state=EDGE_SCAN_RIGHT;
		if (skipped)
		{
			return true;
		}
		else
		{
			m_scanLine.m_i=m_leftEdge.m_i;
			isLeft=true;
			return false;
		}
		return false;
	}
	//-------------------------------

	//-------------------------------
	case EDGE_SCAN_RIGHT: {
		y+=1.0f;
		state=NEWLINE;
		if (x>float(imageWidth))
		{
			return true;
		}
		else
		{
			m_scanLine.m_i=m_rightEdge.m_i;
			isLeft=false;
			return false;
		}
	}
	//-------------------------------

	default: 
		state=FINISHED;
		return false;
	}
}
