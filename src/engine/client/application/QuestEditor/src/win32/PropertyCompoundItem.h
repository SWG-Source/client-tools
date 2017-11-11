// ======================================================================
//
// PropertyCompoundItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyCompoundItem_H
#define INCLUDED_PropertyCompoundItem_H

// ----------------------------------------------------------------------

#include "PropertyList/PropertyItem.h"

// ----------------------------------------------------------------------

#include <qdom.h>

// ----------------------------------------------------------------------

class QLineEdit;

// ----------------------------------------------------------------------

class PropertyCompoundItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyCompoundItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, QDomElement const & configInputElement);
	~PropertyCompoundItem();

	virtual void createChildren();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual bool hasSubItems() const;
	virtual void childValueChanged( PropertyItem *child );
	virtual void notifyValueChange();

protected:
	void PropertyCompoundItem::setConcatenatedValue();

private:
	QLineEdit *lined();
	QGuardedPtr<QLineEdit> m_lin;
	QDomElement m_configInputElement;
};

// ======================================================================

#endif // INCLUDED_PropertyCompoundItem_H
