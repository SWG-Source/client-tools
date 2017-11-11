// ============================================================================
//
// DataTypeEdit.h
//
// The base class for all template table editors. This provides functionality
// for data states that are shared by most/all template datatypes, namely:
//
//  * Item is +/- its parent value
//
//  * Item is weighted
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_DataTypeEdit_H
#define INCLUDED_DataTypeEdit_H

class TableItemBase;

//-----------------------------------------------------------------------------
class DataTypeEdit : public QHBox
{
public:

	DataTypeEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	virtual void init();

	virtual QString getText() const = 0;

	// Deal with weighted item status

	void            setWeightedItemStatus(bool setOn);
	void            setWeight(int weight);
	int             getWeight() const;
	bool            isWeighted() const;
	TableItemBase & getTableItemBase() const;

protected:

	void         setFocusTarget(QWidget &focusTarget);
	//bool         eventFilter(QObject *watched, QEvent *event);
	void         focusInEvent(QFocusEvent *focusEvent);
	void         hideEvent(QHideEvent *hideEvent);
	virtual void keyReleaseEvent(QKeyEvent *keyEvent);

private:

	TableItemBase &m_parentTableItem; // Who we are editing for
	QWidget *      m_focusWidget;
	QSpinBox *     m_weightSpinBox;
	bool           m_isWeightedItem;

private:

	// Disabled

	DataTypeEdit &operator =(DataTypeEdit const &);
};
// ============================================================================

#endif // INCLUDED_DataTypeEdit_H
