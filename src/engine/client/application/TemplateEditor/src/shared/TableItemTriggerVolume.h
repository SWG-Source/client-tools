// ============================================================================
//
//	TableItemTriggerVolume.h
//
//	This TableItem is used for entering a trigger volume (a string, which is
//	a trigger volume name, and a float, the trigger's radius)
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemTriggerVolume_H
#define INCLUDED_TableItemTriggerVolume_H

#include "TableItemBase.h"

class TemplateTableRow;
class TriggerVolumeDialogEdit;
class TriggerVolumeParam;

//-----------------------------------------------------------------------------
class TableItemTriggerVolume : public TableItemBase
{
public:

	TableItemTriggerVolume(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void                     init(TriggerVolumeParam const *triggerVolumeParam, int const parameterIndex, int const weightedListIndex);
	void                     initAsSingle(TriggerVolumeParam const *triggerVolumeParam);
	void                     initAsWeightedList(TriggerVolumeParam const *triggerVolumeParam, int const parameterIndex);

	TriggerVolumeDialogEdit *getTriggerVolumeDialogEdit() const;

private:

	// Disabled

	TableItemTriggerVolume();
	TableItemTriggerVolume(TableItemTriggerVolume const &);
	TableItemTriggerVolume &operator=(TableItemTriggerVolume const &);
};

// ============================================================================

#endif // INCLUDED_TableItemTriggerVolume_H
