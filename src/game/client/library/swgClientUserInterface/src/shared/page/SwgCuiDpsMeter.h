//======================================================================
//
// SwgCuiDpsMeter.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiDpsMeter_H
#define INCLUDED_SwgCuiDpsMeter_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "StringId.h"

class UIImage;
class UIText;
class Vector;
class UIButton;

//----------------------------------------------------------------------

class SwgCuiDpsMeter : 
public SwgCuiLockableMediator 
{
public:

	explicit            SwgCuiDpsMeter  (UIPage & page);

	void                update             (float deltaTimeSecs);
	virtual void        OnButtonPressed    (UIWidget *context );

protected:
	virtual void        performActivate    ();
	virtual void        performDeactivate  ();

private:
	virtual            ~SwgCuiDpsMeter  ();
	SwgCuiDpsMeter ();
	SwgCuiDpsMeter (const SwgCuiDpsMeter &);
	SwgCuiDpsMeter & operator= (const SwgCuiDpsMeter &);

	UIText *                     m_textDps;
	UIText *                     m_textDpsTotal;
	UIText *                     m_textTaken;
	UIText *                     m_textTakenTotal;

	UIButton *                   m_buttonReset;
	
};

//======================================================================

#endif
