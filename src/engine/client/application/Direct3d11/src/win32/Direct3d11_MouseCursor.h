// ======================================================================
//
// Direct3d11_MouseCursor.h
// copyright (c) 2026 Galaxies Reborn
//
// A hardware mouse cursor built from an engine texture.
//
// ----------------------------------------------------------------------
// Why this is Win32 and not D3D
//
// D3D9 had SetCursorProperties: hand the device a surface and it drove the cursor itself. DXGI
// has no equivalent, and that is not an omission -- the cursor was moved back to where it
// belongs, which is the window system. So the D3D path here is only the readback; the cursor
// itself is an HCURSOR made with CreateIconIndirect and installed with SetCursor.
//
// It is worth having rather than falling back. A hardware cursor is composited by the desktop
// at the pointer's real position, so it stays responsive at any frame rate; a cursor drawn as
// geometry inside the scene lags by exactly one frame and visibly stutters when the frame rate
// drops. That difference is most obvious in precisely the situations where it matters.
//
// ----------------------------------------------------------------------
// The fallback is real, not an error path
//
// setMouseCursor returns a bool and the caller checks it: CuiLayer_CursorInterface draws the
// cursor as part of the interface when this returns false. So failing is a supported outcome,
// and every failure here -- an unreadable texture, an unexpected format, a window that is not
// in the foreground -- returns false and lets the interface handle it. DX9 does the same, and
// refuses outright when its window is not the foreground one.
//
// ----------------------------------------------------------------------
// The readback, and why it is not a problem
//
// Building the cursor means getting the texture's pixels to the CPU, which is a blocking
// staging map -- the thing Direct3d11_Metrics counts and requires to be zero inside a frame.
// It is bounded: cursors are a small fixed set, each is built once and cached by texture and
// hot spot, and the engine sets the same handful over and over. So the count rises during
// warm-up and then stops, which is the same shape as the state object and input layout
// creations the metrics already tolerate at startup.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_MouseCursor_H
#define INCLUDED_Direct3d11_MouseCursor_H

// ======================================================================

class Texture;

// ======================================================================

class Direct3d11_MouseCursor
{
public:
	static void remove();

	// Build if necessary, then install. False means the interface should draw its own.
	static bool set(Texture const &texture, int hotSpotX, int hotSpotY);

private:
	Direct3d11_MouseCursor();
	Direct3d11_MouseCursor(Direct3d11_MouseCursor const &);
	Direct3d11_MouseCursor &operator=(Direct3d11_MouseCursor const &);
};

// ======================================================================

#endif
