// ============================================================================
//
// VectorDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_VectorDialogEdit_H
#define INCLUDED_VectorDialogEdit_H

#include "BaseDialogEdit.h"

class TableItemBase;
class VectorDialog;
struct VectorParamData;

//-----------------------------------------------------------------------------
class VectorDialogEdit : public BaseDialogEdit
{
public:

	VectorDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	void setValue(const VectorParamData &vectorParamData);

protected:

	virtual QString getTextFromDialog() const;

private:

	VectorDialog *getVectorDialog() const;

private:

	VectorDialogEdit &operator =(VectorDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_VectorDialogEdit_H
