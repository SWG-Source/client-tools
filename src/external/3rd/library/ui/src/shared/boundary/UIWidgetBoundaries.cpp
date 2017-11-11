// ======================================================================
//
// UIWidgetBoundaries.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIWidgetBoundaries.h"

#include "UIBoundary.h"
#include "UiMemoryBlockManager.h"
#include "UIWidget.h"

#include <algorithm>
#include <cassert>

// ======================================================================

UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(UIWidgetBoundaries, 1);

// ======================================================================

UIWidgetBoundaries::UIWidgetBoundaries (UIWidget & widget) :
mBoundaries (),
mWidget     (widget)
{

}

//-----------------------------------------------------------------

UIWidgetBoundaries::~UIWidgetBoundaries ()
{
	for (BoundaryMap::const_iterator it = mBoundaries.begin (); it != mBoundaries.end (); ++it)
		delete it->second;

	mBoundaries.clear ();
}

//-----------------------------------------------------------------

void UIWidgetBoundaries::addBoundary (UIBoundary & boundary)
{
	mBoundaries.insert(std::make_pair(boundary.getName(), &boundary));
}

//-----------------------------------------------------------------

bool UIWidgetBoundaries::hitTest (const UIPoint & pt) const
{
	for (BoundaryMap::const_iterator it = mBoundaries.begin (); it != mBoundaries.end (); ++it)
	{
		const UIBoundary * boundary = it->second;
		if (boundary->hitTest(pt))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

void UIWidgetBoundaries::onSizeChange(const UISize & prevSize, const UISize & curSize)
{
	for (BoundaryMap::iterator it = mBoundaries.begin (); it != mBoundaries.end (); ++it)
	{
		UIBoundary * boundary = it->second;
		boundary->onSizeChange (prevSize, curSize);
	}
}

//-----------------------------------------------------------------

void UIWidgetBoundaries::removeBoundary (UIBoundary & boundary)
{
	UILowerString keyName(boundary.getName());
	BoundaryMap::iterator itBoundary = mBoundaries.find(keyName);
	
	if (itBoundary != mBoundaries.end())
	{
		mBoundaries.erase(itBoundary);
	}
}

//-----------------------------------------------------------------

bool UIWidgetBoundaries::empty ()
{
	return mBoundaries.empty ();
}

//----------------------------------------------------------------------

UIBoundary * UIWidgetBoundaries::findBoundary(const std::string & name)
{
	UILowerString keyName(name);
	BoundaryMap::const_iterator const itBoundary = mBoundaries.find(keyName);

	if (itBoundary != mBoundaries.end())
	{
		return itBoundary->second;
	}

	return 0;
}

//----------------------------------------------------------------------

const UIBoundary * UIWidgetBoundaries::findBoundary   (const std::string & name) const
{
	return const_cast<UIWidgetBoundaries *>(this)->findBoundary (name);
}

// ======================================================================
