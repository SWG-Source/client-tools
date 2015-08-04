//======================================================================
//
// UICursorInterface.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UICursorInterface_H
#define INCLUDED_UICursorInterface_H

//======================================================================

class UICursor;
class UICanvas;
struct UIPoint;

//----------------------------------------------------------------------

class UICursorInterface
{
public:
	UICursorInterface ();
	virtual ~UICursorInterface ();

	virtual void render    (unsigned long theTime, UICursor * cursor, UICanvas & destinationCanvas, const UIPoint & position) = 0;
};

//----------------------------------------------------------------------

class UICursorInterfaceDefault :
public UICursorInterface
{
public:
	UICursorInterfaceDefault ();
	~UICursorInterfaceDefault ();

	void setCursor (UICursor * cursor);
	void render    (unsigned long theTime, UICursor * cursor, UICanvas & destinationCanvas, const UIPoint & position);
private:

	UICursor * mCursor;
};

//======================================================================

#endif
