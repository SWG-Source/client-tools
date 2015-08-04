//======================================================================
//
// SwgCuiExpMonitorBox.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiExpMonitorBox_H
#define INCLUDED_SwgCuiExpMonitorBox_H

//======================================================================

class SwgCuiExpMonitor;

#include "clientUserInterface/CuiMediator.h"

//----------------------------------------------------------------------

class SwgCuiExpMonitorBox :
public CuiMediator
{
public:
	
	explicit SwgCuiExpMonitorBox (UIPage & page);

protected:

	void  performActivate ();
	void  performDeactivate ();
	
private:

	SwgCuiExpMonitorBox (const SwgCuiExpMonitorBox & rhs);
	SwgCuiExpMonitorBox & operator= (const SwgCuiExpMonitorBox & rhs);

	~SwgCuiExpMonitorBox ();

private:

	UISmartPointer<SwgCuiExpMonitor> m_experienceMonitor;
};

//======================================================================

#endif
