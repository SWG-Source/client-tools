//======================================================================
//
// UIMessageModifierData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIMessageModifierData_H
#define INCLUDED_UIMessageModifierData_H

//======================================================================

class UIMessageModifierData
{
public:

	bool LeftShift;
	bool RightShift;
	bool LeftControl;
	bool RightControl;
	bool LeftAlt;
	bool RightAlt;
	
	bool LeftMouseDown;
	bool MiddleMouseDown;
	bool RightMouseDown;
	
	UIMessageModifierData () :
		LeftShift       (false),
		RightShift      (false),
		LeftControl     (false),
		RightControl    (false),
		LeftAlt         (false),
		RightAlt        (false),
		LeftMouseDown   (false),
		MiddleMouseDown (false),
		RightMouseDown  (false)
	{}

	// @todo: this modifierdata should probably be a bitvector
	bool and (const UIMessageModifierData & rhs) const
	{
		if ((rhs.LeftShift       && ! LeftShift) ||
			(rhs.RightShift      && ! RightShift) ||
			(rhs.LeftControl     && ! LeftControl) ||
			(rhs.RightControl    && ! RightControl) ||
			(rhs.LeftAlt         && ! LeftAlt) ||
			(rhs.RightAlt        && ! RightAlt) ||
			(rhs.LeftMouseDown   && ! LeftMouseDown) ||
			(rhs.MiddleMouseDown && ! MiddleMouseDown) ||
			(rhs.RightMouseDown  && ! RightMouseDown))
			return false;

		return true;
	}

	bool isShiftDown   () const { return LeftShift   || RightShift; }
	bool isControlDown () const { return LeftControl || RightControl; }
	bool isAltDown     () const { return LeftAlt     || RightAlt; }

};

//======================================================================

#endif
