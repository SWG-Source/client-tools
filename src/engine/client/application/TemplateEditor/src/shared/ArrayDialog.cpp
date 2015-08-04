// ============================================================================
//
// ArrayDialog.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ArrayDialog.h"
#include "TemplateTable.h"
#include "TemplateTableRow.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TemplateLoader.h"

// ============================================================================
//
// ArrayDialog
//
// ============================================================================

//-----------------------------------------------------------------------------
ArrayDialog::ArrayDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow)
 : StaticListDialog(parent, name, parentTemplateTableRow)
{
	m_templateTable->setTemplateData(parentTemplateTableRow.getParentTable().getTemplateData());

	TemplateData::Parameter const *parameter = getTemplateTableRow().getParameter();

	switch (parameter->list_type)
	{
		case TemplateData::LIST_INT_ARRAY:
			{
				// Init as a numeric array (item[0], item[1], item[2], etc.)

				for(int row = 0; row < parameter->list_size; ++row)
				{
					TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, row, parentTemplateTableRow, *parameter, row);

					templateTableRow->setName(QString::number(row));
				}
			}
			break;
		case TemplateData::LIST_ENUM_ARRAY:
			{
				// Init as an enumeration array (item[ET_FirstEnumType], item[ET_SecondEnumType])

				TemplateData &templateData = TemplateLoader::getTemplateData();
				TemplateData::EnumList const *enumList = templateData.getEnumList(parameter->enum_list_name, false);
				TemplateData::EnumList::const_iterator iterTemplateDataEnumList = enumList->begin();
				int row = 0;

				m_templateTable->setTemplateData(getTemplateTableRow().getParentTable().getTemplateData());

				for(; iterTemplateDataEnumList != enumList->end(); ++iterTemplateDataEnumList)
				{
					TemplateData::EnumData const &enumData = *iterTemplateDataEnumList;

					TemplateTableRow *templateTableRow = new TemplateTableRow(*m_templateTable, row, parentTemplateTableRow, *parameter, enumData.value);

					QString text;
					text.sprintf("%s[%s]", parentTemplateTableRow.getParameterName().latin1(), enumData.name.c_str());

					templateTableRow->setDecoratedName(text);
					templateTableRow->setChecked(true);
					templateTableRow->setCheckEnabled(false);

					++row;
				}
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected parameter list type."));
			}
			break;
	}
}

// ============================================================================
