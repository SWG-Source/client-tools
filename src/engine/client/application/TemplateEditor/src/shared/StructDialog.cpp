// ============================================================================
//
// StructDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "StructDialog.h"

#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"

// ============================================================================
//
// StructDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
StructDialog::StructDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow)
 : StaticListDialog(parent, name, parentRow)
{
	TemplateData::Parameter const *parameter = parentRow.getParameter();
	NOT_NULL(parameter);

	TemplateData const *structTemplateData = parentRow.getParentTable().getTemplateData().getStruct(parameter->extendedName.c_str());
	NOT_NULL(structTemplateData);

	TpfTemplate *tpfTemplate = parentRow.getTpfTemplate();
	NOT_NULL(tpfTemplate);

	bool const check = tpfTemplate->isParamLoaded(parameter->name.c_str(), false);
	DEBUG_FATAL(!check, ("Parameter not found in the tpf template: %s", parameter->name.c_str()));
	UNREF(check);

	StructParamOT *structParamOT = tpfTemplate->getStructParamOT(parameter->name.c_str(), parentRow.getParameterIndex());
	NOT_NULL(structParamOT);

	TpfTemplate *structTpfTemplate = dynamic_cast<TpfTemplate *>(structParamOT->getValue());
	NOT_NULL(structTpfTemplate);

	m_templateTable->load(*structTemplateData, structTpfTemplate);
	m_templateTable->setCheckedAll(true);
}

//-----------------------------------------------------------------------------
void StructDialog::setValue(TpfTemplate &structTemplate, std::string const &path)
{
	UNREF(structTemplate);
	UNREF(path);

	//m_templateTable->setValues();
}

// ============================================================================
