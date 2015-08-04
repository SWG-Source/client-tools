// ============================================================================
//
// TableItemObjvarType.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TableItemObjvarType.h"
#include "ObjvarTypeEdit.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "sharedUtility/TemplateParameter.h"

// ============================================================================
//
// TableItemObjvarType
//
// ============================================================================

TableItemObjvarType::TableItemObjvarType(TemplateTableRow &parentTemplateTableRow)
 : TableItemBase(parentTemplateTableRow)
{
}

//-----------------------------------------------------------------------------
TableItemObjvarType::~TableItemObjvarType()
{
}

//-----------------------------------------------------------------------------
void TableItemObjvarType::init()
{
	//TemplateTableRow const &parentTemplateTableRow = getTemplateTableRow();
	//
	//if(parentTemplateTableRow.getParameterTypes() & TemplateTableRow::PT_literal)
	//{
	//	initAsSingle(parameterData);
	//}

	//TableItemBase::init();
}

//-----------------------------------------------------------------------------
QString TableItemObjvarType::getTypeString() const
{
	return "Base Name";
}

//-----------------------------------------------------------------------------
int TableItemObjvarType::getSupportedRowTypes() const
{
	return 0;
}

////-----------------------------------------------------------------------------
//const ObjvarTypeEdit *TableItemObjvarType::getObjvarTypeEditor() const
//{
//	return m_objvarTypeEditor;
//}

////-----------------------------------------------------------------------------
//void TableItemObjvarType::initAsSingle(const IntegerParam *parameterData)
//{
//	// Create the editor if this is the first time
//	if(m_objvarTypeEditor == NULL)
//	{
//		m_objvarTypeEditor = new ObjvarTypeEdit(this, "ObjvarTypeEdit");
//	}
//
//	if(parameterData != NULL)
//	{
//		int selectedType = parameterData->getValue();
//		m_objvarTypeEditor->setValue(selectedType);
//	}
//	
//	setCurrentEditor(*m_objvarTypeEditor);
//}

// ============================================================================
