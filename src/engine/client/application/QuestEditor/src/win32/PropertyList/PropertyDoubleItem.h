// ======================================================================
//
// PropertyDoubleItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyDoubleItem_H
#define INCLUDED_PropertyDoubleItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QDoubleValidator;
class QLineEdit;

// ----------------------------------------------------------------------

class PropertyDoubleItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyDoubleItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName);
	~PropertyDoubleItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

	void setMinValue(float minValue);
	void setMaxValue(float maxValue);

private slots:
	void setValue();
	void updateBackground();

private:
	QLineEdit *lined();
	QGuardedPtr<QLineEdit> lin;
	QDoubleValidator * m_doubleValidator;
	double m_minValue;
	double m_maxValue;
};

// ======================================================================

#endif // INCLUDED_PropertyDoubleItem_H
