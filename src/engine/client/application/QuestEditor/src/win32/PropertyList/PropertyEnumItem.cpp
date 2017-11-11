// ======================================================================
//
// PropertyEnumItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyEnumItem.h"

// ----------------------------------------------------------------------

#include <qdrawutil.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qtimer.h>

// ----------------------------------------------------------------------

#include "PropertyEnumItem.moc"

// ----------------------------------------------------------------------

EnumItem::EnumItem(const QString &k, bool s)
: key(k)
, selected(s)
{
}

// ----------------------------------------------------------------------

EnumItem::EnumItem()
: key(QString::null)
, selected(FALSE)
{
}

// ----------------------------------------------------------------------

EnumPopup::EnumPopup(QWidget *parent, const char *name, WFlags f)
: QFrame(parent, name, f)
{
	setLineWidth(1);
	setFrameStyle(Panel | Plain);
	setPaletteBackgroundColor(Qt::white);
	popLayout = new QVBoxLayout(this, 3);
	checkBoxList.setAutoDelete(TRUE);
}

// ----------------------------------------------------------------------

EnumPopup::~EnumPopup()
{
}

// ----------------------------------------------------------------------

void EnumPopup::insertEnums(QValueList<EnumItem> lst)
{
	while (checkBoxList.count())
		checkBoxList.removeFirst();
	
	itemList = lst;
	QCheckBox *cb;
	QValueListConstIterator<EnumItem> it = itemList.begin();
	for (; it != itemList.end(); ++it)
	{
		cb = new QCheckBox(this);
		cb->setText((*it).key);
		cb->setChecked((*it).selected);
		if (it == itemList.begin())
			cb->setFocus();
		checkBoxList.append(cb);
		cb->resize(width(), cb->height());
		popLayout->addWidget(cb);
	}
}

// ----------------------------------------------------------------------

void EnumPopup::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Key_Escape)
	{
		hide();
		emit hidden();
	}
	else if (e->key() == Key_Enter || e->key() == Key_Return)
	{
		closeWidget();
	}
}

// ----------------------------------------------------------------------

void EnumPopup::closeWidget()
{
	QPtrListIterator<QCheckBox> it(checkBoxList);
	int i = 0;
	while (it.current() != 0)
	{
		itemList[i].selected = (*it)->isChecked();
		++it;
		++i;
	}
	close();
	emit closed();
}

// ----------------------------------------------------------------------

QValueList<EnumItem> EnumPopup::enumList() const
{
	return itemList;
}

// ----------------------------------------------------------------------

EnumBox::EnumBox(QWidget *parent, const char *name)
: QComboBox(parent, name)
{
	pop = new EnumPopup(this, "popup", QObject::WType_Popup);
	connect(pop, SIGNAL(hidden()), this, SLOT(popupHidden()));
	connect(pop, SIGNAL(closed()), this, SLOT(popupClosed()));
	popupShown = FALSE;
	arrowDown = FALSE;
}

// ----------------------------------------------------------------------

void EnumBox::popupHidden()
{
	popupShown = FALSE;
}

// ----------------------------------------------------------------------

void EnumBox::popupClosed()
{
	popupShown = FALSE;
	emit valueChanged();
}

// ----------------------------------------------------------------------

void EnumBox::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	const QColorGroup & g = colorGroup();
	p.setPen(g.text());
	
	QStyle::SFlags flags = QStyle::Style_Default;
	if (isEnabled())
		flags |= QStyle::Style_Enabled;
	if (hasFocus())
		flags |= QStyle::Style_HasFocus;
	
	if (width() < 5 || height() < 5)
	{
		qDrawShadePanel(&p, rect().x(), rect().y(), rect().width(), rect().height(), g, FALSE, 2,
			&g.brush(QColorGroup::Button));
		return;
	}
	style().drawComplexControl(QStyle::CC_ComboBox, &p, this, rect(), g,
		flags, (QStyle::SCFlags)QStyle::SC_All, (arrowDown ? QStyle::SC_ComboBoxArrow : QStyle::SC_None));
	
	QRect re = style().querySubControlMetrics(QStyle::CC_ComboBox, this, QStyle::SC_ComboBoxEditField);
	re = QStyle::visualRect(re, this);
	p.setClipRect(re);
	
	if (!str.isNull())
	{
		p.save();
		p.setFont(font());
		QFontMetrics fm(font());
		int x = re.x(), y = re.y() + fm.ascent();
		p.drawText(x, y, str);
		p.restore();
	}
}

// ----------------------------------------------------------------------

void EnumBox::insertEnums(QValueList<EnumItem> lst)
{
	pop->insertEnums(lst);
}

// ----------------------------------------------------------------------

QValueList<EnumItem> EnumBox::enumList() const
{
	return pop->enumList();
}

// ----------------------------------------------------------------------

void EnumBox::popup()
{
	if (popupShown)
	{
		pop->closeWidget();
		popupShown = FALSE;
		return;
	}
	pop->move(((QWidget*)parent())->mapToGlobal(geometry().bottomLeft()));
	pop->setMinimumWidth(width());
	emit aboutToShowPopup();
	pop->show();
	popupShown = TRUE;
}

// ----------------------------------------------------------------------

void EnumBox::mousePressEvent(QMouseEvent *e)
{
	if (e->button() != LeftButton)
		return;
	
	QRect arrowRect = style().querySubControlMetrics(QStyle::CC_ComboBox, this,
		QStyle::SC_ComboBoxArrow);
	arrowRect = QStyle::visualRect(arrowRect, this);
	
	arrowRect.setHeight(QMAX( height() - (2 * arrowRect.y()), arrowRect.height()));
	
	if (arrowRect.contains(e->pos()))
	{
		arrowDown = TRUE;
		repaint(FALSE);
	}
	
	popup();
	QTimer::singleShot(100, this, SLOT(restoreArrow()));
}

// ----------------------------------------------------------------------

void EnumBox::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Key_Space)
	{
		popup();
		QTimer::singleShot(100, this, SLOT(restoreArrow()));
	}
	else if (e->key() == Key_Enter || e->key() == Key_Return)
	{
		popup();
	}
}

// ----------------------------------------------------------------------

void EnumBox::restoreArrow()
{
	arrowDown = FALSE;
	repaint(FALSE);
}

// ----------------------------------------------------------------------

void EnumBox::setText(const QString &text)
{
	str = text;
	repaint(FALSE);
}

// ----------------------------------------------------------------------

PropertyEnumItem::PropertyEnumItem(PropertyList *l,
								   PropertyItem *after,
								   PropertyItem *prop,
								   const QString &propName)
								   : PropertyItem(l, after, prop, propName)
{
	box = new EnumBox(listview->viewport());
	box->hide();
	box->installEventFilter(listview);
	connect(box, SIGNAL(aboutToShowPopup()), this, SLOT(insertEnums()));
	connect(box, SIGNAL(valueChanged()), this, SLOT(setValue()));
}

// ----------------------------------------------------------------------

PropertyEnumItem::~PropertyEnumItem()
{
	delete (EnumBox*)box;
}

// ----------------------------------------------------------------------

void PropertyEnumItem::showEditor()
{
	PropertyItem::showEditor();
	placeEditor(box);
	if (!box->isVisible())
	{
		box->show();
		box->setText(enumString);
		listView()->viewport()->setFocus();
	}
	box->setFocus();
}

// ----------------------------------------------------------------------

void PropertyEnumItem::hideEditor()
{
	PropertyItem::hideEditor();
	box->hide();
}

// ----------------------------------------------------------------------

void PropertyEnumItem::setValue(const QVariant &v)
{
	enumString = "";
	enumList.clear();
	QStringList lst = v.toStringList();
	QValueListConstIterator<QString> it = lst.begin();
	for (; it != lst.end(); ++it)
		enumList.append(EnumItem(*it, FALSE));
	enumList.first().selected = TRUE;
	enumString = enumList.first().key;
	box->setText(enumString);
	setText(1, enumString);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyEnumItem::insertEnums()
{
	box->insertEnums(enumList);
}

// ----------------------------------------------------------------------

void PropertyEnumItem::setValue()
{
	enumList = box->enumList();
	enumString = "";
	QValueListConstIterator<EnumItem> it = enumList.begin();
	for (; it != enumList.end(); ++it)
	{
		if ((*it).selected)
			enumString += "|" + (*it).key;
	}
	if (!enumString.isEmpty())
		enumString.replace(0, 1, "");
	
	box->setText(enumString);
	setText(1, enumString);
	notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyEnumItem::setCurrentValues(QStrList lst)
{
	enumString ="";
	QStrList::ConstIterator it = lst.begin();
	QValueList<EnumItem>::Iterator eit = enumList.begin();
	for (; eit != enumList.end(); ++eit)
	{
		(*eit).selected = FALSE;
		for (it = lst.begin(); it != lst.end(); ++it)
		{
			if (QString(*it) == (*eit).key)
			{
				(*eit).selected = TRUE;
				enumString += "|" + (*eit).key;
				break;
			}
		}
	}
	if (!enumString.isEmpty())
		enumString.replace(0, 1, "");
	box->setText(enumString);
	setText(1, enumString);
}

// ----------------------------------------------------------------------

QString PropertyEnumItem::currentItem() const
{
	return enumString;
}

// ----------------------------------------------------------------------

QString PropertyEnumItem::currentItemFromObject() const
{
	return enumString;
}

// ======================================================================
