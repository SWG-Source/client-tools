// ============================================================================
//
//	TableItemVector.h
//
//	This TableItem is used for editing a vector (x, y, z, and radius)
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemVector_H
#define INCLUDED_TableItemVector_H

#include "TableItemBase.h"


class TemplateTableRow;
class VectorDialogEdit;
class VectorParam;

//-----------------------------------------------------------------------------
class TableItemVector : public TableItemBase
{

public:

	TableItemVector(TemplateTableRow &parentRow);
	virtual ~TableItemVector();

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    init();
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	//void initAsSingle(const VectorParam *parameterData);

private:

	// Disabled

	TableItemVector();
	TableItemVector(TableItemVector const &);
	TableItemVector &operator=(TableItemVector const &);
};

// ============================================================================

#endif // INCLUDED_EditFilenameTableItem_H
