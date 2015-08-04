// ============================================================================
//
// MainWindow.cpp
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"

#include "BaseConsoleWindow.h"
#include "ClientDataFileWriter.h"
#include "CustomVariableWidget.h"
#include "GameWidget.h"
#include "MainWindow.h"
#include "MainWindow.moc"
#include "NpcCuiViewer.h"
#include "NewNpcDialog.h"
#include "ObjectTemplateWriter.h"
#include "PaletteColorPicker.h"
#include "QtUtility.h"
#include "SaveDialog.h"
#include "TemplateConfigDialog.h"

#include <qaction.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpopupmenu.h>
#include <qprocess.h>
#include <qstatusbar.h>
#include <qtextview.h>
#include <qtabwidget.h>
#include <qvbox.h>

#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTableWriter.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedGame/SharedObjectTemplate.h"

#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/CreatureObject.h"
#include "clientObject/GameCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"

#include "NpcEditor.xpm"

// ----------------------------------------------------------------------------

namespace MainWindowNamespace
{
	char const * c_npcEditor = "NpcEditor";
	char const * c_defaultSharedDsrc = "defaultSharedDsrc";
	char const * c_defaultClientDsrc = "defaultClientDsrc";

	char const *c_npcEditorDataTable = "../../exe/win32/NpcEditor.tab";

	bool copyUpdatedFile(char const * const sourceFileName, char const * outputFileName, std::string const & key, std::string const & value);
}

using namespace MainWindowNamespace;

// ----------------------------------------------------------------------------

#define CONSOLE_OUTPUT(a) \
	{ \
		m_consoleWindow->show(); \
		m_consoleWindow->m_textEditOutput->append(a); \
	}

// ----------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *myParent, char const *myName)
 : BaseMainWindow(myParent, myName)
 , m_clientFileInfo()
 , m_serverFileInfo()
 , m_sharedFileInfo()
 , m_clientCdfFileInfo()
 , m_serverIffFileInfo()
 , m_sharedIffFileInfo()
 , m_npcDataTable()
 , m_row(-1)
 , m_serverParameters()
 , m_sharedParameters()
 , m_consoleWindow(0)
 , m_proc(0)
 , m_avatarVBox(0)
 , m_avatarScrollView(0)
 , m_wearableVBox(0)
 , m_wearableScrollView(0)
 , m_procArgList()
 , m_openExistingMode(false)
 , m_dirty(false)
 , m_overwriteTemplatesMode(false)
{
	m_clientFileInfo.setFile("");
	m_serverFileInfo.setFile("");
	m_sharedFileInfo.setFile("");
	m_clientCdfFileInfo.setFile("");
	m_serverIffFileInfo.setFile("");
	m_sharedIffFileInfo.setFile("");

	BaseMainWindow::setIcon(QPixmap(NpcEditor_xpm));

	//-- get rid of the status bar
	statusBar()->hide();

	//-- load the child widget settings
	QtUtility::loadWidget(*this, 0, 0, 1024, 768);

	//-- console system
	m_consoleWindow = new BaseConsoleWindow(this);
	m_proc = new QProcess( this );

	//-- signals & slots
	IGNORE_RETURN(connect(m_proc, SIGNAL(readyReadStdout()), this, SLOT(slotReadProcessOutput())));
	IGNORE_RETURN(connect(m_proc, SIGNAL(readyReadStderr()), this, SLOT(slotReadProcessOutput())));
	IGNORE_RETURN(connect(m_proc, SIGNAL(processExited()), this, SLOT(slotProcessExited())));
	IGNORE_RETURN(connect(m_gameWidget, SIGNAL(viewerDirty()), this, SLOT(slotViewerDirty())));
	IGNORE_RETURN(connect(m_fileNewAction, SIGNAL(activated()), this, SLOT(slotFileNewAction())));
	IGNORE_RETURN(connect(m_fileSaveAction, SIGNAL(activated()), this, SLOT(slotFileSaveAction())));
	IGNORE_RETURN(connect(m_fileSaveAsAction, SIGNAL(activated()), this, SLOT(slotFileSaveAsAction())));
	IGNORE_RETURN(connect(m_fileOpenAction, SIGNAL(activated()), this, SLOT(slotFileOpenAction())));
	IGNORE_RETURN(connect(m_fileExitAction, SIGNAL(activated()), this, SLOT(slotFileExitAction())));
	IGNORE_RETURN(connect(m_toolConfigureTemplatesAction, SIGNAL(activated()), this, SLOT(slotToolConfigureTemplatesAction())));
	IGNORE_RETURN(connect(m_toolCompileAction, SIGNAL(activated()), this, SLOT(slotToolCompileAction())));
	IGNORE_RETURN(connect(m_toolAddToPerforceAction, SIGNAL(activated()), this, SLOT(slotToolAddToPerforceAction())));
	IGNORE_RETURN(connect(m_toolRandomAction, SIGNAL(activated()), this, SLOT(slotToolRandomAction())));
	IGNORE_RETURN(connect(m_viewShowConsoleAction, SIGNAL(activated()), this, SLOT(slotShowConsoleAction())));
	IGNORE_RETURN(connect(m_zoomHeadAction, SIGNAL(activated()), this, SLOT(slotZoomHeadAction())));
	IGNORE_RETURN(connect(m_zoomRootAction, SIGNAL(activated()), this, SLOT(slotZoomRootAction())));
	IGNORE_RETURN(connect(m_wearableAddAction, SIGNAL(activated()), this, SLOT(slotAddWearableAction())));
	IGNORE_RETURN(connect(m_wearableRemoveAction, SIGNAL(activated()), this, SLOT(slotRemoveWearableAction())));
	IGNORE_RETURN(connect(m_wearableSearchAction, SIGNAL(activated()), this, SLOT(slotSearchAction())));
	IGNORE_RETURN(connect(m_comboBoxPresets, SIGNAL(activated(int)), this, SLOT(slotComboBoxPresetsActivated(int))));
	IGNORE_RETURN(connect(m_lineEditFileFilter, SIGNAL(returnPressed()), this, SLOT(slotSearchAction())));
	IGNORE_RETURN(connect(m_lineEditFileFilter, SIGNAL(textChanged(const QString &)), this, SLOT(slotFilterTextChanged(const QString &))));
	IGNORE_RETURN(connect(m_comboBoxFilterPage, SIGNAL(activated(int)), this, SLOT(slotSearchAction())));

	//-- initialization
	m_gameWidget->setupWearableFilterPresets(m_comboBoxPresets);
	slotComboBoxPresetsActivated(0);

	loadNpcDataTable();

	clearTemplateParameters();

	//-- setup the scrollable avatar custom variable controls
	{
		QBoxLayout *topLayout = new QVBoxLayout(m_frameAvatarCustomization);

		//-- use a scroll view for scrollable dynamic custom variable controls
		m_avatarScrollView = new QScrollView(m_frameAvatarCustomization);
		topLayout->addWidget(m_avatarScrollView);

		//-- the default viewport background is different for normal background so change it
		m_avatarScrollView->viewport()->setPaletteBackgroundColor(m_frameAvatarCustomization->paletteBackgroundColor());
		m_avatarScrollView->setHidden(false);
	}	//lint !e429

	//-- setup the scrollable wearable custom variable controls
	{
		QBoxLayout *topLayout = new QVBoxLayout(m_frameWearableCustomization);

		//-- use a scroll view for scrollable dynamic custom variable controls
		m_wearableScrollView = new QScrollView(m_frameWearableCustomization);
		topLayout->addWidget(m_wearableScrollView);

		//-- the default viewport background is different for normal background so change it
		m_wearableScrollView->viewport()->setPaletteBackgroundColor(m_frameWearableCustomization->paletteBackgroundColor());
		m_wearableScrollView->setHidden(false);
	}	//lint !e429

	//-- setup the data driven backdrop menu
	{
		bool done = false;
		int maxBackdrops = 32;
		int i = 0;
		int id = 0;

		BackdropMenu->setCheckable(true);

		while(!done && i < maxBackdrops)
		{
			char const * backdrop = ConfigFile::getKeyString("NpcEditor", "backdrop", i++, 0);

			if(backdrop)
				IGNORE_RETURN(BackdropMenu->insertItem(backdrop, this, SLOT(slotChangeBackdrop(int)), 0, id++));
			else
				done = true;
		}

		slotChangeBackdrop(0);
	}
}

// ----------------------------------------------------------------------------

MainWindow::~MainWindow()
{
	//-- save the widget settings
	QtUtility::saveWidget(*this);
	
	m_consoleWindow = 0; //lint !e423
	m_proc = 0; //lint !e423
	m_avatarVBox = 0;
	m_avatarScrollView = 0; //lint !e423
	m_wearableVBox = 0;
	m_wearableScrollView = 0; //lint !e423
}

// ----------------------------------------------------------------------------

void MainWindow::slotFileNewAction()
{
	if(m_dirty)
	{
		int ret = QMessageBox::warning(this, "New Npc", "Creating a new Npc will lose all the current changes.", QMessageBox::Ok, QMessageBox::Cancel);
		if (ret != QMessageBox::Ok)
			return;
	}

	NewNpcDialog * dlg = new NewNpcDialog(this);

	dlg->setupControls(&m_npcDataTable);

	int result2 = dlg->exec();

	if (result2 == static_cast<int>(QDialog::Accepted))
	{
		clearFiles();

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileNewAction() - ACCEPTED\n"));

		m_row = dlg->getSelectedSATRow();
		std::string satFile(m_npcDataTable.getStringValue("SatFile", m_row));

		m_gameWidget->setAvatar(satFile.c_str());
		createAvatarCustomVariableWidgets();

		m_EditingFrameMain->setEnabled(true);

		m_openExistingMode = false;
		m_overwriteTemplatesMode = false;
		m_dirty = false;
	}
	else
	{
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileNewAction() - CANCELLED\n"));
	}
} //lint !e429

// ----------------------------------------------------------------------------

void MainWindow::slotFileOpenAction()
{
	if(m_dirty)
	{
		int ret = QMessageBox::warning(this, "New Npc", "Openning an Npc will lose all the current changes.", QMessageBox::Ok, QMessageBox::Cancel);
		if (ret != QMessageBox::Ok)
			return;
	}

	QString defaultDirectory = ConfigFile::getKeyString(c_npcEditor, c_defaultSharedDsrc, 0, 0);

	IGNORE_RETURN(defaultDirectory.replace("dsrc", "data"));

	QString selectedFileName(QFileDialog::getOpenFileName(defaultDirectory, "Npc files (*.iff)", this, "OpenFileDialog", "Open Shared Object Template"));
	
	//-- make sure the specified file exists on disk
	if(!selectedFileName.isNull() && FileNameUtils::isReadable(selectedFileName.latin1()))
	{
		clearFiles();

		m_gameWidget->setModel(selectedFileName);
		createAvatarCustomVariableWidgets();

		m_EditingFrameMain->setEnabled(true);

		m_openExistingMode = true;
		m_overwriteTemplatesMode = false;

		CreatureObject const * creature = m_gameWidget->getViewer()->getCreature();
		Appearance const * a = creature->getAppearance();
		AppearanceTemplate const * t = a->getAppearanceTemplate();

		int col = m_npcDataTable.findColumnNumber("SatFile");
		m_row = m_npcDataTable.searchColumnString(col, t->getCrcName().getString());

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - SAT = [%s] Row = [%d]\n", t->getCrcName().getString(), m_row));

		SharedObjectTemplate const * sot = creature->getObjectTemplate()->asSharedObjectTemplate();
		char const * cdfFile = sot->getClientDataFile().c_str();

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - CDF = [%s]\n", cdfFile));

		//-- setup the client data file info
		{
			QString fileName = ConfigFile::getKeyString(c_npcEditor, c_defaultClientDsrc, 0, 0);

			IGNORE_RETURN(fileName.replace("/dsrc/", "/data/"));
			IGNORE_RETURN(fileName.replace("clientdata/npc", cdfFile));

			m_clientCdfFileInfo.setFile(fileName);

			IGNORE_RETURN(fileName.replace("/data/", "/dsrc/"));
			IGNORE_RETURN(fileName.replace(".cdf", ".mif"));

			m_clientFileInfo.setFile(fileName);
			updateCaption();
		}

		//-- setup the shared template file info
		{
			QString fileName = selectedFileName;

			m_sharedIffFileInfo.setFile(fileName);

			IGNORE_RETURN(fileName.replace("/data/", "/dsrc/"));
			IGNORE_RETURN(fileName.replace(".iff", ".tpf"));

			m_sharedFileInfo.setFile(fileName);

		}

		//-- setup the server template file info
		{
			QString fileName = selectedFileName;

			IGNORE_RETURN(fileName.replace("sys.shared", "sys.server"));
			IGNORE_RETURN(fileName.replace("/shared_", "/"));

			m_serverIffFileInfo.setFile(fileName);

			IGNORE_RETURN(fileName.replace("/data/", "/dsrc/"));
			IGNORE_RETURN(fileName.replace(".iff", ".tpf"));

			m_serverFileInfo.setFile(fileName);
		}

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Client Data File = [%s]\n", m_clientCdfFileInfo.filePath().latin1()));
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Client Src  File = [%s]\n", m_clientFileInfo.filePath().latin1()));

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Shared Data File = [%s]\n", m_sharedIffFileInfo.filePath().latin1()));
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Shared Src  File = [%s]\n", m_sharedFileInfo.filePath().latin1()));

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Server Data File = [%s]\n", m_serverIffFileInfo.filePath().latin1()));
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileOpenAction() - Server Src  File = [%s]\n", m_serverFileInfo.filePath().latin1()));

		m_dirty = false;
	}
	else if (!selectedFileName.isEmpty())
	{
		QString text;
		IGNORE_RETURN(text.sprintf("The selected file does not exist on disk: \"%s\"", selectedFileName.latin1()));
		IGNORE_RETURN(QMessageBox::warning(this, "File Error", text, "OK"));
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotFileSaveAsAction()
{
	SaveDialog * saveDialog = new SaveDialog(this, false);

	if(haveFileNames())
	{
		if(m_clientFileInfo.isFile())
			saveDialog->setClientDataFileName(m_clientFileInfo.filePath());
		if(m_serverFileInfo.isFile())
			saveDialog->setServerFileName(m_serverFileInfo.filePath());
		if(m_sharedFileInfo.isFile())
			saveDialog->setSharedFileName(m_sharedFileInfo.filePath());
	}

	int result = saveDialog->exec();

	if(result == static_cast<int>(QDialog::Accepted))
	{
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileSaveAsAction() - ACCEPTED\n"));

		//-- write out all the files
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileSaveAsAction() - [%s]\n", saveDialog->getClientDataFileName()));

		QString originalClientFile = m_clientFileInfo.filePath();
		setClientDataFile(saveDialog->getClientDataFileName());
		saveClientDataFile(originalClientFile);

		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileSaveAsAction() - [%s]\n", saveDialog->getSharedFileName()));

		QString originalSharedFile = m_sharedFileInfo.filePath();
		setSharedTemplate(saveDialog->getSharedFileName());
		saveSharedTemplate(originalSharedFile);
		
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileSaveAsAction() - [%s]\n", saveDialog->getServerFileName()));

		QString originalServerFile = m_serverFileInfo.filePath();
		setServerTemplate(saveDialog->getServerFileName());
		saveServerTemplate(originalServerFile);

		m_dirty = false;
	}
	else
	{
		DEBUG_REPORT_LOG(false, ("MainWindow::slotFileSaveAsAction() - CANCELLED\n"));
	}
} //lint !e429

// ----------------------------------------------------------------------------

void MainWindow::slotFileSaveAction()
{
	if(haveFileNames())
	{
		saveClientDataFile(m_clientFileInfo.filePath());
		saveSharedTemplate(m_sharedFileInfo.filePath());
		saveServerTemplate(m_serverFileInfo.filePath());

		m_dirty = false;
	}
	else
	{
		slotFileSaveAsAction();
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotToolConfigureTemplatesAction()
{
	TemplateConfigDialog * dlg = new TemplateConfigDialog(this);

	dlg->setupControls(&m_npcDataTable, &m_serverParameters, &m_sharedParameters);

	int result = dlg->exec();

	if (result == static_cast<int>(QDialog::Accepted))
	{
		dlg->updateParameters(&m_serverParameters, &m_sharedParameters);

		DEBUG_REPORT_LOG(false, ("MainWindow::slotToolConfigureTemplatesAction() - ACCEPTED\n"));

		if(m_openExistingMode)
		{
			m_overwriteTemplatesMode = true;

			WARNING(true, ("MainWindow::slotToolConfigureTemplatesAction() - Existing templates will be overwritten using the provided parameters."));
		}
	}
	else
	{
		DEBUG_REPORT_LOG(false, ("MainWindow::slotToolConfigureTemplatesAction() - CANCELLED\n"));
	}
}  //lint !e429

// ----------------------------------------------------------------------------

void MainWindow::slotToolCompileAction()
{
	if(m_proc->isRunning() || !m_procArgList.empty())
	{
		CONSOLE_OUTPUT("*** Process running. Please wait for process to exit. ***\n");
		return;
	}

	if(m_serverFileInfo.isFile())
	{
		std::vector< std::string > v;
		v.push_back("templatecompiler");
		v.push_back("-compileeditor");
		v.push_back(m_serverFileInfo.filePath().latin1());

		m_procArgList.push_back(v);
	}

	if(m_sharedFileInfo.isFile())
	{
		std::vector< std::string > v;
		v.push_back("templatecompiler");
		v.push_back("-compileeditor");
		v.push_back(m_sharedFileInfo.filePath().latin1());

		m_procArgList.push_back(v);
	}

	if(m_clientFileInfo.isFile())
	{
		std::vector< std::string > v;
		v.push_back("miff");
		v.push_back("-i");
		v.push_back(m_clientFileInfo.filePath().latin1());
		v.push_back("-o");
		v.push_back(m_clientCdfFileInfo.filePath().latin1());

		m_procArgList.push_back(v);
	}

	startNextProcess();
}

// ----------------------------------------------------------------------------

void MainWindow::startNextProcess()
{
	QString cmdString;

	m_proc->clearArguments();

	if(m_procArgList.empty())
	{
		CONSOLE_OUTPUT("\n*** Finished running all processes ***\n\n");
		return;
	}

	std::vector < std::string > v = m_procArgList.front();

	m_procArgList.pop_front();

	std::vector < std::string > :: iterator i = v.begin();

	for(; i != v.end(); ++i)
	{
		m_proc->addArgument( i->c_str() );

		if(!cmdString.isEmpty())
			IGNORE_RETURN(cmdString.append(" "));
		IGNORE_RETURN(cmdString.append( i->c_str()));
	}

	if ( !m_proc->start() )
	{
		CONSOLE_OUTPUT("*** Failure: [" + cmdString + "] ***\n");

		//-- if there is a failure, run the next process
		startNextProcess();
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotReadProcessOutput()
{
	CONSOLE_OUTPUT(m_proc->readStdout());
	CONSOLE_OUTPUT(m_proc->readStderr());
}

// ----------------------------------------------------------------------------

void MainWindow::slotProcessExited()
{
	startNextProcess();
}

// ----------------------------------------------------------------------------

void MainWindow::AddToPerforce(const QFileInfo &fi)
{
	if(fi.isFile())
	{
		{
			std::vector< std::string > v;
			v.push_back("p4");
			v.push_back("edit");
			v.push_back(fi.filePath().lower().latin1());
		
			m_procArgList.push_back(v);
		}
		{
			std::vector< std::string > v;
			v.push_back("p4");
			v.push_back("add");
			v.push_back(fi.filePath().lower().latin1());
		
			m_procArgList.push_back(v);
		}
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotToolAddToPerforceAction()
{
	if(m_proc->isRunning() || !m_procArgList.empty())
	{
		CONSOLE_OUTPUT("*** Process running. Please wait for process to exit. ***\n");
		return;
	}

	AddToPerforce(m_clientFileInfo);
	AddToPerforce(m_serverFileInfo);
	AddToPerforce(m_sharedFileInfo);
	AddToPerforce(m_clientCdfFileInfo);
	AddToPerforce(m_serverIffFileInfo);
	AddToPerforce(m_sharedIffFileInfo);

	startNextProcess();
}

// ----------------------------------------------------------------------------

void MainWindow::setFileInfo(QString const &path, QFileInfo &dsrcFileInfo, char const * dsrcExt,
														 QFileInfo &dataFileInfo, char const * dataExt)
{
	QString newPath((FileNameUtils::get(path.latin1(),
			FileNameUtils::drive | FileNameUtils::directory | FileNameUtils::fileName).c_str()) + QString(dsrcExt));
	
	//-- when a relative path ("../../blah") is converted as above, the ".." in front gets removed so add it back
	if (newPath.at(0) == '/')
		IGNORE_RETURN(newPath.prepend( ".." ));

	DEBUG_REPORT_LOG(false, ("MainWindow::setFileInfo() - path    = [%s]\n", path.latin1()   ));
	DEBUG_REPORT_LOG(false, ("MainWindow::setFileInfo() - newPath = [%s]\n", newPath.latin1()));

	if(!newPath.isNull() && FileNameUtils::isWritable(newPath.latin1()))
	{
		QString outputFile;

		dsrcFileInfo.setFile(newPath);
	
		outputFile = dsrcFileInfo.filePath();

		IGNORE_RETURN(outputFile.replace("dsrc", "data"));
		IGNORE_RETURN(outputFile.replace(dsrcExt, dataExt));

		dataFileInfo.setFile(outputFile);
	}
	else if (!newPath.isEmpty())
	{
		QString text;

		IGNORE_RETURN(text.sprintf("Error saving: \"%s\". Make sure the file is not read only.",
			FileNameUtils::get(newPath.latin1(), FileNameUtils::fileName | FileNameUtils::extension).c_str()));

		IGNORE_RETURN(QMessageBox::warning(this, "Error Saving File", text, "OK."));
	}
}

// ----------------------------------------------------------------------------

void MainWindow::saveClientDataFile(QString const & originalFile)
{
	std::string sourceFileName;

	//-- pass mif source file when openning existing npcs
	if(m_openExistingMode)
		sourceFileName = originalFile.latin1();
	else
	{
		char fullPathName[2048];
		char const * relativePathName = m_npcDataTable.getStringValue("SoundFile", m_row);
		NOT_NULL(relativePathName);

		if (!TreeFile::getPathName(relativePathName, fullPathName, sizeof(fullPathName) - 1))
		{
			WARNING(true, ("MainWindow::saveClientDataFile() - Could not find [%s]. Check search path in configs.", relativePathName));
			return;
		}

		sourceFileName = fullPathName;
	}

	DEBUG_REPORT_LOG(false, ("MainWindow::saveClientDataFile() - [%s] [%s]\n",
		m_clientFileInfo.filePath().latin1(), sourceFileName.c_str()));

	bool ret = ClientDataFileWriter::write(m_gameWidget->getViewer()->getCreature(),
		m_clientFileInfo.filePath().latin1(), sourceFileName.c_str(), m_openExistingMode);

	if(!ret)
		IGNORE_RETURN(QMessageBox::warning(this, "Error Saving File", m_clientFileInfo.filePath().latin1(), "OK."));
}

// ----------------------------------------------------------------------------

void MainWindow::setClientDataFile(QString const &path)
{
	setFileInfo(path, m_clientFileInfo, ".mif", m_clientCdfFileInfo, ".cdf");
	updateCaption();
}

// ----------------------------------------------------------------------------

void MainWindow::saveServerTemplate(QString const & originalFile)
{
	if(m_serverFileInfo.isFile())
	{
		QString relativeOutputName = m_sharedFileInfo.filePath();
		SaveDialog::makeSharedNameRelative(&relativeOutputName);
		IGNORE_RETURN(relativeOutputName.replace(".tpf", ".iff", false));

		m_serverParameters["sharedTemplate"] = relativeOutputName;

		if (!m_openExistingMode || m_overwriteTemplatesMode)
		{
			QString serverFileName;

			{
				char fullPathName[2048];
				char const * relativePathName = m_npcDataTable.getStringValue("ServerFile", m_row);
				NOT_NULL(relativePathName);

				if (!TreeFile::getPathName(relativePathName, fullPathName, sizeof(fullPathName) - 1))
				{
					WARNING(true, ("MainWindow::saveServerTemplate() - Could not find [%s]. Check search path in configs.", relativePathName));
					return;
				}

				serverFileName = fullPathName;
			}

			IGNORE_RETURN(ObjectTemplateWriter::write(m_serverFileInfo.filePath(), serverFileName, &m_serverParameters));

			DEBUG_REPORT_LOG(false, ("MainWindow::saveServerTemplate() - [%s] [%s]\n",
				m_serverFileInfo.filePath().latin1(), serverFileName.latin1()));

		}
		else
		{
			if (!copyUpdatedFile(originalFile.latin1(), m_serverFileInfo.filePath().latin1(), "sharedTemplate", relativeOutputName.latin1()))
				IGNORE_RETURN(QMessageBox::warning(this, "File Error", "Failed to save server file.", "OK"));
		}

	}
}

// ----------------------------------------------------------------------------

void MainWindow::setServerTemplate(QString const &path)
{
	setFileInfo(path, m_serverFileInfo, ".tpf", m_serverIffFileInfo, ".iff");
}

// ----------------------------------------------------------------------------

void MainWindow::saveSharedTemplate(QString const & originalFile)
{
	if(m_sharedFileInfo.isFile())
	{
		QString relativeOutputName = m_clientFileInfo.filePath();
		SaveDialog::makeClientNameRelative(&relativeOutputName);
		IGNORE_RETURN(relativeOutputName.replace(".mif", ".cdf", false));

		m_sharedParameters["clientDataFile"] = relativeOutputName;

		if (!m_openExistingMode || m_overwriteTemplatesMode)
		{
			QString sharedFileName;

			{
				char fullPathName[2048];
				char const * relativePathName = m_npcDataTable.getStringValue("SharedFile", m_row);
				NOT_NULL(relativePathName);

				if (!TreeFile::getPathName(relativePathName, fullPathName, sizeof(fullPathName) - 1))
				{
					WARNING(true, ("MainWindow::saveSharedTemplate() - Could not find [%s]. Check search path in configs.", relativePathName));
					return;
				}

				sharedFileName = fullPathName;
			}

			m_sharedParameters["appearanceFilename"] = m_npcDataTable.getStringValue("SatFile", m_row);

			DEBUG_REPORT_LOG(false, ("MainWindow::saveSharedTemplate() - [%s] [%s]\n",
				m_sharedFileInfo.filePath().latin1(), sharedFileName.latin1()));

			IGNORE_RETURN(ObjectTemplateWriter::write(m_sharedFileInfo.filePath(), sharedFileName, &m_sharedParameters));
		}
		else
		{
			if (!copyUpdatedFile(originalFile.latin1(), m_sharedFileInfo.filePath().latin1(), "clientDataFile", relativeOutputName.latin1()))
				IGNORE_RETURN(QMessageBox::warning(this, "File Error", "Failed to write shared file.", "OK"));
		}
	}
}

// ----------------------------------------------------------------------------

void MainWindow::setSharedTemplate(QString const &path)
{
	setFileInfo(path, m_sharedFileInfo, ".tpf", m_sharedIffFileInfo, ".iff");
}

// ----------------------------------------------------------------------------

void MainWindow::slotFileExitAction()
{
	if(m_dirty)
	{
		int ret = QMessageBox::warning(this, "Exit without saving", "Are you sure you want to exit without saving?", QMessageBox::Ok, QMessageBox::Cancel);
		if (ret != QMessageBox::Ok)
			return;
	}

	IGNORE_RETURN(close());
}

// ----------------------------------------------------------------------------

void MainWindow::paintEvent(QPaintEvent *eventPaint)
{
	BaseMainWindow::paintEvent(eventPaint);

	updateCaption();
}

// ----------------------------------------------------------------------------

void MainWindow::updateCaption()
{
	//-- set the caption
	std::string path(FileNameUtils::get(m_clientFileInfo.filePath().latin1(), FileNameUtils::fileName | FileNameUtils::extension));

	char text[256];
	sprintf(text, "Npc Editor (%s - %s) : %s", __DATE__, __TIME__, (m_clientFileInfo.fileName() == "") ? "Default" : path.c_str());
	setCaption(text);
}

// ----------------------------------------------------------------------------

void MainWindow::slotAddWearableAction()
{
	//-- set the file
	QString lastPath(FileNameUtils::get(QtUtility::getLastPath().latin1(), FileNameUtils::drive | FileNameUtils::directory).c_str());
	QString selectedFileName(QFileDialog::getOpenFileName(lastPath, "LMG files (*.lmg)", this, "OpenFileDialog", "Open LMG file"));
	
	//-- make sure the specified file exists on disk
	if (   !selectedFileName.isNull()
	    && FileNameUtils::isReadable(selectedFileName.latin1()))
	{
		m_gameWidget->wear(selectedFileName);
		QtUtility::setLastPath(selectedFileName);
	}
	else if (!selectedFileName.isEmpty())
	{
		QString text;
		IGNORE_RETURN(text.sprintf("The selected file does not exist on disk: \"%s\"", selectedFileName.latin1()));
		IGNORE_RETURN(QMessageBox::warning(this, "File Error", text, "OK"));
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotSearchAction()
{
	std::string wearableName = m_lineEditFileFilter->text().ascii();
	m_gameWidget->addFilteredWearables(wearableName.c_str(), m_comboBoxFilterPage);
}

// ----------------------------------------------------------------------------

void MainWindow::slotRemoveWearableAction()
{
	m_gameWidget->removeSelectedWearable();
}

// ----------------------------------------------------------------------------

void MainWindow::slotToolRandomAction()
{
	m_gameWidget->testCustomization();
	m_dirty = true;

	//-- recreate the custom variable widgets
	createAvatarCustomVariableWidgets();
	createWearableCustomVariableWidgets();
}

// ----------------------------------------------------------------------------

void MainWindow::loadNpcDataTable()
{
	if (FileNameUtils::isReadable(c_npcEditorDataTable))
	{
		DataTableWriter dataTableWriter;
		dataTableWriter.loadFromSpreadsheet(c_npcEditorDataTable);
		Iff iff(2);
		dataTableWriter.save(iff);

		iff.allowNonlinearFunctions();
		iff.goToTopOfForm();

		m_npcDataTable.load(iff);
	}
}

// ----------------------------------------------------------------------------

void MainWindow::slotZoomHeadAction()
{
	m_gameWidget->getViewer()->zoomBone("head");
}

// ----------------------------------------------------------------------------

void MainWindow::slotZoomRootAction()
{
	m_gameWidget->getViewer()->zoomBone("root");
}

// ----------------------------------------------------------------------------

void MainWindow::slotViewerDirty()
{
	createWearableCustomVariableWidgets();
}

// ----------------------------------------------------------------------------

void MainWindow::slotComboBoxPresetsActivated(int)
{
	int selectedItem = m_comboBoxPresets->currentItem();

	const char * filter = m_gameWidget->getWearableFilterPreset(selectedItem);

	m_lineEditFileFilter->setText(filter);
}

// ----------------------------------------------------------------------------

bool MainWindow::haveFileNames() const
{
	return m_clientFileInfo.isFile() || m_serverFileInfo.isFile() || m_sharedFileInfo.isFile();
}

// ----------------------------------------------------------------------------

void MainWindow::slotShowConsoleAction()
{
	m_consoleWindow->show();
}

// ----------------------------------------------------------------------------

void MainWindow::createAvatarCustomVariableWidgets()
{
	delete m_avatarVBox;

	m_avatarVBox = new QVBox(m_avatarScrollView->viewport());
	m_avatarScrollView->addChild(m_avatarVBox);

	CustomizationData * cdata = m_gameWidget->getAvatarCustomizationData();
	if(!cdata) return;

	cdata->iterateOverVariables(callbackCreateAvatarCustomVariableWidgets, this);

	cdata->release();

	m_avatarVBox->setHidden(false);
	m_avatarScrollView->setHidden(false);
}

// ----------------------------------------------------------------------------

void MainWindow::callbackCreateAvatarCustomVariableWidgets(const std::string &fullVariablePathName,
																													 CustomizationVariable *cv, void *context)
{
	NOT_NULL(context);
	NOT_NULL(cv);

	MainWindow * mainWindow = static_cast<MainWindow *>(context);

	char const * variableName = fullVariablePathName.c_str();
	int minRange;
	int maxRange;

	CustomVariableWidget * cvw = 0;

	//-- ranged Int Variables
	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		int currentValue = ri->getValue();

		ri->getRange(minRange, maxRange);

		cvw = new CustomVariableWidget(mainWindow->m_avatarVBox, variableName, minRange, maxRange - 1, currentValue);

		IGNORE_RETURN(connect(cvw, SIGNAL(variableChanged(int, const char *)), mainWindow,
			SLOT(slotAvatarCustomizationVariableChanged(int, const char *))));

		return; //lint !e429
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		int currentValue = pv->getValue();

		pv->getRange(minRange, maxRange);

		PaletteArgb const * colorPalette = pv->fetchPalette();

		cvw = new CustomVariableWidget(mainWindow->m_avatarVBox, variableName, minRange, maxRange - 1, currentValue, colorPalette);

		colorPalette->release();

		IGNORE_RETURN(connect(cvw, SIGNAL(variableChanged(int, const char *)), mainWindow,
			SLOT(slotAvatarCustomizationVariableChanged(int, const char *))));

		return; //lint !e429
	}

	return;
}

// ----------------------------------------------------------------------------

void MainWindow::slotAvatarCustomizationVariableChanged(int value, const char * variable)
{
	DEBUG_REPORT_LOG(false, ("MainWindow::slotAvatarCustomizationVariableChanged() - %s = [%d]\n", variable, value));
	
	m_gameWidget->setIntValue(variable, value);
	m_dirty = true;
}

// ----------------------------------------------------------------------------

void MainWindow::createWearableCustomVariableWidgets()
{
	delete m_wearableVBox;

	m_wearableVBox = new QVBox(m_wearableScrollView->viewport());
	m_wearableScrollView->addChild(m_wearableVBox);

	CustomizationData * cdata = m_gameWidget->getWearableCustomizationData();
	if(!cdata) return;

	cdata->iterateOverVariables(callbackCreateWearableCustomVariableWidgets, this, false);

	cdata->release();

	m_wearableVBox->setHidden(false);
	m_wearableScrollView->setHidden(false);
}

// ----------------------------------------------------------------------------

void MainWindow::callbackCreateWearableCustomVariableWidgets(const std::string &fullVariablePathName, CustomizationVariable *cv, void *context)
{
	NOT_NULL(context);
	NOT_NULL(cv);

	MainWindow * mainWindow = static_cast<MainWindow *>(context);

	char const * variableName = fullVariablePathName.c_str();
	int minRange;
	int maxRange;

	CustomVariableWidget * cvw = 0;

	//-- ranged Int Variables
	BasicRangedIntCustomizationVariable * ri = dynamic_cast<BasicRangedIntCustomizationVariable *>(cv);
	if(ri)
	{
		int currentValue = ri->getValue();

		ri->getRange(minRange, maxRange);

		cvw = new CustomVariableWidget(mainWindow->m_wearableVBox, variableName, minRange, maxRange - 1, currentValue);

		IGNORE_RETURN(connect(cvw, SIGNAL(variableChanged(int, const char *)), mainWindow,
			SLOT(slotWearableCustomizationVariableChanged(int, const char *))));

		return; //lint !e429
	}

	PaletteColorCustomizationVariable * pv = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(pv)
	{
		int currentValue = pv->getValue();

		pv->getRange(minRange, maxRange);

		PaletteArgb const * colorPalette = pv->fetchPalette();

		cvw = new CustomVariableWidget(mainWindow->m_wearableVBox, variableName, minRange, maxRange - 1, currentValue, colorPalette);

		colorPalette->release();

		IGNORE_RETURN(connect(cvw, SIGNAL(variableChanged(int, const char *)), mainWindow,
			SLOT(slotWearableCustomizationVariableChanged(int, const char *))));
		
		return; //lint !e429
	}

	return;
}

// ----------------------------------------------------------------------------

void MainWindow::slotWearableCustomizationVariableChanged(int value, const char * variable)
{
	DEBUG_REPORT_LOG(false, ("MainWindow::slotWearableCustomizationVariableChanged() - %s = [%d]\n", variable, value));
	
	m_gameWidget->setWearableIntValue(variable, value);
	m_dirty = true;
}

// ----------------------------------------------------------------------------

void MainWindow::clearFiles()
{
	m_clientFileInfo.setFile("");
	m_serverFileInfo.setFile("");
	m_sharedFileInfo.setFile("");
	m_clientCdfFileInfo.setFile("");
	m_serverIffFileInfo.setFile("");
	m_sharedIffFileInfo.setFile("");

	updateCaption();
}

// ----------------------------------------------------------------------------

void MainWindow::slotChangeBackdrop(int id)
{
	static int previousId = -1;

	DEBUG_REPORT_LOG(false, ("MainWindow::slotChangeBackdrop() - id = %d\n", id));

	if(previousId != -1)
		BackdropMenu->setItemChecked(previousId, false);

	BackdropMenu->setItemChecked(id, true);
	m_gameWidget->setBackdrop(id);

	char const * backdrop = ConfigFile::getKeyString("NpcEditor", "backdrop", id, 0);
	m_gameWidget->getViewer()->setEnvironmentTexture(backdrop);

	previousId = id;
}

// ----------------------------------------------------------------------------

void MainWindow::slotFilterTextChanged(const QString &)
{
	m_comboBoxFilterPage->setEnabled(false);
	m_comboBoxFilterPage->clear();
}

// ----------------------------------------------------------------------------

void MainWindow::clearTemplateParameters()
{
	m_serverParameters.clear();
	m_sharedParameters.clear();

	m_serverParameters["date"] = __DATE__;
	m_serverParameters["time"] = __TIME__;
	m_sharedParameters["date"] = __DATE__;
	m_sharedParameters["time"] = __TIME__;
}

// ----------------------------------------------------------------------------

bool MainWindowNamespace::copyUpdatedFile(char const * const sourceFileName, char const * outputFileName, std::string const & key, std::string const & value)
{
	char line[4096];
	std::vector< std::string > lineBuffer;

	//-- read the full source source file line by line into a vector of strings
	{
		FILE *sourceFile = fopen(sourceFileName, "r");
		if (!sourceFile)
		{
			WARNING(true, ("Failed to open file [%s] for reading.", sourceFileName));
			return false;
		}

		while(!feof(sourceFile) && fgets(line, sizeof(line), sourceFile))
			lineBuffer.push_back(line);

		fclose(sourceFile);
	}

	//-- write the line buffer to the output file, replacing the line that matches key with key="value"
	{
		FILE *outputFile = fopen(outputFileName, "w");
		if (!outputFile)
		{
			WARNING(true, ("Failed to open file [%s] for writing.", outputFileName));
			return false;
		}

		std::vector< std::string >::iterator i = lineBuffer.begin();

		for (; i != lineBuffer.end(); ++i)
		{
			if (i->find(key) != static_cast<unsigned>(std::string::npos))
				*i = key + " = \"" + value + "\"";

			fputs(i->c_str(), outputFile);
		}

		fclose(outputFile);
	}

	return true;
}

// ============================================================================
