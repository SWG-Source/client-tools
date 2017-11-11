// ============================================================================
//
// TableItemColor.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemColor.h"

// ============================================================================
//
// TableItemColor
//
// ============================================================================

//-----------------------------------------------------------------------------
//  Constructor
//
//  parentTable - The table that this cell sits in
//
//  text - The text label for this table cell
//
//  QColor - The initial color for this cell's background
//
//  See QTableItem, TemplateTableRow, QColor
//-----------------------------------------------------------------------------
TableItemColor::TableItemColor(QTable &parentTable, QString const &text, QColor const &cellColor)
 : QTableItem(&parentTable, QTableItem::Never, text)
 , m_cellColor(cellColor)
 , m_textColor()
 , m_number(false)
{
}

//-----------------------------------------------------------------------------
//	paint
//
//	Overloaded paint method -- paints this cell's base color, and then
//    uses the parent to paint the rest as normal.
//
//-----------------------------------------------------------------------------
void TableItemColor::paint(QPainter *painter, const QColorGroup &colorGroup, const QRect &updateRegion, bool selected)
{
	QColorGroup colorGroupCopy(colorGroup);

	colorGroupCopy.setColor(QColorGroup::Base, m_cellColor);
	colorGroupCopy.setColor(QColorGroup::Highlight, m_cellColor);
	colorGroupCopy.setColor(QColorGroup::Text, m_textColor);
	colorGroupCopy.setColor(QColorGroup::HighlightedText, m_textColor);

	if (m_number)
	{
		QString tempText(text());

		setText("");

		QTableItem::paint(painter, colorGroupCopy, updateRegion, selected);

		setText(tempText);

		painter->setPen(m_textColor);

		QRect rect(0, 0, painter->window().width(), painter->window().height());
		painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text());
	}
	else
	{
		QTableItem::paint(painter, colorGroupCopy, updateRegion, selected);
	}
}

//-----------------------------------------------------------------------------
void TableItemColor::setTextColor(QColor const &textColor)
{
	m_textColor = textColor;
}

//-----------------------------------------------------------------------------
//	setCellColor
//
//	Set this cell's background color and refresh it.
//
//	cellColor - New color to change the cell to. If the cell is already set
//	  to this color, then the cell will not waste its time updating.
//
//-----------------------------------------------------------------------------
void TableItemColor::setCellColor(const QColor &cellColor)
{
	if (m_cellColor != cellColor)
	{
		m_cellColor = cellColor;

		//if (isVisible())
		//{
		//	table()->updateCell(row(), col());
		//}
	}
}

//-----------------------------------------------------------------------------
//	getCellColor
//
//	Get this cell's background color
//
//	Returns a reference to a QColor object containing the cell's color.
//
//-----------------------------------------------------------------------------
const QColor &TableItemColor::getCellColor() const
{
	return m_cellColor;
}

//-----------------------------------------------------------------------------
void TableItemColor::setText(const QString &text)
{
	QTableItem::setText(text);
}

//-----------------------------------------------------------------------------
void TableItemColor::setTextNumber(const QString &text)
{
	m_number = true;
	QTableItem::setText(text);
}

// ============================================================================
