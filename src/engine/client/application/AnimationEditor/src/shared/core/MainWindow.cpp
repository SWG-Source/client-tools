// ======================================================================
//
// MainWindow.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/MainWindow.h"

#include "AnimationEditor/AnimationEditorGameWorld.h"
#include "AnimationEditor/FileModel.h"
#include "AnimationEditor/FindDialog.h"
#include "AnimationEditor/FindReplaceDialog.h"
#include "AnimationEditor/AnimationStateHierarchyWidget.h"
#include "AnimationEditor/LogicalAnimationTableWidget.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameAnimationMessageCallback.h"
#include "clientGame/GroundScene.h"
#include "clientSkeletalAnimation/AnimationStateHierarchyTemplateList.h"
#include "clientSkeletalAnimation/EditableAnimationStateHierarchyTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplate.h"
#include "clientSkeletalAnimation/LogicalAnimationTableTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/Object.h"

#include <qfiledialog.h>
#include <qsettings.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qmessagebox.h>
#include <string>
#include <vector>

// ======================================================================

const std::string MainWindow::ms_ashTabNamePrefix("ASH: ");
const std::string MainWindow::ms_latTabNamePrefix("LAT: ");
const std::string MainWindow::ms_unnamedTabNamePrefix("new");

int         MainWindow::ms_newTabCounter;
MainWindow *MainWindow::ms_theMainWindow;

// ======================================================================

MainWindow &MainWindow::getInstance()
{
	NOT_NULL(ms_theMainWindow);
	return *ms_theMainWindow;
}

// ======================================================================

MainWindow::MainWindow(QWidget* newParent, const char* newName, WFlags fl) :
	BaseMainWindow(newParent, newName, fl),
	m_fileModelVector(new FileModelVector()),
	m_findDialog(0),
	m_findReplaceDialog(0)
{
	//-- Set the single MainWindow instance for the app.
	DEBUG_FATAL(ms_theMainWindow, ("A main window has been set already, that shouldn't happen."));
	ms_theMainWindow = this;

	//-- Remove default pages added by Qt.  We procedurally fill pages
	//   into this tab controller.
	m_tabWidget->removePage(tab);
	m_tabWidget->removePage(tab_2);
}

// ----------------------------------------------------------------------

MainWindow::~MainWindow()
{
	// Qt deletes all FileModel (tab) instances for us since all existing
	// file models live as children of the tab widget.
	delete m_fileModelVector;

	//lint -esym(1540, MainWindow::m_hierarchyTab) // not zeroed or freed // Qt deletes for us.
	// m_hierarchyTab = 0; 

	//-- Clear the singleton instance variable.
	ms_theMainWindow = 0;
	delete m_findDialog;
	delete m_findReplaceDialog;
}

// ----------------------------------------------------------------------
/**
 * Opens a new shared creature template and builds up the tabs
 */

void  MainWindow::openSharedCreatureTemplate()
{
	// grab the last path that was used to open a template	
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, "/SOE/AnimationEditor");
	QString previousUsedPath(settings.readEntry("OpenSharedTemplatePath", QString::null));
	
	char warningBuffer[512];

	//-- Get file to open from the Qt common file dialog.
	const QString qPathName = QFileDialog::getOpenFileName(previousUsedPath, "Template Files (*.iff)", this, "open file dialog", "Choose a file to open");
	if (qPathName == QString::null)
	{
		// user aborted
		return;
	}
	
	std::string absolutePath(qPathName);
	std::string  treeFileRelativePath;

	if(absolutePath.find("shared") == std::string::npos)
	{
		snprintf(warningBuffer, sizeof(warningBuffer), "[%s] is not a shared template.", absolutePath.c_str());
		warningBuffer[sizeof(warningBuffer) - 1] = '\0';
		QMessageBox::warning(this, "Shared Template Open Error", warningBuffer, QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	if(absolutePath.find(".iff") == std::string::npos)
	{
		snprintf(warningBuffer, sizeof(warningBuffer), "[%s] is not a template file.", absolutePath.c_str());
		warningBuffer[sizeof(warningBuffer) - 1] = '\0';
		QMessageBox::warning(this, "Shared Template Open Error", warningBuffer, QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	if (!TreeFile::stripTreeFileSearchPathFromFile(absolutePath, treeFileRelativePath))
	{
		snprintf(warningBuffer, sizeof(warningBuffer), "the specified file [%s] is not mappable to your TreeFile path.  Fix path before opening.", static_cast<const char*>(qPathName));
		warningBuffer[sizeof(warningBuffer) - 1] = '\0';
		QMessageBox::warning(this, "Shared Template Open Error", warningBuffer, QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	NetworkScene* networkScene = dynamic_cast<NetworkScene*> (Game::getScene ());
	if (!networkScene)
	{
		WARNING(true, ("Could not get the NetworkScene."));
		return;
	}

	// save out the path that was used to open the template
	IGNORE_RETURN(settings.writeEntry("OpenSharedTemplatePath", qPathName));

	CreatureObject *creatureObject = dynamic_cast<CreatureObject *>(networkScene->getPlayer());
	creatureObject->setupAlternateSharedCreatureObjectTemplate(TemporaryCrcString(treeFileRelativePath.c_str(),true));

	//-- Tell the main window to open ASH and LAT files associated with the player object (the current target).
	// ... first force target object to rebuild so that .ASH and .LAT data are setup on it.
	Object *const focusObject = AnimationEditorGameWorld::getFocusObject();
	if (focusObject)
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(focusObject->getAppearance());
		if (appearance)
		{
			appearance->setUserControlledDetailLevel(true);
			appearance->setDetailLevel(0);
			appearance->rebuildMesh(0);
			appearance->setUserControlledDetailLevel(false);
		}
	}

	// delete all the tabs for the old appearance
	FileModel *fileModel = 0;
	int        index     = -1;
	while(getSelectedFileModel(fileModel, index))
	{
		closeSelectedTab();
	}

	// rebuild the tabs for the new appearance
	openTargetAshFiles();
	openTargetLatFiles();
}

// ----------------------------------------------------------------------
/**
 * Launch the Find text dialog for searching the tree views
*/

void MainWindow::openFindDialog()
{
	if(!m_findDialog)
	{
		m_findDialog = new FindDialog(this,"FindDialog");
	}
	m_findDialog->show();
}

// ----------------------------------------------------------------------
/**
 * Launch the replace text dialog for replacing text in the tree views
*/

void MainWindow::openFindReplaceDialog()
{
	if(!m_findReplaceDialog)
	{
		m_findReplaceDialog = new FindReplaceDialog(this,"FindReplaceDialog");
	}
	m_findReplaceDialog->show();
}

// ----------------------------------------------------------------------
/**
 * Create a new AnimationStateHierarchyTemplate (ASH file) tab window.
 */

void MainWindow::newAnimationStateHierarchy()
{
	FileModel *const newFileModel = new AnimationStateHierarchyWidget(m_tabWidget);
	newFileModel->setFileTitle(getNextNewTitle());

	addFileModel(newFileModel);
	//lint -esym(429, newFileModel) // custodial pointer not freed or returned // it's okay, Qt owns.
}

// ----------------------------------------------------------------------
/**
 * Create a new LogicalAnimationTableTemplate (LAT file) tab window.
 *
 * The user must have the .ash file tab selected from which the
 * LAT file will derive.
 */

void MainWindow::newLogicalAnimationTable()
{
	//-- Get the currently-selected .ash file.

	// Retrieve the currently-selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool hasSelectedFile = getSelectedFileModel(fileModel, index);
	if (!hasSelectedFile)
	{
		WARNING(true, ("User must have an .ash file opened and selected when making a new .lat file."));
		return;
	}

	// Check if selected file model is an .ash file.
	AnimationStateHierarchyWidget *const ashWidget = dynamic_cast<AnimationStateHierarchyWidget*>(fileModel);
	if (!ashWidget)
	{
		WARNING(true, ("User must have an .ash file opened and selected when making a new .lat file."));
		return;
	}

	//-- Create the new LogicalAnimationTableTemplate.
	//lint -esym(429, latTemplate) // custodial pointer not freed or returned // Qt widget owns it.
	LogicalAnimationTableTemplate *const latTemplate = new LogicalAnimationTableTemplate(ashWidget->getHierarchyTemplate());
	latTemplate->createUnspecifiedEntries();

	//-- Create the tab window widget.
	//lint -esym(429, newWidget) // custodial pointer not freed or returned // Qt owns it.
	LogicalAnimationTableWidget *const newWidget = new LogicalAnimationTableWidget(m_tabWidget, *latTemplate, "");
	addFileModel(newWidget);
}

// ----------------------------------------------------------------------
/**
 * Close the selected tab window.
 *
 * The selected tab window is the currently visible tab window.
 */

void MainWindow::closeSelectedTab()
{
	//-- get the currently selected file model (tab)
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("closeSelectedTab(): invoked with no tab selected."));
		return;
	}

	//-- get the widget for the currently selected page
	QWidget *const currentPage = fileModel->getWidget();
	if (!currentPage)	
		return;

	//-- delete the currently selected page
	m_tabWidget->removePage(currentPage);
	delete currentPage;

	//-- remove the element from the FileModel collection
	FileModelVector::iterator deleteIt = m_fileModelVector->begin() + static_cast<FileModelVector::size_type>(index);
	IGNORE_RETURN(m_fileModelVector->erase(deleteIt));
}

// ----------------------------------------------------------------------

void MainWindow::markSelectedTabModified()
{
	//-- get the currently selected file model (tab)
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("markSelectedTabModified(): invoked with no tab selected."));
		return;
	}

	//-- set the state of the file model to "modified"
	NOT_NULL(fileModel);
	fileModel->setModifiedState(true);
}

// ----------------------------------------------------------------------

void MainWindow::syncFocusController()
{
	//-- Get the focus Object.
	Object *const object = AnimationEditorGameWorld::getFocusObject();
	if (!object)
	{
		REPORT_LOG(true, ("No focus object present to do state change.\n"));
		return;
	}

	//-- Get the SkeletalAppearance2 Appearance for the Object.
	SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(object->getAppearance());
	if (!appearance)
	{
		REPORT_LOG(true, ("Focus object is not a skeletal appearance, can't set value.\n"));
		return;
	}

	//-- Get TransformAnimationResolver from appearance, tell it to rebuild controllers.
	appearance->getAnimationResolver().recreateAnimationControllers();
	GameAnimationMessageCallback::addCallback(*object);
}

// ----------------------------------------------------------------------

void MainWindow::openFile()
{
	//-- Get file to open from the Qt common file dialog.
	const QString qPathName = QFileDialog::getOpenFileName(QString::null, "Editor Files (*.ash *.lat *.xml)", this, "open file dialog", "Choose a file to open");
	if (qPathName == QString::null)
	{
		// user aborted
		return;
	}

	//-- Try to convert Qt full pathname to a TreeFile-relative path.  This allows the
	//   data file "name" attribute to match what other assets will load in the game,
	//   causing the editor and the game to use the same (potentially modified) data assets.
	std::string  treeFileRelativePath;

	if (!TreeFile::stripTreeFileSearchPathFromFile(std::string(qPathName), treeFileRelativePath))
	{
		WARNING(true, ("User: the specified file [%s] is not mappable to your TreeFile path.  Fix path before opening.", static_cast<const char*>(qPathName)));
		return;
	}

	doFileOpen(treeFileRelativePath);
}

// ----------------------------------------------------------------------

void MainWindow::openTargetLatFiles()
{
	//-- Get the editor's idea of the focus object.
	const Object *const focusObject = AnimationEditorGameWorld::getFocusObject();
	if (!focusObject)
	{
		DEBUG_REPORT_LOG(true, ("No editor focus object, ignoring request.\n"));
		return;
	}

	//-- Retrieve the skeletal appearance for the object.
	const SkeletalAppearance2 *const appearance = dynamic_cast<const SkeletalAppearance2*>(focusObject->getAppearance());
	if (!appearance)
	{
		DEBUG_REPORT_LOG(true, ("Editor focus object does not have a skeletal appearance, ignoring request.\n"));
		return;
	}

	//-- Get the animation controller for each skeleton template associated with the appearance.
	const TransformAnimationResolver &resolver = appearance->getAnimationResolver();
	const int skeletonTemplateCount            = resolver.getSkeletonTemplateCount();

	for (int i = 0; i < skeletonTemplateCount; ++i)
	{
		// Get the animation controller.
		const StateHierarchyAnimationController *const controller = dynamic_cast<const StateHierarchyAnimationController*>(resolver.getAnimationController(i));
		if (!controller)
			continue;

		// Fetch the LAT data structure from it.
		const LogicalAnimationTableTemplate *const latTemplate = controller->fetchLogicalAnimationTableTemplate();
		if (!latTemplate)
			continue;

		// Open the LAT in the editor.
		doFileOpen(latTemplate->getName().getString());

		// Release local reference to LAT template.
		latTemplate->release();
	}
}

// ----------------------------------------------------------------------

void MainWindow::openTargetAshFiles()
{
	//-- Get the editor's idea of the focus object.
	const Object *const focusObject = AnimationEditorGameWorld::getFocusObject();
	if (!focusObject)
	{
		DEBUG_REPORT_LOG(true, ("No editor focus object, ignoring request.\n"));
		return;
	}

	//-- Retrieve the skeletal appearance for the object.
	const SkeletalAppearance2 *const appearance = dynamic_cast<const SkeletalAppearance2*>(focusObject->getAppearance());
	if (!appearance)
	{
		DEBUG_REPORT_LOG(true, ("Editor focus object does not have a skeletal appearance, ignoring request.\n"));
		return;
	}

	//-- Get the animation controller for each skeleton template associated with the appearance.
	const TransformAnimationResolver &resolver = appearance->getAnimationResolver();
	const int skeletonTemplateCount            = resolver.getSkeletonTemplateCount();

	for (int i = 0; i < skeletonTemplateCount; ++i)
	{
		// Get the animation controller.
		const StateHierarchyAnimationController *const controller = dynamic_cast<const StateHierarchyAnimationController*>(resolver.getAnimationController(i));
		if (!controller)
			continue;

		// Fetch the ASH data structure from it.
		const AnimationStateHierarchyTemplate *const ashTemplate = controller->fetchHierarchyTemplate();
		if (!ashTemplate)
			continue;

		// Open the ASH in the editor.
		doFileOpen(ashTemplate->getName().getString());

		// Release local reference to ASH template.
		ashTemplate->release();
	}
}

// ----------------------------------------------------------------------

void MainWindow::saveSelectedTab()
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("saveSelectedTab(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	if (!fileModel->save())
	{
		WARNING(true, ("saveSelectedTab(): selected file not saved."));
	}
}

// ----------------------------------------------------------------------

void MainWindow::expandAllChildren()
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("saveSelectedTab(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	fileModel->expandAllChildrenForSelectedItem();
}

// ----------------------------------------------------------------------

void MainWindow::collapseAllChildren()
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("saveSelectedTab(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	fileModel->collapseAllChildrenForSelectedItem();
}

// -----------------------------------------------------------------------

void MainWindow::selectMatchingItem(const char* string, bool onlyReplaceableTextItems)
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("selectMatchingItem(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	fileModel->selectMatchingItem(string,onlyReplaceableTextItems);
}

// -----------------------------------------------------------------------

void MainWindow::replaceSelectedText(const char* substringToReplace,const char* newText)
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("replaceSelectedText(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	fileModel->replaceSelectedText(substringToReplace, newText);
}

// -----------------------------------------------------------------------

void MainWindow::replaceAllText(const char* substringToReplace, const char* newText)
{
	//-- Get currently selected file model.
	FileModel *fileModel = 0;
	int        index     = -1;

	const bool haveSelectedFileModel = getSelectedFileModel(fileModel, index);
	if (!haveSelectedFileModel)
	{
		WARNING(true, ("replaceAllText(): invoked with no tab selected."));
		return;
	}

	//-- Save it.
	NOT_NULL(fileModel);
	fileModel->replaceAllText(substringToReplace, newText);
}

// ======================================================================
// class MainWindow: private static member functions
// ======================================================================

std::string MainWindow::getNextNewTitle()
{
	//-- increment new tab widget counter
	++ms_newTabCounter;

	//-- convert tab to string
	char buffer[32];
	sprintf(buffer, "%d", ms_newTabCounter);

	//-- construct return value
	return ms_unnamedTabNamePrefix + buffer;
}

// ----------------------------------------------------------------------

std::string MainWindow::formatTabName(const std::string &fileTypeName, const std::string &fileTitle, bool isModified)
{
	std::string tabName = fileTypeName + std::string(" - ") + fileTitle;

	//-- add modified marker at front of tab name
	if (isModified)
		tabName += " *";

	return tabName;
}

// ----------------------------------------------------------------------

void MainWindow::staticModificationListener(FileModel &fileModel, bool oldModifiedState, bool newModifiedState, const void *context)
{
	NOT_NULL(context);
	
	MainWindow *const mainWindow = const_cast<MainWindow*>(reinterpret_cast<const MainWindow*>(context));
	mainWindow->modificationListener(fileModel, oldModifiedState, newModifiedState);
}

// ======================================================================
// class MainWindow: private member functions
// ======================================================================

void MainWindow::addFileModel(FileModel *fileModel)
{
	NOT_NULL(fileModel);

	//-- Construct name for new FileModel tab.
	const std::string tabName = formatTabName(fileModel->getFileTypeShortName(), fileModel->getFileTitle(), fileModel->isModified());

	//-- Add FileModel's widget to the tab.
	m_tabWidget->insertTab(fileModel->getWidget(), trUtf8(tabName.c_str()));

	//-- Select new widget.
	m_tabWidget->showPage(fileModel->getWidget());

	//-- Keep track of FileModel instance, in the same order as tabs in m_tabWidget.
	m_fileModelVector->push_back(fileModel);

	//-- Register a FileModel modification listener.
	fileModel->addModificationListener(staticModificationListener, this);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the FileModel instance associated with the selected tab.
 *
 * @return  the FileModel instance associated with the selected tab.
 */

bool MainWindow::getSelectedFileModel(FileModel *&fileModel, int &index)
{
	//-- get index of currently selected page
	index = m_tabWidget->currentPageIndex();
	if (index < 0)
	{
		WARNING(true, ("getSelectedFileModel(): current page index %d indicates there is no current page.", index));
		return false;
	}

	//-- return the associated FileModel instance.
	//   The vector of FileModel instances mirrors the order of tabs.
	if (index >= static_cast<int>(m_fileModelVector->size()))
	{
		WARNING(true, ("getSelectedFileModel(): current page index %d >= # FileModel instances %u.", index, m_fileModelVector->size()));
		return false;
	}

	//-- get the FileModel index
	fileModel = (*m_fileModelVector)[static_cast<FileModelVector::size_type>(index)];
	
	//-- return success
	return true;
}

// ----------------------------------------------------------------------

void MainWindow::modificationListener(FileModel &fileModel,bool oldModifiedState, bool newModifiedState)
{
	UNREF(oldModifiedState);

	//-- construct the new tab name
	const std::string  newTabName = formatTabName(fileModel.getFileTypeShortName(), fileModel.getFileTitle(), newModifiedState);

	//-- update the tab filename
	m_tabWidget->setTabLabel(fileModel.getWidget(), QString(newTabName.c_str()));
}


// ----------------------------------------------------------------------

void MainWindow::doFileOpen(const std::string &treeFileRelativePath)
{
	//-- Ignore the request if a file is already open with the same name.
	{
		const CrcLowerString targetPath(treeFileRelativePath.c_str());

		const FileModelVector::iterator endIt = m_fileModelVector->end();
		for (FileModelVector::iterator it = m_fileModelVector->begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);

			if ((*it)->getFilePath() == targetPath)
			{
				// The specified treefile path is already loaded, skip it.
				return;
			}
		}
	}

	//-- Check if specified file exists.
	if (!TreeFile::exists(treeFileRelativePath.c_str()))
	{
		// File doesn't exist, exit here.
		REPORT_LOG(true, ("Specified file [%s] does not exist, skipping.", static_cast<const char*>(treeFileRelativePath.c_str())));
		return;
	}

	//-- Determine what type of file it is by its extension.
	// Get extension.
	const std::string::size_type  extensionStartPosition = treeFileRelativePath.find('.');
	if (static_cast<int>(extensionStartPosition) == static_cast<int>(std::string::npos))
	{
		WARNING(true, ("could not determine a file type because specified path [%s] has no extension.", treeFileRelativePath.c_str()));
		return;
	}

	const std::string  extension(treeFileRelativePath, extensionStartPosition + 1);

	//-- load item based on extension.
	FileModel *newFileModel = NULL;

	if ((extension == "ash") || (extension == "ash.xml"))
	{
		// Create the hierarchy.
		AnimationStateHierarchyTemplate         *const baseAshTemplate = const_cast<AnimationStateHierarchyTemplate*>(AnimationStateHierarchyTemplateList::fetch(CrcLowerString(treeFileRelativePath.c_str())));
		EditableAnimationStateHierarchyTemplate *const ashTemplate     = dynamic_cast<EditableAnimationStateHierarchyTemplate*>(baseAshTemplate);

		if (!ashTemplate)
		{
			WARNING(true, ("could not handle [%s] as an editable animation state hierarchy template.", treeFileRelativePath.c_str()));
			if (baseAshTemplate)
			{
				baseAshTemplate->release();
				return;
			}
		}

		// Create the widget for the .ash file.
		newFileModel = new AnimationStateHierarchyWidget(ashTemplate, treeFileRelativePath, m_tabWidget);
		addFileModel(newFileModel);

		// Release local reference.
		if (ashTemplate)
			ashTemplate->release();
	}
	else if ((extension == "lat") || (extension == "lat.xml"))
	{
		// Create the lat file.
		LogicalAnimationTableTemplate *const latTemplate = const_cast<LogicalAnimationTableTemplate*>(LogicalAnimationTableTemplateList::fetch(CrcLowerString(treeFileRelativePath.c_str())));

		if (!latTemplate)
		{
			WARNING(true, ("failed to load [%s] as a logical animation table template.", treeFileRelativePath.c_str()));
			return;
		}
		
		// add blank .lat LogicalAnimationName entries that are exported by the .ash but not provided by the user.
		latTemplate->createUnspecifiedEntries();

		// Create the widget for the .lat file.
		newFileModel = new LogicalAnimationTableWidget(m_tabWidget, *latTemplate, treeFileRelativePath);
		addFileModel(newFileModel);

		// Release local reference.
		latTemplate->release();
	}
	else
	{
		WARNING(true, ("openFile(): unsupported extension [%s] on file [%s].", extension.c_str(), treeFileRelativePath.c_str()));
		return;
	}

	//-- Mark the newly loaded file model as unmodified.  Loading a file always causes some activies that make us think the file
	//   has been modified.  Note this could be bad logic if there is an auto-upgrade process that occurs.
	NOT_NULL(newFileModel);
	newFileModel->setModifiedState(false);
}

// ======================================================================
