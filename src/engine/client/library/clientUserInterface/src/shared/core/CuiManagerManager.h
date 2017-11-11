//======================================================================
//
// CuiManagerManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiManagerManager_H
#define INCLUDED_CuiManagerManager_H

//======================================================================

class UIPage;

//----------------------------------------------------------------------

/**
* Let the Cui*Manager management begin!  Yeeee hawwww!!!
*/

class CuiManagerManager
{
public:
	static void install (UIPage & rootPage);
	static void remove  ();
	static void update  (float deltaTimeSecs);
};

//======================================================================

#endif
