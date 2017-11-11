// ======================================================================
//
// CuiDataDrivenPageCountdownTimer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiDataDrivenPageCountdownTimer.h"

#include "StringId.h"
#include "UIPage.h"
#include "UIText.h"
#include "sharedGame/SuiPageData.h"

// ======================================================================

CuiDataDrivenPageCountdownTimer::CuiDataDrivenPageCountdownTimer(const std::string & name, UIPage& page, int clientPageId) :
CuiDataDrivenPage(name, page, clientPageId),
m_text(NULL),
m_valueBar(NULL),
m_visible(false),
m_label(),
m_totalTime(0),
m_startTime(0),
m_stopTime(0),
m_lastUpdateTime(0)
{
	getCodeDataObject(TUIText, m_text,     "text");
	getCodeDataObject(TUIPage, m_valueBar, "valueBar");

	if (m_text && m_valueBar)
	{
		m_text->SetVisible(false);
		m_valueBar->SetVisible(false);
		m_text->SetPreLocalized(true);
	}
}

//-----------------------------------------------------------------

CuiDataDrivenPageCountdownTimer::~CuiDataDrivenPageCountdownTimer()
{
	setIsUpdating(false);
}

//----------------------------------------------------------------------

void CuiDataDrivenPageCountdownTimer::update(float deltaTimeSecs)
{
	if (m_text && m_valueBar && (m_startTime > 0) && (m_stopTime > 0) && (m_totalTime > 0))
	{
		time_t now = ::time(NULL);
		if (now != m_lastUpdateTime)
		{
			if (m_lastUpdateTime <= m_stopTime)
			{
				if (!m_visible)
				{
					m_visible = true;
					m_text->SetVisible(true);
					m_valueBar->SetVisible(true);
				}

				const int timeLeft = std::max(static_cast<int>(0), static_cast<int>(m_stopTime - now));

				char buffer[32];
				snprintf(buffer, sizeof(buffer)-1, ": %d", timeLeft);
				buffer[sizeof(buffer)-1] = '\0';

				Unicode::String label = m_label + Unicode::narrowToWide(buffer);
				m_text->SetLocalText(label);

				const UIWidget * const parent = NON_NULL(m_valueBar->GetParentWidget());
				const long usableWidth = parent->GetWidth();
				const long width = timeLeft ? (usableWidth * timeLeft / m_totalTime) : 0L;
				m_valueBar->SetWidth(width);
			}
			else
			{
				// countdown timer has finished, no need for update() to be called anymore
				setIsUpdating(false);
			}

			m_lastUpdateTime = now;
		}
	}

	CuiDataDrivenPage::update(deltaTimeSecs);
}

//-----------------------------------------------------------------

void CuiDataDrivenPageCountdownTimer::onSetProperty(std::string const & widgetPath, bool isThisPage, std::string const & propertyName, Unicode::String const & propertyValue)
{
	if ((_stricmp(widgetPath.c_str(), "comp.text") == 0) && (_stricmp(propertyName.c_str(), CuiDataDrivenPageNamespace::titleProperty) == 0))
	{
		StringId sid(Unicode::wideToNarrow(propertyValue));
		if (sid.isValid())
			m_label = sid.localize();
		else
			m_label = propertyValue;
	}
	else if (isThisPage && (_stricmp(propertyName.c_str(), "countdownTimerTimeValue") == 0))
	{
		int current, total;
		if (sscanf(Unicode::wideToNarrow(propertyValue).c_str(), "%d,%d", &current, &total) == 2)
		{
			// limit the countdown timer to 1 day
			if ((current >= 0) && (total > 0) && (current < total) && (total <= 86400))
			{
				time_t now = ::time(NULL);
				m_startTime = now - current;
				m_stopTime = m_startTime + total;
				m_totalTime = total;
				m_lastUpdateTime = 0;

				// request that update() be called so that we can continuously update the countdown timer
				setIsUpdating(true);
			}
		}
	}

	CuiDataDrivenPage::onSetProperty(widgetPath, isThisPage, propertyName, propertyValue);
}

// ======================================================================
