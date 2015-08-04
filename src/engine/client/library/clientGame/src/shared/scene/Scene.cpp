//
// Scene.cpp
// asommers 5-31-2000
//
// copyright 2000, verant interactive, inc.
//

//-------------------------------------------------------------------

#include "clientGame/FirstClientGame.h"
#include "clientGame/Scene.h"

//-------------------------------------------------------------------

int Scene::currentScreenShot;

//-------------------------------------------------------------------

Scene::Scene (const char* debugName) : 
	IoWin (debugName),
	isOver (false)
{
}

//-------------------------------------------------------------------
	
Scene::~Scene(void)
{
}

//-------------------------------------------------------------------

void Scene::quit (void)
{
	isOver = true;
}

//-------------------------------------------------------------------

