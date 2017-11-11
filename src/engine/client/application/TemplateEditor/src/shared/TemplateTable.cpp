// ============================================================================
//
// TemplateTable.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateTable.h"
#include "TemplateTable.moc"

#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateDataIterator.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"

#include "TableItemColor.h"
#include "TableItemBase.h"
#include "TemplateEditorManager.h"
#include "TemplateEditorUtility.h"
#include "TemplateLoader.h"
#include "TemplateTableRow.h"
#include "WorkingDirectories.h"

// ============================================================================
//
// TemplateTable
//
// ============================================================================

TemplateEditorManager *TemplateTable::ms_templateEditorManager = NULL;
int                    TemplateTable::m_globalCount = 0;

//-----------------------------------------------------------------------------
TemplateTable::TemplateTable(QWidget *parent, const char *name)
 : QTable(parent, name)
 , m_templateData(NULL)
 , m_parameterType(PT_none)
// , m_parentParameterName()
// , m_parentTemplateTable(NULL)
// , m_parentTpfTemplate(NULL)
 , m_parentTemplateTableRow(NULL)
 , m_optimizatedRowInsert(false)
 , m_currentOptimizedRow(0)
{
	// Initialize table appearance

	setSelectionMode(QTable::SingleRow);
	setFocusStyle(QTable::FollowStyle);
	
	horizontalHeader()->hide();
	setTopMargin(0);
	verticalHeader()->hide();
	setLeftMargin(0);

	// When the user selects a cell, let the approprtiate TemplateTableRow know about it.

	connect(this, SIGNAL(currentChanged(int, int)), SLOT(setCurrentCell(int, int)));
	connect(this, SIGNAL(clicked(int, int, int, const QPoint &)), SLOT(toggleEdit(int)));
	connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint &)), SLOT(execParameterPropertyPopupMenu(int, int, const QPoint &)));

	// Initialize data

	clearAll();

	++m_globalCount;
}

//-----------------------------------------------------------------------------
TemplateTable::~TemplateTable()
{
	--m_globalCount;
}

//-----------------------------------------------------------------------------
bool TemplateTable::isParameterType(ParameterType const parameterType) const
{
	return (m_parameterType == parameterType);
}

//-----------------------------------------------------------------------------
void TemplateTable::setParameterType(ParameterType const parameterType)
{
	m_parameterType = parameterType;
}

//-----------------------------------------------------------------------------
TemplateData const &TemplateTable::getTemplateData() const
{
	NOT_NULL(m_templateData);

	return *m_templateData;
}

// templateData - Contains all the parameters to be processed for this table.
//                This could be a TemplateData from a TemplateDefinitionFile or from a struct.
// tpfTemplate  - A TpfTemplate from a struct needs to be passed in since it
//                can't be found by searching through tpf files. At least I do
//                not have that functionality setup. If the TpfTemplate passed
//                in is not null then this template table is loading a struct.
//-----------------------------------------------------------------------------
void TemplateTable::load(TemplateData const &templateData, TpfTemplate *tpfTemplate, bool const quickLoad, bool const addBaseParameter)
{
	setName(templateData.getName().c_str());

	m_templateData = &templateData;

	clearAll();

	TemplateDataIterator iterTemplateData(templateData);

	int const iterTemplateDataSize = iterTemplateData.size();
	UNREF(iterTemplateDataSize);

	setOptimizedRowInsertEnabled(true);
	
	// Add a new row to the table
	
	setNumRows(iterTemplateData.size() + 1);

	int insertionRow = 0;

	if (addBaseParameter && !TemplateLoader::getTpfTemplate().getBaseTemplateName().empty())
	{
		// Insert the special "@base" template row

		new TemplateTableRow(*this, 0, TemplateTableRow::BaseTemplateSpecifier);

		insertionRow = 1;
	}

	// Add the new rows to the table

	bool const wasVisible = isVisible();
	
	if (quickLoad && wasVisible)
	{
		hide();
	}

	while (!iterTemplateData.end())
	{
		if (!quickLoad)
		{
			qApp->processEvents();
		}
	
		// Create the next row
	
		TemplateData::Parameter const &parameter = *(*iterTemplateData);

		// This is a sanity check

		TemplateData::Parameter const *parameterCheck = m_templateData->getParameter(parameter.name.c_str(), true);
		NOT_NULL(parameterCheck);

		// Don't allow comments to be added to a template table row

		if (parameter.type != TemplateData::TYPE_COMMENT)
		{
			// Figure out what TpfTemplate this parameter is from

			TpfTemplate *tpfTemplateForParameter = NULL;
			bool paramLoaded = false;
		
			if (tpfTemplate != NULL)
			{
				paramLoaded = tpfTemplate->isParamLoaded(parameter.name, false);

				if (paramLoaded)
				{
					tpfTemplateForParameter = tpfTemplate;
				}
			}

			if (tpfTemplateForParameter == NULL)
			{
				tpfTemplateForParameter = getTpfTemplateForParameter(parameter.name.c_str());
			}

			if (tpfTemplateForParameter != NULL)
			{
				new TemplateTableRow(*this, insertionRow, *tpfTemplateForParameter, parameter);

				++insertionRow;
			}
			else
			{
				// Error in finding the parameter

				QString error;
				error.sprintf("Error adding parameter to the table: %s", parameter.name);
				TemplateEditorUtility::report(error);
			}
		}

#ifdef WIN32
		Sleep(15);
#endif // WIN32

		// Move to the next parameter

		iterTemplateData.next();
	}

	// Set the loaded row count
	
	setNumRows(insertionRow);

	if (!quickLoad)
	{
		// Autosize the columns so that the data fits well

		adjustColumn(TemplateTableRow::CT_edited);
		adjustColumn(TemplateTableRow::CT_legend);
		adjustColumn(TemplateTableRow::CT_name);
		adjustColumn(TemplateTableRow::CT_value);

		// Although I set the second (parameter data) column of the table to
		// .. stretch (see setColumnStretchable, above), I could not get the
		// .. table to update itself properly until it happened to be resized.
		// .. Updates and repaints wouldn't work, so I'm hacking a manual resize
		// .. here. Feel free to replace this code if you ever find a more
		// .. elegant solution - Dherman

		resize(width() - 1, height());
		resize(width() + 1, height()); // Resize it back or else every time user loads a file the table size shrinks
	}

	if (quickLoad && wasVisible)
	{
		show();
	}

	setOptimizedRowInsertEnabled(false);
}

// Save this table's contents to a file. For the most part, this calls on the
// children rows to save themselves, but occasionally the table is responsible
// for saving out section headers (i.e. which tdf the following rows belong to).
// If the templateData param is NULL, that means the table doesn't have to worry
// about saving headers.
//-----------------------------------------------------------------------------
void TemplateTable::write(QFile &file, bool const firstEntry)
{
	QTextStream textStream(&file);
	int const numItems = numRows();
	TemplateTableRow *rowToSave = NULL;
	static QString currentTdf;
	static QString latestTdf;
	static QStringList tdfStringList;
	static QStringList versionStringList;

	if (firstEntry)
	{
		// This needs to be cleared each time a new file is written

		currentTdf = "";
		latestTdf = "";
		tdfStringList.clear();

		// Build the tdf list which this file inherits from

		TemplateDefinitionFile const *TemplateDefinitionFile = m_templateData->getTdf();

		while (TemplateDefinitionFile != NULL)
		{
			tdfStringList.push_back(TemplateDefinitionFile->getTemplateFilename().c_str());
			versionStringList.push_back(QString::number(TemplateDefinitionFile->getHighestVersion()));

			TemplateDefinitionFile = TemplateDefinitionFile->getBaseDefinitionFile();
		}
	}
	
	for (int currRow = 0; currRow < numItems; ++currRow)
	{
		rowToSave = getTemplateTableRow(currRow);

		QString parameterName(rowToSave->getParameterName().latin1());

		if (firstEntry)
		{
			TemplateDefinitionFile const *TemplateDefinitionFile = TemplateLoader::getTemplateData().getTdfForParameter(parameterName);

			if (TemplateDefinitionFile != NULL)
			{
				latestTdf = TemplateDefinitionFile->getTemplateFilename().c_str();

				// Write the section header if the following parameters are from a new TDF

				if (currentTdf != latestTdf)
				{
					while (!tdfStringList.empty() && (latestTdf != tdfStringList.front()))
					{
						writeSectionHeader(textStream, tdfStringList.front(), versionStringList.front());

						tdfStringList.pop_front();
						versionStringList.pop_front();
					}

					if (!tdfStringList.empty())
					{
						tdfStringList.pop_front();
						versionStringList.pop_front();
					}

					currentTdf = latestTdf;

					writeSectionHeader(textStream, currentTdf, QString::number(TemplateDefinitionFile->getHighestVersion()));
				}
			}
		}

		// Save this parameter

		rowToSave->write(file);
	}

	if (firstEntry)
	{
		// If there is any section headers left, write them out

		while (!tdfStringList.empty())
		{
			writeSectionHeader(textStream, tdfStringList.front(), versionStringList.front());

			tdfStringList.pop_front();
			versionStringList.pop_front();
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTable::writeSectionHeader(QTextStream &textStream, QString const &tdf, QString const &version)
{
	// Class

	textStream << "\n";
	textStream << "@class ";

	// Template filename

	textStream << tdf;

	// Version

	textStream << " "<< version.latin1() << "\n";
}

////-----------------------------------------------------------------------------
//void TemplateTable::setNumTemplateTableRows(int numRows)
//{
//	m_previousRow = m_previousColumn = -1;
//
//	QTable::setNumRows(numRows);
//	QTable::setNumCols(TemplateTableRow::CT_count);
//
//	setColumnStretchable(TemplateTableRow::CT_edited, false);
//	setColumnStretchable(TemplateTableRow::CT_legend, false);
//	setColumnStretchable(TemplateTableRow::CT_name, false);
//	setColumnStretchable(TemplateTableRow::CT_value, true);
//}

//-----------------------------------------------------------------------------
void TemplateTable::setTemplateEditorManager(TemplateEditorManager &manager)
{
	ms_templateEditorManager = &manager;
}

//-----------------------------------------------------------------------------
void TemplateTable::setNumRows(int numRows)
{
	m_previousRow = m_previousColumn = -1;

	QTable::setNumRows(numRows);
	QTable::setNumCols(TemplateTableRow::CT_count);

	if (numRows > 0)
	{
		setColumnStretchable(TemplateTableRow::CT_edited, false);
		setColumnStretchable(TemplateTableRow::CT_legend, false);
		setColumnStretchable(TemplateTableRow::CT_name, false);
		setColumnStretchable(TemplateTableRow::CT_value, true);
	}
}

////-----------------------------------------------------------------------------
//void TemplateTable::resizeData(int)
//{
//}

//-----------------------------------------------------------------------------
TemplateTableRow *TemplateTable::getTemplateTableRow(int const row) const
{
	TemplateTableRow *result = NULL;

	if ((row >= 0) && !m_intToTemplateTableRowMap.empty())
	{
		unsigned int const count = m_intToTemplateTableRowMap.size();
		UNREF(count);

		IntToTemplateTableRowMap::const_iterator iterIntToTemplateTableRowMap = m_intToTemplateTableRowMap.find(row);

		if (iterIntToTemplateTableRowMap != m_intToTemplateTableRowMap.end())
		{
			result = iterIntToTemplateTableRowMap->second;
			NOT_NULL(result);
		}
	}

	return result;
}

////-----------------------------------------------------------------------------
//QTableItem *TemplateTable::item(int row, int column) const
//{
//	QTableItem
//	TemplateTableRow *selectedRow = getTemplateTableRow(row);
//
//	if (selectedRow == NULL)
//	{
//		return NULL;
//	}
//
//	QTableItem *selectedItem = NULL;
//
//	switch (column)
//	{
//		case TemplateTableRow::CT_edited:
//			{
//				selectedItem = dynamic_cast<QTableItem *>(selectedRow->getColumnEdited());
//			}
//			break;
//		case TemplateTableRow::CT_legend:
//			{
//				selectedItem = dynamic_cast<QTableItem *>(selectedRow->getColumnLegend());
//			}
//			break;
//		case TemplateTableRow::CT_name:
//			{
//				selectedItem = dynamic_cast<QTableItem *>(selectedRow->getColumnName());
//			}
//			break;
//		case TemplateTableRow::CT_value:
//			{
//				selectedItem = dynamic_cast<QTableItem *>(selectedRow->getColumnValue());
//			}
//			break;
//		default:
//			{
//				if (column != -1)
//				{
//					DEBUG_FATAL(true, ("Unexpected column type."));
//				}
//			}
//			break;
//	}
//
//	return selectedItem;
//}

// Add a template row at the specified position. If a row already exists there,
// push it down a column to make more space for the new template, else just
// insert it.
//-----------------------------------------------------------------------------
void TemplateTable::insertTemplateTableRow(int const row, TemplateTableRow *templateTableRow, QCheckTableItem *m_definedCheckTableItem, TableItemColor *m_legendColorTableItem, TableItemColor *m_nameColorTableItem, TableItemBase *m_valueBaseTableItem)
{
	int const rowCount = numRows();

	int const newRow = m_optimizatedRowInsert ? m_currentOptimizedRow : row;

	if (!m_optimizatedRowInsert)
	{
		// Add a new row

		insertRows(row, 1);
	}

	// Put the table items in the row

	setItem(newRow, 0, m_definedCheckTableItem);
	setItem(newRow, 1, m_legendColorTableItem);
	setItem(newRow, 2, m_nameColorTableItem);
	setItem(newRow, 3, m_valueBaseTableItem);

	if ((rowCount != 0) && (newRow < rowCount))
	{
		// Shift all the old rows

		IntToTemplateTableRowMap templateIntToTemplateTableRowMap;
		IntToTemplateTableRowMap::iterator iterIntToTemplateTableRowMap = m_intToTemplateTableRowMap.begin();

		int currentRow = 0;

		for (; iterIntToTemplateTableRowMap != m_intToTemplateTableRowMap.end(); ++iterIntToTemplateTableRowMap)
		{
			if (currentRow < newRow)
			{
				templateIntToTemplateTableRowMap.insert(*iterIntToTemplateTableRowMap);
			}
			else
			{
				templateIntToTemplateTableRowMap.insert(std::make_pair(iterIntToTemplateTableRowMap->first + 1, iterIntToTemplateTableRowMap->second));
			}

			++currentRow;
		}

		m_intToTemplateTableRowMap.clear();

		m_intToTemplateTableRowMap = templateIntToTemplateTableRowMap;
	}
	else if (m_optimizatedRowInsert)
	{
		DEBUG_FATAL(true, ("Trying to insert a row that is out of range."));
	}

	// Insert the new row

	m_intToTemplateTableRowMap.insert(std::make_pair(newRow, templateTableRow));

	//insertWidget(row, &templateRowToAdd);

	//int totalRows = numRows();
	//
	//row = clamp<int>(0, row, totalRows);
	//
	//if (row == totalRows || m_intToRowMap[row] != NULL)
	//{
	//	m_previousRow = m_previousColumn = -1; // Hack: Clearing last item seems to solve paint issues, once I figure out tables better I'll find out how to do this right
	//
	//	QTable::setNumRows(totalRows + 1);
	//
	//	// Shift over all of rows below and included 'row' down
	//
	//	for (int currRow = totalRows; currRow > row; --currRow)
	//	{
	//		if (m_intToRowMap[currRow - 1] != NULL)
	//		{
	//			insertTemplateTableRow(currRow, *(m_intToRowMap[currRow - 1]));
	//			clearTemplateRow(currRow - 1, false);
	//		}
	//	}
	//
	//	// Autosize the columns so that the data fits well
	//
	//	adjustColumn(TemplateTableRow::CT_edited);
	//	adjustColumn(TemplateTableRow::CT_name);
	//	adjustColumn(TemplateTableRow::CT_legend);
	//}
	//
	//// Finally, add the new row into the open slot!
	//
	//m_intToRowMap[row] = &templateRowToAdd;
	//
	//templateRowToAdd.getColumnEdited()->setRow(row);
	//templateRowToAdd.getColumnEdited()->setCol(TemplateTableRow::CT_edited);
	//
	//templateRowToAdd.getColumnLegend()->setRow(row);
	//templateRowToAdd.getColumnLegend()->setCol(TemplateTableRow::CT_legend);
	//
	//templateRowToAdd.getColumnName()->setRow(row);
	//templateRowToAdd.getColumnName()->setCol(TemplateTableRow::CT_name);
	//
	//templateRowToAdd.getColumnValue()->setRow(row);
	//templateRowToAdd.getColumnValue()->setCol(TemplateTableRow::CT_value);
	//
	//updateCell(row, TemplateTableRow::CT_edited);
	//updateCell(row, TemplateTableRow::CT_name);
	//updateCell(row, TemplateTableRow::CT_legend);
	//updateCell(row, TemplateTableRow::CT_value);

	++m_currentOptimizedRow;
}

//-----------------------------------------------------------------------------
void TemplateTable::clearTemplateRow(int const row)
{
	unsigned int const intToTemplateTableRowMapSize = m_intToTemplateTableRowMap.size();
	UNREF(intToTemplateTableRowMapSize);

	IntToTemplateTableRowMap::iterator iterIntToTemplateTableRowMap = m_intToTemplateTableRowMap.find(row);

	if (iterIntToTemplateTableRowMap != m_intToTemplateTableRowMap.end())
	{
		// Delete the template table row object

		TemplateTableRow *templateTableRow = iterIntToTemplateTableRowMap->second;
		delete templateTableRow;
		templateTableRow = NULL;

		// Remove the link to the template table row object

		m_intToTemplateTableRowMap.erase(iterIntToTemplateTableRowMap);

		//// Clear all the cells
		//
		//clearCell(row, 0);
		//clearCell(row, 1);
		//clearCell(row, 2);
		//clearCell(row, 3);
	}

	// Delete the row

	removeRow(row);
}

////-----------------------------------------------------------------------------
//void TemplateTable::clearCell(int row, int col)
//{
//	// Internal use only, use clearTemplateRow instead
//
//	if (col == TemplateTableRow::CT_value)
//	{
//		endEdit(m_previousRow, m_previousColumn, true, true);
//
//		clearCellWidget(row, col);
//		//m_intToRowMap[row] = NULL;
//
//		if (row == m_previousRow)
//		{
//			m_previousRow = m_previousColumn = -1;
//		} 
//	}
//
//}

////-----------------------------------------------------------------------------
//QWidget *TemplateTable::cellWidget(int row, int col) const
//{
//	QWidget *result = NULL;
//
//	IntToWidgetMap::const_iterator iter = m_intToWidgetMap.find(indexOf(row, col));
//
//	if (iter != m_intToWidgetMap.end())
//	{
//		result = (*iter).second;
//	}
//	else
//	{
//		result = NULL;
//	}
//
//	return result;
//}

////-----------------------------------------------------------------------------
//void TemplateTable::insertWidget(int row, int col, QWidget *widget)
//{
//	m_intToWidgetMap[indexOf(row, col)] = widget;
//}

////-----------------------------------------------------------------------------
//void  TemplateTable::clearCellWidget(int row, int col)
//{
//	m_intToWidgetMap[indexOf(row, col)] = NULL;
//}

//-----------------------------------------------------------------------------
void TemplateTable::clearAll()
{
	int rowCount = numRows();

	bool const wasVisible = isVisible();

	if (wasVisible)
	{
		hide();
	}

	while (rowCount > 0)
	{
		clearTemplateRow(rowCount - 1);
		rowCount = numRows();
	}

	if (wasVisible)
	{
		show();
	}

	//m_intToRowMap.clear();
	//m_intToWidgetMap.clear();

	m_previousRow = -1;
	m_previousColumn = -1;

	//rowTotal = numRows();
	//
	//DEBUG_FATAL((rowTotal > 0), ("There should be no TemplateTableRow objects allocated for this table: %d allocated", rowTotal));

	//setNumRows(0);
	//setNumCols(0);
}

//-----------------------------------------------------------------------------
void TemplateTable::setCurrentCell(int row, int col)
{
	//if ((row >= 0) && 
	//    (row < numRows()) &&
	//    (col >= 0) &&
	//	(col < numCols()))
	//{
	//	if ((m_previousRow != row) || (m_previousColumn != col))
	//	{
	//		m_previousRow = row;
	//		m_previousColumn = col;
	//
	//		QTable::setCurrentCell(row, col);
	//		
	//		if (col == TemplateTableRow::CT_value)
	//		{
	//			QTable::editCell(row, col);
	//		}
	//
	//		if (ms_templateEditorManager != NULL)
	//		{
	//			TemplateTableRow *templateTableRow = getTemplateTableRow(row);
	//	
	//			ms_templateEditorManager->describeRow(templateTableRow);
	//		}
	//	}
	//}

	if (isVisible())
	{
		QTable::setCurrentCell(row, col);

		if (ms_templateEditorManager != NULL)
		{
			TemplateTableRow *templateTableRow = getTemplateTableRow(row);

			if (templateTableRow != NULL)
			{
				ms_templateEditorManager->describeRow(templateTableRow);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTable::endEditing()
{
	int row = currentRow();

	endEdit(row, TemplateTableRow::CT_value, true, true);
}

//-----------------------------------------------------------------------------
void TemplateTable::toggleEdit(int row)
{
	if (row >= 0)
	{
		int const column = currentColumn();

		if (column == TemplateTableRow::CT_edited)
		{
			TemplateTableRow *templateTableRow = getTemplateTableRow(row);

			if ((templateTableRow != NULL) && templateTableRow->isCheckEnabled())
			{
				templateTableRow->setChecked(!templateTableRow->isChecked());
				
				// See if we need to set the default value again

				if (!templateTableRow->isChecked())
				{
					templateTableRow->reload();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTable::execParameterPropertyPopupMenu(int row, int column, const QPoint &pos)
{
	TemplateTableRow *templateTableRow = getTemplateTableRow(row);

	if ((templateTableRow != NULL) && (column == TemplateTableRow::CT_name))
	{
		templateTableRow->execParameterPropertyPopupMenu(pos);
	}
}

//-----------------------------------------------------------------------------
QWidget *TemplateTable::beginEdit(int row, int column, bool replace)
{
	UNREF(replace);

	int const columnCount = numCols();
	UNREF(columnCount);

	int const rowCount = numRows();
	UNREF(rowCount);

	QWidget *cellEditorWidget = NULL;

	if (column == TemplateTableRow::CT_value)
	{
		QTableItem *tableItem = item(row, column);

		if ((tableItem != NULL) && tableItem->isEnabled())
		{
			cellEditorWidget = tableItem->createEditor();

			//// Set the editor text to what the table item was displaying
			//
			//cellEditorWidget->setText(tableItem->getText());

			if (cellEditorWidget != NULL)
			{
				if (cellEditorWidget != cellWidget(row, column))
				{
					setCellWidget(row, column, cellEditorWidget);
				}

				cellEditorWidget->show();
				//cellEditorWidget->setFocus();
			}
			else
			{
				TemplateEditorUtility::report("Editor missing for parameter.");
			}
		}
	}

	return cellEditorWidget;
}

//-----------------------------------------------------------------------------
void TemplateTable::endEdit(int row, int column, bool accept, bool replace)
{
	UNREF(accept);
	UNREF(replace);

	if (isVisible() && (column == TemplateTableRow::CT_value))
	{
		// In the template table, table items are responsible for deleting their
		// own items.

		QWidget *cellEditorWidget = cellWidget(row, column);

		if ((cellEditorWidget != NULL) && cellEditorWidget->isVisible())
		{
			QTableItem *tableItem = item(row, column);

			if (tableItem != NULL)
			{
				tableItem->setContentFromEditor(cellEditorWidget);
				cellEditorWidget->hide();

				// Save the template every time a change is made

				if (ms_templateEditorManager != NULL)
				{
					ms_templateEditorManager->autoSave();
				}

				//emit valueChanged(row, column);

				//TemplateTableRow *templateTableRow = getTemplateTableRow(row);
				//NOT_NULL(templateTableRow);
				//
				//templateTableRow->refresh();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateTable::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	mouseEvent->accept();
}

//-----------------------------------------------------------------------------
TpfTemplate *TemplateTable::getTpfTemplateForParameter(QString const &name)
{
	TpfTemplate *tpfTemplate = &TemplateLoader::getTpfTemplate();

	if (!tpfTemplate->isParamLoaded(name.latin1(), false))
	{
		TpfFile *tpfFile = TemplateLoader::getTpfFileForParameter(name);

		if (tpfFile == NULL)
		{
			tpfTemplate = NULL;
		}
		else
		{
			tpfTemplate = tpfFile->getTemplate();
		}
	}

	return tpfTemplate;
}

//-----------------------------------------------------------------------------
void TemplateTable::setTemplateData(TemplateData const &templateData)
{
	m_templateData = &templateData;
}

//-----------------------------------------------------------------------------
int TemplateTable::getGlobalCount()
{
	return m_globalCount;
}

//-----------------------------------------------------------------------------
void TemplateTable::setCheckedAll(bool const checked)
{
	int const rowCount = numRows();

	for (int index = 0; index < rowCount; ++index)
	{
		TemplateTableRow *templateTableRow = getTemplateTableRow(index);

		if (templateTableRow != NULL)
		{
			templateTableRow->setChecked(checked);
		}
	}
}

////-----------------------------------------------------------------------------
//void TemplateTable::setParentParameterName(QString const &parameterName)
//{
//	m_parentParameterName = parameterName;
//}
//
////-----------------------------------------------------------------------------
//void TemplateTable::setParentTemplateTable(TemplateTable const &templateTable)
//{
//	m_parentTemplateTable = &templateTable;
//}
//
////-----------------------------------------------------------------------------
//void TemplateTable::setParentTpfTemplate(TpfTemplate const &tpfTemplate)
//{
//	m_parentTpfTemplate = &tpfTemplate;
//}

//-----------------------------------------------------------------------------
void TemplateTable::setParentTemplateTableRow(TemplateTableRow const *templateTableRow)
{
	m_parentTemplateTableRow = templateTableRow;
}

//-----------------------------------------------------------------------------
bool TemplateTable::getNeedsComma(TemplateTableRow const &templateTableRow) const
{
	bool result = false;

	if (m_parentTemplateTableRow != NULL)
	{
		QString parentParameterName(m_parentTemplateTableRow->getParameterName());

		if (!parentParameterName.isNull())
		{
			bool const deepCheck = true;
			
			TemplateData::Parameter const *parentParameter = m_parentTemplateTableRow->getParentTable().getTemplateData().getParameter(parentParameterName.latin1(), deepCheck);

			if (parentParameter != NULL)
			{
				int const row = templateTableRow.getColumnName()->row();

				if ((parentParameter->list_type == TemplateData::LIST_INT_ARRAY) ||
				    (parentParameter->list_type == TemplateData::LIST_ENUM_ARRAY) &&
					(parentParameter->name == templateTableRow.getParameter()->name))
				{
					//if (row < (parentParameter->list_size - 1))
					//{
					//	// Check if this is the last item in a fixed size list
					//
					//	result = true;
					//}

					if (m_parentTemplateTableRow->getParentTable().isParameterType(PT_list))
					{
						result = true;
					}
				}
				else if ((parentParameter->list_type == TemplateData::LIST_LIST) &&
				         (parentParameter->name == templateTableRow.getParameter()->name) &&
				         !m_parentTemplateTableRow->isSingle())
				{
					// Check if this is the last item in a variable size list
				
					if (m_parentTemplateTableRow->getTpfTemplate() != NULL)
					{
						int const length = m_parentTemplateTableRow->getTpfTemplate()->getListLength(parentParameterName.latin1());
				
						if (row < (length - 1))
						{
							result = true;
						}
					}
				}
				else if (parentParameter->type == TemplateData::TYPE_STRUCT)
				{
					// Figure out the number of elements in the struct to determine the number of parameters in the
					// struct

					QString parentParameterExtendedName(parentParameter->extendedName.c_str());
					
					TemplateData const *structTemplateData = m_parentTemplateTableRow->getParentTable().getTemplateData().getStruct(parentParameterExtendedName.latin1());
					NOT_NULL(structTemplateData);
					
					TemplateDataIterator templateDataIterator(*structTemplateData);

					//bool const deepCheck = false;
					//
					//TemplateData::Parameter const *parameter = structTemplateData->getParameter(parentParameter->name.c_str(), deepCheck);
					//NOT_NULL(parameter);
					//
					//StructParamOT *structParamOT = m_parentTemplateTableRow->getTpfTemplate()->getStructParamOT(m_parentParameterName.latin1(), m_parentTemplateTableRow->getParameterIndex());
					//NOT_NULL(structParamOT);
					//
					//TpfTemplate *structTpfTemplate = dynamic_cast<TpfTemplate *>(structParamOT->getValue());
					//NOT_NULL(structTpfTemplate);

					int const size = templateDataIterator.size();

					if (row < (size - 1))
					{
						result = true;
					}
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool TemplateTable::isLastElement(TemplateTableRow const &templateTableRow) const
{
	UNREF(templateTableRow);

	return false;
}

//-----------------------------------------------------------------------------
//TemplateTable const *TemplateTable::getParentTemplateTable() const
//{
//	return m_parentTemplateTable;
//}
//
////-----------------------------------------------------------------------------
//QString const &TemplateTable::getParentParameterName() const
//{
//	return m_parentParameterName;
//}
//
////-----------------------------------------------------------------------------
//TpfTemplate const *TemplateTable::getParentTpfTemplate() const
//{
//	return m_parentTpfTemplate;
//}

//-----------------------------------------------------------------------------
TemplateTableRow const *TemplateTable::getParentTemplateTableRow() const
{
	return m_parentTemplateTableRow;
}

//-----------------------------------------------------------------------------
void TemplateTable::setOptimizedRowInsertEnabled(bool const enabled)
{
	if (enabled)
	{
		m_optimizatedRowInsert = true;
		m_currentOptimizedRow = 0;
	}
	else
	{
		m_optimizatedRowInsert = false;
	}
}

// ============================================================================
