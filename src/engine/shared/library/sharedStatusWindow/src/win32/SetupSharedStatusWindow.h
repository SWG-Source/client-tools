// ======================================================================
//
// SetupSharedStatusWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedStatusWindow_H
#define INCLUDED_SetupSharedStatusWindow_H

// ======================================================================

class SetupSharedStatusWindow
{
public:

	static void install(HINSTANCE hinstance);

private:

	SetupSharedStatusWindow();
	SetupSharedStatusWindow(const SetupSharedStatusWindow &);
	SetupSharedStatusWindow &operator =(const SetupSharedStatusWindow &);
};

// ======================================================================

#endif
