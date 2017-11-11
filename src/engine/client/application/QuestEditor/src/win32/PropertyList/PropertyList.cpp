// ======================================================================
//
// PropertyList.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"

// ----------------------------------------------------------------------

#include "PropertyBoolItem.h"
#include "PropertyColorItem.h"
#include "PropertyCoordItem.h"
#include "PropertyCursorItem.h"
#include "PropertyDateItem.h"
#include "PropertyDateTimeItem.h"
#include "PropertyDoubleItem.h"
#include "PropertyEnumItem.h"
#include "PropertyFileItem.h"
#include "PropertyFontItem.h"
#include "PropertyIntItem.h"
#include "PropertyKeySequenceItem.h"
#include "PropertyLayoutItem.h"
#include "PropertyListItem.h"
#include "PropertyPixmapItem.h"
#include "PropertyTextItem.h"
#include "PropertyTimeItem.h"
#include "PropertySizePolicyItem.h"

// ----------------------------------------------------------------------

#include <qapp.h>
#include <qbutton.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qtextview.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------

#include "PropertyList.moc"

// ----------------------------------------------------------------------

class PropertyList::PropertyWhatsThis : public QWhatsThis
{
public:
	PropertyWhatsThis(PropertyList *l);
	QString text(const QPoint &pos);

private:
	PropertyList *propertyList;

private: //-- disabled
	PropertyWhatsThis();
	PropertyWhatsThis(PropertyWhatsThis const &rhs);
};

// ----------------------------------------------------------------------

PropertyList::PropertyList(QWidget * propertListParent)
: QListView(propertListParent)
{
	showSorted = FALSE;
	header()->setMovingEnabled(FALSE);
	header()->setStretchEnabled(TRUE);

	setResizePolicy(QScrollView::Manual);

	viewport()->setAcceptDrops(TRUE);
	viewport()->installEventFilter(this);

	addColumn(tr("Property"));
    addColumn(tr("Value"));

    connect(header(), SIGNAL(sizeChange(int, int, int)), this, SLOT(updateEditorSize()));
    disconnect(header(), SIGNAL(sectionClicked(int)), this, SLOT(changeSortColumn(int)));

    connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(toggleSort()));
    connect(this, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(itemPressed(QListViewItem *, const QPoint &, int)));
    connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(toggleOpen(QListViewItem *)));

    setSorting(-1);
    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(AlwaysOn);
    setColumnWidthMode(1, Manual);

    mousePressed = FALSE;
    pressItem = 0;
    theLastEvent = MouseEvent;

    header()->installEventFilter(this);

	whatsThis = new PropertyWhatsThis(this);
}

// ----------------------------------------------------------------------

void PropertyList::toggleSort()
{
    showSorted = !showSorted;
    clear();
    setup();
}

// ----------------------------------------------------------------------

void PropertyList::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
    if (currentItem())
		((PropertyItem*)currentItem())->showEditor();
}

// ----------------------------------------------------------------------

void PropertyList::setupProperties()
{
	setCurrentItem(firstChild());
	
	if (showSorted)
	{
		setSorting(0);
		sort();
		setSorting(-1);
		setCurrentItem(firstChild());
		qApp->processEvents();
	}
	
	updateEditorSize();
}

// ----------------------------------------------------------------------

bool PropertyList::addPropertyItem(PropertyItem *&item, const QCString &name, QVariant::Type t)
{
	switch (t)
	{
	case QVariant::String:
		item = new PropertyTextItem(this, item, 0, name, FALSE, FALSE);
		break;
	case QVariant::CString:
		item = new PropertyTextItem(this, item, 0, name, name == "name", FALSE, TRUE);
		break;
	case QVariant::Bool:
		item = new PropertyBoolItem(this, item, 0, name);
		break;
	case QVariant::Font:
		item = new PropertyFontItem(this, item, 0, name);
		break;
	case QVariant::Int:
		item = new PropertyIntItem(this, item, 0, name, TRUE);
		break;
	case QVariant::Double:
		item = new PropertyDoubleItem(this, item, 0, name);
		break;
	case QVariant::KeySequence:
		item = new PropertyKeySequenceItem(this, item, 0, name);
		break;
	case QVariant::UInt:
		item = new PropertyIntItem(this, item, 0, name, FALSE);
		break;
	case QVariant::StringList:
		item = new PropertyListItem(this, item, 0, name, FALSE);
		break;
	case QVariant::Rect:
		item = new PropertyCoordItem(this, item, 0, name, PropertyCoordItem::Rect);
		break;
	case QVariant::Point:
		item = new PropertyCoordItem(this, item, 0, name, PropertyCoordItem::Point);
		break;
	case QVariant::Size:
		item = new PropertyCoordItem(this, item, 0, name, PropertyCoordItem::Size);
		break;
	case QVariant::Color:
		item = new PropertyColorItem(this, item, 0, name, TRUE);
		break;
	case QVariant::Pixmap:
		item = new PropertyPixmapItem(this, item, 0, name, PropertyPixmapItem::Pixmap);
		break;
	case QVariant::IconSet:
		item = new PropertyPixmapItem(this, item, 0, name, PropertyPixmapItem::IconSet);
		break;
	case QVariant::Image:
		item = new PropertyPixmapItem(this, item, 0, name, PropertyPixmapItem::Image);
		break;
	case QVariant::SizePolicy:
		item = new PropertySizePolicyItem(this, item, 0, name);
		break;
	case QVariant::Cursor:
		item = new PropertyCursorItem(this, item, 0, name);
		break;
	case QVariant::Date:
		item = new PropertyDateItem(this, item, 0, name);
		break;
	case QVariant::Time:
		item = new PropertyTimeItem(this, item, 0, name);
		break;
	case QVariant::DateTime:
		item = new PropertyDateTimeItem(this, item, 0, name);
		break;
	default:
		return FALSE;
	}
	
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyList::paintEmptyArea(QPainter *p, const QRect &r)
{
    p->fillRect(r, QColor(255, 255, 255));
}

// ----------------------------------------------------------------------

void PropertyList::setCurrentItem(QListViewItem *i)
{
    if (!i)
		return;
	
    if (currentItem())
		((PropertyItem*)currentItem())->hideEditor();

    QListView::setCurrentItem(i);
    ((PropertyItem*)currentItem())->showEditor();
}

// ----------------------------------------------------------------------

void PropertyList::updateEditorSize()
{
    if (currentItem())
		((PropertyItem*)currentItem())->showEditor();
}

// ----------------------------------------------------------------------

void PropertyList::layoutInitValue(PropertyItem *, bool)
{
}

// ----------------------------------------------------------------------

void PropertyList::itemPressed(QListViewItem *i, const QPoint &p, int c)
{
    if (!i)
		return;
	
    PropertyItem *pi = (PropertyItem*)i;
	pi->showEditor();
	
    if (!pi->hasSubItems())
		return;
	
    if (c == 0 && viewport()->mapFromGlobal(p).x() < 20)
		toggleOpen(i);
}

// ----------------------------------------------------------------------

void PropertyList::toggleOpen(QListViewItem *i)
{
    if (!i)
		return;
    PropertyItem *pi = (PropertyItem*)i;
    if (pi->hasSubItems())
	{
		pi->setOpen(!pi->isOpen());
    }
	else
	{
		pi->toggle();
    }
}

// ----------------------------------------------------------------------

bool PropertyList::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e)
		return TRUE;
	
    PropertyItem *i = (PropertyItem*)currentItem();
	
    if (e->type() == QEvent::KeyPress)
		theLastEvent = KeyEvent;
	
    else if (e->type() == QEvent::MouseButtonPress)
		theLastEvent = MouseEvent;
	
    if (o != this &&e->type() == QEvent::KeyPress)
	{
		QKeyEvent * ke = (QKeyEvent*)e;
		QLineEdit * le = ::qt_cast<QLineEdit *>(o);
		
		if ((ke->key() == Key_Up || ke->key() == Key_Down) &&
			(o != this || o != viewport()) &&
			!(ke->state() & ControlButton))
		{
			QApplication::sendEvent(this, (QKeyEvent*)e);
			return TRUE;
		}
		else if ((!le || (le && le->isReadOnly())) && i && i->hasSubItems())
		{
			if (!i->isOpen() && (ke->key() == Key_Plus || ke->key() == Key_Right))
				i->setOpen(TRUE);
			else if (i->isOpen() && (ke->key() == Key_Minus || ke->key() == Key_Left))
				i->setOpen(FALSE);
		}
		else if ((ke->key() == Key_Return || ke->key() == Key_Enter) && ::qt_cast<QComboBox*>(o))
		{
			QKeyEvent ke2(QEvent::KeyPress, Key_Space, 0, 0);
			QApplication::sendEvent(o, &ke2);
			return TRUE;
		}
    }
	else if (o == viewport())
	{
		QMouseEvent *me;
		PropertyListItem* i;
		switch (e->type())
		{
		case QEvent::MouseButtonPress:
			me = (QMouseEvent*)e;
			i = (PropertyListItem*) itemAt(me->pos());
			if (i && (::qt_cast<PropertyColorItem*>(i) || ::qt_cast<PropertyPixmapItem*>(i)))
			{
				pressItem = i;
				pressPos = me->pos();
				mousePressed = TRUE;
			}
			break;
		case QEvent::MouseMove:
			me = (QMouseEvent*)e;
			if (me && me->state() & LeftButton && mousePressed)
			{
				i = (PropertyListItem*) itemAt(me->pos());
				if (i  && i == pressItem)
				{
					
					if ((pressPos - me->pos()).manhattanLength() > QApplication::startDragDistance())
					{
						if (::qt_cast<PropertyColorItem*>(i))
						{
							QColor col = i->value().asColor();
							QColorDrag *drg = new QColorDrag(col, this);
							QPixmap pix(25, 25);
							pix.fill(col);
							QPainter p(&pix);
							p.drawRect(0, 0, pix.width(), pix.height());
							p.end();
							drg->setPixmap(pix);
							mousePressed = FALSE;
							drg->dragCopy();
						}
						else if (::qt_cast<PropertyPixmapItem*>(i))
						{
							QPixmap pix = i->value().asPixmap();
							if(!pix.isNull())
							{
								QImage img = pix.convertToImage();
								QImageDrag *drg = new QImageDrag(img, this);
								drg->setPixmap(pix);
								mousePressed = FALSE;
								drg->dragCopy();
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
    }
	else if (o == header())
	{
		if (e->type() == QEvent::ContextMenu)
		{
			((QContextMenuEvent *)e)->accept();
			QPopupMenu menu(0);
			menu.setCheckable(TRUE);
			
			const int cat_id = 1;
			const int alpha_id = 2;
			menu.insertItem(tr("Sort &Categorized"), cat_id);
			int alpha = menu.insertItem(tr("Sort &Alphabetically"), alpha_id);
			
			if (showSorted)
				menu.setItemChecked(alpha_id, TRUE);
			else
				menu.setItemChecked(cat_id, TRUE);
			
			int res = menu.exec(((QContextMenuEvent*)e)->globalPos());
			if (res != -1)
			{
				bool newShowSorted = (res == alpha);
				if (showSorted != newShowSorted)
				{
					showSorted = newShowSorted;
					clear();
					setup();
				}
			}
			return TRUE;
		}
    }
	
    return QListView::eventFilter(o, e);
}

// ----------------------------------------------------------------------

void PropertyList::refetchData()
{
	/* TTODO: Unsupported
    QListViewItemIterator it(this);
    for (; it.current(); ++it)
	{
		PropertyItem *i = (PropertyItem*)it.current();
		if (!i->propertyParent())
			setPropertyValue(i);
		if (i->hasSubItems())
			i->initChildren();

		// TTODO
		//bool changed = isPropertyChanged(i)
		//if (changed != i->isChanged())
		//	i->setChanged(changed, FALSE);
    }

    updateEditorSize();
	*/
}

// ----------------------------------------------------------------------

void PropertyList::resetProperty()
{
    if (!currentItem())
		return;
	
    PropertyItem *i = (PropertyItem*)currentItem();

	QVariant resetValue = getResetValue(i);

	if (resetValue != i->value())
	{
		DEBUG_REPORT_LOG_PRINT(true, ("PropertyList::resetProperty() - Value = [%s] Default = [%s]\n", i->value().toString().ascii(), resetValue.toString().ascii()));

		i->setValue(resetValue);
	
	    if (i->hasSubItems())
			i->initChildren();
	}
}

// ----------------------------------------------------------------------

void PropertyList::setCurrentProperty(const QString &n, bool useValueName)
{
	PropertyItem * item = (PropertyItem*)currentItem();
	
	if(item)
	{
		if (useValueName ? item->getValueName() == n : item->text(0) == n)
			return;
	}
	
    QListViewItemIterator it(this);
    for (; it.current(); ++it)
	{
		item = dynamic_cast<PropertyItem *>(it.current());

		if (item && (useValueName ? item->getValueName() == n : item->text(0) == n))
		{
			setCurrentItem(item);
			break;
		}
    }
}

// ----------------------------------------------------------------------

void PropertyList::setPropertyValue(PropertyItem *i)
{
	UNREF(i);
}

// ----------------------------------------------------------------------

void PropertyList::viewportDragEnterEvent(QDragEnterEvent *e)
{
    PropertyListItem *i = (PropertyListItem*) itemAt(e->pos());
    if(!i)
	{
		e->ignore();
		return;
    }
	
    if (::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode(e))
		e->accept();
    else if (::qt_cast<PropertyPixmapItem*>(i) && QImageDrag::canDecode(e))
		e->accept();
    else
		e->ignore();
}

// ----------------------------------------------------------------------

void PropertyList::viewportDragMoveEvent (QDragMoveEvent *e)
{
    PropertyListItem *i = (PropertyListItem*) itemAt(e->pos());
    if(!i)
	{
		e->ignore();
		return;
    }
	
	if (::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode(e))
		e->accept();
    else if (::qt_cast<PropertyPixmapItem*>(i) && QImageDrag::canDecode(e))
		e->accept();
    else
		e->ignore();
}

// ----------------------------------------------------------------------

void PropertyList::viewportDropEvent (QDropEvent *e)
{
    PropertyListItem *i = (PropertyListItem*) itemAt(e->pos());
    if(!i)
	{
		e->ignore();
		return;
    }
	
    if (::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode(e))
	{
		QColor color;
		QColorDrag::decode(e, color);
		i->setValue(QVariant(color));
		valueChanged(i);
		e->accept();
    }
    else if (::qt_cast<PropertyPixmapItem*>(i)  && QImageDrag::canDecode(e))
	{
		QImage img;
		QImageDrag::decode(e, img);
		QPixmap pm;
		pm.convertFromImage(img);
		i->setValue(QVariant(pm));
		valueChanged(i);
		e->accept();
    }
    else
		e->ignore();
}

// ----------------------------------------------------------------------

PropertyList::LastEventType PropertyList::lastEvent()
{
	return theLastEvent;
}

// ----------------------------------------------------------------------

void PropertyList::setup()
{
    viewport()->setUpdatesEnabled(FALSE);
    setupProperties();
    viewport()->setUpdatesEnabled(TRUE);
    updateEditorSize();
}

// ----------------------------------------------------------------------

void PropertyList::clear()
{
	setContentsPos(0, 0);
	QListView::clear();
}

// ----------------------------------------------------------------------

QString PropertyList::whatsThisAt(const QPoint &p)
{
	return whatsThisText(itemAt(p));
}

// ----------------------------------------------------------------------

void PropertyList::showCurrentWhatsThis()
{
	if (!currentItem())
		return;

	QPoint point(0, currentItem()->itemPos());
	point = viewport()->mapToGlobal(contentsToViewport(point));
	QWhatsThis::display(whatsThisText(currentItem()), point, viewport());
}

// ----------------------------------------------------------------------

QString PropertyList::whatsThisText(QListViewItem *i)
{
	PropertyItem * propertyItem = safe_cast<PropertyItem *>(i);

	if (!propertyItem)
		return QWhatsThis::textFor(this);

	return QString(cs_whatsThisTextFormat).arg(propertyItem->getValueName()).arg(propertyItem->getWhatsThisText());
}

// ======================================================================

PropertyList::PropertyWhatsThis::PropertyWhatsThis(PropertyList *l)
: QWhatsThis(l->viewport())
, propertyList(l)
{
}

// ----------------------------------------------------------------------

QString PropertyList::PropertyWhatsThis::text(const QPoint &pos)
{
	return propertyList->whatsThisAt(pos);
}

// ----------------------------------------------------------------------

PropertyItem * PropertyList::getCurrentPropertyItem()
{
	return dynamic_cast<PropertyItem *>(currentItem());
}

// ======================================================================
