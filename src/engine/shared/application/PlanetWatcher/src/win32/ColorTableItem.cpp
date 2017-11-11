// ============================================================================
//
// ColorTableItem.cpp
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstPlanetWatcher.h"
#include "ColorTableItem.h"

// ============================================================================
//
// ColorTableItem
//
// ============================================================================

//-----------------------------------------------------------------------------
ColorTableItem::ColorTableItem(QTable *parent, QString const &text, QColor const &cellColor)
 : QTableItem(parent, QTableItem::Never, text)
 , m_cellColor(cellColor)
 , m_textColor()
 , m_number(false)
{
}

//-----------------------------------------------------------------------------
void ColorTableItem::paint(QPainter *painter, const QColorGroup &colorGroup, const QRect &updateRegion, bool selected)
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
void ColorTableItem::setTextColor(QColor const &textColor)
{
	m_textColor = textColor;
}

//
//-----------------------------------------------------------------------------
void ColorTableItem::setCellColor(const QColor &cellColor)
{
	if (m_cellColor != cellColor)
	{
		m_cellColor = cellColor;
	}
}

//-----------------------------------------------------------------------------
const QColor &ColorTableItem::getCellColor() const
{
	return m_cellColor;
}

//-----------------------------------------------------------------------------
void ColorTableItem::setText(const QString &text)
{
	QTableItem::setText(text);
}

//-----------------------------------------------------------------------------
void ColorTableItem::setTextNumber(const QString &text)
{
	m_number = true;
	QTableItem::setText(text);
}

// ============================================================================
