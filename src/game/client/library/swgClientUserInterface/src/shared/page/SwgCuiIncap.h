//======================================================================
//
// SwgCuiIncap.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiIncap_H
#define INCLUDED_SwgCuiIncap_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

class UIText;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiIncap :
public CuiMediator
{
public:

	explicit                  SwgCuiIncap              (UIPage & page);
	void                      update                   (float deltaTimeSecs);

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:

	                          ~SwgCuiIncap ();
	SwgCuiIncap ();
	SwgCuiIncap (const SwgCuiIncap & rhs);
	SwgCuiIncap & operator= (const SwgCuiIncap & rhs);

	UIText *                    m_text;
	UIPage *                    m_valueBar;
	int                         m_lastCounter;
	int                         m_lastCounterPrinted;
	float                       m_elapsedTimeSecs;
	int                         m_maxRange;
};

//======================================================================

#endif
