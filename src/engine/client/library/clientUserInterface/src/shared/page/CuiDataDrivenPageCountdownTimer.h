// ======================================================================
//
// CuiDataDrivenPageCountdownTimer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiDataDrivenPageCountdownTimer_H
#define INCLUDED_CuiDataDrivenPageCountdownTimer_H

#include "clientUserInterface/CuiDataDrivenPage.h"

class UIPage;
class UIText;

// ======================================================================

class CuiDataDrivenPageCountdownTimer : 
public CuiDataDrivenPage
{
public:
	                           CuiDataDrivenPageCountdownTimer(const std::string & name, UIPage & thePage, int clientPageId);
	virtual                    ~CuiDataDrivenPageCountdownTimer();
	virtual void               update                         (float deltaTimeSecs);

protected:
	virtual void               onSetProperty   (std::string const & widgetPath, bool isThisPage, std::string const & propertyName, Unicode::String const & propertyValue);

private:
	//disabled
	CuiDataDrivenPageCountdownTimer            ();
	CuiDataDrivenPageCountdownTimer            (const CuiDataDrivenPageCountdownTimer& rhs);
	CuiDataDrivenPageCountdownTimer& operator= (const CuiDataDrivenPageCountdownTimer& rhs);

private:
	UIText *    m_text;
	UIPage *    m_valueBar;
	bool        m_visible;
	Unicode::String m_label;
	int         m_totalTime;
	time_t      m_startTime;
	time_t      m_stopTime;
	time_t      m_lastUpdateTime;
};

// ======================================================================

#endif
