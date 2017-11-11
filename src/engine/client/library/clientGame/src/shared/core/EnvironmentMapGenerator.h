// ======================================================================
//
// EnvironmentMapGenerator.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_EnvironmentMapGenerator_H
#define INCLUDED_EnvironmentMapGenerator_H

// ======================================================================

#include "sharedIoWin/IoWin.h"

class GameCamera;
class Object;

// ======================================================================

class EnvironmentMapGenerator : public IoWin
{
public:

	EnvironmentMapGenerator (GameCamera const * sourceCamera, Object const * const excludedObject);
	virtual ~EnvironmentMapGenerator ();

	virtual IoResult processEvent (IoEvent * event);
	virtual void     draw () const;

private:

	GameCamera * const m_camera;
	int                m_state;

private:

	EnvironmentMapGenerator ();
	EnvironmentMapGenerator (EnvironmentMapGenerator const &);
	EnvironmentMapGenerator & operator= (EnvironmentMapGenerator const &);
};

// ======================================================================

#endif
