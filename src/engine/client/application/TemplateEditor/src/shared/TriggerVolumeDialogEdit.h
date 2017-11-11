// ============================================================================
//
// TriggerVolumeDialogEdit.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TriggerVolumeDialogEdit_H
#define INCLUDED_TriggerVolumeDialogEdit_H

#include "BaseDialogEdit.h"

class TriggerVolumeDialog;
struct TriggerVolumeParamData;

//-----------------------------------------------------------------------------
class TriggerVolumeDialogEdit : public BaseDialogEdit
{
public:

	TriggerVolumeDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	void setValue(TriggerVolumeParamData const &triggerVolumeParamData);

protected:

	virtual QString getTextFromDialog() const;

private:

	TriggerVolumeDialog *getTriggerVolumeDialog() const;

private:

	// Disabled

	TriggerVolumeDialogEdit &operator =(TriggerVolumeDialogEdit const &);
};
// ============================================================================

#endif // INCLUDED_TriggerVolumeDialogEdit_H
