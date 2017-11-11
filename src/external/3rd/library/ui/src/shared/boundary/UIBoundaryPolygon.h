// ======================================================================
//
// UIBoundaryPolygon.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIBoundaryPolygon_H
#define INCLUDED_UIBoundaryPolygon_H

// ======================================================================

#include "UIBoundary.h"
#include "UITypes.h"

#include <vector>

//-----------------------------------------------------------------

class UIBoundaryPolygon : 
public UIBoundary
{
public:

	enum PackType
	{
		PT_fn,
		PT_fc,
		PT_ff,
		PT_pn,
		PT_pc,
		PT_pf
	};

	class Rational
	{
	public:
		int n;
		int d;

		Rational () : n (0), d (1) {}
	};

	class PackData
	{
	public:
		PackType type;
		Rational prop;

		PackData () : type (PT_fn), prop () {}
	};

	class PointData
	{
	public:
		UIPoint       pt;
		PackData      pack  [2];

		PointData ();
		explicit PointData (const UIPoint & _pt, PackType _packH = PT_fn, PackType _packV = PT_fn);
	};


	typedef std::vector<PointData>    PointVector;

	                    UIBoundaryPolygon (const std::string & name);

	bool                hitTest           (const UIPoint & pt) const;
	void                onSizeChange      (const UISize & prevSize, const UISize & curSize);

	const PointVector & getPoints         () const;

	void                insertPoint        (int index, const PointData & pd);

	void                makeMarginBoundary (const UISize & size, const UIRect & margin);

private:
	UIBoundaryPolygon (const UIBoundaryPolygon & rhs);
	UIBoundaryPolygon & operator= (const UIBoundaryPolygon & rhs);

	void                         calculateExtent () const;

	PointVector                  mPoints;

	mutable UIRect               mExtentOuter;
//	mutable UIRect               mExtentInner;
	mutable bool                 mExtentDirty;

	UISize                       mSize;

};

// ======================================================================

#endif
