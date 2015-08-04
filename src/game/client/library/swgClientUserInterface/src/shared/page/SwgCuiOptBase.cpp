//======================================================================
//
// SwgCuiOptBase.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptBase.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIUtils.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsOptions.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMessageDispatch/Transceiver.h"
#include <algorithm>
#include <map>

#define WARN_NO_DEFAULT true

//----------------------------------------------------------------------

namespace
{
	const int SLIDER_RANGE     = 100;
	const float SLIDER_RANGE_F = 100.0f;
}

//======================================================================

class SwgCuiOptBase::ControlDataBase
{
public:
	virtual void updateControl     (bool storeDefault) = 0;
	virtual void updateFromControl () = 0;
	virtual void revert            () = 0;
	virtual void resetDefault      () = 0;
	virtual ~ControlDataBase () = 0 {}
	ControlDataBase () : m_ignoreNotification (false) {}

	bool m_ignoreNotification;
};

//----------------------------------------------------------------------

class SwgCuiOptBase::SliderData : public ControlDataBase, public UIEventCallback
{
public:
	
	template <typename T, typename Getter, typename Setter, typename Func> 
		class Value
	{
	public:
		bool          ok;
		T             start;
		T             endAllowed;
		T             end;
		Setter        setter;
		Getter        getter;
		Getter        getterDefault;
		Func          func;
		T             revertData;
		
		long          convertToRaw   (T v);
		T             convertFromRaw (long v);
		
		Value           () :
			ok              (false),
			start           (0),
			end             (0),
			setter          (0),
			getter          (0),
			getterDefault   (0),
			func            (0),
			revertData      (0)
		{
		}
		
		Value           (T _start, T _endAllowed, T _end, Setter _setter, Getter _getter, Getter _getterDefault, Func _func = 0) :
			ok              (true),
			start           (_start),
			endAllowed      (_endAllowed),
			end             (_end),
			setter          (_setter),
			getter          (_getter),
			getterDefault   (_getterDefault),
			func            (_func),
			revertData      (_start)
		{
				NOT_NULL (getter);
				DEBUG_FATAL (start == end, ("empty range"));
		}
	};
	
	typedef Value<int,   StaticGetterInt,   StaticSetterInt,   SliderFuncInt>   ValueInt;
	typedef Value<float, StaticGetterFloat, StaticSetterFloat, SliderFuncFloat> ValueFloat;
	
	ValueInt                valueInt;
	ValueFloat              valueFloat;
	
	SliderFuncRaw           funcRaw;
	UISliderbar *           slider;

	SwgCuiOptBase *         base;
 
	SliderData () :
	valueInt      (),
	valueFloat    (),
	slider        (0),
	funcRaw       (0),
	base          (0)
	{
	}

	//----------------------------------------------------------------------

	SliderData (const SliderData & rhs) :
	valueInt      (rhs.valueInt),
	valueFloat    (rhs.valueFloat),
	slider        (rhs.slider),
	funcRaw       (rhs.funcRaw),
	base          (rhs.base)
	{
		if (slider)
		{
			slider->Attach (0);
			slider->AddCallback (this);
		}
	}

	//----------------------------------------------------------------------

	SliderData & operator= (const SliderData & rhs)
	{
		if (this == &rhs)
			return *this;

		if (rhs.slider)
			rhs.slider->Attach (0);

		if (slider)
		{
			slider->Detach (0);
			slider->RemoveCallback (this);
		}

		valueInt      = rhs.valueInt;
		valueFloat    = rhs.valueFloat;
		slider        = rhs.slider;
		funcRaw       = rhs.funcRaw;
		base          = rhs.base;

		if (slider)
			slider->AddCallback (this);

		return *this;
	}

	//----------------------------------------------------------------------

	SliderData (SwgCuiOptBase & _base, UISliderbar & _slider, StaticSetterInt  _setter, StaticGetterInt   _getter, StaticGetterInt   _getterDefault, int _start,   int _endAllowed, int _end, SliderFuncInt _func = 0,   SliderFuncRaw _funcRaw = 0) :
	valueInt      (_start, _endAllowed, _end, _setter, _getter, _getterDefault, _func),
	valueFloat    (),
	slider        (&_slider),
	funcRaw       (_funcRaw),
	base          (&_base)
	{
		WARNING (WARN_NO_DEFAULT && !_getterDefault, ("SwgCuiOptBase no default [%s]", _slider.GetFullPath ().c_str ()));
		slider->Attach (0);
		slider->AddCallback (this);
		slider->SetLowerLimit (_start);
		slider->SetUpperLimit (_end);
		slider->SetLowerLimit (_start);

		if (_endAllowed < _end)
		{
			slider->SetUpperLimitAllowedEnabled (true);
			slider->SetUpperLimitAllowed (_endAllowed);
		}
		else
			slider->SetUpperLimitAllowedEnabled (false);
	}

	//----------------------------------------------------------------------

	SliderData    (SwgCuiOptBase & _base, UISliderbar & _slider, StaticSetterFloat  _setter, StaticGetterFloat   _getter, StaticGetterFloat   _getterDefault, float _start,   float _endAllowed, float _end, SliderFuncFloat _func = 0,   SliderFuncRaw _funcRaw = 0) :
	valueInt      (),
	valueFloat    (_start, _endAllowed, _end, _setter, _getter, _getterDefault, _func),
	slider        (&_slider),
	funcRaw       (_funcRaw),
	base          (&_base)
	{
		WARNING (WARN_NO_DEFAULT && !_getterDefault, ("SwgCuiOptBase no default [%s]", _slider.GetFullPath ().c_str ()));
		slider->Attach (0);
		slider->AddCallback (this);
		slider->SetLowerLimit (0);
		slider->SetUpperLimit (SLIDER_RANGE);
		slider->SetLowerLimit (0);

		const float f_range = _end - _start;

		if (f_range > 0.0f && _endAllowed < _end)
		{
			const float f_rangeAllowed = _endAllowed - _start;

			slider->SetUpperLimitAllowedEnabled (true);
			slider->SetUpperLimitAllowed (static_cast<long>(SLIDER_RANGE * (f_rangeAllowed / f_range)));
		}
		else
			slider->SetUpperLimitAllowedEnabled (false);

	}

	//----------------------------------------------------------------------

	~SliderData ()
	{
		slider->RemoveCallback (this);
		slider->Detach (0);
	}

	void revert ()
	{
		if (valueInt.ok)
		{
			if (valueInt.setter)
			{
				const int  value     = valueInt.revertData;
				const long valueRaw  = valueInt.convertToRaw (value);

				valueInt.setter (value);
				Unicode::String str;
				UIUtils::FormatInteger (str, value);
				slider->SetLocalTooltip (str);

				const bool oldIgnoreNotification = m_ignoreNotification;
				m_ignoreNotification = true;
				slider->SetValue (valueRaw, true);
				m_ignoreNotification = oldIgnoreNotification;
			}
		}
		else if (valueFloat.ok)
		{
			if (valueFloat.setter)
			{
				const float  value     = valueFloat.revertData;
				const long valueRaw    = valueFloat.convertToRaw (value);
				valueFloat.setter (value);
				Unicode::String str;
				UIUtils::FormatFloat (str, value);
				slider->SetLocalTooltip (str);

				const bool oldIgnoreNotification = m_ignoreNotification;
				m_ignoreNotification = true;
				slider->SetValue        (valueRaw, true);
				m_ignoreNotification = oldIgnoreNotification;

				if (valueFloat.func)
					valueFloat.func (*base, *slider, value);
			}
		}

		if (funcRaw)
			funcRaw (*base, *slider);
	}

	//----------------------------------------------------------------------

	void updateControl (bool storeRevertData = false)
	{
		if (valueInt.ok)
		{
			if (valueInt.getter)
			{
				const int  value     = valueInt.getter       ();
				if (storeRevertData)
					valueInt.revertData = value;
				else
				{
					const long valueRaw  = valueInt.convertToRaw (value);
					slider->SetValue (valueRaw, false);
					Unicode::String str;
					UIUtils::FormatInteger (str, value);
					slider->SetLocalTooltip (str);
				}
			}
		}
		else if (valueFloat.ok)
		{
			if (valueFloat.getter)
			{
				const float value     = valueFloat.getter       ();
				if (storeRevertData)
					valueFloat.revertData = value;
				else
				{
					const long  valueRaw  = valueFloat.convertToRaw (value);
					slider->SetValue (valueRaw, false);
					Unicode::String str;
					UIUtils::FormatFloat    (str, value);
					slider->SetLocalTooltip (str);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	void updateFromControl ()
	{
		const long valueRaw  = slider->GetValue ();
		
		if (valueInt.ok)
		{
			if (valueInt.setter)
			{
				const int  value     = valueInt.convertFromRaw (valueRaw);
				valueInt.setter (value);
				Unicode::String str;
				UIUtils::FormatInteger (str, value);
				slider->SetLocalTooltip (str);
			}
		}
		else if (valueFloat.ok)
		{
			if (valueFloat.setter)
			{
				const float  value     = valueFloat.convertFromRaw (valueRaw);
				valueFloat.setter (value);
				Unicode::String str;
				UIUtils::FormatFloat (str, value);
				slider->SetLocalTooltip (str);

				if (valueFloat.func)
					valueFloat.func (*base, *slider, value);
			}
		}

		if (funcRaw)
			funcRaw (*base, *slider);
	}

	//----------------------------------------------------------------------

	void resetDefault ()
	{
		if (valueInt.ok)
		{
			if (valueInt.setter && valueInt.getterDefault)
				valueInt.setter (valueInt.getterDefault ());
		}
		else if (valueFloat.ok)
		{
			if (valueFloat.setter && valueFloat.getterDefault)
				valueFloat.setter (valueFloat.getterDefault ());				
		}
		updateControl (false);
	}

	//----------------------------------------------------------------------

	void OnSliderbarChanged (UIWidget * context)
	{
		if (base && base->isActive () && context == slider && !m_ignoreNotification)
			updateFromControl ();
	}

};


//----------------------------------------------------------------------


template<> long SwgCuiOptBase::SliderData::ValueInt::convertToRaw (int v)
{
	return v;
}

template<> int SwgCuiOptBase::SliderData::ValueInt::convertFromRaw (long v)
{
	return v;
}

template<typename T, typename Getter, typename Setter, typename Func> 
long SwgCuiOptBase::SliderData::Value<T, Getter, Setter, Func>::convertToRaw(T v)
{
	const float range = static_cast<float>(end - start);
	if (range == 0.0f)
		return long(0);
	return static_cast<long>((static_cast<float>(v) - start) * SLIDER_RANGE_F / range);
}

template<typename T, typename Getter, typename Setter, typename Func> 
T SwgCuiOptBase::SliderData::Value<T, Getter, Setter, Func>::convertFromRaw(long v)
{
	const float range = static_cast<float>(end - start);
	if (range == 0.0f)
		return long(0);
	return static_cast<T>(start + (static_cast<float>(v) * range / SLIDER_RANGE_F));
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

class SwgCuiOptBase::CheckboxData : public ControlDataBase, public UIEventCallback
{
public:

	StaticSetterBool        setter;
	StaticGetterBool        getter;
	StaticGetterBool        getterDefault;
	CheckboxFunc            func;	
	UICheckbox *            checkbox;
	SwgCuiOptBase *         base;
	bool                    revertData;

	//----------------------------------------------------------------------

	CheckboxData (const CheckboxData & rhs) :
	setter          (rhs.setter),
	getter          (rhs.getter),
	getterDefault   (rhs.getterDefault),
	func            (rhs.func),
	checkbox        (rhs.checkbox),
	base            (rhs.base),
	revertData      (rhs.revertData)
	{
		if (checkbox)
		{
			checkbox->Attach (0);
			checkbox->AddCallback (this);
		}
	}

	//----------------------------------------------------------------------

	CheckboxData & operator= (const CheckboxData & rhs)
	{
		if (this == &rhs)
			return *this;
		
		if (rhs.checkbox)
			rhs.checkbox->Attach (0);
		
		if (checkbox)
		{
			checkbox->Detach (0);
			checkbox->RemoveCallback (this);
		}
		
		setter          = rhs.setter;
		getter          = rhs.getter;
		getterDefault   = rhs.getterDefault;
		func            = rhs.func;
		checkbox        = rhs.checkbox;
		base            = rhs.base;
		revertData      = rhs.revertData;
		
		if (checkbox)
			checkbox->AddCallback (this);
		
		return *this;
	}

	//----------------------------------------------------------------------

	CheckboxData (SwgCuiOptBase & _base, UICheckbox & _checkbox, StaticSetterBool  _setter, StaticGetterBool   _getter, StaticGetterBool   _getterDefault, CheckboxFunc _func) :
	setter          (_setter),
	getter          (_getter),
	getterDefault   (_getterDefault),
	func            (_func),
	checkbox        (&_checkbox),
	base            (&_base),
	revertData      (false)
	{
		WARNING (WARN_NO_DEFAULT && !_getterDefault, ("SwgCuiOptBase no default [%s]", _checkbox.GetFullPath ().c_str ()));

		checkbox->Attach (0);
		checkbox->AddCallback (this);
	}

	//----------------------------------------------------------------------

	~CheckboxData ()
	{
		checkbox->RemoveCallback (this);
		checkbox->Detach (0);
	}

	//----------------------------------------------------------------------

	void revert ()
	{
		const bool value  = revertData;
		
		if (setter)
			setter (value);

		const bool oldIgnoreNotification = m_ignoreNotification;
		m_ignoreNotification = true;
		checkbox->SetChecked (value, true);
		m_ignoreNotification = oldIgnoreNotification;

		if (func)
			func (*base, *checkbox, value);
	}

	//----------------------------------------------------------------------

	void updateControl (bool storeRevertData = false)
	{
		if (getter)
		{
			const bool value     = getter       ();

			const bool oldIgnoreNotification = m_ignoreNotification;
			m_ignoreNotification = true;
			checkbox->SetChecked (value, true);
			m_ignoreNotification = oldIgnoreNotification;

			if (storeRevertData)
				revertData = value;
		}
	}

	//----------------------------------------------------------------------

	void updateFromControl ()
	{
		const bool value  = checkbox->IsChecked ();
		
		if (setter)
			setter (value);

		if (func)
			func (*base, *checkbox, value);
	}

	//----------------------------------------------------------------------

	void resetDefault ()
	{
		if (setter && getterDefault)
			setter (getterDefault ());

		updateControl (false);
	}

	//----------------------------------------------------------------------

	void OnCheckboxSet (UIWidget * context)
	{
		if (base->isActive () && context == checkbox && !m_ignoreNotification)
			updateFromControl ();
	}

	//----------------------------------------------------------------------

	void OnCheckboxUnset (UIWidget * context)
	{
		if (base && base->isActive () && context == checkbox && !m_ignoreNotification)
			updateFromControl ();
	}
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


class SwgCuiOptBase::ComboBoxData : public ControlDataBase, public UIEventCallback
{
public:

	ComboSetterFunc        setter;
	ComboGetterFunc        getter;
	ComboGetterFunc        getterDefault;
	UIComboBox *           combo;
	SwgCuiOptBase *        base;
	int                    revertData;

	//----------------------------------------------------------------------

	ComboBoxData (const ComboBoxData & rhs) :
	setter          (rhs.setter),
	getter          (rhs.getter),
	getterDefault   (rhs.getterDefault),
	combo           (rhs.combo),
	base            (rhs.base),
	revertData      (rhs.revertData)
	{
		if (combo)
		{
			combo->Attach (0);
			combo->AddCallback (this);
		}
	}

	//----------------------------------------------------------------------

	ComboBoxData & operator= (const ComboBoxData & rhs)
	{
		if (this == &rhs)
			return *this;
		
		if (rhs.combo)
			rhs.combo->Attach (0);
		
		if (combo)
		{
			combo->Detach (0);
			combo->RemoveCallback (this);
		}
		
		setter          = rhs.setter;
		getter          = rhs.getter;
		getterDefault   = rhs.getterDefault;
		combo           = rhs.combo;
		base            = rhs.base;
		revertData      = rhs.revertData;
		
		if (combo)
			combo->AddCallback (this);
		
		return *this;
	}

	//----------------------------------------------------------------------

	ComboBoxData (SwgCuiOptBase & _base, UIComboBox & _combo, ComboSetterFunc  _setter, ComboGetterFunc   _getter, ComboGetterFunc   _getterDefault) :
	setter          (_setter),
	getter          (_getter),
	getterDefault   (_getterDefault),
	combo           (&_combo),
	base            (&_base),
	revertData      (false)
	{
		WARNING (WARN_NO_DEFAULT && !_getterDefault, ("SwgCuiOptBase no default [%s]", _combo.GetFullPath ().c_str ()));

		combo->Attach (0);
		combo->AddCallback (this);
	}

	//----------------------------------------------------------------------

	~ComboBoxData ()
	{
		combo->RemoveCallback (this);
		combo->Detach (0);
	}

	//----------------------------------------------------------------------

	void revert ()
	{
		const int value  = revertData;
		
		combo->SetSelectedIndex (value, true);

		if (setter)
			setter (*base, *combo, value);
	}

	//----------------------------------------------------------------------

	void updateControl (bool storeRevertData = false)
	{
		if (getter)
		{
			const int value     = getter     (*base, *combo);
			combo->SetSelectedIndex (value, true);

			if (storeRevertData)
				revertData = value;
		}
	}

	//----------------------------------------------------------------------

	void updateFromControl ()
	{
		const int value  = combo->GetSelectedIndex ();
		
		if (setter)
			setter (*base, *combo, value);
	}

	//----------------------------------------------------------------------

	void resetDefault ()
	{
		if (setter && getterDefault)
			setter (*base, *combo, getterDefault (*base, *combo));

		updateControl (false);
	}

	//----------------------------------------------------------------------

	void OnGenericSelectionChanged (UIWidget * context)
	{
		if (base->isActive () && context == combo && !m_ignoreNotification)
			updateFromControl ();
	}
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

SwgCuiOptBase::SwgCuiOptBase     (const char * const name, UIPage & page) :
CuiMediator           (name, page),
UIEventCallback       (),
m_buttonResetDefaults (0),
m_controlDataBaseMap  (new ControlDataBaseMap),
m_callback            (new MessageDispatch::Callback),
m_messageBoxConfirmResetDefaults (0),
m_pageName                       ()
{
	getCodeDataObject (TUIButton, m_buttonResetDefaults, "buttonDefaults", true);

	if (m_buttonResetDefaults)
		registerMediatorObject (*m_buttonResetDefaults, true);

	const UIData * const codeData = NON_NULL (getCodeData ());

	static const UILowerString prop_PageName = UILowerString ("PageName");
	std::string encodedPageName;
	if (!codeData->GetPropertyNarrow (prop_PageName, encodedPageName))
		WARNING (true, ("SwgCuiOptBase no such property %s", prop_PageName.c_str ()));
	else
		StringId  (encodedPageName).localize (m_pageName);
}

//----------------------------------------------------------------------

SwgCuiOptBase::~SwgCuiOptBase ()
{
	std::for_each (m_controlDataBaseMap->begin (), m_controlDataBaseMap->end (), PointerDeleterPairSecond ());
	delete m_controlDataBaseMap;

	delete m_callback;
	m_callback = 0;

	m_controlDataBaseMap    = 0;
}

//----------------------------------------------------------------------

void SwgCuiOptBase::performActivate   ()
{
	for (ControlDataBaseMap::iterator it = m_controlDataBaseMap->begin (); it != m_controlDataBaseMap->end (); ++it)
	{
		ControlDataBase * const data    = NON_NULL ((*it).second);
		data->updateControl (false);
	}

	queryWidgetValues();
}

//----------------------------------------------------------------------

void SwgCuiOptBase::performDeactivate ()
{
}

//----------------------------------------------------------------------

void SwgCuiOptBase::revert            ()
{
	for (ControlDataBaseMap::iterator it = m_controlDataBaseMap->begin (); it != m_controlDataBaseMap->end (); ++it)
	{
		ControlDataBase * const data    = NON_NULL ((*it).second);
		data->revert ();
	}
}

//----------------------------------------------------------------------

void SwgCuiOptBase::storeRevertData   ()
{
	for (ControlDataBaseMap::iterator it = m_controlDataBaseMap->begin (); it != m_controlDataBaseMap->end (); ++it)
	{
		ControlDataBase * const data    = NON_NULL ((*it).second);
		data->updateControl (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptBase::resetDefaults     (bool confirmed)
{
	if (!confirmed)
	{
		if (m_messageBoxConfirmResetDefaults)
		{
			m_messageBoxConfirmResetDefaults->closeMessageBox ();
			m_messageBoxConfirmResetDefaults = 0;
		}

		Unicode::String result;
		CuiStringVariablesManager::process (CuiStringIdsOptions::confirm_reset_defaults_prose, Unicode::emptyString, m_pageName, Unicode::emptyString, result);
		m_messageBoxConfirmResetDefaults = CuiMessageBox::createYesNoBox (result);
		m_callback->connect (m_messageBoxConfirmResetDefaults->getTransceiverClosed (), *this, &SwgCuiOptBase::onConfirmResetDefaultsClosed);
		return;
	}

	for (ControlDataBaseMap::iterator it = m_controlDataBaseMap->begin (); it != m_controlDataBaseMap->end (); ++it)
	{
		ControlDataBase * const data    = NON_NULL ((*it).second);
		data->resetDefault ();
	}
}

//----------------------------------------------------------------------

void SwgCuiOptBase::OnButtonPressed   (UIWidget * context)
{
	if (context == m_buttonResetDefaults)
	{
		resetDefaults (false);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptBase::onConfirmResetDefaultsClosed (const CuiMessageBox & box)
{
	if (&box == m_messageBoxConfirmResetDefaults)
	{
		m_messageBoxConfirmResetDefaults = 0;

		if (box.completedAffirmative ())
			resetDefaults (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerSlider   (UISliderbar & slider, StaticSetterInt   setter, StaticGetterInt   getter, StaticGetterInt   getterDefault, int start,   int endAllowed, int end, SliderFuncInt func,   SliderFuncRaw funcRaw)
{
	const ControlDataBaseMap::iterator it = m_controlDataBaseMap->find (&slider);
	if (it != m_controlDataBaseMap->end ())
		DEBUG_FATAL (true, ("SwgCuiOptBase slider [%s] already registered", slider.GetFullPath ().c_str ()));

	SliderData * const data = new SliderData (*this, slider, setter, getter, getterDefault, start, endAllowed, end, func, funcRaw);

	m_controlDataBaseMap->insert (std::make_pair (&slider, data));
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerSlider   (UISliderbar & slider, StaticSetterFloat setter, StaticGetterFloat getter, StaticGetterFloat getterDefault, float start, float endAllowed, float end, SliderFuncFloat func, SliderFuncRaw funcRaw)
{
	const ControlDataBaseMap::iterator it = m_controlDataBaseMap->find (&slider);
	if (it != m_controlDataBaseMap->end ())
		DEBUG_FATAL (true, ("SwgCuiOptBase slider [%s] already registered", slider.GetFullPath ().c_str ()));

	SliderData * const data = new SliderData (*this, slider, setter, getter, getterDefault, start, endAllowed, end, func, funcRaw);

	m_controlDataBaseMap->insert (std::make_pair (&slider, data));
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerCheckbox (UICheckbox & checkbox, StaticSetterBool  setter, StaticGetterBool  getter, StaticGetterBool  getterDefault, CheckboxFunc func)
{
	const ControlDataBaseMap::iterator it = m_controlDataBaseMap->find (&checkbox);
	if (it != m_controlDataBaseMap->end ())
		DEBUG_FATAL (true, ("SwgCuiOptBase checkbox [%s] already registered", checkbox.GetFullPath ().c_str ()));

	CheckboxData * const data = new CheckboxData (*this, checkbox, setter, getter, getterDefault, func);

	m_controlDataBaseMap->insert (std::make_pair (&checkbox, data));
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerComboBox (UIComboBox & combo, ComboSetterFunc  setter, ComboGetterFunc  getter, ComboGetterFunc  getterDefault)
{
	const ControlDataBaseMap::iterator it = m_controlDataBaseMap->find (&combo);
	if (it != m_controlDataBaseMap->end ())
		DEBUG_FATAL (true, ("SwgCuiOptBase combo [%s] already registered", combo.GetFullPath ().c_str ()));

	ComboBoxData * const data = new ComboBoxData (*this, combo, setter, getter, getterDefault);

	m_controlDataBaseMap->insert (std::make_pair (&combo, data));
}

//----------------------------------------------------------------------

void SwgCuiOptBase::queryWidgetValues ()
{
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerSlider   (UISliderbar & slider, StaticSetterInt   setter, StaticGetterInt   getter, StaticGetterInt   getterDefault, int start,   int end,   SliderFuncInt memberFunc,   SliderFuncRaw memberFuncRaw )
{
	registerSlider (slider, setter, getter, getterDefault, start, end, end, memberFunc, memberFuncRaw);
}

//----------------------------------------------------------------------

void SwgCuiOptBase::registerSlider   (UISliderbar & slider, StaticSetterFloat setter, StaticGetterFloat getter, StaticGetterFloat getterDefault, float start, float end, SliderFuncFloat memberFunc, SliderFuncRaw memberFuncRaw)
{
	registerSlider (slider, setter, getter, getterDefault, start, end, end, memberFunc, memberFuncRaw);
}

//======================================================================
