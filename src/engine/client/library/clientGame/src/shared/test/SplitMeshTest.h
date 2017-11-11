//===================================================================
//
// SplitMeshTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SplitMeshTest_H
#define INCLUDED_SplitMeshTest_H

//===================================================================

#include "clientGame/TestIoWin.h"

//===================================================================

template <typename T> class Watcher;

//----------------------------------------------------------------------

class SplitMeshTest : public TestIoWin
{
public:

	typedef Watcher<Object> ObjectWatcher;

	SplitMeshTest ();
	virtual ~SplitMeshTest ();

	IoResult processEvent (IoEvent* event);
	virtual void draw () const;

private:

	virtual Object* createObjectAndAppearance ();

	float m_objectRadius;

private:

	SplitMeshTest (const SplitMeshTest&);
	SplitMeshTest& operator= (const SplitMeshTest&);

	ObjectWatcher * m_objectBack;
};

//===================================================================

#endif
