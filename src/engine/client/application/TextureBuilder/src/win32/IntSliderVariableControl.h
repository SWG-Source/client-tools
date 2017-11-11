// ======================================================================
//
// IntSliderVariableControl.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_IntSliderVariableControl_h
#define INCLUDED_IntSliderVariableControl_h

// ======================================================================

#include "VariableControl.h"

class CEdit;
class CSliderCtrl;
class CStatic;
class IntVariableElement;

// ======================================================================

class IntSliderVariableControl: public VariableControl
{
public:

	IntSliderVariableControl(CWnd *parentWindow, IntVariableElement &variableElement);
	virtual ~IntSliderVariableControl();

	virtual int         setupDimensions(int startX, int startY, int endX);
	virtual bool        saveControlToVariable(const CWnd *actionSource);
	virtual void        loadControlFromVariable();

private:

	IntVariableElement &getIntVariableElement();

	void                setEditValue(int value);
	int                 getEditValue() const;

private:

	CSliderCtrl *m_sliderControl;
	CEdit       *m_editControl;
	CStatic     *m_variableNameControl;

	bool         m_ignoreNextEditChange;

private:
	// disabled
	IntSliderVariableControl();
	IntSliderVariableControl(const IntSliderVariableControl&);
	IntSliderVariableControl &operator =(const IntSliderVariableControl&);
};

// ======================================================================

#endif
