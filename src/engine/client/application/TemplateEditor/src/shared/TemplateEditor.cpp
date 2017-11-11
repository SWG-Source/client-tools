// ============================================================================
//
// TemplateEditor.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "TemplateEditor.h"
#include "TemplateEditor.moc"

#include "ListViewItemFile.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"
#include "sharedTemplateDefinition/Filename.h"
#include "sharedTemplateDefinition/TpfFile.h"
#include "sharedTemplateDefinition/TpfTemplate.h"
#include "TableItemColor.h"
#include "TemplateEditorManager.h"
#include "TemplateEditorPerforce.h"
#include "TemplateEditorUtility.h"
#include "TemplateLoader.h"
#include "TemplateTable.h"
#include "TemplateVerificationDialog.h"
#include "WorkingDirectories.h"

#include <limits>

// ============================================================================
//
// TemplateEditor
//
// ============================================================================

void errorCallBack(char const *error);

//-----------------------------------------------------------------------------
TemplateEditor::TemplateEditor(QWidget *parent, const char *name)
 : BaseTemplateEditor(parent, name)
{
	// Initialize the list view

	connect(m_listViewFile, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slotFileListViewDoubleClicked(QListViewItem *)));
	connect(m_listViewFile, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)), this, SLOT(slotFileListViewRightButtonClicked(QListViewItem *, const QPoint &, int)));
	m_listViewFile->setAllColumnsShowFocus(true);
	m_listViewFile->setSelectionMode(QListView::Extended);

	// Initialize template field table

	m_templateTable->horizontalHeader()->hide();
	m_templateTable->verticalHeader()->hide();
	m_templateTable->setTopMargin(0);
	m_templateTable->setLeftMargin(0);

	// Initialize legend box

	m_tableLegend->setSelectionMode(QTable::NoSelection);

	m_tableLegend->horizontalHeader()->hide();
	m_tableLegend->verticalHeader()->hide();
	m_tableLegend->setTopMargin(0);
	m_tableLegend->setLeftMargin(0);

	// Initialize parameter description list

	m_descriptionListView->setColumnWidthMode(0, QListView::Maximum);
	m_descriptionListView->setColumnWidthMode(1, QListView::Maximum);

	// Initialize comments text

	m_commentsTextEdit->setReadOnly(true);

	// Status bar

	//statusBar()->message("Status Bar text");
	statusBar()->hide();        // Don't use the status bar for now (maybe never)

	// Setup connections

	connect(m_newAction, SIGNAL(activated()), this, SLOT(onNewActionActivated()));
	connect(m_fileAction, SIGNAL(activated()), this, SLOT(onFileActionActivated()));
	connect(m_openAction, SIGNAL(activated()), this, SLOT(onOpenActionActivated()));
	connect(m_saveAction, SIGNAL(activated()), this, SLOT(onSaveActionActivated()));
	connect(m_saveAsAction, SIGNAL(activated()), this, SLOT(onSaveAsActionActivated()));
	connect(m_exitAction, SIGNAL(activated()), this, SLOT(onExitActionActivated()));
	connect(m_testAllTemplatesAction, SIGNAL(activated()), this, SLOT(onTestAllTemplatesActionActivated()));

	// Load the widget settings

	TemplateEditorUtility::loadWidget(*this, 0, 0, 640, 480);

	// Set up the debug output window (also useful for program warnings)

	//m_debugTextEdit->hide();
	TemplateEditorUtility::setOutputWindow(*m_debugTextEdit);

	// Initialize the hard-working manager

	m_templateEditorManager = new TemplateEditorManager(*this);

	// Setup the callback for errors when loading templates

	File::setErrorCallBack(errorCallBack);

//	int temp = rand() % std::numeric_limits<int>::max();
//	UNREF(temp);
//
//	int temp2 = rand() % std::numeric_limits<int>::min();
//	UNREF(temp2);

	//Random::random(std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max() - 1);
	//Random::random(std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max());
	//Random::random(std::numeric_limits<int>::min(), std::numeric_limits<int>::max() - 1);
	//Random::random(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
}

//-----------------------------------------------------------------------------
TemplateEditor::~TemplateEditor()
{
	// Save the widget settings

	TemplateEditorUtility::saveWidget(*this);

	delete m_templateEditorManager;
}

//-----------------------------------------------------------------------------
void errorCallBack(char const *error)
{
	if (error != NULL)
	{
		TemplateEditorUtility::report(error);
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	UNREF(mouseEvent);
}

//-----------------------------------------------------------------------------
void TemplateEditor::onFileActionActivated()
{
	// Make sure you can only save if a file is loaded

	if (m_fileInfoCurrent.fileName().isEmpty())
	{
		m_saveAction->setEnabled(false);
		m_saveAsAction->setEnabled(false);
	}
	else
	{
		m_saveAction->setEnabled(true);
		m_saveAsAction->setEnabled(true);
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::onNewActionActivated()
{
	////DHERMAN THIS NEEDS TO BE CHANGED TO A TEMPLATE PULLDOWN LIST
	//// Get the last used path
	//
	//QSettings settings;
	//settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
	//QString previousUsedPath(settings.readEntry("NewTemplatePath", "c:\\"));
	//
	//// Get the file
	//
	//QString selectedFile(QFileDialog::getOpenFileName(previousUsedPath, "Template Definition Files (*.tdf)", this, "OpenFileDialog", "Create a Template from a Template Definition"));
	//
	//if (selectedFile != QString::null)
	//{
	//	m_fileInfoCurrent.setFile(selectedFile);
	//
	//	// Double check that the file is still there
	//	if (m_fileInfoCurrent.isReadable())
	//	{
	//		if (m_fileInfoCurrent.extension() == "tdf")
	//		{
	//			m_templateEditorManager->initializeWithTDF(m_fileInfoCurrent.absFilePath().latin1());
	//		}
	//		// else DHERMAN Error here
	//
	//		settings.writeEntry("NewTemplatePath", m_fileInfoCurrent.filePath());
	//
	//		QString caption("Template Editor - " +m_fileInfoCurrent.filePath());
	//		setCaption(caption);
	//	}
	//
	//	else if (!m_fileInfoCurrent.exists())
	//	{
	//		QString warningText("The selected file does not exist on disk: \"" + m_fileInfoCurrent.fileName() + "\"");
	//		QMessageBox::warning(this, "File Does Not Exist", warningText, "OK");
	//	}
	//}
}

//-----------------------------------------------------------------------------
void TemplateEditor::onOpenActionActivated()
{
	// Get the last used path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
	QString previousUsedPath(settings.readEntry("OpenTemplatePath", QDir::root().absPath()));

	// Get the file

	QStringList selectedFileNames(QFileDialog::getOpenFileNames("Object Template files (*.tpf)", previousUsedPath, this, "OpenFileDialog", "Open Object Template"));

	for (QStringList::Iterator current = selectedFileNames.begin(); current != selectedFileNames.end(); ++current)
	{
		// Save the open file location

		settings.writeEntry("OpenTemplatePath", *current);

		// Open this file

		bool const initializeTemplateTable = (selectedFileNames.size() == 1);

		QString currentPath(*current);

		open(currentPath, initializeTemplateTable, true);
	}

	if (selectedFileNames.size() > 0)
	{
		updateWorkingFileList();
	}
}

//-----------------------------------------------------------------------------
bool TemplateEditor::open(QString const &path, bool const initializeTemplateTable, bool const forceUpdateWorkingFileList, bool const quickLoad)
{
	bool loadSuccessful = false;

	QString previousFile(m_fileInfoCurrent.filePath());

	if (initializeTemplateTable)
	{
		if (!path.isNull())
		{
			setCurrentFile(path, forceUpdateWorkingFileList);

			if (m_fileInfoCurrent.isReadable())
			{
				// Check the file type

				if (m_fileInfoCurrent.extension() == TEMPLATE_EXTENSION)
				{
					// Load the current template

					if (TemplateLoader::loadTpf(m_fileInfoCurrent.filePath().latin1()))
					{
						if (!quickLoad)
						{
							// Build the legend

							buildLegend();
						}

						// Initialize the table

						m_templateEditorManager->initializeTemplateTable(quickLoad);

						// Set the filename

						loadSuccessful = true;
					}
					else
					{
						QString text;
						text.sprintf("OPEN: Unable to open template: %s", m_fileInfoCurrent.filePath().latin1());
						TemplateEditorUtility::report(text);
					}
				}
				else if (m_fileInfoCurrent.extension() == IFF_EXTENSION)
				{
				}
				else
				{
					char text[1024];
					sprintf(text, "Trying to open an invalid file when expecting \".%s\" or \".%s\".: ", TEMPLATE_EXTENSION, IFF_EXTENSION, m_fileInfoCurrent.absFilePath());
					QMessageBox::warning(this, "File Error", text, "OK");
				}
			}
			else
			{
				QString error("Unable to read the file (file may not exist): " +  m_fileInfoCurrent.fileName());
				QMessageBox::warning(this, "File Error", error, "OK");
			}
		}
	}

	if (!quickLoad)
	{
		addFileToWorkingList(path, forceUpdateWorkingFileList);
	}

	if (!loadSuccessful)
	{
		// Restore the previous filename if the load failed

		setCurrentFile(previousFile, forceUpdateWorkingFileList);
	}

	return loadSuccessful;
}

//-----------------------------------------------------------------------------
void TemplateEditor::addFileToWorkingList(QString const &path, bool const forceUpdate)
{
	bool sortNeeded = false;

	int const listViewFileChildCount = m_listViewFile->childCount();
	UNREF(listViewFileChildCount);

	// See if this file already exists in the list view

	QListViewItem *listViewItem = m_listViewFile->firstChild();
	bool alreadyInList = false;

	while (listViewItem != NULL)
	{
		ListViewItemFile *listViewItemFile = dynamic_cast<ListViewItemFile *>(listViewItem);

		if (listViewItemFile != NULL)
		{
			QString listViewItemFilePath(listViewItemFile->getPath());

			if (listViewItemFilePath == path)
			{
				alreadyInList = true;
				break;
			}
		}
		
		listViewItem = listViewItem->nextSibling();
	}

	if (!alreadyInList)
	{
		// Add the file to the list view

		ListViewItemFile *listViewItemFile = new ListViewItemFile(m_listViewFile);
		QFileInfo fileInfo(path);
		listViewItemFile->setPath(fileInfo.absFilePath());
		m_listViewFile->insertItem(listViewItemFile);
		int const checkOutColumnWidth = 24;
		m_listViewFile->setColumnWidth(0, checkOutColumnWidth);
		m_listViewFile->setColumnWidth(1, m_listViewFile->contentsRect().width() - checkOutColumnWidth);

		sortNeeded = true;

		if (forceUpdate)
		{
			// Update the working file list to reflect the checked out files

			updateWorkingFileList();
		}
	}

	if (sortNeeded)
	{
		m_listViewFile->setSorting(1);
		m_listViewFile->sort();
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::onSaveActionActivated()
{
	save(m_fileInfoCurrent.filePath());
}

//-----------------------------------------------------------------------------
void TemplateEditor::save(QString const &path)
{
	bool resetToPreviousPath = true;
	QString previousPath(m_fileInfoCurrent.filePath());

	bool const forceUpdateWorkingFileList = true;

	setCurrentFile(path, forceUpdateWorkingFileList);

	if (m_fileInfoCurrent.exists())
	{
		if (!FileNameUtils::isWritable(path.latin1()))
		{
			sourceControlCheckOutFile(path);
		}
	}

	// Make sure there is a file name that is valid

	bool const exists = m_fileInfoCurrent.exists();
	bool const writable = FileNameUtils::isWritable(path.latin1());
	bool const empty = m_fileInfoCurrent.fileName().isEmpty();

	if ((!exists || writable) && !empty)
	{
		m_templateTable->endEditing(); // Make sure all changes are commited before saving

		// Save the path

		QSettings settings;
		settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
		QString dirPath(m_fileInfoCurrent.dirPath());
		settings.writeEntry("SaveTemplatePath", dirPath);

		// Write the file

		QFile file(m_fileInfoCurrent.absFilePath().latin1());
		
		if (file.open(IO_WriteOnly))
		{
			QTextStream writeFileStream(&file);
			QString time(QTime::currentTime().toString(Qt::LocalDate));
			QString date(QDate::currentDate().toString(Qt::TextDate));

			writeFileStream << "// This file ws written by the template editor @ " << time << " -  " << date << endl;

			m_templateTable->write(file, true);

			QString text;
			text.sprintf("SAVE: %s", file.name());
		}
		else
		{
			QString text;
			text.sprintf("SAVE: Error opening file for write (%s)", file.name());
			TemplateEditorUtility::report(text);
		}

		resetToPreviousPath = false;
	}
	else if (m_fileInfoCurrent.isReadable())
	{
		QString warningText("The selected file is read only: \"" + m_fileInfoCurrent.filePath() + "\"\n Make sure you have the correct client spec selected.");
		QMessageBox::warning(this, "Save File Error", warningText, "OK");
	}

	if (resetToPreviousPath)
	{
		bool const forceUpdateWorkingFileList = true;

		setCurrentFile(previousPath, forceUpdateWorkingFileList);

		onSaveAsActionActivated();
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::onSaveAsActionActivated()
{
	if (!m_fileInfoCurrent.fileName().isEmpty())
	{
		// Get the last used path

		QSettings settings;
		settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
		QString previousUsedPath(settings.readEntry("SaveTemplatePath", QDir::root().absPath()));

		// Get the file

		QString selectedFile(QFileDialog::getSaveFileName(previousUsedPath, "Object Template files (*.tpf)", this, "SaveAsFileDialog", "Save the Template"));

		if (!selectedFile.isNull())
		{
			// Automatically add the correct extension

			FilePath filePath(selectedFile.latin1());
			QString path(filePath.get(FilePath::drive | FilePath::directory | FilePath::fileName).c_str());
			path += ".tpf";
			
			setCurrentFile(path, true);

			// Save the file

			onSaveActionActivated();
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::setCurrentFile(QString const &path, bool const forceUpdateWorkingFileList)
{
	if (!path.isEmpty())
	{
		// Set the current filename

		m_fileInfoCurrent.setFile(path);

		if (forceUpdateWorkingFileList)
		{
			// Set the caption

			std::string captionPath;
			std::string absFilePath(path.latin1());

			bool result = TreeFile::stripTreeFileSearchPathFromFile(absFilePath, captionPath);

			if (!result)
			{
				captionPath = absFilePath.c_str();
			}

			QString caption("Template Editor - ");
			caption += captionPath.c_str();
			setCaption(caption);

			// Set the file full path display

			m_fileLineEdit->setText(path);

			addFileToWorkingList(path, forceUpdateWorkingFileList);
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::compileFile(QString const &path)
{
	TpfFile tpfFile;
	FilePath filePath(path.latin1());

	int result = tpfFile.makeIffFiles(Filename(NULL, filePath.getPath().c_str(), filePath.getFileName(), filePath.getExtension()));

	if (result != -1)
	{
		QString text;
		text.sprintf("COMPILE - Success: %s", path.latin1());
		TemplateEditorUtility::report(text);
	}
	else
	{
		QString text;
		text.sprintf("COMPILE - Error: %s", path.latin1());
		TemplateEditorUtility::report(text);
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::onExitActionActivated()
{
	// Maybe check to see if the current data has changed

	close();
}

//-----------------------------------------------------------------------------
void TemplateEditor::onTestAllTemplatesActionActivated()
{
	// Get the last used path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
	QString previousUsedPath(settings.readEntry("TestAllTemplatesPath", QDir::root().absPath()));

	QWidget *parent = this;
	char const name[] = "QFileDialog";
	QString const caption("Specify the root directory");
	bool const dirOnly = true;
	bool const resolveSymlinks = true;

	QString rootDirectory(QFileDialog::getExistingDirectory(previousUsedPath, parent, name, caption, dirOnly, resolveSymlinks));

	if (!rootDirectory.isEmpty())
	{
		// Save the last used path

		settings.insertSearchPath(QSettings::Windows, TemplateEditorUtility::getSearchPath());
		settings.writeEntry("TestAllTemplatesPath", rootDirectory);
		
		// Build a list of all the files we need to open in the template editor

		QStringList allFiles;
		StringSet directorySet;
		QTime time;
		time.start();
		
		QString text;
		text.sprintf("Searching for all templates from the root: %s", rootDirectory.latin1());
		TemplateEditorUtility::report(text);

		searchAllDirectories(rootDirectory, allFiles, directorySet);

		text.sprintf("Found %d templates in %d seconds for testing.", allFiles.size(), time.elapsed() / 1000);
		TemplateEditorUtility::report(text);

		// Prompt for the testing options

		TemplateVerificationDialog templateVerificationDialog(this, "TemplateVerificationDialog", allFiles.size(), directorySet.size());

		int result = templateVerificationDialog.exec();

		if (result == QDialog::Accepted)
		{
			int firstTemplate = 0;
			int lastTemplate = static_cast<int>(allFiles.size());

			if (templateVerificationDialog.m_checkAllRadioButton->isChecked())
			{
			}
			else if (templateVerificationDialog.m_checkOneRadioButton->isChecked())
			{
				QStringList newStringList;

				// Keep only one file per directory

				for (unsigned int index = 0; index < allFiles.size(); ++index)
				{
					QFileInfo fileInfo(allFiles[index]);
					QString directoryPath(fileInfo.dirPath());

					StringSet::iterator iterStringSet = directorySet.find(directoryPath);

					if (iterStringSet != directorySet.end())
					{
						// Save this file

						newStringList.push_back(allFiles[index]);

						directorySet.erase(iterStringSet);
					}
				}

				// Set the files to check list to the new one file per directory list

				allFiles = newStringList;

				lastTemplate = static_cast<int>(allFiles.size());
			}
			else if (templateVerificationDialog.m_checkSubsetRadioButton->isChecked())
			{
				firstTemplate = TemplateEditorUtility::getInt(templateVerificationDialog.m_firstLineEdit);
				lastTemplate = TemplateEditorUtility::getInt(templateVerificationDialog.m_lastLineEdit);
			}
			else
			{
				DEBUG_FATAL(true, ("Unexpected template verification mode"));
			}

			// See if we need to delete the success list file

			if (!templateVerificationDialog.m_resumeCheckBox->isChecked())
			{
				QFile deleteFile(TemplateEditorUtility::getSuccessListPath());
				deleteFile.remove();
			}

			// Hide the template table to speed up the loading

			m_templateTable->hide();

			//int const templateTestCount = (allFiles.size() > 100) ? 100 : allFiles.size();
			int const templateTestCount = allFiles.size();

			// Load all the successfully loaded templates

			QFile readFile(TemplateEditorUtility::getSuccessListPath());
			bool const readFileOpenResult = readFile.open(IO_ReadOnly | IO_Translate);

			QStringList alreadyCheckedFileList;

			if (readFileOpenResult)
			{
				QTextStream readFileStream(&readFile);

				while (!readFileStream.eof())
				{
					alreadyCheckedFileList.push_back(readFileStream.readLine());
				}

				readFile.close();
			}

			int processedTemplates = 0;
			int millisecondsToRemove = 0;

			time.restart();

			// Load each found template

			for (int index = firstTemplate; index < lastTemplate; ++index)
			{
				int startLoopMilliseconds = time.elapsed();

				QString currentFile(allFiles[index]);

				// See if this file has already been processed

				QString first(alreadyCheckedFileList.front());
				QStringList::iterator iter = alreadyCheckedFileList.find(currentFile);

				// Display status information in the caption

				std::string captionPath;
				std::string absFilePath(currentFile);
				bool result = TreeFile::stripTreeFileSearchPathFromFile(absFilePath.c_str(), captionPath);

				if (!result)
				{
					// Try to force the path to something nicer than the full path

					captionPath = currentFile.latin1() + strlen(rootDirectory.latin1());
				}

				int const millisecondsPerTemplate = ((time.elapsed() - millisecondsToRemove) / (processedTemplates + 1));
				int const estimatedMillisecondsRemaining = (templateTestCount - (processedTemplates + 1)) * millisecondsPerTemplate;
				int const etaHours = estimatedMillisecondsRemaining / 1000 / 60 / 60;
				int const etaMinutes = (estimatedMillisecondsRemaining / 1000 / 60) % 60;
				int const etaSeconds = (estimatedMillisecondsRemaining / 1000) % 60;
				int const elapsedHours = time.elapsed() / 1000 / 60 / 60;
				int const elapsedMinutes = (time.elapsed() / 1000 / 60) % 60;
				int const elapsedSeconds = (time.elapsed() / 1000) % 60;

				QString caption;
				caption.sprintf("Template Editor - (%d) (%d) (%d:%02d:%02d) (%d / %d) - %d:%02d:%02d ETA - %s", millisecondsPerTemplate, processedTemplates, elapsedHours, elapsedMinutes, elapsedSeconds, index + 1, templateTestCount, etaHours, etaMinutes, etaSeconds, captionPath.c_str());
				setCaption(caption);

				if (iter != alreadyCheckedFileList.end())
				{
					millisecondsToRemove += (time.elapsed() - startLoopMilliseconds);
					continue;
				}

				// Load the current template

				bool const initializeTemplateTable = true;
				bool const forceUpdateWorkingFileList = false;
				bool const quickLoad = true;
				result = open(currentFile, initializeTemplateTable, forceUpdateWorkingFileList, quickLoad);

				if (result)
				{
					//bool const openSaveOpen = true;
					//
					//if (openSaveOpen)
					{
						// Check the file out

						sourceControlCheckOutFile(currentFile);

						// Save the file

						onSaveActionActivated();

						// Clear the table

						delete m_templateTable;
						m_templateTable = new TemplateTable( Frame5, "m_templateTable" );

						// Reopen the file
						
						result = open(currentFile, initializeTemplateTable, forceUpdateWorkingFileList, quickLoad);

						if (!result)
						{
							QString text("Open-Save-Open failed for template: " + currentFile);
							QMessageBox::warning(this, "Load Error", text, "Abort Test");
							break;
						}
					}
				}

				if (result)
				{
					QFile writeFile(TemplateEditorUtility::getSuccessListPath());
					bool const writeFileOpenResult = writeFile.open(IO_Append | IO_Translate | IO_WriteOnly);

					if (writeFileOpenResult)
					{
						QTextStream writeFileStream(&writeFile);
						writeFileStream << currentFile << '\n';
						writeFile.close();
					}
				}

				delete m_templateTable;
				m_templateTable = new TemplateTable( Frame5, "m_templateTable" );
				//m_templateTable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, m_templateTable->sizePolicy().hasHeightForWidth() ) );

				// Make sure there is only one template table allocated, if there is more than one
				// then there is a leak somewhere

				int const templateTableCount = TemplateTable::getGlobalCount();
				UNREF(templateTableCount);

				DEBUG_FATAL(templateTableCount > 1, ("There should only be one TemplateTable object allocated at this point."));

				++processedTemplates;

				if ((processedTemplates % 20) == 0)
				{
					// Don't let the debug text get too long because it eats memory!

					if (m_debugTextEdit->lines() > 200)
					{
						m_debugTextEdit->clear();
					}

					qApp->processEvents();
				}
			}

			// Show the template table again

			m_templateTable->clearAll();
			m_templateTable->show();

			text.sprintf("Template test - Done - %d templates checked", index);
			TemplateEditorUtility::report(text);

			// Update the working file list

			updateWorkingFileList();
		}
		else
		{
			TemplateEditorUtility::report("Template verification aborted");
			
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::searchAllDirectories(QString const &currentDirectory, QStringList &allFiles, StringSet &directorySet)
{
	// Enable directories in the entry list

	QDir dir(currentDirectory);
	dir.setMatchAllDirs(true);

	int const filterSpec = QDir::DefaultFilter;
	int const sortSpec = QDir::DefaultSort;

	QStringList currentFiles(dir.entryList("*.tpf", filterSpec, sortSpec));

	for (unsigned int index = 0; index < currentFiles.size(); ++index)
	{
		QString path(currentFiles[index]);

		if ((path != ".") && (path != "..") && !path.isEmpty())
		{
			QFileInfo fileInfo(dir.path() + "/" + path);

			if (fileInfo.isFile())
			{
				// Save the file path
			
				QString filePath(fileInfo.absFilePath());

				allFiles.push_back(filePath);
			}
			else if (fileInfo.isDir())
			{
				// Save the directory path

				QString directoryPath(fileInfo.absFilePath());
			
				directorySet.insert(directoryPath);

				// Search the new directory

				searchAllDirectories(directoryPath, allFiles, directorySet);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::slotFileListViewDoubleClicked(QListViewItem *listViewItem)
{
	if (listViewItem != NULL)
	{
		ListViewItemFile *listViewItemFile = dynamic_cast<ListViewItemFile *>(listViewItem);

		if (listViewItemFile != NULL)
		{
			// Save the previous template

			if (listViewItemFile->getPath() != m_fileInfoCurrent.absFilePath())
			{
				open(listViewItemFile->getPath());
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::buildLegend()
{
	m_tableLegend->hide();
	m_tableLegend->setSorting(false);
	m_tableLegend->setShowGrid(false);

	// Clear the legend

	m_tableLegend->setNumRows(0);

	// Build the legend

	TpfTemplate const *tpfTemplateCurrent = &TemplateLoader::getTpfTemplate();
	FilePath filePath(m_fileInfoCurrent.absFilePath().latin1());

	QDir dir(filePath.get(FilePath::drive | FilePath::directory).c_str());
	TpfFile tpfFile;
	
	// Add all the inherited TPF files

	m_tableLegend->setNumCols(2);
	m_tableLegend->insertRows(0, 1);

	TableItemColor *tableItem = new TableItemColor(*m_tableLegend, "       ");
	m_tableLegend->setItem(0, 0, tableItem);

	QTableItem *tableItemColor = new QTableItem(m_tableLegend, QTableItem::Never, filePath.getFileName());
	m_tableLegend->setItem(0, 1, tableItemColor);

	m_tableLegend->update();
	int const colorColumnWidth = 32;
	m_tableLegend->setColumnWidth(0, colorColumnWidth);
	m_tableLegend->setColumnWidth(1, m_tableLegend->contentsRect().width() - colorColumnWidth);

	while ((tpfTemplateCurrent != NULL) && !tpfTemplateCurrent->getBaseTemplateName().empty())
	{
		Filename fileNameTpf(NULL, dir.absPath(), tpfTemplateCurrent->getBaseTemplateName().c_str(), TEMPLATE_EXTENSION);
		QFileInfo fileInfo(fileNameTpf.getFullFilename().c_str());

		// Make sure the file exists on disk before we try to load it with
		// the tpf file, otherwise we get lots of extra errors.

		bool fileExists = fileInfo.exists();
		int result = -1;
		
		if (fileExists)
		{
			result = tpfFile.loadTemplate(fileNameTpf);
		}

		if (result == 0)
		{
			m_tableLegend->insertRows(0, 1);

			TableItemColor *tableItem = new TableItemColor(*m_tableLegend, "       ");
			m_tableLegend->setItem(0, 0, tableItem);

			QTableItem *tableItemColor = new QTableItem(m_tableLegend, QTableItem::Never, fileNameTpf.getName().c_str());
			m_tableLegend->setItem(0, 1, tableItemColor);

			tpfTemplateCurrent = tpfFile.getTemplate();
		}
		else
		{
			// Move up a directory

			bool result = dir.cdUp();

			if (!result)
			{
				break;
			}
		}
	}

	// Now color all the cells

	// Get the number of cells that are tpfs

	int const rowCount = m_tableLegend->numRows();
	int i;

	for (i = 0; i < rowCount; ++i)
	{
		TableItemColor *tableItemColor = dynamic_cast<TableItemColor *>(m_tableLegend->item(i, 0));

		if (tableItemColor != NULL)
		{
			tableItemColor->setTextColor(QColor("black"));
			tableItemColor->setTextNumber(QString::number(i));

			// ROYGBIV

			switch (i % 7)
			{
				case 0:
					{
						tableItemColor->setCellColor(QColor("red"));
					}
					break;
				case 1:
					{
						tableItemColor->setCellColor(QColor("orange"));
					}
					break;
				case 2:
					{
						tableItemColor->setCellColor(QColor("yellow"));
					}
					break;
				case 3:
					{
						tableItemColor->setCellColor(QColor("green"));
					}
					break;
				case 4:
					{
						tableItemColor->setCellColor(QColor(128, 128, 255));
					}
					break;
				case 5:
					{
						tableItemColor->setCellColor(QColor(0, 255, 255)); // indigo
					}
					break;
				case 6:
					{
						tableItemColor->setCellColor(QColor("violet"));
					}
					break;
			}
		}
	}

	m_tableLegend->show();
}

//-----------------------------------------------------------------------------
void TemplateEditor::getColor(QString const &path, QColor &color, int &index)
{
	QColor result("black");

	// Now color all the cells

	int const rowCount = m_tableLegend->numRows();

	for (int i = 0; i < rowCount; ++i)
	{
		QString fileName(FilePath(path.latin1()).getFileName());

		if (m_tableLegend->item(i, 1)->text() == fileName)
		{
			TableItemColor *tableItemColor = dynamic_cast<TableItemColor *>(m_tableLegend->item(i, 0));

			if (tableItemColor != NULL)
			{
				color = tableItemColor->getCellColor();
				index = i;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::slotFileListViewRightButtonClicked(QListViewItem *listViewItem, const QPoint &point, int column)
{
	UNREF(column);

	enum MenuOptions
	{
		  MO_checkOut
		, MO_compile
		, MO_removeFromList
	};

	if (listViewItem != NULL)
	{
		QPopupMenu popUpMenu(this, "QPopupMenu");
		popUpMenu.insertItem("Check Out", MO_checkOut);
		popUpMenu.insertSeparator();
		popUpMenu.insertItem("Compile", MO_compile);
		popUpMenu.insertSeparator();
		popUpMenu.insertItem("RemoveFromList", MO_removeFromList);

		int result = popUpMenu.exec(point);

		QListViewItem *child = m_listViewFile->firstChild();

		while (child != NULL)
		{
			bool deleteChild = false;

			if (child->isSelected())
			{
				ListViewItemFile *listViewItemFile = dynamic_cast<ListViewItemFile *>(child);

				if (listViewItemFile != NULL)
				{
					switch (result)
					{
						case MO_checkOut:
							{
								// Check out the tpf

								if (!sourceControlCheckOutFile(listViewItemFile->getPath().latin1()))
								{
									TemplateEditorUtility::report("Make sure your client spec is set correctly.");
								}
							}
							break;
						case MO_compile:
							{
								// Make sure the tpf file is checked out

								if (!sourceControlCheckOutFile(listViewItemFile->getPath()))
								{
									QString text;
									text.sprintf("CHECKOUT: Unable to checkout: %s ", listViewItemFile->getPath().latin1());
									TemplateEditorUtility::report(text);
								}
								else
								{
									// Check out the corresponding iff files

									char iffPath[_MAX_PATH];
									FilePath filePath(listViewItemFile->getPath().latin1());

									sprintf(iffPath, "%s.iff", filePath.get(FilePath::drive | FilePath::directory | FilePath::fileName).c_str());
							
									// Fixup the 'dsrc' to 'data'

									char *data = strstr(iffPath, "dsrc");
						
									if (data != NULL)
									{
										*(data + 0) = 'd';
										*(data + 1) = 'a';
										*(data + 2) = 't';
										*(data + 3) = 'a';
									}

									// Check out the matching iff file

									if (sourceControlCheckOutFile(iffPath))
									{
										// Compile the tpf

										compileFile(listViewItemFile->getPath());
									}
									else
									{
										QString text;
										text.sprintf("COMPILE: Unable to compile: %s ", listViewItemFile->getPath().latin1());
										TemplateEditorUtility::report(text);
									}
								}
							}
							break;
						case MO_removeFromList:
							{
								deleteChild = true;
							}
							break;
						default:
							{
								if (result != -1)
								{
									DEBUG_FATAL(true, ("TemplateEditor::slotFileListViewRightButtonClicked() - Unexpected popup menu option"));
								}
							}
							break;
					}
				}
			}

			if (deleteChild)
			{
				QListViewItem *childToDelete = child;

				ListViewItemFile *listViewItemFile = dynamic_cast<ListViewItemFile *>(childToDelete);

				// See if we need to clear the template view because the current template was removed from the working list

				if ((listViewItemFile != NULL) && (listViewItemFile->getPath() == m_fileInfoCurrent.filePath()))
				{
					m_templateTable->clearAll();
				}

				child = child->nextSibling();

				delete childToDelete;
			}
			else
			{
				child = child->nextSibling();
			}
		}

		// Update the working file list to reflect the checked out files

		updateWorkingFileList();
	}
}

//-----------------------------------------------------------------------------
bool TemplateEditor::sourceControlCheckOutFile(QString const &path) const
{
	typedef std::vector<std::string> StringVector;
	
	QString drive(QString(FilePath(path.latin1()).getDrive()).lower());
	QString fixedUpPath(drive + QString(FilePath(path.latin1()).get(FilePath::directory | FilePath::fileName | FilePath::extension).c_str()));

	std::string depot;
	std::string clientPath;
	std::string local;

	TemplateEditorPerforce::getInstance().getFileMapping(fixedUpPath.latin1(), depot, clientPath, local);

	//// If the above function fails do it the old fashioned way
	//
	//if (depot.empty())
	//{
	//	depot = "//depot" + QString(FilePath(path.latin1()).get(FilePath::directory | FilePath::fileName | FilePath::extension).c_str());
	//}

	StringVector stringVector;
	stringVector.push_back(depot.c_str());
	
	bool result = false;
	int count = 0;

	do
	{
		result = TemplateEditorPerforce::getInstance().editFiles(stringVector);
		++count;
	}
	while (!result && (count < 20));
	
	if (!result)
	{
		// This file does not exist in Perforce, so lets add it
		
		result = sourceControlAddFile(depot.c_str());
	}

	// We tried hard to get this file open, since it did not work, send out a message

	if (!result)
	{
		QString error;
		error.sprintf("Unable to checkout file from source control: %s", path.latin1());

		TemplateEditorUtility::report(error);
	}

	return result;
}

//-----------------------------------------------------------------------------
bool TemplateEditor::sourceControlAddFile(QString const &path) const
{
	UNREF(path);

	TemplateEditorPerforce::StringVector stringVector;
	stringVector.push_back(path.latin1());
	
	int result = TemplateEditorPerforce::getInstance().addFiles(stringVector);

	if (result)
	{	
		QString text;
		text.sprintf("P4 add: %s", path.latin1());
		TemplateEditorUtility::report(text);
	}

	return result;
}

//-----------------------------------------------------------------------------
void TemplateEditor::updateWorkingFileList()
{
	QListViewItem *listViewItem = m_listViewFile->firstChild();
	TemplateEditorPerforce::OpenFileMap workingFileMap;
	
	TemplateEditorPerforce::getInstance().getOpenedFiles(workingFileMap);
	
	int const workingFileMapSize = workingFileMap.size();
	UNREF(workingFileMapSize);
	
	while (listViewItem != NULL)
	{
		ListViewItemFile *listViewItemFile = dynamic_cast<ListViewItemFile *>(listViewItem);
	
		if (listViewItemFile != NULL)
		{
			QFileInfo fileInfo(listViewItemFile->getPath());
	
			QString file(fileInfo.fileName().latin1());

			TemplateEditorPerforce::OpenFileMap::iterator iterWorkingFileMap = workingFileMap.find(file.latin1());
	
			if (iterWorkingFileMap != workingFileMap.end())
			{
				TemplateEditorPerforce::FileState const fileState = iterWorkingFileMap->second;

				if (fileState == TemplateEditorPerforce::FS_edit)
				{
					listViewItemFile->setText(0, "*");
				}
				else if (fileState == TemplateEditorPerforce::FS_add)
				{
					listViewItemFile->setText(0, "+");
				}
			}
			else
			{
				listViewItemFile->setText(0, "");
			}
		}
		
		listViewItem = listViewItem->nextSibling();
	}
}

//-----------------------------------------------------------------------------
void TemplateEditor::autoSave()
{
	save(m_fileInfoCurrent.filePath());

	// If this becomes slow we can make an update function for one item

	updateWorkingFileList();
}

// ============================================================================
