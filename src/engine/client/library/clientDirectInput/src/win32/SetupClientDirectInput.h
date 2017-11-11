// ======================================================================
//
// SetupClientDirectInput.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupClientDirectInput_H
#define INCLUDED_SetupClientDirectInput_H

// ======================================================================

class SetupClientDirectInput
{
public:

	typedef bool (*IsWindowedMode)();

	static void install(HINSTANCE instanceHandle, HWND window, DWORD menuKey, IsWindowedMode isWindowedMode);

private:
	SetupClientDirectInput();
	SetupClientDirectInput(const SetupClientDirectInput &);
	SetupClientDirectInput &operator =(const SetupClientDirectInput &);
};

// ======================================================================

#endif
