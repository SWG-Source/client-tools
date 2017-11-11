//
// Scene.h
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//

//-------------------------------------------------------------------

#ifndef SCENE_H
#define SCENE_H

//-------------------------------------------------------------------

#include "sharedIoWin/IoWin.h"

class GameCamera;

//-------------------------------------------------------------------

class Scene : public IoWin
{
protected:

	static int currentScreenShot;

private:

	bool isOver;

private:

	//-- explicitly disable the default constructor
	Scene (void);
	Scene (const Scene&);
	Scene& operator= (const Scene&);

protected:

	bool getIsOver (void) const;

public:

	explicit Scene (const char* debugName);
	virtual ~Scene (void)=0;

	virtual void quit (void);
};

//-------------------------------------------------------------------

inline bool Scene::getIsOver (void) const
{
	return isOver;
}

//-------------------------------------------------------------------

#endif
