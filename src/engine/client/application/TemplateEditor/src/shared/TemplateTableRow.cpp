// ============================================================================
//
// TemplateTableRow.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateTableRow.h"
#include "TemplateTableRow.moc"

#include "TableItemColor.h"
#include "sharedFile/TreeFile.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedUtility/TemplateParameter.h"
#include "TableItemAtBase.h"
#include "TableItemBoolean.h"
#include "TableItemEnum.h"
#include "TableItemFileName.h"
#include "TableItemInt.h"
#include "TableItemFloat.h"
#include "TableItemObjvar.h"
#include "TableItemString.h"
#include "TableItemStringId.h"
#include "TableItemStruct.h"
#include "TableItemTemplate.h"
#include "TableItemTriggerVolume.h"
#include "TableItemVector.h"
#include "TemplateEditor.h"
#include "TemplateEditorUtility.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"

// ============================================================================
//
// TemplateTableRow
//
// ============================================================================

int TemplateTableRow::m_globalCount = 0;

//-----------------------------------------------------------------------------
TemplateTableRow::TemplateTableRow(TemplateTable &parentTemplateTable, int const row, TpfTemplate &tpfTemplate, TemplateData::Parameter const &parameter)
 : QWidget(&parentTemplateTable, "TemplateTableRow")
 , m_parentTemplateTable(parentTemplateTable)
 , m_childTable(NULL)
 , m_parameter(&parameter)
 , m_definedCheckTableItem(NULL)
 , m_nameTableItemColor(NULL)
 , m_legendTableItemColor(NULL)
 , m_valueTableItemBase(NULL)
 , m_templateDataParamType(parameter.type)
 , m_parameterProperties(0)
 , m_description(parameter.description.c_str())
 , m_parameterName(parameter.name.c_str())
 , m_parameterIndex(0)
 , m_single(false)
 , m_tpfTemplate(&tpfTemplate)
{
	// Add flag descriptions to this row, depending on the parameter's list type

	switch (m_parameter->list_type)
	{
		case TemplateData::LIST_LIST:
			{
				m_parameterProperty = PP_list;
			}
			break;
		case TemplateData::LIST_INT_ARRAY:
		case TemplateData::LIST_ENUM_ARRAY:
			{
				m_parameterProperty = PP_array;
			}
			break;
		default:
			{
				m_parameterProperty = PP_simple;
			}
			break;
	}

	// Add flag descriptions to this row, depending on the parameter type itself

	switch (m_parameter->type)
	{
		case TemplateData::TYPE_DYNAMIC_VAR: // Objvars themselves are automatically lists (even though their list_type is LIST_NONE)
			{
				m_parameterProperty = PP_list;
			}
			break;
		default:
			{
			}
			break;
	}

	// This row starts out, by default, as its plain list type

	m_parameterProperties = m_parameterProperty;

	// Check box

	m_definedCheckTableItem = new QCheckTableItem(&m_parentTemplateTable, QString::null);

	// Legend color

	m_legendTableItemColor = new TableItemColor(m_parentTemplateTable, "       ");
	
	// Name

	m_nameTableItemColor = new TableItemColor(m_parentTemplateTable, m_parameterName);

	// Initialize the default values

	load(m_parameterName, m_parameterIndex);

	// Put all the table items in the table

	m_parentTemplateTable.insertTemplateTableRow(row, this, m_definedCheckTableItem, m_legendTableItemColor, m_nameTableItemColor, m_valueTableItemBase);

	++m_globalCount;
}

//-----------------------------------------------------------------------------
TemplateTableRow::TemplateTableRow(TemplateTable &parentTemplateTable, int const row, TemplateData::ParamType const paramType, const char *name, const char *description)
 : QWidget(&parentTemplateTable, "TemplateTableRow")
 , m_parentTemplateTable(parentTemplateTable)
 , m_childTable(NULL)
 , m_parameter(NULL)
 , m_definedCheckTableItem(NULL)
 , m_nameTableItemColor(NULL)
 , m_legendTableItemColor(NULL)
 , m_valueTableItemBase(NULL)
 , m_templateDataParamType(paramType)
 , m_parameterProperties(PP_simple)
 , m_parameterProperty(PP_simple)
 , m_description(description)
 , m_parameterName(name)
 , m_parameterIndex(0)
 , m_single(false)
 , m_tpfTemplate(NULL)
{
	//clearParameterProperty(TemplateTableRow::PP_derived);

	// Check box

	m_definedCheckTableItem = new QCheckTableItem(&m_parentTemplateTable, QString::null);

	// Legend color

	m_legendTableItemColor = new TableItemColor(m_parentTemplateTable, "       ");
	
	// Name

	m_nameTableItemColor = new TableItemColor(m_parentTemplateTable, m_parameterName);

	if (paramType == TemplateData::TYPE_STRING)
	{
		delete m_valueTableItemBase;
		m_valueTableItemBase = new TableItemString(*this);
		m_valueTableItemBase->initDefault();
	}
	else if (paramType == TemplateData::TYPE_FLOAT)
	{
		delete m_valueTableItemBase;
		m_valueTableItemBase = new TableItemFloat(*this);
		m_valueTableItemBase->initDefault();
	}
	else if (paramType == TemplateData::TYPE_INTEGER)
	{
		delete m_valueTableItemBase;
		m_valueTableItemBase = new TableItemInt(*this);
		m_valueTableItemBase->initDefault();
	}
	else if (paramType == TemplateData::TYPE_DYNAMIC_VAR)
	{
		delete m_valueTableItemBase;
		m_valueTableItemBase = new TableItemObjvar(*this);
		m_valueTableItemBase->initDefault();
	}
	else
	{
		DEBUG_FATAL(true, ("Unexpected parameter type."));
	}

	// Put all the table items in the table

	m_parentTemplateTable.insertTemplateTableRow(row, this, m_definedCheckTableItem, m_legendTableItemColor, m_nameTableItemColor, m_valueTableItemBase);

	++m_globalCount;
}

// This verison is called by arrays, lists, and weighted lists
//-----------------------------------------------------------------------------
TemplateTableRow::TemplateTableRow(TemplateTable &parentTemplateTable, int const row, const TemplateTableRow &parentRow, TemplateData::Parameter const &parameter, int const parameterIndex, int const weightedListIndex)
 : QWidget(&parentTemplateTable, "TemplateTableRow")
 , m_parentTemplateTable(parentTemplateTable)
 , m_childTable(NULL)
 , m_parameter(&parameter)
 , m_definedCheckTableItem(NULL)
 , m_nameTableItemColor(NULL)
 , m_legendTableItemColor(NULL)
 , m_valueTableItemBase(NULL)
 , m_templateDataParamType(parameter.type)
 , m_parameterProperty(PP_simple)
 , m_parameterProperties(PP_simple)
 , m_description(parameter.description.c_str())
 , m_parameterName(parameter.name.c_str())
 , m_parameterIndex(parameterIndex)
 , m_single(true)
 , m_tpfTemplate(parentRow.getTpfTemplate())
{
	NOT_NULL(m_tpfTemplate);

	// Add flag descriptions to this row

	if (parentRow.isParameterProperty(PP_array))
	{
		m_parameterProperties |= PP_arrayItem;
	}
	else if (parentRow.isParameterProperty(PP_list))
	{
		m_parameterProperties |= PP_listItem;
	}
	else if (parentRow.isParameterProperty(PP_weightedList))
	{
		m_parameterProperties |= PP_weightedItem;
	}

	// Check box

	m_definedCheckTableItem = new QCheckTableItem(&m_parentTemplateTable, QString::null);

	// Legend color

	m_legendTableItemColor = new TableItemColor(m_parentTemplateTable, "       ");
	
	// Name

	m_nameTableItemColor = new TableItemColor(m_parentTemplateTable, m_parameterName);

	// Load the value

	load(m_parameterName, parameterIndex, weightedListIndex);

	// Put all the table items in the table

	m_parentTemplateTable.insertTemplateTableRow(row, this, m_definedCheckTableItem, m_legendTableItemColor, m_nameTableItemColor, m_valueTableItemBase);

	++m_globalCount;
}

//-----------------------------------------------------------------------------
TemplateTableRow::TemplateTableRow(TemplateTable &parentTemplateTable, int const row, SpecialRowType specialRowType)
 : QWidget(&parentTemplateTable, "TemplateTableRow")
 , m_parentTemplateTable(parentTemplateTable)
 , m_childTable(NULL)
 , m_parameter(NULL)
 , m_definedCheckTableItem(NULL)
 , m_nameTableItemColor(NULL)
 , m_legendTableItemColor(NULL)
 , m_valueTableItemBase(NULL)
 , m_parameterProperty(PP_simple)
 , m_parameterProperties(PP_simple)
 , m_templateDataParamType(TemplateData::TYPE_NONE)
 , m_parameterName()
 , m_description()
 , m_parameterIndex(0)
 , m_single(false)
 , m_tpfTemplate(NULL)
{
	if (specialRowType == BaseTemplateSpecifier)
	{
		setName("@base");
		m_description = "Specifies the base template from which data is inherited.";
	}
	else if (specialRowType == ObjvarTypeSelector)
	{
		setName("Objvar type");
		m_description = "select which objvar type this objvar value is";
	}

	// Check box

	m_definedCheckTableItem = new QCheckTableItem(&m_parentTemplateTable, QString::null);

	// Legend color

	m_legendTableItemColor = new TableItemColor(m_parentTemplateTable, "       ");
	
	// Name

	m_nameTableItemColor = new TableItemColor(m_parentTemplateTable, m_parameterName);

	if (specialRowType == BaseTemplateSpecifier)
	{
		m_tpfTemplate = &TemplateLoader::getTpfTemplate();

		load(m_parameterName.latin1(), 0);
	}
	else
	{
		DEBUG_FATAL(true, ("Unexpected parameter type."));
	}

	// Put all the table items in the table

	m_parentTemplateTable.insertTemplateTableRow(row, this, m_definedCheckTableItem, m_legendTableItemColor, m_nameTableItemColor, m_valueTableItemBase);

	++m_globalCount;
}

//-----------------------------------------------------------------------------
TemplateTableRow::~TemplateTableRow()
{
	--m_globalCount;
}

////-----------------------------------------------------------------------------
//TpfTemplate *TemplateTableRow::getParentParameter(TpfFile &tpfFile, TpfTemplate &tpfTemplate, QString const &path) const
//{
/////////////////////////
//	// Test case that found a bug in the template compiler
//	//TpfFile tpfFile2;
//	//Filename filename("C:/projects/swg/current/dsrc/sku.0/sys.server/plt.shared/loc.shared/compiled/game/object/creature/npc/base/", "base_npc", ".tpf");
//	//if (tpfFile2.loadTemplate(filename) == 0)
//	//{
//	//	bool const result1 = tpfFile2.getTemplate()->isLoaded("attributes");
//	//	UNREF(result1);
//	//	bool const result2 = tpfFile2.getTemplate()->isLoaded("minAttributes");
//	//	UNREF(result2);
//	//}
/////////////////////////
//
//	TpfTemplate *resultTpfTemplate = &tpfTemplate;
//
//	// Start checking the parent's parameters
//
//	char drive[_MAX_DRIVE];
//	char directory[_MAX_DIR];
//	char fileName[_MAX_FNAME];
//	char extension[_MAX_EXT];
//
//	_splitpath(path.latin1(), drive, directory, fileName, extension);
//
//	QDir dir(QString(drive) + QString(directory));
//	QString baseTemplateName(resultTpfTemplate->getBaseTemplateName().c_str());
//
//	bool done = false;
//
//	while (!done)
//	{
//		if (!baseTemplateName.isNull())
//		{
//			Filename fileNameTpf(dir.absPath(), baseTemplateName.latin1(), TEMPLATE_EXTENSION);
//
//			int result = tpfFile.loadTemplate(fileNameTpf);
//
//			if (result == 0)
//			{
//				// Store the current template
//
//				resultTpfTemplate = tpfFile.getTemplate();
//
//				// Get the new base template name
//
//				baseTemplateName = resultTpfTemplate->getBaseTemplateName().c_str();
//
//				// Reset the directory path
//
//				dir = (QString(drive) + QString(directory));
//
//				if (resultTpfTemplate->isLoaded(getName()))
//				{
//					done = true;
//				}
//			}
//			else
//			{
//				// Move up a directory
//
//				bool result = dir.cdUp();
//
//				if (!result)
//				{
//					resultTpfTemplate = &tpfTemplate;
//					done = true;
//				}
//			}
//		}
//		else
//		{
//			resultTpfTemplate = &tpfTemplate;
//			done = true;
//		}
//	}
//
//	return resultTpfTemplate;
//}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isParameterDerived() const
{
	bool result = false;

	if (m_tpfTemplate != NULL)
	{
		result = m_tpfTemplate->isParamPureVirtual(m_parameterName.latin1(), false, m_parameterIndex);
	}

	return result;
}

//-----------------------------------------------------------------------------
//  loadValueFromTemplate
//
//  Template tables initially set its values to default settings. When a user
//  indicates a template they want to load, this function will initialize this
//  row's internal data to the appropriate values.
//
//  Note that if this is a special row type (i.e. a row that is not created
//  around a TemplateData::Parameter), then this function will just return.
//  See TemplateTableRow(TemplateTable &parentTemplateTable, int row, int specialRowType);

//  tpfTemplate - Loaded template that contains all of the saved values.
//
//  Used by TemplateTable
//
//  See QPopumMenu, TemplateTableRow::parameterType, TemplateTableRow::openRowTypePopup
//-----------------------------------------------------------------------------
void TemplateTableRow::load(QString const &parameterName, int const parameterIndex, int const weightedListIndex, bool const changeCheckedState)
{
	m_parameterName = parameterName;
	m_parameterIndex = parameterIndex;

	// Set the value cell

	if (m_parameterName == QString("@base"))
	{
		// Setup the special at base parameter

		delete m_valueTableItemBase;
		m_valueTableItemBase = new TableItemAtBase(*this);
		m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_simple);

		TableItemAtBase *tableItemAtBase = dynamic_cast<TableItemAtBase *>(m_valueTableItemBase);
		tableItemAtBase->setPath(TemplateLoader::getTpfTemplate().getBaseTemplateName().c_str());

		setChecked(true);
		setCheckEnabled(false);

		m_parameterProperties |= PP_atBase;
	}
	else
	{
		switch (m_templateDataParamType)
		{
			case TemplateData::TYPE_INTEGER:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemInt(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);

					addParameterProperty(PP_int);
				}
				break;
			case TemplateData::TYPE_ENUM:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemEnum(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);

					addParameterProperty(PP_int);
				}
				break;
			case TemplateData::TYPE_FLOAT:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemFloat(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);

					addParameterProperty(PP_float);
				}
				break;
			case TemplateData::TYPE_BOOL:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemBoolean(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);

					addParameterProperty(PP_bool);
				}
				break;
			case TemplateData::TYPE_STRING:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemString(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_FILENAME:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemFileName(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_TEMPLATE:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemTemplate(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_STRUCT:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemStruct(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_TRIGGER_VOLUME:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemTriggerVolume(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_VECTOR:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemVector(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_STRINGID:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemStringId(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_DYNAMIC_VAR:
				{
					delete m_valueTableItemBase;
					m_valueTableItemBase = new TableItemObjvar(*this);
					m_valueTableItemBase->init(m_tpfTemplate, m_parameterName, parameterIndex, weightedListIndex, PP_undefined);
				}
				break;
			case TemplateData::TYPE_NONE:
				{
				}
				break;
			default:
				{
					if (m_parameterName != QString("@base"))
					{
						DEBUG_FATAL(true, ("TemplateTableRow::loadValueFromTemplate() - Invalid parameter: %s", m_parameterName));
					}
				}
				break;
		}

		// Set the checked state

		if (changeCheckedState)
		{
			bool const pureVirtual = TemplateLoader::getTpfTemplate().isParamPureVirtual(parameterName.latin1(), false, parameterIndex);

			if (!pureVirtual)
			{
				bool const parameterDefinedAtTopLevel = TemplateLoader::getTpfTemplate().isParamLoaded(parameterName.latin1(), false, parameterIndex);

				setChecked(parameterDefinedAtTopLevel);
			}
		}
	}

	refresh();
}

//-----------------------------------------------------------------------------
QString TemplateTableRow::getParameterName() const
{
	return m_parameterName;
}

//-----------------------------------------------------------------------------
QString TemplateTableRow::getDecoratedName() const
{
	return m_nameTableItemColor->text();
}

//-----------------------------------------------------------------------------
QString TemplateTableRow::getType() const
{
	return m_valueTableItemBase->getTypeString();
}

//-----------------------------------------------------------------------------
QString const &TemplateTableRow::getComment() const
{
	return m_description;
}

//-----------------------------------------------------------------------------
int TemplateTableRow::getRow() const
{
	return m_nameTableItemColor->row();
}

//-----------------------------------------------------------------------------
TemplateTable &TemplateTableRow::getParentTable() const
{
	return m_parentTemplateTable;
}

//-----------------------------------------------------------------------------
const TemplateData::Parameter *TemplateTableRow::getParameter() const
{
	return m_parameter;
}

////-----------------------------------------------------------------------------
//int TemplateTableRow::getParameterPropertys() const
//{
//	return m_parameterProperties;
//}

//-----------------------------------------------------------------------------
TemplateTableRow::ParameterProperty TemplateTableRow::getListType() const
{
	return m_parameterProperty;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::execParameterPropertyPopupMenu(const QPoint &popupPosition)
{
	// If the right clicked parameter is marked as defined and it is not the @base
	// parameter, then display a list of the types available for the parameter.

	if (m_definedCheckTableItem->isChecked() && !isParameterProperty(PP_atBase))
	{
		QPopupMenu popupMenu(&getParentTable());

		addPropertyToPopupMenu(popupMenu, PP_simple, m_valueTableItemBase->getTypeString());
		addPropertyToPopupMenu(popupMenu, PP_randomRange, "Random Range");
		addPropertyToPopupMenu(popupMenu, PP_dieRoll, "Dice Roll");
		addPropertyToPopupMenu(popupMenu, PP_weightedList, "Weighted List");
		addPropertyToPopupMenu(popupMenu, PP_derived, "@derived");

		// Insert the quick "Go To" navigation if the parameter type is a filename or @base
		// and this is a valid file type we can handle

		//if ((m_templateDataParamType == TemplateData::TYPE_FILENAME) || (m_parameterProperties | PP_atBase))
		if (m_templateDataParamType == TemplateData::TYPE_FILENAME)
		{
			popupMenu.insertSeparator();
			popupMenu.insertItem("Goto File", this, SLOT(goToFile()));
		}

		popupMenu.insertSeparator();
		popupMenu.insertSeparator();

		addPropertyToPopupMenu(popupMenu, PP_positive, "+ Delta");
		addPropertyToPopupMenu(popupMenu, PP_negative, "- Delta");
		addPropertyToPopupMenu(popupMenu, PP_percent, "% Delta");

		// Show the menu

		ParameterProperty result = static_cast<ParameterProperty>(popupMenu.exec(popupPosition));

		// If the parameter type is not currently simple, then set the parameter
		// type to simple

		switch (result)
		{
			case PP_simple:
			case PP_weightedList:
			case PP_randomRange:
			case PP_dieRoll:
			case PP_derived:
				{
					// If changed, set the new parameter property

					if (!isParameterProperty(result))
					{
						addParameterProperty(result);
						reload();
					}
				}
				break;
			case PP_positive:
				{
					setDeltaPositive(!isDeltaPositive());
				}
				break;
			case PP_negative:
				{
					setDeltaNegative(!isDeltaNegative());
				}
				break;
			case PP_percent:
				{
					setDeltaPercent(!isDeltaPercent());
				}
				break;
			case PP_invalid:
				{
					// Nothing in the menu selected
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Unexpected parameter type specified."));
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
int TemplateTableRow::getParameterIndex() const
{
	return m_parameterIndex;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setName(QString const &name)
{
	m_parameterName = name;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setDecoratedName(QString const &name)
{
	m_nameTableItemColor->setText(name);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setChildTable(TemplateTable &childTable)
{
	m_childTable = &childTable;
}

////-----------------------------------------------------------------------------
//void TemplateTableRow::setToUseParent()
//{
//	addParameterProperty(PP_useParent);
//
//	m_valueTableItemBase->setText(m_originalValueString);
//}
//
////-----------------------------------------------------------------------------
//void TemplateTableRow::setToUseSelf()
//{
//	clearParameterProperty(PP_useParent);
//	addParameterProperty(m_parameterProperty);
//
//	m_valueTableItemBase->setText(m_currentValueString);
//}

//-----------------------------------------------------------------------------
void TemplateTableRow::write(QFile &file)
{
	QTextStream writeFileStream(&file);

	bool const checked = m_definedCheckTableItem->isChecked();

	if (checked)
	{
		//if (m_childTable != NULL)
		//{
		//	m_childTable->setParentTemplateTable(m_parentTemplateTable);
		//	m_childTable->setParentParameterName(m_parameter->name.c_str());
		//	m_childTable->setParentTemplateTableRow(*this);
		//	
		//	if (m_tpfTemplate != NULL)
		//	{
		//		m_childTable->setParentTpfTemplate(*m_tpfTemplate);
		//	}
		//}

		if (m_parameterName == "@base")
		{
			// Remove the quotes

			FilePath filePath(m_valueTableItemBase->text().latin1());
			filePath.swapChar('"', ' ');

			writeFileStream << endl << m_parameterName << " " << filePath.getFullPath().c_str() << endl;
		}
		else if (m_parameterProperty == PP_array)
		{
			// Array special case, can't save as array = [...], but instead must save as array[0] = ..., array[1] = ...,

			if (m_childTable != NULL)
			{
				m_childTable->setParentTemplateTableRow(this);
				m_childTable->write(file);
			}
		}
		else if (m_parameter->type == TemplateData::TYPE_STRUCT)
		{
			if ((m_parameterProperty == PP_list) || (m_parameter->list_type == TemplateData::LIST_ENUM_ARRAY))
			{
				QString decoratedName(getDecoratedName());

				writeFileStream << endl << decoratedName << " = ";
			}

			if ((m_childTable == NULL) || (m_childTable->numRows() <= 0))
			{
				writeFileStream  << "[ ]";
			}
			else
			{
				if (isDeltaPositive())
				{
					writeFileStream  << "+";
				}

				writeFileStream  << "[";

				if (m_parameter->list_type == TemplateData::LIST_LIST)
				{
					m_childTable->setParentTemplateTableRow(this);
				}
				else
				{
					m_childTable->setParentTemplateTableRow(m_parentTemplateTable.getParentTemplateTableRow());
				}

				m_childTable->write(file);

				writeFileStream << "]";
			}

			if (getParentTable().getNeedsComma(*this))
			{
				writeFileStream << ", ";
			}

			//if (!getParentTable().isLastElement(*this))
			//{
			//	writeFileStream << endl;
			//}
		}
		else
		{
			// This is a simple parameter type

			if (m_childTable != NULL)
			{
				m_parentTemplateTable.setParentTemplateTableRow(m_parentTemplateTable.getParentTemplateTableRow());
			}

			QString decoratedName(getDecoratedName());

			// All other items can just be drawn out as 'name' = 'value'

			if ((m_parentTemplateTable.getParentTemplateTableRow() == NULL) ||
			    (!m_parentTemplateTable.getParentTemplateTableRow()->getParentTable().isParameterType(TemplateTable::PT_list)))
			{
				writeFileStream << endl;
			}

			writeFileStream << endl << decoratedName << " = " << m_valueTableItemBase->text();

			if (getParentTable().getNeedsComma(*this))
			{
				writeFileStream << ", ";
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::reload()
{
	ParameterProperty newParameterProperty = PP_simple;

	if (isParameterProperty(PP_weightedList))
	{
		newParameterProperty = PP_weightedList;
	}
	if (isParameterProperty(PP_randomRange))
	{
		newParameterProperty = PP_randomRange;
	}
	if (isParameterProperty(PP_dieRoll))
	{
		newParameterProperty = PP_dieRoll;
	}
	if (isParameterProperty(PP_derived))
	{
		newParameterProperty = PP_derived;
	}

	m_valueTableItemBase->init(m_tpfTemplate, m_parameterName.latin1(), m_parameterIndex, 0, newParameterProperty);

	refresh();
}

//-----------------------------------------------------------------------------
void TemplateTableRow::refresh()
{
	getParentTable().updateCell(getRow(), TemplateTableRow::CT_value);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::addParameterProperty(ParameterProperty const parameterType)
{
	// Prevent bad flags from coming in
	
	if (isSupported(parameterType)) // Does this template item support the change we want to make?
	{
		// Clear away mutually exclusive flags

		switch (parameterType)
		{
			case TemplateTableRow::PP_simple:
			case TemplateTableRow::PP_randomRange:
			case TemplateTableRow::PP_dieRoll:
			case TemplateTableRow::PP_weightedList:
				{
					clearParameterBasic();
					clearParameterListHead();
					clearParameterDeltaType();
				}

				break;
			case TemplateTableRow::PP_array:
			case TemplateTableRow::PP_list:

				{
					clearParameterBasic();
					clearParameterListHead();
				}

				break;
			case TemplateTableRow::PP_arrayItem:
			case TemplateTableRow::PP_listItem:
			case TemplateTableRow::PP_weightedItem:
				{
					clearParameterListItem();
				}

				break;
			case TemplateTableRow::PP_positive:
			case TemplateTableRow::PP_negative:
				{
					clearParameterDeltaType();
				}
				break;
			case TemplateTableRow::PP_derived:
				{
					clearParameterAll();
				}
				break;
			case TemplateTableRow::PP_useParent:
				{
					clearParameterBasic();
					clearParameterListHead();
				}
				break;
		}

		// Add in the new flag

		m_parameterProperties |= parameterType;

		// Adding the new flag may cause others to become absolete eliminate those

		checkParameterProperties(); 
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::checkParameterProperties()
{
	for (int shiftAmount = 1; shiftAmount <= PP_count; ++shiftAmount)
	{
		ParameterProperty parameterTypeToCheck = static_cast<ParameterProperty>(1 << shiftAmount);
	
		if (isParameterProperty(parameterTypeToCheck) && !isSupported(parameterTypeToCheck))
		{
			clearParameterProperty(parameterTypeToCheck);
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterProperty(ParameterProperty const parameterType)
{
	m_parameterProperties &= ~parameterType;
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isParameterProperty(ParameterProperty const parameterType) const
{
	return (m_parameterProperties & parameterType);
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isSupported(ParameterProperty const parameterType) const
{
	if (parameterType == PP_useParent)
	{
		return true; // All cells can defer to their parents
	}

	// First and foremost, we cannot change this item's underlying list type

	if (((parameterType == PP_list) &&         (m_parameterProperty != PP_list)) ||
	    ((parameterType == PP_array) &&        (m_parameterProperty != PP_array)) || 
	    ((parameterType == PP_simple) &&       (m_parameterProperty != PP_simple)) ||
	    ((parameterType == PP_weightedList) && (m_parameterProperty != PP_simple)) ||
	    ((parameterType == PP_randomRange) &&  (m_parameterProperty != PP_simple)) ||
	    ((parameterType == PP_dieRoll) &&      (m_parameterProperty != PP_simple)))
	{
		return false;
	}

	// Does our child cell support the row type?

	bool flagSupported = (m_valueTableItemBase->getSupportedParameterProperties() & parameterType);

	// Finally, does the flag pass some high-level special-case rules as well?

	if (flagSupported)
	{
		// Can't add plus/minus effects on an array, derived item, or a list item

		if ((parameterType == PP_positive) || (parameterType == PP_negative) || (parameterType == PP_percent))
		{
			if (isParameterProperty(PP_array) ||
			    isParameterProperty(PP_derived) ||
			    isParameterProperty(PP_listItem) ||
			    isParameterProperty(PP_weightedItem) ||
			    isParameterProperty(PP_bool))
			{
				flagSupported = false;
			}
		}

		// Can't add PP_negative effects on a list (you can add items to a parent list, but you cannot make a parent list smaller

		if (parameterType == PP_negative)
		{
			if (isParameterProperty(PP_list))
			{
				flagSupported = false;
			}
		}

		// Can't add @derived on an individual list item

		if (parameterType == PP_derived)
		{
			if (isParameterProperty(PP_listItem) || isParameterProperty(PP_weightedItem))
			{
				flagSupported = false; 
			}
		}

		// Value lists cannot be turned into types that return a single value

		if (parameterType == PP_weightedList || parameterType == PP_randomRange || parameterType == PP_dieRoll)
		{
			if (isParameterProperty(PP_array) || isParameterProperty(PP_list))
			{
				flagSupported = false;
			}
		}
	}

	return flagSupported;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterBasic()
{
	clearParameterProperty(PP_simple);
	clearParameterProperty(PP_randomRange);
	clearParameterProperty(PP_dieRoll);
	clearParameterProperty(PP_derived);
	clearParameterProperty(PP_weightedList);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterListHead()
{
	clearParameterProperty(PP_array);
	clearParameterProperty(PP_list);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterListItem()
{
	clearParameterProperty(PP_arrayItem);
	clearParameterProperty(PP_listItem);
	clearParameterProperty(PP_weightedItem);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterDeltaType()
{
	clearParameterProperty(PP_positive);
	clearParameterProperty(PP_negative);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterDeltaPercent()
{
	clearParameterProperty(TemplateTableRow::PP_percent);
}

//-----------------------------------------------------------------------------
void TemplateTableRow::clearParameterAll()
{
	m_parameterProperties = 0;
}

//-----------------------------------------------------------------------------
//  addPropertyToPopupMenu
//
//  This function is a helper for openRowTypePopup. It adds a row type option
//  to the specified popup menu, enabled if that option is supported by this
//  row, disabled otherwise.  Furthermore, if that option is currently selected
//  on this row, it will checkmark the item on the list.
//
//  popupMenu - QPopupMenu to potentially add an item to
//  parameterType - The type of item that this row can become
//  typeName - The name for the flag type to be added to the menu
//  associatedSlot - QT slot function to call if the menu item is chosen.
//
//  See QPopumMenu, TemplateTableRow::RowTypeFlag, TemplateTableRow::openRowTypePopup
//-----------------------------------------------------------------------------
void TemplateTableRow::addPropertyToPopupMenu(QPopupMenu &popupMenu, ParameterProperty const parameterType, QString const &typeName)
{
	if (isSupported(parameterType))
	{
		int menuItemID = popupMenu.insertItem(typeName, parameterType);

		//popupMenu.setItemEnabled(menuItemID, true); // Items are enabled by default

		// If the parameter is currently the specified parameterType, then mark
		// it as selected
		
		if (isParameterProperty(parameterType))
		{
			popupMenu.setItemChecked(menuItemID, true);
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(IntegerParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case IntegerParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case IntegerParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
			case IntegerParam::RANGE:
				{
					addParameterProperty(PP_randomRange);
				}
				break;
			case IntegerParam::DIE_ROLL:
				{
					addParameterProperty(PP_dieRoll);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(FloatParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case FloatParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case FloatParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
			case FloatParam::RANGE:
				{
					addParameterProperty(PP_randomRange);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(BoolParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case BoolParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case BoolParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(StringParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case StringParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case StringParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(VectorParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case VectorParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case VectorParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(StringIdParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case StringIdParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case StringIdParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(TriggerVolumeParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case TriggerVolumeParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case TriggerVolumeParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(DynamicVariableParam *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case DynamicVariableParam::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case DynamicVariableParam::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setParameterPropertys(StructParamOT *param)
{
	if (param != NULL)
	{
		switch (param->getType())
		{
			case StructParamOT::SINGLE:
			default:
				{
					addParameterProperty(m_parameterProperty);
				}
				break;
			case StructParamOT::WEIGHTED_LIST:
				{
					addParameterProperty(PP_weightedList);
				}
				break;
		}
	}
}

//-----------------------------------------------------------------------------
//  createTableItemFromParamType
//
//  This function is an TableItemBase factory. It determines, from the input
//  parameter type, which type of TableItemBase should be created.
//
//  parameter - A parameter struct containing what data-type it is
//
//  See Parameter, EditTableBaseItem
//-----------------------------------------------------------------------------

TableItemBase *TemplateTableRow::createTableItemFromParamType(TemplateData::ParamType paramType)
{
	TableItemBase *result = NULL;

	switch (paramType)
	{
		case TemplateData::TYPE_STRUCT:
			{
				result = new TableItemStruct(*this);
			}
			break;
		case TemplateData::TYPE_TRIGGER_VOLUME:
			{
				result = new TableItemTriggerVolume(*this);
			}
			break;
		case TemplateData::TYPE_VECTOR:
			{
				result = new TableItemVector(*this);
			}
			break;
		case TemplateData::TYPE_STRINGID:
			{
				result = new TableItemStringId(*this);
			}
			break;
		case TemplateData::TYPE_DYNAMIC_VAR:
			{
				result = new TableItemObjvar(*this);
			}
			break;
		case TemplateData::TYPE_COMMENT:
			{
				// Ignore comments
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected template data parameter type."));
				//return new TableItemBase(*this); // DHERMAN This is bad news here in the long run (TableItemBase should be abstract when this program is over)
			}
			break;
	}

	return result;
}

//-----------------------------------------------------------------------------
TableItemBase *TemplateTableRow::createTableItemFromSpecialType(SpecialRowType const specialRowType)
{
	TableItemBase *result = NULL;

	switch (specialRowType)
	{
		case BaseTemplateSpecifier:
			{
				result = new TableItemAtBase(*this);
			}
			break;

		case ObjvarTypeSelector:
			{
				result = new TableItemObjvar(*this);
			}
			break;

		default:
			{
				DEBUG_FATAL(true, ("Unexpected template data parameter type."));
			}
			break;
	}

	return result;
}

//-----------------------------------------------------------------------------
QCheckTableItem *TemplateTableRow::getColumnEdited() const
{
	return m_definedCheckTableItem;
}

//-----------------------------------------------------------------------------
QTableItem *TemplateTableRow::getColumnName() const
{
	return m_nameTableItemColor;
}

//-----------------------------------------------------------------------------
QTableItem *TemplateTableRow::getColumnLegend() const
{
	return m_legendTableItemColor;
}

//-----------------------------------------------------------------------------
TableItemBase *TemplateTableRow::getColumnValue() const
{
	return m_valueTableItemBase;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setChecked(bool const checked)
{
	m_definedCheckTableItem->setChecked(checked);
	m_valueTableItemBase->setEnabled(checked);

	// Set the color of the name cell

	TemplateEditor *templateEditor = dynamic_cast<TemplateEditor *>(qApp->mainWidget());

	if (templateEditor != NULL)
	{
		if (m_definedCheckTableItem->isChecked() || (m_tpfTemplate == NULL) || (m_tpfTemplate->getParentFile() == NULL))
		{
			// Set the color to reflect the loaded tpf file

			QFileInfo fileInfo(TemplateLoader::getTpfPath());
			QColor color;
			int index;
			templateEditor->getColor(fileInfo.fileName(), color, index);

			m_legendTableItemColor->setCellColor(color);
			m_legendTableItemColor->setTextNumber(QString::number(index));

			m_nameTableItemColor->setTextColor(QColor("black"));
		}
		else if (m_tpfTemplate != NULL)
		{
			// Set the color to reflect the inherited tpf file

			// m_tpfTemplate->getBaseTemplateName().c_str() needs to be determined when the tpfTemplate
			// is determined from a struct or from a file

			std::string fileName = m_tpfTemplate->getParentFile()->getFileName();

			QColor color;
			int index;
			templateEditor->getColor(fileName.c_str(), color, index);

			m_legendTableItemColor->setCellColor(color);
			m_legendTableItemColor->setTextNumber(QString::number(index));
			m_nameTableItemColor->setTextColor(QColor("red"));
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setCheckEnabled(bool const enabled)
{
	m_definedCheckTableItem->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isChecked() const
{
	return m_definedCheckTableItem->isChecked();
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isCheckEnabled() const
{
	return m_definedCheckTableItem->isEnabled();
}

// Loads the tpf value if the parameter is the @base or if the parameter is a
// filename
//-----------------------------------------------------------------------------
void TemplateTableRow::goToFile()
{
	QString path;

	if (m_templateDataParamType == TemplateData::TYPE_FILENAME)
	{
		StringParam *param = getTpfTemplate()->getStringParam(m_parameterName.latin1());
		NOT_NULL(param);
		
		if (param->isLoaded())
		{
			path = param->getValue().c_str();
		}
	}
	//else if (m_parameterProperties | PP_atBase)
	//{
	//	path = m_valueTableItemBase->text();
	//}

	if (!path.isEmpty())
	{
		TemplateEditor *templateEditor = dynamic_cast<TemplateEditor *>(qApp->mainWidget());
	
		if (templateEditor != NULL)
		{
			char treeFilePath[_MAX_PATH];
	
			if (TreeFile::getPathName(path.latin1(), treeFilePath, sizeof(treeFilePath)))
			{
				char *data = strstr(treeFilePath, "data");
	
				if (data != NULL)
				{
					*(data + 0) = 'd';
					*(data + 1) = 's';
					*(data + 2) = 'r';
					*(data + 3) = 'c';
				}
	
				FilePath filePath(treeFilePath);
				std::string path(filePath.get(FilePath::drive | FilePath::directory | FilePath::fileName) + ".tpf");
	
				templateEditor->open(path.c_str());
			}
			else
			{
				TemplateEditorUtility::report("Error using GOTO, make sure your working directory is correct.");
			}
		}
	}
}

//-----------------------------------------------------------------------------
TpfTemplate *TemplateTableRow::getTpfTemplate() const
{
	return m_tpfTemplate;
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isSingle() const
{
	return m_single;
}

//-----------------------------------------------------------------------------
TemplateData::ParamType TemplateTableRow::getTemplateDataParamType() const
{
	return m_templateDataParamType;
}

//-----------------------------------------------------------------------------
int TemplateTableRow::getGlobalCount()
{
	return m_globalCount;
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setDeltaPositive(bool const enabled)
{
	if (enabled)
	{
		clearParameterProperty(PP_negative);
		addParameterProperty(PP_positive);
	}
	else
	{
		clearParameterProperty(PP_positive);
		clearParameterProperty(PP_negative);
	}

	// Update the table item with the changes

	m_valueTableItemBase->update();
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setDeltaNegative(bool const enabled)
{
	if (enabled)
	{
		clearParameterProperty(PP_positive);
		addParameterProperty(PP_negative);
	}
	else
	{
		clearParameterProperty(PP_positive);
		clearParameterProperty(PP_negative);
	}

	// Update the table item with the changes

	m_valueTableItemBase->update();
}

//-----------------------------------------------------------------------------
void TemplateTableRow::setDeltaPercent(bool const enabled)
{
	if (enabled)
	{
		addParameterProperty(PP_percent);
	}
	else
	{
		clearParameterProperty(PP_percent);
	}

	// Update the table item with the changes

	m_valueTableItemBase->update();
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isDeltaPositive() const
{
	return (m_parameterProperties & PP_positive);
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isDeltaNegative() const
{
	return (m_parameterProperties & PP_negative);
}

//-----------------------------------------------------------------------------
bool TemplateTableRow::isDeltaPercent() const
{
	return (m_parameterProperties & PP_percent);
}

// ============================================================================
