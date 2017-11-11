// ======================================================================
//
// PropertyItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyItem_H
#define INCLUDED_PropertyItem_H

// ----------------------------------------------------------------------

#include <qguardedptr.h>
#include <qlistview.h>
#include <qvariant.h>

// ----------------------------------------------------------------------

class PropertyList;
class QPushButton;

// ----------------------------------------------------------------------

class PropertyItem : public QListViewItem
{
public:
	PropertyItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyItem();
	
	void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
	void paintBranches(QPainter * p, const QColorGroup & cg,
		int w, int y, int h);
	void paintFocus(QPainter *p, const QColorGroup &cg, const QRect &r);
	
	virtual bool hasSubItems() const;
	virtual void createChildren();
	virtual void initChildren();
	virtual void removeChildren();
	
	bool isOpen() const; //lint !e1511
	void setOpen(bool b);
	
	virtual void showEditor();
	virtual void hideEditor();
	
	virtual void setValue(const QVariant &v);
	QVariant value() const;
	QString name() const;
	virtual void notifyValueChange();
	
	virtual void setChanged(bool b, bool updateDb = TRUE); //lint !e1735
	bool isChanged() const;
	
	virtual void placeEditor(QWidget *w);
	
	virtual PropertyItem *propertyParent() const;
	virtual void childValueChanged(PropertyItem *child);
	
	void addChild(PropertyItem *i);
	int childCount() const; //lint !e1511
	PropertyItem *child(int i) const;
	
	virtual bool hasCustomContents() const;
	virtual void drawCustomContents(QPainter *p, const QRect &r);
	
	void updateBackColor();
	
	void setup() { QListViewItem::setup(); setHeight(QListViewItem::height() + 2); }
	
	virtual QString currentItem() const;
	virtual int currentIntItem() const;
	virtual void setCurrentItem(const QString &s);
	virtual void setCurrentItem(int i);
	virtual int currentIntItemFromObject() const;
	virtual QString currentItemFromObject() const;
	
	void setFocus(QWidget *w);
	
	virtual void toggle();
	void setText(int col, const QString &txt);

	QString const & getValueName() const;
	void setValueName(QString const & newValueName);

	QString const & getWhatsThisText() const;
	void setWhatsThisText(QString const & newWhatsThisText);

protected:
	PropertyList *listview;
	QVariant val;
	
private:
	QColor backgroundColor();
	void createResetButton();
	void updateResetButtonState();
	
private:
	bool open, changed; //lint !e1516
	PropertyItem *property;
	QString propertyName;
	QString valueName;
	QString whatsThisText;
	QPtrList<PropertyItem> children;
	QColor backColor;
	QPushButton *resetButton;
	
private: //-- disabled
	PropertyItem();
};

// ----------------------------------------------------------------------

inline QString const & PropertyItem::getValueName() const
{
	return valueName;
}

// ----------------------------------------------------------------------

inline void PropertyItem::setValueName(QString const & newValueName)
{
	valueName = newValueName;
}


// ----------------------------------------------------------------------

inline QString const & PropertyItem::getWhatsThisText() const
{
	return whatsThisText;
}

// ----------------------------------------------------------------------

inline void PropertyItem::setWhatsThisText(QString const & newWhatsThisText)
{
	whatsThisText = newWhatsThisText;
}

// ======================================================================

#endif // INCLUDED_PropertyItem_H
