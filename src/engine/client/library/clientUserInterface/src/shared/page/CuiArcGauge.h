//======================================================================
//
// CuiArcGuage.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiArcGuage_H
#define INCLUDED_CuiArcGuage_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"
#include "UINotification.h"

//-----------------------------------------------------------------

class UIEllipse;
class UIText;

//-----------------------------------------------------------------

class CuiArcGuage :
public CuiMediator,
public UIEventCallback
{
public:
	explicit CuiArcGuage(UIPage & parentPage);

	void setCurrent(float currentPercent);
	void setMax(float maxValue);

	void setTick(float tickValue);
	void setTickWidth(float tickWidth);
	void setTickVisible(bool isVisible);

	void setOverlayTicks(float tickCount);
	void setOverlayVisible(bool isVisible);

	void setText(char const * newText);
	void setTextDisplayMultiplier(float multiplierValue);

	void update(float elapsedTimeSecs);

protected:
	void performActivate();
	void performDeactivate();

private:
	CuiArcGuage();
	~CuiArcGuage();
	CuiArcGuage(const CuiArcGuage & rhs);
	CuiArcGuage & operator=(const CuiArcGuage & rhs);

	void updateText();
	void updateGauges();

private:
	UIEllipse * m_arcGauge;
	UIEllipse * m_arcGaugeOverdrive;
	UIEllipse * m_arcTick;
	UIEllipse * m_arcOverlay;
	UIText * m_textOut;
	UIString * m_textBuffer;

	float m_readingCurrent;
	float m_readingMax;
	float m_readingDisplayMultiplier;
	
	float m_tickDesired;

	bool m_dataIsDirty;
	bool m_tickIsVisible;
	bool m_overlayIsVisible;
};


//======================================================================

#endif
