// ======================================================================
//
// PropertyKeySequenceItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyKeySequenceItem_H
#define INCLUDED_PropertyKeySequenceItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QEvent;
class QHBox;
class QKeyEvent;
class QLineEdit;

// ----------------------------------------------------------------------

class PropertyKeySequenceItem : public QObject, public PropertyItem
{
    Q_OBJECT

public:
    PropertyKeySequenceItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName);
    ~PropertyKeySequenceItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue(const QVariant &v);

private slots:
    void setValue();

private:
    bool eventFilter(QObject *o, QEvent *e);
    void handleKeyEvent(QKeyEvent *e);
    int translateModifiers(int state);

    QGuardedPtr<QLineEdit> sequence;
    QGuardedPtr<QHBox> box;
    int k1,k2,k3,k4;
    int num;
    bool mouseEnter;
};

// ======================================================================

#endif // INCLUDED_PropertyKeySequenceItem_H
