// ======================================================================
//
// TreeListView.cpp
// Copyright 2004, Sony Online Entertainment Inc.
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList/PropertyList.h"
#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"
#include "TreeItem.h"
#include "TreeListView.h"

// ----------------------------------------------------------------------

#include <qcursor.h>
#include <qdragobject.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------

#include "TreeListView.moc"

// ----------------------------------------------------------------------

class TreeListView::TreeListViewWhatsThis : public QWhatsThis
{
public:
	TreeListViewWhatsThis(TreeListView *l);
	QString text(const QPoint &pos);

private:
	TreeListView * m_treeListView;

private: //-- disabled
	TreeListViewWhatsThis();
	TreeListViewWhatsThis(TreeListViewWhatsThis const &rhs);
};

// ----------------------------------------------------------------------

TreeListView::TreeListView(QWidget *parentTreeListView, const char *nameTreeListView)
: QListView(parentTreeListView, nameTreeListView)
, m_dragging(false)
, m_pressPos()
{
	QListView::setAcceptDrops(true);

	m_itemWhatsThis = new TreeListViewWhatsThis(this);
}

// ----------------------------------------------------------------------

TreeListView::~TreeListView()
{
}

// ----------------------------------------------------------------------

void TreeListView::dragEnterEvent(QDragEnterEvent *dragEvent)
{
	UNREF(dragEvent);
	DEBUG_REPORT_LOG(false, ("TreeListView::dragEnterEvent()\n"));

	if (dragEvent->provides("text/nodeid"))
		dragEvent->accept();
}

// ----------------------------------------------------------------------

void TreeListView::dropEvent(QDropEvent *eventDrop)
{
	if (!eventDrop->provides("text/nodeid"))
		return;

	QString sourceId;

	if (QTextDrag::decode(eventDrop, sourceId))
	{
		TreeItem *after = dynamic_cast<TreeItem *>(itemAt(viewport()->mapFromParent(eventDrop->pos())));

		if (!after)
			return;

		QString destinationId = after->getNode().toElement().attribute(cs_Id);

		if (sourceId != destinationId)
			emit nodeDropped(sourceId, destinationId);
	}
}

// ----------------------------------------------------------------------

void TreeListView::contentsMousePressEvent(QMouseEvent *mouseEvent)
{
	UNREF(mouseEvent);
	DEBUG_REPORT_LOG(false, ("TreeListView::contentsMousePressEvent()\n"));

	QListView::contentsMousePressEvent(mouseEvent);

	m_dragging = true;
	m_pressPos = mouseEvent->pos();
}

// ----------------------------------------------------------------------

void TreeListView::contentsMouseMoveEvent(QMouseEvent *mouseEvent)
{
	UNREF(mouseEvent);
	DEBUG_REPORT_LOG(false, ("TreeListView::contentsMouseMoveEvent()\n"));

	QListView::contentsMouseMoveEvent(mouseEvent);

	if (!m_dragging)
		return;

	if (!currentItem())
		return;

	if ((m_pressPos - mouseEvent->pos()).manhattanLength() > QApplication::startDragDistance())
	{
		TreeItem * item = dynamic_cast<TreeItem *>(currentItem());

		if (!item)
			return;

		QDomElement element = item->getNode().toElement();

		if (element.isNull() || !element.hasAttribute(cs_Id))
			return;

		QTextDrag *drg = new QTextDrag(element.attribute(cs_Id), this);

		const QPixmap *p = currentItem()->pixmap(0);

		if (p)
			drg->setPixmap(*p);

		drg->setSubtype("nodeid");
		drg->dragCopy();
		m_dragging = false;
	}//lint !e429
}

// ----------------------------------------------------------------------

void TreeListView::contentsMouseReleaseEvent(QMouseEvent *mouseEvent)
{
	UNREF(mouseEvent);
	DEBUG_REPORT_LOG(false, ("TreeListView::contentsMouseReleaseEvent()\n"));

	QListView::contentsMouseReleaseEvent(mouseEvent);
	m_dragging = false;
}

// ----------------------------------------------------------------------

QString TreeListView::whatsThisAt(const QPoint &p)
{
	return whatsThisText(itemAt(p));
}

// ----------------------------------------------------------------------

QString TreeListView::whatsThisText(QListViewItem *i)
{
	TreeItem * treeItem = safe_cast<TreeItem *>(i);

	if (!treeItem)
		return QWhatsThis::textFor(this);

	QString const type = treeItem->getNode().toElement().attribute(cs_Type);
	QDomElement task = QuestEditorConfig::getEditorTaskNode(type).toElement();
	QString const info = task.attribute(cs_Info, cs_defaultInfoText);

	return QString(cs_whatsThisTextFormat).arg(type).arg(info);
}

// ======================================================================

TreeListView::TreeListViewWhatsThis::TreeListViewWhatsThis(TreeListView *l)
: QWhatsThis(l->viewport())
, m_treeListView(l)
{
}

// ----------------------------------------------------------------------

QString TreeListView::TreeListViewWhatsThis::text(const QPoint &pos)
{
	return m_treeListView->whatsThisAt(pos);
}

// ======================================================================
