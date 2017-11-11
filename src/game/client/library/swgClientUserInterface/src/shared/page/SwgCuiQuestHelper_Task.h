// ======================================================================
//
// SwgCuiQuestHelper_Task.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiQuestHelper_Task_H
#define INCLUDED_SwgCuiQuestHelper_Task_H

// ======================================================================

#include "sharedMath/Vector.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiQuestHelper.h"
#include "UIEventCallback.h"

// ----------------------------------------------------------------------

class UIEffector;
class UIImage;
class UIText;

// ======================================================================

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiQuestHelper::Task : public CuiMediator, public UIEventCallback
{
public:
	enum ProgressHighlight
	{
		PH_none,
		PH_flash,
		PH_fadeFull,
	};

	enum IndicatorImage
	{
		II_none,
		II_taskDot,
		II_waypointArrow,
		II_waypointAbove,
		II_waypointBelow,
		II_waypointSpace
	};

	enum TaskHighlight
	{
		TH_none,
		TH_progress,
		TH_task
	};

	enum TaskHighlightSound
	{
		THS_none,
		THS_counter
	};

public:
	Task(UIPage & page);
	~Task();

	bool OnMessage(UIWidget * context, UIMessage const & msg);
	void OnSizeChanged(UIWidget * context);

	void setTaskTitle(UIString const & taskTitle);
	void setTaskProgress(UIString const & taskProgress, ProgressHighlight const ph);
	void setWaypointDistance(UIString const & waypointDistance);
	void setWaypointRotation(float rotation);

	bool isSameHelperTask(uint32 questCrc, int taskId) const;
	void setQuestTaskData(uint32 questCrc, int taskId);

	bool cameraPositionChanged(Vector const & cameraPosition) const;
	void setCameraPosition(Vector const & cameraPosition);

	bool timerValueChanged(float timerValue) const;
	void setTimerValue(float timerValue);

	bool counterValuesChanged(int counterValue, int counterMax) const;
	void setCounterValues(int counterValue, int counterMax);

	bool playerPositionChanged(Vector const & playerPosition) const;
	void setPlayerPosition(Vector const & playerPosition);

	uint32 getQuestCrc() const;
	int getTaskId() const;

	void sizeToContent(boolean forceResize);
	long getHeight() const;

	void setIndicatorImage(IndicatorImage const indicatorImage);
	IndicatorImage getIndicatorImage() const;

	void highlightTask(TaskHighlight taskHighlight, TaskHighlightSound taskHighlightSound);

	bool hasCounter() const;

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	void cancelProgressEffectors();

private:
	MessageDispatch::Callback * m_callback;
	UIText * m_taskTitle;
	UIText * m_taskProgress;
	UIText * m_waypointDistance;
	UIImage * m_waypointArrow;
	UIImage * m_taskDot;
	UIImage * m_waypointAbove;
	UIImage * m_waypointBelow;
	UIImage * m_waypointSpace;
	UIPage * m_taskIndicator;
	UIEffector * m_flashProgress;
	UIEffector * m_fadeFullProgress;
	UIEffector * m_taskHighlight;
	uint32 m_questCrc;
	int m_taskId;
	Vector m_cameraPosition;
	float m_timerValue;
	int m_counterValue;
	int m_counterMax;
	Vector m_playerPosition;
	boolean m_needsResize;
	IndicatorImage m_indicatorImage;

private: //-- disabled
	Task();
	Task(Task const & rhs);
	Task & operator= (Task const & rhs);
};

// ======================================================================

#endif
