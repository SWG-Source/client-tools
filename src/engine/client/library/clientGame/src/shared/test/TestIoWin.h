//===================================================================
//
// TestIoWin.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TestIoWin_H
#define INCLUDED_TestIoWin_H

//===================================================================

#include "sharedIoWin/IoWin.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"

class Light;
class MouseCursor;
class Object;
class ObjectList;
class ObjectListCamera;

//===================================================================

class TestIoWin : public IoWin
{
public:

	TestIoWin (const char* const name=0);
	virtual ~TestIoWin ();

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw () const;

protected:

	enum Keys
	{
		K_up,
		K_down,
		K_left,
		K_right,
		K_insert,
		K_delete,
		K_home,
		K_end,
		K_pageUp,
		K_pageDown,
		K_f1,
		K_f2,
		K_f3,
		K_f4,
		K_alt,
		K_shift,

		K_count
	};

protected:

	bool              m_initialized;
	bool              m_paused;
	MouseCursor*      m_mouseCursor;
	ObjectListCamera* m_camera;
	ObjectList*       m_lightList;
	ObjectList*       m_objectList;
	float             m_cameraYaw;
	float             m_cameraPitch;
	float             m_cameraZoom;
	Vector            m_cameraPivot;
	float             m_lightYaw;
	float             m_lightPitch;
	float             m_lightZoom;
	Vector            m_lightPivot;
	Object*           m_object;
	Light*            m_light;
	bool              m_button0;
	bool              m_button1;
	bool              m_button2;
	bool              m_shouldUpdate;
	bool              m_keys [K_count];

private:

	bool              m_first;
	bool              m_wire;

protected:

	bool            isInitialized () const;
	virtual void    initialize ();
	virtual void    createLights ();
	virtual void    createFloor ();
	virtual Object* createObjectAndAppearance ();
	void            reload();

private:

	TestIoWin (const TestIoWin&);
	TestIoWin& operator= (const TestIoWin&);
};

//===================================================================

#endif
