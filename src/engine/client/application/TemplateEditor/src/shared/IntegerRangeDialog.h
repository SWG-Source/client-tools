// ============================================================================
//
// IntegerRangeDialog.h
//
// Dialog that lets user set a range between a minimum and a maximum integer 
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_IntegerRangeDialog_H
#define INCLUDED_IntegerRangeDialog_H

#include "BaseIntegerRangeDialog.h"

//-----------------------------------------------------------------------------
class IntegerRangeDialog : public BaseIntegerRangeDialog
{
	Q_OBJECT

public:

	IntegerRangeDialog(QWidget *parent, char const *name, int const limitMin, int const limitMax);

	void         getValue(int &valueMin, int &valueMax) const;
	void         setValue(int const valueMin, int const valueMax);
	bool         isLimitMin();
	bool         isLimitMax();
	virtual void show();

private slots:

	void slotAcceptButtonClicked();
	void slotCancelButtonClicked();
	void slotMinLimitCheckBoxClicked();
	void slotMaxLimitCheckBoxClicked();

private:

	int getMin() const;
	int getMax() const;

	int  m_finalValueMin;
	int  m_finalValueMax;
	bool m_finalLimitMin;
	bool m_finalLimitMax;
};

// ============================================================================

#endif // INCLUDED_IntegerRangeDialog_H
