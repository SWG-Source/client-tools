//======================================================================
//
// SwgCuiOptGraphics.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptGraphics_H
#define INCLUDED_SwgCuiOptGraphics_H

#include "swgClientUserInterface/SwgCuiOptBase.h"

class UISliderbar;

//======================================================================

class SwgCuiOptGraphics : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptGraphics (UIPage & page);
	~SwgCuiOptGraphics () {}

	void                       performActivate ();

private:

	SwgCuiOptGraphics & operator=(const SwgCuiOptGraphics & rhs);
	SwgCuiOptGraphics            (const SwgCuiOptGraphics & rhs);

	UISliderbar * m_sliderBrightness;
	UISliderbar * m_sliderContrast;
	UISliderbar * m_sliderGamma;
};

//======================================================================

#endif
