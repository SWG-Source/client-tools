// ======================================================================
//
// NpcCuiConsole.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef NpcCuiConsole_H
#define NpcCuiConsole_H

#include "clientUserInterface/CuiMediator.h"

class UIPage;

//----------------------------------------------------------------------

class NpcCuiConsole : 
public CuiMediator
{
public:
	explicit                       NpcCuiConsole      (UIPage & page);

protected:
	virtual void                   performActivate   ();
	virtual void                   performDeactivate ();

private:
	virtual                       ~NpcCuiConsole ();
	                               NpcCuiConsole ();
	                               NpcCuiConsole (const NpcCuiConsole &);
	NpcCuiConsole &                operator=   (const NpcCuiConsole &);

private:
};

//-----------------------------------------------------------------

#endif
