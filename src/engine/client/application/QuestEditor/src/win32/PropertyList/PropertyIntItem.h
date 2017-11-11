// ======================================================================
//
// PropertyIntItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyIntItem_H
#define INCLUDED_PropertyIntItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

#include <qspinbox.h>

// ----------------------------------------------------------------------

class PropertyIntItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyIntItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, bool s);
	~PropertyIntItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

	void setMinValue(int minValue);
	void setMaxValue(int maxValue);

private slots:
	void setValue();

private:
	QSpinBox *spinBox();
	QGuardedPtr<QSpinBox> spinBx;
	bool signedValue;
	int m_minValue;
	int m_maxValue;
};

// ======================================================================

#endif // INCLUDED_PropertyIntItem_H
