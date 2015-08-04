//======================================================================
//
// UITextStyleWrappedText.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UITextStyleWrappedText_H
#define INCLUDED_UITextStyleWrappedText_H

//======================================================================

#include <vector>

typedef std::vector<long>                           LongVector;
typedef std::vector<Unicode::String::const_iterator>       UIStringConstIteratorVector;

class UITextStyleWrappedText
{
public:

	UIStringConstIteratorVector linePointers;
	LongVector                  lineWidths;

	//----------------------------------------------------------------------
	
	void clear ()
	{
		linePointers.clear ();
		lineWidths.clear   ();
	}
	
	//----------------------------------------------------------------------
	
	int size ()
	{
		return static_cast<int>(linePointers.size ());
	}
};

//======================================================================

#endif
