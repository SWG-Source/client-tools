// ======================================================================
//
// LocListWidget.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocListWidget_H
#define INCLUDED_LocListWidget_H

#include "ui_ListWidget.h"
// todo remove the listener interface from this header
#include "DataChangeListener.h"

class LocEditingWidget;
class QPixmap;
class QFont;

// ======================================================================

class LocListWidget : public ListWidget, public DataChangeListener
{
private:

	Q_OBJECT

public:

		                      LocListWidget (LocEditingWidget *, QWidget * parent);
	                         ~LocListWidget ();
	void                      updateData ();
	void                      dataChanged ();
	void                      currentEditingChanged ();
	void                      readOnlyChanged ();
	void                      deleteSelection ();

	const QFont               getListFont () const;
	void                      setListFont (const QFont &);

public slots:
  virtual void                startEditingItem(QListViewItem *);

private:
	                          LocListWidget ();
	                          LocListWidget (const LocListWidget & rhs);
	LocListWidget &           operator=    (const LocListWidget & rhs);


	void                      fitColumnWidths ();

protected:
	void                      resizeEvent (QResizeEvent *);

private:

	LocEditingWidget *    m_editingWidget;
	QPixmap *             m_outOfDatePixmap;
	QPixmap *             m_modifiedPixmap;
	bool                  m_ignoreDataChange;
};

// ======================================================================

#endif
