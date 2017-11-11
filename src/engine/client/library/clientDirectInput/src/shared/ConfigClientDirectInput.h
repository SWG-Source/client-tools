// ======================================================================
//
// ConfigClientDirectInput.h
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConfigClientDirectInput_H
#define INCLUDED_ConfigClientDirectInput_H

// ======================================================================

class ConfigClientDirectInput
{
public:

	static void         install();
	static bool         getUseJoysticks();
	static int          getUseJoystick();
	static bool         getUseMouse();
	static bool         getWindowsKeyEnabledWhenFullscreen();
	static bool         getWindowsKeyEnabledWhenWindowed();
	static char const * getKeyboardTranslationTable();
	static bool         getEnableForceFeedback();
};

// ======================================================================

#endif
