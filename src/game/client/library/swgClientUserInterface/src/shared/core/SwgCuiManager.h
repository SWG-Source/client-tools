// ======================================================================
//
// SwgCuiManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiManager_H
#define INCLUDED_SwgCuiManager_H

// ======================================================================

class SwgCuiManager
{
public:

	static void install ();
	static void remove  ();
	static bool test    (std::string & result);
	static void update  (float delta);

private:
	SwgCuiManager (const SwgCuiManager & rhs);
	SwgCuiManager & operator= (const SwgCuiManager & rhs);
};

// ======================================================================

#endif
