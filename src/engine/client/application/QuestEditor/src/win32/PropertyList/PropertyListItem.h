// ======================================================================
//
// PropertyListItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyListItem_H
#define INCLUDED_PropertyListItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

#include <qguardedptr.h>

// ----------------------------------------------------------------------

class PropertyList;
class QComboBox;

// ----------------------------------------------------------------------

class PropertyListItem : public QObject, public PropertyItem
{
    Q_OBJECT

public:
    PropertyListItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, bool editable);
    ~PropertyListItem();

    virtual void showEditor();
    virtual void hideEditor();
    virtual void setValue(const QVariant &v);

    QString currentItem() const;
    int currentIntItem() const;
    void setCurrentItem(const QString &s);
    void setCurrentItem(int i);
    int currentIntItemFromObject() const;
    QString currentItemFromObject() const;
    void addItem(const QString &s);

private slots:
    void setValue();

private:
    QComboBox * combo();

private:
    QGuardedPtr<QComboBox> m_comboBox;
    int m_oldInt;
    bool m_editable;
    QString m_oldString;
	QStringList m_comboStringList;
};

// ======================================================================

#endif // INCLUDED_PropertyListItem_H
