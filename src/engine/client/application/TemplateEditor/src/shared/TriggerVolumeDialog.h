// ============================================================================
//
// TriggerVolumeDialog.h
//
// Dialog that lets user edit the contents of a trigger volume (a name and a radius)
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TriggerVolumeDialog_H
#define INCLUDED_TriggerVolumeDialog_H

#include "StaticListDialog.h"

class  TemplateTableRow;
struct TriggerVolumeParamData;

//-----------------------------------------------------------------------------
class TriggerVolumeDialog : public StaticListDialog
{
public:

	TriggerVolumeDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow);

	void    setValue(TriggerVolumeParamData const &triggerVolumeParamData);
	QString getName() const;
	QString getRadius() const;

private:

	TemplateTableRow *m_templateTableRowName;
	TemplateTableRow *m_templateTableRowRadius;

private:

	// Disabled

	TriggerVolumeDialog();
	TriggerVolumeDialog(TriggerVolumeDialog const &);
	TriggerVolumeDialog &operator=(TriggerVolumeDialog const &);
};

// ============================================================================

#endif // INCLUDED_TriggerVolumeDialog_H
