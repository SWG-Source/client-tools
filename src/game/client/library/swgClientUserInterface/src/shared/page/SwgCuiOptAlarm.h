//======================================================================
//
// SwgCuiOptAlarm.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiOptAlarm_H
#define INCLUDED_SwgCuiOptAlarm_H

#include "swgClientUserInterface/SwgCuiOptBase.h"
#include "clientAudio/SoundId.h"

class UIButton;
class UICheckbox;
class UIComboBox;
class UIList;
class UIText;
class UITextbox;

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiOptAlarm : 
public SwgCuiOptBase
{
public:
	explicit SwgCuiOptAlarm                (UIPage & page);

	virtual void OnButtonPressed           (UIWidget *context);
	virtual void OnCheckboxSet             (UIWidget *context);
	virtual bool OnMessage                 (UIWidget *context, const UIMessage &message);

	virtual void update                    (float deltaTimeSecs);

protected:

	virtual void performActivate           ();
	virtual void performDeactivate         ();
	virtual void queryWidgetValues();

private:

	typedef stdvector<int>::fwd             AlarmIdVector;
	void         updateAlarms              (AlarmIdVector const &alarms);
	void         addNewAlarm               ();
	void         removeAlarms              ();

	UIText *                    m_textAlarmEarthTime;
	UITextbox *                 m_textBoxAlarmInHours;
	UITextbox *                 m_textBoxAlarmInMinutes;
	UIComboBox *                m_comboBoxAlarmAtHour;
	UIComboBox *                m_comboBoxAlarmAtMinute;
	UIComboBox *                m_comboBoxAlarmSound;
	UIButton *                  m_buttonTestAlarmSound;
	UICheckbox *                m_checkBoxRecurring;
	UIText *                    m_textAlarmMessage;
	UIButton *                  m_buttonAddAlarm;
	UIButton *                  m_buttonRemoveAlarm;
	UIList *                    m_listAlarms;
	UICheckbox *                m_checkBoxAlarmAt;
	UICheckbox *                m_checkBoxAlarmIn;
	SoundId                     m_soundId;

	MessageDispatch::Callback * m_callback;

private:

	~SwgCuiOptAlarm ();

	SwgCuiOptAlarm & operator=(const SwgCuiOptAlarm & rhs);
	SwgCuiOptAlarm            (const SwgCuiOptAlarm & rhs);
};

//======================================================================

#endif
