// ======================================================================
//
// PropertyEnumItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyEnumItem_H
#define INCLUDED_PropertyEnumItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qptrlist.h>
#include <qvaluelist.h>

// ----------------------------------------------------------------------

class QVBoxLayout;

// ----------------------------------------------------------------------

struct EnumItem
{
    EnumItem(const QString &k, bool s);
    EnumItem();
    bool operator==(const EnumItem &item) const;
    QString key;
    bool selected;
};

// ----------------------------------------------------------------------

inline bool EnumItem::operator==(const EnumItem &item) const
{
	return key == item.key;
}

// ----------------------------------------------------------------------

class EnumPopup : public QFrame
{
    Q_OBJECT

public:
    EnumPopup(QWidget *parent, const char *name, WFlags f=0);
    ~EnumPopup();
    void insertEnums(QValueList<EnumItem> lst);
    QValueList<EnumItem> enumList() const;
    void closeWidget();

signals:
    void closed();
    void hidden();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    QValueList<EnumItem> itemList;
    QPtrList<QCheckBox> checkBoxList;
    QVBoxLayout *popLayout;
};

// ----------------------------------------------------------------------

class EnumBox : public QComboBox
{
    Q_OBJECT

public:
    EnumBox(QWidget *parent, const char *name = 0);
    ~EnumBox() {}
    void setText(const QString &text);
    void insertEnums(QValueList<EnumItem> lst);
    QValueList<EnumItem> enumList() const;

signals:
    void aboutToShowPopup();
    void valueChanged();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);

protected slots:
    void restoreArrow();
    void popupHidden();
    void popupClosed();

private:
    void popup();
    bool arrowDown;
    QString str;
    bool popupShown;
    EnumPopup *pop;
};

// ----------------------------------------------------------------------

class PropertyEnumItem : public QObject, public PropertyItem
{
    Q_OBJECT

public:
    PropertyEnumItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
    ~PropertyEnumItem();

    void showEditor();
    void hideEditor();
    void setValue(const QVariant &v);
    QString currentItem() const;
    QString currentItemFromObject() const;
    void setCurrentValues(QStrList lst);

private slots:
    void setValue();
    void insertEnums();

private:
    QGuardedPtr<EnumBox> box;
    QValueList<EnumItem> enumList;
    QString enumString;
};

// ======================================================================

#endif // INCLUDED_PropertyEnumItem_H
