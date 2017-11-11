// ======================================================================
//
// UIWidgetBoundaries.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIWidgetBoundaries_H
#define INCLUDED_UIWidgetBoundaries_H

// ======================================================================

class UIBoundary;
class UIWidget;
class UiMemoryBlockManager;

#include "UILowerString.h"
#include "UITypes.h"
#include "UiMemoryBlockManagerMacros.h"

#include <map>

//-----------------------------------------------------------------

class UIWidgetBoundaries
{
	UI_MEMORY_BLOCK_MANAGER_INTERFACE;

public:
	// RLS TODO: Consider using a hash_map?
	typedef std::map<UILowerString /*name*/, UIBoundary *> BoundaryMap;

	                       UIWidgetBoundaries  (UIWidget & widget);
	                      ~UIWidgetBoundaries ();

	void                   addBoundary    (UIBoundary & boundary);
	void                   removeBoundary (UIBoundary & boundary);

	const BoundaryMap & getBoundaries () const;

	bool                   empty ();

	bool                   hitTest        (const UIPoint & pt) const;

	void                   onSizeChange   (const UISize & prevSize, const UISize & curSize);

	UIBoundary *           findBoundary   (const std::string & name);
	const UIBoundary *     findBoundary   (const std::string & name) const;

private:
	                       UIWidgetBoundaries ();
	                       UIWidgetBoundaries (const UIWidgetBoundaries & rhs);
	UIWidgetBoundaries &   operator= (const UIWidgetBoundaries & rhs);

	BoundaryMap         mBoundaries;

	UIWidget &             mWidget;
};

//-----------------------------------------------------------------

inline const UIWidgetBoundaries::BoundaryMap & UIWidgetBoundaries::getBoundaries () const
{
	return mBoundaries;
}

// ======================================================================

#endif
