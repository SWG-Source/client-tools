//======================================================================
//
// SwgCuiLocationDisplay.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiLocationDisplay_H
#define INCLUDED_SwgCuiLocationDisplay_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "StringId.h"

class UIImage;
class UIText;
class Vector;
class UIButton;

//----------------------------------------------------------------------

class SwgCuiLocationDisplay : 
public SwgCuiLockableMediator 
{
public:

	explicit            SwgCuiLocationDisplay  (UIPage & page);

	void                update             (float deltaTimeSecs);

protected:
	virtual void        performActivate    ();
	virtual void        performDeactivate  ();

private:
	virtual            ~SwgCuiLocationDisplay  ();
	SwgCuiLocationDisplay ();
	SwgCuiLocationDisplay (const SwgCuiLocationDisplay &);
	SwgCuiLocationDisplay & operator= (const SwgCuiLocationDisplay &);

	UIText *                     m_textLat;
	UIText *                     m_textHeight;
	UIText *                     m_textLong;

	UIPoint                      m_lastCoord;
	
	StringId                     m_lastRegion;
};

//======================================================================

#endif
