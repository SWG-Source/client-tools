// ======================================================================
//
// SwgCuiOptBase.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiOptBase_H
#define INCLUDED_SwgCuiOptBase_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UISliderbar;
class UICheckbox;
class UIComboBox;
class CuiMessageBox;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiOptBase :
public CuiMediator,
public UIEventCallback
{
public:

	                         SwgCuiOptBase     (const char * const name, UIPage & page);

	//- PS UI support
	virtual void             OnButtonPressed    (UIWidget * context);

	virtual void             revert            ();
	virtual void             storeRevertData   ();

	virtual void             resetDefaults     (bool confirmed);

	void                     onConfirmResetDefaultsClosed (const CuiMessageBox & box);
	virtual void             queryWidgetValues ();

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	typedef void (*CheckboxFunc)         (const SwgCuiOptBase & base, const UICheckbox & box,     bool value);
	typedef void (*SliderFuncRaw)        (const SwgCuiOptBase & base, const UISliderbar & slider);
	typedef void (*SliderFuncFloat)      (const SwgCuiOptBase & base, const UISliderbar & slider, float);
	typedef void (*SliderFuncInt)        (const SwgCuiOptBase & base, const UISliderbar & slider, int);
	typedef void (*ComboSetterFunc)      (const SwgCuiOptBase & base, const UIComboBox & combo, int index);
	typedef int  (*ComboGetterFunc)      (const SwgCuiOptBase & base, const UIComboBox & combo);

	typedef void (*StaticSetterInt)                        (int);
	typedef void (*StaticSetterFloat)                      (float);
	typedef void (*StaticSetterBool)                       (bool);

	typedef int   (*StaticGetterInt)                       ();
	typedef float (*StaticGetterFloat)                     ();
	typedef bool  (*StaticGetterBool)                      ();

	void registerSlider   (UISliderbar & slider, StaticSetterInt   setter, StaticGetterInt   getter, StaticGetterInt   getterDefault, int start,   int endAllowed, int end,   SliderFuncInt memberFunc = 0,   SliderFuncRaw memberFuncRaw = 0);
	void registerSlider   (UISliderbar & slider, StaticSetterFloat setter, StaticGetterFloat getter, StaticGetterFloat getterDefault, float start, float endAllowed, float end, SliderFuncFloat memberFunc = 0, SliderFuncRaw memberFuncRaw = 0);

	void registerSlider   (UISliderbar & slider, StaticSetterInt   setter, StaticGetterInt   getter, StaticGetterInt   getterDefault, int start,   int end,   SliderFuncInt memberFunc = 0,   SliderFuncRaw memberFuncRaw = 0);
	void registerSlider   (UISliderbar & slider, StaticSetterFloat setter, StaticGetterFloat getter, StaticGetterFloat getterDefault, float start, float end, SliderFuncFloat memberFunc = 0, SliderFuncRaw memberFuncRaw = 0);

	void registerCheckbox (UICheckbox & checkbox, StaticSetterBool  setter, StaticGetterBool  getter, StaticGetterBool  getterDefault, CheckboxFunc memberFunc = 0);

	void registerComboBox (UIComboBox & checkbox, ComboSetterFunc  setter, ComboGetterFunc  getter, ComboGetterFunc  getterDefault);

	virtual                 ~SwgCuiOptBase ();

	static bool  getTrue  ()  { return true; }
	static bool  getFalse ()  { return false; }
	static float getOne   ()  { return 1.0f; }
	static float getZero  ()  { return 0.0f; }

private:
	                         SwgCuiOptBase ();
	                         SwgCuiOptBase (const SwgCuiOptBase & rhs);
	SwgCuiOptBase &          operator=     (const SwgCuiOptBase & rhs);

protected:

	class ControlDataBase;
	class SliderData;
	class CheckboxData;
	class ComboBoxData;

	typedef stdmap<UIWidget *, ControlDataBase *>::fwd   ControlDataBaseMap;

	UIButton *                  m_buttonResetDefaults;
	ControlDataBaseMap *        m_controlDataBaseMap;

	MessageDispatch::Callback * m_callback;
	CuiMessageBox *             m_messageBoxConfirmResetDefaults;

	Unicode::String             m_pageName;
};

// ======================================================================

#endif
