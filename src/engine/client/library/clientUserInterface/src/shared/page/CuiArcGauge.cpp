//======================================================================
//
// CuiArcGuage.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiArcGauge.h"

#include "UIClock.h"
#include "UIEllipse.h"
#include "UIText.h"

//-----------------------------------------------------------------

namespace CuiArcGuageNamespace
{
	float const c_epsilon = 0.00001f;
	float const c_tickWidth = 0.015f;
	float const c_tickHalfWidth = c_tickWidth * 0.5f;
}

using namespace CuiArcGuageNamespace;

//-----------------------------------------------------------------

CuiArcGuage::CuiArcGuage(UIPage & page) :
CuiMediator("CuiArcGuage", page),
UIEventCallback(),
m_arcGauge(NULL),
m_arcGaugeOverdrive(NULL),
m_arcTick(NULL),
m_arcOverlay(NULL),
m_textOut(NULL),
m_textBuffer(new UIString),
m_readingCurrent(0.0f),
m_readingMax(1.0f),
m_readingDisplayMultiplier(1.0f),
m_tickDesired(1.0f),
m_dataIsDirty(true),
m_tickIsVisible(true),
m_overlayIsVisible(false)
{
	getCodeDataObject(TUIEllipse, m_arcGauge, "ArcGauge");
	getCodeDataObject(TUIEllipse, m_arcGaugeOverdrive, "ArcGaugeOverdrive");
	getCodeDataObject(TUIEllipse, m_arcTick, "ArcTick", true);
	getCodeDataObject(TUIEllipse, m_arcOverlay, "ArcOverlay", true);
	getCodeDataObject(TUIText, m_textOut, "Text", true);
}

//-----------------------------------------------------------------

CuiArcGuage::~CuiArcGuage()
{
	m_arcTick = NULL;
	m_arcGauge = NULL;
	m_arcOverlay = NULL;

	m_textOut = NULL;

	delete m_textBuffer;
	m_textBuffer = NULL;
}

//-----------------------------------------------------------------

void CuiArcGuage::performActivate()
{
	// Force an update.
	m_dataIsDirty = true;

	setIsUpdating(true);
}

//-----------------------------------------------------------------

void CuiArcGuage::performDeactivate()
{
	setIsUpdating(false);
}

//-----------------------------------------------------------------
void CuiArcGuage::update(float const elapsedTimeSecs)
{
	CuiMediator::update(elapsedTimeSecs);

	if (elapsedTimeSecs > FLT_MIN)
	{
		//-- Update the rest of the info if necessary.
		updateGauges();
		updateText();
		
		// reset this at this point.
		m_dataIsDirty = false;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::setCurrent(float const current)
{
	if (!WithinEpsilonInclusive(m_readingCurrent, current, c_epsilon))
	{
		m_readingCurrent = current;
		m_dataIsDirty = true;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::setTextDisplayMultiplier(float const multiplierValue)
{
	if (!WithinEpsilonInclusive(m_readingDisplayMultiplier, multiplierValue, c_epsilon))
	{
		m_readingDisplayMultiplier = multiplierValue;
		m_dataIsDirty = true;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::setMax(float const maxValue)
{
	if (!WithinEpsilonInclusive(m_readingMax, maxValue, c_epsilon))
	{
		m_readingMax = maxValue;
		m_dataIsDirty = true;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::setTick(float const tickValue)
{
	if (!WithinEpsilonInclusive(m_tickDesired, tickValue, c_epsilon))
	{
		m_tickDesired = tickValue;
		m_dataIsDirty = true;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::setTickVisible(bool const isVisible)
{
	if (!((m_tickIsVisible && isVisible) || (!m_tickIsVisible && !isVisible)))
	{
		m_tickIsVisible = isVisible;
		m_dataIsDirty = true;
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::updateText()
{
	if (m_textOut && m_dataIsDirty)
	{
		char buffer[128];
		snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(m_readingCurrent * m_readingDisplayMultiplier));
		buffer[sizeof(buffer) - 1] = NULL;
		m_textOut->SetText(Unicode::narrowToWide(buffer));
	}
}

//-----------------------------------------------------------------

void CuiArcGuage::updateGauges()
{
	if (m_dataIsDirty)
	{
		//-- Prevent the blowout.
		float readingMax = m_readingMax;
		float readingCurrent = m_readingCurrent;
		float readingOverdrive = 0.0f;
	
		if (m_readingCurrent > m_readingMax)
		{
			readingOverdrive = m_readingCurrent - m_readingMax;
			readingMax = m_readingCurrent;
			readingCurrent = m_readingMax;
		}

		//-- Update the tick.
		float const arcBegin = m_arcGauge->GetArcBegin();
		float const arcEnd = m_arcGauge->GetArcEnd();
		float const arcDiff = arcEnd - arcBegin;

		//-- Update the readout.
		float const ooMaxReading = (readingMax > c_epsilon) ? (1.f / readingMax) : 0.0f;
		float const gaugePercentage = clamp(0.0f, readingCurrent * ooMaxReading, 1.0f) + 0.005f;
		m_arcGauge->SetPercent(gaugePercentage);
		
		//-- Update the overdrive.
		if (m_arcGaugeOverdrive != NULL)
		{
			if (readingOverdrive > 0.0f)
			{
				m_arcGaugeOverdrive->SetVisible(true);
				m_arcGaugeOverdrive->SetPercent(1.0f - gaugePercentage);
				m_arcGaugeOverdrive->SetArcBegin(arcEnd);
				m_arcGaugeOverdrive->SetArcEnd(arcBegin);
			}
			else
			{
				m_arcGaugeOverdrive->SetVisible(false);
			}
		}

		//-- The Tick!
		if (m_arcTick)
		{
			m_arcTick->SetVisible(m_tickIsVisible);

			if (m_tickIsVisible)
			{
				float const tickPerc = (readingMax > c_epsilon) ? (m_tickDesired / readingMax) : 0.0f;
				float const arcTickBegin = arcBegin + arcDiff * tickPerc;
				m_arcTick->SetArcBegin(arcTickBegin - c_tickHalfWidth);
				m_arcTick->SetArcEnd(arcTickBegin + c_tickHalfWidth);
			}
		}

		if (m_arcOverlay)
		{
			m_arcOverlay->SetVisible(getPage().IsVisible());
		}
	}
}

//=================================================================
