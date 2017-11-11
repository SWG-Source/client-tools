//======================================================================
//
// SwgCuiHudGround.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudGround_H
#define INCLUDED_SwgCuiHudGround_H

#include "swgClientUserInterface/SwgCuiHud.h"

//======================================================================

class CuiConversationMenu;

//======================================================================

class SwgCuiHudGround : public SwgCuiHud
{
public:

	static void createHudTemplate();
	static void createFreshHud();

public:
	 SwgCuiHudGround (UIPage & page);
	 ~SwgCuiHudGround ();

	void update(float deltaTimeSecs);

	void performActivate();
	void performDeactivate();

private:
	SwgCuiHudGround();
	SwgCuiHudGround(const SwgCuiHudGround & rhs);
	SwgCuiHudGround & operator= (const SwgCuiHudGround & rhs);

private:

	void newConvo();
	void deleteConvo();

	CuiConversationMenu * m_convo;
};

//======================================================================

#endif
