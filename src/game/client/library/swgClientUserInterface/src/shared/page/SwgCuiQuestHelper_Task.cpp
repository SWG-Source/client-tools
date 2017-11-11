// ======================================================================
//
// SwgCuiQuestHelper_Task.cpp
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiQuestHelper_Task.h"

// ----------------------------------------------------------------------

#include "clientGame/QuestJournalManager.h"
#include "clientUserInterface/CuiSoundManager.h"
#include "clientUserInterface/CuiSounds.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "UIComposite.h"
#include "UIEffector.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIText.h"

// ======================================================================

namespace SwgCuiQuestHelper_TaskNamespace
{
}

using namespace SwgCuiQuestHelper_TaskNamespace;

// ======================================================================

SwgCuiQuestHelper::Task::Task(UIPage & page)
 : CuiMediator("SwgCuiQuestHelper_Task", page)
 , UIEventCallback()
 , m_callback(new MessageDispatch::Callback)
 , m_taskTitle(0)
 , m_taskProgress(0)
 , m_waypointDistance(0)
 , m_waypointArrow(0)
 , m_taskDot(0)
 , m_waypointAbove(0)
 , m_waypointBelow(0)
 , m_waypointSpace(0)
 , m_taskIndicator(0)
 , m_flashProgress(0)
 , m_fadeFullProgress(0)
 , m_taskHighlight(0)
 , m_questCrc(0)
 , m_taskId(-1)
 , m_cameraPosition()
 , m_timerValue(-1)
 , m_counterValue(-1)
 , m_counterMax(-1)
 , m_playerPosition()
 , m_needsResize(false)
 , m_indicatorImage(II_none)
{
	getCodeDataObject(TUIText, m_taskTitle, "taskTitle");
	getCodeDataObject(TUIText, m_taskProgress, "taskProgress");
	getCodeDataObject(TUIText, m_waypointDistance, "waypointDistance");
	getCodeDataObject(TUIImage, m_waypointArrow, "waypointArrow");
	getCodeDataObject(TUIImage, m_waypointAbove, "waypointAbove");
	getCodeDataObject(TUIImage, m_waypointBelow, "waypointBelow");
	getCodeDataObject(TUIImage, m_waypointSpace, "waypointSpace");
	getCodeDataObject(TUIImage, m_taskDot, "taskDot");
	getCodeDataObject(TUIPage, m_taskIndicator, "taskIndicator");
	getCodeDataObject(TUIEffector, m_flashProgress, "flashProgress");
	getCodeDataObject(TUIEffector, m_fadeFullProgress, "fadeFullProgress");
	getCodeDataObject(TUIEffector, m_taskHighlight, "taskHighlight");

	registerMediatorObject(getPage(), true);

	setQuestTaskData(0, -1);
}

// ----------------------------------------------------------------------

SwgCuiQuestHelper::Task::~Task()
{
	delete m_callback;
	m_callback = 0;

	m_taskTitle = 0;
	m_taskProgress = 0;
	m_waypointDistance = 0;
	m_waypointArrow = 0;
	m_waypointAbove = 0;
	m_waypointBelow = 0;
	m_waypointSpace = 0;
	m_taskDot = 0;
	m_taskIndicator = 0;
	m_flashProgress = 0;
	m_fadeFullProgress = 0;
	m_taskHighlight = 0;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::performActivate()
{
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::performDeactivate()
{
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setTaskTitle(UIString const & taskTitle)
{
	m_taskTitle->SetLocalText(taskTitle);
	m_needsResize = true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setTaskProgress(UIString const & taskProgress, ProgressHighlight const ph)
{
	m_taskProgress->SetVisible(true);
	m_taskProgress->SetLocalText(taskProgress);
	m_needsResize = true;

	switch (ph)
	{
	case PH_flash:
		UIManager::gUIManager().ExecuteEffector(m_flashProgress, m_taskProgress, false);
		break;
	case PH_fadeFull:
		m_taskProgress->SetOpacity(0);
		UIManager::gUIManager().ExecuteEffector(m_fadeFullProgress, m_taskProgress, false);
		break;
	case PH_none:
		cancelProgressEffectors();
		break;
	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setWaypointDistance(UIString const & waypointDistance)
{
	m_waypointDistance->SetVisible(true);
	m_waypointDistance->SetLocalText(waypointDistance);
	m_needsResize = true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setWaypointRotation(float rotation)
{
	m_waypointArrow->SetRotation(rotation);
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setQuestTaskData(uint32 questCrc, int taskId)
{
	m_questCrc = questCrc;
	m_taskId = taskId;
	m_taskProgress->SetVisible(false);
	cancelProgressEffectors();
	m_waypointDistance->SetVisible(false);
	setIndicatorImage(II_taskDot);
	m_needsResize = true;
	m_timerValue = -1;
	m_counterValue = -1;
	m_counterMax = -1;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::isSameHelperTask(uint32 questCrc, int taskId) const
{
	return m_questCrc == questCrc && m_taskId == taskId;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::cameraPositionChanged(Vector const & cameraPosition) const
{
	return m_cameraPosition != cameraPosition;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setCameraPosition(Vector const & cameraPosition)
{
	m_cameraPosition = cameraPosition;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::timerValueChanged(float timerValue) const
{
	return !WithinEpsilonInclusive(m_timerValue, timerValue, 0.1f) || !m_taskProgress->IsVisible();
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setTimerValue(float timerValue)
{
	m_timerValue = timerValue;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::counterValuesChanged(int counterValue, int counterMax) const
{
	return m_counterValue != counterValue || m_counterMax != counterMax || !m_taskProgress->IsVisible();
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setCounterValues(int counterValue, int counterMax)
{
	m_counterValue = counterValue;
	m_counterMax = counterMax;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::playerPositionChanged(Vector const & playerPosition) const
{
	return m_playerPosition != playerPosition || !m_waypointDistance->IsVisible();
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setPlayerPosition(Vector const & playerPosition)
{
	m_playerPosition = playerPosition;
}

// ----------------------------------------------------------------------

uint32 SwgCuiQuestHelper::Task::getQuestCrc() const
{
	return m_questCrc;
}

// ----------------------------------------------------------------------

int SwgCuiQuestHelper::Task::getTaskId() const
{
	return m_taskId;
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::OnMessage(UIWidget * context, const UIMessage & msg)
{
	if (context == &getPage())
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			UIWidget *widget = getPage().GetWidgetFromPoint(msg.MouseCoords, false);

			if (widget && widget->GetName() == "waypointArrow")
				QuestJournalManager::toggleQuestWaypoint(m_questCrc, m_taskId);
		}
	}

	return true;
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::sizeToContent(boolean forceResize)
{
	if (m_needsResize || forceResize)
	{
		UIScalar spacing = getComposite().GetSpacing();
		UIScalar widthUsed = m_taskIndicator->GetSize().x + spacing;
		
		if (m_taskProgress->IsVisible())
		{
			m_taskProgress->SizeToContent();
			widthUsed += m_taskProgress->GetSize().x + spacing;
		}

		if (m_waypointDistance->IsVisible())
		{
			m_waypointDistance->SizeToContent();
			widthUsed += m_waypointDistance->GetSize().x + spacing;
		}

		UIScalar remainingWidth = getPage().GetSize().x - widthUsed;

		m_taskTitle->SetMaximumSize(UISize(remainingWidth, m_taskTitle->GetMaximumSize().y));
		m_taskTitle->SizeToContent();

		getPage().SetSize(UISize(getPage().GetWidth(), m_taskTitle->GetHeight()));

		getComposite().Pack();

		m_needsResize = false;
	}
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::OnSizeChanged(UIWidget * context)
{
	if (context == &getPage())
	{
		sizeToContent(true);
	}
}

// ----------------------------------------------------------------------

long SwgCuiQuestHelper::Task::getHeight() const
{
	return getPage().GetHeight();
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::cancelProgressEffectors()
{
	m_taskProgress->CancelEffector(*m_flashProgress);
	m_taskProgress->CancelEffector(*m_fadeFullProgress);
	m_taskProgress->SetOpacity(1);
}

// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::setIndicatorImage(IndicatorImage const indicatorImage)
{
	if (m_indicatorImage == indicatorImage)
		return;

	m_waypointArrow->SetVisible(indicatorImage == II_waypointArrow);
	m_waypointAbove->SetVisible(indicatorImage == II_waypointAbove);
	m_waypointBelow->SetVisible(indicatorImage == II_waypointBelow);
	m_waypointSpace->SetVisible(indicatorImage == II_waypointSpace);
	m_taskDot->SetVisible(indicatorImage == II_taskDot);

	m_indicatorImage = indicatorImage;
}

// ----------------------------------------------------------------------

SwgCuiQuestHelper::Task::IndicatorImage SwgCuiQuestHelper::Task::getIndicatorImage() const
{
	return m_indicatorImage;
}


// ----------------------------------------------------------------------

void SwgCuiQuestHelper::Task::highlightTask(TaskHighlight taskHighlight, TaskHighlightSound taskHighlightSound)
{
	switch (taskHighlight)
	{
	case TH_none:
		break;
	case TH_progress:
		m_taskProgress->SetBackgroundOpacity(1.0);
		UIManager::gUIManager().ExecuteEffector(m_taskHighlight, m_taskProgress, false);
		break;
	case TH_task:
		getPage().SetBackgroundOpacity(1.0);
		UIManager::gUIManager().ExecuteEffector(m_taskHighlight, &getPage(), false);
		break;
	default:
		DEBUG_WARNING(true, ("No highlight effect defined!"));
		break;
	}

	switch (taskHighlightSound)
	{
	case THS_none:
		break;
	case THS_counter:
		CuiSoundManager::play(CuiSounds::npe2_quest_counter);
		break;
	default:
		DEBUG_WARNING(true, ("No highlight sound defined!"));
		break;
	}
}

// ----------------------------------------------------------------------

bool SwgCuiQuestHelper::Task::hasCounter() const
{
	return m_counterValue != -1;
}

// ======================================================================
