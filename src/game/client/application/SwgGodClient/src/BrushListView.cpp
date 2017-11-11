// ======================================================================
//
// BrushListView.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "BrushListView.h"
#include "BrushListView.moc"

#include "ActionHack.h"
#include "ActionsScript.h"
#include "ActionsTool.h"
#include "BrushData.h"

#include <qdragobject.h>
#include <qpopupmenu.h>

//-----------------------------------------------------------------

BrushListView::BrushListView (QWidget* theParent, const char* theName)
: QListView(theParent, theName)
{
	IGNORE_RETURN(QListView::addColumn ("Brush"));
	QListView::setResizeMode (QListView::AllColumns);
	QListView::setRootIsDecorated (true);

	IGNORE_RETURN (connect(this, SIGNAL (contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(onContextMenuRequested(QListViewItem*, const QPoint&, int))));
	IGNORE_RETURN (connect(this, SIGNAL (selectionChanged()), this, SLOT(onSelectionChanged())));
}

//-----------------------------------------------------------------

QDragObject * BrushListView::dragObject()
{
	return new QTextDrag(BrushData::Messages::BRUSH_DRAGGED, this, "menu");
}

//-----------------------------------------------------------------

void BrushListView::onSelectionChanged() const
{
}

//-----------------------------------------------------------------

void BrushListView::onRefreshList() const
{
}

//-----------------------------------------------------------------

void BrushListView::onContextMenuRequested(QListViewItem* item, const QPoint& p, int)
{
	//only show the popup if clicking on an item
	if (item == 0)
		return;

	QPopupMenu* const m_pop = new QPopupMenu (this, "menu");

	ActionsTool* at =&ActionsTool::getInstance();

	IGNORE_RETURN(at->m_pasteBrushHere->addTo(m_pop));

	m_pop->popup(p);
} //lint !e818 item "could" be const, but Qt allows us to change it

// ======================================================================
