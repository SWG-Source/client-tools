// ============================================================================
//
//	TableItemColor.h
//
//	This class extends the functionality of a QTableItem. On top of the 
//	abilities that the QTableItem class provides, this class also allows the
//	programmer to:
//
//	  - set the cell's color. The cell will retain its color even when the
//	    cell is selected (i.e. no inverted color).
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemColor_H
#define INCLUDED_TableItemColor_H

//-----------------------------------------------------------------------------
class TableItemColor : public QTableItem
{
public:

	TableItemColor(QTable &parentRow, QString const &text, QColor const &cellColor = Qt::white);

	virtual void     paint(QPainter *painter, QColorGroup const &colorGroup, QRect const &rect, bool selected);

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

#endif // INCLUDED_TableItemColor_H
