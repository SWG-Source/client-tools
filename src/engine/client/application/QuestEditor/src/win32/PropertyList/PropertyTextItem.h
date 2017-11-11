// ======================================================================
//
// PropertyTextItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyTextItem_H
#define INCLUDED_PropertyTextItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QHBox;
class QPushButton;
class QValidator;

// ----------------------------------------------------------------------

class PropertyTextItem : public QObject, public PropertyItem
{
    Q_OBJECT

public:
    PropertyTextItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, bool comment, bool multiLine, bool a = FALSE);
    ~PropertyTextItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue(const QVariant &v);
    virtual bool hasSubItems() const;
    virtual void childValueChanged(PropertyItem *child);

    virtual void setChanged(bool b, bool updateDb = TRUE);
	virtual void setValidator(QValidator const * validator);

private slots:
    void setValue();
    void getText();

private:
    QLineEdit *lined();
    QGuardedPtr<QLineEdit> lin;
    QGuardedPtr<QHBox> box;
    QPushButton *button;
    bool withComment, hasMultiLines, accel;
};

// ======================================================================

#endif // INCLUDED_PropertyTextItem_H
