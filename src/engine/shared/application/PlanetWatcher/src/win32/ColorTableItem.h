// ============================================================================
//
//	ColorTableItem.h
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ColorTableItem_H
#define INCLUDED_ColorTableItem_H

//-----------------------------------------------------------------------------
class ColorTableItem : public QTableItem
{
public:

	ColorTableItem(QTable *parent, QString const &text, QColor const &cellColor = Qt::white);

	virtual void  paint(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect, bool selected);

	void          setCellColor(QColor const &cellColor);
	const QColor &getCellColor() const;
	void          setTextColor(QColor const &textColor);
	void          setTextNumber(QString const &text);

	virtual void setText(QString const &text);

private:

	QColor m_cellColor;
	QColor m_textColor;
	bool   m_number;
};

// ============================================================================

#endif // INCLUDED_ColorTableItem_H
