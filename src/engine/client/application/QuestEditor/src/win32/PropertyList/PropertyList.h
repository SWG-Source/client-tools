// ======================================================================
//
// PropertyList.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyList_H
#define INCLUDED_PropertyList_H

// ----------------------------------------------------------------------

#include <qlistview.h>
#include <qvariant.h>

// ----------------------------------------------------------------------

class PropertyItem;
class PropertyListItem;

// ----------------------------------------------------------------------

//-- Help constants
char const * const cs_whatsThisTextFormat = "<p><b>%1</b></p><p>%2</p>";
char const * const cs_defaultInfoText = "No information available.";

// ----------------------------------------------------------------------

class PropertyList : public QListView
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

	class PropertyWhatsThis;

public:
	explicit PropertyList(QWidget * propertListParent);
	
	virtual void setupProperties();
	
	virtual void setCurrentItem(QListViewItem *i);
	virtual void valueChanged(PropertyItem *i) = 0;
	virtual void refetchData();
	virtual void setPropertyValue(PropertyItem *i);
	virtual void setCurrentProperty(const QString &n, bool useValueName);
	
	void layoutInitValue(PropertyItem *i, bool changed = FALSE);
	QString whatsThisAt(const QPoint &p);
	void showCurrentWhatsThis();
	
	enum LastEventType { KeyEvent, MouseEvent };
	LastEventType lastEvent();
	
	virtual void clear();
	virtual void setup();
	virtual QVariant getResetValue(PropertyItem *item) = 0;
	
	bool eventFilter(QObject *o, QEvent *e);

	PropertyItem * getCurrentPropertyItem();
	
public slots:
	void updateEditorSize();
	void resetProperty();
	void toggleSort();
	
private slots:
	void itemPressed(QListViewItem *i, const QPoint &p, int c);
	void toggleOpen(QListViewItem *i);
	
protected:
	void resizeEvent(QResizeEvent *e);
	void paintEmptyArea(QPainter *p, const QRect &r);
	bool addPropertyItem(PropertyItem *&item, const QCString &name, QVariant::Type t);
	
	void viewportDragEnterEvent(QDragEnterEvent *);
	void viewportDragMoveEvent (QDragMoveEvent *);
	void viewportDropEvent (QDropEvent *);
	
private:
	void readPropertyDocs();
	QString whatsThisText(QListViewItem *i);
	
private:
	PropertyListItem* pressItem;
	QPoint pressPos;
	bool mousePressed;
	bool showSorted;
	QMap<QString, QString> propertyDocs;
	LastEventType theLastEvent;
	PropertyWhatsThis *whatsThis;

private: //-- disabled
	PropertyList();
	PropertyList(PropertyList const &);
	PropertyList & operator=(PropertyList const &);
};

// ======================================================================

#endif // INCLUDED_PropertyList_H
