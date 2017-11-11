//
// Panorama.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_Panorama_H
#define INCLUDED_Panorama_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"

class GameCamera;
class Transform;

//-------------------------------------------------------------------

class Panorama : public IoWin
{
private:

	GameCamera* camera;

	int         state;

private:

	Panorama (const Panorama&);
	Panorama& operator= (const Panorama&);

public:

	explicit Panorama (const GameCamera* sourceCamera, bool align);
	virtual ~Panorama (void);

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw (void) const;
};

//-------------------------------------------------------------------

#endif
