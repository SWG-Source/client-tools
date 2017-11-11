// ============================================================================
//
// DataTypeEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "DataTypeEdit.h"
#include "TableItemBase.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// DataTypeEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
DataTypeEdit::DataTypeEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : QHBox(parentWidget, name)
 , m_parentTableItem(parentTableItemBase)
 , m_focusWidget(NULL)
 , m_weightSpinBox(NULL)
 , m_isWeightedItem(false)
{
}

//-----------------------------------------------------------------------------
void DataTypeEdit::init()
{
}

//-----------------------------------------------------------------------------
void DataTypeEdit::setWeightedItemStatus(bool setOn)
{
	if (setOn)
	{
		if (m_weightSpinBox == NULL)
		{
			m_weightSpinBox = new QSpinBox(1, 100, 1, this);
			m_weightSpinBox->setSuffix(" %");
		}

		m_weightSpinBox->show();
	}
	else
	{
		if (m_weightSpinBox != NULL)
		{
			m_weightSpinBox->hide();
		}
	}

	m_isWeightedItem = setOn;
}

// This call is ignored if this item is not weighted
//-----------------------------------------------------------------------------
void DataTypeEdit::setWeight(int weight)
{
	if ((m_weightSpinBox != NULL) && m_isWeightedItem)
	{
		m_weightSpinBox->setValue(weight);
	}
}

//-----------------------------------------------------------------------------
int DataTypeEdit::getWeight() const
{
	int result = 0;

	if (m_isWeightedItem)
	{
		result = m_weightSpinBox->value();
	}

	return result;
}

//-----------------------------------------------------------------------------
void DataTypeEdit::setFocusTarget(QWidget &focusTarget)
{
	m_focusWidget = &focusTarget;

	if (m_focusWidget->focusProxy() != NULL)
	{
		m_focusWidget->focusProxy()->installEventFilter(this);
	}
	else
	{
		m_focusWidget->installEventFilter(this);
	}
}

//// Make sure Table Cell Editors don't eat up valuable table navigation keys
////-----------------------------------------------------------------------------
//bool DataTypeEdit::eventFilter(QObject * watched, QEvent *event)
//{
//	bool handled = false;
//
//	if (event->type() == QEvent::KeyPress)
//	{
//		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//
//		if ((keyEvent->key() == Qt::Key_Up)   ||
//		   (keyEvent->key() == Qt::Key_Down) ||
//		   (keyEvent->key() == Qt::Key_Tab))
//		{
//			QObject &parentTable = m_parentTableItem.getParentTable();
//
//			handled = parentTable.event(event);
//		}
//
//		if (keyEvent->key() == Qt::Key_Enter)
//		{
//			//m_parentTableItem->
//		}
//	}
//
//	if (!handled)
//	{
//		handled = QHBox::eventFilter(watched, event);
//	}
//
//	return handled;
//}

//-----------------------------------------------------------------------------
void DataTypeEdit::focusInEvent(QFocusEvent *focusEvent)
{
	UNREF(focusEvent); // Don't care about the specifics, just wanted to know that this widget got the focus

	if (m_focusWidget != NULL)
	{
		m_focusWidget->setFocus();
	}

}

//-----------------------------------------------------------------------------
void DataTypeEdit::hideEvent(QHideEvent *hideEvent)
{
	UNREF(hideEvent); // Don't care about the specifics, just wanted to know that this widget is about to be hidden
}

//-----------------------------------------------------------------------------
void DataTypeEdit::keyReleaseEvent(QKeyEvent *keyEvent)
{
	keyEvent->ignore();
}

//-----------------------------------------------------------------------------
TableItemBase &DataTypeEdit::getTableItemBase() const
{
	return m_parentTableItem;
}

//-----------------------------------------------------------------------------
bool DataTypeEdit::isWeighted() const
{
	return m_isWeightedItem;
}

// ============================================================================
