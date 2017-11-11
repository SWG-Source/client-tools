//======================================================================
//
// UIActionListener.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIActionListener_H
#define INCLUDED_UIActionListener_H

class UIBaseObject;

//======================================================================

class UIActionListener
{
public:
	virtual ~UIActionListener () = 0 {}
	virtual bool performActionForObject    (const UIBaseObject & obj) = 0;
	virtual bool constructTooltipForObject (const UIBaseObject & obj, Unicode::String & str) = 0;
};

//======================================================================

#endif
