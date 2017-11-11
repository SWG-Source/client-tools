// ============================================================================
//
// TriggerVolumeDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TriggerVolumeDialog.h"

#include "TableItemBase.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// TriggerVolumeDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
TriggerVolumeDialog::TriggerVolumeDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : StaticListDialog(parent, name, parentTemplateTableRow)
{
	// Add the name row

	m_templateTableRowName = new TemplateTableRow(*m_templateTable, 0, TemplateData::TYPE_STRING, "Volume Name");
	NOT_NULL(m_templateTableRowName);
	m_templateTableRowName->setChecked(true);
	m_templateTableRowName->setCheckEnabled(false);

	// Add the radius row

	m_templateTableRowRadius = new TemplateTableRow(*m_templateTable, 1, TemplateData::TYPE_FLOAT, "Trigger Radius");
	NOT_NULL(m_templateTableRowRadius);
	m_templateTableRowRadius->setChecked(true);
	m_templateTableRowRadius->setCheckEnabled(false);
}

//-----------------------------------------------------------------------------
void TriggerVolumeDialog::setValue(TriggerVolumeParamData const &triggerVolumeParamData)
{
	m_templateTableRowName->getColumnValue()->init(&triggerVolumeParamData.name, 0, 0);
	m_templateTableRowRadius->getColumnValue()->init(&triggerVolumeParamData.radius, 0, 0, TemplateTableRow::PP_undefined);
}

//-----------------------------------------------------------------------------
QString TriggerVolumeDialog::getName() const
{
	return m_templateTableRowName->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
QString TriggerVolumeDialog::getRadius() const
{
	return m_templateTableRowRadius->getColumnValue()->text();
}

// ============================================================================
