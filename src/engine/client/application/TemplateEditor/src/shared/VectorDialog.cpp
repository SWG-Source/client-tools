// ============================================================================
//
// VectorDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "VectorDialog.h"

#include "TableItemBase.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "TemplateLoader.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include "sharedUtility/TemplateParameter.h"

// ============================================================================
//
// VectorDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
VectorDialog::VectorDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow)
 : StaticListDialog(parent, name, parentRow)
 , m_indexX(0)
 , m_indexY(1)
 , m_ignoreIndexY(2)
 , m_indexZ(3)
 , m_indexRadius(4)
{
	// Index X

	new TemplateTableRow(*m_templateTable, m_indexX, TemplateData::TYPE_FLOAT, "X-Coordinate");

	// Index Y

	new TemplateTableRow(*m_templateTable, m_indexY, TemplateData::TYPE_FLOAT, "Y-Coordinate");

	// Ignore Index Y

	new TemplateTableRow(*m_templateTable, m_ignoreIndexY, TemplateData::TYPE_BOOL, "Ignore Y?");

	// Index Z

	new TemplateTableRow(*m_templateTable, m_indexZ, TemplateData::TYPE_FLOAT, "Z-Coordinate");

	// Index Radius

	new TemplateTableRow(*m_templateTable, m_indexRadius, TemplateData::TYPE_FLOAT, "Radius");
}

//-----------------------------------------------------------------------------
void VectorDialog::setValue(const VectorParamData &vectorParamData)
{
	BoolParam ignoreIndexYParam;

	if (vectorParamData.ignoreY)
	{
		ignoreIndexYParam.setValue(true);
	}
	else
	{
		ignoreIndexYParam.setValue(false);
	}

	//getTemplateTableRow(m_indexX)->getColumnValue()->init(&vectorParamData.x);
	//getTemplateTableRow(m_indexY)->getColumnValue()->init(&vectorParamData.y);
	//getTemplateTableRow(m_ignoreIndexY)->getColumnValue()->init(&ignoreIndexYParam);
	//getTemplateTableRow(m_indexZ)->getColumnValue()->init(&vectorParamData.z);
	//getTemplateTableRow(m_indexRadius)->getColumnValue()->init(&vectorParamData.radius);
}

//-----------------------------------------------------------------------------
QString VectorDialog::getCoordXText() const
{
	return getTemplateTableRow(m_indexX)->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
QString VectorDialog::getCoordYText() const
{
	return getTemplateTableRow(m_indexY)->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
QString VectorDialog::getCoordZText() const
{
	return getTemplateTableRow(m_indexZ)->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
QString VectorDialog::getRadiusText() const
{
	return getTemplateTableRow(m_indexRadius)->getColumnValue()->text();
}

//-----------------------------------------------------------------------------
bool VectorDialog::ignoreIndexY() const
{
	if(getTemplateTableRow(m_ignoreIndexY)->getColumnValue()->text() == "True")
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ============================================================================
