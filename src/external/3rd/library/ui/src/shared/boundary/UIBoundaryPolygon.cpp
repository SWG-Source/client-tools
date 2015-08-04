// ======================================================================
//
// UIBoundaryPolygon.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIBoundaryPolygon.h"

#include <cassert>

// ======================================================================

UIBoundaryPolygon::PointData::PointData () :
pt ()
{}

//----------------------------------------------------------------------

UIBoundaryPolygon::PointData::PointData (const UIPoint & _pt, PackType _packH, PackType _packV) :
pt (_pt)
{
	pack [0].type = _packH;
	pack [1].type = _packV;
}

//----------------------------------------------------------------------

UIBoundaryPolygon::UIBoundaryPolygon (const std::string & name) :
UIBoundary (name),
mPoints (),
mExtentOuter (),
mExtentDirty (true)
{
}

//-----------------------------------------------------------------

bool UIBoundaryPolygon::hitTest (const UIPoint & pt) const
{
	if (mPoints.size () < 3)
		return true;

	if (mExtentDirty)
		calculateExtent ();

	if (pt.x < mExtentOuter.left && pt.x >= mExtentOuter.right && pt.y < mExtentOuter.top && pt.y >= mExtentOuter.bottom)
		return false;

	//-- test extent
//	if (pt.x >= mExtentInner.left && pt.x < mExtentInner.right && pt.y >= mExtentInner.top && pt.y < mExtentInner.bottom)
//		return true;

	bool in = false;

	//-- check to make sure the point is inside the shape
	size_t i;
	size_t j;
	const size_t n = mPoints.size ();
	for (i = 0, j = n - 1; i < n; j = i++) 
	{
		const UIPoint & pt1 = mPoints [i].pt;
		const UIPoint & pt2 = mPoints [j].pt;

		if (((pt1.y <= pt.y) && (pt.y < pt2.y)) || ((pt2.y <= pt.y) && (pt.y < pt1.y)))
		{
//			const float val = ((pt2.x - pt1.x) * (x - pt1.y)) / (pt2.y - pt1.y) + pt1.x;

			const long val = static_cast<long>(static_cast<float>((pt2.x - pt1.x) * (pt.x - pt1.y)) / static_cast<float>(pt2.y - pt1.y)) + pt1.x;
			if (pt.x < val)
				in = !in;
		}
	}

	return in;
}

//-----------------------------------------------------------------

void UIBoundaryPolygon::calculateExtent () const
{
	mExtentDirty = false;
	mExtentOuter.left  = mExtentOuter.top    =  1000000;
	mExtentOuter.right = mExtentOuter.bottom = -1000000;

	for (PointVector::const_iterator it = mPoints.begin (); it != mPoints.end (); ++it)	
	{
		const UIPoint & pt = (*it).pt;
		mExtentOuter.left = std::min (mExtentOuter.left, pt.x);
		mExtentOuter.top  = std::min (mExtentOuter.top, pt.y);

		mExtentOuter.right  = std::max (mExtentOuter.right, pt.x);
		mExtentOuter.bottom = std::max (mExtentOuter.bottom, pt.y);
	}
}

//-----------------------------------------------------------------

void UIBoundaryPolygon::insertPoint       (int index, const PointData & pd)
{
	PointVector::iterator it = mPoints.begin ();
	std::advance (it, index);
	mPoints.insert (it, pd);
}

//----------------------------------------------------------------------

namespace
{
	void computeFixed (long control, long currentControl, long & val)
	{
		val = currentControl - control + val;
	}
	
	//----------------------------------------------------------------------
	
	void updatePacking (const UIBoundaryPolygon::PackData & pdata, long prevSize, long curSize, long & val)
	{	
		switch (pdata.type)
		{
		case UIBoundaryPolygon::PT_fc:
			{
				computeFixed (prevSize / 2L, curSize / 2L, val);
			}
			break;
		case UIBoundaryPolygon::PT_ff:
			{
				computeFixed (prevSize, curSize, val);
			}
			break;
		case UIBoundaryPolygon::PT_pn:
		case UIBoundaryPolygon::PT_pc:
		case UIBoundaryPolygon::PT_pf:
			assert (false);
			break;
		}
	}
}

//----------------------------------------------------------------------

void UIBoundaryPolygon::onSizeChange (const UISize & prevSize, const UISize & curSize)
{
	for (PointVector::iterator it = mPoints.begin (); it != mPoints.end (); ++it)	
	{
		PointData & pd = (*it);
		
		updatePacking (pd.pack [0], prevSize.x, curSize.x, pd.pt.x);
		updatePacking (pd.pack [1], prevSize.y, curSize.y, pd.pt.y);
	}
	
	mExtentDirty = true;
}

//----------------------------------------------------------------------

void UIBoundaryPolygon::makeMarginBoundary (const UISize & size, const UIRect & margin)
{
	mPoints.clear ();
	const UIPoint & loc = margin.Location ();

	mExtentOuter.left   = loc.x;
	mExtentOuter.top    = loc.y;
	mExtentOuter.right  = size.x - margin.right;
	mExtentOuter.bottom = size.y - margin.bottom;

	PointData pd;

	pd.pt = loc;
	mPoints.push_back (pd);

	pd.pt.y = mExtentOuter.bottom;
	pd.pack [1].type = PT_ff;
	mPoints.push_back (pd);

	pd.pack [0].type = PT_ff;
	pd.pt.x = mExtentOuter.right;
	mPoints.push_back (pd);

	pd.pt.y = mExtentOuter.top;
	pd.pack [1].type = PT_fn;
	mPoints.push_back (pd);
}

// ======================================================================
