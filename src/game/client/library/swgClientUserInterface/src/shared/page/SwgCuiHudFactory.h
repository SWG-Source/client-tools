//======================================================================
//
// SwgCuiHudFactory.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudFactory_H
#define INCLUDED_SwgCuiHudFactory_H

//======================================================================

class SwgCuiHud;

//----------------------------------------------------------------------

class SwgCuiHudFactory
{
public:
	static void setHudActive(bool active);
	static void createHudTemplates();
	static void releaseHudIfNeeded();
	static void createHudIfNeeded();
	static SwgCuiHud * findMediatorForCurrentHud();
	static bool isHudActive();
	static bool shouldCreateInterface();
	static void reset();
	static void updateShaders();
};

//======================================================================

#endif
