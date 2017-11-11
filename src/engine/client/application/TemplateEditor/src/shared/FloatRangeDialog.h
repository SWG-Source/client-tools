// ============================================================================
//
// FloatRangeDialog.h
//
// Dialog that lets user set a range between a minimum and a maximum float 
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FloatRangeDialog_H
#define INCLUDED_FloatRangeDialog_H

#include "BaseFloatRangeDialog.h"

//-----------------------------------------------------------------------------
class FloatRangeDialog : public BaseFloatRangeDialog
{
	Q_OBJECT

public:

	FloatRangeDialog(QWidget *parent, char const *name, float limitMin, float limitMax);

	void         setValue(float const valueMin, float const valueMax);
	void         getValue(float &valueMin, float &valueMax) const;
	bool         isLimitMin();
	bool         isLimitMax();
	virtual void show();

private slots:

	void slotAcceptButtonClicked();
	void slotCancelButtonClicked();
	void slotMinLimitCheckBoxClicked();
	void slotMaxLimitCheckBoxClicked();

private:

	float getMin() const;
	float getMax() const;
	bool  rangeValidated() const;
	bool  validate();

	float m_finalValueMin;
	float m_finalValueMax;
	bool  m_finalLimitMin;
	bool  m_finalLimitMax;
	float m_limitMin;
	float m_limitMax;
};

// ============================================================================

#endif // INCLUDED_FloatRangeDialog_H
